/* ZoomCode.c -
	Note: this code is from develop magazine, issue 17. It is referreed by Apple and
	may be freely used in Macintosh programs.
 */
#include "ZoomCode.h"

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

static pascal void CalcWindowAreaOnScreen(short depth, short deviceFlags, GDHandle targetDevice,
									long userData);

short CalculateOffsetAmount(short idealStartPoint, short idealEndPoint,
							short idealOnScreenStartPoint, short idealOnScreenEndPoint,
							short screenEdge1, short screenEdge2);

static RgnHandle GetWindowContentRegion(WindowPeek theWindow);
static RgnHandle GetWindowStructureRegion(WindowPeek theWindow);
static void GetWindowPortRect(WindowPeek theWindow, Rect *portRect);
static void SetWindowStandardState(WindowPeek theWindow, const Rect *standardState);
static void GetWindowUserState(WindowPeek theWindow, Rect *userState);


void ZoomTheWindow(WindowPeek theWindow, short zoomState,
					CalcIdealDocumentSizeProcPtr calcRoutine)
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
	static DeviceLoopDrawingUPP deviceLoopUpdate = nil;
	

	GetPort(&currentPort);
	SetPort((WindowPtr) theWindow);
	contentRegion = GetWindowContentRegion(theWindow);
	structureRegion = GetWindowStructureRegion(theWindow);
	GetWindowPortRect(theWindow, &portRect);
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
	// to display the window�s information.
	mainDevice = GetMainDevice();
	if (zoomState == inZoomOut) {
		zoomData.screenWithLargestPartOfWindow = mainDevice;
		zoomData.largestArea = 0;
	
		// Usually, we would use the content region�s bounding box to determine the monitor
		// with largest portion of the window�s area. However, if the entire content region
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
		if(nil == deviceLoopUpdate){
			deviceLoopUpdate = NewDeviceLoopDrawingProc(CalcWindowAreaOnScreen);
		}
		DeviceLoop(scratchRegion, deviceLoopUpdate, (long) &zoomData,
					(DeviceLoopFlags) singleDevices);
		DisposeRgn(scratchRegion);
		screenRect = (**(zoomData.screenWithLargestPartOfWindow)).gdRect;
		
		// If the monitor being zoomed to is the main monitor, change the top of the
		// useable screen area to avoid putting the title bar underneath the menubar.
		if (zoomData.screenWithLargestPartOfWindow == mainDevice)
			screenRect.top += GetMBarHeight();
			
		// Go figure out the perfect size for the window as if we had an infinitely large
		// screen
		(*calcRoutine)((WindowPtr) theWindow, &newStandardRect);
		
		// Anchor the new rectangle at the window�s current top left corner
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
		
		// If the new rectangle falls off the edge of the screen, nudge it so that it�s just
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
	
		// If we�re still falling off the edge of the screen, that means that the perfect
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

		// We�ve got the best possible window position. Remove the
		// frame, slam it into the WStateData record and let ZoomWindow
		// take care of the rest.
		newStandardRect.top += windowFrameTopSize;
		newStandardRect.left += windowFrameLeftSize;
		newStandardRect.right -= windowFrameRightSize;
		newStandardRect.bottom -= windowFrameBottomSize;
		SetWindowStandardState(theWindow, &newStandardRect);
	}
	else
		GetWindowUserState(theWindow, &newStandardRect);
		
	// If the window is still anchored at the current location, then just resize it
	if ((newStandardRect.left == contentRegionBoundingBox.left) &&
		(newStandardRect.top == contentRegionBoundingBox.top)) {
		OffsetRect(&newStandardRect, -newStandardRect.left, -newStandardRect.top);
		SizeWindow((WindowPtr) theWindow, newStandardRect.right, newStandardRect.bottom,
					true);
	}
	else {
		scratchRegion = NewRgn();
		GetClip(scratchRegion);
		ClipRect(&portRect);
		EraseRect(&portRect);
		ZoomWindow((WindowPtr) theWindow, zoomState, false);
		SetClip(scratchRegion);
		DisposeRgn(scratchRegion);
	}
	
	SetPort(currentPort);
}

static pascal void	CalcWindowAreaOnScreen(short depth, short deviceFlags, GDHandle targetDevice, long userData)
{
	ZoomDataPtr	zoomData;
	long		windowAreaOnScreen;
	Rect		windowPortionOnScreen;

	zoomData = (ZoomDataPtr) userData;

	// Find the rectangle that encloses the intersection of the window and this screen.
	SectRect(&(zoomData->windowBounds), &((**targetDevice).gdRect), &windowPortionOnScreen);
	
	// Offset the rectangle so that it�s right and bottom are also it�s width and height.
	OffsetRect(&windowPortionOnScreen, -windowPortionOnScreen.left, -windowPortionOnScreen.top);
	
	// Calculate the area of the portion of the window that�s on this screen.
	windowAreaOnScreen = (long) windowPortionOnScreen.right * (long) windowPortionOnScreen.bottom;
	
	// If this is the largest portion of the window that has been encountered so far,
	// remember this screen as the potential screen to zoom to.
	if (windowAreaOnScreen > zoomData->largestArea) {
		zoomData->largestArea = windowAreaOnScreen;
		zoomData->screenWithLargestPartOfWindow = targetDevice;
	}
}

// Figure out how much we need to move the window to get it entirely on the monitor.  If
// the window wouldn�t fit completely on the monitor anyway, don�t move it at all; we�ll
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
			// Otherwise, it�s below or to the right of the screen
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

/*
	WindowRecord accessor functions
*/

RgnHandle GetWindowContentRegion(WindowPeek theWindow)
{
	return (theWindow->contRgn);
}

RgnHandle GetWindowStructureRegion(WindowPeek theWindow)
{
	return (theWindow->strucRgn);
}

void GetWindowPortRect(WindowPeek theWindow, Rect *portRect)
{
	*portRect = theWindow->port.portRect;
}

void SetWindowStandardState(WindowPeek theWindow, const Rect *standardState)
{
	(**((WStateDataHandle) theWindow->dataHandle)).stdState = *standardState;
}

void GetWindowUserState(WindowPeek theWindow, Rect *userState)
{
	*userState = (**((WStateDataHandle) theWindow->dataHandle)).userState;
}
