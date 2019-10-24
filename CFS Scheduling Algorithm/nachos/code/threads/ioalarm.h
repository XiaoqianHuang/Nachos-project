// ioalarm.h
// alarm for raising io interrupt in scheduled time

#pragma once
#ifndef IOALARM_H
#define IOALARM_H

#include "utility.h"
#include "callback.h"
#include "iotimer.h"
#include "ioevent.h"
#include "main.h"

// The following class defines an io alarm. 
class IoAlarm : public CallBackObj {
public:
  IoAlarm(bool doRandomYield);	// Initialize the timer, and callback 
      // to "toCall" every time slice.
  ~IoAlarm() { delete iotimer; }

  void WaitUntil(int x);	// suspend execution until time > now + x
                              // this method is not yet implemented

  void SetCompletionTime(int ct) { completionTime = ct; } // set completion time 
  void SetAlarm(int TotalTicks, int type);

private:
  IoTimer *iotimer;		// the hardware timer device

  void CallBack();		// called when the hardware
      // timer generates an interrupt

  int completionTime;
  Statistics *stats;
  int eventNum = 0;

};

#endif // IOALARM_H
