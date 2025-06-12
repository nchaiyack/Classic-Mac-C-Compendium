/*
 * begin mods by aw0g for use softkiss
 */
#define main ShowINIT
#include "CShowINIT_PROC.h"
/*
 * end mods by aw0g for use in softkiss
 */

/*-------------------------------------------------------------------------
  Filename: CShowINIT_PROC.c
  Color ShowINIT, for use with LightspeedC
  This translation by Ken McLeod (thecloud@dhw68k.cts.com)
  Version of: Thursday, April 6, 1989 3:30:00 PM

  INIT notification routine
  by Paul Mercer, Darin Adler, and Paul Snively from an idea by Steve Capps
  Version of: Friday, July 15, 1988 12:08:09 AM	(1.1B1)
    -revved back to previous calling interface.
    -you only need to call ShowINIT now and due to popular demand,
     deltaX is back!
    -also due to popular demand, color icons are now done automatically.
    -note that the color icon is only used if 4 bits or more is available on
     the main graphics device; the normal #ICN is used for all other cases.
  
  Build & save this file as a 'PROC' resource, and include it in your
  INIT's resource file.  Use the following code within your INIT to load
  the 'PROC' and call CShowINIT:
  
  	Handle	procH;
  	
  	if ((procH = GetResource('PROC', PROC_ID)) != 0L)	{
			HLock(procH);
			CallPascal(ICON_ID, -1, *procH);
			HUnlock(procH);
		}

  -------------------------------------------------------------------------*/

/* #include <MacHeaders> */
/* #include <Color.h> */

typedef struct QuickDraw {       /* struct to hold QuickDraw globals */
  char private[76];
  long randSeed;
  BitMap screenBits;
  Cursor arrow;
  Pattern dkGray;
  Pattern ltGray;
  Pattern gray;
  Pattern black;
  Pattern white;
  GrafPtr thePort;
} QuickDraw;

/*extern short ROM85 : 0x28E;*/
/*extern GDHandle MainDevice : 0x8A4;*/

extern short myH : 0x92C;        /* CurApName+28   */
extern short myCheck: 0x92E;     /* CurApName+30   */

#define  firstX         8        /* left margin - offset to first icon */
#define  bottomEdge     8        /* this far from bottom of screen */
#define  iconWidth      32       /* size of icon (square normally) */
#define  defaultMoveX   40       /* default amount to move icons */
#define  checksumConst  0x1021   /* constant used for computing checksum */
#define  minColorDepth  4        /* min. bits/pixel for drawing color icons */
#define  maskOffset     128      /* offset to mask in ICN# resource */
#define  iconRowBytes   32/8     /* 32/8 bits */
#define  hasCQDBit      6        /* bit in ROM85 cleared if CQD available */

/*-------------------------------------------------------------------------
  Display the ICN# (cicn when in 4 bit mode or higher) specified by iconID
  and move the pen horizontally by moveX.  Pass a -1 in moveX to move the
  standard amount (40 pixels).

  pascal void ShowINIT(iconID, moveX)
    short iconID, moveX;
    extern;

  -------------------------------------------------------------------------*/

pascal void main(iconID, moveX)
short iconID, moveX;
{
  Handle  theIconHdl;  					/* handle to the icon (or cicn) */
  short 	dh;             			/* for calculating horizontal offset */
  short  colorFlag;             /* set if drawing a color icon */
  short  theDepth;              /* depth of main screen; used for CQD only */
  GDHandle theMainDevice;       /* handle to main screen device; CQD only */
  Rect srcRect, destRect;       /* source & destination rectangles */
  BitMap myBitMap;              /* icon bitmap; used for b/w icon only */
  GrafPort myPort;              /* port we draw into */
  QuickDraw qdGlobals;         	/* our own personal QD globals... */
  Ptr localA5;                  /* pointer to qdGlobals.thePort */
  Ptr	savedA5;									/* storage for saved contents of A5 */
  
  asm {
    move.l  A5,savedA5					/* save "real" QD globals ptr */
    lea  		localA5,A5					/* set up A5 to point to our globals */
    move.l  A5,CurrentA5
  }
  InitGraf(&qdGlobals.thePort);	/* initialize our qdGlobals structure */
  OpenPort(&myPort);
  colorFlag = 0;								/* default: no color */

  if (!(BitTst(&ROM85, 7-hasCQDBit))) {	/* does CQD exist? */
    theMainDevice = MainDevice;					/* yes; get handle to main device */
		theDepth = (*(*theMainDevice)->gdPMap)->pixelSize;
    if (theDepth >= minColorDepth)	{		/* deep enough to draw in color? */
      if ((theIconHdl = (Handle)GetCIcon(iconID)) != 0L)
        colorFlag = 1;									/* found a color icon; set flag */
    }
  }

  if (!(colorFlag))	{  /* no CQD, insufficient depth, or lack of 'cicn' */
    if (!(theIconHdl = GetResource('ICN#',iconID))) {
      SysBeep(3);  /* can't get b/w icon; signal error and bail out */
      goto out;
    }
  }
  dh = (myH << 1) ^ checksumConst;           /* checksum to find dh */
  myH = ((dh == myCheck) ? (myH):(firstX));  /* reset if necessary */
  /* notice that we stored the new horizontal value directly back into
  	the low-memory 'myH' location, rather than using a temporary variable.
  	This is the way the original ShowINIT works, and IconWrap relies on it. */

  destRect.bottom = myPort.portRect.bottom - bottomEdge;
  destRect.left = myPort.portRect.left + myH;
  destRect.top = destRect.bottom - iconWidth;
  destRect.right = destRect.left + iconWidth;

  if (colorFlag) {                                /* draw color icon */
  	PlotCIcon(&destRect,(CIconHandle)theIconHdl);
    DisposCIcon((CIconHandle)theIconHdl);
  }
  else {                                          /* draw b/w icon */
    HLock(theIconHdl);
    srcRect.top = srcRect.left = 0;
    srcRect.bottom = srcRect.right = iconWidth;
    myBitMap.rowBytes = iconRowBytes;
    myBitMap.bounds = srcRect;
    myBitMap.baseAddr = *theIconHdl + maskOffset; /* punch hole with mask */
    CopyBits(&myBitMap, &myPort.portBits, &srcRect, &destRect, srcBic, 0L);
    myBitMap.baseAddr = *theIconHdl;              /* now draw the icon */
    CopyBits(&myBitMap, &myPort.portBits, &srcRect, &destRect, srcOr, 0L);
    HUnlock(theIconHdl);
    ReleaseResource(theIconHdl);
  }
  myH += ((moveX == -1) ? (defaultMoveX):(moveX));  /* advance for next time */
  myCheck = (myH << 1) ^ checksumConst;             /* calc new checksum */

out:
  ClosePort(&myPort);
  asm {
    move.l  savedA5,A5
    move.l  A5,CurrentA5
  }

}

/*-------------------------------------------------------------------------*/
