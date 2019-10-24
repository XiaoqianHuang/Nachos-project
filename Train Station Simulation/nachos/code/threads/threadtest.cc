/* threadtest.cc 
   There are 3 threads defined at threadtest.cc: adminThread, trainThread and requestThread. 
  -	threadtest creates an Admin object and forks the adminThread() to start simulation.
  -	adminThread first creates 6 trains according to the stored strings, and then forks the 
  threads for them. Then it starts simulation from 6:00 a.m. to 22:00 p.m. During this time, 
  5 requestThread will be forked at the beginning of every 10 minutes to process request 
  though the Admin. At the end of every minute, all the trains will be synchronized the time.
  -	trainThread runs a while loop within the simulation time and call the train to operate 
  according to the time.
  -	reservationThread generates and acquires the processed result through Admin object, add 
  request to onboard list when request is served and remove it when the service is finished. 
 */

#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "Admin.h"
#include "Request.h"
#include "Train.h"

void
ReservationThread(Admin *admin) {

  Request *newReq = admin->processReservation(kernel->currentThread);

  if(newReq == NULL){
    kernel->currentThread->Finish();
  }

  kernel->interrupt->SetLevel(IntOff);
  kernel->currentThread->Sleep(FALSE);

  // handling request :onboard
  admin->addOnBoard(newReq);
  kernel->currentThread->Sleep(FALSE);

  // handling request :offboard
  admin->removeOnBoard(newReq);
  kernel->currentThread->Finish();
}

void
TrainThread(Train *Train) {
  while (Train->getCurrentTime() < 22 * 60) {
    Train->operateTrain(1);
    kernel->currentThread->Yield();
  }
}

void
AdminThread(Admin *admin)
{
  int requestNum = 0; // total request
  int grantedRequestNum = 0, refusedRequestNum = 0;
  int previousGrantedNum = 0;
  int previousRefusedNum = 0;

  admin->createTrains();

  // create train threads
  List<Train*>* trainList = admin->getTrainList();
  for (int i = 0; i < trainList->NumInList(); i++) {
    Thread *trainThread = new Thread("Train Thread");

    trainThread->Fork((VoidFunctionPtr)TrainThread, (Train*)trainList->getItem(i));
    admin->addTrainThread(trainThread);
  }

  for (int hour = 6; hour < 22; hour++) {
    for (int minute = 0; minute < 60; minute++) {
      admin->setCurrentTime(hour*60 + minute); // synchronize time
      if (minute % 10 == 0) {
        printf("*****[%02d:%02d]*****\n", hour, minute);
        for (int n = 0; n < 5; n++) { // generate 5 requests
          Thread *newReq = new Thread("Reservation Thread");
          newReq->Fork((VoidFunctionPtr)ReservationThread, (Admin*)admin);
        }

        kernel->currentThread->Yield(); // yield to other threads

        cout << "# Granted Request: " << admin->getGrantedNum() - previousGrantedNum << "\n";
        cout << "# Refused Request: " << admin->getRefusedNum() - previousRefusedNum << "\n";
        previousGrantedNum = admin->getGrantedNum();
        previousRefusedNum = admin->getRefusedNum(); // update data
      }

      admin->synchronizeTrains();
      kernel->currentThread->Yield();
    }
  }

  cout << "\n--------------------------------------------------------------------------------\n\n";
  cout << "Simulation Summary: \n";
  cout << "# Request: " << admin->getRequestNum() << "\n";
  cout << "# Granted Request: " << admin->getGrantedNum() << "\n";

  cout << "\n";

  admin->summaryTrains();
  cout << "\n* Assumption: The busiest section is the time when the number of passengers on the train is max (i.e. new passengers take on and old passengers not get off yet). \n";
}

void
ThreadTest()
{
  srand((unsigned int)time(0));
  Admin *admin = new Admin();
  Thread *t = new Thread("Admin thread");
  t->Fork((VoidFunctionPtr)AdminThread, (Admin*) admin);
}

