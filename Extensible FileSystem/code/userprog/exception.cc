// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);
    int result;
    char *string = new char[100];

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:{	
        DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

        SysHalt();

        ASSERTNOTREACHED(); }

	break;

      case SC_Add: {

        DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

        /* Process SysAdd Systemcall*/
        result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
          /* int op2 */(int)kernel->machine->ReadRegister(5));

        DEBUG(dbgSys, "Add returning with " << result << "\n");
        /* Prepare Result */
        kernel->machine->WriteRegister(2, (int)result);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }

        return;

        ASSERTNOTREACHED();
      }
	break;
      
      case SC_SysRead: {
        DEBUG(dbgSys, "SysRead.\n");
        // void SysRead(char* buffer, int size);
        int readbuffer = (int)kernel->machine->ReadRegister(4);
        int readsize = (int)kernel->machine->ReadRegister(5);
        result = SysSRead(readbuffer, readsize); // the content is stored at buffer

        cout << "The read content is: [";

        for (int i = 0; i < readsize; i++) {
          int temp;
          kernel->machine->ReadMem((readbuffer+i), 1, &temp);
          cout << (char)temp << flush;

        }
        cout << "]\n";

        kernel->machine->WriteRegister(2, result); // set return value 

                /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }

        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_SysWrite: {
        DEBUG(dbgSys, "SysWrite.\n");
        //void SysWrite(char* buffer, int size);
        int writebuffer = (int)kernel->machine->ReadRegister(4);
        int wirtesize = (int)kernel->machine->ReadRegister(5);

        result = SysSWrite(writebuffer, wirtesize); // the content is stored at buffer
        kernel->machine->WriteRegister(2, result);// set return value 

                /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_SysFork: {
        DEBUG(dbgSys, "Thread Fork.\n");
        // ThreadId ThreadFork(void (*func)());
        int func = (int)kernel->machine->ReadRegister(4);
        result = SysFork(func);
        kernel->machine->WriteRegister(2, result);// set return value 

                /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Exec: {
        DEBUG(dbgSys, "Exec.\n");
        //SpaceId Exec(char* exec_name);
        //int output = 0;
        int addr = kernel->machine->ReadRegister(4);

        char *execName = getString(string, addr);
        result = checkCMD(execName); // command
        if (result == -1) {
           result = SysExec(execName); // finename
        }

        kernel->machine->WriteRegister(2, result);// set return value 
                /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Exit: {
        DEBUG(dbgSys, "Exit.\n");
        // void Exit(int status);
        int status = (int)kernel->machine->ReadRegister(4);
        SysExit();
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Join: {
        DEBUG(dbgSys, "Join.\n");
        // void Exit(int status);
        int childid = (int)kernel->machine->ReadRegister(4);
        result = SysJoin(childid);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Create: {
        DEBUG(dbgSys, "Create.\n");
        // int Create(char *name, int protection);
        int createNameAddr = (int)kernel->machine->ReadRegister(4);
        int createProtection = (int)kernel->machine->ReadRegister(5);

        //int output2 = 0;
        //char createName[100] = { 0 };
        //int count2 = 0;
        //kernel->machine->ReadMem(createNameAddr, 1, &output2);
        //createName[0] = (char)output2;
        //while ((char)output2 != '\0') {
        //  count2++;
        //  kernel->machine->ReadMem(createNameAddr + count2, 1, &output2);
        //  createName[count2] = (char)output2;
        //}
        char *createName = getString(string, createNameAddr);
        DEBUG(dbgSys, "name " << createName);
        DEBUG(dbgSys, "pro " << createProtection);
        result = SysCreate(createName, createProtection);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Remove: {
        DEBUG(dbgSys, "Remove.\n");
        // int Remove(char *name);
        int removeNameAddr = (int)kernel->machine->ReadRegister(4);

        char* removeName = getString(string, removeNameAddr);

        result = SysRemove(removeName);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
         break;

      case SC_Open: {
        DEBUG(dbgSys, "Open.\n");
        // OpenFileId Open(char *name, int mode);
        int openNameAddr = (int)kernel->machine->ReadRegister(4);
        int openMode = (int)kernel->machine->ReadRegister(5);

        char *openName = getString(string, openNameAddr);

        DEBUG(dbgSys, "name " << openName);
        DEBUG(dbgSys, "mode " << openMode);

        result = SysOpen(openName, openMode);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Write: {
        DEBUG(dbgSys, "Write.\n");
        // int Write(char *buffer, int size, OpenFileId id);
        int writeNameAddr = (int)kernel->machine->ReadRegister(4);
        int writeSize = (int)kernel->machine->ReadRegister(5);
        int writeId = (int)kernel->machine->ReadRegister(6);

        if (writeSize > 100) {
          cout << "size too large\n";
          result = -1;
        }
        else {
          //char *writeContent = getString(string, writeNameAddr);
          result = SysWrite(writeNameAddr, writeSize, writeId);
        }

        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
         break;

      case SC_Read: {
        DEBUG(dbgSys, "Read.\n");
        // int Read(char *buffer, int size, OpenFileId id);
        int readNameAddr = (int)kernel->machine->ReadRegister(4);
        int readSize = (int)kernel->machine->ReadRegister(5);
        int readId = (int)kernel->machine->ReadRegister(6);

        if (readSize > 100) {
          cout << "size too large\n";
          result = -1;
        }
        else {
          result = SysRead(readNameAddr, readSize, readId);
        }
        kernel->machine->WriteRegister(2, result);// set return value 

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Seek: {
        DEBUG(dbgSys, "Seek.\n");
        // int Seek(int position, OpenFileId id);
        int seekPos = (int)kernel->machine->ReadRegister(4);
        int seekOpenId = (int)kernel->machine->ReadRegister(5);

        result = SysSeek(seekPos, seekOpenId);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      case SC_Close: {
        DEBUG(dbgSys, "close.\n");
        // int Close(OpenFileId id);
        int closeOpenId = (int)kernel->machine->ReadRegister(4);

        result = SysClose(closeOpenId);
        kernel->machine->WriteRegister(2, result);// set return value 
        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        return;
        ASSERTNOTREACHED();
      }
        break;

      default: {
        cerr << "Unexpected system call " << type << "\n";
      }
	break;
      }
      break;

    case PageFaultException: {
      kernel->stats->numPageFaults++;

      // fetch virtual page that raises the exception
      DEBUG(dbgSys, "In page fault exception handler:");
      int pageFaultId = (int)kernel->machine->ReadRegister(BadVAddrReg);
      // calculate virtual page number
      int pageFaultPageNum = pageFaultId / PageSize;

      // check the free physical number
      int physicalPageNum = kernel->freeMap->FindAndSet();
      // fetch pagefault entry of the current thread
      TranslationEntry *pageEntry = kernel->currentThread->space->getPageEntry(pageFaultPageNum);

      if (physicalPageNum != -1) { // in physical memory
        pageEntry->physicalPage = physicalPageNum;
        pageEntry->valid = TRUE;
        kernel->swapSpace->ReadAt(&(kernel->machine->mainMemory[physicalPageNum*PageSize]), PageSize, pageEntry->virtualPage*PageSize);
        kernel->stats->memRefNum = kernel->stats->memRefNum + PageSize;
        if (kernel->isRandom == TRUE) {
          kernel->FIFO->Append(pageEntry);
        }
        else
        {
          kernel->EntryCache->set(kernel->stats->totalTicks, pageEntry); // LRU
        }

      }
      else {
        TranslationEntry *LRUEntry;
        if (kernel->isRandom == TRUE) { // randomly pick
          srand((unsigned int)time(0));
          int random = rand() % 128;
          TranslationEntry *randomEntry = kernel->FIFO->getItem(random);
          LRUEntry = randomEntry;
          kernel->FIFO->Remove(randomEntry);
        }
        else
        {
          LRUEntry = kernel->EntryCache->oldestNode()->entry; // LRU
        }
        // swap out
        
        // fetch the physical page number of the evicted page
        int physicalPageNum = LRUEntry->physicalPage;
        //if (LRUEntry->dirty == TRUE) { // if the file is modifed.
          // copy evicted from memory to disk
        kernel->swapSpace->WriteAt(&(kernel->machine->mainMemory[physicalPageNum*PageSize]), PageSize, LRUEntry->virtualPage*PageSize); // write back
        LRUEntry->physicalPage = -1;
        LRUEntry->valid = FALSE;
        //}

        if (kernel->isRandom == FALSE) {
          cout << "Swapping out from " << LRUEntry->virtualPage << "(last used time: " << kernel->EntryCache->oldestNode()->LRUTime << " to " << pageEntry->virtualPage << " at phy #" << physicalPageNum << "\n";
        }

        // swap in
        pageEntry->physicalPage = physicalPageNum;
        pageEntry->valid = TRUE;
        kernel->swapSpace->ReadAt(&(kernel->machine->mainMemory[physicalPageNum*PageSize]), PageSize, pageEntry->virtualPage*PageSize);
        if (kernel->isRandom == TRUE) { // randomly pick
          kernel->FIFO->Append(pageEntry);
        }
        else // LRU
        {
          TranslationEntry * removedNode = kernel->EntryCache->set(kernel->stats->totalTicks, pageEntry);
          ASSERT(removedNode == LRUEntry);
        }
      }

      if (kernel->useTLB == TRUE) { // use TLB
        kernel->machine->UpdateTLB(pageEntry);
      }

      DEBUG(dbgSys, "Memory Referrence Num:" << kernel->stats->memRefNum);
      return;
      ASSERTNOTREACHED();
    }
                         
      break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
