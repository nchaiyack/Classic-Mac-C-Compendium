/*
	New ShowInit.c (Derivative of Apple's ShowInit.c code).
	
	Code to handle the display of an icon at the bottom of the screen during startup.
	This code is a modification of the original ShowInit code (see author's comments
	below).  It has been modified to do a couple of things:
	
		1.	It uses icon families.  That means that the appropriate icon will be displayed to match the
			depth of the monitor.
			
		2.	It loads and calls an externally defined code resource which returns a boolean value.  This
			value indicates which icon family to be displayed.  Family #128 is used when the init is
			able to load (verification routine returns true) and family #129 is used when the init is
			unable to load (verification routine returns false).
			
		3.	If the init is able to load, it loads and calls an externally defined code resource to handle
			the in-depth stuff.
		
		4.	Failure on the part of the code to find the verification routine indicates that the init cannot load.
			If the icons can not be found, no icon is displayed.  If the second code resource is unavailable,
			the init code just exits.
			
	By doing the init resource this way, this code resource can be compiled and copied into many different
	files.  It provides a standard way of creating the extensions/cdevs/whatever that allows them to have
	an icon displayed at startup.
		
	4/24/94 dn - Created (from the original).
	5/7/94 dn - Following modifications:
		� Doesn't use the verify code resource, always draws the good icon.
		� Doesn't have a bad icon, the only icon displayed is family number 300
		� Doesn't use the init code resource from #3 above.
	7/3/94 dn - Modified to use with the Universal Headers.
	
	(view using geneva 9 point font)
*/

/*
	Written 12:55 pm Jul 27, 1988 by jwhitnell@cup.portal.com.UUCP in 
	zaphod:comp.sys.mac.programmer.  It's short enough that I'll break the UseNet 
	cardinal rule and post it here:

	Simple INIT notification routine in LightSpeedC 2.15
*/
/*
	Jerry Whitnell
	jwhitnell@cup.portal.com
	..!sun!cup.portal.com!jwhitnell
*/

/*
	Also code from Think Reference v2.0.  The code comes from the section under 'Using QD Globals in an INIT'.
*/

#include <Icons.h>

#include <Finder.h>
#include <GestaltEqu.h>

#include "LogLibComponent Private.h"

/*
	Duplicates the QD global structure.
*/
typedef struct {
	char		privates[76];
	long		randSeed;
	BitMap	screenBits;
	Cursor	arrow;
	Pattern	dkGray;
	Pattern	ltGray;
	Pattern	black;
	Pattern	white;
	GrafPtr	thePort;
	long		qdEnd;
} QD_Globals;


/*
	This is where the horizontal offset is stored.  An INIT uses these values to determine where it should draw
	it's icon, afterwards it updates the values for the next INIT.
*/
extern short myHoriz=	0x092c;			// CurAppName + 28
extern short myCheck=	0x092e;			// CurAppName +30
extern long oldSig=		0x0a78;			// ApplScratch
extern short oldHoriz=	0x0a7e;			// ApplScratch + 6

// local prototypes.
void ComponentShowInit(void);
void DrawTheIconFamily(short id,GrafPtr port);

/*
	ComponentShowIcon
	
	This accesses two code resources: the first is a verification resource which lets the procedure know
	which icon to display.  The second handles the 'guts' of the process.  Control is passed to this code resource
	where it is supposed to do the INIT/patch/whatever thing that it is supposed to do.
	
	Gutted to merely display the icon at startup time.
*/
void ComponentShowInit(){
	long	oldA5;						// a5 register value
	long result;						// result from the Gestalt call
	QD_Globals qd;						// the fake QD globals
	CGrafPort	gp;						// the color graf port to be drawing in
	OSErr err;						// os errors
	GrafPtr localPtr;					// the pointer to the port
	
	oldA5 = SetA5((long) &qd.qdEnd);		// Tell A5 to point to the 'fake' QD Globals
	InitGraf(&qd.thePort);				// Initialize the fake QD Globals

	// This code tests the screen device type, so we can tell whether to open up a Color or B&W GrafPort.
	err = Gestalt (gestaltQuickdrawVersion, &result);
	
	if ((err == noErr) && (result >= gestalt8BitQD))	// then this is a color mac
		OpenCPort(&gp);
	else										// then it is only black and white
		OpenPort((GrafPtr)&gp);
	
	localPtr=(GrafPtr)(&gp);						// cast to get the pointer to a normal graf port
	
	DrawTheIconFamily(300,localPtr);
	
	ClosePort((GrafPtr)&gp);						// close the port
	
	SetA5(oldA5); 								// Restore A5 to its previous value
}

/*
	DrawTheIconFamily
	
	Given the id of the family to draw, this routine will draw the appropriate icon family (using PlotIconID)
	at the next available icon position.
*/
void DrawTheIconFamily(short id,GrafPtr localPort){
	register short hoffset;						// horizontal offset
	Rect dest;									// destination rectange where the icon should be drawn
	
	hoffset=myHoriz;							// get the current horizontal position
#if USESROUTINEDESCRIPTORS
	hoffset >>= 1;
	hoffset = (hoffset | 0x1021)&(~(hoffset & 0x1021));
#else
	asm {									// compute a checksum on the value
			rol #1,hoffset
			eor #0x1021,hoffset
	}
#endif

	if (hoffset==myCheck){						// if they match
		hoffset=myHoriz;						// use the original value
	} else {									// otherwise
		if (oldSig=='Paul')						// if the sig is 'Paul' (I assume for compatibility reasons)
			hoffset=oldHoriz;					// use the old horizontal offset
		else									// otherwise
			hoffset=8;						// we are the first, use 8
	}
	
	/*
		add code here to wrap the icon around?
	*/
	
	dest.bottom=localPort->portRect.bottom-8;		// set the bottom to the vertical position
	dest.left=localPort->portRect.left+hoffset;		// set the left to the horizontal position
	dest.top=dest.bottom-32;						// make room for 32 bit tall icons
	dest.right=dest.left+32;						// make room for 32 bit wide icons
	
	hoffset += 40;								// compute the next position
	myHoriz=hoffset;							// save it for the next INIT

#if USESROUTINEDESCRIPTORS
	hoffset >>= 1;
	hoffset = (hoffset | 0x1021)&(~(hoffset & 0x1021));
#else
	asm {									// compute the checksum
			rol #1,hoffset
			eor #0x1021,hoffset
	}
#endif
	
	myCheck=hoffset;							// save it for the next INIT too
	
	PlotIconID(&dest,atVerticalCenter+atHorizontalCenter,ttNone,id);	// draw the icon centered with no transformations
}
