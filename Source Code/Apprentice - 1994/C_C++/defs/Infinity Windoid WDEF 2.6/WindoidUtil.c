// *****************************************************************************
//
//	FILE:
//		WindoidUtil.c
//
//	WRITTEN BY:
//		Troy Gaul
//		Infinity Systems
//
//		й 1991-94 Infinity Systems
//		All rights reserved.
//
//	DESCRIPTION:
//		This file contains utility routines that the Infinity Windoid
//		WDEF uses in order to get its job done.
//
//	HOW TO CONTACT THE AUTHOR:
//		Send e-mail to: t-gaul@i-link.com
//
// *****************************************************************************

// *****************************************************************************
//	Headers
// -----------------------------------------------------------------------------

#include "WindoidDefines.h"			// must be included before Apple interfaces

#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <Windows.h>
#include <Palettes.h>
#include <ToolUtils.h>
#include <GestaltEqu.h>

#include "WindoidTypes.h"
#include "WindoidUtil.h"

// *****************************************************************************
//	Environment-determining Routines																			 
// -----------------------------------------------------------------------------
	//	These use SysEnvirons by default so we don't have to rely on Gestalt 
	//	being available and so MPW won't include that code in our resource. 
	//	This can be changed by defining USE_GESTALT

Boolean 
HasSystem7(void) {
#ifndef USE_GESTALT
	SysEnvRec theWorld;
	
	return (SysEnvirons(1, &theWorld) == noErr 
			&& theWorld.systemVersion >= 0x0700);
#else   
	long vers = 0;
	
	return (Gestalt(gestaltSystemVersion, &vers) == noErr 
			&& ((vers & 0xFFFF) >= 0x0700));
#endif
}

// -----------------------------------------------------------------------------

Boolean 
HasCQDraw(void) {
#ifndef USE_GESTALT
	SysEnvRec theWorld;
	
	return ((SysEnvirons(1, &theWorld) == noErr) && 
			theWorld.hasColorQD);
#else
 	long vers = 0;
	
	return (Gestalt(gestaltQuickdrawVersion, &vers) == noErr 
			&& (vers & 0xFF00));
#endif  
}

// *****************************************************************************
//	SyncPorts
// -----------------------------------------------------------------------------
	//	Straight from the pages of _Macintosh Programming Secrets_, Second 
	//	Edition by Scott Knaster and Keith Rollin (page 425). (except that this
	//	version doesn't check Gestalt, it will only be called if CQD is running)
	//	This routines was added to 2.3. It makes sure the drawing environment 
	//	is set correctly if the system has color. This is not needed for the 
	//	code in this WDEF as it is, but if a DoWDrawGIcon handler is implemented, 
	//	this is needed to make sure the drawing environment is set as Apple 
	//	tells us it will be for drawing the gray, xor'ed border.

void 
SyncPorts(void) {
	GrafPtr bwPort;
	CGrafPtr colorPort;
	
	GetWMgrPort(&bwPort);
	GetCWMgrPort(&colorPort);
	SetPort((GrafPtr) colorPort);
	
	BlockMove(&bwPort->pnLoc, &colorPort->pnLoc, 10);
	BlockMove(&bwPort->pnVis, &colorPort->pnVis, 14);
	PenPat((ConstPatternParam) &bwPort->pnPat);
	BackPat((ConstPatternParam) &bwPort->bkPat);
}

// *****************************************************************************
//	OurDeviceLoop
// -----------------------------------------------------------------------------
#ifndef SYS7_OR_LATER

#ifdef UNIV_HEADERS

void 
OurDeviceLoop(RgnHandle drawingRgn, DeviceLoopDrawingUPP drawingProc,
			  long userData, DeviceLoopFlags flags) {
#else

void 
OurDeviceLoop(RgnHandle drawingRgn, DeviceLoopDrawingProcPtr drawingProc,
			  long userData, DeviceLoopFlags flags) {
#endif

	if (HasSystem7()) {
		DeviceLoop(drawingRgn, drawingProc, userData, flags);
			// this works with or without Color Quickdraw
	} else if (HasCQDraw()) {
		short depth;
		Rect deviceRect;
		GDHandle theDevice;
		RgnHandle saveClip = NewRgn();
		RgnHandle deviceRgn = NewRgn();
		RgnHandle intersectingRgn = NewRgn();
		
		GetClip(saveClip);
		
		
		//	Get the handle to the first device in the list.
		
		theDevice = GetDeviceList();
		
		
		//	Loop through all the devices in the list.
		
		while (theDevice) {
			
			// Get the device's gdRect and convert it to local coordinates.
			
			deviceRect = (**theDevice).gdRect;
			depth = (**(**theDevice).gdPMap).pixelSize;
			
			GlobalToLocal((Point*)&deviceRect.top);
			GlobalToLocal((Point*)&deviceRect.bottom);
			
			
			//	Check if the app's window rect intersects the device's, and if it
			//	does, set the clip region's rect to the intersection, then DRAW!
			
			RectRgn(deviceRgn, &deviceRect);
			SectRgn(drawingRgn, deviceRgn, intersectingRgn);
			SectRgn(intersectingRgn, saveClip, intersectingRgn);
			
			if (!EmptyRgn(intersectingRgn)) {
				SetClip(intersectingRgn);

#ifdef UNIV_HEADERS
				CallDeviceLoopDrawingProc(drawingProc, depth, 
										  (**theDevice).gdFlags, theDevice, 
										  userData);
#else
				(*drawingProc)(depth, (**theDevice).gdFlags, theDevice, userData);
#endif
			}
			
			
			//	Get the next device in the list.
			
			theDevice = GetNextDevice(theDevice);
		}
		
		SetClip(saveClip);
		DisposeRgn(saveClip);
		DisposeRgn(deviceRgn);
		DisposeRgn(intersectingRgn);
	} else {

		//	68000 machine, original QuickDraw, punt

#ifdef UNIV_HEADERS
		CallDeviceLoopDrawingProc(drawingProc, 1, 0, nil, userData);
#else
		(*drawingProc)(1, 0, nil, userData);
#endif
	}
}

#endif
// *****************************************************************************
//	Color Mixing Routines																	 
// -----------------------------------------------------------------------------

//	This routine will return some defaults in case neither the window's color
//	table nor the System's is long enough to contain the color requested
//	It was provided by Jim Petrick as part of a fix for a bug in version 2.3
//	of the Infinity Windoid. This problem would be seen if a custom WCTB was
//	being used which was not as long as the default System one (or if the
//	System one had been changed to a shorter size). The rest of Jim's fix
//	can be found in GetWctbColor.

static void 
UseDefaultColor(short index, RGBColor *theColor) {
	switch (index) {
		case wContentColor:			//	 0
		case wTitleBarColor:		//	 4
		case wHiliteColorLight:		//	 5
		case wTitleBarLight:		//	 7
			theColor->red = theColor->green = theColor->blue = 0xFFFF;
			break;
				
		case wDialogLight:			//	 9
		case wTingeLight:			//	11
			theColor->red = theColor->green = 0xCCCC;
			theColor->blue = 0xFFFF;
			break;
		
		case wTingeDark:			//	12
			theColor->red = theColor->green = 0x3333;
			theColor->blue = 0x6666;
			break;

		default:
			theColor->red = theColor->green = theColor->blue = 0;
			break;
	}
}

// -----------------------------------------------------------------------------

static void 
GetWctbColor(WindowPeek window, short partCode, RGBColor *theColor) {
	
	//	Given a partCode, return the RGBColor associated with it. (Using the
	//	default window color table.)
	
	AuxWinHandle awHndl;
	short count;
	
	
	//	Get the Color table for the window if it has one.

	(void) GetAuxWin((WindowPtr) window, &awHndl); 
	count = (**(WCTabHandle) ((**awHndl).awCTable)).ctSize;
	

	//	If the table didn't contain the entry of interest, look to the 
	//	default table.
	
	if (count < partCode) {
		GetAuxWin(nil, &awHndl); 
		count = (**(WCTabHandle) ((**awHndl).awCTable)).ctSize;
	}
			

	//	If the entry is there, use it, if not make a best guess at a default value.

	if (count < partCode)
		UseDefaultColor(partCode, theColor);
	else
		*theColor = (**(WCTabHandle) ((**awHndl).awCTable)).ctTable[partCode].rgb;
}

// -----------------------------------------------------------------------------

void 
WctbForeColor(WindowPeek window, short partCode) {
	RGBColor theColor;

	GetWctbColor(window, partCode, &theColor);
	RGBForeColor(&theColor);
}

// -----------------------------------------------------------------------------

void
WctbBackColor(WindowPeek window, short partCode) {
	RGBColor theColor;

	GetWctbColor(window, partCode, &theColor);
	RGBBackColor(&theColor);
}

// -----------------------------------------------------------------------------
#pragma processor 68020
	//	Note: this is okay because this will only be called if we are
	//	doing System 7 color, which requires Color Quickdraw, which is only
	//	available on systems with 68020's or better. This is done to reduce
	//	code size. If it isn't compiled this way, several routines will be
	//	added to the code WDEF resource to handle the long integer arithmetic.

static void 
MixColor(const RGBColor *light, const RGBColor *dark, 
		 short shade, RGBColor *result) {
	shade = 0x0F - shade;
		//	This is necessary because we give shades between light and
		//	dark (0% is light), but for colors, $0000 is black and $FFFF 
		//	is dark.

	result->red	  = (long) (light->red   - dark->red)   * shade / 15 + dark->red;
	result->green = (long) (light->green - dark->green) * shade / 15 + dark->green;
	result->blue  = (long) (light->blue  - dark->blue)  * shade / 15 + dark->blue;
}

#pragma processor 68000
// -----------------------------------------------------------------------------

static void 
AvgWctbColor(WindowPeek window, short light, short dark, short shade, 
			 RGBColor *theColor) {

	// Mix two parts by the given shade, which is actually a value
	// between 0 (0%) and 15 (100%), return the RGBColor.

	RGBColor lightColor;
	RGBColor darkColor;

	GetWctbColor(window, light, &lightColor);
	GetWctbColor(window, dark, &darkColor);
	MixColor(&lightColor, &darkColor, shade, theColor);
}

// -----------------------------------------------------------------------------

void 
AvgWctbForeColor(WindowPeek window, short light, short dark, short shade) {
	RGBColor theColor;
	
	AvgWctbColor(window, light, dark, shade, &theColor);
	RGBForeColor(&theColor);
}

// -----------------------------------------------------------------------------

void 
AvgWctbBackColor(WindowPeek window, short light, short dark, short shade) {
	RGBColor theColor;
	
	AvgWctbColor(window, light, dark, shade, &theColor);
	RGBBackColor(&theColor);
}

// *****************************************************************************
//	CheckDisplay -- Check to see if we are using color title bars
// -----------------------------------------------------------------------------

static Boolean 
CheckAvailable(WindowPeek window, short light, short dark, 
			   short count, short *ramp) {

	//	Given a light and dark index value, a count, and and an array of
	//	'percentage' values (0x0 to 0xF, or 0 to 15), see if each of the
	//	values in the ramp maps to a different color on the screen. If not,
	//	we need to use black-and-white.
	
	RGBColor theColor;
	short i;
	short colorIndex = 0;
	short lastIndex;
	
	for (i = 0 ; i < count ; i++) {
		AvgWctbColor(window, light, dark, ramp[i], &theColor);	
		
		lastIndex = colorIndex;
		colorIndex = Color2Index(&theColor);
		
		if (i > 0 && colorIndex == lastIndex)	// return false if two entries
			return false;						// have the same index value
	}
	return true;
}

// -----------------------------------------------------------------------------

short 
CheckDisplay(short theDepth, short deviceFlags, 
			 GDHandle targetDevice, WindowPeek window) {
	Boolean		inColor;
	short		result;
	RGBColor	testColor;
	GDHandle	saveDevice;
		
	inColor = HasCQDraw() && (deviceFlags & (0x0001 << gdDevType));
	
	result = blackandwhite;					// assume Black and White

#ifdef NOT_PBOOK_AWARE
	if (theDepth >= 4) {
#else
	if (theDepth >= 4 && (**targetDevice).gdType != fixedType) {
#endif

#ifndef SYS7_OR_LATER
		if (!HasSystem7()) {
			result = sys6color;				// System 6.0.x Color
		} else {
#endif
			GetWctbColor(window, wTingeLight, &testColor);
			if (testColor.red != 0 || testColor.green != 0 || testColor.blue != 0) 
				// check for B&W control panel setting, otherwise:
				result = sys7color;			// System 7.0 Color
#ifndef SYS7_OR_LATER
		}
#endif
	}
	//	Note: Since I didn't find another way to see if the user had changed
	//	the settings in the Color control panel to the Black-and-white setting,
	//	I actually check to see if the rgb components of the light tinge color
	//	are non-zero (which seemed to be the case with that setting). 
	
	
	//	This part checks to see if there are 'enough' colors to draw the 
	//	title bar in color under System 7. It is supposed to do so in the
	//	same way that Apple's system WDEF does. I essentially took the 
	//	assembly code that Apple released and tried to make this use the
	//	same algorithm.

	if (result == sys7color && inColor && theDepth <= 8) {
		short ramp[5];
			// Make sure this array is allocated big enough for the largest ramp.
		
		result = blackandwhite;
		saveDevice = GetGDevice();
		SetGDevice(targetDevice);

		ramp[0] = 0x00;
		ramp[1] = 0x07;
		ramp[2] = 0x08;
		ramp[3] = 0x0A;
		ramp[4] = 0x0D;
		if (CheckAvailable(window, wHiliteColorLight, wHiliteColorDark, 5, ramp)) {
			ramp[0] = 0x00;
			ramp[1] = 0x01;
			ramp[2] = 0x04;
			if (CheckAvailable(window, wTitleBarLight, wTitleBarDark, 3, ramp)) {
				ramp[0] = 0x00;
				ramp[1] = 0x04;
				ramp[2] = 0x0F;
				if (CheckAvailable(window, wTingeLight, wTingeDark, 3, ramp))
					result = sys7color;
			}
		}
		SetGDevice(saveDevice);
	}
	

	return result;
}

// *****************************************************************************
//	Color Utility routines
// -----------------------------------------------------------------------------

void
ColorsNormal(void) {
	ForeColor(blackColor);
	BackColor(whiteColor);
}

// *****************************************************************************
//	General Helper Functions
// -----------------------------------------------------------------------------

void 
FrameBox(const Rect *theRect) {
	Rect tempRect = *theRect;
	
	FrameRect(theRect);
	InsetRect(&tempRect, 1, 1);
	EraseRect(&tempRect);
}

// -----------------------------------------------------------------------------

void
FrameTopLeftShading(Rect theRect) {
	theRect.right--;		// compensate for the way the rectangle hangs
	theRect.bottom--;

	MoveTo(theRect.left,  theRect.bottom);			//	еееее
	LineTo(theRect.left,  theRect.top   );			//	е
	LineTo(theRect.right, theRect.top   );			//	е
}

// -----------------------------------------------------------------------------

void
FrameBottomRightShading(Rect theRect) {
	theRect.right--;		// compensate for the way the rectangle hangs
	theRect.bottom--;

	MoveTo(theRect.left,  theRect.bottom);			//		е
	LineTo(theRect.right, theRect.bottom);			//		е
	LineTo(theRect.right, theRect.top   );			//	еееее
}

// *****************************************************************************

void
GetGlobalMappingPoint(WindowPeek window, Point *thePoint) {

	//	This routine returns a point that gives the horizontal and vertical
	//  offsets needed to map something into global coordinates.

	GrafPtr savePort;
	
	GetPort(&savePort);
	SetPort((GrafPtr) window);
	
	SetPt(thePoint, 0, 0);
	LocalToGlobal(thePoint);
	
	SetPort(savePort);
}

// -----------------------------------------------------------------------------

void 
GetGlobalContentRect(WindowPeek window, Rect *contentRect) {
	Point mappingPoint;

	*contentRect = window->port.portRect;
	GetGlobalMappingPoint(window, &mappingPoint);
	OffsetRect(contentRect, mappingPoint.h, mappingPoint.v);
}

// *****************************************************************************
