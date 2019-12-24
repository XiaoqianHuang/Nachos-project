#ifndef LOCKS_H
#define LOCKS_H

#include "synch.h"
#include <map>

class Locks {
public:
  Locks();
  map<string, Lock*> *WRLocks; // locks for read write for each file
  map<string, Lock*> *RCLocks;
};

#endif // !LOCKS_H


