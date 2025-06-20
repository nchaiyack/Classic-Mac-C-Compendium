/* =========================================================
   ======================= GWorld.c ========================
   =========================================================
   
   � 1994 by Thomas Reed
   
   GWorld.c contains 4 routines:
   
     UseOffWorld(GWorldPtr)
     
       Sets up a pre-allocated GWorld for drawing.  You must call
       this routine before doing ANY drawing in a GWorld (not
       counting CopyBits calls).  This will set the GWorld as
       the current port.
       
     DoneWithOffWorld(GWorldPtr)
     
       Call this after you've done all the drawing you want in
       the GWorld.  This will restore the port that was set
       before calling UseOffWorld.
       
     CreateOffscreenBitMap(GrafPtr *, Rect *)
     
       Call this to allocate space for an offscreen GrafPort,
       and initialize the fields.  After calling this, you can
       go ahead and start drawing to the buffer.
       
     DestroyOffscreenBitMap(GrafPtr)
     
       De-allocates the offscreen GrafPort set up with
       CreateOffscreenBitMap.  This releases all space taken
       up by the GrafPort.
       
   Notice that there are no routines for setting up and disposing
   a GWorld.  This is because it's gotten much simpler between the
   times of GrafPorts and the current GWorlds.  To allocate a GWorld,
   just call the Toolbox routine:
   
     myErr = NewGWorld(&offscreenGWorld,pixelDepth,&boundsRect,cTable,aGDevice,flags);
     (further described in IM VI chap. 21 pg 12)
   
   To dispose of the GWorld, call:
   
     DisposeGWorld(offscreenGWorld);
     (further described in IM VI chap. 21 pg 19)
   
   Make sure to include the GWorld.h header in your source files that
   call the routines contained herein.
   
   If you use this source, just give me some credit in your About box.
   I ask nothing more...
*/

#include "GWorld.h"

#define NIL		0L

/* for BitMap calls */
#define kIsVisible TRUE
#define kNoGoAway FALSE
#define kNoWindowStorage 0L
#define kFrontWindow ((WindowPtr) -1L)


GDHandle		gOldDevice;
CGrafPtr		gOldPort;


void UseOffWorld(GWorldPtr offWorlder)
{
  GetGWorld(&gOldPort, &gOldDevice);
  LockPixels(GetGWorldPixMap(offWorlder));
  SetGWorld(offWorlder, NIL);
}

void DoneWithOffWorld(GWorldPtr offWorlder)
{
  UnlockPixels(GetGWorldPixMap(offWorlder));
  SetGWorld(gOldPort, gOldDevice);
}

/* === BitMap routines === */

Boolean CreateOffscreenBitMap(GrafPtr *newOffscreen, Rect *inBounds)
{
  GrafPtr savePort;
  GrafPtr newPort;

  GetPort(&savePort); /* need this to restore thePort after OpenPort */

  newPort = (GrafPtr)NewPtr(sizeof(GrafPort));/* allocate the grafPort */
  if(MemError() != noErr)
    return FALSE;											
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

  newPort->portBits.rowBytes = ((inBounds->right - inBounds->left + 15) >> 4) << 1;

  /* number of bytes in BitMap is rowBytes * number of rows */
  /* see notes at end of example about using NewPtr instead of NewHandle */
  newPort->portBits.baseAddr = 
    NewPtr(newPort->portBits.rowBytes * (long)(inBounds->bottom - inBounds->top));

  if(MemError() != noErr)
  {
    SetPort(savePort);
    ClosePort(newPort);			/* dump the visRgn and clipRgn */
    DisposPtr((Ptr)newPort);	/* dump the GrafPort */
    return FALSE;	/* tell caller we failed */
  }

  /* since the bits are just memory, let's clear them before we start */
  EraseRect(inBounds);/* OpenPort did a SetPort(newPort) so we are OK*/
  *newOffscreen = newPort;
  SetPort(savePort);
  return TRUE;						/* success */
}

/*
	DestroyOffscreenBitMap - get rid of an off-screen bitmap created
	by CreateOffscreenBitMap
*/
void DestroyOffscreenBitMap(GrafPtr oldOffscreen)
{
  ClosePort(oldOffscreen);			/* dump the visRgn and clipRgn */
  DisposPtr(oldOffscreen->portBits.baseAddr);	/* dump the bits */
  DisposPtr((Ptr)oldOffscreen);			/* dump the port */
}