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
#include "ModulatedPulseTransmitter.h"
 
#define INPUT_BUFFER_SIZE 20
#define FIRMWARE_VERSION "VOpenNetHome 1.0"
#define SHORTEST_PULSE 70
#define RF_INPUT_PIN 2
#define IR_INPUT_PIN 3


static char inputBuffer[INPUT_BUFFER_SIZE];
volatile byte state = LOW;
word lastSpace = 0;
word lastMark = 0;

void setup() {
  pinMode(RF_INPUT_PIN, INPUT_PULLUP);
  pinMode(IR_INPUT_PIN, INPUT);
  PulseTransmitter.reset();
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  handlePulses();
  handleInput();
}

inline word limitWord(unsigned long data) {
  return data > 0xFFFF ? 0xFFFF : data;
}

void handlePulses() {
  static word space;
  static word mark;
  if ((space = PulseReceiver.read()) != 0) {
    mark = PulseReceiver.read();
    // Debounce the pulses.
    if (lastMark < SHORTEST_PULSE) {
      lastSpace = limitWord(lastSpace + lastMark + space);
      lastMark = mark;
    } else if (space < SHORTEST_PULSE) {
      lastMark = limitWord(lastMark + space + mark); 
    } else {
      printPulse(lastSpace, lastMark);
      lastSpace = space;
      lastMark = mark;  
    }
  }
}

void printPulse(word space, word mark) {
  Serial.print("P");
  printhex(space);
  printhex(mark);
  Serial.print("\n");  
}

void handleInput() {
  if (Serial.available() > 0) {
    byte result = Serial.readBytesUntil('\n', inputBuffer, INPUT_BUFFER_SIZE);
    inputBuffer[result] = 0;
    interpretCommand(inputBuffer, result);
  }
}

void interpretCommand(char* inputBuffer, byte commandLength) {
  if (inputBuffer[0] == 'A' && commandLength >= 9) {
    addRawPulse(&inputBuffer[0]);
  } else if (inputBuffer[0] == 'V' ) {
    Serial.println(FIRMWARE_VERSION);
  }  else if (inputBuffer[0] == 'R' && commandLength >= 2) {
    if (inputBuffer[1] == '1') {
      PulseReceiver.begin(RF_INPUT_PIN, HIGH);
      Serial.println("o1");
    } else if (inputBuffer[1] == '2') {
      PulseReceiver.begin(IR_INPUT_PIN, LOW);
      Serial.println("o2");
    } else {
      PulseReceiver.end();
      Serial.println("o0");
    }
  } else {
    Serial.println("e");
  }
  Serial.flush();
}

/**
 * Add a new pulse to the transmit buffer.
 * \param in parameter string: "Ammmmssss" where "mmmm" is the mark flank length in us
 * in HEX format and "ssss" is the following space flank
 */
void
addRawPulse(char *in) {
  unsigned char pulseByte;
  word pulseWord;

  // Add the Mark flank
  fromhex(in+1, &pulseByte, 1);					// Read high byte
  pulseWord = pulseByte << 8;
  fromhex(in+3, &pulseByte, 1);					// Read low byte
  pulseWord += pulseByte;
  PulseTransmitter.write(pulseWord);

  // Add the Space flank
  fromhex(in+5, &pulseByte, 1);					// Read high byte
  pulseWord = pulseByte << 8;
  fromhex(in+7, &pulseByte, 1);					// Read low byte
  pulseWord += pulseByte;
  if (PulseTransmitter.write(pulseWord) == 0) {
    Serial.print('o');
    printhex(PulseTransmitter.written());
    Serial.println("");
  } else {
    Serial.println('e');
  }
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

/*
 * Converts a hex string to a buffer. Not hex characters will be skipped
 * Returns the hex bytes found. Single-Nibbles wont be converted.
 */
int fromhex(const char *in, unsigned char *out, int buflen)
{
  unsigned char *op = out, c, h = 0, fnd, step = 0;
  while((c = *in++)) {
    fnd = 0;
    if(c >= '0' && c <= '9') { h |= c-'0';    fnd = 1; }
    if(c >= 'A' && c <= 'F') { h |= c-'A'+10; fnd = 1; }
    if(c >= 'a' && c <= 'f') { h |= c-'a'+10; fnd = 1; }
    if(!fnd) {
      if(c != ' ')
        break;
      continue;
    }
    if(step++) {
      *op++ = h;
      if(--buflen <= 0)
        return (op-out);
      step = 0;
      h = 0;
    } else {
      h <<= 4;
    }
  }
  return op-out;
}

