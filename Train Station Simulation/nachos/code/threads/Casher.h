#pragma once
// Casher.h
// Simulate the behavour of the casher and record their status.

#ifndef CAHSER_H
#define CASHER_H
#include "../lib/list.h"
#include "Customer.h"
#include <stdlib.h>
#include "kernel.h"
#include "debug.h"
#include <string>
#include <sstream>

class Casher {
public:
  Casher(List<Customer*> *waitingLine, int id) { // constructor
    //_totalServiceTime = CalculateServiceTime(customer.getItemNum());
    _line = new List<Customer*>();
    _waitingLine = waitingLine;
    _remainServiceTime = 0;
    _id = id;
  }

  int CalculateServiceTime(int itemNum); // calculate the service time according to item number and round up in minute
  int NextMove(int minuteCount);  // next move in 1 minute
  bool addCustomerInLine(Customer *newCus, int minuteCount); // push a customer in line, called by simulator 
  bool isFull();
  int getInLineNum();
  int getRemainTime();
  int getTotalTime();
  Customer* getCus();

private:
  //bool _is_serving; // if the casher is serving a customer
  int _totalServiceTime; // total serving time for the customer in minute.
  int _remainServiceTime; // remain serving time in minute
  List<Customer*> *_line; // the checkout line for casher to service
  List<Customer*>  *_waitingLine;
  Customer *_currentCus = NULL;
  int _id;
};

#endif // CASHER_H
