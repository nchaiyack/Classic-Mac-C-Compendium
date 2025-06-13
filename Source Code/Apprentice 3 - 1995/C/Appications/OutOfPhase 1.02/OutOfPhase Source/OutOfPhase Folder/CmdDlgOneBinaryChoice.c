/* CmdDlgOneBinaryChoice.c */
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

#include "CmdDlgOneBinaryChoice.h"
#include "Memory.h"
#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"
#include "SimpleButton.h"
#include "WrapTextBox.h"
#include "DataMunging.h"
#include "Alert.h"
#include "RadioButton.h"


#define WINXSIZE (350)

#define PROMPTX (10)
#define PROMPTY (5)
#define PROMPTWIDTH (WINXSIZE - (2 * PROMPTX))
#define PROMPTHEIGHT (50)

#define TRUEBUTTONX (PROMPTX)
#define TRUEBUTTONY (PROMPTY + PROMPTHEIGHT + 5)
#define TRUEBUTTONWIDTH (PROMPTWIDTH)
#define TRUEBUTTONHEIGHT (19)

#define FALSEBUTTONX (TRUEBUTTONX)
#define FALSEBUTTONY (TRUEBUTTONY + TRUEBUTTONHEIGHT + 2)
#define FALSEBUTTONWIDTH (TRUEBUTTONWIDTH)
#define FALSEBUTTONHEIGHT (TRUEBUTTONHEIGHT)

#define CANCELBUTTONWIDTH (80)
#define CANCELBUTTONHEIGHT (21)
#define CANCELBUTTONX ((1 * WINXSIZE) / 4 - (CANCELBUTTONWIDTH / 2))
#define CANCELBUTTONY (FALSEBUTTONY + FALSEBUTTONHEIGHT + 10)

#define OKBUTTONWIDTH (CANCELBUTTONWIDTH)
#define OKBUTTONHEIGHT (CANCELBUTTONHEIGHT)
#define OKBUTTONX ((3 * WINXSIZE) / 4 - (OKBUTTONWIDTH / 2))
#define OKBUTTONY (CANCELBUTTONY)

#define WINYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 20)


typedef struct
	{
		WinType*					ScreenID;
		char*							PromptText;
		RadioButtonRec*		TrueButton;
		RadioButtonRec*		FalseButton;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static void			RedrawWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawRadioButton(Window->TrueButton);
		RedrawRadioButton(Window->FalseButton);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		SetClipRect(Window->ScreenID,0,0,WINXSIZE,WINYSIZE);
		DrawWrappedTextBox(Window->ScreenID,Window->PromptText,GetScreenFont(),9,
			PROMPTX,PROMPTY,PROMPTWIDTH,PROMPTHEIGHT);
	}


/* dialog box that gives the user a choice of two radio buttons.  it returns True */
/* if the user accepted the change and there was a change. */
MyBoolean				CommandDialogOneBinaryChoice(char* Prompt, char* TrueButtonName,
									char* FalseButtonName, MyBoolean* FlagInOut)
	{
		WindowRec*		Window;
		MyBoolean			LoopFlag;
		MyBoolean			DoItFlag EXECUTE(= -31342);
		MyBoolean			ReturnValue;

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),
			"CommandDialogOneBinaryChoice:  WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to edit the "
					"command parameters.",NIL);
				return False;
			}
		Window->PromptText = Prompt;

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

		Window->TrueButton = NewRadioButton(Window->ScreenID,TrueButtonName,
			TRUEBUTTONX,TRUEBUTTONY,TRUEBUTTONWIDTH,TRUEBUTTONHEIGHT);
		if (Window->TrueButton == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}

		Window->FalseButton = NewRadioButton(Window->ScreenID,FalseButtonName,
			FALSEBUTTONX,FALSEBUTTONY,FALSEBUTTONWIDTH,FALSEBUTTONHEIGHT);
		if (Window->FalseButton == NIL)
			{
			 FailurePoint6:
				DisposeRadioButton(Window->TrueButton);
				goto FailurePoint5;
			}

		if (*FlagInOut)
			{
				SetRadioButtonState(Window->TrueButton,True);
			}
		 else
			{
				SetRadioButtonState(Window->FalseButton,True);
			}


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
							SetArrowCursor();
							break;
						case eNoEvent:
							break;
						case eMenuStarting:
							break;
						case eMenuCommand:
							EXECUTE(PRERR(AllowResume,
								"CommandDialogOneBinaryChoice: Undefined menu option chosen"));
							break;
						case eKeyPressed:
							if (KeyPress == 13)
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
							else if (RadioButtonHitTest(Window->TrueButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->TrueButton,X,Y))
										{
											SetRadioButtonState(Window->FalseButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->FalseButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->FalseButton,X,Y))
										{
											SetRadioButtonState(Window->TrueButton,False);
										}
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"CommandDialogOneBinaryChoice:  DoItFlag is neither true nor false"));

		ReturnValue = False;

		if (DoItFlag)
			{
				MyBoolean					PossibleValue;

				if (GetRadioButtonState(Window->TrueButton))
					{
						PossibleValue = True;
					}
				else if (GetRadioButtonState(Window->FalseButton))
					{
						PossibleValue = False;
					}
				else
					{
						EXECUTE(PRERR(ForceAbort,
							"CommandDialogOneBinaryChoice:  button inconsistency"));
					}

				if (PossibleValue != *FlagInOut)
					{
						ReturnValue = True;
						*FlagInOut = PossibleValue;
					}
			}

		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		DisposeRadioButton(Window->TrueButton);
		DisposeRadioButton(Window->FalseButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);

		return ReturnValue;
	}
