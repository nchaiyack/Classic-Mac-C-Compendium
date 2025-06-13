//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow.cp
//|
//| This file contains the implementation of an enhanced window.  This window
//| is more aware of multiple screens that a normal window, and can place
//| and zoom itself intelligently depending on the screen setup.
//|___________________________________________________________________________

#include "CEnhancedWindow.h"
#include <limits.h>



//=============================== Procedure Prototypes ===============================\\

pascal void CalcWindowAreaOnScreen(short depth, short deviceFlags, GDHandle targetDevice,
									long userData);
short CalculateOffsetAmount(short idealStartPoint, short idealEndPoint,
							short idealOnScreenStartPoint, short idealOnScreenEndPoint,
							short screenEdge1, short screenEdge2);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::IEnhancedWindow
//|
//| Purpose: Initialize the window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CEnhancedWindow::IEnhancedWindow(short WINDid, Boolean aFloating,
						CDesktop *anEnclosure, CDirector *aSupervisor)
{

	inherited::IWindow(WINDid, aFloating, anEnclosure, aSupervisor);

	title_bar_height = 18;

}	//==== CEnhancedWindow::IEnhancedWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::Zoom
//|
//| Purpose: Zoom the window.  This code is from Develop magazine, issue 17.
//|
//| Parameters: passed to superclass
//|__________________________________________________________________________

		struct ZoomData {
			GDHandle		screenWithLargestPartOfWindow;
			unsigned long	largestArea;
			Rect			windowBounds;
		};
		typedef struct ZoomData ZoomData, *ZoomDataPtr;
		
		enum {
			kNudgeSlop	=	4,
			kIconSpace	=	64
		};

void CEnhancedWindow::Zoom(short zoomState)
{

	ZoomData	zoomData;
	Rect		newStandardRect;
	Rect		scratchRect;
	Rect		screenRect;
	Rect		portRect;
	Rect		contentRegionBoundingBox;
	Rect		structureRegionBoundingBox;
	Rect		deviceLoopRect;
	GrafPtr		currentPort;
	RgnHandle	scratchRegion;
	RgnHandle	contentRegion;
	RgnHandle	structureRegion;
	GDHandle	mainDevice;
	short		horizontalAmountOffScreen;
	short		verticalAmountOffScreen;
	short		windowFrameTopSize;
	short		windowFrameLeftSize;
	short		windowFrameRightSize;
	short		windowFrameBottomSize;
	

	GetPort(&currentPort);
	Prepare();
	contentRegion = ((CWindowPeek) GetMacPort())->contRgn;
	structureRegion = ((CWindowPeek) GetMacPort())->strucRgn;
	portRect = ((CWindowPeek) GetMacPort())->port.portRect;
	contentRegionBoundingBox = (**contentRegion).rgnBBox;
	structureRegionBoundingBox = (**structureRegion).rgnBBox;
	
	// Determine the size of the window frame
	windowFrameTopSize = contentRegionBoundingBox.top - 
									structureRegionBoundingBox.top;
	windowFrameLeftSize = contentRegionBoundingBox.left - 
									structureRegionBoundingBox.left;
	windowFrameRightSize = structureRegionBoundingBox.right - 
									contentRegionBoundingBox.right;
	windowFrameBottomSize = structureRegionBoundingBox.bottom - 
									contentRegionBoundingBox.bottom;
									
	// If the window is being zoomed into the standard state, calculate the best size
	// to display the window’s information.
	mainDevice = GetMainDevice();
	if (zoomState == inZoomOut) {
		zoomData.screenWithLargestPartOfWindow = mainDevice;
		zoomData.largestArea = 0;
	
		// Usually, we would use the content region’s bounding box to determine the monitor
		// with largest portion of the window’s area. However, if the entire content region
		// of the window is not on any screen, the structure region should be used instead.
		scratchRegion = NewRgn();
		SectRgn(GetGrayRgn(), contentRegion, scratchRegion);
		if (EmptyRgn(scratchRegion))
			zoomData.windowBounds = structureRegionBoundingBox;
		else
			zoomData.windowBounds = contentRegionBoundingBox;
	
		// Use DeviceLoop to walk through all the active screens to find the one with the
		// largest portion of the zoomed window
		deviceLoopRect = zoomData.windowBounds;
		GlobalToLocal((Point *)&deviceLoopRect);
		GlobalToLocal((Point *)&deviceLoopRect.bottom);
		RectRgn(scratchRegion, &deviceLoopRect);
		DeviceLoop(scratchRegion, &CalcWindowAreaOnScreen, (long) &zoomData,
					(DeviceLoopFlags) singleDevices);
		DisposeRgn(scratchRegion);
		screenRect = (**(zoomData.screenWithLargestPartOfWindow)).gdRect;
		
		// If the monitor being zoomed to is the main monitor, change the top of the
		// useable screen area to avoid putting the title bar underneath the menubar.
		if (zoomData.screenWithLargestPartOfWindow == mainDevice)
			screenRect.top += GetMBarHeight();
			
		// Go figure out the perfect size for the window as if we had an infinitely large
		// screen
		FindIdealSize(&newStandardRect);
		
		// Anchor the new rectangle at the window’s current top left corner
		OffsetRect(&newStandardRect, -newStandardRect.left, -newStandardRect.top);
		OffsetRect(&newStandardRect, contentRegionBoundingBox.left,
					contentRegionBoundingBox.top);
		
		// newStandardRect is the ideal size for the content area. The window frame
		// needs to be accounted for when we see if the window needs to be moved,
		// or resized, so add in the dimensions of the window frame.
		newStandardRect.top -= windowFrameTopSize;
		newStandardRect.left -= windowFrameLeftSize;
		newStandardRect.right += windowFrameRightSize;
		newStandardRect.bottom += windowFrameBottomSize;
		
		// If the new rectangle falls off the edge of the screen, nudge it so that it’s just
		// on the screen. CalculateOffsetAmount determines how much of the window is offscreen.
		SectRect(&newStandardRect, &screenRect, &scratchRect);
		if (!EqualRect(&newStandardRect, &scratchRect)) {
			horizontalAmountOffScreen = CalculateOffsetAmount(newStandardRect.left,
															   newStandardRect.right,
															   scratchRect.left,
															   scratchRect.right,
															   screenRect.left,
															   screenRect.right);
			verticalAmountOffScreen = CalculateOffsetAmount(newStandardRect.top,
															newStandardRect.bottom,
															scratchRect.top,
															scratchRect.bottom,
															screenRect.top,
															screenRect.bottom);
			OffsetRect(&newStandardRect, horizontalAmountOffScreen,
						verticalAmountOffScreen);
		}
	
		// If we’re still falling off the edge of the screen, that means that the perfect
		// size is larger than the screen, so we need to shrink down the standard size
		SectRect(&newStandardRect, &screenRect, &scratchRect);
		if (!EqualRect(&newStandardRect, &scratchRect)) {

		// First shrink the width of the window. If the window is wider than the screen
		// it is zooming to, we can just pin the standard rectangle to the edges of the
		// screen, leaving some slop. If the window is narrower than the screen, we know
		// we just nudged it into position, so nothing needs to be done.
			if ((newStandardRect.right - newStandardRect.left) >
				(screenRect.right - screenRect.left)) {
				newStandardRect.left = screenRect.left + kNudgeSlop;
				newStandardRect.right = screenRect.right - kNudgeSlop;

				if ((zoomData.screenWithLargestPartOfWindow == mainDevice) &&
					(newStandardRect.right > (screenRect.right - kIconSpace)))
					newStandardRect.right = screenRect.right - kIconSpace;
			}

			// Move in the top. Like the width of the window, nothing needs to be done unless
			// the window is taller than the height of the screen.
			if ((newStandardRect.bottom - newStandardRect.top) >
				(screenRect.bottom - screenRect.top)) {
				newStandardRect.top = screenRect.top + kNudgeSlop;
				newStandardRect.bottom = screenRect.bottom - kNudgeSlop;
			}
		}

		// We’ve got the best possible window position. Remove the
		// frame, slam it into the WStateData record and let ZoomWindow
		// take care of the rest.
		newStandardRect.top += windowFrameTopSize;
		newStandardRect.left += windowFrameLeftSize;
		newStandardRect.right -= windowFrameRightSize;
		newStandardRect.bottom -= windowFrameBottomSize;
		(**((WStateDataHandle) ((CWindowPeek) GetMacPort())->dataHandle)).stdState = newStandardRect;
		//  the above line sets the standard state
	}
	else
		newStandardRect = (**((WStateDataHandle) ((CWindowPeek) GetMacPort())->dataHandle)).userState;
		//  the above line puts the user state in newStandardRect
		
	// If the window is still anchored at the current location, then just resize it
	if ((newStandardRect.left == contentRegionBoundingBox.left) &&
		(newStandardRect.top == contentRegionBoundingBox.top)) {
		OffsetRect(&newStandardRect, -newStandardRect.left, -newStandardRect.top);
		ChangeSize(newStandardRect.right, newStandardRect.bottom);
		
	}
	else {
		scratchRegion = NewRgn();
		GetClip(scratchRegion);
		ClipRect(&portRect);
		EraseRect(&portRect);
		inherited::Zoom(zoomState);				//  Zoom, using superclass
		SetClip(scratchRegion);
		DisposeRgn(scratchRegion);
	}
	
	SetPort(currentPort);

}	//==== CEnhancedWindow:: Zoom() ====\\



pascal void	CalcWindowAreaOnScreen(short depth, short deviceFlags, GDHandle targetDevice, long userData)
{

	ZoomDataPtr	zoomData = (ZoomDataPtr) userData;
	long		windowAreaOnScreen;
	Rect		windowPortionOnScreen;
	
	// Find the rectangle that encloses the intersection of the window and this screen.
	SectRect(&(zoomData->windowBounds), &((**targetDevice).gdRect), &windowPortionOnScreen);
	
	// Offset the rectangle so that it’s right and bottom are also it’s width and height.
	OffsetRect(&windowPortionOnScreen, -windowPortionOnScreen.left, -windowPortionOnScreen.top);
	
	// Calculate the area of the portion of the window that’s on this screen.
	windowAreaOnScreen = (long) windowPortionOnScreen.right * (long) windowPortionOnScreen.bottom;
	
	// If this is the largest portion of the window that has been encountered so far,
	// remember this screen as the potential screen to zoom to.
	if (windowAreaOnScreen > zoomData->largestArea) {
		zoomData->largestArea = windowAreaOnScreen;
		zoomData->screenWithLargestPartOfWindow = targetDevice;
	}
}

// Figure out how much we need to move the window to get it entirely on the monitor.  If
// the window wouldn’t fit completely on the monitor anyway, don’t move it at all; we’ll
// make it fit later on.

short CalculateOffsetAmount(short idealStartPoint, short idealEndPoint, short idealOnScreenStartPoint,
							short idealOnScreenEndPoint, short screenEdge1, short screenEdge2)
{
	short	offsetAmount;

	// First check to see if the window fits on the screen in this dimension.
	if ((idealStartPoint < screenEdge1) && (idealEndPoint > screenEdge2))
		offsetAmount = 0;
	else {
	
		// Find out how much of the window lies off this screen by subtracting the amount of the window
		// that is on the screen from the size of the entire window in this dimension. If the window
		// is completely offscreen, the offset amount is going to be the distance from the ideal
		// starting point to the first edge of the screen.
		if ((idealOnScreenStartPoint - idealOnScreenEndPoint) == 0) {
			// See if the window is lying to the left or above the screen
			if (idealEndPoint < screenEdge1)
				offsetAmount = screenEdge1 - idealStartPoint + kNudgeSlop;
			else
			// Otherwise, it’s below or to the right of the screen
				offsetAmount = screenEdge2 - idealEndPoint - kNudgeSlop;
		}
		else {
			// Window is already partially or completely on the screen
			offsetAmount = (idealEndPoint - idealStartPoint) -
							(idealOnScreenEndPoint - idealOnScreenStartPoint);
	
			// If we are offscreen a little, move the window in a few more pixels from the edge of the screen.
			if (offsetAmount != 0)
				offsetAmount += kNudgeSlop;
			
			// Check to see which side of the screen the window was falling off of, so that it can be
			// nudged in the opposite direction.
			if (idealEndPoint > screenEdge2)
				offsetAmount = -offsetAmount;
		}
	}
	
	return offsetAmount;
}



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::FindIdealSize
//|
//| Purpose: Compute ideal size for this window.overridden
//|
//| Parameters: rect: receives the ideal window size
//|__________________________________________________________________________

void CEnhancedWindow::FindIdealSize(Rect *rect)
{

	rect->left = 0;					//  Default makes a square 500 by 500 window
	rect->right = 500;
	rect->top = 0;
	rect->bottom = 500;

}	//==== CEnhancedWindow::FindIdealSize() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::MakeFullScreen
//|
//| Purpose: Resizes the window to fill the screen
//|
//| Parameters: none
//|___________________________________________________________________________

void CEnhancedWindow::MakeFullScreen(void)
{
	
	Move(0, 0);											//  Move the window to the very top left.  Note that
														//   this moved the structure offscreen so only the
														//   contents is visible, and that even the top of
														//   the contents will be obscured unless the menu bar
														//   is hidden
	
	Rect main_monitor_bounds;
	GDHandle graphics_device;
	graphics_device = GetDeviceList ();					//  Get the first (main) device in the list
	main_monitor_bounds = (*graphics_device)->gdRect;	//  Get the main device bounds rect

	ChangeSize(main_monitor_bounds.right -				//  Resize the window to fill the screen
					main_monitor_bounds.left,
				main_monitor_bounds.bottom -
					main_monitor_bounds.top);

}	//==== CEnhancedWindow::MakeFullScreen() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::Place
//|
//| Purpose: Place the window according to bounds.
//|
//| Parameters: bounds: the desired position of the window
//|__________________________________________________________________________

void CEnhancedWindow::Place(Rect *bounds)
{
	
	Move(bounds->left, bounds->top);				//  Move the window
	ChangeSize(bounds->right - bounds->left,		//  Resize the window
				bounds->bottom - bounds->top);

}	//==== CEnhancedWindow::Place() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::GetRect
//|
//| Purpose: Get the size and position of a window
//|
//| Parameters: window_rect: receives the current window position
//|___________________________________________________________________________

void CEnhancedWindow::GetRect(Rect *window_rect)
{

	Point upper_left;
	upper_left.h = upper_left.v = 0;							//  Find upper left corner
	Prepare();
	LocalToGlobal(&upper_left);

	LongRect window_interior;									//  Find height and width
	GetInterior(&window_interior);
	
	window_rect->left = window_interior.left + upper_left.h;		//  Generate bounding rect
	window_rect->right = window_interior.right + upper_left.h;
	window_rect->top = window_interior.top + upper_left.v;
	window_rect->bottom = window_interior.bottom + upper_left.v;
	
}	//==== CEnhancedWindow::GetRect() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow::PlaceWithVerify
//|
//| Purpose: Place the window according to bounds.  If this would result in
//|          the window being placed offscreen, move it onscreen.
//|
//| Parameters: bounds: the desired position of the window
//|__________________________________________________________________________

void CEnhancedWindow::PlaceWithVerify(Rect *bounds)
{

	Rect main_monitor_bounds;
	GDHandle graphics_device;
	graphics_device = GetDeviceList();							//  Get the first (main) device in the list
	main_monitor_bounds = (*graphics_device)->gdRect;			//  Save the main device bounds rect
	main_monitor_bounds.top += GetMBarHeight();					//  Shrink screen by menubar size

	Rect drag_rect = *bounds;
	drag_rect.bottom = drag_rect.top;
	drag_rect.top -= title_bar_height;							//  Get the drag rectangle for this window.

	Boolean enough_intersection;
	do															//  Scan through all devices
		{
		Rect intersection;
		Rect this_monitor_rect;
		
		this_monitor_rect = (*graphics_device)->gdRect;			//  Get the rect for this monitor

		if (graphics_device == GetMainDevice())					//  Shrink screen by size of menubar
			this_monitor_rect.top += GetMBarHeight();

		SectRect (&drag_rect, &this_monitor_rect,				//  If this device intersects the drag rectangle
					&intersection);								//   in a 4x4 block, we're okay.
		enough_intersection = 	
				(((intersection.bottom - intersection.top) >= 4) &&
				((intersection.right - intersection.left) >= 4));

		graphics_device =										//  Next device
				(GDHandle) (*graphics_device)->gdNextGD;
		}
	while (graphics_device && (!enough_intersection));

	if (!enough_intersection)									//  This window is not draggable from any
																//  monitor; move it to the main monitor.
		{
		
		const short kSlopFactor = 4;

		short window_width = bounds->right - bounds->left;		//  Find size of window
		short window_height = bounds->bottom - bounds->top;
		
		short main_monitor_width = main_monitor_bounds.right -
									main_monitor_bounds.left;	//  Find size of main monitor
		short main_monitor_height =
								main_monitor_bounds.bottom -
									main_monitor_bounds.top;
		
		if (window_width > main_monitor_width)					
			window_width = main_monitor_width - 2*kSlopFactor;	//  Fit window to monitor, horizontally
		
		if (window_height > main_monitor_height)
			window_height = main_monitor_height - 2*kSlopFactor;//  Fit window to monitor, vertically

		if (bounds->left > main_monitor_bounds.right)
			OffsetRect(bounds, -(bounds->left -
						main_monitor_bounds.right +
							window_width + kSlopFactor), 0);	//  Nudge barely on screen, to left
		
		if (bounds->right < main_monitor_bounds.left)
			OffsetRect(bounds, main_monitor_bounds.left -
						bounds->left + kSlopFactor,
						0);										//  Nudge barely on screen, to right

		if (bounds->top + kSlopFactor < main_monitor_bounds.top)
			OffsetRect(bounds, 0, main_monitor_bounds.top -
				bounds->top + title_bar_height + kSlopFactor);	//  Nudge barely on screen, down

		if (bounds->top - title_bar_height - kSlopFactor > 
								main_monitor_bounds.bottom)
			OffsetRect(bounds, 0, -(bounds->top -
							main_monitor_bounds.bottom +
								window_height + kSlopFactor));	//  Nudge barely on screen, up

		}

	Place(bounds);												//  Place the window

}	//==== CEnhancedWindow::PlaceWithVerify() ====\\