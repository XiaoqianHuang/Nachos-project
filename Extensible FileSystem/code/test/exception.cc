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
        char *readbuffer = (char*)kernel->machine->ReadRegister(4);
        int readsize = (int)kernel->machine->ReadRegister(5);
        result = SysSRead(readbuffer, readsize); // the content is stored at buffer
        cout << "The read content is: [";
        for (int i = 0; i < readsize; i++) {
          int temp;
          kernel->machine->ReadMem((int)(readbuffer+i), 1, &temp);
          cout << (char)temp;
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
        char *writebuffer = (char*)kernel->machine->ReadRegister(4);
        int wirtesize = (int)kernel->machine->ReadRegister(5);
        cout << "The content to write is: [";
        for (int i = 0; i < wirtesize; i++) {
          cout << (char)*(writebuffer + i);
        }
        cout << "]\n";
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

      case SC_ThreadFork: {
        DEBUG(dbgSys, "Thread Fork.\n");
        // ThreadId ThreadFork(void (*func)());
        VoidNoArgFunctionPtr func = (VoidNoArgFunctionPtr)kernel->machine->ReadRegister(4);
        result = SysThreadFork(func);
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
        int* output;
        char execName[100];
        int count = 0;
        kernel->machine->ReadMem(kernel->machine->ReadRegister(4), 1, output);
        execName[0] = *((char*)(output));
        while (*((char*)(output)) != '\0') {
          kernel->machine->ReadMem(kernel->machine->ReadRegister(4) + count, 1, output);
          count++;
          execName[count] = *((char*)(output));
        }

        result = SysExec(execName);
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

      default: {
        cerr << "Unexpected system call " << type << "\n";
      }
	break;
      }
      break;

    case PageFaultException: {
      // set WriteRegister(2) = PageFaultException // raise exception
      // fetch virtual page that raises the exception
      int pageFaultId = (int)kernel->machine->ReadRegister(BadVAddrReg);
      // calculate virtual page number
      int pageFaultPageNum = pageFaultId / PageSize;

      // check the free physical number
      int physicalPageNum = kernel->freeMap->FindAndSet();
      cout << "phy addr:" << physicalPageNum << "\n";
      // fetch pagefault entry of the current thread
      TranslationEntry *pageEntry = kernel->currentThread->space->getPageEntry(pageFaultPageNum);

      if (physicalPageNum != -1) { // in physical memory
        pageEntry->physicalPage = physicalPageNum;
        pageEntry->valid = TRUE;
        kernel->swapSpace->ReadAt(&kernel->machine->mainMemory[physicalPageNum*PageSize], PageSize, pageEntry->virtualPage*PageSize);
        kernel->EntryCache->set(pageFaultPageNum, pageEntry);

        //cout << "page fault num:" << pageFaultPageNum;
        //for (int i = 0; i < PageSize; i = i + 4) {
        //  int data = *(unsigned int *) &kernel->machine->mainMemory[physicalPageNum*PageSize+i];
        //  cout << i<<": "<<data << "\n";
        //}
      }
      else {
        // swap out
        TranslationEntry *LRUEntry = kernel->EntryCache->oldestNode()->value;  //kernel->EntryCache->setRandomSelectedNode(); // randomly select a node
        // fetch the physical page number of the evicted page
        int physicalPageNum = LRUEntry->physicalPage;
        //if (LRUEntry->dirty == TRUE) { // if the file is modifed.
          // copy evicted from memory to disk
        kernel->swapSpace->WriteAt(&kernel->machine->mainMemory[physicalPageNum*PageSize], PageSize, LRUEntry->virtualPage*PageSize); // write back
        LRUEntry->valid = FALSE;
        //}

        // swap in
        pageEntry->physicalPage = physicalPageNum;
        pageEntry->valid = TRUE;
        kernel->swapSpace->ReadAt(&kernel->machine->mainMemory[physicalPageNum*PageSize], PageSize, pageEntry->virtualPage*PageSize);
        TranslationEntry * removedNode = kernel->EntryCache->set(pageFaultPageNum, pageEntry);
        ASSERT(removedNode == LRUEntry);
      }
      //kernel->machine->WriteRegister(2, physicalPageNum);
      //int *registers;
      //registers = kernel->currentThread->getUserRegisters();
      //registers[BadVAddrReg] = physicalPageNum * PageSize;
      return;
    }
                         
      break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
