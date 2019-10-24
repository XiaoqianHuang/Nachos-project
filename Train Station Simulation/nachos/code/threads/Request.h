/* Request.h
   -	Request stores all the information relating to the specific request 
   and provides operations to get and set data.
*/

#pragma once

#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>
#include "../lib/list.h" 

class Request {
public:
  Request(int id, int currentTime) { // constructor
    _id = id;
    _generateRequest(currentTime);
  }

  int getId();
  int getDepartureStation();
  int getDestinationStation();
  int getClass();
  int getPassengerNum();
  void setGrantedTrainId(int id);
  void setSeatId(List<int> *_seatIdList);
  int getGrantedTrainId();
  int getDepartureTime();
  int printInfo();

private:
  void _generateRequest(int currentTime);

  int _id;
  int _departureStation = -1; // 0-19
  int _destinationStation = -1; 
  int _class = -1; // 0 for business, 1 for coach
  int _passenger_Num = 0; // 1-8
  int _departureTime = 0;
  int _grantedTrainId = -1; // when -1, the request is not granted
  List<int> *_seatIdList;
};

#endif // REQUEST_H
