/***********************************************************
   This is the code for ShowINIT as printed in Dave Mark's
   great book 'Mac Programming Primer' along with some
   changes to use 'cicn' resources and some other trivialities.

   Modified by Tom Bridgwater 2/16/1994 (see //TB comments)
   (please give me credit if you do anything significant 
    with this -- Thanks  tbridgwa@cymbal.calpoly.edu)
   [first used in my QuietStart 1.2 freeware extension]
       1. Always uses 'cicn' for color and B/W
       2. Displacement parameters for icon positioning
       3. Debugging support
          (normal application procedure if _TEST defined)

************************************************************
		typical call :
************************************************************

Handle	procH;

	if((procH = GetResource('PROC', PROC_ID)) != 0L)
	{
		HLock(procH);
//TB: displacement parameters:
		CallPascal(ICON_ID, -1, 0, 0, *procH);
		HUnlock(procH);
	}

***********************************************************/

// #include <color.h>

typedef struct QuickDraw
{	/* struct to hold QuickDraw Vriables */
	char	private[76];
	long	randSeed;
	BitMap	screenBits;
	Cursor	arrow;
	Pattern	dkGray;
	Pattern	ltGray;
	Pattern	gray;
	Pattern	black;
	Pattern	white;
	GrafPtr	thePort;
} QuickDraw;

extern short	myH		: 0x92c;	/* CurApName + 28 */
extern short	myCheck	: 0x92E;	/* CurApName + 30 */

#define firstX			8
	/* left margin - offset to first icon */

#define bottomEdge		8
	/* this far from bottom of screen */

//TB: iconWidth is taken from 'cicn', not #defined

#define defaultMoveX	40
	/* default amount to move icons */

#define checksumConst	0x1021
	/* constant used for computing checksum */

#define minColorDepth	4
	/* min. bits/pixel for drawing color icon */

//TB: access to 'cicn' data
#define dataIndex		6
	/* long index of byte offset to mask & B/W data in cicn */
	
//TB: access to 'cicn' data
#define maskOffset		10
	/* byte offset to mask data from beginning of data [dataIndex] */

#define iconRowBytes	4
	/* 32/8 bits */

//TB: access to 'cicn' data
#define cicnRowBytesOffset	0x1b
	/* index of short in resource */

#define hasCQDBit		6
	/* bit in ROM85 cleared if CQD available */

//TB: procedure name not main() while debugging
//TB: displacement parameters
#ifdef _TEST
pascal void showInit(iconID, moveX, dispX, dispY)
#else
pascal void main(iconID, moveX, dispX, dispY)
#endif _TEST
short	iconID, moveX, dispX, dispY;
{
Handle		theIconHdl;
short		dh, colorFlag, theDepth;
GDHandle	theMainDevice;
Rect		srcRect, destRect;
BitMap		myBitMap;
GrafPort	myPort;
QuickDraw	qdGlobals;
Ptr			localA5, savedA5;

//TB: don't setup A5 while debugging
#ifndef _TEST
	asm
	{
		move.l	A5, savedA5
		lea		localA5,A5
		move.l	A5, CurrentA5;
	}
#endif _TEST

	InitGraf(&qdGlobals.thePort);
	OpenPort(&myPort);
	colorFlag = 0;
	
	if (!(BitTst(&ROM85, 7-hasCQDBit)))
	{
		theMainDevice = MainDevice;
		theDepth = (*(*theMainDevice)->gdPMap)->pixelSize;
		if (theDepth >= minColorDepth)
			if((theIconHdl = (Handle)GetCIcon(iconID)) != 0L)
				colorFlag = 1;
	}
	
	if (!colorFlag)
//TB: load 'cicn' instead of 'ICN#' for B/W
		if (!(theIconHdl = GetResource('cicn', iconID)))
		{
			SysBeep(3);
			goto out;
		}
	dh = (myH << 1) ^ checksumConst;
	myH = ((dh == myCheck) ? (myH):(firstX));
	
//TB: include displacement in destination
	destRect.bottom = myPort.portRect.bottom - bottomEdge - dispY;
	destRect.left = myPort.portRect.left + myH + dispX;
//TB: account for variable size 'cicn' in destination
	destRect.top = destRect.bottom -
					(	(*(CIconHandle)theIconHdl)->iconPMap.bounds.bottom
					 -	(*(CIconHandle)theIconHdl)->iconPMap.bounds.top );
	destRect.right = destRect.left + 
					(	(*(CIconHandle)theIconHdl)->iconPMap.bounds.right
					 -	(*(CIconHandle)theIconHdl)->iconPMap.bounds.left);

	if (colorFlag)
	{
		PlotCIcon(&destRect, (CIconHandle)theIconHdl);
		DisposCIcon((CIconHandle)theIconHdl);
	} else
	{
		HLock(theIconHdl);
		srcRect.top = srcRect.left = 0;
//TB: account for variable size 'cicn' in source
		srcRect.bottom = destRect.bottom - destRect.top;
		srcRect.right = destRect.right - destRect.left;
//TB: account for variable size 'cicn' in BitMap parameters
		myBitMap.rowBytes = ((short*)(*theIconHdl))[cicnRowBytesOffset];
		myBitMap.bounds = srcRect;
//TB: use 'cicn' data to find mask
		myBitMap.baseAddr = *theIconHdl + ((long*)*theIconHdl)[dataIndex] + maskOffset;
		/* punch hole with mask */
		CopyBits(&myBitMap, &myPort.portBits, &srcRect, &destRect, srcBic, 0L);
//TB: use 'cicn' data to find B/W icon
		/* icon immideatly follows mask, so add size of mask to get icon baseAddr */
		myBitMap.baseAddr += myBitMap.rowBytes * srcRect.bottom;
		/* now draw the icon */
		CopyBits (&myBitMap, &myPort.portBits, &srcRect, &destRect, srcOr, 0L);
		HUnlock(theIconHdl);
		ReleaseResource(theIconHdl);
	}
	myH += ((moveX == -1) ? (defaultMoveX) : (moveX));
	myCheck = (myH << 1) ^ checksumConst;
	
out:
	ClosePort(&myPort);

//TB: don't restore A5 while debugging
#ifndef _TEST
	asm
	{
		move.l	savedA5, A5
		move.l	A5, CurrentA5
	}
#endif _TEST
}

		
