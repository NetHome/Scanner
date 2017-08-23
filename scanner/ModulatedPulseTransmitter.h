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
 
#include "Arduino.h"

// Number of pulses to store for sending. Has to be an even number since each pulse
// consists of a space followed by a mark.
#define SEND_BUFFER_SIZE 500

class ModulatedPulseTransmitter {
  public:
  
    // Reset the send buffer
    void reset();
    
    // Write the next pulse length in micro seconds to the send buffer. 
    // The first pulse written after a reset() is interpreted as a mark pulse,
    // after that the next pulse is a space pulse, the next mark and so on.
    int write(word pulse);
    
    // Return number of pulses written to the send buffer since last reset()
    int written();
    
    // Send the current send buffer
    void send(byte channel, byte repeat, byte onPeriod, byte offPeriod, byte repeatPoint);
    
  private:
    word sendBuffer[SEND_BUFFER_SIZE];
    int sendBufferPointer = 0;
};

extern ModulatedPulseTransmitter PulseTransmitter;

