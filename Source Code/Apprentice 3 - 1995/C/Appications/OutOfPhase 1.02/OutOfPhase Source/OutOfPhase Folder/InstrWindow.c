/* InstrWindow.c */
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

#include "InstrWindow.h"
#include "InstrObject.h"
#include "MainWindowStuff.h"
#include "InstrList.h"
#include "TextEdit.h"
#include "WindowDispatcher.h"
#include "Memory.h"
#include "GrowIcon.h"
#include "Main.h"
#include "FindDialog.h"
#include "DataMunging.h"
#include "GlobalWindowMenuList.h"


#define WINSIZEX (490)
#define WINSIZEY (320)
#define TITLEINDENT (8)

#define NAMEX(W,H) (-1)
#define NAMEY(W,H) (2)
#define NAMEWIDTH(W,H) (W + 2)
#define NAMEHEIGHT(W,H) (40)

#define BODYX(W,H) (NAMEX(W,H))
#define BODYY(W,H) (NAMEY(W,H) + NAMEHEIGHT(W,H) + 4)
#define BODYWIDTH(W,H) (NAMEWIDTH(W,H))
#define BODYHEIGHT(W,H) (H - BODYY(W,H) + 1)


struct InstrWindowRec
	{
		InstrObjectRec*			InstrObj;
		MainWindowRec*			MainWindow;
		InstrListRec*				InstrList;

		WinType*						ScreenID;
		TextEditRec*				NameEdit;
		TextEditRec*				BodyEdit;
		TextEditRec*				ActiveTextEdit;
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;
	};


/* create a new instrument specification editing window */
InstrWindowRec*			NewInstrWindow(struct InstrObjectRec* InstrObj,
											struct MainWindowRec* MainWindow,
											struct InstrListRec* InstrList, short WinX, short WinY,
											short WinWidth, short WinHeight)
	{
		InstrWindowRec*		Window;
		OrdType						FontHeight;
		char*							StringTemp;

		/* deal with window placement */
		if ((WinWidth < 100) || (WinHeight < 100) || ((eOptionKey & CheckModifiers()) != 0))
			{
				WinX = 20 + WindowOtherEdgeWidths(eDocumentWindow);
				WinY = 20 + WindowTitleBarHeight(eDocumentWindow);
				WinWidth = WINSIZEX;
				WinHeight = WINSIZEY;
			}
		MakeWindowFitOnScreen(&WinX,&WinY,&WinWidth,&WinHeight);

		CheckPtrExistence(InstrObj);
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(InstrList);
		Window = (InstrWindowRec*)AllocPtrCanFail(sizeof(InstrWindowRec),"InstrWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Window->InstrObj = InstrObj;
		Window->MainWindow = MainWindow;
		Window->InstrList = InstrList;

		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,WinX,WinY,WinWidth,WinHeight,
			(void (*)(void*))&InstrWindowUpdate,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		FontHeight = GetFontHeight(GetScreenFont(),9);

		Window->NameEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),9,
			NAMEX(WinWidth,WinHeight),NAMEY(WinWidth,WinHeight) + FontHeight,
			NAMEWIDTH(WinWidth,WinHeight),NAMEHEIGHT(WinWidth,WinHeight) - FontHeight);
		if (Window->NameEdit == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		StringTemp = InstrObjectGetNameCopy(InstrObj);
		if (StringTemp == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->NameEdit);
				goto FailurePoint3;
			}
		TextEditNewRawData(Window->NameEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NameEdit);

		Window->BodyEdit = NewTextEdit(Window->ScreenID,
			(TEScrollType)(eTEVScrollBar | eTEHScrollBar),GetMonospacedFont(),9,
			BODYX(WinWidth,WinHeight),BODYY(WinWidth,WinHeight) + FontHeight,
			BODYWIDTH(WinWidth,WinHeight),BODYHEIGHT(WinWidth,WinHeight) - FontHeight);
		if (Window->BodyEdit == NIL)
			{
			 FailurePoint5:
				goto FailurePoint4;
			}
		StringTemp = InstrObjectGetDefinitionCopy(InstrObj);
		if (StringTemp == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->BodyEdit);
				goto FailurePoint5;
			}
		SetTextEditTabSize(Window->BodyEdit,MainWindowGetTabSize(MainWindow));
		TextEditNewRawData(Window->BodyEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->BodyEdit);
		SetTextEditAutoIndent(Window->BodyEdit,True);
		Window->ActiveTextEdit = Window->BodyEdit;

		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&InstrWindowDoIdle,
			(void (*)(void*))&InstrWindowBecomeActive,
			(void (*)(void*))&InstrWindowBecomeInactive,
			(void (*)(void*))&InstrWindowResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&InstrWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&InstrWindowDoKeyDown,
			(void (*)(void*))&InstrWindowClose,
			(void (*)(void*))&InstrWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&InstrWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint7:
				goto FailurePoint6;
			}

		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"x",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint8:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint7;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint9:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint8;
			}

		InstrWindowResetTitlebar(Window);

		return Window;
	}


/* write back modified data and dispose of the instrument window */
void								DisposeInstrWindow(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);

		/* save data */
		if (!InstrWindowWritebackModifiedData(Window))
			{
				/* failed -- now what? */
			}

		CheckOutDyingWindow(Window->MyGenericWindow);
		InstrObjectWindowCloseNotify(Window->InstrObj,
			GetWindowXStart(Window->ScreenID),GetWindowYStart(Window->ScreenID),
			GetWindowWidth(Window->ScreenID),GetWindowHeight(Window->ScreenID));
		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		DisposeTextEdit(Window->NameEdit);
		DisposeTextEdit(Window->BodyEdit);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


/* bring the window to the top and give it the focus */
void								BringInstrWindowToFront(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		ActivateThisWindow(Window->ScreenID);
	}


/* returns True if the data has been modified since the last file save. */
MyBoolean						HasInstrWindowBeenModified(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditDoesItNeedToBeSaved(Window->NameEdit)
			|| TextEditDoesItNeedToBeSaved(Window->BodyEdit);
	}


/* highlight the line in the instrument specification edit */
void								InstrWindowHilightLine(InstrWindowRec* Window, long LineNumber)
	{
		CheckPtrExistence(Window);
		if (Window->ActiveTextEdit != Window->BodyEdit)
			{
				DisableTextEditSelection(Window->ActiveTextEdit);
				Window->ActiveTextEdit = Window->BodyEdit;
				EnableTextEditSelection(Window->ActiveTextEdit);
			}
		SetTextEditSelection(Window->BodyEdit,LineNumber,0,LineNumber + 1,0);
		TextEditShowSelection(Window->BodyEdit);
	}


void								InstrWindowDoIdle(InstrWindowRec* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->ActiveTextEdit);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->NameEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->BodyEdit,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								InstrWindowBecomeActive(InstrWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->ActiveTextEdit);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(True/*enablegrowicon*/));
	}


void								InstrWindowBecomeInactive(InstrWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->ActiveTextEdit);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(False/*disablegrowicon*/));
	}


void								InstrWindowResized(InstrWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;
		OrdType						FontHeight;

		CheckPtrExistence(Window);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawBoxErase(Window->ScreenID,0,0,XSize,YSize);
		FontHeight = GetFontHeight(GetScreenFont(),9);
		SetTextEditPosition(Window->NameEdit,
			NAMEX(XSize,YSize),NAMEY(XSize,YSize) + FontHeight,
			NAMEWIDTH(XSize,YSize),NAMEHEIGHT(XSize,YSize) - FontHeight);
		SetTextEditPosition(Window->BodyEdit,
			BODYX(XSize,YSize),BODYY(XSize,YSize) + FontHeight,
			BODYWIDTH(XSize,YSize),BODYHEIGHT(XSize,YSize) - FontHeight);
	}


void								InstrWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				InstrWindowResized(Window);
			}
		else if (TextEditHitTest(Window->NameEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->NameEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->NameEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->NameEdit,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->BodyEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->BodyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->BodyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->BodyEdit,XLoc,YLoc,Modifiers);
			}
	}


void								InstrWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditDoKeyPressed(Window->ActiveTextEdit,KeyCode,Modifiers);
	}


void								InstrWindowClose(InstrWindowRec* Window)
	{
		DisposeInstrWindow(Window);
	}


void								InstrWindowMenuSetup(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		MainWindowEnableGlobalMenus(Window->MainWindow);
		EnableMenuItem(mPaste);
		ChangeItemName(mPaste,"Paste Text");
		if (TextEditIsThereValidSelection(Window->ActiveTextEdit))
			{
				EnableMenuItem(mCut);
				ChangeItemName(mCut,"Cut Text");
				EnableMenuItem(mCopy);
				ChangeItemName(mCopy,"Copy Text");
				EnableMenuItem(mClear);
				ChangeItemName(mClear,"Clear Text");
			}
		EnableMenuItem(mShiftLeft);
		EnableMenuItem(mShiftRight);
		EnableMenuItem(mBalanceParens);
		EnableMenuItem(mSelectAll);
		ChangeItemName(mSelectAll,"Select All Text");
		if (TextEditCanWeUndo(Window->ActiveTextEdit))
			{
				EnableMenuItem(mUndo);
				ChangeItemName(mUndo,"Undo Text Change");
			}
		ChangeItemName(mCloseFile,"Close Instrument Editor");
		EnableMenuItem(mCloseFile);
		ChangeItemName(mDeleteObject,"Delete Instrument");
		EnableMenuItem(mDeleteObject);
		EnableMenuItem(mFind);
		if (PtrSize(GlobalSearchString) != 0)
			{
				EnableMenuItem(mFindAgain);
				if ((Window->ActiveTextEdit != NIL)
					&& TextEditIsThereValidSelection(Window->ActiveTextEdit))
					{
						EnableMenuItem(mReplace);
						EnableMenuItem(mReplaceAndFindAgain);
					}
			}
		EnableMenuItem(mShowSelection);
		if ((Window->ActiveTextEdit != NIL)
			&& TextEditIsThereValidSelection(Window->ActiveTextEdit))
			{
				EnableMenuItem(mEnterSelection);
			}
		SetItemCheckmark(Window->MyMenuItem);
		EnableMenuItem(mBuildFunction);
		ChangeItemName(mBuildFunction,"Build Instrument");
		EnableMenuItem(mUnbuildFunction);
		ChangeItemName(mUnbuildFunction,"Unbuild Instrument");
	}


void								InstrWindowDoMenuCommand(InstrWindowRec* Window,
											struct MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (MainWindowDoGlobalMenuItem(Window->MainWindow,MenuItem))
			{
			}
		else if (MenuItem == mPaste)
			{
				TextEditDoMenuPaste(Window->ActiveTextEdit);
			}
		else if (MenuItem == mCut)
			{
				TextEditDoMenuCut(Window->ActiveTextEdit);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->ActiveTextEdit);
			}
		else if (MenuItem == mClear)
			{
				TextEditDoMenuClear(Window->ActiveTextEdit);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->ActiveTextEdit);
			}
		else if (MenuItem == mUndo)
			{
				TextEditDoMenuUndo(Window->ActiveTextEdit);
			}
		else if (MenuItem == mCloseFile)
			{
				InstrWindowClose(Window);
			}
		else if (MenuItem == mShiftLeft)
			{
				TextEditShiftSelectionLeftOneTab(Window->ActiveTextEdit);
			}
		else if (MenuItem == mShiftRight)
			{
				TextEditShiftSelectionRightOneTab(Window->ActiveTextEdit);
			}
		else if (MenuItem == mBalanceParens)
			{
				TextEditBalanceParens(Window->ActiveTextEdit);
			}
		else if (MenuItem == mDeleteObject)
			{
				InstrListDeleteInstr(Window->InstrList,Window->InstrObj);
			}
		else if (MenuItem == mFind)
			{
				if (Window->ActiveTextEdit != Window->BodyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->BodyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				switch (DoFindDialog(&GlobalSearchString,&GlobalReplaceString,
					mCut,mPaste,mCopy,mUndo,mSelectAll,mClear))
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"FunctionWindowDoMenuCommand:  bad value from DoFindDialog"));
							break;
						case eFindCancel:
						case eDontFind:
							break;
						case eFindFromStart:
							SetTextEditInsertionPoint(Window->ActiveTextEdit,0,0);
							TextEditFindAgain(Window->ActiveTextEdit,GlobalSearchString);
							TextEditShowSelection(Window->ActiveTextEdit);
							break;
						case eFindAgain:
							TextEditFindAgain(Window->ActiveTextEdit,GlobalSearchString);
							TextEditShowSelection(Window->ActiveTextEdit);
							break;
					}
			}
		else if (MenuItem == mFindAgain)
			{
				if (Window->ActiveTextEdit != Window->BodyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->BodyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditFindAgain(Window->ActiveTextEdit,GlobalSearchString);
				TextEditShowSelection(Window->ActiveTextEdit);
			}
		else if (MenuItem == mReplace)
			{
				if (Window->ActiveTextEdit != Window->BodyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->BodyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				if (TextEditIsThereValidSelection(Window->ActiveTextEdit))
					{
						TextEditInsertRawData(Window->ActiveTextEdit,GlobalReplaceString,
							SYSTEMLINEFEED);
					}
			}
		else if (MenuItem == mReplaceAndFindAgain)
			{
				if (Window->ActiveTextEdit != Window->BodyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->BodyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				if (TextEditIsThereValidSelection(Window->ActiveTextEdit))
					{
						TextEditInsertRawData(Window->ActiveTextEdit,GlobalReplaceString,
							SYSTEMLINEFEED);
						TextEditFindAgain(Window->ActiveTextEdit,GlobalSearchString);
						TextEditShowSelection(Window->ActiveTextEdit);
					}
			}
		else if (MenuItem == mShowSelection)
			{
				TextEditShowSelection(Window->ActiveTextEdit);
			}
		else if (MenuItem == mEnterSelection)
			{
				char*						NewString;

				NewString = TextEditGetSelection(Window->ActiveTextEdit);
				if (NewString != NIL)
					{
						ReleasePtr(GlobalSearchString);
						GlobalSearchString = NewString;
					}
			}
		else if (MenuItem == mBuildFunction)
			{
				BuildInstrObject(Window->InstrObj);
			}
		else if (MenuItem == mUnbuildFunction)
			{
				UnbuildInstrObject(Window->InstrObj);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"FunctionWindowDoMenuCommand:  unknown menu command"));
			}
	}


void								InstrWindowUpdate(InstrWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->NameEdit);
		TextEditFullRedraw(Window->BodyEdit);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Instrument Name:",16,
			NAMEX(XSize,YSize) + TITLEINDENT,NAMEY(XSize,YSize),eBold);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Instrument Body:",16,
			BODYX(XSize,YSize) + TITLEINDENT,BODYY(XSize,YSize),eBold);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize - 15,YSize - 15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


/* get a copy of the instrument name */
char*								InstrWindowGetNameCopy(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditGetRawData(Window->NameEdit,"\x0a");
	}


/* get a copy of the instrument definition text */
char*								InstrWindowGetDefinitionCopy(InstrWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditGetRawData(Window->BodyEdit,"\x0a");
	}


/* the filename has changed, so update the window title bar.  NewFilename is a */
/* non-null-terminated string which must be disposed by the caller. */
void								InstrWindowGlobalNameChange(InstrWindowRec* Window, char* NewFilename)
	{
		char*							LocalNameCopy;

		CheckPtrExistence(Window);
		CheckPtrExistence(NewFilename);
		LocalNameCopy = InstrWindowGetNameCopy(Window);
		if (LocalNameCopy != NIL)
			{
				char*							SeparatorString;

				SeparatorString = StringToBlockCopy(":  ");
				if (SeparatorString != NIL)
					{
						char*							LeftHalfOfString;

						LeftHalfOfString = ConcatBlockCopy(NewFilename,SeparatorString);
						if (LeftHalfOfString != NIL)
							{
								char*							TotalString;

								TotalString = ConcatBlockCopy(LeftHalfOfString,LocalNameCopy);
								if (TotalString != NIL)
									{
										char*							NullTerminatedString;

										NullTerminatedString = BlockToStringCopy(TotalString);
										if (NullTerminatedString != NIL)
											{
												SetWindowName(Window->ScreenID,NullTerminatedString);
												ChangeItemName(Window->MyMenuItem,NullTerminatedString);
												ReleasePtr(NullTerminatedString);
											}
										ReleasePtr(TotalString);
									}
								ReleasePtr(LeftHalfOfString);
							}
						ReleasePtr(SeparatorString);
					}
				ReleasePtr(LocalNameCopy);
			}
	}


/* update the title bar of the window even if the filename hasn't changed */
void								InstrWindowResetTitlebar(InstrWindowRec* Window)
	{
		char*							DocumentName;

		CheckPtrExistence(Window);
		DocumentName = GetCopyOfDocumentName(Window->MainWindow);
		if (DocumentName != NIL)
			{
				InstrWindowGlobalNameChange(Window,DocumentName);
				ReleasePtr(DocumentName);
			}
	}


/* write back all modified data to the instrument object */
MyBoolean						InstrWindowWritebackModifiedData(InstrWindowRec* Window)
	{
		MyBoolean					SuccessFlag = True;

		CheckPtrExistence(Window);

		/* save the name if it has been altered */
		if (TextEditDoesItNeedToBeSaved(Window->NameEdit))
			{
				char*					NameTemp;

				NameTemp = InstrWindowGetNameCopy(Window);
				if (NameTemp != NIL)
					{
						InstrObjectPutName(Window->InstrObj,NameTemp);
						TextEditHasBeenSaved(Window->NameEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		/* save the body if it has been altered */
		if (TextEditDoesItNeedToBeSaved(Window->BodyEdit))
			{
				char*							Temp;

				Temp = InstrWindowGetDefinitionCopy(Window);
				if (Temp != NIL)
					{
						InstrObjectPutDefinition(Window->InstrObj,Temp);
						TextEditHasBeenSaved(Window->BodyEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		return SuccessFlag;
	}
