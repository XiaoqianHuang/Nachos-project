#include "Simulator.h"

int Simulator::TimeInMinute(int hour, int minute){
  return 60 * (hour-14) + minute;
}

int Simulator::OpenCasherCount(int arr[10]) {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (arr[i] == 1) {
      count++;
    }
  }
  return count;
}

int Simulator::Min(int a, int b) {
  if (a > b) {
    return b;
  }
  return a;
}

int Simulator::Max(int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

void Simulator::Simulating() {
  srand((unsigned int)time(0));

  Casher *casher_arr[10]; // array to store the addr of casher object
  int open_flag[10] = {0}; // flag to indicate if a checkout line is closed
  List<Customer*>  *waitingLine = new List<Customer*>(); // the single waiting line
  int waitingNum = 0;

  // initialize data, only 2 line open 
  for(int i = 0; i < 10; i++){
    casher_arr[i] = new Casher(waitingLine, i);
  }
  open_flag[0] = 1;
  open_flag[1] = 1;

  // variables for entire simulation 
  double total_avrWaitimgTimeSum = 0, total_avrServiceSum = 0;
  int total_minWaitingTime = 300, total_maxWaitingTime = 0, total_minServiceTime = 300, total_maxServiceTime = 0;
  int total_maxWLNum = 0;
  
  // start simulating
  for (int hour = 14; hour < 19; hour++) { // simulating hours
    // variables for each hour
    double hour_totalArrivingNum = 0;
    int hour_cusServedNum = 0;
    int hour_maxWaitingTime = 0, hour_minWaitingTime = 10000, hour_totalWaitingTime = 0;
    int hour_maxServiceTime = 0, hour_minServiceTime = 300, hour_totalServiceTime = 0;
    int hour_maxOpenLineNum = 2, hour_totalOpenLineNum = 0;
    int hour_threeCusTime = 0, hour_threeCusTimeSum = 0;
    int hour_maxWLNum = 0, hour_minWLNum = 10000, hour_totalWLNum = 0;

    int offset = 0;
    cout << "[Time: " << hour << ":00 pm]";
    if (hour >= 16 && hour < 18) { // peak time  
      offset = 5; //customer_Num = 5 + rand() % 6; // 5~10
      cout << "Peak Time\n" ;
    }
    else { // regular time
      offset = 0; //customer_Num = rand() % 6; // 0~5
      cout << "Regular Time\n";
    }
    
    for (int minute = 0; minute < 60; minute++) { //simulating minutes
      bool isFull = false;
      
      if (minute < 10){
        stringstream str;
        str << "-" << hour << ":0" << minute << "pm-";
        DEBUG('z', str.str());
      }else{
        stringstream str;
        str << "-" << hour << ":" << minute << "pm-";
        DEBUG('z', str.str());
      }

      // step #0, generate customer number
      int customer_Num = 0;
      customer_Num = offset + rand() % 6;
      stringstream str;
      str << "[" << customer_Num << " Customer(s) are generated.]";
      DEBUG('z', str.str());
      //customer_Num Customer(s) are generate
      hour_totalArrivingNum = hour_totalArrivingNum + customer_Num;

      // step #1, arrange customers
      // check for every casher
      while (isFull == false && customer_Num > 0){
        DEBUG('z', "--> Attempting to arrange to cashers");
        int fullNum = 0;
        for (int i = 0; i < 10; i++) {
          if (customer_Num == 0){
            break;
          }
          if (open_flag[i] == 1 && casher_arr[i]->getInLineNum() < 5) { 
            int old = casher_arr[i]->getInLineNum();
            casher_arr[i]->addCustomerInLine(new Customer(TimeInMinute(hour, minute)),TimeInMinute(hour, minute));
            stringstream cs_str;
            cs_str << "+ Casher " << i << " Checkout line from " << old << " -> " << casher_arr[i]->getInLineNum();
            DEBUG('z', cs_str.str());
            customer_Num--;              
          }else if(open_flag[i] == 1 && casher_arr[i]->getInLineNum() == 5){
              fullNum++;
          }
          if(fullNum == OpenCasherCount(open_flag)){
             //casher opened
             isFull = true;
             break;
          }
        }
      }

      if (customer_Num > 0) { // there are customer not in queue
        // all checkout lines are full
        DEBUG('z', "--> Attempting to arrange to waiting line");
        if (waitingLine->NumInList() < 10) { // waiting queue not full
          int enqueNum = Min(customer_Num, 10 - waitingLine->NumInList());
          for (int i = 0; i < enqueNum; i++) {
            waitingLine->Append(new Customer(TimeInMinute(hour, minute)));
            customer_Num--;
          }
        }
      }

      while (customer_Num > 0) { // there are customer not in queue
        DEBUG('z',"--> Attempting to open new cashers");
        // all lines are full, add casher (must < 10)
        if (OpenCasherCount(open_flag) < 10) {
          for (int i = 0; i < 10; i++) {
            if (open_flag[i] == 0) {
              //cout << "New Casher " << i << " Opened!\n";
              //casher_arr[i] = new Casher(waitingLine);
              open_flag[i] = 1; // mark as opened
              int newNum = Min(5, customer_Num);
              for (int j = 0; j < 5; j++) {
                casher_arr[i]->addCustomerInLine(waitingLine->RemoveFront(), TimeInMinute(hour, minute)); // arrange waiting list to new casher
                casher_arr[i]->getCus()->setStartTime(TimeInMinute(hour, minute));
                if (j < newNum){ // arrange new customers to waiting list
                   waitingLine->Append(new Customer(TimeInMinute(hour, minute)));
                   customer_Num --;
                }
              }
              break;
            }
          }
        }
        else {
          for (int i = 0; i < customer_Num; i++) {          
            waitingLine->Append(new Customer(TimeInMinute(hour, minute)));
          }
          //All Cashers are full
          break;
        }
      }
     
      stringstream w_str;
      w_str << waitingLine->NumInList() << " in waitng line!";
      DEBUG('z',w_str.str());
      DEBUG('z',"---------------Advancing One Minute------------------");
      bool allThreeFlag = true;
      // step #2, advance 1 minute
      for (int i = 0; i < 10; i++) {
        if (open_flag[i] == 1) { // only process the open line
          
          if (casher_arr[i]->getRemainTime() == 0 && casher_arr[i]->getCus() != NULL){ // last customer left, record data.
            hour_cusServedNum ++;
            int newRecord = casher_arr[i]->getCus()->getStartTime() - casher_arr[i]->getCus()->getArrivalTime();
            hour_minWaitingTime = Min(hour_minWaitingTime, newRecord);
            hour_maxWaitingTime = Max(hour_maxWaitingTime, newRecord);
            hour_totalWaitingTime = hour_totalWaitingTime + newRecord;     

            newRecord = casher_arr[i]->getTotalTime();
            hour_minServiceTime = Min(hour_minServiceTime, newRecord);
            hour_maxServiceTime = Max(hour_maxServiceTime, newRecord);
            hour_totalServiceTime = hour_totalServiceTime + newRecord;

            hour_maxOpenLineNum = Max(hour_maxOpenLineNum, OpenCasherCount(open_flag));
          }

          if (casher_arr[i]->NextMove(TimeInMinute(hour, minute)) == -1) { // advance one minute for the casher, check whether the casher need to be closed
            if (OpenCasherCount(open_flag) > 2) { // keep minimum checkout line num
              open_flag[i] = 0;
            }
          }
          if (allThreeFlag == false || casher_arr[i]->getInLineNum() <= 3){
            allThreeFlag = false;
          }
          stringstream c_str;
          c_str << "Casher @" << i << " has #" << casher_arr[i]->getInLineNum() << "# in wating line!";
          DEBUG('z', c_str.str());
        }
      }
     stringstream wt_str;
     wt_str << waitingLine->NumInList() << " in waitng line!\n";
     DEBUG('z',wt_str.str());
     int newRecord = waitingLine->NumInList();
     hour_minWLNum = Min(hour_minWLNum, newRecord);
     hour_maxWLNum = Max(hour_maxWLNum, newRecord);
     hour_totalWLNum = hour_totalWLNum + newRecord; 
      
     if (allThreeFlag == true){
       hour_threeCusTime ++;
     }
   
    hour_totalOpenLineNum = hour_totalOpenLineNum + OpenCasherCount(open_flag);
    stringstream d_str;
    d_str << "Minute summary:\n" << "- Average/shortest/longest waiting time: - / " << hour_minWaitingTime << " / " << hour_maxWaitingTime << "\n" << "- Average/shortest/longest service time: - / " << hour_minServiceTime << " / " << hour_maxServiceTime << "\n" << "- Maximum number of open lines: " << hour_maxOpenLineNum << "\n - Average/smallest/largest number of customers in the waiting queue: - / " << hour_minWLNum << " / " << hour_maxWLNum << "\n";
    DEBUG('z', d_str.str());

    } // minute
    
    total_avrWaitimgTimeSum = total_avrWaitimgTimeSum + hour_totalWaitingTime / (double)hour_cusServedNum;
    total_avrServiceSum = total_avrServiceSum + hour_totalServiceTime / (double)hour_cusServedNum;
    total_minWaitingTime = Min(total_minWaitingTime, hour_minWaitingTime);
    total_maxWaitingTime = Max(total_maxWaitingTime, hour_maxWaitingTime);
    total_minServiceTime = Min(total_minServiceTime, hour_minServiceTime);
    total_maxServiceTime = Max(total_maxServiceTime, hour_maxServiceTime);
    total_maxWLNum = Max(total_maxWLNum, hour_maxWLNum);

    cout << "Hour summary:\n";
    cout << "- Average number of customers arriving for checkout: " << hour_totalArrivingNum/60.00 << "\n";
    cout << "- Average/shortest/longest waiting time: " << hour_totalWaitingTime / (double)hour_cusServedNum << " / " << hour_minWaitingTime << " / " << hour_maxWaitingTime << "\n"; 
    cout << "- Average/shortest/longest service time: " << hour_totalServiceTime / (double)hour_cusServedNum << " / " << hour_minServiceTime << " / " << hour_maxServiceTime << "\n"; 
    cout << "- Average number of open lines: " << hour_totalOpenLineNum / 60.00 << " \n";
    cout << "- Maximum number of open lines: " << hour_maxOpenLineNum << " \n";
    cout << "- Average time each casher will have more than 3 customers standing in line: " << hour_threeCusTime << " \n";
    cout << "- Average/smallest/largest number of customers in the waiting queue: " << hour_totalWLNum / 60.00 << " / " << hour_minWLNum << " / " << hour_maxWLNum << "\n\n"; 
  } // hour
  cout << "\n--------------------------------------------------------------------------------\n\n";
  cout << "Total summary:\n";
  cout << "- Average/shortest/longest waiting time: " << total_avrWaitimgTimeSum / 5.00 << " / " << total_minWaitingTime << " / " << total_maxWaitingTime << "\n"; 
  cout << "- Average/shortest/longest service time: " << total_avrServiceSum / 5.00 << " / " << total_minServiceTime << " / " << total_maxServiceTime << "\n"; 
  cout << "- Maximum number of customers in the waiting queue at any time: " << total_maxWLNum << " \n";
}
