#include "ioalarm.h"

// constructor of io alarm

IoAlarm::IoAlarm(bool doRandom)
{
  iotimer = new IoTimer(doRandom, this);
  stats = kernel->stats;
}

// timer interrupt handler, called by each timer slice (200 ticks)

void
IoAlarm::CallBack()
{
  Interrupt *interrupt = kernel->interrupt;
  MachineStatus status = interrupt->getStatus();

  if (status != IdleMode) {
    interrupt->YieldOnReturn();
  }

  cout << "\nTick: [" << stats->totalTicks << "] IO Interrupt Raised\n";

  // wake up the on due io event's callback, put to list
  while (kernel->ioEventQueue->NumInList() != 0 && kernel->ioEventQueue->Front()->getCompletionTime() <= stats->totalTicks) {
    IoEvent* next = kernel->ioEventQueue->RemoveFront();
    kernel->scheduler->ReadyToRun(next->getCallingThread());
    next->CallBack();
    eventNum++;
    cout << "***************** I/O Event[" << next->getCallingThread()->getName() << "] *****************\n";
    kernel->interrupt->DumpState();
    cout << "[" << eventNum << "] events finished in total.\n";
    kernel->setTotalFinishedIoThreadNum(eventNum);
    cout << "******************************************************************\n";
  }

  if (kernel->ioEventQueue->NumInList() == 0) {
    cout << "No event in the queue any more!\n";
    cout << eventNum << " IO events in total!\n";
  }
}


void IoAlarm::SetAlarm(int TotalTicks, int type) {
  iotimer->SetInterrupt(TotalTicks, type);
}