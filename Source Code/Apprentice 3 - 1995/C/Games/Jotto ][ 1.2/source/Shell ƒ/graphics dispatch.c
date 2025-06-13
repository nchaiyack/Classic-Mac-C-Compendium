#include "graphics dispatch.h"
#include "window layer.h"
#include "program globals.h"
#include "edit functions.h"
#include "text twiddling.h"
#include "generic window handlers.h"
#include "about.h"
#include "about MSG.h"
#include "other MSG window.h"
#include "help.h"
#include "jotto main window.h"
#include "jotto note pad.h"

enum DispatchError SetupWindowDispatch(short index, WindowPtr theWindow)
{
	SetWindowIndex(theWindow, index);
	SetWindowTE(theWindow, 0L);
	SetWindowVScrollBar(theWindow, 0L);
	SetWindowHScrollBar(theWindow, 0L);
	
	switch (index)
	{
		case kAboutWindow:
			SetupTheAboutWindow(theWindow);
			return kSuccess;
		case kAboutMSGWindow:
			SetupTheAboutMSGWindow(theWindow);
			return kSuccess;
		case kOtherMSGWindow:
			SetupTheOtherMSGWindow(theWindow);
			return kSuccess;
		case kHelpWindow:
			SetupTheHelpWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			SetupTheMainWindow(theWindow);
			return kSuccess;
		case kNotePad:
			SetupTheNotePad(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError ShutdownWindowDispatch(short index)
{
	switch (index)
	{
		case kAboutWindow:
			ShutDownTheAboutWindow();
			return kSuccess;
		case kAboutMSGWindow:
			ShutDownTheAboutMSGWindow();
			return kSuccess;
		case kHelpWindow:
			ShutDownTheHelpWindow();
			return kSuccess;
		case kMainWindow:
			ShutDownTheMainWindow();
			return kSuccess;
		case kNotePad:
			ShutDownTheNotePad();
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
		case kAboutMSGWindow:
			OpenTheMSGWindow(theWindow);
			return kSuccess;
		case kOtherMSGWindow:
			OpenTheOtherMSGWindow(theWindow);
			return kSuccess;
		case kHelpWindow:
			OpenTheHelpWindow(theWindow);
			return kSuccess;
		case kNotePad:
			OpenTheNotePad(theWindow);
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
	}
	
	return kFailure;
}

enum DispatchError DisposeWindowDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutWindow:
			DisposeTheAboutWindow(theWindow);
			return kSuccess;
		case kOtherMSGWindow:
			DisposeTheOtherMSGWindow(theWindow);
			return kSuccess;
		case kHelpWindow:
			DisposeTheHelpWindow(theWindow);
			return kSuccess;
		case kNotePad:
			DisposeTheNotePad(theWindow);
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
		case kAboutMSGWindow:
			DrawTheAboutMSGWindow(theWindow, theDepth);
			return kSuccess;
		case kHelpWindow:
			DrawTheHelpWindow(theWindow, theDepth);
			return kSuccess;
		case kMainWindow:
			DrawTheMainWindow(theWindow, theDepth);
			return kSuccess;
			break;
	}
	
	return kFailure;
}

enum DispatchError ChangeDepthDispatch(short index, short newDepth)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kAboutMSGWindow:
			ChangeDepthTheAboutMSGWindow(theWindow, newDepth);
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
		case kOtherMSGWindow:
			CopybitsTheOtherMSGWindow(theWindow, offscreenWindow);
			return kSuccess;
		case kMainWindow:
			return (CopybitsTheMainWindow(theWindow, offscreenWindow) ? kSuccess : kFailure);
		case kNotePad:
			CopybitsTheNotePad(theWindow, offscreenWindow);
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
		case kNotePad:
			IdleInNotePad(theWindow, mouseLoc);
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
		case kOtherMSGWindow:
			GenericActivate(theWindow);
			ActivateTheOtherMSGWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			ActivateTheMainWindow(theWindow);
			return kSuccess;
		case kNotePad:
			GenericActivate(theWindow);
			ActivateTheNotePad(theWindow);
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
		case kOtherMSGWindow:
			GenericDeactivate(theWindow);
			DeactivateTheOtherMSGWindow(theWindow);
			return kSuccess;
		case kMainWindow:
			DeactivateTheMainWindow(theWindow);
			return kSuccess;
		case kNotePad:
			GenericDeactivate(theWindow);
			DeactivateTheNotePad(theWindow);
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
		case kOtherMSGWindow:
			GenericResizeControls(theWindow);
			return kSuccess;
		case kNotePad:
			GenericResizeControls(theWindow);
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
		case kOtherMSGWindow:
			GenericResizeControls(theWindow);
			return kSuccess;
		case kNotePad:
			GenericResizeControls(theWindow);
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
		case kOtherMSGWindow:
			GenericGetGrowSize(theWindow, sizeRect);
			return kSuccess;
		case kNotePad:
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
		case kOtherMSGWindow:
			GenericKeyPressedInWindow(theWindow, theChar);
			return kSuccess;
		case kHelpWindow:
			KeyPressedInHelpWindow(theWindow, theChar);
			return kSuccess;
		case kMainWindow:
			KeyPressedInMainWindow(theWindow, theChar);
			return kSuccess;
		case kNotePad:
			if (!GenericKeyPressedInWindow(theWindow, theChar))
				KeyPressedInNotePad(theWindow, theChar);
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
		case kOtherMSGWindow:
			GenericMouseClickedInWindow(theWindow, thePoint, FALSE);
			return kSuccess;
		case kHelpWindow:
			MouseClickedInHelpWindow(theWindow, thePoint);
			return kSuccess;
		case kMainWindow:
			MouseClickedInMainWindow(theWindow, thePoint);
			return kSuccess;
		case kNotePad:
			GenericMouseClickedInWindow(theWindow, thePoint, TRUE);
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
		case kNotePad:
			GenericCut(theWindow);
			return kSuccess;
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
		case kNotePad:
			GenericCopy(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError PasteDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kNotePad:
			PasteInNotePad(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError ClearDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kNotePad:
			GenericClear(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}

enum DispatchError SelectAllDispatch(short index)
{
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(index);
	
	switch (index)
	{
		case kNotePad:
			GenericSelectAll(theWindow);
			return kSuccess;
	}
	
	return kFailure;
}
