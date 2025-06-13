/* IconButton.c */
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

#include "IconButton.h"
#include "Memory.h"
#include "EventLoop.h"


struct IconButtonRec
	{
		WinType*					Window;
		OrdType						XLoc;
		OrdType						YLoc;
		OrdType						Width;
		OrdType						Height;
		Bitmap*						Unselected;
		Bitmap*						UnselectedMouseDown;
		Bitmap*						Selected;
		Bitmap*						SelectedMouseDown;
		MyBoolean					DisposeBitmapsWhenDone;
		MyBoolean					ButtonState;
		IconButtonModes		Mode;
	};


/* allocate a new icon button.  This one is used if the button should convert */
/* the specified raw images into internal bitmaps. */
IconButtonRec*	NewIconButtonRawBitmaps(WinType* Window, OrdType X, OrdType Y,
									OrdType Width, OrdType Height, unsigned char* RawUnselected,
									unsigned char* RawUnselectedMouseDown, unsigned char* RawSelected,
									unsigned char* RawSelectedMouseDown, int BytesPerRow,
									IconButtonModes WhatMode)
	{
		IconButtonRec*	TheButton;

		TheButton = (IconButtonRec*)AllocPtrCanFail(sizeof(IconButtonRec),"IconButtonRec");
		if (TheButton == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		TheButton->Unselected = MakeBitmap(RawUnselected,Width,Height,BytesPerRow);
		if (TheButton->Unselected == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)TheButton);
				goto FailurePoint1;
			}
		TheButton->UnselectedMouseDown = MakeBitmap(RawUnselectedMouseDown,
			Width,Height,BytesPerRow);
		if (TheButton->UnselectedMouseDown == NIL)
			{
			 FailurePoint3:
				DisposeBitmap(TheButton->Unselected);
				goto FailurePoint2;
			}
		TheButton->Selected = MakeBitmap(RawSelected,Width,Height,BytesPerRow);
		if (TheButton->Selected == NIL)
			{
			 FailurePoint4:
				DisposeBitmap(TheButton->UnselectedMouseDown);
				goto FailurePoint3;
			}
		TheButton->SelectedMouseDown = MakeBitmap(RawSelectedMouseDown,
			Width,Height,BytesPerRow);
		if (TheButton->SelectedMouseDown == NIL)
			{
			 FailurePoint5:
				DisposeBitmap(TheButton->Selected);
				goto FailurePoint4;
			}
		TheButton->DisposeBitmapsWhenDone = True;
		TheButton->Window = Window;
		TheButton->XLoc = X;
		TheButton->YLoc = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
		TheButton->ButtonState = False;
		TheButton->Mode = WhatMode;
		return TheButton;
	}


/* allocate a new icon button.  this one is used if the bitmaps are already */
/* allocated and you just want to use them.  when disposed, the bitmaps will NOT */
/* be disposed. */
IconButtonRec*	NewIconButtonPreparedBitmaps(WinType* Window, OrdType X, OrdType Y,
									OrdType Width, OrdType Height, Bitmap* Unselected,
									Bitmap* UnselectedMouseDown, Bitmap* Selected,
									Bitmap* SelectedMouseDown, IconButtonModes WhatMode)
	{
		IconButtonRec*	TheButton;

		TheButton = (IconButtonRec*)AllocPtrCanFail(sizeof(IconButtonRec),"IconButtonRec");
		if (TheButton == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		TheButton->Window = Window;
		TheButton->XLoc = X;
		TheButton->YLoc = Y;
		TheButton->Width = Width;
		TheButton->Height = Height;
		TheButton->Unselected = Unselected;
		TheButton->UnselectedMouseDown = UnselectedMouseDown;
		TheButton->Selected = Selected;
		TheButton->SelectedMouseDown = SelectedMouseDown;
		TheButton->DisposeBitmapsWhenDone = False;
		TheButton->ButtonState = False;
		TheButton->Mode = WhatMode;
		return TheButton;
	}


/* dispose the button, and the bitmaps if appropriate */
void						DisposeIconButton(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		if (TheButton->DisposeBitmapsWhenDone)
			{
				DisposeBitmap(TheButton->Unselected);
				DisposeBitmap(TheButton->UnselectedMouseDown);
				DisposeBitmap(TheButton->Selected);
				DisposeBitmap(TheButton->SelectedMouseDown);
			}
		ReleasePtr((char*)TheButton);
	}


/* find out where the icon button is */
OrdType					GetIconButtonXLoc(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->XLoc;
	}


/* find out where the icon button is */
OrdType					GetIconButtonYLoc(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->YLoc;
	}


/* find out where the icon button is */
OrdType					GetIconButtonWidth(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Width;
	}


/* find out where the icon button is */
OrdType					GetIconButtonHeight(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		return TheButton->Height;
	}


/* change the location of the icon button */
void						SetIconButtonLocation(IconButtonRec* TheButton, OrdType X, OrdType Y)
	{
		CheckPtrExistence(TheButton);
		TheButton->XLoc = X;
		TheButton->YLoc = Y;
		RedrawIconButton(TheButton);
	}


/* this is an internal routine for redrawing the button. */
static void			IconButtonInternalRedraw(IconButtonRec* TheButton, MyBoolean MouseDown)
	{
		CheckPtrExistence(TheButton);
		SetClipRect(TheButton->Window,TheButton->XLoc,TheButton->YLoc,
			TheButton->Width,TheButton->Height);
		if (TheButton->ButtonState)
			{
				if (MouseDown)
					{
						CheckPtrExistence(TheButton->SelectedMouseDown);
						DrawBitmap(TheButton->Window,TheButton->XLoc,TheButton->YLoc,
							TheButton->SelectedMouseDown);
					}
				 else
					{
						CheckPtrExistence(TheButton->Selected);
						DrawBitmap(TheButton->Window,TheButton->XLoc,TheButton->YLoc,
							TheButton->Selected);
					}
			}
		 else
			{
				if (MouseDown)
					{
						CheckPtrExistence(TheButton->UnselectedMouseDown);
						DrawBitmap(TheButton->Window,TheButton->XLoc,TheButton->YLoc,
							TheButton->UnselectedMouseDown);
					}
				 else
					{
						CheckPtrExistence(TheButton->Unselected);
						DrawBitmap(TheButton->Window,TheButton->XLoc,TheButton->YLoc,
							TheButton->Unselected);
					}
			}
	}


/* do a full redraw of the button */
void						RedrawIconButton(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		IconButtonInternalRedraw(TheButton,False);
	}


/* handle a mouse down in the button.  returns True if the state of the button */
/* changed.  (or if the mouse went up inside the button, if it is in simple */
/* button mode.)  If Tracking != NIL, then it will be repeatedly called, with */
/* the Inside status until the mouse goes up. */
MyBoolean				IconButtonMouseDown(IconButtonRec* TheButton, OrdType X, OrdType Y,
									void (*Tracking)(void* Refcon, MyBoolean Inside), void* Refcon)
	{
		MyBoolean				Inside;
		MyBoolean				OldInside;

		OldInside = False;
		do
			{
				Inside = IconButtonHitTest(TheButton,X,Y);
				if (Inside != OldInside)
					{
						IconButtonInternalRedraw(TheButton,Inside);
						OldInside = Inside;
					}
				if (Tracking != NIL)
					{
						(*Tracking)(Refcon,Inside);
					}
			} while (GetAnEvent(&X,&Y,NIL,NIL,NIL,NIL) != eMouseUp);
		switch (TheButton->Mode)
			{
				case eIconCheckMode:
					if (Inside)
						{
							TheButton->ButtonState = !TheButton->ButtonState;
						}
					break;
				case eIconRadioMode:
					if (Inside && !TheButton->ButtonState)
						{
							TheButton->ButtonState = !TheButton->ButtonState;
						}
					 else
						{
							Inside = False;
						}
					break;
				case eIconSimpleMode:
					/* don't change button state at all, just return inside status. */
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"IconButtonMouseDown:  bad mode selector"));
					break;
			}
		IconButtonInternalRedraw(TheButton,False);
		return Inside;
	}


/* if the button is a stateful button, this forces the state to a value */
void						SetIconButtonState(IconButtonRec* TheButton, MyBoolean TheState)
	{
		CheckPtrExistence(TheButton);
		ERROR(TheButton->Mode == eIconSimpleMode,PRERR(AllowResume,
			"SetIconButtonState called on button in SimpleButton mode"));
		if (TheButton->ButtonState != TheState)
			{
				TheButton->ButtonState = TheState;
				RedrawIconButton(TheButton);
			}
	}


/* get the value of the state variable */
MyBoolean				GetIconButtonState(IconButtonRec* TheButton)
	{
		CheckPtrExistence(TheButton);
		ERROR(TheButton->Mode == eIconSimpleMode,PRERR(AllowResume,
			"GetIconButtonState called on button in SimpleButton mode"));
		return TheButton->ButtonState;
	}


/* see if the specified location is within the bounds of the button */
MyBoolean				IconButtonHitTest(IconButtonRec* TheButton, OrdType X, OrdType Y)
	{
		return (X >= TheButton->XLoc) && (Y >= TheButton->YLoc)
			&& (X < TheButton->XLoc + TheButton->Width)
			&& (Y < TheButton->YLoc + TheButton->Height);
	}
