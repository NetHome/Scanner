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
 
#include "ModulatedPulseTransmitter.h"

void ModulatedPulseTransmitter::reset() {
  sendBufferPointer = 0;
}

int ModulatedPulseTransmitter::write(word pulse) {
  if (sendBufferPointer < SEND_BUFFER_SIZE) {
    sendBuffer[sendBufferPointer++] = pulse;
    return 0;
  }
  return 1;
}

int ModulatedPulseTransmitter::written() {
  return sendBufferPointer;
}

ModulatedPulseTransmitter PulseTransmitter;

