#include "QDUtils.h"

void CenterRect(Rect *innerRect, Rect *outerRect) {
	short hDiff = ((outerRect->right - outerRect->left) - (innerRect->right - innerRect->left)) / 2;
	short vDiff = ((outerRect->bottom - outerRect->top) - (innerRect->bottom - innerRect->top)) / 2;
	innerRect->left = outerRect->left + hDiff;
	innerRect->right = outerRect->right - hDiff;
	innerRect->top = outerRect->top + vDiff;
	innerRect->bottom = outerRect->bottom - vDiff;
} // END CenterRect

void MoveRectTo(short h, short v, Rect *theRect) {
	register short width = theRect->right - theRect->left;
	register short height = theRect->bottom - theRect->top;
	theRect->left = h;
	theRect->top = v;
	theRect->right = theRect->left + width;
	theRect->bottom = theRect->top + height;
} // END OffsetRectTo

void MoveRgnTo(short hLoc, short vLoc, RgnHandle theRgn) {
	register short hDiff;
	register short vDiff;
	hDiff = hLoc - (**theRgn).rgnBBox.left;
	vDiff = vLoc - (**theRgn).rgnBBox.top;
 	OffsetRgn(theRgn, hDiff, vDiff);
} // END MoveRgnTo

void MoveRgnToRect(Rect *theRect, RgnHandle theRgn) {
	register short hDiff;
	register short vDiff;
	vDiff = theRect->top - (**theRgn).rgnBBox.top;
	hDiff  = theRect->left - (**theRgn).rgnBBox.left;
	OffsetRgn(theRgn, hDiff, vDiff);
} // END MoveRgnToRect

/*************************************************************************
**************************************************************************

This Code was written by

		Brad Anderson
in the Think C 6.0 environment
		
where ever you see
"
	asm {
		dc.b	"QDOffscreens Copyright 1993 S-FX"
	}
"
you are not aloud to erase this or comment this out.  If you do, the you are
liable for the max. penalty in law.  Just kidding.
Conditions:
	1)If you do use this source code then Please leave this note in, just to
	  acknowledge me.
	2)Please put your "About Box" that I did this.
	3)If you are any way grateful for me to do this, then please, send me one
	  copy of the finish product.
	4)No copy and pasting of this source code.  Leave it whole. You can adjust it
	  to work on your compiler and you can add your own error calls where necessary.
	5)If you alter the source and plan to redistribute the source with stuff that you
	  did then contact me before hand.

Include:
	MacHeaders
	MacTraps	//for CGrafPort Making & BitMap making
	
	Feel free to give this out to anyone in its unaltered state.
	My AOL Address is:
		KEA6820
	and CompuServe ID is:
		71170,3562

**************************************************************************
*************************************************************************/


/************************************************************************
 *	MakeBitMap
 *		Makes a BitMap and returns it in a GrafPtr.  This is a universal way of making
 *		a BitMap.  
 *
************************************************************************/

Boolean NewOffScreenBitMap(GrafPtr *newOffscreen, Rect *inBounds, int id)
{
	GrafPtr savePort;
	GrafPtr newPort;
	PicHandle	pict;
	
	if(id != 0)
		pict = GetPicture(id);		/* Get Pict	*/
	GetPort(&savePort);				/* Save old Port */

	newPort = (GrafPtr) NewPtr(sizeof(GrafPort));	/* makes space in RAM */
	if (MemError() != noErr)						/* if memory is low	*/
	{
		return false;
	}
	OpenPort(newPort);						/* Inits some vars	*/
	newPort->portRect = *inBounds;			/* set rect of port*/
	newPort->portBits.bounds = *inBounds;	/* set rect of BitMap*/
	RectRgn(newPort->clipRgn, inBounds);
	RectRgn(newPort->visRgn, inBounds);
	
	/* calc. the size of each horizontal line and move it up till its / by 4 */
	newPort->portBits.rowBytes = ((inBounds->right - inBounds->left + 15) >> 4) << 1;

	
	newPort->portBits.baseAddr = 			/* make new Ptr */
		NewPtr(newPort->portBits.rowBytes * (long) (inBounds->bottom - inBounds->top));
	if (MemError() != noErr)				/* if memory is low */
	{
		SetPort(savePort);
		ClosePort(newPort);      			/* dump the visRgn and clipRgn */
		DisposPtr((Ptr)newPort); 			/* dump the GrafPort */
		return false;            			/* tell caller we failed */
	}
	SetPort(newPort);			 			/* Set Port before drawing */
	if(id != 0)
		DrawPicture(pict, inBounds);		/* Draw */
	else
		EraseRect(inBounds);
	*newOffscreen = newPort;
	SetPort(savePort);						/* Restore Old Port */
	return true;
}


/*********** DestroyOffscreen ***********/
/* Destroys a BitMap */

void DestroyOffscreenBitMap(GrafPtr oldOffscreen)
{
	ClosePort(oldOffscreen);
	DisposPtr(oldOffscreen->portBits.baseAddr);
	DisposPtr((Ptr)oldOffscreen);
}