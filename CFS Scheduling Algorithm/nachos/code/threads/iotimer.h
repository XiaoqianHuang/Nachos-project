#ifndef IOTIMER_H
#define IOTIMER_H

#include "copyright.h"
#include "utility.h"
#include "callback.h"
#include "kernel.h"
#include "../machine/interrupt.h"

// The following class defines a hardware timer. 
class IoTimer : public CallBackObj {
public:
  IoTimer(bool doRandom, CallBackObj *toCall);
  // Initialize the timer, and callback to "toCall"
  // every time slice.
  virtual ~IoTimer() {}

  void Disable() { disable = TRUE; }
  // Turn timer device off, so it doesn't
// generate any more interrupts.
  void SetInterrupt(int TotalTicks, int type);  	// cause an interrupt to occur in the

private:
  bool randomize;		// set if we need to use a random timeout delay
  CallBackObj *callOnDue; // call back to ioalarm
  bool disable;		// turn off the timer device after next
          // interrupt.

  void CallBack();		// called internally when the hardware
      // timer generates an interrupt

          // the future after a fixed or random
      // delay
  Statistics *stats;
};

#endif // IOTIMER_H
