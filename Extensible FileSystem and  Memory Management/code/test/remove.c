#include "syscall.h"

int
main()
{
  int fd;
  char name[9] = "testOpen";
  char content[9] = "testOpen";
  char result[9];
  int i;

  fd = Open(name, 2); // wr
  Remove(name);
  Close(fd);
}