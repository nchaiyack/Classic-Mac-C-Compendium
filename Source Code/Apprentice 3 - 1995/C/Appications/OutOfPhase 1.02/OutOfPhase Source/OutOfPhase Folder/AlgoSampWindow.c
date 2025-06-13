/* AlgoSampWindow.c */
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

#include "AlgoSampWindow.h"
#include "MainWindowStuff.h"
#include "AlgoSampObject.h"
#include "TextEdit.h"
#include "IconButton.h"
#include "WindowDispatcher.h"
#include "Memory.h"
#include "Alert.h"
#include "Numbers.h"
#include "GrowIcon.h"
#include "DataMunging.h"
#include "Main.h"
#include "AlgoSampList.h"
#include "EditImages.h"
#include "FindDialog.h"
#include "GlobalWindowMenuList.h"


#define WINSIZEX (490)
#define WINSIZEY (300)
#define TITLEINDENT (8)

#define NAMEEDITX (80)
#define NAMEEDITY (1)
#define NAMEEDITWIDTH (80)
#define NAMEEDITHEIGHT (19)

#define SAMPLINGRATEEDITX (80)
#define SAMPLINGRATEEDITY (21)
#define SAMPLINGRATEEDITWIDTH (80)
#define SAMPLINGRATEEDITHEIGHT (19)

#define ORIGINEDITX (240)
#define ORIGINEDITY (1)
#define ORIGINEDITWIDTH (80)
#define ORIGINEDITHEIGHT (19)

#define BASEFREQEDITX (240)
#define BASEFREQEDITY (21)
#define BASEFREQEDITWIDTH (80)
#define BASEFREQEDITHEIGHT (19)

#define LOOPSTARTEDIT1X (400)
#define LOOPSTARTEDIT1Y (1)
#define LOOPSTARTEDIT1WIDTH (80)
#define LOOPSTARTEDIT1HEIGHT (19)

#define LOOPSTARTEDIT2X (240)
#define LOOPSTARTEDIT2Y (42)
#define LOOPSTARTEDIT2WIDTH (80)
#define LOOPSTARTEDIT2HEIGHT (19)

#define LOOPSTARTEDIT3X (400)
#define LOOPSTARTEDIT3Y (42)
#define LOOPSTARTEDIT3WIDTH (80)
#define LOOPSTARTEDIT3HEIGHT (19)

#define LOOPENDEDIT1X (400)
#define LOOPENDEDIT1Y (21)
#define LOOPENDEDIT1WIDTH (80)
#define LOOPENDEDIT1HEIGHT (19)

#define LOOPENDEDIT2X (240)
#define LOOPENDEDIT2Y (63)
#define LOOPENDEDIT2WIDTH (80)
#define LOOPENDEDIT2HEIGHT (19)

#define LOOPENDEDIT3X (400)
#define LOOPENDEDIT3Y (63)
#define LOOPENDEDIT3WIDTH (80)
#define LOOPENDEDIT3HEIGHT (19)

#define BITS8X (10)
#define BITS8Y (45)
#define BITS8WIDTH (32)
#define BITS8HEIGHT (32)

#define BITS16X (BITS8X + BITS8WIDTH + 1)
#define BITS16Y (BITS8Y)
#define BITS16WIDTH (BITS8WIDTH)
#define BITS16HEIGHT (BITS8HEIGHT)

#define MONOX (BITS16X + BITS16WIDTH + 10)
#define MONOY (BITS16Y)
#define MONOWIDTH (BITS16WIDTH)
#define MONOHEIGHT (BITS16HEIGHT)

#define STEREOX (MONOX + MONOWIDTH + 1)
#define STEREOY (MONOY)
#define STEREOWIDTH (MONOWIDTH)
#define STEREOHEIGHT (MONOHEIGHT)

#define SAMPLENAMEX (3)
#define SAMPLENAMEY (4)

#define SAMPLERATEX (3)
#define SAMPLERATEY (23)

#define ORIGINX (170)
#define ORIGINY (4)

#define BASEFREQX (170)
#define BASEFREQY (23)

#define LOOPSTART1X (330)
#define LOOPSTART1Y (4)

#define LOOPSTART2X (170)
#define LOOPSTART2Y (44)

#define LOOPSTART3X (330)
#define LOOPSTART3Y (44)

#define LOOPEND1X (330)
#define LOOPEND1Y (23)

#define LOOPEND2X (170)
#define LOOPEND2Y (65)

#define LOOPEND3X (330)
#define LOOPEND3Y (65)

#define BODYX (-1)
#define BODYY (STEREOY + STEREOHEIGHT + 4)
#define BODYWIDTH(Width) ((Width) + 2)
#define BODYHEIGHT(Height) ((Height) - BODYY + 1)


struct AlgoSampWindowRec
	{
		MainWindowRec*			MainWindow;
		AlgoSampObjectRec*	AlgoSampObject;
		AlgoSampListRec*		AlgoSampList;

		WinType*						ScreenID;
		TextEditRec*				NameEdit;
		TextEditRec*				BodyEdit;
		TextEditRec*				SamplingRateEdit;
		TextEditRec*				OriginEdit;
		TextEditRec*				NaturalFrequencyEdit;
		TextEditRec*				LoopStartEdit1;
		TextEditRec*				LoopStartEdit2;
		TextEditRec*				LoopStartEdit3;
		TextEditRec*				LoopEndEdit1;
		TextEditRec*				LoopEndEdit2;
		TextEditRec*				LoopEndEdit3;
		TextEditRec*				ActiveTextEdit;
		IconButtonRec*			MonoButton;
		IconButtonRec*			StereoButton;
		MyBoolean						ChannelsButtonChanged;
		IconButtonRec*			Bits8Button;
		IconButtonRec*			Bits16Button;
		MyBoolean						BitsButtonChanged;
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;
	};


/* allocate a new algorithmic sample window */
AlgoSampWindowRec*	NewAlgoSampWindow(struct MainWindowRec* MainWindow,
											struct AlgoSampObjectRec* AlgoSampObject,
											struct AlgoSampListRec* AlgoSampList, OrdType WinX, OrdType WinY,
											OrdType WinWidth, OrdType WinHeight)
	{
		AlgoSampWindowRec*	Window;
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

		Window = (AlgoSampWindowRec*)AllocPtrCanFail(
			sizeof(AlgoSampWindowRec),"AlgoSampWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to display the algorithmic "
					"sample editor.",NIL);
				return NIL;
			}
		Window->MainWindow = MainWindow;
		Window->AlgoSampObject = AlgoSampObject;
		Window->AlgoSampList = AlgoSampList;
		Window->ChannelsButtonChanged = False;
		Window->BitsButtonChanged = False;

		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,WinX,WinY,WinWidth,WinHeight,
			(void (*)(void*))&AlgoSampWindowUpdator,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		FontHeight = GetFontHeight(GetScreenFont(),9);

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
		StringTemp = AlgoSampObjectGetNameCopy(AlgoSampObject);
		if (StringTemp == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->NameEdit);
				goto FailurePoint3;
			}
		TextEditNewRawData(Window->NameEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NameEdit);

		/* create sampling rate edit */
		Window->SamplingRateEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,SAMPLINGRATEEDITX,SAMPLINGRATEEDITY,
			SAMPLINGRATEEDITWIDTH,SAMPLINGRATEEDITHEIGHT);
		if (Window->SamplingRateEdit == NIL)
			{
			 FailurePoint5:
				goto FailurePoint4;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetSamplingRate(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->SamplingRateEdit);
				goto FailurePoint5;
			}
		TextEditNewRawData(Window->SamplingRateEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->SamplingRateEdit);

		/* create origin edit */
		Window->OriginEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,ORIGINEDITX,ORIGINEDITY,ORIGINEDITWIDTH,ORIGINEDITHEIGHT);
		if (Window->OriginEdit == NIL)
			{
			 FailurePoint7:
				goto FailurePoint6;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetOrigin(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint8:
				DisposeTextEdit(Window->OriginEdit);
				goto FailurePoint7;
			}
		TextEditNewRawData(Window->OriginEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->OriginEdit);

		/* create base frequency edit */
		Window->NaturalFrequencyEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,BASEFREQEDITX,BASEFREQEDITY,BASEFREQEDITWIDTH,
			BASEFREQEDITHEIGHT);
		if (Window->NaturalFrequencyEdit == NIL)
			{
			 FailurePoint9:
				goto FailurePoint8;
			}
		StringTemp = LongDoubleToString(AlgoSampObjectGetNaturalFrequency(AlgoSampObject),
			13,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint10:
				DisposeTextEdit(Window->NaturalFrequencyEdit);
				goto FailurePoint9;
			}
		TextEditNewRawData(Window->NaturalFrequencyEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->NaturalFrequencyEdit);

		/* create loop start edit */
		Window->LoopStartEdit1 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPSTARTEDIT1X,LOOPSTARTEDIT1Y,LOOPSTARTEDIT1WIDTH,
			LOOPSTARTEDIT1HEIGHT);
		if (Window->LoopStartEdit1 == NIL)
			{
			 FailurePoint11:
				goto FailurePoint10;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopStart1(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint12:
				DisposeTextEdit(Window->LoopStartEdit1);
				goto FailurePoint11;
			}
		TextEditNewRawData(Window->LoopStartEdit1,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopStartEdit1);

		Window->LoopStartEdit2 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPSTARTEDIT2X,LOOPSTARTEDIT2Y,LOOPSTARTEDIT2WIDTH,
			LOOPSTARTEDIT2HEIGHT);
		if (Window->LoopStartEdit2 == NIL)
			{
			 FailurePoint13:
				goto FailurePoint12;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopStart2(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint14:
				DisposeTextEdit(Window->LoopStartEdit2);
				goto FailurePoint13;
			}
		TextEditNewRawData(Window->LoopStartEdit2,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopStartEdit2);

		Window->LoopStartEdit3 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPSTARTEDIT3X,LOOPSTARTEDIT3Y,LOOPSTARTEDIT3WIDTH,
			LOOPSTARTEDIT3HEIGHT);
		if (Window->LoopStartEdit3 == NIL)
			{
			 FailurePoint15:
				goto FailurePoint14;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopStart3(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint16:
				DisposeTextEdit(Window->LoopStartEdit3);
				goto FailurePoint15;
			}
		TextEditNewRawData(Window->LoopStartEdit3,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopStartEdit3);

		/* create loop end edit */
		Window->LoopEndEdit1 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPENDEDIT1X,LOOPENDEDIT1Y,LOOPENDEDIT1WIDTH,LOOPENDEDIT1HEIGHT);
		if (Window->LoopEndEdit1 == NIL)
			{
			 FailurePoint17:
				goto FailurePoint16;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopEnd1(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint18:
				DisposeTextEdit(Window->LoopEndEdit1);
				goto FailurePoint17;
			}
		TextEditNewRawData(Window->LoopEndEdit1,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopEndEdit1);

		Window->LoopEndEdit2 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPENDEDIT2X,LOOPENDEDIT2Y,LOOPENDEDIT2WIDTH,LOOPENDEDIT2HEIGHT);
		if (Window->LoopEndEdit2 == NIL)
			{
			 FailurePoint19:
				goto FailurePoint18;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopEnd2(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint20:
				DisposeTextEdit(Window->LoopEndEdit2);
				goto FailurePoint19;
			}
		TextEditNewRawData(Window->LoopEndEdit2,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopEndEdit2);

		Window->LoopEndEdit3 = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,LOOPENDEDIT3X,LOOPENDEDIT3Y,LOOPENDEDIT3WIDTH,LOOPENDEDIT3HEIGHT);
		if (Window->LoopEndEdit3 == NIL)
			{
			 FailurePoint21:
				goto FailurePoint19;
			}
		StringTemp = IntegerToString(AlgoSampObjectGetLoopEnd3(AlgoSampObject));
		if (StringTemp == NIL)
			{
			 FailurePoint22:
				DisposeTextEdit(Window->LoopEndEdit3);
				goto FailurePoint20;
			}
		TextEditNewRawData(Window->LoopEndEdit3,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoopEndEdit3);

		Window->MonoButton = NewIconButtonPreparedBitmaps(Window->ScreenID,MONOX,MONOY,
			MONOWIDTH,MONOHEIGHT,MonoUnselected,MonoMouseDown,MonoSelected,MonoSelected,
			eIconRadioMode);
		if (Window->MonoButton == NIL)
			{
			 FailurePoint23:
				goto FailurePoint22;
			}

		Window->StereoButton = NewIconButtonPreparedBitmaps(Window->ScreenID,STEREOX,STEREOY,
			STEREOWIDTH,STEREOHEIGHT,StereoUnselected,StereoMouseDown,StereoSelected,
			StereoSelected,eIconRadioMode);
		if (Window->StereoButton == NIL)
			{
			 FailurePoint24:
				DisposeIconButton(Window->MonoButton);
				goto FailurePoint23;
			}
		switch (AlgoSampObjectGetNumChannels(AlgoSampObject))
			{
				case eSampleStereo:
					SetIconButtonState(Window->StereoButton,True);
					break;
				case eSampleMono:
					SetIconButtonState(Window->MonoButton,True);
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"NewAlgoSampWindow:  bad num channels value"));
					break;
			}

		Window->Bits8Button = NewIconButtonPreparedBitmaps(Window->ScreenID,BITS8X,BITS8Y,
			BITS8WIDTH,BITS8HEIGHT,Bits8Unselected,Bits8MouseDown,Bits8Selected,Bits8Selected,
			eIconRadioMode);
		if (Window->Bits8Button == NIL)
			{
			 FailurePoint25:
				DisposeIconButton(Window->StereoButton);
				goto FailurePoint24;
			}

		Window->Bits16Button = NewIconButtonPreparedBitmaps(Window->ScreenID,BITS16X,BITS16Y,
			BITS16WIDTH,BITS16HEIGHT,Bits16Unselected,Bits16MouseDown,Bits16Selected,
			Bits16Selected,eIconRadioMode);
		if (Window->Bits16Button == NIL)
			{
			 FailurePoint26:
				DisposeIconButton(Window->Bits8Button);
				goto FailurePoint25;
			}
		switch (AlgoSampObjectGetNumBits(AlgoSampObject))
			{
				case eSample16bit:
					SetIconButtonState(Window->Bits16Button,True);
					break;
				case eSample8bit:
					SetIconButtonState(Window->Bits8Button,True);
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"NewAlgoSampWindow:  bad num bits value"));
					break;
			}

		Window->BodyEdit = NewTextEdit(Window->ScreenID,
			(TEScrollType)(eTEVScrollBar | eTEHScrollBar),GetMonospacedFont(),9,
			BODYX,BODYY + FontHeight,BODYWIDTH(WinWidth),BODYHEIGHT(WinHeight) - FontHeight);
		if (Window->BodyEdit == NIL)
			{
			 FailurePoint27:
				DisposeIconButton(Window->Bits16Button);
				goto FailurePoint26;
			}
		SetTextEditTabSize(Window->BodyEdit,MainWindowGetTabSize(MainWindow));
		StringTemp = AlgoSampObjectGetFormulaCopy(AlgoSampObject);
		if (StringTemp == NIL)
			{
			 FailurePoint28:
				DisposeTextEdit(Window->BodyEdit);
				goto FailurePoint27;
			}
		TextEditNewRawData(Window->BodyEdit,StringTemp,"\x0a");
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->BodyEdit);
		SetTextEditAutoIndent(Window->BodyEdit,True);

		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&AlgoSampWindowDoIdle,
			(void (*)(void*))&AlgoSampWindowBecomeActive,
			(void (*)(void*))&AlgoSampWindowBecomeInactive,
			(void (*)(void*))&AlgoSampWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&AlgoSampWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&AlgoSampWindowDoKeyDown,
			(void (*)(void*))&AlgoSampWindowClose,
			(void (*)(void*))&AlgoSampWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&AlgoSampWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint29:
				goto FailurePoint28;
			}

		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"x",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint30:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint29;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint31:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint30;
			}

		AlgoSampWindowResetTitlebar(Window);
		Window->ActiveTextEdit = Window->BodyEdit;

		return Window;
	}


/* write back modified data and dispose of the window */
void								DisposeAlgoSampWindow(AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);

		/* save data */
		if (!AlgoSampWindowWritebackModifiedData(Window))
			{
				/* failed -- now what? */
			}

		AlgoSampObjectClosingWindowNotify(Window->AlgoSampObject,
			GetWindowXStart(Window->ScreenID),GetWindowYStart(Window->ScreenID),
			GetWindowWidth(Window->ScreenID),GetWindowHeight(Window->ScreenID));
		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		DisposeTextEdit(Window->NameEdit);
		DisposeTextEdit(Window->BodyEdit);
		DisposeTextEdit(Window->SamplingRateEdit);
		DisposeTextEdit(Window->OriginEdit);
		DisposeTextEdit(Window->NaturalFrequencyEdit);
		DisposeTextEdit(Window->LoopStartEdit1);
		DisposeTextEdit(Window->LoopStartEdit2);
		DisposeTextEdit(Window->LoopStartEdit3);
		DisposeTextEdit(Window->LoopEndEdit1);
		DisposeTextEdit(Window->LoopEndEdit2);
		DisposeTextEdit(Window->LoopEndEdit3);
		DisposeIconButton(Window->StereoButton);
		DisposeIconButton(Window->MonoButton);
		DisposeIconButton(Window->Bits16Button);
		DisposeIconButton(Window->Bits8Button);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


/* return True if the data in the window has been changed since the last */
/* time the file was saved. */
MyBoolean						HasAlgoSampWindowBeenModified(AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->BitsButtonChanged || Window->ChannelsButtonChanged
			|| TextEditDoesItNeedToBeSaved(Window->NameEdit)
			|| TextEditDoesItNeedToBeSaved(Window->BodyEdit)
			|| TextEditDoesItNeedToBeSaved(Window->SamplingRateEdit)
			|| TextEditDoesItNeedToBeSaved(Window->OriginEdit)
			|| TextEditDoesItNeedToBeSaved(Window->NaturalFrequencyEdit)
			|| TextEditDoesItNeedToBeSaved(Window->LoopStartEdit1)
			|| TextEditDoesItNeedToBeSaved(Window->LoopStartEdit2)
			|| TextEditDoesItNeedToBeSaved(Window->LoopStartEdit3)
			|| TextEditDoesItNeedToBeSaved(Window->LoopEndEdit1)
			|| TextEditDoesItNeedToBeSaved(Window->LoopEndEdit2)
			|| TextEditDoesItNeedToBeSaved(Window->LoopEndEdit3);
	}


/* highlight the specified line number in the formula window. */
void								AlgoSampWindowHiliteLine(AlgoSampWindowRec* Window,
											long LineNumber)
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


/* bring the window to the top and give it the focus */
void								AlgoSampWindowBringToTop(AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		ActivateThisWindow(Window->ScreenID);
	}


void								AlgoSampWindowDoIdle(AlgoSampWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->ActiveTextEdit);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->NameEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->BodyEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->SamplingRateEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->OriginEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->NaturalFrequencyEdit,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopStartEdit1,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopStartEdit2,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopStartEdit3,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopEndEdit1,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopEndEdit2,XLoc,YLoc)
					|| TextEditIBeamTest(Window->LoopEndEdit3,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								AlgoSampWindowBecomeActive(AlgoSampWindowRec* Window)
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


void								AlgoSampWindowBecomeInactive(AlgoSampWindowRec* Window)
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


void								AlgoSampWindowJustResized(AlgoSampWindowRec* Window)
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
		SetTextEditPosition(Window->BodyEdit,
			BODYX,BODYY + FontHeight,BODYWIDTH(XSize),BODYHEIGHT(YSize) - FontHeight);
	}


void								AlgoSampWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				AlgoSampWindowJustResized(Window);
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
		else if (TextEditHitTest(Window->SamplingRateEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->SamplingRateEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->SamplingRateEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->SamplingRateEdit,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->OriginEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->OriginEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->OriginEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->OriginEdit,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->NaturalFrequencyEdit,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->NaturalFrequencyEdit)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->NaturalFrequencyEdit;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->NaturalFrequencyEdit,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopStartEdit1,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopStartEdit1)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopStartEdit1;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopStartEdit1,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopStartEdit2,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopStartEdit2)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopStartEdit2;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopStartEdit2,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopStartEdit3,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopStartEdit3)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopStartEdit3;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopStartEdit3,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopEndEdit1,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopEndEdit1)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopEndEdit1;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopEndEdit1,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopEndEdit2,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopEndEdit2)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopEndEdit2;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopEndEdit2,XLoc,YLoc,Modifiers);
			}
		else if (TextEditHitTest(Window->LoopEndEdit3,XLoc,YLoc))
			{
				if (Window->ActiveTextEdit != Window->LoopEndEdit3)
					{
						DisableTextEditSelection(Window->ActiveTextEdit);
						Window->ActiveTextEdit = Window->LoopEndEdit3;
						EnableTextEditSelection(Window->ActiveTextEdit);
					}
				TextEditDoMouseDown(Window->LoopEndEdit3,XLoc,YLoc,Modifiers);
			}
		else if (IconButtonHitTest(Window->MonoButton,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->MonoButton,XLoc,YLoc,NIL,NIL))
					{
						Window->ChannelsButtonChanged = True;
						SetIconButtonState(Window->StereoButton,False);
					}
			}
		else if (IconButtonHitTest(Window->StereoButton,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->StereoButton,XLoc,YLoc,NIL,NIL))
					{
						Window->ChannelsButtonChanged = True;
						SetIconButtonState(Window->MonoButton,False);
					}
			}
		else if (IconButtonHitTest(Window->Bits8Button,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->Bits8Button,XLoc,YLoc,NIL,NIL))
					{
						Window->BitsButtonChanged = True;
						SetIconButtonState(Window->Bits16Button,False);
					}
			}
		else if (IconButtonHitTest(Window->Bits16Button,XLoc,YLoc))
			{
				if (IconButtonMouseDown(Window->Bits16Button,XLoc,YLoc,NIL,NIL))
					{
						Window->BitsButtonChanged = True;
						SetIconButtonState(Window->Bits8Button,False);
					}
			}
	}


void								AlgoSampWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers,AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((KeyCode == 9) && (Window->ActiveTextEdit != Window->BodyEdit))
			{
				DisableTextEditSelection(Window->ActiveTextEdit);
				if ((Modifiers & eShiftKey) != 0)
					{
						/* backwards */
						if (Window->ActiveTextEdit == Window->LoopEndEdit3)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit3;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit3)
							{
								Window->ActiveTextEdit = Window->LoopEndEdit2;
							}
						else if (Window->ActiveTextEdit == Window->LoopEndEdit2)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit2;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit2)
							{
								Window->ActiveTextEdit = Window->LoopEndEdit1;
							}
						else if (Window->ActiveTextEdit == Window->LoopEndEdit1)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit1;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit1)
							{
								Window->ActiveTextEdit = Window->NaturalFrequencyEdit;
							}
						else if (Window->ActiveTextEdit == Window->NaturalFrequencyEdit)
							{
								Window->ActiveTextEdit = Window->OriginEdit;
							}
						else if (Window->ActiveTextEdit == Window->OriginEdit)
							{
								Window->ActiveTextEdit = Window->SamplingRateEdit;
							}
						else if (Window->ActiveTextEdit == Window->SamplingRateEdit)
							{
								Window->ActiveTextEdit = Window->NameEdit;
							}
						else
							{
								Window->ActiveTextEdit = Window->LoopEndEdit3;
							}
					}
				 else
					{
						/* forwards */
						if (Window->ActiveTextEdit == Window->NameEdit)
							{
								Window->ActiveTextEdit = Window->SamplingRateEdit;
							}
						else if (Window->ActiveTextEdit == Window->SamplingRateEdit)
							{
								Window->ActiveTextEdit = Window->OriginEdit;
							}
						else if (Window->ActiveTextEdit == Window->OriginEdit)
							{
								Window->ActiveTextEdit = Window->NaturalFrequencyEdit;
							}
						else if (Window->ActiveTextEdit == Window->NaturalFrequencyEdit)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit1;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit1)
							{
								Window->ActiveTextEdit = Window->LoopEndEdit1;
							}
						else if (Window->ActiveTextEdit == Window->LoopEndEdit1)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit2;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit2)
							{
								Window->ActiveTextEdit = Window->LoopEndEdit2;
							}
						else if (Window->ActiveTextEdit == Window->LoopEndEdit2)
							{
								Window->ActiveTextEdit = Window->LoopStartEdit3;
							}
						else if (Window->ActiveTextEdit == Window->LoopStartEdit3)
							{
								Window->ActiveTextEdit = Window->LoopEndEdit3;
							}
						else
							{
								Window->ActiveTextEdit = Window->NameEdit;
							}
					}
				EnableTextEditSelection(Window->ActiveTextEdit);
				TextEditDoMenuSelectAll(Window->ActiveTextEdit);
			}
		else
			{
				TextEditDoKeyPressed(Window->ActiveTextEdit,KeyCode,Modifiers);
			}
	}


void								AlgoSampWindowClose(AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DisposeAlgoSampWindow(Window);
	}


void								AlgoSampWindowUpdator(AlgoSampWindowRec* Window)
	{
		OrdType			XSize;
		OrdType			YSize;
		FontType		ScreenFont;

		CheckPtrExistence(Window);
		ScreenFont = GetScreenFont();
		TextEditFullRedraw(Window->NameEdit);
		TextEditFullRedraw(Window->BodyEdit);
		TextEditFullRedraw(Window->SamplingRateEdit);
		TextEditFullRedraw(Window->OriginEdit);
		TextEditFullRedraw(Window->NaturalFrequencyEdit);
		TextEditFullRedraw(Window->LoopStartEdit1);
		TextEditFullRedraw(Window->LoopStartEdit2);
		TextEditFullRedraw(Window->LoopStartEdit3);
		TextEditFullRedraw(Window->LoopEndEdit1);
		TextEditFullRedraw(Window->LoopEndEdit2);
		TextEditFullRedraw(Window->LoopEndEdit3);
		RedrawIconButton(Window->Bits16Button);
		RedrawIconButton(Window->Bits8Button);
		RedrawIconButton(Window->MonoButton);
		RedrawIconButton(Window->StereoButton);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Sample Name:",12,
			SAMPLENAMEX,SAMPLENAMEY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Sampling Rate:",14,
			SAMPLERATEX,SAMPLERATEY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Origin:",7,ORIGINX,ORIGINY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Natural Freq:",13,
			BASEFREQX,BASEFREQY,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop Start 1:",13,
			LOOPSTART1X,LOOPSTART1Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop Start 2:",13,
			LOOPSTART2X,LOOPSTART2Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop Start 3:",13,
			LOOPSTART3X,LOOPSTART3Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop End 1:",11,
			LOOPEND1X,LOOPEND1Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop End 2:",11,
			LOOPEND2X,LOOPEND2Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Loop End 3:",11,
			LOOPEND3X,LOOPEND3Y,ePlain);
		DrawTextLine(Window->ScreenID,ScreenFont,9,"Waveform Generating Function:",29,
			BODYX + TITLEINDENT,BODYY,eBold);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


void								AlgoSampWindowMenuSetup(AlgoSampWindowRec* Window)
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
		ChangeItemName(mCloseFile,"Close Algorithmic Sample Editor");
		EnableMenuItem(mCloseFile);
		ChangeItemName(mBuildFunction,"Build Sample");
		EnableMenuItem(mBuildFunction);
		ChangeItemName(mUnbuildFunction,"Unbuild Sample");
		EnableMenuItem(mUnbuildFunction);
		ChangeItemName(mAlgoSampToSample,"Open Algorithmic Sample As New Sample");
		EnableMenuItem(mAlgoSampToSample);
		ChangeItemName(mDeleteObject,"Delete Algorithmic Sample");
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


void								AlgoSampWindowDoMenuCommand(AlgoSampWindowRec* Window,
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
				AlgoSampWindowClose(Window);
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
				AlgoSampObjectBuild(Window->AlgoSampObject);
			}
		else if (MenuItem == mUnbuildFunction)
			{
				AlgoSampObjectUnbuild(Window->AlgoSampObject);
			}
		else if (MenuItem == mAlgoSampToSample)
			{
				if (AlgoSampObjectMakeUpToDate(Window->AlgoSampObject))
					{
						MainWindowCopyRawSampleAndOpen(Window->MainWindow,
							AlgoSampObjectGetRawData(Window->AlgoSampObject),
							AlgoSampObjectGetNumBits(Window->AlgoSampObject),
							AlgoSampObjectGetNumChannels(Window->AlgoSampObject),
							AlgoSampObjectGetOrigin(Window->AlgoSampObject),
							AlgoSampObjectGetLoopStart1(Window->AlgoSampObject),
							AlgoSampObjectGetLoopStart2(Window->AlgoSampObject),
							AlgoSampObjectGetLoopStart3(Window->AlgoSampObject),
							AlgoSampObjectGetLoopEnd1(Window->AlgoSampObject),
							AlgoSampObjectGetLoopEnd2(Window->AlgoSampObject),
							AlgoSampObjectGetLoopEnd3(Window->AlgoSampObject),
							AlgoSampObjectGetSamplingRate(Window->AlgoSampObject),
							AlgoSampObjectGetNaturalFrequency(Window->AlgoSampObject));
					}
			}
		else if (MenuItem == mDeleteObject)
			{
				AlgoSampListDeleteAlgoSamp(Window->AlgoSampList,Window->AlgoSampObject);
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
								"AlgoSampWindowDoMenuCommand:  bad value from DoFindDialog"));
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
				EXECUTE(PRERR(AllowResume,"AlgoSampWindowDoMenuCommand:  unknown menu command"));
			}
	}


/* get a copy of the name of the sample. */
char*								AlgoSampWindowGetNameCopy(AlgoSampWindowRec* Window)
	{
		char*							ReturnValue;

		CheckPtrExistence(Window);
		ReturnValue = TextEditGetRawData(Window->NameEdit,"\x0a");
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"AlgoSampNameCopy");
			}
		return ReturnValue;
	}


/* get a copy of the sample's formula */
char*								AlgoSampWindowGetFormulaCopy(AlgoSampWindowRec* Window)
	{
		char*							ReturnValue;

		CheckPtrExistence(Window);
		ReturnValue = TextEditGetRawData(Window->BodyEdit,"\x0a");
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"AlgoSampFormulaCopy");
			}
		return ReturnValue;
	}


/* get the origin point of the sample */
long								AlgoSampWindowGetOrigin(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->OriginEdit,"\x0a");
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


/* get the first loop start point of the sample */
long								AlgoSampWindowGetLoopStart1(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopStartEdit1,"\x0a");
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


/* get the second loop start point of the sample */
long								AlgoSampWindowGetLoopStart2(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopStartEdit2,"\x0a");
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


/* get the third loop start point of the sample */
long								AlgoSampWindowGetLoopStart3(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopStartEdit3,"\x0a");
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


/* get the first loop end point of the sample */
long								AlgoSampWindowGetLoopEnd1(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopEndEdit1,"\x0a");
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


/* get the second loop end point of the sample */
long								AlgoSampWindowGetLoopEnd2(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopEndEdit2,"\x0a");
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


/* get the third loop end point of the sample */
long								AlgoSampWindowGetLoopEnd3(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->LoopEndEdit3,"\x0a");
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


/* get the natural pitch for playback conversion */
double							AlgoSampWindowGetNaturalFrequency(AlgoSampWindowRec* Window)
	{
		double						ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->NaturalFrequencyEdit,"\x0a");
		if (StringTemp != NIL)
			{
				ReturnValue = StringToLongDouble(StringTemp,PtrSize(StringTemp));
				ReleasePtr(StringTemp);
			}
		 else
			{
				ReturnValue = 0;
			}
		return ReturnValue;
	}


/* get the sampling rate */
long								AlgoSampWindowGetSamplingRate(AlgoSampWindowRec* Window)
	{
		long							ReturnValue;
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = TextEditGetRawData(Window->SamplingRateEdit,"\x0a");
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


/* get the number of bits that the sample uses */
NumBitsType					AlgoSampWindowGetNumBits(AlgoSampWindowRec* Window)
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


/* get the number of channels that the sample uses */
NumChannelsType			AlgoSampWindowGetNumChannels(AlgoSampWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if (GetIconButtonState(Window->StereoButton))
			{
				return eSampleStereo;
			}
		 else
			{
				return eSampleMono;
			}
	}


/* the name of the document has changed, so change the title bar.  the caller is */
/* responsible for disposing of the string, which must be non-null-terminated */
void								AlgoSampWindowGlobalNameChange(AlgoSampWindowRec* Window,
											char* NewFilename)
	{
		char*							LocalNameCopy;

		CheckPtrExistence(Window);
		CheckPtrExistence(NewFilename);
		LocalNameCopy = AlgoSampWindowGetNameCopy(Window);
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


/* update the name in the title bar even if the document name hasn't changed. */
void								AlgoSampWindowResetTitlebar(AlgoSampWindowRec* Window)
	{
		char*							DocumentName;

		CheckPtrExistence(Window);
		DocumentName = GetCopyOfDocumentName(Window->MainWindow);
		if (DocumentName != NIL)
			{
				AlgoSampWindowGlobalNameChange(Window,DocumentName);
				ReleasePtr(DocumentName);
			}
	}


/* write back all modified data to the algosamp object */
MyBoolean						AlgoSampWindowWritebackModifiedData(AlgoSampWindowRec* Window)
	{
		MyBoolean					SuccessFlag = True;

		CheckPtrExistence(Window);

		if (TextEditDoesItNeedToBeSaved(Window->NameEdit))
			{
				char*						String;

				String = AlgoSampWindowGetNameCopy(Window);
				if (String != NIL)
					{
						AlgoSampObjectNewName(Window->AlgoSampObject,String);
						TextEditHasBeenSaved(Window->NameEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		if (TextEditDoesItNeedToBeSaved(Window->BodyEdit))
			{
				char*						String;

				String = AlgoSampWindowGetFormulaCopy(Window);
				if (String != NIL)
					{
						AlgoSampObjectNewFormula(Window->AlgoSampObject,String);
						TextEditHasBeenSaved(Window->BodyEdit);
					}
				 else
					{
						SuccessFlag = False;
					}
			}

		if (TextEditDoesItNeedToBeSaved(Window->OriginEdit))
			{
				AlgoSampObjectPutOrigin(Window->AlgoSampObject,
					AlgoSampWindowGetOrigin(Window));
				TextEditHasBeenSaved(Window->OriginEdit);
			}

		if (TextEditDoesItNeedToBeSaved(Window->SamplingRateEdit))
			{
				AlgoSampObjectPutSamplingRate(Window->AlgoSampObject,
					AlgoSampWindowGetSamplingRate(Window));
				TextEditHasBeenSaved(Window->SamplingRateEdit);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopStartEdit1))
			{
				AlgoSampObjectPutLoopStart1(Window->AlgoSampObject,
					AlgoSampWindowGetLoopStart1(Window));
				TextEditHasBeenSaved(Window->LoopStartEdit1);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopStartEdit2))
			{
				AlgoSampObjectPutLoopStart2(Window->AlgoSampObject,
					AlgoSampWindowGetLoopStart2(Window));
				TextEditHasBeenSaved(Window->LoopStartEdit2);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopStartEdit3))
			{
				AlgoSampObjectPutLoopStart3(Window->AlgoSampObject,
					AlgoSampWindowGetLoopStart3(Window));
				TextEditHasBeenSaved(Window->LoopStartEdit3);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopEndEdit1))
			{
				AlgoSampObjectPutLoopEnd1(Window->AlgoSampObject,
					AlgoSampWindowGetLoopEnd1(Window));
				TextEditHasBeenSaved(Window->LoopEndEdit1);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopEndEdit2))
			{
				AlgoSampObjectPutLoopEnd2(Window->AlgoSampObject,
					AlgoSampWindowGetLoopEnd2(Window));
				TextEditHasBeenSaved(Window->LoopEndEdit2);
			}

		if (TextEditDoesItNeedToBeSaved(Window->LoopEndEdit3))
			{
				AlgoSampObjectPutLoopEnd3(Window->AlgoSampObject,
					AlgoSampWindowGetLoopEnd3(Window));
				TextEditHasBeenSaved(Window->LoopEndEdit3);
			}

		if (TextEditDoesItNeedToBeSaved(Window->NaturalFrequencyEdit))
			{
				AlgoSampObjectPutNaturalFrequency(Window->AlgoSampObject,
					AlgoSampWindowGetNaturalFrequency(Window));
				TextEditHasBeenSaved(Window->NaturalFrequencyEdit);
			}

		if (Window->ChannelsButtonChanged)
			{
				AlgoSampObjectPutNumChannels(Window->AlgoSampObject,
					AlgoSampWindowGetNumChannels(Window));
				Window->ChannelsButtonChanged = False;
			}

		if (Window->BitsButtonChanged)
			{
				AlgoSampObjectPutNumBits(Window->AlgoSampObject,
					AlgoSampWindowGetNumBits(Window));
				Window->BitsButtonChanged = False;
			}

		return SuccessFlag;
	}
