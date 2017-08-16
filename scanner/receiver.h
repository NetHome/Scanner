
#include "Arduino.h"

#define REC_BUFFER_LEN 128

class Receiver {

  public:
    void begin(void);
    word read(void);
    
  private:
    static void flankDetected(void);
    byte canRead();
    
    static word scannedPulses[REC_BUFFER_LEN];
    static volatile byte nextRead;
    static volatile byte nextWrite;
    static volatile byte nextWriteCandidate;
    static volatile word counter;
    static volatile byte lastRfInput;
    static volatile byte now;
    static volatile word mark;
    static volatile byte scanOverflow;
    
  public:
    static volatile byte scanState;
    static volatile unsigned long space;
};

extern Receiver PulseReceiver;
