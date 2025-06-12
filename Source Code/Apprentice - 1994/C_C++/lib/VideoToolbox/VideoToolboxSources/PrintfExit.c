/* 
PrintfExit.c

PrintfExit(const char *format,...);
is normally equivalent to calling printf and exit.

Lots of VideoToolbox routines, when they find a really grievous error, print out
a message and exit. Having one routine that does both makes the code slightly
neater, since the if statement then doesn't need braces. Furthermore, both of
these functions, printf, and exit are liable to break in foreign environments,
e.g when running as a MEX resource under MATLAB. Now the problem is confined to
this file, where it can be handled by conditional compilation.

Since PrintfExit is called only when we're near death, it seems prudent to make
sure there's enough stack space before calling printf, which crashes if there's
less than about 4500 byts of stack. Note that StackGrow() moves memory.

I've replaced all calls to exit() in the entire VideoToolbox by calls to
PrintfExit().

StackGrow() is defined in VideoToolbox.h.

HISTORY:
2/20/93	dgp	Wrote it based on conversation with David Brainard.
7/9/93	dgp	Test MATLAB in #if instead of #ifdef.
9/12/93	dgp	Moved Required() to Require.c.
9/15/93	dgp	Added "const" to prototype.
*/
#include "VideoToolbox.h"
#include <stdarg.h>			/* for variable-number-of-argument macros */

int PrintfExit(const char *format,...)
{
	va_list args;
	int i;
	long value=0;
  
	#if MAC_C
		/*
		The main program may have changed the current device. Let's
		restore the main device before doing the printf.
		*/
		Gestalt(gestaltQuickdrawVersion,&value);
		if(value>=gestalt8BitQD)SetGDevice(GetMainDevice());
		
		if(StackSpace()<6000)StackGrow(6000-StackSpace());
	#endif
	#if !MATLAB
		#if MAC_C
			/* printf crashes if there's less than about 4500 bytes of stack space */
			if(StackSpace()<5000){
				SysBeep(20);
				exit(EXIT_FAILURE);
			}
		#endif
		va_start(args,format);
		i=vfprintf(stdout,format,args);
		va_end(args);
		exit(EXIT_FAILURE);
	#else
	{
		char s[256];
		
		va_start(args,format);
		i=vsprintf(s,format,args);
		va_end(args);
		mex_error(s);	// Ask MATLAB to report the error.
	}
	#endif
}

