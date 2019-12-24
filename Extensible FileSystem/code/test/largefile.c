#pragma once

#include "syscall.h"

int
main()
{
  int fd;
  char largeStr[50*1024];
  char name[6] = "large";
  int i;

  for (i = 0; i < 50 * 1024; i++) {
    largeStr[i] = '1';
  }

  //Create(name, 7);
  //fd = Open(name, 2); //wr
  //Write(largeStr, 50*1024, fd);
  //Close(fd);
  ////Remove("large");
  //Halt();
  /* not reached */
}
