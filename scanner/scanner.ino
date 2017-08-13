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
 
#define REC_BUFFER_LEN 64
#define SPACE_INPUT LOW
#define MARK_INPUT HIGH

const byte rfInputPin = 2;
volatile byte state = LOW;
word scannedPulses[REC_BUFFER_LEN];
volatile byte nextRead = 0;
volatile byte nextWrite = 0;
volatile byte nextWriteCandidate = 0;
volatile word counter = 0;
volatile byte lastRfInput = HIGH;
volatile byte now;
volatile unsigned long space;
volatile word mark;
volatile byte scanState = 0;
volatile byte scanOverflow = 0;

static byte canRead() {
  return nextRead != nextWrite;  
}

static byte nextPointer(byte current) {
  return (current + 1) % REC_BUFFER_LEN;
}

static void write(word value) {
  byte nextNextWrite = nextPointer(nextWrite);
  if (nextNextWrite != nextRead) {
    scannedPulses[nextWrite] = value;
    nextWrite = nextNextWrite;
  }
}

static word read() {
  word result = scannedPulses[nextRead];
  nextRead = nextPointer(nextRead);
  return result;
}

static void activityLed (byte on) {
  digitalWrite(LED_BUILTIN, on);
}

static const char hexString[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static void printhex(word data) {
  Serial.print(hexString[(data >> 12) & 0xF]);
  Serial.print(hexString[(data >> 8) & 0xF]);
  Serial.print(hexString[(data >> 4) & 0xF]);
  Serial.print(hexString[(data) & 0xF]);
}

void setup() {
  cli();
  TCCR1A = 0;// clear TCCR1A register
  TCCR1B = 0;// clear TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  // set compare match register
  OCR1A = 0xFFFF;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bits for 8 prescaler
  TCCR1B |= (1 << CS11);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  Serial.begin(115200);
  pinMode(rfInputPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(rfInputPin), flank, CHANGE);
}

void loop() {
  activityLed(state);
  if (canRead()) {
    Serial.print("P");
    printhex(read());
    printhex(read());
    Serial.print("\n");
  }
}

ISR(TIMER1_COMPA_vect){
  scanState = 0; // Assume space state on overflow
  state = 0;
  space += 0x7FFF;
  if (space > 0xFFFF) {
    space = 0xFFFF;
  }  
}

/**
 * Interrupt service routine for changes on the RF input pin
 */
void flank() {
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
    state = 1;
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
