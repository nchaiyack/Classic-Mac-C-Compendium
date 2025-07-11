//-- Error.c

// Error management routines.

#include <unix.h>
#include <stdio.h>
#include "error.h"
#include "res.h"

jmp_buf callStack[MAXJUMP];
short callStackPtr;

//-- Throw

// This 'throws' the error message back to the 'catch' routine.

void Throw(i)
int i;
{
	if (callStackPtr > 0) {
		callStackPtr--;
		longjmp(callStack[callStackPtr],i);
	}
}



//-- PostError

// This posts the error message specified to the screen.

void PostError(i)
int i;
{
	char buffer[255];

	GetIndString(buffer,ERRORSTRS,i);
	ParamText(buffer,NULL,NULL,NULL);
	Alert(ERRORALRT,NULL);
}
