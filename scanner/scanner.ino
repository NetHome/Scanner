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
 
#define REC_BUFFER_LEN 16

const byte rfInputPin = 2;
volatile byte state = LOW;
word scannedPulses[REC_BUFFER_LEN];
volatile byte nextRead = 0;
volatile byte nextWrite = 0;
volatile word counter = 0;
volatile byte lastRfInput = HIGH;
volatile byte now; 

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
  digitalWrite(LED_BUILTIN, !on);
}

static const char hexString[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static void printhex(word data) {
  Serial.print(hexString[(data >> 12) & 0xF]);
  Serial.print(hexString[(data >> 8) & 0xF]);
  Serial.print(hexString[(data >> 4) & 0xF]);
  Serial.println(hexString[(data) & 0xF]);
}

void setup() {
  cli();
  TCCR1A = 0;// clear TCCR1A register
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 0xFFFF;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
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
    printhex(read());
  }
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
  state = !state;
}

/**
 * Interrupt service routine for changes on the RF input pin
 */
void flank() {
  state = !state;
  now = digitalRead(rfInputPin);
  if (now != lastRfInput) {
    lastRfInput = now;
    scannedPulses[nextWrite] = TCNT1;
    TCNT1 = 0;
    if (now) {
      scannedPulses[nextWrite] |= 0xF000;
    }
    nextWrite = (nextWrite + 1) % REC_BUFFER_LEN;
  }
}
