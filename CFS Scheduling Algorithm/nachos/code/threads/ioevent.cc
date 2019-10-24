#include "ioevent.h"

void IoEvent::_setWaitingTime(){ // generate waiting time randomly
  if (_ioType == 0) { // write, small waiting time, 0-2000
    _waitingTime = rand() % 2001;
  }
  else { // read, large waiting time, 10000-20000
    _waitingTime = rand() % 10001 + 10000 ;
  }
}

int IoEvent::getWaitingTime() {
  return _waitingTime;
}

int IoEvent::getCompletionTime() {
  return _completionTime;
}

void IoEvent::startProcessing(int completionTime) {
  _completionTime = completionTime;
}


void IoEvent::setCompletionTime(int time) {
  _completionTime = time;
}

void IoEvent::operateIo() {
  if (_ioType == 0) {
    cout << "=========== Write Operation Finished! =========== \n";
  }
  else {
    cout << "=========== Read Operation Finished! ============ \n";
  }
}

Thread* IoEvent::getCallingThread() {
  return _callingThread;
}

// io interrupt handler
void IoEvent::CallBack() { 
  cout << "Call back to ioevent: completion time[" << _completionTime << "]\n";
}

int IoEvent::getType() {
  return _ioType;
}