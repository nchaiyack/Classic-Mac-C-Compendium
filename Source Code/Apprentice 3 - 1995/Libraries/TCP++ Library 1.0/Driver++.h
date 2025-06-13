
#ifndef __DRIVER__
#define __DRIVER__

#ifndef __DEVICES__
#include <Devices.h>
#endif

class DriverRef {
public:
	short	fRef;
			DriverRef()	{	OSErr theErr;
							if ((theErr = OpenDriver("\p.ipp", &fRef)) != noErr)
								fRef = 0;
						};
						
	short	GetRef()	{ return fRef; }
} DriverRef;

static struct DriverRef gRef;

#endif
