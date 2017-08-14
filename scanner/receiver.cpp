

#include "receiver.h"
#include "Arduino.h"

#define SPACE_INPUT LOW
#define MARK_INPUT HIGH

const byte rfInputPin = 2;

word Receiver::scannedPulses[REC_BUFFER_LEN];
volatile byte Receiver::nextRead = 0;
volatile byte Receiver::nextWrite = 0;
volatile byte Receiver::nextWriteCandidate = 0;
volatile word Receiver::counter = 0;
volatile byte Receiver::lastRfInput = HIGH;
volatile byte Receiver::now;
volatile unsigned long Receiver::space;
volatile word Receiver::mark;
volatile byte Receiver::scanState = 0;
volatile byte Receiver::scanOverflow = 0;

void Receiver::start() {
  cli();
  TCCR1A = 0;// clear TCCR1A register
  TCCR1B = 0;// clear TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  OCR1A = 0xFFFF; // set compare match register
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS11); // Set CS11 bits for 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
  pinMode(rfInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rfInputPin), Receiver::flank, CHANGE);
}

void Receiver::flank() {
  // Save counter and reset
  counter = (TCNT1 >> 1);
  TCNT1 = 0;
  
  now = digitalRead(rfInputPin);
  if ((scanState == 0) && (now == MARK_INPUT)) {
    // Handle start of mark
    space += counter;
    if (space > 0xFFFF) {
      space = 0xFFFF;
    }  
    scanState = 1;
  } else if ((scanState == 1) && (now == SPACE_INPUT)) {
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
    // Handle error?  
    scannedPulses[nextWrite] = 0;
    scannedPulses[nextWrite + 1] = 0;
    nextWrite = (nextWrite + 2) % REC_BUFFER_LEN;
    if (nextWrite == nextRead) {
      scanOverflow = 1;
    }
    scanState = 0;
  }
}
