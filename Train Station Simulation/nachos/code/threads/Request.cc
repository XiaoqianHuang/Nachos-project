#include "Request.h"

void Request::_generateRequest(int currentTime) {
  _departureStation = rand() % 20; // 0-19
  _destinationStation = rand() % 20;
  while (_destinationStation == _departureStation) {
    _destinationStation = rand() % 20;
  }
  _class = rand() % 2;
  _passenger_Num = rand() % 8 + 1;
  _departureTime = rand() % (60 * 22 - currentTime) + currentTime; // no later than current time
  cout << "New Request: from " << _departureStation << " to " << _destinationStation << " pNum: "
    << _passenger_Num << " class: " << _class << " departureTime: ";
    printf("%02d:%02d \n", _departureTime/60, _departureTime%60);
}

int Request::getId() {
  return _id;
}

int Request::getDepartureStation() {
  return _departureStation;
}

int Request::getDestinationStation() {
  return _destinationStation;
}

int Request::getClass() {
  return _class;
}

int Request::getPassengerNum() {
  return _passenger_Num;
}

int Request::getDepartureTime(){
  return _departureTime;
}

void Request::setGrantedTrainId(int id) {
  _grantedTrainId = id;
}

void Request::setSeatId(List<int> *seatIdList) {
  _seatIdList = seatIdList;
}

int Request::getGrantedTrainId() {
  return _grantedTrainId;
}

int Request::printInfo(){
  cout << "@Request " << _id << "from " << _departureStation << " to " << _destinationStation <<
  "with " << _passenger_Num << " passengers assigned to train " << _grantedTrainId << "\n";
}
