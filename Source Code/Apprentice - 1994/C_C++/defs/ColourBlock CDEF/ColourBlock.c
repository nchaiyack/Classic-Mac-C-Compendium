/*	NAME:
		ColourBlock.c

	WRITTEN BY:
		Dair Grant
		grantd@dcs.gla.ac.uk
				
	DESCRIPTION:
		A CDEF for getting a colour value from the user.

	___________________________________________________________________________

	VERSION HISTORY:
		1.0: (Jan 1994, dg)
			¥	First public release.

	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <QuickDraw.h>
#include "ColourBlock.h"







//=============================================================================
//		Private defines															 
//-----------------------------------------------------------------------------
#define kBlockWidth			21								// Width of colour block
#define kBlockHeight		12								// Height of colour block
#define kTitleOffset		4								// Offset of title







//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
pascal long main(int theVarCode, ControlHandle theCntl, int theMsg, long theParam);

void		InitialiseMyControl(void);
void		DisposeMyControl(void);
long		TestMyControl(void);
void		DrawMyControl(void);
pascal void	DeviceLoopRoutine(short theDepth, short theFlags, GDHandle theDevice,
							  Handle theHnd);
RgnHandle	CalculateMyRegion(void);
long		TrackMyControl(void);







//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
Boolean			gHasBeenCalled=false;				// Has the CDEF been called before?
RGBColor		gFrameColour;						// Colour of the control frame
RGBColor		gTextColour;						// Colour of the control text
RGBColor		gBodyColour;						// Fill colour for the control
Rect			gTheColourRect;						// Rectangle for the colour block
Point			gTheTitlePt;						// Point to draw the title on
	
int				gTheVarCode;						// The varCode paramater
ControlHandle	gTheCntl;							// The theCntl paramater
int				gTheMsg;							// The theMsg paramater
long			gTheParam;							// The theParam paramater









//=============================================================================
//		main : Entry point for the WDEF definition.
//-----------------------------------------------------------------------------
//		Note :	Basically we case out on theMsg and let other routines do the
//				work.
//-----------------------------------------------------------------------------
pascal long main(int theVarCode, ControlHandle theCntl, int theMsg, long theParam)
{	long retVal = 0L;



	// Set up A4 so we can have access to our globals
	GetGlobals();
	

	// Store the paramaters in globals for speed
	gTheVarCode		= theVarCode;
	gTheCntl		= theCntl;
	gTheMsg			= theMsg;
	gTheParam		= theParam;
	
	
	// Case out on what we're meant to be doing
	switch (theMsg) {
		case initCntl:					// Initialise our private data for this control
			InitialiseMyControl();
			break;
		

		case dispCntl:					// Dispose of our private data for this control
			DisposeMyControl();
			break;


		case testCntl:					// Test mouse to see if in control
			retVal = TestMyControl();
			break;
		
		
		case drawCntl:					// Draw the correct part of this control
			DrawMyControl();
			break;
		
		
		case calcCRgns:					// Calculate the draw region of this control
		case calcCntlRgn:
		case calcThumbRgn:
			retVal = (long) CalculateMyRegion();
			break;
		
		
		case autoTrack:					// Ignored messages
		case posCntl:	
		case thumbCntl:
		case dragCntl:
			break;
		

		default:						// Anything else
			break;
	}
	



	// Restore A4 for our caller and return the result
	UngetGlobals();
	return(retVal);
}











//=============================================================================
//		InitialiseMyControl : Create a new instance of the control.
//-----------------------------------------------------------------------------
//		Note :	Allocate some private data and initialise a few variables,
//				including auxillary colour.
//-----------------------------------------------------------------------------
void InitialiseMyControl(void)
{	CDEFStructHnd	theCDEFDataHnd;
	CDEFStructPtr	theCDEFDataPtr;
	AuxCtlHandle	theAuxHnd;
	CCTabHandle		theTableHnd;
	
	
	
	
	
	// If this is the first control this CDEF has created,
	// we initialise some shared variables.
	if (!gHasBeenCalled)
		{
		// We assume we have colour QuickDraw, and try and get the System colours
		GetAuxCtl(gTheCntl, &theAuxHnd);
		if (theAuxHnd)
			{
			// Lock the handle and dereference.
			HLock(theAuxHnd);
			theTableHnd = (*theAuxHnd)->acCTable;
			
			
			// Copy the right colours
			gFrameColour	= (*theTableHnd)->ctTable[cFrameColor].rgb; 
			gTextColour		= (*theTableHnd)->ctTable[cTextColor].rgb;
			gBodyColour		= (*theTableHnd)->ctTable[cBodyColor].rgb;
			
			
			// Unlock the handle
			HUnlock(theAuxHnd);
			}
		
		
		// If we couldn't get the colour table, we use some hardwired defaults
		else
			{
			PackRGB(gFrameColour,	0x0000, 0x0000, 0x0000);
			PackRGB(gTextColour,	0x0000, 0x0000, 0x0000);
			PackRGB(gBodyColour,	0xFFFF, 0xFFFF, 0xFFFF);
			}
		
		
		// None of the above code needs to be done again
		gHasBeenCalled = true;
		}



	// Create some space for the control's data and initialise it.
	theCDEFDataHnd = (CDEFStructHnd) NewHandle(sizeof(CDEFStruct));
	if (theCDEFDataHnd)
		{
		// Start off white
		PackRGB((*theCDEFDataHnd)->blockColour, 0xFFFF, 0xFFFF, 0xFFFF);
		}


	// Copy the handle into the control
	(*gTheCntl)->contrlData = (Handle) theCDEFDataHnd;
}











//=============================================================================
//		DisposeMyControl : Dispose of an instance of the control.
//-----------------------------------------------------------------------------
void DisposeMyControl(void)
{


	// If the control has a private data structure, dispose of it
	if ((*gTheCntl)->contrlData)
		DisposeHandle((*gTheCntl)->contrlData);
}











//=============================================================================
//		TestMyControl : Return the part code for a point.
//-----------------------------------------------------------------------------
//		Note :	This routine is used when a mousedown has occurred or a track
//				is underway. We have to decide if a point is within our
//				boundaries.
//-----------------------------------------------------------------------------
long TestMyControl(void)
{	Point			testPoint;
	long			retVal=0;
	
	
	
	// We only test if we're enabled
	if ((*gTheCntl)->contrlHilite < 255)
		{
		// Convert the two coordinates to a point
		testPoint.h = LoWord(gTheParam);
		testPoint.v = HiWord(gTheParam);

	
		// Return inCheckBox if it's in our control at all
		if (PtInRect(testPoint, &(*gTheCntl)->contrlRect))
			retVal = inCheckBox;
		}

	
	// Return the result
	return(retVal);
}











//=============================================================================
//		DrawMyControl : Draw a part of a control.
//-----------------------------------------------------------------------------
//		Note :	We use DeviceLoop to call the actual drawing routine.
//-----------------------------------------------------------------------------
void DrawMyControl(void)
{	int				center;
	RGBColor		saveForeColour;
	FontInfo		theFontInfo;




	// Quick (and lazy) test to see if we've to do anything
	if (!(*gTheCntl)->contrlVis)
		return;
		
		
	// Zap the high word of gTheParam
	gTheParam = LoWord(gTheParam);
	
	
	// The colour block is put on the left side of the control rectangle...
	gTheColourRect			= (*gTheCntl)->contrlRect;
	gTheColourRect.right	= gTheColourRect.left + kBlockWidth;
	
	// And centered vertically inside it...
	center		= gTheColourRect.top + ((gTheColourRect.bottom - gTheColourRect.top) >> 1);
	gTheColourRect.top		= center - (kBlockHeight >> 1);
	gTheColourRect.bottom	= gTheColourRect.top + kBlockHeight;
	


	// The title is placed next to the colour block...
	gTheTitlePt.h = gTheColourRect.right + kTitleOffset;

	// And centered vertically for the font size...	we have the center of
	// the colour block, and we want to center the title on that.
	GetFontInfo(&theFontInfo);
	gTheTitlePt.v	= center + (theFontInfo.ascent >> 1) - 1;
	


	// Save the foreground colour
	GetForeColor(&saveForeColour);


	// Call the DeviceLoopRoutine with DeviceLoop to do the actual drawing.
	DeviceLoop((*gTheCntl)->contrlOwner->visRgn,
			   (DeviceLoopDrawingProcPtr) DeviceLoopRoutine, 0, allDevices);


	// Restore the foreground colour
	RGBForeColor(&saveForeColour);
}











//=============================================================================
//		DeviceLoopRoutine : Called by DeviceLoop to do the drawing.
//-----------------------------------------------------------------------------
//		Note :	If we're disabled, we draw ourselves in a real gray if the
//				device is deep enough, and a dithered gray otherwise.
//-----------------------------------------------------------------------------
pascal void DeviceLoopRoutine(short theDepth, short theFlags, GDHandle theDevice,
							  Handle theHnd)
{	RGBColor		blockColour, titleColour;
	Rect			theColourRect;
	CDEFStructHnd	theCDEFDataHnd;
	Boolean			ditherText=false;



	// Draw what we're to draw
	switch (gTheParam) {
		case inCheckBox:					// Draw/remove a selection from the block
			// Set the draw colour
			if ((*gTheCntl)->contrlHilite)
				RGBForeColor(&gFrameColour);
			else
				RGBForeColor(&gBodyColour);


			// Draw the selection rectangle
			theColourRect = gTheColourRect;
			InsetRect(&theColourRect, 1, 1);
			FrameRect(&theColourRect);
			break;
		
		
		case 255:							// Change in dimming
		case 0:								// Draw the entire control
			// Draw the frame
			theColourRect = gTheColourRect;
			RGBForeColor(&gFrameColour);
			FrameRect(&theColourRect);


			// Draw the selection rectangle
			RGBForeColor(&gBodyColour);
			InsetRect(&theColourRect, 1, 1);
			FrameRect(&theColourRect);


			// The interior of the colour block and the title are
			// coloured white and grey if the control is disabled.
			// If we're on a mono display, we will dither the
			// text if it's disabled.
			if ((*gTheCntl)->contrlHilite == 255)
				{
				blockColour = gBodyColour;
				PackRGB(titleColour, 0x8000, 0x8000, 0x8000);
				
				if (theDepth == 1)
					ditherText = true;
				}
			
			
			// Otherwise we use the normal colours.
			else
				{
				theCDEFDataHnd	= (CDEFStructHnd) ((*gTheCntl)->contrlData);
				blockColour		= (*theCDEFDataHnd)->blockColour;
				titleColour		= gFrameColour;
				}
				
			
			// Draw the interior of the colour block
			RGBForeColor(&blockColour);
			InsetRect(&theColourRect, 1, 1);
			PaintRect(&theColourRect);
						
			
			// Draw the title - switching to dithered text if in mono
			RGBForeColor(&titleColour);
			MoveTo(gTheTitlePt.h, gTheTitlePt.v);
			if (ditherText)
				TextMode(grayishTextOr);
			DrawString(&(*gTheCntl)->contrlTitle);
			if (ditherText)
				TextMode(srcOr);
			break;
	}
}











//=============================================================================
//		CalculateMyRegion : Calculate the control's area region.
//-----------------------------------------------------------------------------
//		Note :	If we're running under 24-bit addressing, gTheMsg will be
//				calcCRgns. Depending on the high bit of gTheParam we're
//				then to calculate either the indicator region (bit is set)
//				or the entire control (bit is not set).
//
//				Under 32-bit mode we can get sent either calcCntlRgn or
//				calcThumbRgn messages.
//
//				We check first for the calcCRgns message, and fudge things
//				so it turns into either a calcCntlRgn or a calcThumbRgn
//				message.
//-----------------------------------------------------------------------------
RgnHandle CalculateMyRegion(void)
{	RgnHandle	theRegion;
	Rect		theRect;
	int			theMsg;




	// Cast the param to a RegionHandle
	theRegion = (RgnHandle) gTheParam;
	
	
	// If we're running under 24 bit addressing, find out which message we're
	// meant to be doing.
	if (gTheMsg == calcCRgns)
		{
		// If the high bit is set we're to calculate the indicator
		if (0x80000000L & (long) theRegion)
			theMsg = calcThumbRgn;
			
			
		// Otherwise we're to calculate the entire control
		else
			theMsg = calcCntlRgn;
			
			
		// Mask off the high bit from the region handle
		theRegion = (RgnHandle) (0x7FFFFFFFL & (long) theRegion); 
		}
	else
		theMsg == gTheMsg;
	
	

	// theMsg is either calcThumbRgn or calcCntl region, irrespective
	// of 24 or 32 bit addressing. We handle it accordingly
	theRect = (*gTheCntl)->contrlRect;
	if (theMsg == calcThumbRgn)
		{
		// Calculate the indicator region
		theRect = (*gTheCntl)->contrlRect;
		theRect.right = theRect.left + kBlockWidth;
		}
	
	// Calculate the entire control region
	else
		{
		theRect = (*gTheCntl)->contrlRect;
		}
	
	
	
	// Set theRegion to the rectangle
	RectRgn(theRegion, &theRect);
	
	
	
	// See 'Control Manager Q&As' Tech note for why we return 1 if
	// we were called under 32 bit mode.
	if (gTheMsg != calcCRgns)
		theRegion = (RgnHandle) 0x1L;
		
	
	// Return the answer
	return(theRegion);
}
