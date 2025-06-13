/* SimpleButton.c */
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

#include "SimpleButton.h"
#include "Screen.h"
#include "Memory.h"
#include "EventLoop.h"
#include "DataMunging.h"


struct SimpleButtonRec
	{
		WinType*			Window;
		char*					Name;
		OrdType				X;
		OrdType				Y;
		OrdType				Width;
		OrdType				Height;
		MyBoolean			Default;
	};


/* allocate the simple button */
SimpleButtonRec*		NewSimpleButton(WinType* Window, char* Name,
											OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		SimpleButtonRec*		TheButton;
		char*								NameTemp;
		long								Scan;
		long								Index;

		TheButton = (SimpleButtonRec*)AllocPtrCanFail(
			sizeof(SimpleButtonRec),"SimpleButton");
		if (TheButton == NIL)
			{
				return NIL;
			}
		TheButton->Window = Window;
		Scan = 0;
		while (Name[Scan] != 0)
			{
				Scan += 1;
			}
		NameTemp = AllocPtrCanFail(Scan + 1,"SimpleButtonName");
		if (NameTemp == NIL)
			{
				ReleasePtr((char*)TheButton);
				return NIL;
			}
		for (Index = 0; Index <= Scan; Index += 1)
			{
				NameTemp[Index] = Name[Index];
			}
		TheButton->Name = NameTemp;
		TheButton->X = X;
		TheButton->Y = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
		TheButton->Default = False;
		return TheButton;
	}


/* dispose of the button and internal data */
void								DisposeSimpleButton(SimpleButtonRec* TheButton)
	{
		ReleasePtr(TheButton->Name);
		ReleasePtr((char*)TheButton);
	}


/* indicate whether this button is a default button or not.  if it is, it will */
/* be hilighted specially (heavy outline).  It will NOT be drawn though */
void								SetDefaultButtonState(SimpleButtonRec* TheButton, MyBoolean Default)
	{
		CheckPtrExistence(TheButton);
		TheButton->Default = Default;
	}


/* find out where the button is located */
OrdType							GetSimpleButtonXLoc(SimpleButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->X;
	}


/* find out where the button is located */
OrdType							GetSimpleButtonYLoc(SimpleButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Y;
	}


/* find out where the button is located */
OrdType							GetSimpleButtonWidth(SimpleButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Width;
	}


/* find out where the button is located */
OrdType							GetSimpleButtonHeight(SimpleButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Height;
	}


/* change the location of the button */
void								SetSimpleButtonLocation(SimpleButtonRec* TheButton,
											OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(TheButton);
		TheButton->X = X;
		TheButton->Y = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
	}


/* internal routine for drawing button */
static void					InternalRedrawButton(SimpleButtonRec* TheButton, MyBoolean Hilited)
	{
		OrdType		TextStartX;
		OrdType		TextStartY;

		CheckPtrExistence(TheButton);
		SetClipRect(TheButton->Window,TheButton->X - 4,TheButton->Y - 4,
			TheButton->Width + 4 + 4,TheButton->Height + 4 + 4);
		TextStartX = (TheButton->Width - LengthOfText(GetUglyFont(),12,
		TheButton->Name,StrLen(TheButton->Name),ePlain)) / 2 + TheButton->X;
		TextStartY = (TheButton->Height - GetFontHeight(GetUglyFont(),12)) / 2
			+ TheButton->Y;
		if (Hilited)
			{
				DrawRoundBoxPaint(TheButton->Window,eBlack,TheButton->X,TheButton->Y,
					TheButton->Width,TheButton->Height,10,10);
				InvertedTextLine(TheButton->Window,GetUglyFont(),12,TheButton->Name,
					StrLen(TheButton->Name),TextStartX,TextStartY,ePlain);
			}
		 else
			{
				DrawRoundBoxErase(TheButton->Window,TheButton->X,TheButton->Y,
					TheButton->Width,TheButton->Height,10,10);
				DrawTextLine(TheButton->Window,GetUglyFont(),12,TheButton->Name,
					StrLen(TheButton->Name),TextStartX,TextStartY,ePlain);
			}
		DrawRoundBoxFrame(TheButton->Window,eBlack,TheButton->X,TheButton->Y,
			TheButton->Width,TheButton->Height,10,10);
		if (TheButton->Default)
			{
				DrawRoundBoxFrame(TheButton->Window,eBlack,
					TheButton->X - 2,TheButton->Y - 2,
					TheButton->Width + (2 * 2),
					TheButton->Height + (2 * 2),12,12);
				DrawRoundBoxFrame(TheButton->Window,eBlack,
					TheButton->X - 3,TheButton->Y - 3,
					TheButton->Width + (2 * 3),
					TheButton->Height + (2 * 3),12,12);
				DrawRoundBoxFrame(TheButton->Window,eBlack,
					TheButton->X - 3 - 1,TheButton->Y - 3,
					TheButton->Width + (2 * 3),
					TheButton->Height + (2 * 3),12,12);
				DrawRoundBoxFrame(TheButton->Window,eBlack,
					TheButton->X - 3 + 1,TheButton->Y - 3,
					TheButton->Width + (2 * 3),
					TheButton->Height + (2 * 3),12,12);
				DrawRoundBoxFrame(TheButton->Window,eBlack,
					TheButton->X - 4,TheButton->Y - 4,
					TheButton->Width + (2 * 4),
					TheButton->Height + (2 * 4),12,12);
			}
	}


/* do a full redraw of the button */
void								RedrawSimpleButton(SimpleButtonRec* TheButton)
	{
		InternalRedrawButton(TheButton,False);
	}


/* do a mouse down in the button.  if Tracking != NIL, then it will be repeatedly */
/* called with the Inside status until the mouse goes up */
MyBoolean						SimpleButtonMouseDown(SimpleButtonRec* TheButton, OrdType X, OrdType Y,
											void (*Tracking)(void* Refcon, MyBoolean Inside), void* Refcon)
	{
		MyBoolean				Inside;
		MyBoolean				OldInside;

		OldInside = False;
		do
			{
				Inside = SimpleButtonHitTest(TheButton,X,Y);
				if (OldInside != Inside)
					{
						InternalRedrawButton(TheButton,Inside);
						OldInside = Inside;
					}
				if (Tracking != NIL)
					{
						(*Tracking)(Refcon,Inside);
					}
			} while (GetAnEvent(&X,&Y,NIL,NIL,NIL,NIL) != eMouseUp);
		InternalRedrawButton(TheButton,False);
		return Inside;
	}


/* momentarily flash the button.  this is used to provide visual feedback in case */
/* a key press performs the same function as clicking the button. */
void								FlashButton(SimpleButtonRec* TheButton)
	{
		double						EntryTime;

		InternalRedrawButton(TheButton,True);
		EntryTime = ReadTimer();
		while (TimerDifference(ReadTimer(),EntryTime) < 0.1)
			{
				RelinquishCPUCheckCancel();
			}
		InternalRedrawButton(TheButton,False);
	}


/* find out if the specified location is within the button box */
MyBoolean						SimpleButtonHitTest(SimpleButtonRec* TheButton, OrdType X,
											OrdType Y)
	{
		return (X >= TheButton->X) && (Y >= TheButton->Y)
			&& (X < TheButton->X + TheButton->Width)
			&& (Y < TheButton->Y + TheButton->Height);
	}
