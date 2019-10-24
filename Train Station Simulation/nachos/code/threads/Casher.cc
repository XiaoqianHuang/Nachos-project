#include "Casher.h"

int Casher::CalculateServiceTime(int itemNum) {
  int serviceTime = (10 + 5 * itemNum + 90) / 60;
  if ((10 + 5 * itemNum + 90) % 60 > 0) { // round up
    serviceTime++;
  }
  //Total Service Time
  stringstream str;
  str << "[Casher " << _id << "]New Customer! [Waiting Time: " << _currentCus->getStartTime() - _currentCus->getArrivalTime() << "][Total Service Time: " << serviceTime << "]";
  DEBUG('z', str.str());
  return serviceTime;
}

int Casher::NextMove(int minuteCount) {
  if (_remainServiceTime == 0) { // last customer finished
    //Serving Finished
    // # Step 1, Dequeue the next customer to be served
    if (_line->IsEmpty()) { // if checkout line is empty
      if (_waitingLine->IsEmpty()) {
        return -1; // the line closed
      }
      Customer *p_cus = _currentCus;
      _currentCus = _waitingLine->RemoveFront();
      _currentCus->setStartTime(minuteCount);
      _remainServiceTime = CalculateServiceTime(_currentCus->getItemNum()); // serve next customer from single waiting line
      _totalServiceTime = _remainServiceTime;
      delete p_cus;
    }
    else if (!_line->IsEmpty()) { // serve checkout line customer, dequeue from checkout line
      Customer *p_cus = _currentCus;
      _currentCus = _line->RemoveFront();
      _currentCus->setStartTime(minuteCount);
      _remainServiceTime = CalculateServiceTime(_currentCus->getItemNum()); // serve next customer from checkout line
      _totalServiceTime = _remainServiceTime;
      delete p_cus;
    }

    // # Step 2, move customer to checkout line 
    if (!_waitingLine->IsEmpty()) { // move customer to checkout line
      // enque customer if checkout list not full
      size_t enqueNum = 5 - _line->NumInList();
      if (_waitingLine->NumInList() < enqueNum) {
        enqueNum = _waitingLine->NumInList();
      }
      for (size_t i = 0; i < enqueNum; i++) {
        _line->Append(_waitingLine->RemoveFront());
      }
      stringstream a_str;
      a_str << "[Casher " << _id << "] " << enqueNum << " Customer added to checkout list by casher!";
      DEBUG('z', a_str.str());
    }
  }

  _remainServiceTime--;
  //before next minute _remainServiceTime minutes remain
  stringstream str;
  str << "[Casher " << _id << "]Remain Service Time: {" << _remainServiceTime << "}";
  DEBUG('z', str.str());
  return 1;
}

int Casher::getRemainTime() {
  return _remainServiceTime;
} // get remain service time

int Casher::getTotalTime() {
  return _totalServiceTime;
} // get total service time

bool Casher::addCustomerInLine(Customer *newCus, int minuteCount) {
  if (_line->NumInList() == 0 && _remainServiceTime == 0){// immediately serving
    Customer *p_cus = _currentCus;
    _currentCus = newCus;
    _currentCus->setStartTime(minuteCount);
    _remainServiceTime = CalculateServiceTime(_currentCus->getItemNum()); // serve next customer from single waiting line
    _totalServiceTime = _remainServiceTime;
    delete p_cus;
  }
  else if (_line->NumInList() <5) {
    _line->Append(newCus);
    return true;
  }
  return false;
}

bool Casher::isFull() {
  if (_line->NumInList() >= 5) {
    return true;
  }
  return false;
}

int Casher::getInLineNum() {
  return _line->NumInList();
}

Customer* Casher::getCus(){
  return _currentCus;
}
