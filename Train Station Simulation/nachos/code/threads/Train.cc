# include "Train.h"

void Train::_setAttri() {
  char *str = _data;

  _departureRequestList = new List<List<Thread*>*>();
  _destintationRequestList = new List<List<Thread*>*>();
  _seats = new List<Bitmap*>();

  for (int i = 0; i < 20; i++) { // initialize
    List<Thread*> *temp = new List<Thread*>();
    _departureRequestList->Append(temp);
    List<Thread*> *temp2 = new List<Thread*>();
    _destintationRequestList->Append(temp2);
    Bitmap *temp3 = new Bitmap(60);
    _seats->Append(temp3);
  }

  int fare = 0;
  int name = 0;
  while (*str != ' ') {
    name = name * 10 + ((int)(*str - '0'));
    str++;
  }
  _name = name;
  std::cout << "Train " << _name << ": ";

  str++;// for space

  for (int i = 0; i < 20; i++) {
    int hour = 0;
    int minute = 0;
    if(*str != '-'){
      hour = ((int)(*str - '0')) * 10 + ((int)(*(str + 1) - '0'));
      minute = ((int)(*(str + 3) - '0')) * 10 + ((int)(*(str + 4) - '0'));
    }
    if(hour == 0 && minute == 0){
      printf("--:-- ", hour,minute);
    }else{
      printf("%02d:%02d ", hour,minute);
    }
    _route[i] = hour * 60 + minute;
    str = str + 6;
  }

  while (*str != ',') {
    fare = fare * 10 + ((int)(*str - '0'));
    str++;
  }
  _BusFare = fare;
  fare = 0;
  std::cout << "$" << _BusFare << "/";

  str++; // for ','

  while (*str != '\0') {
    fare = fare * 10 + ((int)(*str - '0'));
    str++;
  }
  _coachFare = fare;
  std::cout << "$" << _coachFare << "\n";
  _checkSchedule();

}

void Train::operateTrain(int reserved){ // run the train
  _checkSchedule();
  kernel->interrupt->SetLevel(IntOff);
  if (_currentTime == _nextArrivalTime) { 
    for (int i = 0; i < _departureRequestList->getItem(_nextStation)->NumInList(); i++) { // get in, wake up all the request
      kernel->scheduler->ReadyToRun(_departureRequestList->getItem(_nextStation)->getItem(i));
    }
    cout << "Train[" << _name << "] - ";
    cout << "departure station: " <<  _nextStation << " at time ";
    printf("%02d:%02d \n", _currentTime/60, _currentTime%60);
    cout << "# Itinerary: " << _departureRequestList->getItem(_nextStation)->NumInList() << "\n";
    cout << "# Passenger boarding: " << _passangerInNum[_nextStation] << "\n";
    _seats->getItem(_nextStation)->Print();
    _totalRequestNum += _departureRequestList->getItem(_nextStation)->NumInList();
    _currentPassangerNum += _passangerInNum[_nextStation];
    _updateBusiestInfo();
    _lastStation = _nextStation; // record current station
    _lastArrivalTime = _nextArrivalTime;
    _departureRequestList->getItem(_nextStation)->RemoveAll();
  }    
  else if(_currentTime == _lastArrivalTime+10){ // time to leave, get out. The departure time is 10 minute after the arrival time
    for (int i = 0; i < _destintationRequestList->getItem(_lastStation)->NumInList(); i++) { // get in
      kernel->scheduler->ReadyToRun(_destintationRequestList->getItem(_lastStation)->getItem(i));
    }
    cout << "Train[" << _name << "] - ";
    cout << "destination station: " <<  _lastStation << " at time ";
    printf("%02d:%02d \n", _currentTime/60, _currentTime%60);
    cout << "# Passenger getting off: " << _passangerOutNum[_lastStation] << "\n";
    _seats->getItem(_lastStation)->Print();
    _currentPassangerNum -= _passangerOutNum[_lastStation];
    _destintationRequestList->getItem(_lastStation)->RemoveAll();
  }
}

List<Bitmap*>* Train::getSeatList() {
  return _seats;
}

int Train::getBusSeatNum(int stationId) {
  int num = 0;
  for (int i = 0; i < 20; i++) {
    if (_seats->getItem(stationId)->Test(i) == false) {
      num++;
    }
  }
  return num;
}

int Train::getCoachSeatNum(int stationId) {
  int num = 0;
  for (int i = 20; i < 60; i++) {
    if (_seats->getItem(stationId)->Test(i) == false) {
      num++;
    }
  }
  return num;
}

int Train::getBusFare() {
  return _BusFare;
}

int Train::getCoachFare() {
  return _coachFare;
}

int Train::getArrivalTime(int stationId) {
  return _route[stationId];
}

void Train::addGrantedRequest(int departureStationId, int destinationStationId, Thread *grantedRequest) {
  _departureRequestList->getItem(departureStationId)->Append(grantedRequest);
  _destintationRequestList->getItem(destinationStationId)->Append(grantedRequest);
}

List<int>* Train::assignSeat(Request *req) {
  int start, end;
  List<int> *seatIdList = findSeat(req);
  if(seatIdList == NULL){
    return NULL;
  }
  List<int> *assignedSeatIdList = new List<int>;
  if (req->getDepartureStation() > req->getDestinationStation()) { // reverse direction
    start = req->getDestinationStation() + 1;
    end = req->getDepartureStation();
  }
  else {
    start = req->getDepartureStation();
    end = req->getDestinationStation() - 1;
  }

  for(int j = 0; j < req->getPassengerNum(); j++) {
    for(int i = start; i < end + 1; i++){
      _seats->getItem(i)->Mark(seatIdList->getItem(j));
    }
      assignedSeatIdList->Append(seatIdList->getItem(j));
  }

  _passangerInNum[req->getDepartureStation()] += req->getPassengerNum(); // record passenger num
  _passangerOutNum[req->getDestinationStation()] += req->getPassengerNum();

  //delete seatIdList;
  return assignedSeatIdList;
}

bool Train::testSeat(Request *req) {
  List<int> *seatIdList = findSeat(req);
  if (seatIdList != NULL) { // available
    //delete seatIdList;
    return true;
  }
  //delete seatIdList;
  return false;
}

List<int>* Train::findSeat(Request *req) {
  int start, end;
  bool hasSeat = TRUE;
  int seatId;
  List<int> *seatIdList = new List<int>();
  int count = 0;
  if (req->getDepartureStation() > req->getDestinationStation()) { // reverse direction
    start = req->getDestinationStation() + 1;
    end = req->getDepartureStation();
  }
  else {
    start = req->getDepartureStation();
    end = req->getDestinationStation() - 1;
  }

  if (req->getClass() == 0) { // business
    for (int j = 0; j < 20; j++) {
      if (_seats->getItem(start)->Test(j) == false) { // find the available seat of the first station
        if (_validate(start + 1, end, j) == true) {
          count++;
          seatIdList->Append(j);
        }
      }
    }
  }
  else { // coach 
    for (int j = 20; j < 60; j++) {
      if (_seats->getItem(start)->Test(j) == false) {
        if (_validate(start + 1, end, j) == true) {
          count++;
          seatIdList->Append(j);
        }
      }
    }
  }
  
  if(count >= req->getPassengerNum()){
    return seatIdList;
  }

  return NULL;
}

bool Train::_validate(int startStationId, int endStationId, int seatNum) { // validate the remain stations
  for (int i = startStationId; i < endStationId + 1; i++) {
    if (_seats->getItem(i)->Test(seatNum) == true) {
      return false;
    }
  }
  return true;
}

void Train::setCurrentTime(int currentTime) {
  _currentTime = currentTime;
}

void Train::_checkSchedule() {
  int min = 1321;
  int stationId = -1;
  for (int i = 0; i < 20; i++) {
    int time = _route[i];
    if (time >= _currentTime) {
      if (time <= min) {
        min = time;
        stationId = i;
      }
    }
  }
  _nextArrivalTime = min;
  _nextStation = stationId;
}

void Train::_updateBusiestInfo() {
  if (_busiestPassengerNum < _currentPassangerNum) {
    _busiestTime = _currentTime;
    _busiestStation = _nextStation;
    _busiestPassengerNum = _currentPassangerNum;
  }
}

void Train::summary() {
  cout << "Train " << _name << " Summary: \n";
  cout << "# Total served itinerary: " << _totalRequestNum << "\n";
  int pNum = 0;
  for (int i = 0; i < 20; i++) {
    pNum += _passangerInNum[i];
  }
  cout << "# Total passengers: " << pNum << "\n";
  cout << "Busiest Section: ";
  printf("%02d:%02d",_busiestTime / 60, _busiestTime % 60);
  cout << " at station #" << _busiestStation << " with " << _busiestPassengerNum << " passengers on the train.\n";
}


int Train::getCurrentTime(){
  return _currentTime;
}

int Train::getTrainId(){
  return _id;
}

int Train::getName(){
  return _name;
}
