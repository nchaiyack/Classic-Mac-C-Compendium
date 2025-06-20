	

#include <QuickDraw.h>
#include <Memory.h>
#include <Windows.h>
#include <GraphicsModule_Types.h>
#include "ad_flame.h"
#include "xlock.h"

#include <GestaltEqu.h>

#include "GraphicsModule_Types.h"
#include "Sounds.h"


#define ABOUT_PICT 256 // the about box pict resource ID


// these are the functs that need defined ...
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params);

// extra ones
OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params);
OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params);


short exponentiate( short x, short n);

short	gTotalPoints;
Boolean gRainbowOption = FALSE;
short	gMaxBatch;					// how many points to plot at a time
long	gTimeSlice;					// time slice for drawflame()-end-recursion
									// * in seconds *
extern Boolean gWaitForClick;		// don't wake up on mouse movement ?
extern short theScreen;				// which monitor[] are we working on?


//////////////////////////////////////////////////////////////////////////////////////
// this is the first funct called by AD ... we need to allocate and initialize here
OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	Handle 			h;

	GetDateTime( (unsigned long *)&(params->qdGlobalsCopy->qdRandSeed) );
	
	// if we're in demo mode, mouse movement can happen
	gWaitForClick = !EmptyRect( &(params->demoRect) );
	
// for each of the monitors, initialize the flame stuff
	for (theScreen=0; theScreen < params->monitors->monitorCount; theScreen++ ) {
		// call initflame with the rectangle and it's depth
		initflame(  &(params->monitors->monitorList[theScreen].bounds),
				params->monitors->monitorList[theScreen].curDepth);
	}
	
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// the screen saver has been awakened! time to ditch the storage and wave goodbye
OSErr 
DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	// no storage here, we've used globals through the whole thing
	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////
// make the screen go black
OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	// have to do this, else we get a color flash
	BackColor( blackColor);
	EraseRgn(blankRgn);	
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// this is the workhorse routine. It does the continual screen work to make
// this screen saver what it is.
OSErr 
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	
	gRainbowOption = params->controlValues[0] ;
	gTotalPoints = 5000 + (5000 * (params->controlValues[1] / 34));
	
	gMaxBatch = exponentiate(10, params->controlValues[2] / 26  );
	// values of 0...100 gives 0,1,2,3
	// which exponentiate to 1, 10, 100, 1000
	
	// min of 10 seconds ... up to 60 seconds
	gTimeSlice = params->controlValues[3] + 20;
	
	if (theScreen < 0 || 
			theScreen >= params->monitors->monitorCount ||
				theScreen >= MAXSCREENS)
		theScreen = 0;
		 
	drawflame();
	
	theScreen++;	// next time, do the next screen

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





OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params)
{
	GrafPtr	savePort;
	PicHandle	pictH;
unsigned short RangedRdm( unsigned short min, unsigned short max );
	short pixelSize;


	gTotalPoints = 1000;
	gMaxBatch = 1;
	gTimeSlice = 30;
	
	GetPort( &savePort);
	
	if (HasColorQD()) {
	
		pixelSize = ( *((CGrafPtr)savePort)->portPixMap)->pixelSize;
		if (  !RangedRdm(0, 2) )
			gRainbowOption = TRUE;
		else 
			gRainbowOption = FALSE;
	} else { 
		gRainbowOption = FALSE;	// B&W means no rainbows
		pixelSize = 1;
	}
	
// ********** do it

	theScreen = 0;
	
	// setup "screen 0" to be the demo rectangle!
	initflame(  &(savePort->portRect), pixelSize);

	while (Button())	// let go of the button!!
		;
		
	gWaitForClick = TRUE; 		// mouse movement doesn't matter
	
	while (!Button()) {			// keep drawing while there is no button
		drawflame();
		//EraseRect( &(savePort->portRect) );
	}
	
	while (Button())		// let go of the button!!
		;		
		
// ********** show the about picture ...

	pictH = GetPicture( ABOUT_PICT);			// color about box
	
	if (pictH == NULL) {
		MoveTo( 10, 10);
		DrawString("\pAbout-box picture missing!!");
		MoveTo( 10, 30);
		DrawString("\p<click to continue>");
	} else		// ahhhh... couldn't we dissolve the pict into the window ??? huh?
		DrawPicture( pictH, &(*pictH)->picFrame); 	// was:  &(savePort->portRect) );

	while (!Button())	// wait for a click
		;
		
		
	ReleaseResource( (Handle)pictH );

	return noErr;
}

#if 1
Boolean
HasColorQD(void) {
	OSErr	err;
	Boolean answer = true;
	long	gestaltResult;

	err = Gestalt(gestaltQuickdrawVersion, &gestaltResult);

	answer = (err == noErr) && (gestaltResult >= gestalt8BitQD);
	return answer;
}
#endif

short exponentiate( short x, short n) {
short p;

	for (p=1; n>0; --n)
		p = p * x;
	return p;


}