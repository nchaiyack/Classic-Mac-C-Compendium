/* AlgoWaveTableWindow.c */
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

#include "AlgoWaveTableWindow.h"
#include "MainWindowStuff.h"
#include "AlgoWaveTableObject.h"
#include "AlgoWaveTableList.h"
#include "TextEdit.h"
#include "IconButton.h"
#include "WindowDispatcher.h"
#include "Memory.h"
#include "Alert.h"
#include "Numbers.h"
#include "GrowIcon.h"
#include "DataMunging.h"
#include "Main.h"
#include "EditImages.h"
#include "FindDialog.h"
#include "WaveTableSizeDialog.h"
#include "GlobalWindowMenuList.h"


#define WINSIZEX (490)
#define WINSIZEY (300)
#define TITLEINDENT (8)

#define NAMEEDITX (80)
#define NAMEEDITY (1)
#define NAMEEDITWIDTH (80)
#define NAMEEDITHEIGHT (19)

#define NUMFRAMESEDITX (240)
#define NUMFRAMESEDITY (1)
#define NUMFRAMESEDITWIDTH (80)
#define NUMFRAMESEDITHEIGHT (19)

#define NUMTABLESEDITX (400)
#define NUMTABLESEDITY (1)
#define NUMTABLESEDITWIDTH (80)
#define NUMTABLESEDITHEIGHT (19)

#define BITS8X (10)
#define BITS8Y (NUMFRAMESEDITY + NUMFRAMESEDITHEIGHT + 4)
#define BITS8WIDTH (32)
#define BITS8HEIGHT (32)

#define BITS16X (BITS8X + BITS8WIDTH + 1)
#define BITS16Y (BITS8Y)
#define BITS16WIDTH (BITS8WIDTH)
#define BITS16HEIGHT (BITS8HEIGHT)

#define NAMEX (3)
#define NAMEY (4)

#define NUMFRAMESX (170)
#define NUMFRAMESY (4)

#define NUMTABLESX (330)
#define NUMTABLESY (4)

#define FUNCTIONEDITX (-1)
#define FUNCTIONEDITY (BITS16Y + BITS16HEIGHT + 4)
#define FUNCTIONEDITWIDTH(Width) ((Width) + 2)
#define FUNCTIONEDITHEIGHT(Height) ((Height) - FUNCTIONEDITY + 1)


struct AlgoWaveTableWindowRec
	{
		MainWindowRec*					MainWindow;
		AlgoWaveTableObjectRec*	AlgoWaveTableObject;
		AlgoWaveTableListRec*		AlgoWaveTableList;

		WinType*								ScreenID;
		TextEditRec*						NameEdit;
		TextEditRec*						FunctionEdit;
		TextEditRec*						NumFramesEdit;
		TextEditRec*						NumTablesEdit;
		TextEditRec*						ActiveTextEdit;
		IconButtonRec*					Bits8Button;
		IconButtonRec*					Bits16Button;
		MyBoolean								BitsChanged;
		GenericWindowRec*				MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*						MyMenuItem;
	};


/* create a new algorithmic wave table editing window */
AlgoWaveTableWindowRec*	NewAlgoWaveTableWindow(struct MainWindowRec* MainWindow,
											struct AlgoWaveTableObjectRec* AlgoWaveTableObject,
											struct AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType WinX, OrdType WinY, OrdType WinWidth, OrdType WinHeight)
	{
		AlgoWaveTableWindowRec*	Window;
		char*										StringTemp;

		/* deal with window placement */
		if ((WinWidth < 100) || (WinHeight < 100) || ((eOptionKey & CheckModifiers()) != 0))
			{
				WinX = 20 + WindowOtherEdgeWidths(eDocumentWindow);
				WinY = 20 + WindowTitleBarHeight(eDocumentWindow);
				WinWidth = WINSIZEX;
				WinHeight = WINSIZEY;
			}
		MakeWindowFitOnScreen(&WinX,&WinY,&WinWidth,&WinHeight);

		Window = (AlgoWaveTableWindowRec*)AllocPtrCanFail(
			sizeof(AlgoWaveTableWindowRec),"AlgoWaveTableWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to display the algorithmic "
					"wave table editor.",NIL);
				return NIL;
			}
		Window->MainWindow = MainWindow;
		Window->AlgoWaveTableObject = AlgoWaveTableObject;
		Window->AlgoWaveTableList = AlgoWaveTableList;
		Window->BitsChanged = False;

		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,WinX,WinY,WinWidth,WinHeight,
			(void (*)(void*))&AlgoWaveTableWindowUpdator,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}

		/* create name edit */
		Window->NameEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,NAMEEDITX,NAMEEDITY,NAMEEDITWIDTH,NAMEEDITHEIGHT);
		if (Window->NameEdit == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		/* install new text in name edit */
		StringTemp = AlgoWaveTableObjectGetNameCopy(AlgoWaveTableObject);
		if (StringTemp == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->NameEdit);
				goto FailurePoint3;
			}
		TextEditNewRawData(Window->NameEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NameEdit);

		/* create function edit */
		Window->FunctionEdit = NewTextEdit(Window->ScreenID,
			(TEScrollType)(eTEVScrollBar | eTEHScrollBar),GetMonospacedFont(),9,
			FUNCTIONEDITX,FUNCTIONEDITY + GetFontHeight(GetScreenFont(),9),
			FUNCTIONEDITWIDTH(WinWidth),
			FUNCTIONEDITHEIGHT(WinHeight) - GetFontHeight(GetScreenFont(),9));
		if (Window->FunctionEdit == NIL)
			{
			 FailurePoint5:
				goto FailurePoint4;
			}
		SetTextEditTabSize(Window->FunctionEdit,MainWindowGetTabSize(MainWindow));
		StringTemp = AlgoWaveTableObjectGetFormulaCopy(AlgoWaveTableObject);
		if (StringTemp == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->FunctionEdit);
				goto FailurePoint5;
			}
		TextEditNewRawData(Window->FunctionEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->FunctionEdit);
		SetTextEditAutoIndent(Window->FunctionEdit,True);

		Window->NumFramesEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,NUMFRAMESEDITX,NUMFRAMESEDITY,NUMFRAMESEDITWIDTH,NUMFRAMESEDITHEIGHT);
		if (Window->NumFramesEdit == NIL)
			{
			 FailurePoint7:
				goto FailurePoint6;
			}
		StringTemp = IntegerToString(AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObject));
		if (StringTemp == NIL)
			{
			 FailurePoint8:
				DisposeTextEdit(Window->NumFramesEdit);
				goto FailurePoint7;
			}
		TextEditNewRawData(Window->NumFramesEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NumFramesEdit);

		Window->NumTablesEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,NUMTABLESEDITX,NUMTABLESEDITY,NUMTABLESEDITWIDTH,NUMTABLESEDITHEIGHT);
		if (Window->NumTablesEdit == NIL)
			{
			 FailurePoint9:
				goto FailurePoint8;
			}
		StringTemp = IntegerToString(AlgoWaveTableObjectGetNumTables(AlgoWaveTableObject));
		if (StringTemp == NIL)
			{
			 FailurePoint10:
				DisposeTextEdit(Window->NumTablesEdit);
				goto FailurePoint9;
			}
		TextEditNewRawData(Window->NumTablesEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NumTablesEdit);

		Window->Bits8Button = NewIconButtonPreparedBitmaps(Window->ScreenID,BITS8X,BITS8Y,
			BITS8WIDTH,BITS8HEIGHT,Bits8Unselected,Bits8MouseDown,Bits8Selected,Bits8Selected,
			eIconRadioMode);
		if (Window->Bits8Button == NIL)
			{
			 FailurePoint11:
				goto FailurePoint10;
			}
		Window->Bits16Button = NewIconButtonPreparedBitmaps(Window->ScreenID,BITS16X,BITS16Y,
			BITS16WIDTH,BITS16HEIGHT,Bits16Unselected,Bits16MouseDown,Bits16Selected,
			Bits16Selected,eIconRadioMode);
		if (Window->Bits16Button == NIL)
			{
			 FailurePoint12:
				DisposeIconButton(Window->Bits8Button);
				goto FailurePoint11;
			}
		switch (AlgoWaveTableObjectGetNumBits(AlgoWaveTableObject))
			{
				case eSample16bit:
					SetIconButtonState(Window->Bits16Button,True);
					break;
				case eSample8bit:
					SetIconButtonState(Window->Bits8Button,True);
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"NewAlgoWaveTableWindow:  bad num bits"));
					break;
			}

		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&AlgoWaveTableWindowDoIdle,
			(void (*)(void*))&AlgoWaveTableWindowBecomeActive,
			(void (*)(void*))&AlgoWaveTableWindowBecomeInactive,
			(void (*)(void*))&AlgoWaveTableWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&AlgoWaveTableWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&AlgoWaveTableWindowDoKeyDown,
			(void (*)(void*))&AlgoWaveTableWindowClose,
			(void (*)(void*))&AlgoWaveTableWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&AlgoWaveTableWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint13:
				DisposeIconButton(Window->Bits16Button);
				goto FailurePoint12;
			}

		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"x",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint14:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint13;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint15:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint14;
			}

		Window->ActiveTextEdit = Window->FunctionEdit;
		AlgoWaveTableWindowResetTitlebar(Window);

		return Window;
	}


/* write data back to the object and dispose of the window */
void								DisposeAlgoWaveTableWindow(AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);

		/* save the data */
		if (!AlgoWaveTableWindowWritebackModifiedData(Window))
			{
				/* failed -- now what? */
			}

		AlgoWaveTableObjectClosingWindowNotify(Window->AlgoWaveTableObject,
			GetWindowXStart(Window->ScreenID),GetWindowYStart(Window->ScreenID),
			GetWindowWidth(Window->ScreenID),GetWindowHeight(Window->ScreenID));
		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		DisposeTextEdit(Window->NameEdit);
		DisposeTextEdit(Window->FunctionEdit);
		DisposeTextEdit(Window->NumFramesEdit);
		DisposeTextEdit(Window->NumTablesEdit);
		DisposeIconButton(Window->Bits16Button);
		DisposeIconButton(Window->Bits8Button);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


/* find out if the wave table has been modified since the last file save */
MyBoolean						HasAlgoWaveTableWindowBeenModified(AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->BitsChanged
			|| TextEditDoesItNeedToBeSaved(Window->NameEdit)
			|| TextEditDoesItNeedToBeSaved(Window->FunctionEdit)
			|| TextEditDoesItNeedToBeSaved(Window->NumFramesEdit)
			|| TextEditDoesItNeedToBeSaved(Window->NumTablesEdit);
	}


/* highlight a line in the formula edit box */
void								AlgoWaveTableWindowHiliteLine(AlgoWaveTableWindowRec* Window,
											long LineNumber)
	{
		CheckPtrExistence(Window);
		if (Window->ActiveTextEdit != Window->FunctionEdit)
			{
				DisableTextEditSelection(Window->ActiveTextEdit);
				Window->ActiveTextEdit = Window->FunctionEdit;
				EnableTextEditSelection(Window->ActiveTextEdit);
			}
		SetTextEditSelection(Window->FunctionEdit,LineNumber,0,LineNumber + 1,0);
		TextEditShowSelection(Window->FunctionEdit);
	}


/* bring the window to the top and give it the focus */
void								AlgoWaveTableWindowBringToTop(AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		ActivateThisWindow(Window->ScreenID);
	}


void								AlgoWaveTableWindowDoIdle(AlgoWaveTableWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->ActiveTextEdit);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->NameEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->FunctionEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->NumFramesEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->NumTablesEdit,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								AlgoWaveTableWindowBecomeActive(AlgoWaveTableWindowRec* Window)
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


void								AlgoWaveTableWindowBecomeInactive(AlgoWaveTableWindowRec* Window)
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


void								AlgoWaveTableWindowJustResized(AlgoWaveTableWindowRec* Window)
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
		SetTextEditPosition(Window->FunctionEdit,FUNCTIONEDITX,FUNCTIONEDITY + FontHeight,
			FUNCTIONEDITWIDTH(XSize),FUNCTIONEDITHEIGHT(YSize) - FontHeight);
	}


void								AlgoWaveTableWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				AlgoWaveTableWindowJustResized(Window);
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
		else if (TextEditHitTest(Window->FunctionEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->FunctionEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->FunctionEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->FunctionEdit,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->NumFramesEdit,XLoc,YLoc))
			{
				long							OldNumFrames;
				long							NewNumFrames;
				char*							NewStringTemp;

				OldNumFrames = AlgoWaveTableWindowGetNumFrames(Window);
				NewNumFrames = AskForNewWaveTableSize(OldNumFrames);
				NewStringTemp = IntegerToString(NewNumFrames);
				if (NewStringTemp != NIL)
					{
						TextEditNewRawData(Window->NumFramesEdit,NewStringTemp,"\x0a");
						ReleasePtr(NewStringTemp);
					}
			}
		else if (TextEditHitTest(Window->NumTablesEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->NumTablesEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->NumTablesEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->NumTablesEdit,XLoc,YLoc,Modifiers);
			}
		else if (IconButtonHitTest(Window->Bits8Button,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->Bits8Button,XLoc,YLoc,NIL,NIL))
					{
						Window->BitsChanged = True;
						SetIconButtonState(Window->Bits16Button,False);
					}
			}
		else if (IconButtonHitTest(Window->Bits16Button,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->Bits16Button,XLoc,YLoc,NIL,NIL))
					{
						Window->BitsChanged = True;
						SetIconButtonState(Window->Bits8Button,False);
					}
			}
	}


void								AlgoWaveTableWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers,AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditDoKeyPressed(Window->ActiveTextEdit,KeyCode,Modifiers);
	}


void								AlgoWaveTableWindowClose(AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DisposeAlgoWaveTableWindow(Window);
	}


void								AlgoWaveTableWindowUpdator(AlgoWaveTableWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;
		FontType					ScreenFont;

		CheckPtrExistence(Window);
		ScreenFont = GetScreenFont();
		TextEditFullRedraw(Window->NameEdit);
		TextEditFullRedraw(Window->FunctionEdit);
		TextEditFullRedraw(Window->NumFramesEdit);
		TextEditFullRedraw(Window->NumTablesEdit);
		RedrawIconButton(Window->Bits16Button);
		RedrawIconButton(Window->Bits8Button);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Name:",5,NAMEX,NAMEY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Num Tables:",11,
			NUMTABLESX,NUMTABLESY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Num Frames:",11,
			NUMFRAMESX,NUMFRAMESY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Waveform Generating Function:",29,
			FUNCTIONEDITX + TITLEINDENT,FUNCTIONEDITY,eBold);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


void								AlgoWaveTableWindowMenuSetup(AlgoWaveTableWindowRec* Window)
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
		ChangeItemName(mCloseFile,"Close Algorithmic Wave Table Editor");
		EnableMenuItem(mCloseFile);
		ChangeItemName(mBuildFunction,"Build Wave Table");
		EnableMenuItem(mBuildFunction);
		ChangeItemName(mUnbuildFunction,"Unbuild Wave Table");
		EnableMenuItem(mUnbuildFunction);
		ChangeItemName(mAlgoSampToSample,"Open Algorithmic Wave Table As New Wave Table");
		EnableMenuItem(mAlgoSampToSample);
		ChangeItemName(mDeleteObject,"Delete Algorithmic Wave Table");
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


void								AlgoWaveTableWindowDoMenuCommand(AlgoWaveTableWindowRec* Window,
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
				AlgoWaveTableWindowClose(Window);
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
				AlgoWaveTableObjectBuild(Window->AlgoWaveTableObject);
			}
		else if (MenuItem == mUnbuildFunction)
			{
				AlgoWaveTableObjectUnbuild(Window->AlgoWaveTableObject);
			}
		else if (MenuItem == mAlgoSampToSample)
			{
				if (AlgoWaveTableObjectMakeUpToDate(Window->AlgoWaveTableObject))
					{
						char*					Buffer;
						long					NumTables;
						long					SliceSize;
						long					BytesPerSlice;

						NumTables = AlgoWaveTableObjectGetNumTables(Window->AlgoWaveTableObject);
						SliceSize = AlgoWaveTableObjectGetNumFrames(Window->AlgoWaveTableObject);
						BytesPerSlice = SliceSize;
						if (eSample16bit == AlgoWaveTableObjectGetNumBits(Window->AlgoWaveTableObject))
							{
								BytesPerSlice *= 2;
							}
						Buffer = AllocPtrCanFail(BytesPerSlice * NumTables,"AlgoWaveTableTemp");
						if (Buffer == NIL)
							{
								AlertHalt("There is not enough memory to convert the wave table.",NIL);
							}
						 else
							{
								long					Scan;

								for (Scan = 0; Scan < NumTables; Scan += 1)
									{
										char*					OneSlice;

										OneSlice = AlgoWaveTableObjectGetRawSlice(
											Window->AlgoWaveTableObject,Scan);
										PRNGCHK(Buffer,&(Buffer[Scan * BytesPerSlice]),BytesPerSlice);
										CopyData(OneSlice,&(Buffer[Scan * BytesPerSlice]),BytesPerSlice);
									}
								MainWindowCopyRawWaveTableAndOpen(Window->MainWindow,Buffer,
									AlgoWaveTableObjectGetNumBits(Window->AlgoWaveTableObject),
									NumTables,SliceSize);
								ReleasePtr(Buffer);
							}
					}
			}
		else if (MenuItem == mDeleteObject)
			{
				AlgoWaveTableListDeleteAlgoWaveTable(Window->AlgoWaveTableList,
					Window->AlgoWaveTableObject);
			}
		else if (MenuItem == mFind)
			{
				if (Window->ActiveTextEdit != Window->FunctionEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->FunctionEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				switch (DoFindDialog(&GlobalSearchString,&GlobalReplaceString,
					mCut,mPaste,mCopy,mUndo,mSelectAll,mClear))
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"AlgoWaveTableWindowDoMenuCommand:  bad value from DoFindDialog"));
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
				if (Window->ActiveTextEdit != Window->FunctionEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->FunctionEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditFindAgain(Window->ActiveTextEdit,GlobalSearchString);
				TextEditShowSelection(Window->ActiveTextEdit);
			}
		else if (MenuItem == mReplace)
			{
				if (Window->ActiveTextEdit != Window->FunctionEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->FunctionEdit;
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
				if (Window->ActiveTextEdit != Window->FunctionEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->FunctionEdit;
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
				EXECUTE(PRERR(AllowResume,"AlgoWaveTableWindowDoMenuCommand:  unknown menu command"));
			}
	}


/* get a copy of the algorithmic wave table's name */
char*								AlgoWaveTableWindowGetNameCopy(AlgoWaveTableWindowRec* Window)
	{
		char*							ReturnValue;

		CheckPtrExistence(Window);
		ReturnValue = TextEditGetRawData(Window->NameEdit,"\x0a");
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"AlgoWaveTableNameCopy");
			}
		return ReturnValue;
	}


/* get a copy of the algorithmic wave table's formula */
char*								AlgoWaveTableWindowGetFormulaCopy(AlgoWaveTableWindowRec* Window)
	{
		char*							ReturnValue;

		CheckPtrExistence(Window);
		ReturnValue = TextEditGetRawData(Window->FunctionEdit,"\x0a");
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"AlgoWaveTableFormulaCopy");
			}
		return ReturnValue;
	}


/* get the number of bits used for this algorithmic wave table */
NumBitsType					AlgoWaveTableWindowGetNumBits(AlgoWaveTableWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if (GetIconButtonState(Window->Bits16Button))
			{
				return eSample16bit;
			}
		 else
			{
				return eSample8bit;
			}
	}


/* get the number of periods in the table */
long								AlgoWaveTableWindowGetNumTables(AlgoWaveTableWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->NumTablesEdit,"\x0a");
		if (StringTemp != NIL)
			{
				ReturnValue = StringToInteger(StringTemp,PtrSize(StringTemp));
				ReleasePtr(StringTemp);
			}
		 else
			{
				ReturnValue = 0;
			}
		if (ReturnValue < 0)
			{
				ReturnValue = 0;
			}
		return ReturnValue;
	}


/* get the number of frames in each period */
long								AlgoWaveTableWindowGetNumFrames(AlgoWaveTableWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->NumFramesEdit,"\x0a");
		if (StringTemp != NIL)
			{
				ReturnValue = StringToInteger(StringTemp,PtrSize(StringTemp));
				ReleasePtr(StringTemp);
			}
		 else
			{
				ReturnValue = 0;
			}
		return ReturnValue;
	}


/* the name of the file has changed, so update the title bar of the window.  the */
/* NewFilename is a non-null-terminated string which must be disposed by the caller */
void								AlgoWaveTableWindowGlobalNameChange(AlgoWaveTableWindowRec* Window,
											char* NewFilename)
	{
		char*							LocalNameCopy;

		CheckPtrExistence(Window);
		CheckPtrExistence(NewFilename);
		LocalNameCopy = AlgoWaveTableWindowGetNameCopy(Window);
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


/* reset the title bar name of the window even if the filename hasn't changed */
void								AlgoWaveTableWindowResetTitlebar(AlgoWaveTableWindowRec* Window)
	{
		char*							DocumentName;

		CheckPtrExistence(Window);
		DocumentName = GetCopyOfDocumentName(Window->MainWindow);
		if (DocumentName != NIL)
			{
				AlgoWaveTableWindowGlobalNameChange(Window,DocumentName);
				ReleasePtr(DocumentName);
			}
	}


/* write back all modified data to the object */
MyBoolean						AlgoWaveTableWindowWritebackModifiedData(AlgoWaveTableWindowRec* Window)
	{
		MyBoolean					SuccessFlag = True;

		CheckPtrExistence(Window);

		if (TextEditDoesItNeedToBeSaved(Window->NameEdit))
			{
				char*						String;

				String = AlgoWaveTableWindowGetNameCopy(Window);
				if (String != NIL)
					{
						AlgoWaveTableObjectNewName(Window->AlgoWaveTableObject,String);
						TextEditHasBeenSaved(Window->NameEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		if (TextEditDoesItNeedToBeSaved(Window->FunctionEdit))
			{
				char*						String;

				String = AlgoWaveTableWindowGetFormulaCopy(Window);
				if (String != NIL)
					{
						AlgoWaveTableObjectNewFormula(Window->AlgoWaveTableObject,String);
						TextEditHasBeenSaved(Window->FunctionEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		if (TextEditDoesItNeedToBeSaved(Window->NumFramesEdit))
			{
				AlgoWaveTableObjectPutNumFrames(Window->AlgoWaveTableObject,
					AlgoWaveTableWindowGetNumFrames(Window));
				TextEditHasBeenSaved(Window->NumFramesEdit);
			}

		if (TextEditDoesItNeedToBeSaved(Window->NumTablesEdit))
			{
				AlgoWaveTableObjectPutNumTables(Window->AlgoWaveTableObject,
					AlgoWaveTableWindowGetNumTables(Window));
				TextEditHasBeenSaved(Window->NumTablesEdit);
			}

		if (Window->BitsChanged)
			{
				AlgoWaveTableObjectPutNumBits(Window->AlgoWaveTableObject,
					AlgoWaveTableWindowGetNumBits(Window));
				Window->BitsChanged = False;
			}

		return SuccessFlag;
	}
