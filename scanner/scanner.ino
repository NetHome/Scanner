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
 
#include "receiver.h"
 
volatile byte state = LOW;
word spaceTime;

void setup() {
  PulseReceiver.begin();
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if ((spaceTime = PulseReceiver.read()) != 0) {
    Serial.print("P");
    printhex(spaceTime);
    printhex(PulseReceiver.read());
    Serial.print("\n");
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

