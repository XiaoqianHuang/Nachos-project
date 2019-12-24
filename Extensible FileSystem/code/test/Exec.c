#include "syscall.h"

int
main()
{
  //printf("Starting Exec Syscall\n");
  char name[6] = "Write";
  Exec(name);
}
