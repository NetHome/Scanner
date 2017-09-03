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

// Number of pulse lengths to buffer. Has to be an even number since each pulse
// consists of a space followed by a mark.
#define REC_BUFFER_LEN 128

/**
 * Receiver class for pulses. This class can only be in one singleton instance
 * "PulseReceiver" since all variables are static for speed in the interrupt routines.
 * The class uses TIMER1 and one input pin supporting interrupts (choosen by the user).
 */
class InterruptPulseReceiver {
  public:
  
    // Start scanning for pulses on the input pin
    void begin(byte pin, byte markLevel);
    
    // Read the next pulse length in micro seconds. The pulses are allways delivered in pairs, 
    // first a space pulse and then a mark pulse. 
    // It is therefore important to call this twise to get both pulses so you don't get out of sync.
    // If there is no data the space pulse is 0, and then there is no need to call a second time
    // for the mark pulse.
    word read(void);
    
    // Stop scanning for pulses, but leaves the input buffer intact so any received data can be read
    void end(void);
    
  private:
    static void flankDetected(void);
    inline byte canRead();
    
    static word scannedPulses[REC_BUFFER_LEN];
    static volatile byte markLevel;
    static volatile byte nextRead;
    static volatile byte nextWrite;
    static volatile byte nextWriteCandidate;
    static volatile word counter;
    static volatile byte now;
    static volatile word mark;
    static volatile byte scanOverflow;
    static volatile byte rfInputPin;
    static volatile byte scanState;
    static volatile unsigned long space;
    
  public:
    inline static void timerCompareInterrupt();
};

extern InterruptPulseReceiver PulseReceiver;

