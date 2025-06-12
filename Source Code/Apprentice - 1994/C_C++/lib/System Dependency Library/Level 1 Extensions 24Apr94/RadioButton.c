/* RadioButton.c */
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

#include "RadioButton.h"
#include "Screen.h"
#include "EventLoop.h"
#include "Memory.h"
#include "DataMunging.h"


struct RadioButtonRec
	{
		WinType*		Window;
		char*				Name;
		OrdType			X;
		OrdType			Y;
		OrdType			Width;
		OrdType			Height;
		MyBoolean		State;
	};


static char						oNormalUnselected[] =
	{
		0x1F,0x80,0x60,0x60,0x40,0x20,0x80,0x10,0x80,0x10,0x80,0x10,
		0x80,0x10,0x80,0x10,0x80,0x10,0x40,0x20,0x60,0x60,0x1F,0x80
	};

static char						oNormalSelected[] =
	{
		0x1F,0x80,0x60,0x60,0x40,0x20,0x8F,0x10,0x9F,0x90,0x9F,0x90,
		0x9F,0x90,0x9F,0x90,0x8F,0x10,0x40,0x20,0x60,0x60,0x1F,0x80
	};

static char						oMouseDownUnselected[] =
	{
		0x1F,0x80,0x7F,0xE0,0x60,0x60,0xC0,0x30,0xC0,0x30,0xC0,0x30,
		0xC0,0x30,0xC0,0x30,0xC0,0x30,0x60,0x60,0x7F,0xE0,0x1F,0x80
	};

static char						oMouseDownSelected[] =
	{
		0x1F,0x80,0x7F,0xE0,0x60,0x60,0xCF,0x30,0xDF,0xB0,0xDF,0xB0,
		0xDF,0xB0,0xDF,0xB0,0xCF,0x30,0x60,0x60,0x7F,0xE0,0x1F,0x80
	};

static long						BitmapReferenceCount = 0;

static Bitmap*				NormalUnselected;
static Bitmap*				NormalSelected;
static Bitmap*				MouseDownUnselected;
static Bitmap*				MouseDownSelected;


/* allocate a new radio button.  Name is null terminated */
RadioButtonRec*			NewRadioButton(WinType* Window, char* Name,
											OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		RadioButtonRec*			TheButton;

		if (BitmapReferenceCount == 0)
			{
				NormalUnselected = MakeBitmap(oNormalUnselected,12,12,2);
				if (NormalUnselected == NIL)
					{
					 FailurePoint1:
						return NIL;
					}
				NormalSelected = MakeBitmap(oNormalSelected,12,12,2);
				if (NormalSelected == NIL)
					{
					 FailurePoint2:
						DisposeBitmap(NormalUnselected);
						goto FailurePoint1;
					}
				MouseDownUnselected = MakeBitmap(oMouseDownUnselected,12,12,2);
				if (MouseDownUnselected == NIL)
					{
					 FailurePoint3:
						DisposeBitmap(NormalSelected);
						goto FailurePoint2;
					}
				MouseDownSelected = MakeBitmap(oMouseDownSelected,12,12,2);
				if (MouseDownSelected == NIL)
					{
					 FailurePoint4:
						DisposeBitmap(MouseDownUnselected);
						goto FailurePoint3;
					}
			}
		BitmapReferenceCount += 1;
		TheButton = (RadioButtonRec*)AllocPtrCanFail(sizeof(RadioButtonRec),"RadioButton");
		if (TheButton == NIL)
			{
			 OtherFailurePoint:
				BitmapReferenceCount -= 1;
				if (BitmapReferenceCount == 0)
					{
						DisposeBitmap(MouseDownSelected);
						goto FailurePoint4;
					}
				 else
					{
						return NIL;
					}
			}
		TheButton->Window = Window;
		TheButton->Name = StringToBlockCopy(Name);
		if (TheButton->Name == NIL)
			{
				ReleasePtr((char*)TheButton);
				goto OtherFailurePoint;
			}
		TheButton->X = X;
		TheButton->Y = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
		TheButton->State = False;
		return TheButton;
	}


/* dispose of radio button and any internal data structures. */
void								DisposeRadioButton(RadioButtonRec* TheButton)
	{
		BitmapReferenceCount -= 1;
		ERROR(BitmapReferenceCount < 0,PRERR(ForceAbort,
			"DisposeRadioButton:  bitmap reference count is negative"));
		if (BitmapReferenceCount == 0)
			{
				DisposeBitmap(NormalUnselected);
				DisposeBitmap(NormalSelected);
				DisposeBitmap(MouseDownUnselected);
				DisposeBitmap(MouseDownSelected);
			}
		ReleasePtr(TheButton->Name);
		ReleasePtr((char*)TheButton);
	}


/* find out where the radio button is located */
OrdType							GetRadioButtonXLoc(RadioButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->X;
	}


/* find out where the radio button is located */
OrdType							GetRadioButtonYLoc(RadioButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Y;
	}


/* find out where the radio button is located */
OrdType							GetRadioButtonWidth(RadioButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Width;
	}


/* find out where the radio button is located */
OrdType							GetRadioButtonHeight(RadioButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Height;
	}


/* change the location of the radio button */
void								SetRadioButtonLocation(RadioButtonRec* TheButton,
											OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(TheButton);
		TheButton->X = X;
		TheButton->Y = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
	}


/* internal routine for drawing button */
static void					InternalRedrawRadio(RadioButtonRec* TheButton, MyBoolean Hilited)
	{
		OrdType			YStart;

		CheckPtrExistence(TheButton);
		SetClipRect(TheButton->Window,TheButton->X,TheButton->Y,
			TheButton->Width,TheButton->Height);
		YStart = (TheButton->Height - 12) / 2 + TheButton->Y;
		if (TheButton->State)
			{
				if (Hilited)
					{
						DrawBitmap(TheButton->Window,TheButton->X,YStart,MouseDownSelected);
					}
				 else
					{
						DrawBitmap(TheButton->Window,TheButton->X,YStart,NormalSelected);
					}
			}
		 else
			{
				if (Hilited)
					{
						DrawBitmap(TheButton->Window,TheButton->X,YStart,MouseDownUnselected);
					}
				 else
					{
						DrawBitmap(TheButton->Window,TheButton->X,YStart,NormalUnselected);
					}
			}
		DrawTextLine(TheButton->Window,GetScreenFont(),9,TheButton->Name,
			PtrSize(TheButton->Name),TheButton->X + 12 + 6,YStart,ePlain);
	}


/* do a full redraw of the button */
void								RedrawRadioButton(RadioButtonRec* TheButton)
	{
		InternalRedrawRadio(TheButton,False);
	}


/* handle a mouse down in the button.  returns True if the state changed. */
MyBoolean						RadioButtonMouseDown(RadioButtonRec* TheButton, OrdType X, OrdType Y)
	{
		MyBoolean				Inside;
		MyBoolean				OldInside;

		OldInside = False;
		do
			{
				Inside = RadioButtonHitTest(TheButton,X,Y);
				if (OldInside != Inside)
					{
						InternalRedrawRadio(TheButton,Inside);
						OldInside = Inside;
					}
			} while (GetAnEvent(&X,&Y,NIL,NIL,NIL,NIL) != eMouseUp);
		if (Inside && !TheButton->State)
			{
				TheButton->State = True;
			}
		 else
			{
				Inside = False; /* radio buttons do NOT toggle */
			}
		InternalRedrawRadio(TheButton,False);
		return Inside;
	}


/* force the state of the button to be a certain value */
void								SetRadioButtonState(RadioButtonRec* TheButton, MyBoolean TheState)
	{
		CheckPtrExistence(TheButton);
		TheButton->State = TheState;
		RedrawRadioButton(TheButton);
	}


/* get the state of the button */
MyBoolean						GetRadioButtonState(RadioButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->State;
	}


/* see if the location is in the radio button's box */
MyBoolean						RadioButtonHitTest(RadioButtonRec* TheButton, OrdType X, OrdType Y)
	{
		return (X >= TheButton->X) && (Y >= TheButton->Y)
			&& (X < TheButton->X + TheButton->Width) && (Y < TheButton->Y + TheButton->Height);
	}
