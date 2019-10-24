#pragma once
// Simulator.h
// Simulate the checkout process of the market from 14:00pm to 19:00pm.

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "Casher.h"
#include "Customer.h"
#include "debug.h"
#include <iostream>
#include <string>
#include <sstream>

class Simulator {
public:
  Simulator() { // constructor
  }
 int TimeInMinute(int hour, int minute);

 int OpenCasherCount(int arr[10]);

 int Min(int a, int b);

 int Max(int a, int b);

 void Simulating(); 
};

#endif // SIMULATOR_H
