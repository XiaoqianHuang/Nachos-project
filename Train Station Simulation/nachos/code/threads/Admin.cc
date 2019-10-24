#include "Admin.h"

void Admin::simulating() {
}

int Admin::_translateTime(int hour, int minute) {
  return hour * 60 + minute;
}

void Admin::createTrains() {
  _trainList = new List<Train*>();
  List<char*> *fileContent = new List<char*>();
  // Data format: [Train Name] [Station1 ArrivalTime] .. [Station20 ArrivalTime] [Business fair, Coach fair]
  // The data format length of time is fixed.
  fileContent->Append("1 06:00 07:20 08:20 09:10 09:33 10:40 11:30 12:11 13:29 14:20 15:15 15:35 16:43 17:24 18:20 19:45 20:23 20:59 21:54 22:00 20,10");
  fileContent->Append("2 22:00 21:40 20:40 20:00 19:20 18:00 18:23 17:39 16:23 15:45 15:00 14:24 13:56 12:38 11:10 10:00 09:33 08:24 07:12 06:00 25,15");
  fileContent->Append("3 --:-- --:-- --:-- --:-- --:-- --:-- --:-- 06:00 07:20 08:20 09:10 09:33 10:40 11:30 12:11 13:29 14:20 15:15 15:35 16:43 50,20");
  fileContent->Append("4 --:-- --:-- --:-- --:-- --:-- --:-- --:-- 22:00 21:40 20:40 20:00 19:20 18:00 18:23 17:39 16:23 15:45 15:00 14:24 13:56 15,05");
  fileContent->Append("5 --:-- --:-- --:-- --:-- --:-- 06:00 07:20 08:20 09:10 09:33 10:40 11:30 12:11 13:29 14:20 15:15 15:35 16:43 17:24 18:20 20,10");
  fileContent->Append("6 --:-- --:-- --:-- --:-- --:-- --:-- --:-- --:-- --:-- --:-- --:-- --:-- 22:00 21:54 20:59 20:23 19:45 16:43 15:35 15:15 40,20");
   
  int num = fileContent->NumInList();
  for (int i = 0; i < num; i++) {
    char* str = fileContent->RemoveFront();
    Train *newTrain = new Train(i, str);
    _trainList->Append(newTrain);
  }
}

Request* Admin::processReservation(Thread *resThread) { //reservation thread
  _requestNum++;
  int reservationId = _requestNum;
  Request *newRequest = new Request(_requestNum, _timer);
  ListIterator<Train*> iter(_trainList);
  List<Train*> *availableList = new List<Train*>; // need to delete
  int start, end;

  while (iter.IsDone() == FALSE) {
    // Assumption: the departure time of a request is the latest time it can accept to leave apart
    if (iter.Item()->getArrivalTime(newRequest->getDepartureStation()) >= _timer &&
      iter.Item()->getArrivalTime(newRequest->getDepartureStation()) <= newRequest->getDepartureTime() &&
      iter.Item()->getArrivalTime(newRequest->getDestinationStation()) > iter.Item()->getArrivalTime(newRequest->getDepartureStation())) { // time match
      if (iter.Item()->testSeat(newRequest) == true) {
        availableList->Append(iter.Item());
        cout << "[" <<iter.Item()->getName() << "] ";
      }
    }
    iter.Next();
  }
  cout << availableList->NumInList() << " trains can be granted.";
  if (availableList->NumInList() > 0) {
    int selectedTrain = rand() % availableList->NumInList(); // randomly select a qualified train
    selectedTrain = availableList->getItem(selectedTrain)->getTrainId();
    newRequest->setGrantedTrainId(selectedTrain);
    _grantedRequestList->Append(newRequest);
    _trainList->getItem(selectedTrain)->addGrantedRequest(newRequest->getDepartureStation(), newRequest->getDestinationStation(), resThread); // add the request to the train granted list
    cout << " - Request " << newRequest->getId() << " is granted to train[" << _trainList->getItem(newRequest->getGrantedTrainId())->getName() << "] with " << newRequest->getPassengerNum() << " passengers. \n";
    cout << "   Class: " << newRequest->getClass() << ", from " << newRequest->getDepartureStation() << " to " << newRequest->getDestinationStation() << "\n";
  }
  else {
    _refusedRequestList->Append(newRequest);
    cout << " - Request " << newRequest->getId() << " is refused with " << newRequest->getPassengerNum() << " passengers. \n";
    cout << "   Class: " << newRequest->getClass() << ", from " << newRequest->getDepartureStation() << " to " << newRequest->getDestinationStation() << "\n";
    return NULL;
  }
  // assign seat
  List<int> *seatIdList = _trainList->getItem(newRequest->getGrantedTrainId())->assignSeat(newRequest);

  newRequest->setSeatId(seatIdList); // record the assign result in request object

  //delete availableList;
  return newRequest;
}


void Admin::addOnBoard(Request *newReq){
  _onBoardRequestList->Append(newReq);
}

void Admin::removeOnBoard(Request *newReq){
  _onBoardRequestList->Remove(newReq);
}

void Admin::setCurrentTime(int time) {
  _timer = time;
}

int Admin::getRequestNum() {
  return _requestNum;
}

int Admin::getGrantedNum() {
  return _grantedRequestList->NumInList();
}

int Admin::getRefusedNum() {
  return _refusedRequestList->NumInList();
}

int Admin::getTrainNum() {
  return _trainList->NumInList();
}

void Admin::synchronizeTrains() {
  for (int i = 0; i < _trainList->NumInList(); i++) { // synchronize time for trains
    _trainList->getItem(i)->setCurrentTime(_timer);
  }
}

void Admin::summaryTrains() {
  for (int i = 0; i < _trainList->NumInList(); i++) {
    _trainList->getItem(i)->summary(); // print summary information
  }
}

List<Train*>* Admin::getTrainList() {
  return _trainList;
}

void Admin::addTrainThread(Thread* thread) {
  _trainThreadList->Append(thread);
}
