#include	"UtilsSys7.h"


#if defined(powerc) || defined (__powerc)

void MyZeroBuffer(long *p, long s)
{
for ( ; s-- >= 0; *p++ = 0L);
}


#endif

/* NewHandleGeneral: makes a new handle in main or temporary memory, arguably
in the more suitable heap zone */

Handle NewHandleGeneral(Size blockSize)
{
register Handle	myH;
OSErr	err;

if (TempFreeMem() > FreeMem()) {
	if ((myH = TempNewHandle(blockSize, &err)) == nil) {
		ReserveMem(blockSize);
		myH = NewHandle(blockSize);
		}
	}
else {
	ReserveMem(blockSize);
	if ((myH = NewHandle(blockSize)) == nil)
		myH = TempNewHandle(blockSize, &err);
	}
return(myH);
}

