/* Admin.h
   -	Admin serves as the management system for maintaining all the simulation 
   data and operations including generating trains and requests, scheduling 
   trains, processing requests and recording all the required data. 
*/

#pragma once

#ifndef ADMIN_H
#define ADMIN_H

#include "Request.h"
#include "Train.h"
#include "kernel.h"

#include "main.h"
#include "thread.h"
#include "../lib/utility.h"

class Admin {
public:
  Admin() { // constructor
    _grantedRequestList = new List<Request*>();
    _refusedRequestList = new List<Request*>();
    _onBoardRequestList = new List<Request*>();
    _trainThreadList = new List<Thread*>();
  }
  void simulating();
  void createTrains();
  void setCurrentTime(int time);
  Request* processReservation(Thread *resThread); // reservation thread
  int getRequestNum();
  int getGrantedNum();
  int getRefusedNum();
  int getTrainNum();
  void synchronizeTrains();
  void summaryTrains();
  List<Train*>* getTrainList();
  void addTrainThread(Thread* thread);
  void addOnBoard(Request *newReq);
  void removeOnBoard(Request *newReq);

private:
  int _translateTime(int hour, int minute);

  List<Train*>* _trainList;
  List<Request*>* _grantedRequestList;
  List<Request*>* _refusedRequestList;
  List<Request*>* _onBoardRequestList;
  List<Thread*> *_trainThreadList;
  int _requestNum = 0;
  int _timer = 0;
};

#endif // ADMIN_H
