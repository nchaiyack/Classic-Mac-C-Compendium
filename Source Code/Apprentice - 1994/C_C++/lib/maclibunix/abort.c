/* Abort emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#include "macdefs.h"

abort()
{
	Debugger();
	exit(2);
}
