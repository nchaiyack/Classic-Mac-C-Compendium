#include "graphics dispatch.h"
#include "drag utilities.h"
#include "window layer.h"
#include "program globals.h"
#include "edit functions.h"
#include "text twiddling.h"
#include "generic window handlers.h"
#include "about.h"
#include "other products window.h"
#include "help.h"
#include "brlr main window.h"
#include "brlr floating window.h"

enum DispatchError SetupWindowDispatch(short index, WindowPtr theWindow)
{
	Boolean			success=FALSE;
	RgnHandle		hiliteRgn;
	
	SetWindowIndex(theWindow, index);
	SetWindowTE(theWindow, 0L);
	SetWindowVScrollBar(theWindow, 0L);
	SetWindowHScrollBar(theWindow, 0L);
	SetWindowIsDraggable(theWindow, FALSE);
	SetWindowHiliteRgn(theWindow, 0L);
	SetWindowOldClickLoopProc(theWindow, 0L);
	SetWindowDrawGrowIconLines(theWindow, TRUE);
	SetWindowIsPrintable(theWindow, FALSE);
	
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
		case kFloatingWindow:
			SetupTheFloatingWindow(theWindow);
			success=TRUE;
			break;
	}
	
	if ((DragManagerAvailableQQ()) && (success) && (WindowIsDraggableQQ(theWindow)))
	{
		hiliteRgn=NewRgn();
		SetWindowHiliteRgn(theWindow, hiliteRgn);
		InstallTrackingHandler((DragTrackingHandler)MyTrackingHandler, theWindow, (void *)theWindow);
		InstallReceiveHandler((DragReceiveHandler)MyReceiveDropHandler, theWindow, (void *)theWindow);
	}
	
	return success ? kSuccess : kFailure;
}

enum DispatchError ShutdownWindowDispatch(short index)
{
	switch (index)
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
	}
	
	return kFailure;
}

enum DispatchError OpenWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
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
		case kFloatingWindow:
			OpenTheFloatingWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError CloseWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			if (CloseTheMainWindow(theWindow))
				return kSuccess;
			else
				return kCancel;
		case kFloatingWindow:
			CloseTheFloatingWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError DisposeWindowDispatch(short index)
{
	WindowPtr		theWindow;
	Boolean			success=FALSE;
	RgnHandle		hiliteRgn;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
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
		case kFloatingWindow:
			DisposeTheFloatingWindow(theWindow);
			success=TRUE;
			break;
	}
	
	if ((DragManagerAvailableQQ()) && (success) && (WindowIsDraggableQQ(theWindow)))
	{
		RemoveTrackingHandler((DragTrackingHandler)MyTrackingHandler, theWindow);
		RemoveReceiveHandler((DragReceiveHandler)MyReceiveDropHandler, theWindow);
		hiliteRgn=GetWindowHiliteRgn(theWindow);
		DisposeRgn(hiliteRgn);
		SetWindowHiliteRgn(theWindow, 0L);
	}
	
	return success ? kSuccess : kFailure;
}

enum DispatchError ChangeDepthDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			ChangeDepthTheAboutWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError DrawWindowDispatch(short index, short theDepth)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			DrawTheAboutWindow(theWindow, theDepth);
			return kSuccess;
		case kHelpWindow:
			DrawTheHelpWindow(theWindow, theDepth);
			return kSuccess;
		case kFloatingWindow:
			DrawTheFloatingWindow(theWindow, theDepth);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError CopybitsDispatch(short index, WindowPtr offscreenWindow)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			CopybitsTheAboutWindow(theWindow, offscreenWindow);
			return kSuccess;
		case kOtherProductsWindow:
		case kMainWindow:
			GenericCopybits(theWindow, offscreenWindow, WindowIsActiveQQ(theWindow),
				WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError IdleWindowDispatch(short index, Point mouseLoc)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			IdleInTheAboutWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			IdleInMainWindow(theWindow, mouseLoc);
			GenericIdleInWindow(theWindow, mouseLoc);
			return kSuccess;
		case kFloatingWindow:
			GenericIdleInWindow(theWindow, mouseLoc);
			return kPassThrough;
	}
	
	return kFailure;
}

enum DispatchError ActivateWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kOtherProductsWindow:
		case kFloatingWindow:
			GenericActivate(theWindow, WindowDrawGrowIconLinesQQ(theWindow));
			return kSuccess;
		case kMainWindow:
			GenericActivate(theWindow, WindowDrawGrowIconLinesQQ(theWindow));
			ActivateTheMainWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError DeactivateWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kOtherProductsWindow:
		case kFloatingWindow:
			GenericDeactivate(theWindow);
			return kSuccess;
		case kMainWindow:
			GenericDeactivate(theWindow);
			DeactivateTheMainWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError GrowWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericResizeControls(theWindow, 0, 0, FALSE);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError ZoomWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericResizeControls(theWindow, 0, 0, FALSE);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError GetGrowSizeDispatch(short index, Rect *sizeRect)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kOtherProductsWindow:
		case kMainWindow:
			GenericGetGrowSize(theWindow, sizeRect);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError KeyDownDispatch(short index, unsigned char theChar)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
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
		case kFloatingWindow:
			if (!KeyPressedInFloatingWindow(theWindow, theChar))
				return kPassThrough;
			else
				return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError MouseDownDispatch(short index, Point thePoint)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			MouseDownInAboutWindow(theWindow, thePoint);
			return kSuccess;
		case kHelpWindow:
			MouseClickedInHelpWindow(theWindow, thePoint);
			return kSuccess;
		case kOtherProductsWindow:
			GenericMouseClickedInWindow(theWindow, thePoint, FALSE, gDynamicScroll, FALSE);
			return kSuccess;
		case kMainWindow:
		case kFloatingWindow:
			GenericMouseClickedInWindow(theWindow, thePoint, TRUE, gDynamicScroll, FALSE);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError UndoDispatch(short index)
{
	switch (index)
	{
	}
	
	return kFailure;
}

enum DispatchError CutDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			GenericCut(theWindow);
			return kSuccess;
			break;
		case kFloatingWindow:
			return kPassThrough;
			break;
	}
	
	return kFailure;
}

enum DispatchError CopyDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			GenericCopy(theWindow);
			return kSuccess;
		case kFloatingWindow:
			return kPassThrough;
			break;
	}
	
	return kFailure;
}

enum DispatchError PasteDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			GenericPaste(theWindow);
			return kSuccess;
		case kFloatingWindow:
			return kPassThrough;
			break;
	}
	
	return kFailure;
}

enum DispatchError ClearDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			GenericClear(theWindow);
			return kSuccess;
		case kFloatingWindow:
			return kPassThrough;
			break;
	}
	
	return kFailure;
}

enum DispatchError SelectAllDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kMainWindow:
			GenericSelectAll(theWindow);
			return kSuccess;
		case kFloatingWindow:
			return kPassThrough;
			break;
	}
	
	return kFailure;
}

enum DispatchError ReceiveDragDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kFloatingWindow:
			ReceiveDragInFloatingWindow(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}
