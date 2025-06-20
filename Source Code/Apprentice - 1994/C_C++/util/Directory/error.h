//-- error.h

// Error management.


#include <setjmp.h>

//-- Constants.

#define	MAXJUMP				10						/* Max level of return that can be done */

//-- Error constants.

#define	INITOUTMEM			1						/* Out of memory on init */
#define OUTWINDOWS			2						/* No more windows to open */
#define OUTMEM				3						/* Out of memory */

//-- Globals.

extern jmp_buf callStack[MAXJUMP];
extern short callStackPtr;

//-- Macros and functions.

extern void Throw(int i);							/* Call this with error message */
extern void PostError(int i);						/* Post error message */
#define Catch()			((callStackPtr < MAXJUMP) ? setjmp(callStack[callStackPtr++]) : 0)
#define Uncatch()		((callStackPtr > 0) ? --callStackPtr : 0)
