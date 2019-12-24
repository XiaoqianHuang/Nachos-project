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
  Write(content, 8, fd);
  Seek(0, fd);
  Read(result, 8, fd);
  Close(fd);


  fd = Open(name, 3); // append
  Write(content, 8, fd);
  Read(result, 8, fd);
  Close(fd);

  fd = Open(name, 1); // ro
  Write(content, 8, fd);
  Seek(0, fd);
  Read(result, 16, fd);
  Close(fd);
}