#include "syscall.h"

typedef void(*VoidNoArgFunctionPtr)();

void
ForkFunction() {
  
}

int
main()
{
  int i;
  for (i = 0; i < 10; i++) {
    if (i == 5) {
      SysFork((VoidNoArgFunctionPtr)ForkFunction);
    }
  }

  //Halt();
}
