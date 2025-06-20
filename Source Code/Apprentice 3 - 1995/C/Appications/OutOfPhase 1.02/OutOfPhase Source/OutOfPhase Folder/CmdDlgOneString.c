/* CmdDlgOneString.c */
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

#include "CmdDlgOneString.h"
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


#define WINXSIZE (500)

#define PROMPTX (10)
#define PROMPTY (5)
#define PROMPTWIDTH (WINXSIZE - (2 * PROMPTX))
#define PROMPTHEIGHT (50)

#define BOXPROMPTX (PROMPTX)
#define BOXPROMPTY (PROMPTY + PROMPTHEIGHT + 5)

#define BOXEDITX (BOXPROMPTX + 50)
#define BOXEDITY (BOXPROMPTY - 3)
#define BOXEDITWIDTH (WINXSIZE - BOXEDITX - 5)
#define BOXEDITHEIGHT (60)

#define CANCELBUTTONWIDTH (80)
#define CANCELBUTTONHEIGHT (21)
#define CANCELBUTTONX ((1 * WINXSIZE) / 4 - (CANCELBUTTONWIDTH / 2))
#define CANCELBUTTONY (BOXEDITY + BOXEDITHEIGHT + 10)

#define OKBUTTONWIDTH (CANCELBUTTONWIDTH)
#define OKBUTTONHEIGHT (CANCELBUTTONHEIGHT)
#define OKBUTTONX ((3 * WINXSIZE) / 4 - (OKBUTTONWIDTH / 2))
#define OKBUTTONY (CANCELBUTTONY)

#define WINYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 20)


typedef struct
	{
		WinType*					ScreenID;
		char*							PromptText;
		char*							EditPromptText;
		TextEditRec*			EditBox;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static void			RedrawWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->EditBox);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		SetClipRect(Window->ScreenID,0,0,WINXSIZE,WINYSIZE);
		DrawWrappedTextBox(Window->ScreenID,Window->PromptText,GetScreenFont(),9,
			PROMPTX,PROMPTY,PROMPTWIDTH,PROMPTHEIGHT);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,Window->EditPromptText,
			StrLen(Window->EditPromptText),BOXPROMPTX,BOXPROMPTY,ePlain);
	}


/* present a dialog box that allows the user to edit a single string parameter */
/* returns True if the user changes the value and clicks OK. */
MyBoolean				CommandDialogOneString(char* Prompt, char* BoxName, char** DataInOut)
	{
		WindowRec*		Window;
		MyBoolean			LoopFlag;
		MyBoolean			DoItFlag EXECUTE(= -31342);
		MyBoolean			ReturnValue;

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),
			"CommandDialogOneString:  WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to edit the "
					"command parameters.",NIL);
				return False;
			}
		Window->PromptText = Prompt;
		Window->EditPromptText = BoxName;

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

		Window->EditBox = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),9,
			BOXEDITX,BOXEDITY,BOXEDITWIDTH,BOXEDITHEIGHT);
		if (Window->EditBox == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}
		TextEditNewRawData(Window->EditBox,*DataInOut,SYSTEMLINEFEED);
		TextEditHasBeenSaved(Window->EditBox);


		EnableTextEditSelection(Window->EditBox);
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
							if (TextEditIBeamTest(Window->EditBox,X,Y))
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
							TextEditUpdateCursor(Window->EditBox);
							break;
						case eMenuStarting:
							EnableMenuItem(mPaste);
							if (TextEditIsThereValidSelection(Window->EditBox))
								{
									EnableMenuItem(mCut);
									EnableMenuItem(mCopy);
									EnableMenuItem(mClear);
								}
							EnableMenuItem(mSelectAll);
							if (TextEditCanWeUndo(Window->EditBox))
								{
									EnableMenuItem(mUndo);
								}
							break;
						case eMenuCommand:
							if (MenuItem == mPaste)
								{
									TextEditDoMenuPaste(Window->EditBox);
								}
							else if (MenuItem == mCut)
								{
									TextEditDoMenuCut(Window->EditBox);
								}
							else if (MenuItem == mCopy)
								{
									TextEditDoMenuCopy(Window->EditBox);
								}
							else if (MenuItem == mClear)
								{
									TextEditDoMenuClear(Window->EditBox);
								}
							else if (MenuItem == mUndo)
								{
									TextEditDoMenuUndo(Window->EditBox);
									TextEditShowSelection(Window->EditBox);
								}
							else if (MenuItem == mSelectAll)
								{
									TextEditDoMenuSelectAll(Window->EditBox);
								}
							else
								{
									EXECUTE(PRERR(AllowResume,
										"CommandDialogOneString: Undefined menu option chosen"));
								}
							break;
						case eKeyPressed:
							if ((KeyPress == 13) && ((Modifiers & eCommandKey) == 0))
								{
									FlashButton(Window->OKButton);
									DoItFlag = True;
									LoopFlag = False;
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									DoItFlag = False;
									LoopFlag = False;
								}
							else
								{
									TextEditDoKeyPressed(Window->EditBox,KeyPress,Modifiers);
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
							else if (TextEditHitTest(Window->EditBox,X,Y))
								{
									TextEditDoMouseDown(Window->EditBox,X,Y,Modifiers);
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"CommandDialogOneString:  DoItFlag is neither true nor false"));

		ReturnValue = False;

		if (DoItFlag)
			{
				MyBoolean					ErrorNoMemory;
				char*							StringTemp;

				ErrorNoMemory = False;

				if (TextEditDoesItNeedToBeSaved(Window->EditBox))
					{
						StringTemp = TextEditGetRawData(Window->EditBox,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								ReleasePtr(*DataInOut);
								*DataInOut = StringTemp;
								SetTag(StringTemp,"CommandDialogOneString return value");
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
		DisposeTextEdit(Window->EditBox);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);

		return ReturnValue;
	}
