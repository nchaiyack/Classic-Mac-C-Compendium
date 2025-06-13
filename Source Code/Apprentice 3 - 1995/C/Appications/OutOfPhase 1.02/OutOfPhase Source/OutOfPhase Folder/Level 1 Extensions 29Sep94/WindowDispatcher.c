/* WindowDispatcher.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "WindowDispatcher.h"
#include "Array.h"
#include "Memory.h"


struct GenericWindowRec
	{
		/* Level 0 window ID for identifying it from the event loop */
		WinType*				ScreenID;
		/* reference value for callbacks */
		void*						Reference;
		/* callback hooks */
		void						(*WindowDoIdle)(void* Reference, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);
		void						(*WindowBecomeActive)(void* Reference);
		void						(*WindowBecomeInactive)(void* Reference);
		void						(*WindowResized)(void* Reference);
		void						(*WindowDoMouseDown)(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, void* Reference);
		void						(*WindowDoKeyDown)(unsigned char KeyCode,
											ModifierFlags Modifiers, void* Reference);
		void						(*WindowClose)(void* Reference);
		void						(*WindowMenuSetup)(void* Reference);
		void						(*WindowDoMenuCommand)(void* Reference, MenuItemType* MenuItem);
	};


/* this is a list of all of the windows registered with us */
static ArrayRec*							TotalWindowList;

/* this contains the window that was last active.  NIL means no active window. */
static GenericWindowRec*			TheActiveWindow = NIL;


struct IdleTaskSignature
	{
		void						(*IdleCallback)(void* Refcon);
		void*						IdleRefcon;
	};


/* list of installed idle tasks */
static ArrayRec*							IdleTaskList;


/* initialize internal data structures for the window dispatch handler */
MyBoolean						InitializeWindowDispatcher(void)
	{
		APRINT(("+InitializeWindowDispatcher"));
		TotalWindowList = NewArray();
		if (TotalWindowList == NIL)
			{
			 FailurePoint1:
				APRINT(("-InitializeWindowDispatcher failed"));
				return False;
			}
		IdleTaskList = NewArray();
		if (IdleTaskList == NIL)
			{
			 FailurePoint2:
				DisposeArray(TotalWindowList);
				goto FailurePoint1;
			}
		APRINT(("-InitializeWindowDispatcher"));
		return True;
	}


/* dispose of internal data structures for the window dispatch handler */
void								ShutdownWindowDispatcher(void)
	{
		APRINT(("+ShutdownWindowDispatcher"));
		ERROR(ArrayGetLength(TotalWindowList) != 0,PRERR(AllowResume,
			"ShutdownWindowDispatcher:  some windows haven't been disposed of"));
		ERROR(ArrayGetLength(IdleTaskList) != 0,PRERR(AllowResume,
			"ShutdownWindowDispatcher:  some idle tasks are still installed"));
		DisposeArray(TotalWindowList);
		DisposeArray(IdleTaskList);
		APRINT(("-ShutdownWindowDispatcher"));
	}


/* check in a new window, install callback routines, and obtain a unique identifier */
GenericWindowRec*		CheckInNewWindow(WinType* ScreenID, void* Reference,
											void (*WindowDoIdle)(void* Reference, MyBoolean CheckCursorFlag,
														OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers),
											void (*WindowBecomeActive)(void* Reference),
											void (*WindowBecomeInactive)(void* Reference),
											void (*WindowResized)(void* Reference),
											void (*WindowDoMouseDown)(OrdType XLoc, OrdType YLoc,
														ModifierFlags Modifiers, void* Reference),
											void (*WindowDoKeyDown)(unsigned char KeyCode,
														ModifierFlags Modifiers, void* Reference),
											void (*WindowClose)(void* Reference),
											void (*WindowMenuSetup)(void* Reference),
											void (*WindowDoMenuCommand)(void* Reference,
														MenuItemType* MenuItem))
	{
		GenericWindowRec*	WindowRecord;

		EXECUTE({long Scan; for (Scan = 0; Scan < ArrayGetLength(TotalWindowList);
			Scan += 1) {if (ScreenID == (*(GenericWindowRec*)ArrayGetElement(
			TotalWindowList,Scan)).ScreenID){PRERR(AllowResume,
			"CheckInNewWindow:  Window is already checked in");}}})
		ERROR((WindowDoIdle == NIL) || (WindowBecomeActive == NIL)
			|| (WindowBecomeInactive == NIL) || (WindowResized == NIL)
			|| (WindowDoMouseDown == NIL) || (WindowDoKeyDown == NIL)
			|| (WindowClose == NIL) || (WindowMenuSetup == NIL)
			|| (WindowDoMenuCommand == NIL),PRERR(ForceAbort,
			"CheckInNewWindow:  a callback has not been specified"));
		CheckPtrExistence(ScreenID);
		WindowRecord = (GenericWindowRec*)AllocPtrCanFail(sizeof(GenericWindowRec),
			"GenericWindowRec");
		if (WindowRecord != NIL)
			{
				if (ArrayAppendElement(TotalWindowList,WindowRecord))
					{
						WindowRecord->ScreenID = ScreenID;
						WindowRecord->Reference = Reference;
						WindowRecord->WindowDoIdle = WindowDoIdle;
						WindowRecord->WindowBecomeActive = WindowBecomeActive;
						WindowRecord->WindowBecomeInactive = WindowBecomeInactive;
						WindowRecord->WindowResized = WindowResized;
						WindowRecord->WindowDoMouseDown = WindowDoMouseDown;
						WindowRecord->WindowDoKeyDown = WindowDoKeyDown;
						WindowRecord->WindowClose = WindowClose;
						WindowRecord->WindowMenuSetup = WindowMenuSetup;
						WindowRecord->WindowDoMenuCommand = WindowDoMenuCommand;
					}
				 else
					{
						ReleasePtr((char*)WindowRecord);
						WindowRecord = NIL;
					}
			}
		return WindowRecord;
	}


/* notify the window dispatcher that this window is no longer with us */
void								CheckOutDyingWindow(GenericWindowRec* Window)
	{
		/* if the active window is dying, then clear the active window register */
		if (TheActiveWindow == Window)
			{
				TheActiveWindow = NIL;
			}
		/* delete element from array.  causes error if window isn't in list */
		ArrayDeleteElement(TotalWindowList,ArrayFindElement(TotalWindowList,Window));
		/* dump the window record */
		ReleasePtr((char*)Window);
	}


/* get the identifier for the currently active window */
GenericWindowRec*		GetCurrentWindowID(void)
	{
		return TheActiveWindow;
	}


/* internal routine to deal with Level 0 window IDs */
static GenericWindowRec*	MapWindowIDToRecord(WinType* ScreenID)
	{
		long									Scan;

		for (Scan = 0; Scan < ArrayGetLength(TotalWindowList); Scan += 1)
			{
				GenericWindowRec*			Window;

				Window = (GenericWindowRec*)ArrayGetElement(TotalWindowList,Scan);
				if (Window->ScreenID == ScreenID)
					{
						CheckPtrExistence(Window);
						return (GenericWindowRec*)ArrayGetElement(TotalWindowList,Scan);
					}
			}
		EXECUTE(PRERR(ForceAbort,"MapWindowIDToRecord:  Couldn't find window"));
	}


void								DispatchDoIdle(WinType* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowDoIdle))(OtherWindow->Reference,
					CheckCursorFlag,XLoc,YLoc,Modifiers);
			}
		DispatchIdleTaskCallback();
	}


void								DispatchActiveWindowJustChanged(WinType* NewWindow)
	{
		/* deactivate the current window */
		if (TheActiveWindow != NIL)
			{
				(*(TheActiveWindow->WindowBecomeInactive))(TheActiveWindow->Reference);
				TheActiveWindow = NIL;
			}
		/* find the new active window (if there are any windows) */
		if (NewWindow != NIL)
			{
				TheActiveWindow = MapWindowIDToRecord(NewWindow);
			}
		/* activate it (if there is one) */
		if (TheActiveWindow != NIL)
			{
				(*(TheActiveWindow->WindowBecomeActive))(TheActiveWindow->Reference);
			}
	}


void								DispatchWindowJustResized(WinType* Window)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowResized))(OtherWindow->Reference);
			}
	}


void								DispatchDoMouseDown(WinType* Window, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowDoMouseDown))(XLoc,YLoc,Modifiers,OtherWindow->Reference);
			}
	}


void								DispatchDoKeyDown(WinType* Window, unsigned char KeyCode,
											ModifierFlags Modifiers)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowDoKeyDown))(KeyCode,Modifiers,OtherWindow->Reference);
			}
	}


void								DispatchCloseWindow(WinType* Window)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowClose))(OtherWindow->Reference);
			}
	}


void								DispatchMenuStarting(WinType* Window)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowMenuSetup))(OtherWindow->Reference);
			}
	}


void								DispatchProcessMenuCommand(WinType* Window, MenuItemType* MenuItem)
	{
		if (Window != NIL)
			{
				GenericWindowRec*		OtherWindow;

				OtherWindow = MapWindowIDToRecord(Window);
				(*(OtherWindow->WindowDoMenuCommand))(OtherWindow->Reference,MenuItem);
			}
	}


/* install an idle task to be executed whenever an idle event occurs */
IdleTaskSignature*	DispatchInstallIdleTask(void (*Proc)(void* Refcon), void* Refcon)
	{
		IdleTaskSignature*	Signature;

		Signature = (IdleTaskSignature*)AllocPtrCanFail(sizeof(IdleTaskSignature),
			"IdleTaskSignature");
		if (Signature == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Signature->IdleCallback = Proc;
		Signature->IdleRefcon = Refcon;
		if (!ArrayAppendElement(IdleTaskList,Signature))
			{
			 FailurePoint2:
				ReleasePtr((char*)Signature);
				goto FailurePoint1;
			}
		return Signature;
	}


/* remove an idle task */
void								DispatchRemoveIdleTask(IdleTaskSignature* Signature)
	{
		CheckPtrExistence(Signature);
		ArrayDeleteElement(IdleTaskList,ArrayFindElement(IdleTaskList,Signature));
		ReleasePtr((char*)Signature);
	}


/* dispatch a callback to all installed idle tasks */
void								DispatchIdleTaskCallback(void)
	{
		long							Scan;

		Scan = 0;
		while (Scan < ArrayGetLength(IdleTaskList))
			{
				IdleTaskSignature*	Signature;

				Signature = (IdleTaskSignature*)ArrayGetElement(IdleTaskList,Scan);
				(*(Signature->IdleCallback))(Signature->IdleRefcon);
				Scan += 1;
			}
	}
