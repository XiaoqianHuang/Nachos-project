#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "Simulator.h"

void
SimpleThread(int which)
{
    cout << "Start Simulating..\n";
    Simulator *sim = new Simulator();
    sim->Simulating();
}

void
ThreadTest()
{
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
}

