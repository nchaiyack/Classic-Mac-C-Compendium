


#include "MSUtil.h"

/* check current grafPort support for color drawing	*/
Boolean MSColorIndexesOK()	
{
	return ( MSColorQDExists() && MSColorGrafPortIsTrue());
}

/* Check for Color QuickDraw available at runtime	*/
Boolean MSColorQDExists()	
{
	SysEnvRec	theWorld;
	OSErr		theErr;

	theErr = SysEnvirons( 20, &theWorld );
	return theWorld.hasColorQD;
}

/* check current grafPort for color	*/
Boolean MSColorGrafPortIsTrue()			
{
	CGrafPtr	colorPortPtr;
	short		versionHold;

	GetPort( (GrafPtr *)&colorPortPtr );
	versionHold = colorPortPtr->portVersion;

	/* If the first two bits of portVersion are set, this is a color GrafPort	*/
	return (BitTst( (Ptr)&versionHold, 0 ) && BitTst( (Ptr)&versionHold, 1 ));
}
