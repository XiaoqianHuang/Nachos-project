#include "syscall.h"

int
main()
{
  //printf("Starting Read Syscall and Write Syscall\n");
  char b[48];
  SysRead(b, 48);
  SysWrite(b, 48);

  //Halt();
}
