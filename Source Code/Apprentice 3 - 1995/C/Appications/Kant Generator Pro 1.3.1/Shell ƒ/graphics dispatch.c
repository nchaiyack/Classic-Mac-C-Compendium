#include "graphics dispatch.h"
#include "program globals.h"
#if USE_DRAG
#include "drag layer.h"
#endif
#include "window layer.h"
#include "edit functions.h"
#include "text layer.h"
#include "generic window handlers.h"
#include "about.h"
#include "other products window.h"
#include "help.h"
#include "kant main window.h"
#include "kant build window.h"

DispatchError SetupWindowDispatch(short index, WindowRef theWindow)
{
	Boolean			success=FALSE;
#if USE_DRAG
	RgnHandle		hiliteRgn;
#endif
	
	SetWindowIndex(theWindow, index);
	SetWindowTE(theWindow, 0L);
	SetWindowVScrollBar(theWindow, 0L);
	SetWindowHScrollBar(theWindow, 0L);
#if USE_DRAG
	SetWindowIsDraggable(theWindow, FALSE);
	SetWindowHiliteRgn(theWindow, 0L);
#endif
	SetWindowOldClickLoopProc(theWindow, 0L);
	SetWindowDrawGrowIconLines(theWindow, TRUE);
#if USE_PRINTING
	SetWindowIsPrintable(theWindow, FALSE);
#endif
	SetWindowHasPermanentOffscreenWorld(theWindow, FALSE);
	SetWindowPermanentOffscreenWorld(theWindow, 0L);
	SetWindowDepth(theWindow, GetWindowRealDepth(0L));
	SetWindowIsColor(theWindow, GetWindowReallyIsColor(0L));
	SetWindowIsEditable(theWindow, FALSE);
	SetWindowIsZoomable(theWindow, FALSE);
	SetWindowIsZoomed(theWindow, FALSE);
	
	switch (index)
	{
		case kAboutWindow:
			SetupTheAboutWindow(theWindow);
			success=TRUE;
			break;
		case kOtherProductsWindow:
			SetupTheOtherProductsWindow(theWindow);
			success=TRUE;
			break;
		case kHelpWindow:
			SetupTheHelpWindow(theWindow);
			success=TRUE;
			break;
		case kMainWindow:
			SetupTheMainWindow(theWindow);
			success=TRUE;
			break;
		case kBuildWindow:
			SetupTheBuildWindow(theWindow);
			success=TRUE;
			break;
	}
	
#if USE_DRAG
	if ((DragManagerAvailableQQ()) && (success) && (WindowIsDraggableQQ(theWindow)))
	{
		hiliteRgn=NewRgn();
		SetWindowHiliteRgn(theWindow, hiliteRgn);
		SetWindowDragTrackingHandler(theWindow, NewDragTrackingHandlerProc(MyTrackingHandler));
		SetWindowDragReceiveHandler(theWindow, NewDragReceiveHandlerProc(MyReceiveDropHandler));
		InstallTrackingHandler((DragTrackingHandler)GetWindowDragTrackingHandler(theWindow),
			theWindow, (void *)theWindow);
		InstallReceiveHandler((DragReceiveHandler)GetWindowDragReceiveHandler(theWindow),
			theWindow, (void *)theWindow);
	}
#endif
	
	return success ? kSuccess : kFailure;
}

DispatchError ShutdownWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			ShutDownTheAboutWindow();
			return kSuccess;
		case kOtherProductsWindow:
			ShutDownTheOtherProductsWindow();
			return kSuccess;
		case kHelpWindow:
			ShutDownTheHelpWindow();
			return kSuccess;
		case kMainWindow:
			ShutDownTheMainWindow();
			return kSuccess;
		case kBuildWindow:
			ShutDownTheBuildWindow();
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError OpenWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			OpenTheAboutWindow(theWindow);
			return kSuccess;
		case kOtherProductsWindow:
			OpenTheOtherProductsWindow(theWindow);
			return kSuccess;
		case kHelpWindow:
			OpenTheHelpWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			OpenTheMainWindow(theWindow);
			return kSuccess;
		case kBuildWindow:
			OpenTheBuildWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError CloseWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			if (CloseTheMainWindow(theWindow))
				return kSuccess;
			else
				return kCancel;
	}
	
	return kFailure;
}

DispatchError DisposeWindowDispatch(WindowRef theWindow)
{
	Boolean			success=FALSE;
	RgnHandle		hiliteRgn;
	
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			DisposeTheAboutWindow(theWindow);
			success=TRUE;
			break;
		case kOtherProductsWindow:
			DisposeTheOtherProductsWindow(theWindow);
			success=TRUE;
			break;
		case kHelpWindow:
			DisposeTheHelpWindow(theWindow);
			success=TRUE;
			break;
		case kMainWindow:
			DisposeTheMainWindow(theWindow);
			success=TRUE;
			break;
		case kBuildWindow:
			DisposeTheBuildWindow(theWindow);
			success=TRUE;
			break;
	}
	
#if USE_DRAG
	if ((DragManagerAvailableQQ()) && (success) && (WindowIsDraggableQQ(theWindow)))
	{
		RemoveTrackingHandler((DragTrackingHandler)GetWindowDragTrackingHandler(theWindow),
			theWindow);
		RemoveReceiveHandler((DragReceiveHandler)GetWindowDragReceiveHandler(theWindow),
			theWindow);
		DisposeRoutineDescriptor(GetWindowDragTrackingHandler(theWindow));
		DisposeRoutineDescriptor(GetWindowDragReceiveHandler(theWindow));
		hiliteRgn=GetWindowHiliteRgn(theWindow);
		DisposeRgn(hiliteRgn);
		SetWindowHiliteRgn(theWindow, 0L);
	}
#endif
	
	return success ? kSuccess : kFailure;
}

DispatchError ChangeDepthDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			ChangeDepthTheAboutWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError DrawWindowDispatch(WindowRef theWindow, short theDepth)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			DrawTheAboutWindow(theWindow, theDepth);
			return kSuccess;
		case kHelpWindow:
			DrawTheHelpWindow(theWindow, theDepth);
			return kSuccess;
		case kBuildWindow:
			DrawTheBuildWindow(theWindow, theDepth);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError CopybitsDispatch(WindowRef theWindow, WindowRef offscreenWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			CopybitsTheAboutWindow(theWindow, offscreenWindow);
			return kSuccess;
		case kOtherProductsWindow:
		case kMainWindow:
			GenericCopybits(theWindow, offscreenWindow, WindowIsActiveQQ(theWindow),
				WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
		case kBuildWindow:
			CopybitsTheBuildWindow(theWindow, offscreenWindow, WindowIsActiveQQ(theWindow),
				WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
			break;
	}
	
	return kFailure;
}

DispatchError IdleWindowDispatch(WindowRef theWindow, Point mouseLoc)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			IdleInTheAboutWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			GenericIdleInWindow(theWindow, mouseLoc);
			return kSuccess;
		case kBuildWindow:
			IdleInBuildWindow(theWindow, mouseLoc);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError ActivateWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericActivate(theWindow, WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
		case kBuildWindow:
			ActivateTheBuildWindow(theWindow, WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError DeactivateWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericDeactivate(theWindow);
			return kSuccess;
		case kBuildWindow:
			DeactivateTheBuildWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError GrowWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericResizeControls(theWindow, 0, 0, FALSE);
			return kSuccess;
		case kBuildWindow:
			ResizeTheBuildWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError ZoomWindowDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericResizeControls(theWindow, 0, 0, FALSE);
			return kSuccess;
		case kBuildWindow:
			ResizeTheBuildWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError GetGrowSizeDispatch(WindowRef theWindow, Rect *sizeRect)
{
	switch (GetWindowIndex(theWindow))
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericGetGrowSize(theWindow, sizeRect);
			return kSuccess;
		case kBuildWindow:
			GetGrowSizeTheBuildWindow(theWindow, sizeRect);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError KeyDownDispatch(WindowRef theWindow, unsigned char theChar)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			KeyDownInAboutWindow(theWindow, theChar);
			return kSuccess;
		case kOtherProductsWindow:
			GenericKeyPressedInWindow(theWindow, theChar, FALSE, FALSE);
			return kSuccess;
		case kHelpWindow:
			KeyPressedInHelpWindow(theWindow, theChar);
			return kSuccess;
		case kMainWindow:
			if (!GenericKeyPressedInWindow(theWindow, theChar, TRUE, FALSE))
				KeyPressedInMainWindow(theWindow, theChar);
			return kSuccess;
		case kBuildWindow:
			KeyPressedInBuildWindow(theWindow, theChar);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError MouseDownDispatch(WindowRef theWindow, Point thePoint)
{
	switch (GetWindowIndex(theWindow))
	{
		case kAboutWindow:
			MouseDownInAboutWindow(theWindow, thePoint);
			return kSuccess;
		case kOtherProductsWindow:
			GenericMouseClickedInWindow(theWindow, thePoint, FALSE, gDynamicScroll, FALSE);
			return kSuccess;
		case kHelpWindow:
			MouseClickedInHelpWindow(theWindow, thePoint);
			return kSuccess;
		case kMainWindow:
			GenericMouseClickedInWindow(theWindow, thePoint, TRUE, gDynamicScroll, FALSE);
			return kSuccess;
		case kBuildWindow:
			MouseClickedInBuildWindow(theWindow, thePoint, gDynamicScroll);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError UndoDispatch(WindowRef theWindow)
{
	#pragma unused(theWindow)

	return kFailure;
}

DispatchError CutDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			GenericCut(theWindow);
			return kSuccess;
			break;
	}
	
	return kFailure;
}

DispatchError CopyDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			GenericCopy(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError PasteDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			PasteInMainWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError ClearDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			GenericClear(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

DispatchError SelectAllDispatch(WindowRef theWindow)
{
	switch (GetWindowIndex(theWindow))
	{
		case kMainWindow:
			GenericSelectAll(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}
