#include "Customer.h"

int Customer::getItemNum() {
  return _itemNum;
}

int Customer::getArrivalTime(){
  return _arrivalTime;
}


void Customer::setStartTime(int startTime){
  _startServiceTime = startTime;
}

int Customer::getStartTime(){
  return _startServiceTime;
}
