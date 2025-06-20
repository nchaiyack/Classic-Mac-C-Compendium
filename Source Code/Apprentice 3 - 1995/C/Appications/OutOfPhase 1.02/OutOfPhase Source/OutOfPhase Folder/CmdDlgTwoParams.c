/* CmdDlgTwoParams.c */
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

#include "CmdDlgTwoParams.h"
#include "Memory.h"
#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"
#include "TextEdit.h"
#include "SimpleButton.h"
#include "WrapTextBox.h"
#include "DataMunging.h"
#include "Main.h"
#include "Alert.h"
#include "Numbers.h"


#define WINXSIZE (350)

#define PROMPTX (10)
#define PROMPTY (5)
#define PROMPTWIDTH (WINXSIZE - (2 * PROMPTX))
#define PROMPTHEIGHT (50)

#define FIRSTBOXPROMPTX (PROMPTX)
#define FIRSTBOXPROMPTY (PROMPTY + PROMPTHEIGHT + 5)

#define FIRSTBOXEDITX (FIRSTBOXPROMPTX + 140)
#define FIRSTBOXEDITY (FIRSTBOXPROMPTY - 3)
#define FIRSTBOXEDITWIDTH (140)
#define FIRSTBOXEDITHEIGHT (21)

#define SECONDBOXPROMPTX (FIRSTBOXPROMPTX)
#define SECONDBOXPROMPTY (FIRSTBOXPROMPTY + FIRSTBOXEDITHEIGHT + 3)

#define SECONDBOXEDITX (FIRSTBOXEDITX)
#define SECONDBOXEDITY (SECONDBOXPROMPTY - 3)
#define SECONDBOXEDITWIDTH (FIRSTBOXEDITWIDTH)
#define SECONDBOXEDITHEIGHT (FIRSTBOXEDITHEIGHT)

#define CANCELBUTTONWIDTH (80)
#define CANCELBUTTONHEIGHT (21)
#define CANCELBUTTONX ((1 * WINXSIZE) / 4 - (CANCELBUTTONWIDTH / 2))
#define CANCELBUTTONY (SECONDBOXEDITY + SECONDBOXEDITHEIGHT + 10)

#define OKBUTTONWIDTH (CANCELBUTTONWIDTH)
#define OKBUTTONHEIGHT (CANCELBUTTONHEIGHT)
#define OKBUTTONX ((3 * WINXSIZE) / 4 - (OKBUTTONWIDTH / 2))
#define OKBUTTONY (CANCELBUTTONY)

#define WINYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 20)


typedef struct
	{
		WinType*					ScreenID;
		char*							PromptText;
		char*							FirstPromptText;
		TextEditRec*			FirstBoxEdit;
		char*							SecondPromptText;
		TextEditRec*			SecondBoxEdit;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static void			RedrawWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->FirstBoxEdit);
		TextEditFullRedraw(Window->SecondBoxEdit);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		SetClipRect(Window->ScreenID,0,0,WINXSIZE,WINYSIZE);
		DrawWrappedTextBox(Window->ScreenID,Window->PromptText,GetScreenFont(),9,
			PROMPTX,PROMPTY,PROMPTWIDTH,PROMPTHEIGHT);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,Window->FirstPromptText,
			StrLen(Window->FirstPromptText),FIRSTBOXPROMPTX,FIRSTBOXPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,Window->SecondPromptText,
			StrLen(Window->SecondPromptText),SECONDBOXPROMPTX,SECONDBOXPROMPTY,ePlain);
	}


/* present a dialog box that allows the user to edit two parameters */
/* returns True if the user changes the value and clicks OK. */
MyBoolean				CommandDialogTwoParams(char* Prompt, char* FirstBoxName,
									double* FirstDataInOut, char* SecondBoxName, double* SecondDataInOut)
	{
		WindowRec*		Window;
		char*					StringTemp;
		MyBoolean			LoopFlag;
		MyBoolean			DoItFlag EXECUTE(= -31342);
		MyBoolean			ReturnValue;
		TextEditRec*	ActiveTextEdit;

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),
			"CommandDialogTwoParams: WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to edit the "
					"command parameters.",NIL);
				return False;
			}
		Window->PromptText = Prompt;
		Window->FirstPromptText = FirstBoxName;
		Window->SecondPromptText = SecondBoxName;

		Window->ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(WINXSIZE),
			DialogTopEdge(WINYSIZE),WINXSIZE,WINYSIZE,(void (*)(void*))&RedrawWindow,Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Edit Command");

		Window->OKButton = NewSimpleButton(Window->ScreenID,"OK",OKBUTTONX,OKBUTTONY,
			OKBUTTONWIDTH,OKBUTTONHEIGHT);
		if (Window->OKButton == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		SetDefaultButtonState(Window->OKButton,True);

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",CANCELBUTTONX,
			CANCELBUTTONY,CANCELBUTTONWIDTH,CANCELBUTTONHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint4:
				DisposeSimpleButton(Window->OKButton);
				goto FailurePoint3;
			}

		Window->FirstBoxEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,FIRSTBOXEDITX,FIRSTBOXEDITY,FIRSTBOXEDITWIDTH,FIRSTBOXEDITHEIGHT);
		if (Window->FirstBoxEdit == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}
		StringTemp = LongDoubleToString(*FirstDataInOut,12,1e-6,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->FirstBoxEdit);
				goto FailurePoint5;
			}
		TextEditNewRawData(Window->FirstBoxEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->FirstBoxEdit);

		Window->SecondBoxEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,SECONDBOXEDITX,SECONDBOXEDITY,SECONDBOXEDITWIDTH,SECONDBOXEDITHEIGHT);
		if (Window->SecondBoxEdit == NIL)
			{
			 FailurePoint7:
				goto FailurePoint6;
			}
		StringTemp = LongDoubleToString(*SecondDataInOut,12,1e-6,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint8:
				DisposeTextEdit(Window->SecondBoxEdit);
				goto FailurePoint7;
			}
		TextEditNewRawData(Window->SecondBoxEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->SecondBoxEdit);


		ActiveTextEdit = Window->FirstBoxEdit;
		EnableTextEditSelection(ActiveTextEdit);
		LoopFlag = True;
		while (LoopFlag)
			{
				OrdType							X;
				OrdType							Y;
				ModifierFlags				Modifiers;
				MenuItemType*				MenuItem;
				char								KeyPress;

				switch (GetAnEvent(&X,&Y,&Modifiers,NIL,&MenuItem,&KeyPress))
					{
						default:
							break;
						case eCheckCursor:
							if (TextEditIBeamTest(Window->FirstBoxEdit,X,Y)
								|| TextEditIBeamTest(Window->SecondBoxEdit,X,Y))
								{
									SetIBeamCursor();
								}
							 else
								{
									SetArrowCursor();
								}
							goto UpdateCursorPoint;
							break;
						case eNoEvent:
						 UpdateCursorPoint:
							TextEditUpdateCursor(ActiveTextEdit);
							break;
						case eMenuStarting:
							EnableMenuItem(mPaste);
							if (TextEditIsThereValidSelection(ActiveTextEdit))
								{
									EnableMenuItem(mCut);
									EnableMenuItem(mCopy);
									EnableMenuItem(mClear);
								}
							EnableMenuItem(mSelectAll);
							if (TextEditCanWeUndo(ActiveTextEdit))
								{
									EnableMenuItem(mUndo);
								}
							break;
						case eMenuCommand:
							if (MenuItem == mPaste)
								{
									TextEditDoMenuPaste(ActiveTextEdit);
								}
							else if (MenuItem == mCut)
								{
									TextEditDoMenuCut(ActiveTextEdit);
								}
							else if (MenuItem == mCopy)
								{
									TextEditDoMenuCopy(ActiveTextEdit);
								}
							else if (MenuItem == mClear)
								{
									TextEditDoMenuClear(ActiveTextEdit);
								}
							else if (MenuItem == mUndo)
								{
									TextEditDoMenuUndo(ActiveTextEdit);
									TextEditShowSelection(ActiveTextEdit);
								}
							else if (MenuItem == mSelectAll)
								{
									TextEditDoMenuSelectAll(ActiveTextEdit);
								}
							else
								{
									EXECUTE(PRERR(AllowResume,
										"CommandDialogTwoParams: Undefined menu option chosen"));
								}
							break;
						case eKeyPressed:
							if (KeyPress == 13)
								{
									FlashButton(Window->OKButton);
									DoItFlag = True;
									LoopFlag = False;
								}
							else if (KeyPress == 9)
								{
									DisableTextEditSelection(ActiveTextEdit);
									if ((eShiftKey & Modifiers) == 0)
										{
											/* tab forwards */
											if (ActiveTextEdit == Window->FirstBoxEdit)
												{
													ActiveTextEdit = Window->SecondBoxEdit;
												}
											else
												{
													ActiveTextEdit = Window->FirstBoxEdit;
												}
										}
									 else
										{
											/* tab backwards */
											if (ActiveTextEdit == Window->SecondBoxEdit)
												{
													ActiveTextEdit = Window->FirstBoxEdit;
												}
											else
												{
													ActiveTextEdit = Window->SecondBoxEdit;
												}
										}
									TextEditDoMenuSelectAll(ActiveTextEdit);
									EnableTextEditSelection(ActiveTextEdit);
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									DoItFlag = False;
									LoopFlag = False;
								}
							else
								{
									TextEditDoKeyPressed(ActiveTextEdit,KeyPress,Modifiers);
								}
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window->OKButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->OKButton,X,Y,NIL,NIL))
										{
											DoItFlag = True;
											LoopFlag = False;
										}
								}
							else if (SimpleButtonHitTest(Window->CancelButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->CancelButton,X,Y,NIL,NIL))
										{
											DoItFlag = False;
											LoopFlag = False;
										}
								}
							else if (TextEditHitTest(Window->FirstBoxEdit,X,Y))
								{
									if (ActiveTextEdit != Window->FirstBoxEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->FirstBoxEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->SecondBoxEdit,X,Y))
								{
									if (ActiveTextEdit != Window->SecondBoxEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->SecondBoxEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"CommandDialogTwoParams:  DoItFlag is neither true nor false"));

		ReturnValue = False;

		if (DoItFlag)
			{
				MyBoolean					ErrorNoMemory;

				ErrorNoMemory = False;

				if (TextEditDoesItNeedToBeSaved(Window->FirstBoxEdit))
					{
						StringTemp = TextEditGetRawData(Window->FirstBoxEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								*FirstDataInOut = StringToLongDouble(StringTemp,PtrSize(StringTemp));
								ReleasePtr(StringTemp);
								ReturnValue = True;
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->SecondBoxEdit))
					{
						StringTemp = TextEditGetRawData(Window->SecondBoxEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								*SecondDataInOut = StringToLongDouble(StringTemp,PtrSize(StringTemp));
								ReleasePtr(StringTemp);
								ReturnValue = True;
							}
					}

				if (ErrorNoMemory)
					{
						AlertHalt("There was not enough memory available to save all of "
							"the attributes.",NIL);
					}
			}

		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		DisposeTextEdit(Window->FirstBoxEdit);
		DisposeTextEdit(Window->SecondBoxEdit);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);

		return ReturnValue;
	}
