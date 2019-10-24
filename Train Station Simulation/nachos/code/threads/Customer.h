#pragma once
// Customer.h
// Simulate the behavour of the customer and generate relating data.

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <stdlib.h>

class Customer {
public:
  Customer(int arrTime) { // constructor
    // generate item num
    _itemNum = 5 + rand() % 36;
    _arrivalTime = arrTime;
  }
  void setStartTime(int startTime);
  int getStartTime();

  int getItemNum();
  int getArrivalTime();
private:
  int _itemNum;
  int _arrivalTime; // record in minutes
  int _startServiceTime;
};

#endif // CUSTOMER_H
