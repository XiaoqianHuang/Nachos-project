#include "lock.h"

Locks::Locks() {
    WRLocks = new map<string, Lock*>(); // locks for read write for each file
    RCLocks = new map<string, Lock*>();
}