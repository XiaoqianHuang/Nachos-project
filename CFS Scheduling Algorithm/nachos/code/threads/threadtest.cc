#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "ioevent.h"
#include "ioalarm.h"

void
IoThread(IoAlarm *ioAlarm) // launch IO operation
{
  Statistics *stats = kernel->stats;
  IoEvent *newEvent = new IoEvent(rand()%2, rand()%20, kernel->currentThread);
  newEvent->setCompletionTime(newEvent->getWaitingTime() + stats->totalTicks); // set completion time
  cout << "IO Event from [" <<  kernel->currentThread->getName()  << "] is created and added to queue ( interrupt at " << newEvent->getCompletionTime()  << " ticks ). \n";
  kernel->ioEventQueue->Insert(newEvent);
  ioAlarm->SetAlarm(newEvent->getCompletionTime(), newEvent->getType());

  kernel->interrupt->SetLevel(IntOff);
  kernel->currentThread->Sleep(FALSE);

  if (kernel->interrupt->getLevel() == IntOff) {
    kernel->interrupt->SetLevel(IntOn);
  }

  newEvent->operateIo();
  cout << "Total ticks: " << kernel->stats->totalTicks << "\n";
  cout << "IO Event from [" << kernel->currentThread->getName() << "] is finished ( scheduled completed at " << newEvent->getCompletionTime() << " ticks ). \n";

  if (kernel->getTotalFinishedIoThreadNum() >= 6 ) { // terminate the system when all the io events finished
    kernel->interrupt->Halt();
  }
  kernel->currentThread->Finish();
}

void
CPUThread(int which) { // simulate the cpu computation
  for(int i = 0; i < which; i++) {
    if (kernel->interrupt->getLevel() == IntOff) {
      kernel->interrupt->SetLevel(IntOn);
    }
    kernel->interrupt->OneTick();
  }
}

void
MIXThread(IoAlarm* ioAlarm) { // mix cpu and io
  CPUThread(50);
  IoThread(ioAlarm);
  CPUThread(300);
}


char* getThreadName(const char*  type, int i) { // build the thread name to identify each thread
  int intLen = 1, reminder = 0;
  char* num = new char[10];
  reminder = i % 10;
  num[0] = ' ';
  num[1] = (char)(reminder + '0');
  while (i / 10 > 0) {
    intLen++;
    i = i / 10;
    reminder = i % 10;
    num[intLen] = (char)(reminder + '0');
  }

  // reverse string
  int j = intLen;
  for (int s = 1; s <= (intLen + 1) / 2; s++) {
    char temp = num[s];
    num[s] = num[j];
    num[j] = temp;
    j--;
  }

  int totalLen = strlen(type) + intLen;
  char *n_str = new char[totalLen + 1];
  strcpy(n_str, type);
  strcat(n_str, num);

  return n_str;
}


void
ThreadTest()
{
  srand((unsigned int)time(0));
  IoAlarm *ioAlarm = new IoAlarm(FALSE);
  Thread *t;
  int CPUCycles = 10000;

  t = new Thread("IO-Bound 1");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);

  t = new Thread("IO-Bound 2");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);

  t = new Thread("IO-Bound 3");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);

  t = new Thread("IO-Bound 4");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);

  t = new Thread("IO-Bound 5");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);

  t = new Thread("MIX-Thread");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)MIXThread, (void *)ioAlarm);

  t = new Thread("CPU-Bound 1");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 2");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 3");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 4");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 5");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 6");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 7");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 8");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 9");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 10");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 11");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 12");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 13");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 14");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);

  t = new Thread("CPU-Bound 15");
  t->setWeight(1);
  t->Fork((VoidFunctionPtr)CPUThread, (void *)CPUCycles);
}
