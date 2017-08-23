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
 
#define INPUT_BUFFER_SIZE 20
#define FIRMWARE_VERSION "VOpenNetHome 1.0"

static char inputBuffer[INPUT_BUFFER_SIZE];
volatile byte state = LOW;

void setup() {
  PulseReceiver.begin(2);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  handlePulses();
  handleInput();
}

void handlePulses() {
  static word space;
  static word mark;
  if ((space = PulseReceiver.read()) != 0) {
    mark = PulseReceiver.read();
    Serial.print("P");
    printhex(space);
    printhex(mark);
    Serial.print("\n");
    vet(space);
    vet(mark);
  }
}

void handleInput() {
  if (Serial.available() > 0) {
    char result = Serial.readBytesUntil('\n', inputBuffer, INPUT_BUFFER_SIZE);
    inputBuffer[result] = 0;
    interpretCommand(inputBuffer);
  }
}

void interpretCommand(char* inputBuffer) {
  if (inputBuffer[0] == 'V' ) {
    Serial.println(FIRMWARE_VERSION);
  } else {
    Serial.println("e");
  }
  Serial.flush();
}

void vet(word pulse) {
  static int goodCounter = 0;
  
  if ((pulse > 200) && (pulse < 3000)) {
    goodCounter += 1;
    if (goodCounter >= 3) {
      goodCounter = 3;
      activityLed(HIGH);
    }
  } else {
    goodCounter = 0;
    if (goodCounter <= 0) {
      goodCounter = 0;
      activityLed(LOW);
    }
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

