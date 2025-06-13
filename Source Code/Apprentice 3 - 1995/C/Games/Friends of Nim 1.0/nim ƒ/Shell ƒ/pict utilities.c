#include "pict utilities.h"

PicHandle DrawThePicture(PicHandle thePict, short whichPict, short x, short y)
/* a standard routine for loading a picture (if necessary) and then drawing it */
{
	Rect			temp;
	
	if (thePict==0L)		/* get it if it doesn't exist */
		thePict=(PicHandle)GetPicture(whichPict);
	
	HLock((Handle)thePict);		/* lock it down for dereferencing to get picture bounds */
	temp.top=y;
	temp.left=x;
	temp.bottom=temp.top+(**thePict).picFrame.bottom-(**thePict).picFrame.top;
	temp.right=temp.left+(**thePict).picFrame.right-(**thePict).picFrame.left;
	DrawPicture(thePict, &temp);	/* draw picture at (x,y) */
	HUnlock((Handle)thePict);		/* unlock for better memory management */
	return thePict;
}

PicHandle ReleaseThePict(PicHandle thePict)
{
	if (thePict!=0L)	/* if exists, release it */
		ReleaseResource((Handle)thePict);
	return 0L;
}

Boolean DrawPictureCentered(PicHandle thePict, Rect boundsRect, Rect *destRect)
/* returns TRUE if picture was drawn successfully */
/* returns FALSE if picture was larger than boundsRect or PicHandle was NULL */
{
	Rect			temp;
	
	if (thePict==0L)
		return FALSE;
	
	HLock((Handle)thePict);
	temp.top=boundsRect.top;
	temp.left=boundsRect.left;
	temp.bottom=temp.top+(**thePict).picFrame.bottom-(**thePict).picFrame.top;
	temp.right=temp.left+(**thePict).picFrame.right-(**thePict).picFrame.left;
	HUnlock((Handle)thePict);
	if ((temp.bottom-temp.top>boundsRect.bottom-boundsRect.top) ||
		(temp.right-temp.left>boundsRect.right-boundsRect.left))
		return FALSE;
	
	OffsetRect(&temp, (boundsRect.right-boundsRect.left-(temp.right-temp.left))/2,
		(boundsRect.bottom-boundsRect.top-(temp.bottom-temp.top))/2);
	DrawPicture(thePict, &temp);
	*destRect=temp;
	
	return TRUE;
}
