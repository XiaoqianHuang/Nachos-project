/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include <string>
#include "synchdisk.h"
#include "synchconsole.h"

bool CheckAndDelete(int id) {
  string name = string(kernel->openFileTable->find(id)->second->getFullName());
  (*kernel->openNumForWR)[name] = (*kernel->openNumForWR)[name] - 1;
  // check delete list.
  for (int i = 0; i < kernel->pendingDeleteFiles->NumInList(); i++) {
    if (kernel->pendingDeleteFiles->getItem(i) == name) { // in pending delete list
      if ((*kernel->openNumForWR)[name] == 0) { // no file access anymore //after reducing counter of this
        kernel->fileSystem->Remove(kernel->openFileTable->find(id)->second->getFullName()); // remove file
        kernel->openFileTable->erase(id);
        kernel->pendingDeleteFiles->Remove(name);
        return TRUE;
      }
      break;
    }
  }
  return FALSE;
}

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysSRead(int buffer, int size) { // read content to the buffer
  char *content = "*********default content for SysRead() *********";
  if (size > 48 || size < 0) {
    cout << "invalid input\n";
      return 0;
  }

  for (int i = 0; i < size; i++) {
    kernel->machine->WriteMem(buffer, 1, (int)*(content + i));
    if (i == size - 1) {
      break;
    }
    buffer++;
  }
  return size;
}

int SysSWrite(int buffer, int size) { // write the content of buffer to the console
  int temp;
  if (size > 48 || size < 0) {
    cout << "invalid input\n";
    return 0;
  }

  cout << "Writing to console:";
  for (int i = 0; i < size; i++) {
    kernel->machine->ReadMem(buffer, 1, &temp);
    cout << (char)temp << flush;
    if (i == size - 1) {
      break;
    }
    buffer++;
  }
  cout << "\n";
  return size;
}

void RestoreFunction(int restore) { // restore the state of thread
  kernel->currentThread->RestoreUserState();
  kernel->currentThread->space->RestoreState(); // swap into machine
  kernel->machine->Run();
}

SpaceId SysFork(int func) { // thread fork
  Thread *newThread = new Thread("Forked Thread");
  // page switching
  AddrSpace *newSpace = new AddrSpace(*(kernel->currentThread->space)); // copy the memory of current thread
  newThread->space = newSpace;
  newThread->SaveUserState(); // save the state of current registers to new thread
  int *registers;
  registers = newThread->getUserRegisters();
  registers[PCReg] = func; // save the PC
  registers[NextPCReg] =  func + 4; 
  newThread->Fork((VoidFunctionPtr)RestoreFunction, (void*)0);
  cout << "New pid = " << newThread->pid << "\n";
  return (int)newThread->pid; 
}

void ForkFunction(Thread* t) {
  t->space->Execute();
}

SpaceId SysExec(char* exec_name) {
  Thread *newThread = new Thread("Exec Thread");
  newThread->space = new AddrSpace();
  int pid = -1;
  if (newThread->space->Load(exec_name) == TRUE) { // only exec the prog when the prog exist
    cout << "[" << exec_name << "]is opened \n";
    newThread->Fork((VoidFunctionPtr)ForkFunction, (void*)newThread);
    cout << "New pid = " << newThread->pid << "\n";
    pid = newThread->pid;
    kernel->currentThread->childrenTable->Append(newThread); // add to children table
    newThread->parent = kernel->currentThread; // set parent
  }
  else {
    cout << "Cannot open [" << exec_name << "]! \n";
  }

  return pid;
}

void SysExit() { // update TLB
  cout << "The thread exit\n";

  // wake up the blocked parent process
  if (kernel->currentThread->parent != NULL && kernel->currentThread->waitFlag == TRUE) { 
    bool canWake = TRUE;
    kernel->currentThread->waitFlag == FALSE;
    for (int i = 0; i < kernel->currentThread->parent->childrenTable->NumInList(); i++) {
      if (kernel->currentThread->parent->childrenTable->getItem(i)->waitFlag == TRUE) {
        canWake = FALSE;
        break;
      }
    }
    if (canWake == TRUE) {
      kernel->scheduler->ReadyToRun(kernel->currentThread->parent); // all the children is finished, can wake up the parent
    }
  }

  // clean the openfile list (delete each openfile, close each file)
  map<string, int>::iterator iter;
  for (iter = kernel->currentThread->space->openFileTable->begin(); iter != kernel->currentThread->space->openFileTable->end(); iter++) {
    if (CheckAndDelete(iter->second) == TRUE) {
      cout << "File "<< iter->first <<" Deleted.\n";
      kernel->removeFileFromTable(iter->second); // remove from system-wide table
    }
  }

  // update parent-children table
  if (kernel->currentThread->parent != NULL) {
    kernel->currentThread->parent->childrenTable->Remove(kernel->currentThread); // remove this from parent's children list
  }
  kernel->ProcessTable->erase(kernel->currentThread->pid);
 
  // terminate the process
  kernel->currentThread->Finish();
  // garbage collection done in destructor
}

int SysJoin(int childid) {
  bool isChild = FALSE;
  Thread* childThread;
  for (int i = 0; i < kernel->currentThread->childrenTable->NumInList(); i++) { // is in current children list
    if (kernel->currentThread->childrenTable->getItem(i)->pid == childid) {
      childThread = kernel->currentThread->childrenTable->getItem(i);
      isChild = TRUE;
      break;
    }
  }

  if (isChild == FALSE) {
    DEBUG(dbgSys, "Join: no such child right now.");
    return -1;
  }

  ASSERT(childThread->parent == kernel->currentThread);
  if (childThread->waitFlag == TRUE) {
    cout << "The child " << childid << " process has been joined!\n";
    return - 1;
  }
  childThread->waitFlag = TRUE; // indicating the parent is blocked and waiting to be awaked.
  IntStatus oldlevel =  kernel->interrupt->SetLevel(IntOff);
  kernel->currentThread->Sleep(FALSE); // block the parent
  kernel->interrupt->SetLevel(oldlevel);

  return 1;
}

int SysCreate(char *name, int protection) {
  // privilege of current dir checked in Create()
  if (kernel->fileSystem->Create(name, 0, kernel->currentThread->space->currentDirSector, protection) == FALSE) {
    cout << "Create Fail.\n";
    return -1;
  }
  // success
  cout << "Create Success: " << name << "\n";
  return 1;
}

int SysRemove(char *name) {
  char* fullName = kernel->fileSystem->getFullName(name, kernel->currentThread->space->currentDirSector);
  if (fullName == NULL) {
    return -1;
  }

  // start check privilege //
  string path = string(fullName);
  char* dirPath = kernel->fileSystem->getParentDirectory(fullName);
  int protectionFileBit = kernel->fileSystem->getProtectionBit(fullName, 1);
  int protectionDirBit = kernel->fileSystem->getProtectionBit(dirPath, 1);

  if (kernel->fileSystem->canRead(protectionFileBit) == FALSE
    || kernel->fileSystem->canRead(protectionDirBit) == FALSE) { //check file and dir protection bit
    cout << "Privilege Denied: file - " << protectionFileBit << "dir - " << protectionDirBit << "\n";
    return -1;
  }
  // end check privilege //

  map<int, OpenFile*>::iterator iter;
  bool isOpened = FALSE;
  for (iter = kernel->openFileTable->begin(); iter != kernel->openFileTable->end(); iter++) {
    if (string(iter->second->getFullName()) == path) {
      isOpened = TRUE;
      break;
    }
  }
  if (isOpened == FALSE) { // no thread access to it, delete directly
    kernel->fileSystem->Remove(fullName); 
  }

  kernel->pendingDeleteFiles->Append(path); // add to pending delete list, will delete when no other file access to
  return 1;
}

OpenFileId SysOpen(char *name, int mode) {
  char* fullName = kernel->fileSystem->getFullName(name, kernel->currentThread->space->currentDirSector);
  if (fullName == NULL) {
    return -1;
  }

  // start check privilege //
  string path = string(fullName);
  char* dirPath = kernel->fileSystem->getParentDirectory(fullName);
  int protectionFileBit = kernel->fileSystem->getProtectionBit(fullName, 1);
  int protectionDirBit = kernel->fileSystem->getProtectionBit(dirPath, 1);

  if (kernel->fileSystem->canRead(protectionFileBit) == FALSE
    || kernel->fileSystem->canRead(protectionDirBit) == FALSE) { //check file and dir protection bit
    cout << "Privilege Denied: file - " << protectionFileBit << "dir - " << protectionDirBit << "\n";
    return -1;
  }
  // end check privilege //

  if (kernel->pendingDeleteFiles->IsInList(name) == TRUE) { // if the file is deleted
    cout << "Cannot open. The file is deleted.\n";
    return -1; 
  }
  
  OpenFile *file = kernel->fileSystem->Open(name);

  if (file == NULL) {
    cout << "Cannot find the file.\n";
    return -1;
  }

  if (kernel->currentThread->space->openFileTable->find(file->getFullName()) != kernel->currentThread->space->openFileTable->end()) { // existed in thread list
    cout << "The file has been opened!\n";
    return -1;
  }

  // start opening
  kernel->insertFileToTable(file); // add to system-wide table
  (*kernel->currentThread->space->openFileTable)[file->getFullName()] = file->getOpenfileId(); // add to thread table
  string filename = string(file->getFullName()); //update WR tables

  if (kernel->locks->WRLocks->find(filename) == kernel->locks->WRLocks->end()) { // no such lock
    (*kernel->locks->WRLocks)[filename] = new Lock(file->getFullName());// create rw lock
    (*kernel->readCount)[filename] = 0;// create rc
    (*kernel->locks->RCLocks)[filename] = new Lock(file->getFullName());// create rc lock
  }

  if ((*kernel->openNumForWR).find(filename) != (*kernel->openNumForWR).end()) { // file exist in map
    (*kernel->openNumForWR)[filename] == (*kernel->openNumForWR)[filename] + 1;
  }
  else {
    (*kernel->openNumForWR)[filename] = 1;
  }

  // set mode
  file->setMode(mode);

  return file->getOpenfileId(); // return open file id
}

int SysWrite(int buffer, int size, OpenFileId id) {
  if (id == CONSOLEOUTPUT) { // console out
    for (int i = 0; i < size; i++) {
      int temp;
      kernel->machine->ReadMem(buffer, 1, &temp);
      kernel->synchConsoleOut->PutChar((char)temp);
      if (i == size - 1) {
        break;
      }
      buffer++;
    }
    return size;
  }

  if (kernel->currentThread->space->isExisted(id)) { // cannot find in thread table
    cout << "Fail, no such id.\n";
    return -1;
  }

  if (kernel->openFileTable->find(id)->second->getMode() != 2 || kernel->openFileTable->find(id)->second->getMode() == 3) { // 2: RW, 3: APPEND
    cout << "Fail, mode: " << kernel->openFileTable->find(id)->second->getMode();
    return -1;
  }

  // start check privilege //
  char* fullName = kernel->openFileTable->find(id)->second->getFullName();
  string path = string(fullName);
  char* dirPath = kernel->fileSystem->getParentDirectory(fullName);
  int protectionFileBit = kernel->openFileTable->find(id)->second->getProtectionBit();
  int protectionDirBit = kernel->fileSystem->getProtectionBit(dirPath, 1);

  if (kernel->fileSystem->canRead(protectionFileBit) == FALSE
    || kernel->fileSystem->canRead(protectionDirBit) == FALSE) { //check file and dir protection bit
    cout << "Privilege Denied: file - " << protectionFileBit << "dir - " << protectionDirBit << "\n";
    return -1;
  }
  // end check privilege //

  if (kernel->openFileTable->find(id)->second->getMode() == 3) { // append mode
    kernel->openFileTable->find(id)->second->Seek(kernel->openFileTable->find(id)->second->Length() + 1);
  }

  char *content = new char[100];
  int temp;
  int i;
  for (i = 0; i < size; i++) {
    kernel->machine->ReadMem(buffer, 1, &temp);
    content[i] = (char)temp ;
    if (i == size - 1) {
      break;
    }
    buffer++;
  }
  i++;
  content[i] = '\0';

  size = kernel->openFileTable->find(id)->second->Write(content, size);
  return size;
}


int SysRead(int buffer, int size, OpenFileId id) {
  if (id == CONSOLEINPUT) { // console in
    char str;
    for (int i = 0; i < size; i++) {
      str = kernel->synchConsoleIn->GetChar(); // get the char 
      kernel->machine->WriteMem(buffer, 1, (int)str); // write to buffer
      if (i == size - 1) {
        break;
      }
      buffer++;
    }
    return size;
  }

  if (kernel->currentThread->space->isExisted(id)) { // cannot find in thread table
    cout << "Fail, no such id.\n";
    return -1;
  }

  if (kernel->openFileTable->find(id)->second->getMode() != 2 || kernel->openFileTable->find(id)->second->getMode() == 1) { // 2: RW, 1: RO
    cout << "Fail, mode: " << kernel->openFileTable->find(id)->second->getMode();
    return -1;
  }

  // start check privilege //
  char* fullName = kernel->openFileTable->find(id)->second->getFullName();
  string path = string(fullName);
  char* dirPath = kernel->fileSystem->getParentDirectory(fullName);
  int protectionFileBit = kernel->openFileTable->find(id)->second->getProtectionBit();
  int protectionDirBit = kernel->fileSystem->getProtectionBit(dirPath, 1);

  if (kernel->fileSystem->canRead(protectionFileBit) == FALSE
    || kernel->fileSystem->canRead(protectionDirBit) == FALSE) { //check file and dir protection bit
    cout << "Privilege Denied: file - " << protectionFileBit << "dir - " << protectionDirBit << "\n";
    return -1;
  }
  // end check privilege //

  char *content = new char[100];

  size = kernel->openFileTable->find(id)->second->Read(content, size);

  //read content
  for (int i = 0; i < size; i++) {
    kernel->machine->WriteMem(buffer, 1, (int)*(content + i)); // write to buffer
    if (i == size - 1) {
      break;
    }
    buffer++;
  }

  return size;
}

int SysSeek(int position, OpenFileId id) {
  if (kernel->currentThread->space->isExisted(id)) { // cannot find in thread table
    cout << "Fail, no such id.\n";
    return -1;
  }

  if (kernel->openFileTable->find(id)->second->getMode() != 2 || kernel->openFileTable->find(id)->second->getMode() == 1) { // 2: RW, 1: RO
    cout << "Fail, mode: " << kernel->openFileTable->find(id)->second->getMode();
    return -1;
  }

  kernel->openFileTable->find(id)->second->Seek(position);
  return 1;
}

int SysClose(OpenFileId id) {
  if (kernel->currentThread->space->isExisted(id)) { // cannot find
    cout << "Fail, no such id.\n";
    return -1;
  }
  
  if (CheckAndDelete(id) == TRUE) { // check if the file is pending deleted
    cout << "File "<< id <<" Deleted.\n";
    kernel->removeFileFromTable(id); // remove from system-wide table
  }
  kernel->currentThread->space->removeById(id);// remove from per-process table

  return id;
}

char* getString(char *str, int addr) {
  int output = 0;
  int count = 0;
  kernel->machine->ReadMem(addr, 1, &output);
  str[0] = (char)output;
  while ((char)output != '\0') {
    count++;
    kernel->machine->ReadMem(addr + count, 1, &output);
    str[count] = (char)output;
  }
  return str;
}


int checkCMD(char *str) {
  string s = string(str);
  string cmd[10];
  size_t pos = s.find(' ');
  size_t lastPos = 0;
  int i = 0;
  while(pos != string::npos) {
    if (s.substr(lastPos, pos) != " ") {
      cmd[i] = s.substr(lastPos, pos);
      i++;
    }
    lastPos = pos + 1;
    //s = s.substr(pos+1);
    pos = s.find(' ');
  }

  cmd[i] = s;

  if (cmd[0] == "ls") {
    kernel->fileSystem->List();
  }
  else if (cmd[0] == "cd") {
    if (i < 1) {
      cout << "Too few argument.\n";
      return 0;
    }
    char* filename = new char[cmd[1].length() + 1]; // arg1
    strcpy(filename, cmd[1].c_str());
    int sector = kernel->fileSystem->ChangeDir(&filename, kernel->currentThread->space->currentDirSector); // fullpath will be output at filename
    if (sector == -1) {
      return 0;
    }
    else { // update thread info
      kernel->currentThread->space->currentDirSector = sector;
      kernel->currentThread->space->currentDir = kernel->fileSystem->getFullName(sector);
    }
  }
  else if (cmd[0] == "pwd") {
    cout << kernel->currentThread->space->currentDir << "\n";
  }
  else if (cmd[0] == "mkdir") {
    if (i < 1) {
      cout << "Too few argument.\n";
      return 0;
    }
    char* filename = new char[cmd[1].length() + 1]; // arg1
    strcpy(filename, cmd[1].c_str());
    int sector = kernel->fileSystem->MakeDir(&filename, kernel->currentThread->space->currentDirSector); // fullpath will be output at filename
    if (sector == -1) {
      return 0;
    }
    cout << filename << " is created\n";
  }
  else if (cmd[0] == "cp") {

  }
  else if (cmd[0] == "mv") {

  }
  else if (cmd[0] == "rm") {
    if (i < 1) {
      cout << "Too few argument.\n";
      return 0;
    }
    char* filename = new char[cmd[1].length() + 1]; // arg1
    strcpy(filename, cmd[1].c_str());
    SysRemove(filename);
  }
  else if (cmd[0] == "rmdir") {
    if (i < 1) {
      cout << "Too few argument.\n";
      return 0;
    }
    char* filename = new char[cmd[1].length() + 1]; // arg1
    char* dirpath = kernel->fileSystem->getParentDirectory(dirpath);
    strcpy(filename, cmd[1].c_str());
    SysRemove(filename);
  }
  else if (cmd[0] == "chmod") {
    //arg 1 = protection bit(int)
    //arg 2 = program name
    if (i < 2) {
      cout << "Too few argument.\n";
      return 0;
    }
    char* filename = new char[cmd[2].length() + 1]; // arg2
    strcpy(filename, cmd[2].c_str());
    int protectionBit = atoi(cmd[1].c_str());    //atoi(cmd[1].c_str())
    if (kernel->fileSystem->setProtectionBit(filename, protectionBit) == FALSE) {
      return 0;
    }
    cout << "Success, Protection Bit: " << protectionBit << "\n";
  }
  else {
    return -1;
  }
  return 0;
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
