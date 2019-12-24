#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = CONSOLEINPUT;
    OpenFileId output = CONSOLEOUTPUT;
    char prompt[2], ch, buffer[60], prog[60];
    int i, j, z, lastIndex;

    prompt[0] = '>';
    prompt[1] = '>';

    while(1)
    {
	Write(prompt, 2, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

  lastIndex = 0;
	if( i > 0 ) {
    for (j = 0; j < i; j++) {
      if (buffer[j] == ';') { // seperate the command
        
        for ( z = 0; z < j - lastIndex; z++) {
          prog[z] = buffer[z + lastIndex];
        }
        prog[z] = '\0';
        newProc = Exec(prog);
        Join(newProc);
        lastIndex = j + 1;
      }
    }

    for (z = 0; z < i; z++) {
      prog[z] = buffer[z + lastIndex];
    }
    prog[z] = '\0';
    newProc = Exec(prog);
    Join(newProc);
	}
    }
}

