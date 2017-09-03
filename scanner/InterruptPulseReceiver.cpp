
/*
 * Copyright (C) 2005-2017, Stefan Strömberg <stefangs@nethome.nu>
 *
 * This file is part of OpenNetHome  (http://opennethome.org)
 *
 * OpenNetHome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenNetHome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "InterruptPulseReceiver.h"

#define MARK_INPUT HIGH
#define DEBOUNCE_TIME 20

word InterruptPulseReceiver::scannedPulses[REC_BUFFER_LEN];
volatile byte InterruptPulseReceiver::nextRead = 0;
volatile byte InterruptPulseReceiver::markLevel = HIGH;
volatile byte InterruptPulseReceiver::nextWrite = 0;
volatile byte InterruptPulseReceiver::nextWriteCandidate = 0;
volatile word InterruptPulseReceiver::counter = 0;
volatile byte InterruptPulseReceiver::now;
volatile unsigned long InterruptPulseReceiver::space;
volatile word InterruptPulseReceiver::mark;
volatile byte InterruptPulseReceiver::scanState = 0;
volatile byte InterruptPulseReceiver::scanOverflow = 0;
volatile byte InterruptPulseReceiver::rfInputPin = 2;

InterruptPulseReceiver PulseReceiver;

word InterruptPulseReceiver::read(void) {
  if (!canRead()) {
    return 0;
  }
  word result = scannedPulses[nextRead];
  nextRead = (nextRead + 1) % REC_BUFFER_LEN;
  return result == 0 ? 1 : result;
}

byte InterruptPulseReceiver::canRead() {
  return nextRead != nextWrite;
}

void InterruptPulseReceiver::begin(byte pin, byte markSignalLevel) {
  cli();
  rfInputPin = pin;
  TCCR1A = 0;// clear TCCR1A register
  TCCR1B = 0;// clear TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  OCR1A = 0xFFFF; // set compare match register
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS11); // Set CS11 bits for 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  markLevel = markSignalLevel;
  sei();
  attachInterrupt(digitalPinToInterrupt(rfInputPin), InterruptPulseReceiver::flankDetected, CHANGE);
  space = 0;
  nextRead = 0;
  nextWrite = 0;
  scanState = 0;
}

void InterruptPulseReceiver::end() {
  detachInterrupt(digitalPinToInterrupt(rfInputPin));
}

inline void InterruptPulseReceiver::timerCompareInterrupt() {
  scanState = 0; // Assume space state on overflow
  space += 0x7FFF;
  if (space > 0xFFFF) {
    space = 0xFFFF;
  }    
}

ISR(TIMER1_COMPA_vect){
  InterruptPulseReceiver::timerCompareInterrupt();
}

/**
 * Interrupt service routine for changes on the RF input pin
 */
void InterruptPulseReceiver::flankDetected() {
  // Save counter and reset
  counter = (TCNT1 >> 1);
  TCNT1 = 0;
  
  // Wait a while for the input to settle
  while (TCNT1 < (DEBOUNCE_TIME << 1));
  
  now = digitalRead(rfInputPin);
  if ((scanState == 0) && (now == markLevel)) {
    // Handle start of mark
    space += counter;
    if (space > 0xFFFF) {
      space = 0xFFFF;
    }  
    scanState = 1;
  } else if ((scanState == 1) && (now == !markLevel)) {
    if (scanOverflow && (nextWrite == nextRead)) {
      // If we were in overflow state, but buffer is now drained, clear the state and signal it
      // by writing special values
      scanOverflow = 0;
      space = 0xFFFF;
      counter = 0xFFFF;
    }
    // Handle end of mark, by writing the pulse
    scannedPulses[nextWrite + 1] = counter;
    scannedPulses[nextWrite] = space;
    nextWriteCandidate = (nextWrite + 2) % REC_BUFFER_LEN;
    if (nextWriteCandidate == nextRead) {
      scanOverflow = 1;
    } else if (!scanOverflow){
      nextWrite = nextWriteCandidate;
    }
    space = 0;
    scanState = 0;
  } else {
    // Not the expected input state - ignore this pulse flank
    TCNT1 = TCNT1 + (counter << 1);
  }
}
