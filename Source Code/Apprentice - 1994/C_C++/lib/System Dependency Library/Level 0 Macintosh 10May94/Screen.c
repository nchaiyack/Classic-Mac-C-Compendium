/* Screen.c */
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

#pragma options(pack_enums)
#include <Quickdraw.h>
#include <Windows.h>
#include <Menus.h>
#include <Fonts.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Script.h>
#pragma options(!pack_enums)

#include "Screen.h"
#include "Menus.h"
#include "Memory.h"
#include "EventLoop.h"
#include "Scrap.h"
#include "Array.h"
#include "Files.h"


/* maximum string length that can be sized in a single call to TextWidth or DrawText */
/* (such a big number doesn't actually make much sense since 65535 is the number of */
/* pixels, so if there are 32767 chars, we are way over the QuickDraw limit.) */
#define MAXTEXTSIZING (32767)


/* this structure contains everything about a window */
struct WinType
	{
		/* document window or dialog box */
		WinForm								WhatKindOfWindow;
		/* callback routine for redrawing it */
		void									(*UpdateRoutine)(void* Refcon);
		/* reference value for callback */
		void*									Refcon;
		/* current clip rect (for AddClipRect) */
		Rect									CurrentClipRect;
		/* actual window record */
		WindowRecord					ActualWindow;
	};


/* structure for bitmap */
struct Bitmap
	{
		/* bytes per row for larger than image or partial bitmaps */
		short									BytesPerRow; /* 2 bytes */
		/* dimensions of bitmap */
		OrdType								Width; /* 2 bytes */
		OrdType								Height; /* 2 bytes */
		/* the bitmap that the system uses */
		BitMap								SystemBitmap; /* 14 bytes */
		/* data array here (block will actually be as large as necessary) */
		char									Data[1]; /* long word aligned (20th offset) */
	};


/* list of windows */
static ArrayRec*					OurWindowList;

/* list of windows that need to be updated */
static ArrayRec*					PendingDeferredUpdates;

/* debugging flag */
EXECUTE(static MyBoolean	Initialized = False;)

/* pattern bitmap definitions.  patterns are 8x8 pixel blocks */
static unsigned char			PatternMapping[5][8] =
	{
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* White */
		{0x88,0x22,0x88,0x22,0x88,0x22,0x88,0x22}, /* Light Grey */
		{0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55}, /* Medium Grey */
		{0x77,0xDD,0x77,0xDD,0x77,0xDD,0x77,0xDD}, /* Dark Grey */
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF} /* Black */
	};

/* since I don't know how to get a list of fonts, I make this menu */
/* which contains all of the fonts and then I extract the names from it */
static MenuHandle					FunnyFontMenu;


/* Initialize the screen management subsystem.  This routine must be called before */
/* any graphics routines are called.  this routine initializes the entire Level 0 */
/* module set except for some optional modules (like Network).  if it returns  */
/* False then initialization failed and the program must terminate immediately. */
MyBoolean						InitializeScreen(void)
	{
		ERROR(Initialized,PRERR(ForceAbort,"InitializeScreen called more than once"));
		/* initialize operating system managers */
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		TEInit();
		InitDialogs(NIL);
		InitCursor();
		InitMenus();
#if __option(mc68020) || __option(mc68881)
#endif
#if __option(profile)
		{
			void				InitializeProfile(void);

			InitializeProfile();
		}
#endif
		/* initialize our local memory manager */
		if (!Eep_InitMemory())
			{
			 FailurePoint1:
				return False;
			}
		/* initialize error handling.  no return code from this */
		Eep_InitPRERR();
		/* initialize our window array */
		OurWindowList = NewArray();
		if (OurWindowList == NIL)
			{
			 FailurePoint2:
				Eep_FlushMemory();
				goto FailurePoint1;
			}
		/* initialize our deferred update list */
		PendingDeferredUpdates = NewArray();
		if (PendingDeferredUpdates == NIL)
			{
			 FailurePoint3:
				DisposeArray(OurWindowList);
				goto FailurePoint2;
			}
		/* initialize cut and paste */
		if (!Eep_InitializeScrapHandler())
			{
			 FailurePoint4:
				DisposeArray(PendingDeferredUpdates);
				goto FailurePoint3;
			}
		/* initialize event loop */
		if (!Eep_InitEventLoop())
			{
			 FailurePoint5:
				Eep_ShutdownScrapHandler();
				goto FailurePoint4;
			}
		/* initialize menu subsystem */
		if (!Eep_InitializeMenus())
			{
			 FailurePoint6:
				Eep_ShutdownEventLoop();
				goto FailurePoint5;
			}
		/* initialize file system */
		if (!Eep_InitializeFiles())
			{
			 FailurePoint7:
				Eep_ShutdownMenus();
				goto FailurePoint6;
			}
		/* create the font menu for obtaining names of fonts */
		/* menu manager uses menu IDs 256 and up, so we'll grab ID 1 */
		FunnyFontMenu = NewMenu(1,"\pSilly Stupid Little Font Menu");
		if (FunnyFontMenu == NIL)
			{
			 FailurePoint8:
				Eep_ShutdownFiles();
				goto FailurePoint7;
			}
		AddResMenu(FunnyFontMenu,'FONT');
		/* initialization done */
		EXECUTE(Initialized = True;)
		return True;
	}


/* close all open windows and perform any cleanup or server disconnection before */
/* the program terminates */
void								ShutdownScreen(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		/* shutdown subsystems in reverse order */
		DisposeMenu(FunnyFontMenu); /* get rid of the funny font menu */
		Eep_ShutdownFiles();
		Eep_ShutdownMenus();
		Eep_ShutdownEventLoop();
		Eep_ShutdownScrapHandler();
		/* clean up our own data structures */
		ERROR(ArrayGetLength(OurWindowList) != 0,PRERR(AllowResume,
			"ShutdownScreen:  there are still some windows open"));
		DisposeArray(OurWindowList);
		DisposeArray(PendingDeferredUpdates);
		/* dump memory subsystem */
		Eep_FlushMemory();
		Eep_ShutdownPRERR();
		EXECUTE(Initialized = False;)
#if __option(profile)
		{
			void				DumpProfile(void);
			void				ProfileKillElement(unsigned char* FunctionName);

			ProfileKillElement("\pGetAnEvent");
			ProfileKillElement("\pRelinquishCPUCheckCancel");
			ProfileKillElement("\pPutFile");
			ProfileKillElement("\pGetFileStandard");
			ProfileKillElement("\pGetFileAny");
			DumpProfile();
		}
#endif
	}


/* get size of screen.  If there are multiple screens, the result is implementation */
/* defined, but should not be counted on.  On the Macintosh, this returns only the */
/* size of the main screen.  Caveats aside, you are guarranteed that there is at */
/* least this much screen space in the form of a complete rectangle. */
OrdType							GetScreenHeight(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - GetMBarHeight();
	}


/* get size of screen.  If there are multiple screens, the result is implementation */
/* defined, but should not be counted on.  On the Macintosh, this returns only the */
/* size of the main screen.  Caveats aside, you are guarranteed that there is at */
/* least this much screen space in the form of a complete rectangle. */
OrdType							GetScreenWidth(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return qd.screenBits.bounds.right - qd.screenBits.bounds.left;
	}


/* how big is a window's title bar */
OrdType							WindowTitleBarHeight(WinForm WindowKind)
	{
		switch (WindowKind)
			{
				case eDocumentWindow:
					return 19;
				case eDialogWindow:
				case eModelessDialogWindow:
					return 6;
				default:
					EXECUTE(PRERR(AllowResume,"WindowTitleBarHeight:  Unknown window type"));
			}
	}


/* how big are the other edges of a window */
OrdType							WindowOtherEdgeWidths(WinForm WindowKind)
	{
		switch (WindowKind)
			{
				case eDocumentWindow:
					return 2;
				case eDialogWindow:
				case eModelessDialogWindow:
					return 6;
				default:
					EXECUTE(PRERR(AllowResume,"WindowTitleBarHeight:  Unknown window type"));
			}
	}


/* create a new window.  if WindowKind = eDocumentWindow, then the window is a */
/* standard window with a name (image is implementation defined).  In this case */
/* Zoomable determines whether there will be a "Maximize" button, and Closable */
/* determines whether there will be a "Close" button. */
/* The window returned will be considered in the "disabled" state and any */
/* objects installed in it should be disabled.  Eventually GetAnEvent will return */
/* an active window change event disabling the window previously on top and */
/* enabling this window. */
WinType*						MakeNewWindow(WinForm WindowKind, WinCloseType Closable,
											WinZoomType Zoomable, WinSizingType Resizing, OrdType Left,
											OrdType Top, OrdType Width, OrdType Height,
											void (*UpdateRoutine)(void* Refcon), void* Refcon)
	{
		WinType*					Window;
		Rect							Where;
		short							WDef;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ERROR(UpdateRoutine == NIL,PRERR(ForceAbort,
			"MakeNewWindow:  can't have NIL update routine"));
		/* allocate the window record */
		Window = (WinType*)AllocPtrCanFail(sizeof(WinType),"WindowRecord");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		/* add window to window list */
		if (!ArrayAppendElement(OurWindowList,Window))
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		/* create the actual window */
		Where.left = Left;
		Where.top = Top + GetMBarHeight();
		Where.right = Left + Width;
		Where.bottom = Top + Height + GetMBarHeight();
		switch (WindowKind)
			{
				case eDocumentWindow:
					if (Zoomable == eWindowZoomable)
						{
							WDef = zoomNoGrow;
						}
					 else
						{
							WDef = noGrowDocProc;
						}
					break;
				case eDialogWindow:
					WDef = dBoxProc;
					break;
				case eModelessDialogWindow:
					WDef = movableDBoxProc;
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"MakeNewWindow:  unknown WindowKind"));
					break;
			}
		if (NIL == NewWindow(&(Window->ActualWindow),&Where,"\p"/*notitle*/,
			True/*visible*/,WDef,(void*)-1L/*windowinfront*/,(Closable == eWindowClosable),
			(long)Window/*refcon is window's base pointer*/))
			{
			 FailurePoint3:
				ArrayDeleteElement(OurWindowList,ArrayFindElement(OurWindowList,Window));
				goto FailurePoint2;
			}
		/* initialize static fields */
		Window->CurrentClipRect.left = 0;
		Window->CurrentClipRect.top = 0;
		Window->CurrentClipRect.right = Width;
		Window->CurrentClipRect.bottom = Height;
		Window->UpdateRoutine = UpdateRoutine;
		Window->Refcon = Refcon;
		Window->WhatKindOfWindow = WindowKind;
		return Window;
	}


/* change the size of the window.  The window will be guarranteed to be the specified */
/* size, but significant portions may not be on screen, so be careful */
void								ResizeWindow(WinType* Window, OrdType Width, OrdType Height)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		/* change window size */
		SizeWindow((WindowPtr)&(Window->ActualWindow),Width,Height,False);
		/* invalidate the window so it gets updated */
		SetPort((WindowPtr)&(Window->ActualWindow));
		InvalRect(&(Window->ActualWindow.port.portRect));
	}


/* close a window and release all associated space.  The window refnum may be reused */
/* An active window change event will be issued activating the window that is */
/* next in the stack */
void								KillWindow(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		Eep_WindowDying(Window);
		ArrayDeleteElement(OurWindowList,ArrayFindElement(OurWindowList,Window));
		/* remove it from the pending updates list if it's still there */
		if (ArrayFindElement(PendingDeferredUpdates,Window) >= 0)
			{
				ArrayDeleteElement(PendingDeferredUpdates,
					ArrayFindElement(PendingDeferredUpdates,Window));
			}
		/* dispose the OS window */
		DisposeWindow((WindowPtr)&(Window->ActualWindow));
		/* dispose the window object */
		ReleasePtr((char*)Window);
	}


/* get the size of the usable portion of the window */
OrdType							GetWindowHeight(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		return Window->ActualWindow.port.portRect.bottom
			- Window->ActualWindow.port.portRect.top;
	}


/* get the size of the usable portion of the window */
OrdType							GetWindowWidth(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		return Window->ActualWindow.port.portRect.right
			- Window->ActualWindow.port.portRect.left;
	}


/* Get the global coordinate location of the window */
OrdType							GetWindowXStart(WinType* Window)
	{
		Point							Delta = {0,0};

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		/* this is really goofy, but I can't think of any other way of doing it. */
		SetPort((WindowPtr)&(Window->ActualWindow));
		LocalToGlobal(&Delta); /* find top-left with respect to screen */
		return Delta.h;
	}


/* Get the global coordinate location of the window */
OrdType							GetWindowYStart(WinType* Window)
	{
		Point			Delta = {0,0};

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		LocalToGlobal(&Delta); /* find top-left with respect to screen */
		return Delta.v - GetMBarHeight();
	}


/* Adjust the global position of the window. */
void								SetWindowPosition(WinType* Window, OrdType NewXLocation,
											OrdType NewYLocation)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		MoveWindow((WindowPtr)&(Window->ActualWindow),NewXLocation,
			NewYLocation + GetMBarHeight(),False);
	}


/* Get what type of window it is */
WinForm							GetWindowKind(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		return Window->WhatKindOfWindow;
	}


/* allow system to resize window after user clicked in some area */
void								UserGrowWindow(WinType* Window, OrdType X, OrdType Y)
	{
		Rect							Limits = {64,64,32767,32767};
		Point							Where;
		unsigned long			NewSize;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Where.h = X;
		Where.v = Y;
		LocalToGlobal(&Where);
		NewSize = GrowWindow((WindowPtr)&(Window->ActualWindow),Where,&Limits);
		SizeWindow((WindowPtr)&(Window->ActualWindow),
			NewSize & 0xffff,(NewSize >> 16) & 0xffff,False);
		Limits.left = 0;
		Limits.top = 0;
		Limits.right = GetWindowWidth(Window);
		Limits.bottom = GetWindowHeight(Window);
		/* invalidate window so it gets redrawn */
		EraseRect(&Limits);
		InvalRect(&Limits);
	}


/* bring window to the top of the window stack */
void								ActivateThisWindow(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SelectWindow((WindowPtr)&(Window->ActualWindow));
	}


/* this function is used by MakeWindowFitOnScreen */
static MyBoolean		SeeIfWindowFitsOnScreen(OrdType X, OrdType Y,
											OrdType Width, OrdType Height)
	{
		RgnHandle					OurRegion;
		RgnHandle					Intersection;
		MyBoolean					Result;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));

		/* default value:  in case we run out of memory, don't do anything */
		Result = True;
		/* we add some constants in here to account for window title bar */
		Y = Y + GetMBarHeight() - 19;
		Height = Height + 19;

		/* create a region for the window the user wants to create */
		OurRegion = NewRgn();
		if (OurRegion != NIL)
			{
				SetRectRgn(OurRegion,X,Y,X + Width,Y + Height);

				/* create a region to get the intersection of the window & the screen */
				Intersection = NewRgn();
				if (Intersection != NIL)
					{
						/* find the intersection of the regions */
						SectRgn(GetGrayRgn(),OurRegion,Intersection);

						/* if the window region and the intersection are equal, then the window */
						/* fits on the screen */
						Result = EqualRgn(Intersection,OurRegion);

						DisposeRgn(Intersection);
					}

				DisposeRgn(OurRegion);
			}

		return Result;
	}


/* this routine helps make sure the rectangle fits on the screen.  If the rectangle */
/* already fits on the screen, X and Y will not be adjusted, but if it doesn't, some */
/* undefined adjustment will be made to ensure that the rectangle fits on the screen. */
/* If the rectangle is so large that it can't be made to fit on the screen, then */
/* the size of the window is reduced so that the window will fit on screen. */
void								MakeWindowFitOnScreen(OrdType* X, OrdType* Y,
											OrdType* Width, OrdType* Height)
	{
		RgnHandle					OurRegion;
		RgnHandle					Intersection;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ERROR((X == NIL) || (Y == NIL) || (Width == NIL) || (Height == NIL),PRERR(ForceAbort,
			"MakeWindowFitOnScreen:  an output parameter is NIL"));
		if (!SeeIfWindowFitsOnScreen(*X,*Y,*Width,*Height))
			{
				*X = 3;
				*Y = 23;
				if (!SeeIfWindowFitsOnScreen(*X,*Y,*Width,*Height))
					{
						if (*Width > GetScreenWidth() - *X - 2 - 1)
							{
								*Width = GetScreenWidth() - *X - 2 - 1;
							}
						if (*Height > GetScreenHeight() - *Y - 2 - 1)
							{
								*Height = GetScreenHeight() - *Y - 2 - 1;
							}
					}
			}
	}


/* obtain the edge of a window, conforming to the user interface */
/* guidelines of the implementation's platform */
OrdType							AlertLeftEdge(OrdType AlertWidth)
	{
		short							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		Temp = (GetScreenWidth() - AlertWidth) / 2;
		if (Temp < 4)
			{
				Temp = 4;
			}
		return Temp;
	}


/* obtain the edge of a window, conforming to the user interface */
/* guidelines of the implementation's platform */
OrdType							AlertTopEdge(OrdType AlertHeight)
	{
		short							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		Temp = (GetScreenHeight() / 3) - AlertHeight;
		if (Temp < 4 + 20 + GetMBarHeight())
			{
				Temp = 4 + 20 + GetMBarHeight();
			}
		return Temp;
	}


/* obtain the edge of a window, conforming to the user interface */
/* guidelines of the implementation's platform */
OrdType							DialogLeftEdge(OrdType DialogWidth)
	{
		short							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		Temp = (GetScreenWidth() - DialogWidth) / 2;
		if (Temp < 4)
			{
				Temp = 4;
			}
		return Temp;
	}


/* obtain the edge of a window, conforming to the user interface */
/* guidelines of the implementation's platform */
OrdType							DialogTopEdge(OrdType DialogHeight)
	{
		short				Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		Temp = (GetScreenHeight() - DialogHeight) / 3;
		if (Temp < 4 + 20 + GetMBarHeight())
			{
				Temp = 4 + 20 + GetMBarHeight();
			}
		return Temp;
	}


/* change window's name */
void								SetWindowName(WinType* Window, char* Name)
	{
		unsigned char			NameTemp[256];
		long							Scan;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		ERROR(Name == NIL,PRERR(ForceAbort,"SetWindowName:  name is NIL"));
		Scan = 0;
		while ((Scan < 255) && (Name[Scan] != 0))
			{
				NameTemp[Scan + 1] = Name[Scan];
				Scan += 1;
			}
		NameTemp[0] = Scan;
		SetWTitle((WindowPtr)&(Window->ActualWindow),NameTemp);
	}


/* invoke a window's update routine */
void								CallWindowUpdate(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		(*Window->UpdateRoutine)(Window->Refcon);
	}


/* if the program is doing something so that processing an update event would */
/* cause it to crash, then the window is marked and will be redrawn as soon as */
/* possible.   This is used during RelinquishCPU in the EventLoop module since */
/* the windowing system might be in an inconsistent state when that routine is */
/* called.  (If we ignored the event, then the window would not be redrawn at all) */
void								MarkForDeferredUpdate(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		if (ArrayFindElement(PendingDeferredUpdates,Window) < 0)
			{
				/* don't add it to list unless it isn't there */
				/* if this fails, then it'll just not redraw the window. */
				ArrayAppendElement(PendingDeferredUpdates,Window);
			}
	}


/* redraw all windows whose updates have been deferred */
void								PerformDeferredUpdates(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		while (ArrayGetLength(PendingDeferredUpdates) != 0)
			{
				WinType*					Window;

				/* get the window */
				Window = ArrayGetElement(PendingDeferredUpdates,0);
				CheckPtrExistence(Window);
				/* remove the window from the deferred update list */
				ArrayDeleteElement(PendingDeferredUpdates,0);
				/* erase window */
				SetClipRect(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
				DrawBoxErase(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
				/* call the redraw callback */
				CallWindowUpdate(Window);
			}
	}


/* get the refcon from the window */
void*								GetWindowRefcon(WinType* Window)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		return Window->Refcon;
	}


/* set the clipping rectangle for the window.  Drawing outside of this rectangle */
/* will not be change any of the window */
void								SetClipRect(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Window->CurrentClipRect.left = Left;
		Window->CurrentClipRect.top = Top;
		Window->CurrentClipRect.right = Left + Width;
		Window->CurrentClipRect.bottom = Top + Height;
		ClipRect(&(Window->CurrentClipRect));
	}


/* constrain the clipping rectangle for the window.  The new clipping rectangle is */
/* the intersection of the specified one and the previous one. */
void								AddClipRect(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height)
	{
		Rect							Old;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Old = Window->CurrentClipRect;
		Window->CurrentClipRect.left = Left;
		Window->CurrentClipRect.top = Top;
		Window->CurrentClipRect.right = Left + Width;
		Window->CurrentClipRect.bottom = Top + Height;
		SectRect(&(Window->CurrentClipRect),&Old,&(Window->CurrentClipRect));
		ClipRect(&(Window->CurrentClipRect));
	}


/* returns True if any part of the specified rectangle in the window is visible. */
/* this is used for making redrawing more efficient. */
MyBoolean						IsRectVisible(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height)
	{
		Rect							Location;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		Location.left = Left;
		Location.top = Top;
		Location.right = Left + Width;
		Location.bottom = Top + Height;
		return (MyBoolean)RectInRgn(&Location,Window->ActualWindow.port.visRgn);
	}


/* Draw a line one pixel thick.  XDisp and YDisp may be negative. */
void								DrawLine(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		MoveTo(X,Y);
		LineTo(X + XDisp,Y + YDisp);
	}


/* Draw a box with a 1 pixel thick frame.  Note that the last pixel touched */
/* is X + XDisp - 1 and Y + YDisp - 1. */
void								DrawBoxFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		FrameRect(&Temp);
	}


/* paint the box with the specified pattern */
void								DrawBoxPaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		PaintRect(&Temp);
	}


/* paint the box with white */
void								DrawBoxErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		EraseRect(&Temp);
	}


#if 0
/* And-mask the contents of the box with the pattern */
void								DrawBoxScreen(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcBic);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		PaintRect(&Temp);
	}
#endif


/* Draw a box, but round off the corners with circles. */
void								DrawRoundBoxFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp,
											OrdType DiameterX, OrdType DiameterY)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		FrameRoundRect(&Temp,DiameterX,DiameterY);
	}


/* Draw a box, but round off the corners with circles. */
void								DrawRoundBoxPaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp,
											OrdType DiameterX, OrdType DiameterY)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		PaintRoundRect(&Temp,DiameterX,DiameterY);
	}


/* Draw a box, but round off the corners with circles. */
void								DrawRoundBoxErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp, OrdType DiameterX, OrdType DiameterY)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		EraseRoundRect(&Temp,DiameterX,DiameterY);
	}


void								DrawCircleFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		FrameOval(&Temp);
	}


void								DrawCirclePaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp)
	{
		Rect							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		PaintOval(&Temp);
	}


void								DrawCircleErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp)
	{
		Rect			Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Temp.left = X;
		Temp.top = Y;
		Temp.right = X + XDisp;
		Temp.bottom = Y + YDisp;
		EraseOval(&Temp);
	}


/* fill a triangle */
void								DrawTrianglePaint(WinType* Window, Patterns Pattern, OrdType X1,
											OrdType Y1, OrdType X2, OrdType Y2, OrdType X3, OrdType Y3)
	{
		PolyHandle				Poly;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Poly = OpenPoly();
		MoveTo(X1,Y1);
		LineTo(X2,Y2);
		LineTo(X3,Y3);
		LineTo(X1,Y1);
		ClosePoly();
		PenPat(PatternMapping[Pattern - eWhite]);
		PenMode(srcCopy);
		PaintPoly(Poly);
		KillPoly(Poly);
	}


/* Get the ID of a heavier screen font (Macintosh == Chicago) */
FontType						GetUglyFont(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return systemFont;
	}


/* Get the ID of the default screen font (Macintosh == Geneva) */
FontType						GetScreenFont(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return geneva;
	}


/* Get the ID of the normal monospaced font, usually courier or monaco */
FontType						GetMonospacedFont(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return monaco;
	}


/* Get the ID of the named font.  If no such font exists, then it is an error */
FontType						GetFontByName(char* Name)
	{
		unsigned char			NameTemp[256];
		long							Scan;
		short							FontID;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		Scan = 0;
		while ((Scan < 255) && (Name[Scan] != 0))
			{
				NameTemp[Scan + 1] = Name[Scan];
				Scan += 1;
			}
		NameTemp[0] = Scan;
		GetFNum(NameTemp,&FontID);
#if DEBUG
		if (FontID == 0)
			{
				unsigned char			SysFontName[256];

				GetFontName(0,SysFontName);
				if (!EqualString(NameTemp,SysFontName,False,False))
					{
						PRERR(AllowResume,"GetFontByName:  font doesn't exist");
					}
			}
#endif
		return FontID;
	}


/* Get the total number of pixels high a line is using the specified font */
OrdType							GetFontHeight(FontType FontID, FontSizeType PointSize)
	{
		GrafPort					Temp;
		FontInfo					TheFont;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		OpenPort(&Temp);
		TextFont(FontID);
		TextFace(0);
		TextMode(srcCopy);
		TextSize(PointSize);
		SpaceExtra(0);
		GetFontInfo(&TheFont);
		ClosePort(&Temp);
		return TheFont.ascent + TheFont.descent + TheFont.leading;
	}


/* return a Ptr containing the name of the font, null terminated */
char*								GetNameOfFont(FontType FontID)
	{
		unsigned char			DaFontName[256];
		char*							Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		GetFontName(FontID,DaFontName);
		ERROR(DaFontName[0] == 0,PRERR(AllowResume,"GetNameOfFont:  nonexistent font"));
		Temp = AllocPtrCanFail(DaFontName[0] + 1,"FontName");
		if (Temp != NIL)
			{
				CopyData((char*)&(DaFontName[1]),&(Temp[0]),DaFontName[0]);
				Temp[DaFontName[0]] = 0;
			}
		return Temp;
	}


/* get number of fonts */
long								GetNumAvailableFonts(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		return CountMItems(FunnyFontMenu);
	}


/* get the FontType of an indexed font. indices are from 0 to GetNumAvailableFonts - 1 */
FontType						GetIndexedFont(long FontIndex)
	{
		short							DaID;
		unsigned char			DaName[256];

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ERROR((FontIndex < 0) || (FontIndex >= GetNumAvailableFonts()),PRERR(ForceAbort,
			"GetIndexedFont:  index out of range"));
		/* we cleverly make a menu, pile the fonts in there, and then extract */
		GetItem(FunnyFontMenu,FontIndex + 1,DaName);
		GetFNum(DaName,&DaID);
		return DaID;
	}


/* find the total number of pixels long the string of text is */
OrdType							LengthOfText(FontType Font, FontSizeType PointSize, char* Text,
											long Length, FontStyleType FontStyle)
	{
		GrafPort					Temp;
		long							CurrentLinePixels;
		Style							FontThangs;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		/* open a temporary port to work from */
		OpenPort(&Temp);
		/* set font */
		TextFont(Font);
		/* figure out what style to use */
		FontThangs = 0;
		if ((FontStyle & eBold) != 0)
			{
				FontThangs |= bold;
			}
		if ((FontStyle & eItalic) != 0)
			{
				FontThangs |= italic;
			}
		if ((FontStyle & eUnderline) != 0)
			{
				FontThangs |= underline;
			}
		TextFace(FontThangs);
		TextMode(srcCopy);
		TextSize(PointSize);
		SpaceExtra(0);
		/* loop through string & add up lengths */
		CurrentLinePixels = 0;
		while (Length > 0)
			{
				if (Length > MAXTEXTSIZING)
					{
						CurrentLinePixels += TextWidth(Text,0,MAXTEXTSIZING);
						Length -= MAXTEXTSIZING;
						Text += MAXTEXTSIZING;
					}
				 else
					{
						CurrentLinePixels += TextWidth(Text,0,Length);
						Length = 0;
					}
			}
		ClosePort(&Temp);
		return CurrentLinePixels;
	}


/* draw a line of text */
void								DrawTextLine(WinType* Window, FontType Font, FontSizeType PointSize,
											char* Text, long Length, OrdType X, OrdType Y,
											FontStyleType FontStyle)
	{
		FontInfo					Info;
		Style							FontThangs;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ERROR(Text == NIL,PRERR(ForceAbort,"DrawTextLine:  Text is NIL"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		TextFont(Font);
		FontThangs = 0;
		if ((FontStyle & eBold) != 0)
			{
				FontThangs |= bold;
			}
		if ((FontStyle & eItalic) != 0)
			{
				FontThangs |= italic;
			}
		if ((FontStyle & eUnderline) != 0)
			{
				FontThangs |= underline;
			}
		TextFace(FontThangs);
		TextMode(srcCopy);
		TextSize(PointSize);
		SpaceExtra(0);
		GetFontInfo(&Info);
		MoveTo(X,Y + Info.leading + Info.ascent);
		while (Length > 0)
			{
				if (Length > MAXTEXTSIZING)
					{
						DrawText(Text,0,MAXTEXTSIZING);
						Length -= MAXTEXTSIZING;
						Text += MAXTEXTSIZING;
					}
				 else
					{
						DrawText(Text,0,Length);
						Length = 0;
					}
			}
		if (Info.leading > 0)
			{
				Rect				EraseMe;

				/* since srcCopy doesn't erase the leading, we have to do it. */
				EraseMe.left = X;
				EraseMe.top = Y;
				EraseMe.right = Window->ActualWindow.port.pnLoc.h;
				EraseMe.bottom = EraseMe.top + Info.leading;
				EraseRect(&EraseMe);
			}
	}


/* draw a line of text, but with white background and black letters */
void								InvertedTextLine(WinType* Window, FontType Font,
											FontSizeType PointSize, char* Text, long Length,
											OrdType X, OrdType Y, FontStyleType FontStyle)
	{
		FontInfo					Info;
		Style							FontThangs;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		TextFont(Font);
		FontThangs = 0;
		if ((FontStyle & eBold) != 0)
			{
				FontThangs |= bold;
			}
		if ((FontStyle & eItalic) != 0)
			{
				FontThangs |= italic;
			}
		if ((FontStyle & eUnderline) != 0)
			{
				FontThangs |= underline;
			}
		TextFace(FontThangs);
		TextMode(notSrcCopy);
		TextSize(PointSize);
		SpaceExtra(0);
		GetFontInfo(&Info);
		MoveTo(X,Y + Info.leading + Info.ascent);
		while (Length > 0)
			{
				if (Length > MAXTEXTSIZING)
					{
						DrawText(Text,0,MAXTEXTSIZING);
						Length -= MAXTEXTSIZING;
						Text += MAXTEXTSIZING;
					}
				 else
					{
						DrawText(Text,0,Length);
						Length = 0;
					}
			}
		if (Info.leading > 0)
			{
				Rect				EraseMe;

				/* since srcCopy doesn't erase the leading, we have to do it. */
				EraseMe.left = X;
				EraseMe.top = Y;
				EraseMe.right = Window->ActualWindow.port.pnLoc.h;
				EraseMe.bottom = EraseMe.top + Info.leading;
				PenPat(qd.black);
				PenMode(patCopy);
				PaintRect(&EraseMe);
			}
	}


/* move the specified rectangle of of pixels. XDisplacement and YDisplacement */
/* positive mean to the right and down.  Area opened up is erased with white. */
/* no area outside of the rectangle is touched. */
void								ScrollArea(WinType* Window, OrdType Left, OrdType Top, OrdType Width,
											OrdType Height, OrdType XDisplacement, OrdType YDisplacement)
	{
		Rect							Where;
		RgnHandle					UpdateRegion;
		RgnHandle					ScreenRegion;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		SetPort((WindowPtr)&(Window->ActualWindow));
		Where.left = Left;
		Where.top = Top;
		Where.right = Left + Width;
		Where.bottom = Top + Height;
		UpdateRegion = NewRgn();
		if (UpdateRegion != NIL)
			{
				ScrollRect(&Where,XDisplacement,YDisplacement,UpdateRegion);
				/* we discard the update region because the scroller is expected to redraw */
				/* it himself without getting and update event */
				DisposeRgn(UpdateRegion);
			}
	}


/* convert a raw packed-byte list of data (upper bit of each byte is leftmost */
/* on the screen) to an internal bitmap */
Bitmap*							MakeBitmap(char* RawData, OrdType Width, OrdType Height,
											long BytesPerRow)
	{
		short							TrueBytesPerRow;
		short							RowScan;
		short							ColumnScan;
		long							MapIndex;
		Bitmap*						Data;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ERROR(RawData == NIL,PRERR(ForceAbort,"MakeBitmap:  data is NIL"));
		/* calculate local bytes per row so that it's a long-word multiple */
		TrueBytesPerRow = ((Width + (8 * sizeof(long)) - 1) / (8 * sizeof(long))) * 4;
		Data = (Bitmap*)AllocPtrCanFail(TrueBytesPerRow * Height + sizeof(Bitmap),"Bitmap");
		if (Data == NIL)
			{
				return NIL;
			}
		MapIndex = 0;
		for (RowScan = 0; RowScan < Height; RowScan += 1)
			{
				/* copy over one row worth of data */
				for (ColumnScan = 0; ColumnScan < BytesPerRow; ColumnScan += 1)
					{
						Data->Data[MapIndex + ColumnScan] = RawData[ColumnScan];
					}
				/* pad the extra space with zeros */
				for (ColumnScan = 0; ColumnScan < TrueBytesPerRow - BytesPerRow;
					ColumnScan += 1)
					{
						Data->Data[MapIndex + ColumnScan + BytesPerRow] = 0;
					}
				MapIndex += TrueBytesPerRow;
				RawData += BytesPerRow;
			}
		/* assign internal values */
		Data->Width = Width;
		Data->Height = Height;
		Data->BytesPerRow = TrueBytesPerRow;
		/* initialize the system's bitmap */
		Data->SystemBitmap.baseAddr = Data->Data;
		Data->SystemBitmap.rowBytes = TrueBytesPerRow;
		Data->SystemBitmap.bounds.left = 0;
		Data->SystemBitmap.bounds.top = 0;
		Data->SystemBitmap.bounds.right = Data->Width;
		Data->SystemBitmap.bounds.bottom = Data->Height;
		return Data;
	}


/* dispose of the bitmap made by MakeBitmap */
void								DisposeBitmap(Bitmap* TheBitmap)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		ReleasePtr((char*)TheBitmap);
	}


/* copy the bitmap to the area specified. */
void								DrawBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap)
	{
		Rect							DestRect;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		CheckPtrExistence(TheBitmap);
		SetPort((WindowPtr)&(Window->ActualWindow));
		DestRect.left = X;
		DestRect.top = Y;
		DestRect.right = X + TheBitmap->Width;
		DestRect.bottom = Y + TheBitmap->Height;
		CopyBits(&(TheBitmap->SystemBitmap),&(Window->ActualWindow.port.portBits),
			&(TheBitmap->SystemBitmap.bounds),&DestRect,srcCopy,NIL);
	}


/* logical-or the bitmap onto the window */
void								OrBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap)
	{
		Rect							DestRect;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		CheckPtrExistence(TheBitmap);
		SetPort((WindowPtr)&(Window->ActualWindow));
		DestRect.left = X;
		DestRect.top = Y;
		DestRect.right = X + TheBitmap->Width;
		DestRect.bottom = Y + TheBitmap->Height;
		CopyBits(&(TheBitmap->SystemBitmap),&(Window->ActualWindow.port.portBits),
			&(TheBitmap->SystemBitmap.bounds),&DestRect,srcOr,NIL);
	}


/* Bit-clear the bitmap onto the window:  Where the bitmap is set, the */
/* window will be erased; otherwise the window will be untouched */
void								BicBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap)
	{
		Rect							DestRect;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Window);
		CheckPtrExistence(TheBitmap);
		SetPort((WindowPtr)&(Window->ActualWindow));
		DestRect.left = X;
		DestRect.top = Y;
		DestRect.right = X + TheBitmap->Width;
		DestRect.bottom = Y + TheBitmap->Height;
		CopyBits(&(TheBitmap->SystemBitmap),&(Window->ActualWindow.port.portBits),
			&(TheBitmap->SystemBitmap.bounds),&DestRect,srcBic,NIL);
	}


/* duplicate the bitmap */
Bitmap*							DuplicateBitmap(Bitmap* Original)
	{
		Bitmap*						Copy;
		long							TotalSize;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(Original);
		/* this is easy since there are no sub-objects */
		TotalSize = PtrSize((char*)Original);
		Copy = (Bitmap*)AllocPtrCanFail(TotalSize,"DuplicateBitmap");
		if (Copy != NIL)
			{
				CopyData((char*)Original,(char*)Copy,TotalSize);
				Copy->SystemBitmap.baseAddr = Copy->Data;
			}
		return Copy;
	}


/* logical-or the first bitmap onto the second.  sizes must be the same */
void								BitmapOrIntoBitmap(Bitmap* NotChanged, Bitmap* IsChanged)
	{
		long							Scan;
		long							Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(NotChanged);
		CheckPtrExistence(IsChanged);
		ERROR((NotChanged->BytesPerRow != IsChanged->BytesPerRow)
			|| (NotChanged->Width != IsChanged->Width)
			|| (NotChanged->Height != IsChanged->Height),PRERR(ForceAbort,
			"BitmapOrIntoBitmap:  bitmaps are not the same size"));
		/* we can perform operations on long words since we ensure that our */
		/* internal bitmap representation is long-word aligned */
		Limit = (IsChanged->BytesPerRow * IsChanged->Height) / sizeof(unsigned long);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				PRNGCHK(IsChanged,&(((unsigned long*)(IsChanged->Data))[Scan]),
					sizeof(unsigned long));
				PRNGCHK(NotChanged,&(((unsigned long*)(NotChanged->Data))[Scan]),
					sizeof(unsigned long));
				((unsigned long*)(IsChanged->Data))[Scan]
					|= ((unsigned long*)(NotChanged->Data))[Scan];
			}
	}


/* logical-and the first bitmap onto the second.  sizes must be the same */
void								BitmapAndIntoBitmap(Bitmap* NotChanged, Bitmap* IsChanged)
	{
		long							Scan;
		long							Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"Screen subsystem hasn't been initialized"));
		CheckPtrExistence(NotChanged);
		CheckPtrExistence(IsChanged);
		ERROR((NotChanged->BytesPerRow != IsChanged->BytesPerRow)
			|| (NotChanged->Width != IsChanged->Width)
			|| (NotChanged->Height != IsChanged->Height),PRERR(ForceAbort,
			"BitmapAndIntoBitmap:  bitmaps are not the same size"));
		/* we can perform operations on long words since we ensure that our */
		/* internal bitmap representation is long-word aligned */
		Limit = (IsChanged->BytesPerRow * IsChanged->Height) / sizeof(unsigned long);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				PRNGCHK(IsChanged,&(((unsigned long*)(IsChanged->Data))[Scan]),
					sizeof(unsigned long));
				PRNGCHK(NotChanged,&(((unsigned long*)(NotChanged->Data))[Scan]),
					sizeof(unsigned long));
				((unsigned long*)(IsChanged->Data))[Scan]
					&= ((unsigned long*)(NotChanged->Data))[Scan];
			}
	}
