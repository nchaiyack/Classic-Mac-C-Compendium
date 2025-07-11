/* FunctionWindow.c */
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

#include "FunctionWindow.h"
#include "MainWindowStuff.h"
#include "FunctionObject.h"
#include "TextEdit.h"
#include "WindowDispatcher.h"
#include "Memory.h"
#include "GrowIcon.h"
#include "DataMunging.h"
#include "Main.h"
#include "DisassemblyWindow.h"
#include "Alert.h"
#include "FunctionList.h"
#include "FindDialog.h"
#include "GlobalWindowMenuList.h"


#define WINSIZEX (400)
#define WINSIZEY (300)
#define TITLEINDENT (8)

#define NAMEX(W,H) (-1)
#define NAMEY(W,H) (2)
#define NAMEWIDTH(W,H) (W + 2)
#define NAMEHEIGHT(W,H) (40)

#define BODYX(W,H) (NAMEX(W,H))
#define BODYY(W,H) (NAMEY(W,H) + NAMEHEIGHT(W,H) + 4)
#define BODYWIDTH(W,H) (NAMEWIDTH(W,H))
#define BODYHEIGHT(W,H) (H - BODYY(W,H) + 1)


struct FunctionWindowRec
	{
		MainWindowRec*			MainWindow;
		FunctionObjectRec*	FunctionObject;
		FunctionListRec*		FunctionList;

		WinType*						ScreenID;
		TextEditRec*				NameEdit;
		TextEditRec*				BodyEdit;
		TextEditRec*				ActiveTextEdit;
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;
	};


FunctionWindowRec*	NewFunctionWindow(struct MainWindowRec* MainWindow,
											struct FunctionObjectRec* FunctionObject,
											struct FunctionListRec* FunctionList, OrdType WinX, OrdType WinY,
											OrdType WinWidth, OrdType WinHeight)
	{
		FunctionWindowRec*	Window;
		OrdType							FontHeight;
		char*								StringTemp;

		/* deal with window placement */
		if ((WinWidth < 100) || (WinHeight < 100) || ((eOptionKey & CheckModifiers()) != 0))
			{
				WinX = 20 + WindowOtherEdgeWidths(eDocumentWindow);
				WinY = 20 + WindowTitleBarHeight(eDocumentWindow);
				WinWidth = WINSIZEX;
				WinHeight = WINSIZEY;
			}
		MakeWindowFitOnScreen(&WinX,&WinY,&WinWidth,&WinHeight);

		/* initialize existence of object */
		Window = (FunctionWindowRec*)AllocPtrCanFail(
			sizeof(FunctionWindowRec),"FunctionWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Window->MainWindow = MainWindow;
		Window->FunctionObject = FunctionObject;
		Window->FunctionList = FunctionList;

		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,WinX,WinY,WinWidth,WinHeight,
			(void (*)(void*))&FunctionWindowUpdator,Window);
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
		StringTemp = FunctionObjectGetNameCopy(FunctionObject);
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
		StringTemp = FunctionObjectGetSourceCopy(FunctionObject);
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
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&FunctionWindowDoIdle,
			(void (*)(void*))&FunctionWindowBecomeActive,
			(void (*)(void*))&FunctionWindowBecomeInactive,
			(void (*)(void*))&FunctionWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&FunctionWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&FunctionWindowDoKeyDown,
			(void (*)(void*))&FunctionWindowClose,
			(void (*)(void*))&FunctionWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&FunctionWindowDoMenuCommand);
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

		FunctionWindowResetTitlebar(Window);

		return Window;
	}


void								DisposeFunctionWindow(FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);

		/* save data */
		if (!FunctionWindowWritebackModifiedData(Window))
			{
				/* failed -- now what? */
			}

		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		FunctionObjectClosingWindowNotify(Window->FunctionObject,
			GetWindowXStart(Window->ScreenID),GetWindowYStart(Window->ScreenID),
			GetWindowWidth(Window->ScreenID),GetWindowHeight(Window->ScreenID));
		DisposeTextEdit(Window->NameEdit);
		DisposeTextEdit(Window->BodyEdit);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


void								FunctionWindowDoIdle(FunctionWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
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


void								FunctionWindowBecomeActive(FunctionWindowRec* Window)
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


void								FunctionWindowBecomeInactive(FunctionWindowRec* Window)
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


void								FunctionWindowJustResized(FunctionWindowRec* Window)
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


void								FunctionWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				FunctionWindowJustResized(Window);
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


void								FunctionWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers,FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditDoKeyPressed(Window->ActiveTextEdit,KeyCode,Modifiers);
	}


void								FunctionWindowClose(FunctionWindowRec* Window)
	{
		DisposeFunctionWindow(Window);
	}


void								FunctionWindowUpdator(FunctionWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->NameEdit);
		TextEditFullRedraw(Window->BodyEdit);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Function Name:",14,
			NAMEX(XSize,YSize) + TITLEINDENT,NAMEY(XSize,YSize),eBold);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Function Body:",14,
			BODYX(XSize,YSize) + TITLEINDENT,BODYY(XSize,YSize),eBold);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize - 15,YSize - 15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


void								FunctionWindowMenuSetup(FunctionWindowRec* Window)
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
		ChangeItemName(mCloseFile,"Close Function Editor");
		EnableMenuItem(mCloseFile);
		ChangeItemName(mBuildFunction,"Compile Function Module");
		EnableMenuItem(mBuildFunction);
		ChangeItemName(mUnbuildFunction,"Unbuild This Function Module");
		EnableMenuItem(mUnbuildFunction);
		ChangeItemName(mDisassembleFunction,"Disassemble Function Module");
		EnableMenuItem(mDisassembleFunction);
		ChangeItemName(mDeleteObject,"Delete Function");
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
	}


void								FunctionWindowDoMenuCommand(FunctionWindowRec* Window,
											MenuItemType* MenuItem)
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
				FunctionWindowClose(Window);
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
		else if (MenuItem == mBuildFunction)
			{
				FunctionObjectBuild(Window->FunctionObject);
			}
		else if (MenuItem == mUnbuildFunction)
			{
				FunctionObjectUnbuild(Window->FunctionObject);
			}
		else if (MenuItem == mDisassembleFunction)
			{
				char*					Disassembly;

				Disassembly = FunctionObjectDisassemble(Window->FunctionObject);
				if (Disassembly != NIL)
					{
						if (NewDisassemblyWindow(Disassembly,Window->MainWindow) == NIL)
							{
								AlertHalt("There is not enough memory available to "
									"show the disassembly window.",NIL);
							}
						ReleasePtr(Disassembly);
					}
			}
		else if (MenuItem == mDeleteObject)
			{
				FunctionListDeleteFunction(Window->FunctionList,Window->FunctionObject);
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
		else
			{
				EXECUTE(PRERR(AllowResume,"FunctionWindowDoMenuCommand:  unknown menu command"));
			}
	}


/* utility routine to hilite a text line on which a compile error has occurred */
void								FunctionWindowHiliteLine(FunctionWindowRec* Window, long ErrorLine)
	{
		CheckPtrExistence(Window);
		if (Window->ActiveTextEdit != Window->BodyEdit)
			{
				DisableTextEditSelection(Window->ActiveTextEdit);
				Window->ActiveTextEdit = Window->BodyEdit;
				EnableTextEditSelection(Window->ActiveTextEdit);
			}
		SetTextEditSelection(Window->BodyEdit,ErrorLine,0,ErrorLine + 1,0);
		TextEditShowSelection(Window->BodyEdit);
	}


/* utility routine to bring this window to the top. */
void								FunctionWindowBringToTop(FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		ActivateThisWindow(Window->ScreenID);
	}


/* check to see if data in text edit boxes has been altered */
MyBoolean						HasFunctionWindowBeenModified(FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditDoesItNeedToBeSaved(Window->NameEdit)
			|| TextEditDoesItNeedToBeSaved(Window->BodyEdit);
	}


/* get a copy of the name edit */
char*								FunctionWindowGetNameCopy(FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditGetRawData(Window->NameEdit,"\x0a");
	}


/* get a copy of the source text edit */
char*								FunctionWindowGetSourceCopy(FunctionWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return TextEditGetRawData(Window->BodyEdit,"\x0a");
	}


/* the name of the document has changed, so change the name of the window */
void								FunctionWindowGlobalNameChange(FunctionWindowRec* Window,
											char* NewFilename)
	{
		char*							LocalNameCopy;

		CheckPtrExistence(Window);
		CheckPtrExistence(NewFilename);
		LocalNameCopy = FunctionWindowGetNameCopy(Window);
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


/* refresh the titlebar of the window */
void								FunctionWindowResetTitlebar(FunctionWindowRec* Window)
	{
		char*							DocumentName;

		CheckPtrExistence(Window);
		DocumentName = GetCopyOfDocumentName(Window->MainWindow);
		if (DocumentName != NIL)
			{
				FunctionWindowGlobalNameChange(Window,DocumentName);
				ReleasePtr(DocumentName);
			}
	}


/* force the function window to write back to the object any data that has changed */
MyBoolean						FunctionWindowWritebackModifiedData(FunctionWindowRec* Window)
	{
		MyBoolean					SuccessFlag = True;

		CheckPtrExistence(Window);

		/* save the name if it has been altered */
		if (TextEditDoesItNeedToBeSaved(Window->NameEdit))
			{
				char*					NameTemp;

				NameTemp = FunctionWindowGetNameCopy(Window);
				if (NameTemp != NIL)
					{
						FunctionObjectNewName(Window->FunctionObject,NameTemp);
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

				Temp = FunctionWindowGetSourceCopy(Window);
				if (Temp != NIL)
					{
						FunctionObjectNewSource(Window->FunctionObject,Temp);
						TextEditHasBeenSaved(Window->BodyEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		return SuccessFlag;
	}
