#include <MacHeaders>
#include "constants.h"
#include "globals.h"

/*********************
** SetupOffscreen()
**
** initializes the offscreen bitmap buffer.
**********************/
void SetupOffscreen(void)
{
	GrafPtr	savedPort;
	
	GetPort( &savedPort);
	
	gOffBM.rowBytes = 26;	// 208/8
	SetRect( &gOffBM.bounds, 0,0, 208,208);
	gOffBM.baseAddr = NewPtrClear( 26* 208);
	if (gOffBM.baseAddr == NULL)
	{
		// error: we can't get offscreen bitmap
		Error( kBitmapAllocErr, 0, 0, 0, stopIcon);
		Cleanup();
	}
	// else I got the bitmap allocated
	OpenPort( &gOffGP);
	SetPortBits( &gOffBM);
	gOffGP.portRect = gOffBM.bounds;

	SetPort(savedPort);
} // SetupOffscreen()


/**********************
** CleanupOffscreen()
**
** disposes of everything we needed for
** the offscreen buffer.
***********************/
void CleanupOffscreen(void)
{
	if (gOffGP.portBits.baseAddr != NULL)
		DisposPtr( gOffGP.portBits.baseAddr);
	
	ExitToShell();
} // CleanupOffscreen()


