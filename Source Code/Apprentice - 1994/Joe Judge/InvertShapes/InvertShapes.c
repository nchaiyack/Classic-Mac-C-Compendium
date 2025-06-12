
// ver 1.0.1 - updated the RangedRdm() routine
// 		- fixed where I had called it badly

#include <QuickDraw.h>
#include <Memory.h>
#include <Resources.h>

#include <QDoffscreen.h>

#include "GraphicsModule_Types.h"
#include "Sounds.h"

unsigned short RangedRdm( unsigned short min, unsigned short max );

// these are the functs that need defined ...
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params);

// extra ones
OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params);
OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params);



// this is borrowed from the example code from Bouncing Ball ...
/* some macros to simplify synchronizing to the vertical retrace. */
#define SynchFlag(m) (params->monitors->monitorList[m].synchFlag)
#define SynchVBL(m) synchFlag = &SynchFlag(m); *synchFlag = false; while(!*synchFlag);






//////////////////////////////////////////////////////////////////////////////////////
// this is the first funct called by AD ... we need to allocate and initialize here
OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	
	// Randomize...
	params->qdGlobalsCopy->qdRandSeed = TickCount();

	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// the screen saver has been awakened! time to ditch the storage and wave goodbye
OSErr 
DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////
// make the screen go black
OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	if (params->controlValues[2])
		FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
	return noErr;

}

//////////////////////////////////////////////////////////////////////////////////////
// this is the workhorse routine. It does the continual screen work to make
// this screen saver what it is.
OSErr 
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	long	when;
	Rect	r, s;
	short val;
	static theMonitor = 0;
	static theCountdown = 0;
	long color = blackColor;
	Boolean doColor;


//	if (  !(Random()%10) ) {	// how fast these happen
	if (!theCountdown) {
		short wheretodim;
		wheretodim = (params->controlValues[theMonitor] * 255 ) / 100;
		
		params->brightness -= 1;	//5;
		
		if (params->brightness < wheretodim) 
			params->brightness = wheretodim;
	}

	
	// if we have color and this monitor is not B&W, set the flag
	doColor = params->colorQDAvail &&									// we have color?
		params->monitors->monitorList[theMonitor].curDepth > 1 &&		// screen has some depth?
		params->controlValues[3];										// check box checked?
		
	if (doColor) {
		short pm;
		
		color = RangedRdm(0, 7);	// 8 predefined colors
		switch (color) {
			case 1: ForeColor(blackColor); break;
			case 2: ForeColor(whiteColor); break;
			case 3: ForeColor(redColor); break;
			case 4:	ForeColor(greenColor); break;
			case 5:	ForeColor(blueColor); break;
			case 6:	ForeColor(cyanColor); break;
			case 7:	ForeColor(magentaColor); break;
			default:ForeColor(yellowColor); break;
		}
		// should I change the backcolor here also ?
		do
			pm = RangedRdm( patCopy, notPatBic);
		while (pm == patBic);	// anything but picBic - that makes it white
		PenMode( pm );
	}
	
	theCountdown++;
	if (theCountdown >= 5) theCountdown = 0;
	
	
	r.left = RangedRdm( params->monitors->monitorList[theMonitor].bounds.left, 
			params->monitors->monitorList[theMonitor].bounds.right);
			
	r.right = RangedRdm( params->monitors->monitorList[theMonitor].bounds.left,
			params->monitors->monitorList[theMonitor].bounds.right);
	
	r.top = RangedRdm( params->monitors->monitorList[theMonitor].bounds.top, 
			params->monitors->monitorList[theMonitor].bounds.bottom);
			
	r.bottom = RangedRdm( params->monitors->monitorList[theMonitor].bounds.top,
			params->monitors->monitorList[theMonitor].bounds.bottom);

	if (++theMonitor >= params->monitors->monitorCount)
		theMonitor=0;
	
	val = params->controlValues[1];
	// first menu item is RANDOM!!!!
	if (val < 2) 
		val = RangedRdm(2, 5);		// 4 choices now
		
	switch( val ) {
	case 2:	
		if (doColor)
			PaintRect( &r);
		else
			InvertRect( &r );			
		break;
	case 3:	
		if (doColor)
			PaintArc( &r, RangedRdm(0, 360), RangedRdm(0, 360) );		
		else
			InvertArc( &r, RangedRdm(0, 360), RangedRdm(0, 360) );		
		break;
	case 4:	
		if (doColor)
			PaintOval( &r);		
		else
			InvertOval( &r );		
		break;
	case 5:	{
			short w, h;
			
			w = r.right - r.left;
			if (w <= 2) 
				w = 2;
			else 
				w = RangedRdm(2, w);
			h = r.bottom - r.top;
			if (h <= 2) 
				h = 2;
			else 
				h = RangedRdm(2, h);
			if (doColor)
				PaintRoundRect( &r, w, h);
			else
				InvertRoundRect( &r, w, h);	
		}
		break;
	default:	
		SysBeep(0);
		InvertRect( &r );	
		break;
	}
	
	return noErr;
	
}
//////////////////////////////////////////////////////////////////////////////////////
// this is called when they click on something in the control panel
OSErr 
DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params) {

	return noErr;
}



OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params)
{
	// I tried playing with params here and they don't seem instantiated,
	// so don't play too much in this routine
	return noErr;
}



// this is from the Think C reference code example ...
unsigned short RangedRdm( unsigned short min, unsigned short max )
/* assume that min is less than max */
{
	// uh ... not this isn't quite right - it's between 0 and 65535, not 65536
	unsigned	qdRdm;	/* treat return value as 0-65536 */
	long	range, t;
	
	// just to be safe, I'll put this here
	if (min > max) DebugStr("\pMin greater then Max in RangedRdm");
	
	qdRdm = Random();
	range = max - min;
	// max - min gives us the the difference between max and min ... that is 
	// not inclusive. It gives us { min <= range < max }
	// so we never see that max number!!
	range++;
	t = ((long)qdRdm * range) / 65536; 	/* now 0 <= t <= range */
	return( t+min );
}



OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params)
{
	// I set the proper resource, but I never get this call!!!???
	SysBeep(0);		// lets see if we ever get this verdammnt routine called.
	return noErr;
}
