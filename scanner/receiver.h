
#include "Arduino.h"

#define REC_BUFFER_LEN 64

class Receiver {

  public:
    void start(void);
    
  private:
    static void flank(void);
    
    static word scannedPulses[REC_BUFFER_LEN];
    static volatile byte nextRead;
    static volatile byte nextWrite;
    static volatile byte nextWriteCandidate;
    static volatile word counter;
    static volatile byte lastRfInput;
    static volatile byte now;
    static volatile unsigned long space;
    static volatile word mark;
    static volatile byte scanState;
    static volatile byte scanOverflow;
};
