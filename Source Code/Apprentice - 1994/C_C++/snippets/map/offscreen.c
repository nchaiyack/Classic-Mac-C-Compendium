#include <Events.h>
#include <Quickdraw.h>
#include <Windows.h>
#include <Memory.h>
#include "trigtab.h"

#define kIsVisible true
#define kNoGoAway false
#define kNoWindowStorage 0L
#define kFrontWindow ((WindowPtr) -1L)

Boolean CreateOffscreenBitMap(GrafPtr *newOffscreen, Rect *inBounds)
{
	GrafPtr savePort;
	GrafPtr newPort;

	GetPort(&savePort); /* need this to restore thePort after OpenPort */

	newPort = (GrafPtr)NewPtr(sizeof(GrafPort));/* allocate the grafPort */
	if(MemError() != noErr)
		return false;									
/* failure to allocate off-screen port */

	/*	the call to OpenPort does the following:

		allocates space for visRgn (set to screenBits.bounds)
		and clipRgn (set wide open)
		sets portBits to screenBits
		sets portRect to screenBits.bounds
		etc. (see Inside Macintosh Volume 1 pages 163-164)
		side effect: does a SetPort (&offScreen)
	*/

	OpenPort(newPort);

	/* make bitmap the size of the bounds that caller supplied */
	newPort->portRect = *inBounds;
	newPort->portBits.bounds = *inBounds;
	RectRgn(newPort->clipRgn, inBounds);
	RectRgn(newPort->visRgn, inBounds);
/* rowBytes is size of row, must be rounded up to an even number of bytes */

	newPort->portBits.rowBytes =
		((inBounds->right - inBounds->left + 15) >> 4) << 1;

	/* number of bytes in BitMap is rowBytes * number of rows */
	/* see notes at end of example about using NewPtr instead of NewHandle */
	newPort->portBits.baseAddr =
		NewPtr(newPort->portBits.rowBytes * (long)(inBounds->bottom - inBounds->top));

	if(MemError() != noErr)
		{
		SetPort(savePort);
		ClosePort(newPort);		/* dump the visRgn and clipRgn */
		DisposPtr((Ptr)newPort);	/* dump the GrafPort */
		return false;	/* tell caller we failed */
		}

	/* since the bits are just memory, let's clear them before we start */
	EraseRect(inBounds);/* OpenPort did a SetPort(newPort) so we are OK*/
	*newOffscreen = newPort;
	SetPort(savePort);
	return true;					/* success */
}

/*
	DestroyOffscreenBitMap - get rid of an off-screen bitmap created
	by CreateOffscreenBitMap
*/
void DestroyOffscreenBitMap(GrafPtr oldOffscreen)
{
	ClosePort(oldOffscreen);		/* dump the visRgn and clipRgn */
	DisposPtr(oldOffscreen->portBits.baseAddr);	/* dump the bits */
	DisposPtr((Ptr)oldOffscreen);		/* dump the port */
}

