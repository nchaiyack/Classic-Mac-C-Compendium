/* 
    Copyright � General Consulting & Research Inc. 1993-1994 All rights reserved.
  	Author: 	Peter H. Teeson 
    CIS:		72647,3674
    AppleLink:	CDA0197
 	Date:		1 June 1993 
  	Path:IconShow++ �:IconShow.cp
 			
	Please read the ReadMeIS.note.
	
	The algorithm:
 		� Step (0): Initialise our own local Quickdraw and determine environment.
		� Step (1): Calculate our actual screen size and setup globals
		� Step (2):	Map low memory as a table of unsigned shorts
		� Step (3): Calculate where to place our icon
		� Step (4): Open GrafPort, draw the icon, close GrafPort, maybe advance

 */

#include <GestaltEqu.h>
#include <SITable.h>			// the class for the Show Init table
struct myQDGlobals {	// for our local QD
	char privates[76];
	long randSeed;
	BitMap screenBits;
	Cursor arrow;
	Pattern dkGray;
	Pattern ltGray;
	Pattern gray;
	Pattern black;
	Pattern white;
	GrafPtr thePort;
	long end;
};
unsigned short myScreenWidth=0,myScreenDepth=0; 
// -------- prototypes & inlines --------
pascal void IconShow(short iconId, Boolean advance);
OSErr DrawBWIcon( short iconId, Rect *icon_rect, Boolean visible );
pascal OSErr PlotIconID
	(const Rect *theRect, short align, short  transform, short theResID)
	= {0x303C, 0x0500, 0xABC9};	
// -------- code --------
pascal void IconShow( short iconId, Boolean advance )
{				
 	/*
 		� Step (0): Initialise our own local Quickdraw and determine environment
 	*/
 	GrafPtr	savePort;							// Save old GrafPort
 	GetPort(&savePort);
 	myQDGlobals	qd;								// holds our QD globals
 		
#ifdef	__A4_GLOBALS__							// if we are NOT debugging
 	long oldA5 = SetA5((long)&qd.end);				// our own A5
#endif
  	InitGraf(&qd.thePort);						// sets up our local qd globals
	
	OSErr 		err=noErr;						// temp
	long		SysVers,QDVers;					// environment vars
	Boolean		hasColourQD=false;				// assume not
	
	err=Gestalt(gestaltSystemVersion,&SysVers);	// what System version	
	err=noErr;									// do we have colour QuickDraw
	err=Gestalt(gestaltQuickdrawVersion,&QDVers);	
	if ((err == noErr) && QDVers>=0x0100)
		hasColourQD=true;

	/*
	 	� Step (1): Calculate our actual screen size and setup globals
	*/
	Rect qd_rect=qd.screenBits.bounds;
  	myScreenWidth = qd_rect.right - qd_rect.left;	// Actual width
	myScreenWidth -= myScreenWidth % XOffset;		// modulus icon width + a bit
	myScreenDepth = qd_rect.bottom - qd_rect.top;	// Actual depth
	myScreenDepth -= myScreenDepth % YOffset;		// modulus icon depth + a bit

	/*
		� Step (2):	Map low memory as a table of unsigned shorts
	*/	
	SITable	STable;								// ShowINIT's table

	/*
	 	� Step (3): Calculate where to place our icon
	*/
 	Rect iconRect;								// the rect we use to place our icon
 	iconRect.left = STable[hloc] % myScreenWidth;	// may be bad values			
 	iconRect.top  = STable[vloc] % myScreenDepth;	// left by others
	iconRect.right = iconRect.left + IconWidth;
	iconRect.bottom = iconRect.top + IconWidth;
	
	/*
		� Step (4): Open GrafPort, draw the icon, close GrafPort, maybe advance
	 */
	CGrafPort gp;								// our Grafport
	
	if ((SysVers >= 0x0700) && hasColourQD)		// System 7 & has colour Quickdraw
	{
		OpenCPort(&gp);								// Colour 
		err = PlotIconID( &iconRect, 0x0000, 0x0000, iconId );
		if (advance && !err) 					// Advance iff we plotted
			STable.NextLoc(STable);
		CloseCPort(&gp);
	}
	else										// the ones that are not
	{
		OpenPort((GrafPtr)&gp);						// B & W
		err=DrawBWIcon( iconId, &iconRect, true );
		if (advance && !err) 					// Advance iff we plotted
			STable.NextLoc(STable);
		ClosePort((GrafPtr)&gp);
	}		

#ifdef	__A4_GLOBALS__						// if we are NOT debugging
  	SetA5(oldA5);									// restore original A5
#endif

	SetPort(savePort);
}

OSErr DrawBWIcon( short iconId, Rect *icon_rect, Boolean visible )
{	// Draws the iconid member of the 'ICN#' icon family.
	OSErr	ans=TRUE;
	Handle icon = Get1Resource('ICN#', iconId);
	if (!icon)
		return ans=FALSE;		// This means iconid was not found in resource file
	HLock(icon);
	
	/* prepare the source and destination bitmaps. */
	BitMap source;
	source.baseAddr = *icon + 128;						// mask address
	source.rowBytes = 4;
	SetRect(&source.bounds, 0, 0, 32, 32);
	GrafPtr port;
	GetPort( &port );
	BitMap destination = port->portBits;
	RgnHandle empty_mask = visible? nil : NewRgn();
	
	/* transfer the mask. */
	CopyBits(&source, &destination, &source.bounds, icon_rect,
		srcBic, empty_mask);
	
	/* and the icon. */
	source.baseAddr = *icon;	
	CopyBits(&source, &destination, &source.bounds, icon_rect,
		srcOr, empty_mask);
	
	if (empty_mask) DisposeRgn( empty_mask );
	return ans;
}
