/* Scroll.c */
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
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "Scroll.h"
#include "Screen.h"
#include "Memory.h"


#define SCROLLDELAY (0.1)


struct ScrollRec
	{
		long						Total;
		long						Current;
		OrdType					X;
		OrdType					Y;
		OrdType					Length;
		ScrollBarType		Direction;
		MyBoolean				Enabled;
		WinType*				Window;
	};


static unsigned char		RawUpArrow[] =
	{
		0xFF,0xFF,0x80,0x01,0x81,0x81,0x82,0x41,0x84,0x21,0x88,0x11,0x90,0x09,0x9C,0x39,
		0x84,0x21,0x84,0x21,0x84,0x21,0x84,0x21,0x87,0xE1,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawDownArrow[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x87,0xE1,0x84,0x21,0x84,0x21,0x84,0x21,0x84,0x21,
		0x9C,0x39,0x90,0x09,0x88,0x11,0x84,0x21,0x82,0x41,0x81,0x81,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawRightArrow[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x80,0xC1,0x80,0xA1,0x9F,0x91,0x90,0x09,0x90,0x05,
		0x90,0x05,0x90,0x09,0x9F,0x91,0x80,0xA1,0x80,0xC1,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawLeftArrow[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x83,0x01,0x85,0x01,0x89,0xF9,0x90,0x09,0xA0,0x09,
		0xA0,0x09,0x90,0x09,0x89,0xF9,0x85,0x01,0x83,0x01,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawUpArrowHilited[] =
	{
		0xFF,0xFF,0x80,0x01,0x81,0x81,0x83,0xC1,0x87,0xE1,0x8F,0xF1,0x9F,0xF9,0x9F,0xF9,
		0x87,0xE1,0x87,0xE1,0x87,0xE1,0x87,0xE1,0x87,0xE1,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawDownArrowHilited[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x87,0xE1,0x87,0xE1,0x87,0xE1,0x87,0xE1,0x87,0xE1,
		0x9F,0xF9,0x9F,0xF9,0x8F,0xF1,0x87,0xE1,0x83,0xC1,0x81,0x81,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawRightArrowHilited[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x80,0xC1,0x80,0xE1,0x9F,0xF1,0x9F,0xF9,0x9F,0xFD,
		0x9F,0xFD,0x9F,0xF9,0x9F,0xF1,0x80,0xE1,0x80,0xC1,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char		RawLeftArrowHilited[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x83,0x01,0x87,0x01,0x8F,0xF9,0x9F,0xF9,0xBF,0xF9,
		0xBF,0xF9,0x9F,0xF9,0x8F,0xF9,0x87,0x01,0x83,0x01,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static long					ArrowReferenceCount = 0;

static Bitmap*			LeftArrow = NIL;
static Bitmap*			RightArrow = NIL;
static Bitmap*			UpArrow = NIL;
static Bitmap*			DownArrow = NIL;
static Bitmap*			LeftArrowHilited = NIL;
static Bitmap*			RightArrowHilited = NIL;
static Bitmap*			UpArrowHilited = NIL;
static Bitmap*			DownArrowHilited = NIL;


/* create a new scroll bar, returning a Ptr to the private data structure. */
/* Length is the total number of pixels long the scroll bar is */
/* The scrollbar is initially in a DISABLED state. */
ScrollRec*			NewScrollBar(WinType* TheWindow, ScrollBarType Kind,
									OrdType X, OrdType Y, OrdType Length)
	{
		ScrollRec*			Temp;

		if (ArrowReferenceCount == 0)
			{
				LeftArrow = MakeBitmap(RawLeftArrow,16,16,2);
				if (LeftArrow == NIL)
					{
					 FailurePoint1:
						return NIL;
					}
				RightArrow = MakeBitmap(RawRightArrow,16,16,2);
				if (RightArrow == NIL)
					{
					 FailurePoint2:
						DisposeBitmap(LeftArrow);
						goto FailurePoint1;
					}
				UpArrow = MakeBitmap(RawUpArrow,16,16,2);
				if (UpArrow == NIL)
					{
					 FailurePoint3:
						DisposeBitmap(RightArrow);
						goto FailurePoint2;
					}
				DownArrow = MakeBitmap(RawDownArrow,16,16,2);
				if (DownArrow == NIL)
					{
					 FailurePoint4:
						DisposeBitmap(UpArrow);
						goto FailurePoint3;
					}
				LeftArrowHilited = MakeBitmap(RawLeftArrowHilited,16,16,2);
				if (LeftArrowHilited == NIL)
					{
					 FailurePoint5:
						DisposeBitmap(DownArrow);
						goto FailurePoint4;
					}
				RightArrowHilited = MakeBitmap(RawRightArrowHilited,16,16,2);
				if (RightArrowHilited == NIL)
					{
					 FailurePoint6:
						DisposeBitmap(LeftArrowHilited);
						goto FailurePoint5;
					}
				UpArrowHilited = MakeBitmap(RawUpArrowHilited,16,16,2);
				if (UpArrowHilited == NIL)
					{
					 FailurePoint7:
						DisposeBitmap(RightArrowHilited);
						goto FailurePoint6;
					}
				DownArrowHilited = MakeBitmap(RawDownArrowHilited,16,16,2);
				if (DownArrowHilited == NIL)
					{
					 FailurePoint8:
						DisposeBitmap(UpArrowHilited);
						goto FailurePoint7;
					}
			}
		ArrowReferenceCount += 1;
		Temp = (ScrollRec*)AllocPtrCanFail(sizeof(ScrollRec),"ScrollRec");
		if (Temp == NIL)
			{
				ArrowReferenceCount -= 1;
				if (ArrowReferenceCount == 0)
					{
						DisposeBitmap(DownArrowHilited);
						goto FailurePoint8;
					}
				 else
					{
						return NIL;
					}
			}
		Temp->Total = 1;
		Temp->Current = 0;
		Temp->X = X;
		Temp->Y = Y;
		Temp->Length = Length;
		Temp->Direction = Kind;
		Temp->Enabled = False;
		Temp->Window = TheWindow;
		return Temp;
	}


/* dispose the scroll bar record allocated by NewScrollBar */
void						DisposeScrollBar(ScrollRec* TheBar)
	{
		ArrowReferenceCount -= 1;
		ERROR(ArrowReferenceCount < 0,PRERR(ForceAbort,
			"DisposeScrollBar: negative reference count for scrollbar icons"));
		if (ArrowReferenceCount == 0)
			{
				DisposeBitmap(LeftArrow);
				DisposeBitmap(RightArrow);
				DisposeBitmap(UpArrow);
				DisposeBitmap(DownArrow);
				DisposeBitmap(LeftArrowHilited);
				DisposeBitmap(RightArrowHilited);
				DisposeBitmap(UpArrowHilited);
				DisposeBitmap(DownArrowHilited);
			}
		ReleasePtr((char*)TheBar);
	}


/* get the position on the screen of the scroll bar */
OrdType					GetScrollXPosition(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		return TheBar->X;
	}


/* get the position on the screen of the scroll bar */
OrdType					GetScrollYPosition(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		return TheBar->Y;
	}


/* get the position on the screen of the scroll bar */
OrdType					GetScrollLength(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		return TheBar->Length;
	}


/* set the position of the scroll bar on the screen */
void						SetScrollLocation(ScrollRec* TheBar, OrdType NewX, OrdType NewY,
									OrdType NewLength)
	{
		CheckPtrExistence(TheBar);
		TheBar->X = NewX;
		TheBar->Y = NewY;
		TheBar->Length = NewLength;
	}


/* get the maximum number of indices the scroll bar can have */
long						GetMaxScrollIndex(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		return TheBar->Total;
	}


/* get the current index for the scroll bar's position */
long						GetCurrentScrollIndex(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		return TheBar->Current;
	}


/* set the maximum number of indices the scroll bar can have */
void						SetMaxScrollIndex(ScrollRec* TheBar, long NewMaxScrollIndex)
	{
		CheckPtrExistence(TheBar);
		TheBar->Total = NewMaxScrollIndex;
	}


/* set the current index; NewIndex may be out of range */
void						SetScrollIndex(ScrollRec* TheBar, long NewIndex)
	{
		CheckPtrExistence(TheBar);
		TheBar->Current = NewIndex;
	}


/* enable or disable the scroll bars (for inactive windows) */
void						EnableScrollBar(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		TheBar->Enabled = True;
		RedrawScrollBar(TheBar);
	}


/* enable or disable the scroll bars (for inactive windows) */
void						DisableScrollBar(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		TheBar->Enabled = False;
		RedrawScrollBar(TheBar);
	}


/* internal routine for drawing scrollbar */
static void			InternalRedrawScrollBar(ScrollRec* TheBar,
									MyBoolean MinusArrowHit, MyBoolean PlusArrowHit)
	{
		long						Maximum;
		long						Position;
		OrdType					StartEdge;
		OrdType					EndEdge;
		OrdType					OtherCoordinate;
		Bitmap*					Arrow;
		WinType*				Window;
		ScrollBarType		Direction;
		OrdType					DohickyStartEdge;

		CheckPtrExistence(TheBar);
		Maximum = TheBar->Total;
		Position = TheBar->Current;
		Window = TheBar->Window;
		Direction = TheBar->Direction;
		switch (Direction)
			{
				case eVScrollBar:
					StartEdge = TheBar->Y;
					OtherCoordinate = TheBar->X;
					SetClipRect(Window,OtherCoordinate,StartEdge,OtherCoordinate + 16,
						StartEdge + TheBar->Length + 1);
					break;
				case eHScrollBar:
					StartEdge = TheBar->X;
					OtherCoordinate = TheBar->Y;
					SetClipRect(Window,StartEdge,OtherCoordinate,
						StartEdge + TheBar->Length + 1,OtherCoordinate + 16);
					break;
			}
		EndEdge = StartEdge + TheBar->Length;
		if ((EndEdge - StartEdge < 16 + 16 + 16) || !TheBar->Enabled)
			{
				/* either the scrollbar is too small to fit any pictures on */
				/* or it has been disabled (window deactivated), so we only draw */
				/* empty rectangles around the outline */
				switch (Direction)
					{
						case eVScrollBar:
							DrawBoxErase(Window,OtherCoordinate + 1,StartEdge + 1,
								16 - 2,EndEdge - StartEdge - 2);
							DrawBoxFrame(Window,eBlack,OtherCoordinate,StartEdge,
								16,EndEdge - StartEdge);
							break;
						case eHScrollBar:
							DrawBoxErase(Window,StartEdge + 1,OtherCoordinate + 1,
								EndEdge - StartEdge - 2,16 - 2);
							DrawBoxFrame(Window,eBlack,StartEdge,OtherCoordinate,
								EndEdge - StartEdge,16);
							break;
					}
				return;
			}
		switch (Direction)
			{
				case eVScrollBar:
					if (MinusArrowHit)
						{
							Arrow = UpArrowHilited;
						}
					 else
						{
							Arrow = UpArrow;
						}
					DrawBitmap(Window,OtherCoordinate,StartEdge,Arrow);
					if (PlusArrowHit)
						{
							Arrow = DownArrowHilited;
						}
					 else
						{
							Arrow = DownArrow;
						}
					DrawBitmap(Window,OtherCoordinate,EndEdge - 16,Arrow);
					break;
				case eHScrollBar:
					if (MinusArrowHit)
						{
							Arrow = LeftArrowHilited;
						}
					 else
						{
							Arrow = LeftArrow;
						}
					DrawBitmap(Window,StartEdge,OtherCoordinate,Arrow);
					if (PlusArrowHit)
						{
							Arrow = RightArrowHilited;
						}
					 else
						{
							Arrow = RightArrow;
						}
					DrawBitmap(Window,EndEdge - 16,OtherCoordinate,Arrow);
					break;
			}
		StartEdge += 16;
		EndEdge -= 16;
		if (Maximum > 1)
			{
				DohickyStartEdge = (((EndEdge - 16 - StartEdge)
					* Position) / (Maximum - 1)) + StartEdge;
				if (DohickyStartEdge < StartEdge)
					{
						DohickyStartEdge = StartEdge;
					}
				if (DohickyStartEdge > EndEdge - 16)
					{
						DohickyStartEdge = EndEdge - 16;
					}
				/* draw the dohicky thing */
			 RedrawPoint:
				switch (Direction)
					{
						case eVScrollBar:
							DrawBoxPaint(Window,eLightGrey,OtherCoordinate + 1,StartEdge,
								16 - 2,EndEdge - StartEdge);
							DrawBoxFrame(Window,eBlack,OtherCoordinate,StartEdge - 1,
								16,EndEdge - StartEdge + 2);
							DrawBoxErase(Window,OtherCoordinate + 1,DohickyStartEdge,14,16);
							DrawBoxFrame(Window,eBlack,OtherCoordinate + 1,DohickyStartEdge,14,16);
							break;
						case eHScrollBar:
							DrawBoxPaint(Window,eLightGrey,StartEdge,OtherCoordinate + 1,
								EndEdge - StartEdge,16 - 2);
							DrawBoxFrame(Window,eBlack,StartEdge - 1,OtherCoordinate,
								EndEdge - StartEdge + 2,16);
							DrawBoxErase(Window,DohickyStartEdge + 1,
								OtherCoordinate + 2,16 - 2,14 - 2);
							DrawBoxFrame(Window,eBlack,DohickyStartEdge,OtherCoordinate + 1,16,14);
							break;
					}
			}
		 else
			{
				/* Maximum is either negative, 0, or 1. */
				if (Position < 0)
					{
						DohickyStartEdge = StartEdge;
						goto RedrawPoint;
					}
				else if (Position > 0)
					{
						DohickyStartEdge = EndEdge - 16;
						goto RedrawPoint;
					}
				else
					{
						switch (Direction)
							{
								case eVScrollBar:
									DrawBoxErase(Window,OtherCoordinate + 1,StartEdge + 1 - 1,
										16 - 2,EndEdge - StartEdge - 2 + 2);
									DrawBoxFrame(Window,eBlack,OtherCoordinate,StartEdge - 1,
										16,EndEdge - StartEdge + 2);
									break;
								case eHScrollBar:
									DrawBoxErase(Window,StartEdge + 1 - 1,OtherCoordinate + 1,
										EndEdge - StartEdge - 2,16 - 2 + 2);
									DrawBoxFrame(Window,eBlack,StartEdge - 1,OtherCoordinate,
										EndEdge - StartEdge + 2,16);
									break;
							}
					}
			}
		/* the end! */
	}


/* this is so that we don't redraw the bar incorrectly.  What might happen is that */
/* the scrollbar gets redrawn from the scrollhook callback.  That would draw it */
/* normally, which would turn off any hilighting of the arrows.  So we use this */
/* variable to prevent that from happening.  We can get away with this because only */
/* one scrollbar can be scrolling at any time. */
static ScrollRec*				TheOneThatsScrolling = NIL;


/* redraw the scroll bar normally */
void						RedrawScrollBar(ScrollRec* TheBar)
	{
		CheckPtrExistence(TheBar);
		if (TheOneThatsScrolling != TheBar)
			{
				InternalRedrawScrollBar(TheBar,False,False);
			}
	}


/* internal states for tracking what part of the scrollbar was clicked in */
typedef enum
	{
		eNoButton EXECUTE(= -134),
		eMinusButton,
		ePlusButton,
		ePageMinus,
		ePagePlus,
		eDohickyBox
	} States;

/* states for throttle control */
typedef enum
	{
		eWaitThrottle1 EXECUTE(= -552),
		eWaitThrottle2,
		eNoThrottle
	} Throttles;

/* call this when the mouse goes down in the scroll bar, providing the */
/* information which came with the mouse down event.  ScrollHook is a routine */
/* which actually performs the scrolling: */
/* If How == eScrollToPosition, then scroll and redraw the image at the index */
/* represented by Parameter */
/* If How == eScrollPageMinus or eScrollPagePlus, then scroll as much of the */
/* image is necessary to scroll a page worth */
void						ScrollHitProc(ScrollRec* TheBar, ModifierFlags Modifiers,
									OrdType X, OrdType Y, void* Refcon,
									void (*ScrollHook)(long Parameter, ScrollType How, void* Refcon))
	{
		long						Maximum;
		long						Position;
		OrdType					StartEdge;
		OrdType					EndEdge;
		OrdType					OtherCoordinate;
		WinType*				Window;
		ScrollBarType		Direction;
		OrdType					DohickyStartEdge;
		OrdType					MouseInlineIndex;
		OrdType					MouseOtherIndex;
		States					State;
		MyBoolean				DohickyEnabled;
		Throttles				ThrottleState;

		State = eNoButton;
		CheckPtrExistence(TheBar);
		TheOneThatsScrolling = TheBar; /* prevent them from redrawing it; only we can */
		ThrottleState = eWaitThrottle1;
	 LoopPoint:
		if ((ThrottleState != eNoThrottle) && ((State == eMinusButton)
			|| (State == ePlusButton) || (State == ePageMinus) || (State == ePagePlus)))
			{
				double					When;

				/* the first time through the loop, delay a bit */
				switch (ThrottleState)
					{
						default:
							EXECUTE(PRERR(AllowResume,"ScrollHitProc:  unknown throttle state"));
							break;
						case eWaitThrottle1:
							ThrottleState = eWaitThrottle2;
							break;
						case eWaitThrottle2:
							ThrottleState = eNoThrottle;
							When = ReadTimer();
							while (TimerDifference(ReadTimer(),When) < SCROLLDELAY)
								{
									if (GetAnEvent(&X,&Y,&Modifiers,NIL,NIL,NIL) == eMouseUp)
										{
											goto DoneScrollingExitPoint;
										}
								}
							break;
						case eNoThrottle:
							break;
					}
			}
		Maximum = TheBar->Total;
		Position = TheBar->Current;
		Window = TheBar->Window;
		Direction = TheBar->Direction;
		switch (Direction)
			{
				case eVScrollBar:
					StartEdge = TheBar->Y;
					OtherCoordinate = TheBar->X;
					break;
				case eHScrollBar:
					StartEdge = TheBar->X;
					OtherCoordinate = TheBar->Y;
					break;
			}
		EndEdge = StartEdge + TheBar->Length;
		switch (Direction)
			{
				case eVScrollBar:
					MouseInlineIndex = Y;
					MouseOtherIndex = X;
					break;
				case eHScrollBar:
					MouseInlineIndex = X;
					MouseOtherIndex = Y;
					break;
			}
		if (Maximum != 1)
			{
				DohickyStartEdge = ((((EndEdge - 16) - 16 - (StartEdge + 16))
					* Position) / (Maximum - 1)) + (StartEdge + 16);
				if (DohickyStartEdge < (StartEdge + 16))
					{
						DohickyStartEdge = (StartEdge + 16);
					}
				if (DohickyStartEdge > (EndEdge - 16) - 16)
					{
						DohickyStartEdge = (EndEdge - 16) - 16;
					}
				DohickyEnabled = True;
			}
		 else
			{
				DohickyEnabled = False;
			}

		switch (State)
			{
				case eMinusButton:
					/* only scroll if we are in the button */
					if ((MouseInlineIndex >= StartEdge)
						&& (MouseInlineIndex < StartEdge + 16)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							(*ScrollHook)(0,eScrollLineMinus,Refcon);
							InternalRedrawScrollBar(TheBar,True,False);
						}
					 else
						{
							InternalRedrawScrollBar(TheBar,False,False);
						}
					break;
				case ePlusButton:
					if ((MouseInlineIndex < EndEdge)
						&& (MouseInlineIndex >= EndEdge - 16)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							/* hit in plus button */
							(*ScrollHook)(0,eScrollLinePlus,Refcon);
							InternalRedrawScrollBar(TheBar,False,True);
						}
					 else
						{
							InternalRedrawScrollBar(TheBar,False,False);
						}
					break;
				case ePageMinus:
					if ((MouseInlineIndex >= StartEdge)
						&& (MouseInlineIndex < DohickyStartEdge)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							(*ScrollHook)(0,eScrollPageMinus,Refcon);
							InternalRedrawScrollBar(TheBar,False,False);
						}
					break;
				case ePagePlus:
					if ((MouseInlineIndex < EndEdge)
						&& (MouseInlineIndex >= DohickyStartEdge + 16)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							(*ScrollHook)(0,eScrollPagePlus,Refcon);
							InternalRedrawScrollBar(TheBar,False,False);
						}
					break;
				case eDohickyBox:
					if (DohickyEnabled)
						{
							Position = (((((MouseInlineIndex - 8)
								- (StartEdge + 16)) * (Maximum - 1))
								+ (((EndEdge - 16) - (StartEdge + 16) - 16) / 2))
								/ ((EndEdge - 16) - (StartEdge + 16) - 16));
							if (Position < 0)
								{
									Position = 0;
								}
							if (Position > Maximum - 1)
								{
									Position = Maximum - 1;
								}
							if (Position != TheBar->Current)
								{
									(*ScrollHook)(Position,eScrollToPosition,Refcon);
									InternalRedrawScrollBar(TheBar,False,False);
								}
						}
					break;
				case eNoButton:
					/* we don't know where we are, so we hit test to find where we start */
					if ((MouseInlineIndex >= StartEdge)
						&& (MouseInlineIndex < StartEdge + 16)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							State = eMinusButton;
							goto LoopPoint;
						}
					if ((MouseInlineIndex < EndEdge)
						&& (MouseInlineIndex >= EndEdge - 16)
						&& (MouseOtherIndex >= OtherCoordinate)
						&& (MouseOtherIndex < OtherCoordinate + 16))
						{
							State = ePlusButton;
							goto LoopPoint;
						}
					/* wasn't in a button, so it's somewhere in the grey area between them */
					EndEdge -= 16;
					StartEdge += 16;
					if (DohickyEnabled)
						{
						 DohickyPoint:
							if ((MouseInlineIndex >= StartEdge)
								&& (MouseInlineIndex < DohickyStartEdge)
								&& (MouseOtherIndex >= OtherCoordinate)
								&& (MouseOtherIndex < OtherCoordinate + 16))
								{
									State = ePageMinus;
									goto LoopPoint;
								}
							if ((MouseInlineIndex < EndEdge)
								&& (MouseInlineIndex >= DohickyStartEdge + 16)
								&& (MouseOtherIndex >= OtherCoordinate)
								&& (MouseOtherIndex < OtherCoordinate + 16))
								{
									State = ePagePlus;
									goto LoopPoint;
								}
							if ((MouseInlineIndex >= DohickyStartEdge)
								&& (MouseInlineIndex < DohickyStartEdge + 16)
								&& (MouseOtherIndex >= OtherCoordinate)
								&& (MouseOtherIndex < OtherCoordinate + 16))
								{
									State = eDohickyBox;
									goto LoopPoint;
								}
						}
					 else
						{
							/* no dohicky */
							if (Position < 0)
								{
									DohickyStartEdge = StartEdge;
									MouseInlineIndex = DohickyStartEdge + 16;
									goto DohickyPoint;
								}
							else if (Position > 0)
								{
									DohickyStartEdge = EndEdge - 16;
									MouseInlineIndex = DohickyStartEdge - 1;
									goto DohickyPoint;
								}
						}
			}

		/* reading new mouse position */
		if (GetAnEvent(&X,&Y,&Modifiers,NIL,NIL,NIL) == eMouseUp)
			{
			 DoneScrollingExitPoint:
				InternalRedrawScrollBar(TheBar,False,False);
				TheOneThatsScrolling = NIL; /* they can redraw again */
				return; /* all done */
			}
		GetMousePosition(&X,&Y);
		goto LoopPoint;
	}


/* see if the position is in the scrollbar box */
MyBoolean				ScrollHitTest(ScrollRec* TheBar, OrdType X, OrdType Y)
	{
		switch (TheBar->Direction)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"ScrollHitTest:  bad internal scrollbar type"));
					break;
				case eVScrollBar:
					return (X >= TheBar->X) && (Y >= TheBar->Y)
						&& (X < TheBar->X + 16) && (Y < TheBar->Y + TheBar->Length);
				case eHScrollBar:
					return (X >= TheBar->X) && (Y >= TheBar->Y)
						&& (X < TheBar->X + TheBar->Length) && (Y < TheBar->Y + 16);
			}
	}
