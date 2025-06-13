/* DisassemblyWindow.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "DisassemblyWindow.h"
#include "Screen.h"
#include "TextEdit.h"
#include "Memory.h"
#include "MainWindowStuff.h"
#include "GrowIcon.h"
#include "Main.h"
#include "WindowDispatcher.h"
#include "GlobalWindowMenuList.h"


struct DisaWindowRec
	{
		MainWindowRec*			Owner;
		WinType*						ScreenID;
		TextEditRec*				Editor;
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;
	};


/* create a new disassembly window.  the window is basically a non-editable text */
/* field containing the Data passed in (linefeed = 0x0a).  the caller is responsible */
/* for disposing of Data.  the function automatically notifies the main window */
/* that it has been created. */
DisaWindowRec*			NewDisassemblyWindow(char* Data, MainWindowRec* Owner)
	{
		DisaWindowRec*		Window;

		Window = (DisaWindowRec*)AllocPtrCanFail(sizeof(DisaWindowRec),"DisaWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Window->Owner = Owner;
		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,GetScreenWidth()
			- (2 + WindowOtherEdgeWidths(eDocumentWindow)) - 400,
			2 + WindowTitleBarHeight(eDocumentWindow),400,GetScreenHeight()
			- WindowTitleBarHeight(eDocumentWindow)
			- WindowOtherEdgeWidths(eDocumentWindow) - 4,
			(void (*)(void*))&DisassemblyWindowUpdator,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Disassembly");
		Window->Editor = NewTextEdit(Window->ScreenID,
			(TEScrollType)(eTEVScrollBar | eTEHScrollBar),GetMonospacedFont(),9,-1,-1,
			GetWindowWidth(Window->ScreenID) + 2,GetWindowHeight(Window->ScreenID) + 2);
		if (Window->Editor == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		if (!MainWindowNewDisassemblyNotify(Owner,Window))
			{
			 FailurePoint4:
				DisposeTextEdit(Window->Editor);
				goto FailurePoint3;
			}
		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&DisassemblyWindowDoIdle,
			(void (*)(void*))&DisassemblyWindowBecomeActive,
			(void (*)(void*))&DisassemblyWindowBecomeInactive,
			(void (*)(void*))&DisassemblyWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&DisassemblyWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&DisassemblyWindowDoKeyDown,
			(void (*)(void*))&DisassemblyWindowClose,
			(void (*)(void*))&DisassemblyWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&DisassemblyWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint5:
				MainWindowDisassemblyClosingNotify(Owner,Window);
				goto FailurePoint4;
			}
		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"Disassembly",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint6:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint5;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint7:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint6;
			}
		TextEditNewRawData(Window->Editor,Data,"\x0a");
		return Window;
	}


/* dispose of the disassembly window.  this automatically notifies the main window */
/* that it has been destroyed. */
void								DisposeDisassemblyWindow(DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		DisposeTextEdit(Window->Editor);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


void								DisassemblyWindowDoIdle(DisaWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->Editor);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->Editor,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								DisassemblyWindowBecomeActive(DisaWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->Editor);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(True/*enablegrowicon*/));
	}


void								DisassemblyWindowBecomeInactive(DisaWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->Editor);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(False/*disablegrowicon*/));
	}


void								DisassemblyWindowJustResized(DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		SetTextEditPosition(Window->Editor,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
	}


void								DisassemblyWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				DisassemblyWindowJustResized(Window);
			}
		else if (TextEditHitTest(Window->Editor,XLoc,YLoc))
			{
				TextEditDoMouseDown(Window->Editor,XLoc,YLoc,Modifiers);
			}
	}


void								DisassemblyWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((KeyCode == eLeftArrow) || (KeyCode == eRightArrow)
			|| (KeyCode == eUpArrow) || (KeyCode == eDownArrow))
			{
				TextEditDoKeyPressed(Window->Editor,KeyCode,Modifiers);
			}
	}


void								DisassemblyWindowClose(DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		/* notification of closing is here and not in dispose because FunctionWindow */
		/* calls dispose, so it knows that we are dying, but this routine handles */
		/* a user close, which FunctionWindow doesn't know about */
		MainWindowDisassemblyClosingNotify(Window->Owner,Window);
		DisposeDisassemblyWindow(Window);
	}


void								DisassemblyWindowUpdator(DisaWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence((char*)Window);
		TextEditFullRedraw(Window->Editor);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


void								DisassemblyWindowMenuSetup(DisaWindowRec* Window)
	{
		CheckPtrExistence(Window);
		MainWindowEnableGlobalMenus(Window->Owner);
		ChangeItemName(mCloseFile,"Close Disassembly Viewer");
		EnableMenuItem(mCloseFile);
		if (TextEditIsThereValidSelection(Window->Editor))
			{
				EnableMenuItem(mCopy);
				ChangeItemName(mCopy,"Copy Text");
			}
		EnableMenuItem(mSelectAll);
		ChangeItemName(mSelectAll,"Select All Text");
		SetItemCheckmark(Window->MyMenuItem);
	}


void								DisassemblyWindowDoMenuCommand(DisaWindowRec* Window,
											MenuItemType* MenuItem)
	{
		if (MainWindowDoGlobalMenuItem(Window->Owner,MenuItem))
			{
			}
		else if (MenuItem == mCloseFile)
			{
				DisassemblyWindowClose(Window);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->Editor);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->Editor);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,
					"DisassemblyWindowDoMenuCommand:  unknown menu command"));
			}
	}
