/* NoTrace - Routines which must be compiled with -trace off                  */

#ifndef __NOTRACE__
#define __NOTRACE__


		// ¥ MacApp
#ifndef __UMemory__
#include "UMemory.h"
#endif


const int kRsrvSize = 40000;

HandleListHandle gMyDocList;
Boolean gPurgedHist, gMemIsLow;
Handle gMemReserve, gMemReserve2;


pascal long GrowZone (long needed);

pascal void InitNoTrace (void);

#endif
