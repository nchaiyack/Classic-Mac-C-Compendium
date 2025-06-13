/* CheckBox.c */
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

#include "CheckBox.h"
#include "Screen.h"
#include "Memory.h"
#include "EventLoop.h"
#include "DataMunging.h"


struct CheckBoxRec
	{
		WinType*		Window;
		char*				Name;
		OrdType			X;
		OrdType			Y;
		OrdType			Width;
		OrdType			Height;
		MyBoolean		State;
	};


static unsigned char				oNormalUnselected[] =
	{
		0xFF,0xF0,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,
		0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0xFF,0xF0
	};

static unsigned char				oNormalSelected[] =
	{
		0xFF,0xF0,0x80,0x10,0x80,0x10,0x9F,0x90,0x9F,0x90,0x9F,0x90,
		0x9F,0x90,0x9F,0x90,0x9F,0x90,0x80,0x10,0x80,0x10,0xFF,0xF0
	};

static unsigned char				oMouseDownUnselected[] =
	{
		0xFF,0xF0,0xFF,0xF0,0xC0,0x30,0xC0,0x30,0xC0,0x30,0xC0,0x30,
		0xC0,0x30,0xC0,0x30,0xC0,0x30,0xC0,0x30,0xFF,0xF0,0xFF,0xF0
	};

static unsigned char				oMouseDownSelected[] =
	{
		0xFF,0xF0,0xFF,0xF0,0xC0,0x30,0xDF,0xB0,0xDF,0xB0,0xDF,0xB0,
		0xDF,0xB0,0xDF,0xB0,0xDF,0xB0,0xC0,0x30,0xFF,0xF0,0xFF,0xF0
	};

static long						BitmapReferenceCount = 0;

static Bitmap*				NormalUnselected;
static Bitmap*				NormalSelected;
static Bitmap*				MouseDownUnselected;
static Bitmap*				MouseDownSelected;


/* create a new check box.  Name is null-terminated.  returns NIL if it failed */
CheckBoxRec*		NewCheckBox(WinType* Window, char* Name,
									OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		CheckBoxRec*	TheButton;

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
		TheButton = (CheckBoxRec*)AllocPtrCanFail(sizeof(CheckBoxRec),"CheckBox");
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


/* dispose of a check box and internal data */
void						DisposeCheckBox(CheckBoxRec* TheButton)
	{
		BitmapReferenceCount -= 1;
		ERROR(BitmapReferenceCount < 0,PRERR(ForceAbort,
			"DisposeCheckBox:  bitmap reference count is negative"));
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


/* find out where the box is located */
OrdType					GetCheckBoxXLoc(CheckBoxRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->X;
	}


/* find out where the box is located */
OrdType					GetCheckBoxYLoc(CheckBoxRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Y;
	}


/* find out where the box is located */
OrdType					GetCheckBoxWidth(CheckBoxRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Width;
	}


/* find out where the box is located */
OrdType					GetCheckBoxHeight(CheckBoxRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Height;
	}


/* change the location of the box */
void						SetCheckBoxLocation(CheckBoxRec* TheButton,
									OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(TheButton);
		TheButton->X = X;
		TheButton->Y = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
	}


/* internal utility routine that actually does the redrawing */
static void			InternalRedrawCheckBox(CheckBoxRec* TheButton, MyBoolean Hilited)
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


/* redraw the box completely */
void						RedrawCheckBox(CheckBoxRec* TheButton)
	{
		InternalRedrawCheckBox(TheButton,False);
	}


/* handle mouse downs.  returns True if the state of the box changed. */
MyBoolean				CheckBoxMouseDown(CheckBoxRec* TheButton, OrdType X, OrdType Y)
	{
		MyBoolean				Inside;
		MyBoolean				OldInside;

		OldInside = False;
		do
			{
				Inside = CheckBoxHitTest(TheButton,X,Y);
				if (Inside != OldInside)
					{
						InternalRedrawCheckBox(TheButton,Inside);
						OldInside = Inside;
					}
			} while (GetAnEvent(&X,&Y,NIL,NIL,NIL,NIL) != eMouseUp);
		if (Inside)
			{
				TheButton->State = !TheButton->State;
			}
		InternalRedrawCheckBox(TheButton,False);
		return Inside;
	}


/* force the state of the box to a certain value */
void						SetCheckBoxState(CheckBoxRec* TheButton, MyBoolean TheState)
	{
		CheckPtrExistence(TheButton);
		TheButton->State = TheState;
		RedrawCheckBox(TheButton);
	}


/* get the state of the box */
MyBoolean				GetCheckBoxState(CheckBoxRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->State;
	}


/* check to see if the specified location is in the box */
MyBoolean				CheckBoxHitTest(CheckBoxRec* TheButton, OrdType X, OrdType Y)
	{
		return (X >= TheButton->X) && (Y >= TheButton->Y)
			&& (X < TheButton->X + TheButton->Width) && (Y < TheButton->Y + TheButton->Height);
	}
