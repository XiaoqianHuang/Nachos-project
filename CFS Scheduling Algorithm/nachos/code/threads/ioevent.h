#pragma once

#ifndef IOEVENT_H
#define IOEVENT_H

#include "thread.h"

class IoEvent {
public:
  IoEvent(int ioType, int parameter, Thread *callingThread) {
    _ioType = ioType;
    _parameter = parameter;
    _callingThread = callingThread;
    _setWaitingTime();
  }

  int getWaitingTime();
  void startProcessing(int currentTime);
  int getCompletionTime();
  void setCompletionTime(int time);
  void operateIo();
  void CallBack(); // interrupt handler
  Thread* getCallingThread();
  int getType(); // 0: write

private:
  void _setWaitingTime();

  int _ioType; // 0: write, 1: read
  Thread *_callingThread;
  char* _buffer;
  int _parameter; // size of buffer (counting by char)
  int _waitingTime;
  int _completionTime; // random waiting time + starting running time
  int _executionTime = 0; 
};

#endif // IOEVENT_H