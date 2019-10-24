/* Train.h
   -	Train stores all the information relating to that train, and providing 
  operations including operating trains, finding available seats, assigning 
  seats, recording data and printing out summaries. Of this, when operating 
  trains, the current time will be compared with the next schedule and then 
  processing getting on and off passengers and updating schedule according 
  to the schedule.
*/

#pragma once

#ifndef TRAIN_H
#define TRAIN_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include "Request.h"
#include "thread.h"
#include "kernel.h"
#include "main.h"
#include "../lib/list.h" 
#include "../lib/bitmap.h" 

class Train {
public:
  Train(int id, char *data){ // constructor
    _id = id;
    _data = data;
    _setAttri();
  }

  void operateTrain(int reserved);
  List<Bitmap*>* getSeatList(); // get the list of available seat
  int getBusFare();
  int getCoachFare();
  int getArrivalTime(int stationId);
  int getBusSeatNum(int stationId);
  int getCoachSeatNum(int stationId);
  void addGrantedRequest(int departureStationId, int destinationStationId, Thread *grantedRequest); // store the unhandled requests
  bool testSeat(Request *req); // test if there are seats available
  List<int>* assignSeat(Request *req); // assign seat
  List<int>* findSeat(Request *req);
  void setCurrentTime(int currentTime);
  int getCurrentTime();
  void summary();
  int getTrainId();
  int getName();

private:
  void _setAttri();
  bool _validate(int startStationId, int endStationId, int seatNum);
  void _checkSchedule();
  void _updateBusiestInfo();

  char *_data;
  int _id;
  int _name;
  int _route[20];
  List<Bitmap*> *_seats;
  int _nextStation;
  int _lastStation;
  int _nextArrivalTime;
  int _lastArrivalTime;
  int _BusFare, _coachFare;
  int _totalRequestNum = 0;
  List<List<Thread*>*> *_departureRequestList; // each station maintain 1 list, used to take on 
  List<List<Thread*>*> *_destintationRequestList; // each station maintain 1 list, used to take off
  int _currentTime = 0;
  int _passangerInNum[20] = {0};
  int _passangerOutNum[20] = {0};
  int _currentPassangerNum = 0;
  int _busiestTime = 0;
  int _busiestStation = -1;
  int _busiestPassengerNum = 0;
};

#endif // RESERVATION_H
