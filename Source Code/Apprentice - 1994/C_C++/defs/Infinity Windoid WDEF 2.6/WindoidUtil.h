// *****************************************************************************
//
//	WindoidUtil.h
//
// *****************************************************************************
#ifndef __WindoidUtil__
#define __WindoidUtil__

// -----------------------------------------------------------------------------

#ifndef __WindoidDefines__
#include "WindoidDefines.h"
#endif

// *****************************************************************************
//	Macros to make code cleaner
// -----------------------------------------------------------------------------

#define IsOdd(value) ((value) & 1)
#define IsEven(value) (!IsOdd(value))

// *****************************************************************************
//	Prototypes
// -----------------------------------------------------------------------------

Boolean HasSystem7(void);

Boolean HasCQDraw(void);

// -----------------------------------------------------------------------------

void SyncPorts(void);

// -----------------------------------------------------------------------------

#ifdef SYS7_OR_LATER
	#define OurDeviceLoop(drawingRgn, drawingProc, userData, flags)	\
			   DeviceLoop(drawingRgn, drawingProc, userData, flags)
#else
	#ifdef UNIV_HEADERS
		void OurDeviceLoop(RgnHandle drawingRgn, DeviceLoopDrawingUPP drawingProc,
						   long userData, DeviceLoopFlags flags);
	#else
		void OurDeviceLoop(RgnHandle drawingRgn, DeviceLoopDrawingProcPtr drawingProc,
						   long userData, DeviceLoopFlags flags);
	#endif
#endif

// -----------------------------------------------------------------------------

void WctbForeColor(WindowPeek window, short partCode);

void WctbBackColor(WindowPeek window, short partCode);

// -----------------------------------------------------------------------------

void AvgWctbForeColor(WindowPeek window, short light, short dark, short shade);

void AvgWctbBackColor(WindowPeek window, short light, short dark, short shade);

// -----------------------------------------------------------------------------

void SwapForeBackColor(short variation);

void ColorsNormal(void);

// -----------------------------------------------------------------------------

void FrameBox(const Rect *theRect);

void FrameTopLeftShading(Rect theRect);

void FrameBottomRightShading(Rect theRect);

// -----------------------------------------------------------------------------

void GetGlobalMappingPoint(WindowPeek window, Point *thePoint);

void GetGlobalContentRect(WindowPeek window, Rect *contentRect);

// -----------------------------------------------------------------------------

short CheckDisplay(short theDepth, short deviceFlags, 
				   GDHandle targetDevice, WindowPeek window);

// *****************************************************************************
#endif