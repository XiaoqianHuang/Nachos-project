#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "ioevent.h"
#include "ioalarm.h"

#define IO_NUM 5
#define CPU_NUM 15
#define MIX_NUM 1

void
IoThread(IoAlarm *ioAlarm)
{
  Statistics *stats = kernel->stats;
  IoEvent *newEvent = new IoEvent(rand()%2, rand()%20, kernel->currentThread);
  newEvent->setCompletionTime(newEvent->getWaitingTime() + stats->totalTicks); // set completion time
  cout << "Thread [" <<  kernel->currentThread->getName()  << "] is created and added to queue ( interrupt at " << newEvent->getCompletionTime()  << " ticks ). \n";
  kernel->ioEventQueue->Insert(newEvent);
  ioAlarm->SetAlarm(newEvent->getCompletionTime(), newEvent->getType());

  kernel->interrupt->SetLevel(IntOff);
  kernel->currentThread->Sleep(FALSE);

  if (kernel->interrupt->getLevel() == IntOff) {
    kernel->interrupt->SetLevel(IntOn);
  }

  newEvent->operateIo();
  cout << "Total ticks: " << kernel->stats->totalTicks << "\n";
  cout << "Thread [" << kernel->currentThread->getName() << "] is finished ( scheduled completed at " << newEvent->getCompletionTime() << " ticks ). \n";

  if (kernel->getTotalFinishedIoThreadNum() >= IO_NUM) {
    kernel->interrupt->Halt();
  }
  kernel->currentThread->Finish();

}

void
CPUThread(int which) {
  for(int i = 0; i < 500; i++) {
    if (kernel->interrupt->getLevel() == IntOff) {
      kernel->interrupt->SetLevel(IntOn);
    }
    kernel->interrupt->OneTick();
  }
  kernel->interrupt->Halt();
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
  int IOthreadNum = IO_NUM;
  int CPUThreadNum = CPU_NUM;
  int MIXThreadNum = 0;
  for (int i = 0; i < IOthreadNum; i++) {
    t = new Thread("IO");
    t->setWeight(2);
    t->Fork((VoidFunctionPtr)IoThread, (void *)ioAlarm);
  }
  for (int i = 0; i < CPUThreadNum; i++) {
    t = new Thread("CPU");
    t->setWeight(1);
    t->Fork((VoidFunctionPtr)CPUThread, (void *)i);
  }

  //for (int i = 0; i < MIXThreadNum; i++) {
  //  t = new Thread(getThreadName("MIX Thread", i));
  //  t->setWeight(1);
  //  t->Fork((VoidFunctionPtr)MIXThread, (void *)ioAlarm);
  //}
}
