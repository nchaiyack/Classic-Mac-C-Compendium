/* EventLoop.c */
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

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <AppleEvents.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <Quickdraw.h>
#include <Desk.h>
#include <ToolUtils.h>
#include <GestaltEqu.h>
#include <Power.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "MyMalloc.h"
#include "EventLoop.h"
#include "Memory.h"
#include "Menus.h"


/* how many 1/60ths of a second occur between mouse image update events */
#define CURSORUPDATEDELAY (15)

/* how many 1/60ths of a second should we leave menu hilited */
#define MENUDELAY (4)

/* how many 1/60ths of a second between WaitNextEvents() when calling */
/* RelinquishCPUJudiciously while application is in the foreground. */
#define JUDICIOUSDELAYFOREGROUND (19)

/* same as JUDICIOUSDELAYFOREGROUND, except for when application is in background */
#define JUDICIOUSDELAYBACKGROUND (5)

/* timeout factor for event loop when mouse is down (to facilitate mouse tracking) */
#define VERYSMALLTIMEINTERVAL (1)

/* what timeout should RelinquishCPU (not judiciously) use */
#define RELINQUISHCPUNORMALDELAY (5)

/* how long should we beep or invert the menu bar */
#define BEEPDURATION (20)

/* possible states that the menu handling stuff could be in */
typedef enum
	{
		eNoMenu EXECUTE(= -8764),
		eMenuPendingMouse,
		eMenuPendingKey,
		eMenuSelected
	} MenuStates;


/* NIL = no mouse down; otherwise, it's the window that the mouse went down in */
/* so that the mouse up event can be reported to the same window. */
static WindowPtr			LastMouseDownInThisWindow = NIL;

/* when did we last check the mouse cursor image. */
static long						LastCursorCheck = 0;

/* when was the last time we actually waited for an event (this is used by */
/* RelinquishCPUJudiciously to keep from doing it too often) */
static long						LastEventTime = 0;

/* what is the current timeout for WaitNextEvent */
static long						SleepTime = CURSORUPDATEDELAY;

/* what window was the last one that was active (for detecting changes) */
static WindowPtr			LastActiveWindow = NIL;

/* flag indicating whether we are in the foreground or not */
static MyBoolean			RunningInForeground = True;

/* sticky flag that remembers if the user tried to cancel during RelinquishCPU */
static MyBoolean			CancelPending = False;

/* buffer for keys received during RelinquishCPU.  if it is NIL, then the buffer */
/* does not exist. */
static EventRecord*		KeyboardEventBuffer = NIL;

/* this is used during RelinquishCPUJudiciously to keep PowerBooks from */
/* going into idle state */
static MyBoolean			IsThisAPowerBook = False;

/* current delay for RelinquishCPUJudiciously.  It depends on RunningInForeground */
static long						JudiciousInterval = JUDICIOUSDELAYFOREGROUND;

/* flag that tells whether we should make beeping noises or not */
static MyBoolean			MakeErrorBeeps = True;


#if (CURRENTPROCTYPE == PROC68000) || (CURRENTPROCTYPE == PROC68020)
	/* efficiency hack for 680x0 Macs */
	#define TickCount() (*((volatile unsigned long*)0x016a))
#else
	/* 'nicer' for PowerPC */
#endif


/* initialize internal event loop data structures */
MyBoolean					Eep_InitEventLoop(void)
	{
		OSErr						Error;
		long						Result;

		/* allocate keyboard buffer.  if this fails & it returns NIL, it's ok. */
		KeyboardEventBuffer = (EventRecord*)AllocPtrCanFail(0,"KeyboardEventBuffer");
		/* figure out if we are running on a sleepy powerbook */
		Error = Gestalt(gestaltPowerMgrAttr,&Result);
		IsThisAPowerBook = ((Result & (1 << gestaltPMgrExists)) != 0) && (Error == noErr);
		return True;
	}


/* dispose of any internal event loop structures */
void							Eep_ShutdownEventLoop(void)
	{
		if (KeyboardEventBuffer != NIL)
			{
				ReleasePtr((char*)KeyboardEventBuffer);
			}
		KeyboardEventBuffer = NIL;
	}


/* returns the ID number of the current window.  Returns 0 if there are no windows */
static WinType*	GetCurrentWindow(void)
	{
		if ((FrontWindow() != NIL) && RunningInForeground)
			{
				CheckPtrExistence((WinType*)GetWRefCon(FrontWindow()));
				return (WinType*)GetWRefCon(FrontWindow());
			}
		 else
			{
				return NIL;
			}
	}


/* local routine that converts Toolbox modifier flags to our own modifier flags */
static short			FormModifiers(short Modifiers)
	{
		short						Value;

		Value = 0;
		if ((Modifiers & shiftKey) != 0)
			{
				Value |= eShiftKey;
			}
		if ((Modifiers & controlKey) != 0)
			{
				Value |= eControlKey;
			}
		if ((Modifiers & cmdKey) != 0)
			{
				Value |= eCommandKey;
			}
		if ((Modifiers & optionKey) != 0)
			{
				Value |= eOptionKey;
			}
		if ((Modifiers & alphaLock) != 0)
			{
				Value |= eCapsLockKey;
			}
		if ((Modifiers & btnState) != 0)
			{
				Value |= eMouseDownFlag;
			}
		return Value;
	}


/* Fetch an event from the event queue and return it.  Only some of the parameters */
/* returned may be valid; see the enumeration comments above for EventType to see */
/* which.  Mouse coordinates are always local to the current window, or undefined */
/* if there is no current window.  If there are no events, the routine will return */
/* after some amount of time.  This routine may call the Menu manager, so menus */
/* should be initialized before this routine is called.  Any parameter may be passed */
/* as NIL if the user doesn't care about the result.  Window changes do not occur */
/* if the mouse is down.  If the current window is a dialog box, then a window */
/* change will never be returned for another window.  Mouse up events are always */
/* returned with the same window as the mosue down event, even if the mouse is no */
/* longer in the window. */
EventType					GetAnEvent(OrdType* Xloc, OrdType* Yloc, ModifierFlags* Modifiers,
										WinType** Window, MenuItemType** MenuCommand, char* KeyPressed)
	{
		/* the event.  statically allocated so that it is still valid when called */
		/* again.  this is used for remembering the event that triggered a menu thing. */
		static EventRecord		MyEvent;
		/* state variable for menu handling */
		static MenuStates			MenuState = eNoMenu;
		/* when was the last menu selected (for slowing down menu bar flash) */
		static unsigned long	WhenMenuWasSelected;

		WindowPtr							WhichWindow;
		long									MenuCommandInteger;


		/* redraw the menu bar if it has changed */
		Eep_RedrawMenuBar();

		/* the cancel pending flag for RelinquishCPU is sticky, so that if the user */
		/* cancels once, it continues to return True.  It gets cleared the next time */
		/* the program tries to handle a "real" event. */
		CancelPending = False;

		/* redraw any windows that couldn't be redrawn when the event was received. */
		PerformDeferredUpdates();

		/* main loop */
	 LoopPoint:

		/* check to see if active window has changed */
		if (LastActiveWindow != FrontWindow())
			{
				LastActiveWindow = FrontWindow();
				if (Window != NIL)
					{
						*Window = GetCurrentWindow();
					}
				return eActiveWindowChanged;
			}

		/* if there are any queued up keypresses from RelinquishCPU, report them now */
		if ((KeyboardEventBuffer != NIL) && (PtrSize((char*)KeyboardEventBuffer) > 0))
			{
				EventRecord*	Temp;
				long					OldSize;

				CheckPtrExistence(KeyboardEventBuffer);
				/* a keypress was buffered during a RelinquishCPU call */
				MyEvent = KeyboardEventBuffer[0]; /* save the event */
				OldSize = PtrSize((char*)KeyboardEventBuffer);
				PRNGCHK(KeyboardEventBuffer,&(KeyboardEventBuffer[1]),
					OldSize - sizeof(EventRecord));
				PRNGCHK(KeyboardEventBuffer,&(KeyboardEventBuffer[0]),
					OldSize - sizeof(EventRecord));
				MoveData((char*)&(KeyboardEventBuffer[1]),(char*)&(KeyboardEventBuffer[0]),
					OldSize - sizeof(EventRecord));
				Temp = (EventRecord*)ResizePtr((char*)KeyboardEventBuffer,
					OldSize - sizeof(EventRecord));
				if (Temp == NIL)
					{
						/* if we run out of memory, then we just lose all the buffered keypresses */
						ReleasePtr((char*)KeyboardEventBuffer);
						KeyboardEventBuffer = (EventRecord*)AllocPtrCanFail(0,"KeyboardEventBuffer");
					}
				 else
					{
						KeyboardEventBuffer = Temp;
					}
				/* now, jump so that we fake the event */
				goto HandleEventSwitchPoint;
			}

		/* handle any menu operations that are in progress. */
		switch (MenuState)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetAnEvent:  bad MenuState"));
					break;
				case eNoMenu:
					break; /* continue on through */
				case eMenuPendingMouse:
					/* let system process event */
					/* MyEvent is still valid from last time through */
					MenuCommandInteger = MenuSelect(MyEvent.where);
					/* if the menu was actually chosen, then return it */
					if (MenuCommand != NIL)
						{
							*MenuCommand = Eep_MMID2ItemID(MenuCommandInteger);
							if (*MenuCommand != NIL)
								{
									/* we only report a menu choice if the user actually chose something */
									MenuState = eMenuSelected;
									WhenMenuWasSelected = TickCount();
									if (Modifiers != NIL)
										{
											*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
										}
									if (Window != NIL)
										{
											*Window = GetCurrentWindow();
										}
									MenuState = eMenuSelected;
									return eMenuCommand;
								}
						}
					/* if menu wasn't chosen, then reset menu state & get another event */
					MenuState = eNoMenu;
					goto LoopPoint;
				case eMenuPendingKey:
					/* convert the key to a menu thing */
					/* MyEvent is still valid from last time through */
					MenuCommandInteger = MenuKey(MyEvent.message & charCodeMask);
					/* convert the menu command into something we can handle */
					if (MenuCommand != NIL)
						{
							/* only do menus if they can handle them */
							*MenuCommand = Eep_MMID2ItemID(MenuCommandInteger);
							/* if the menu actually happened, then report it */
							if (*MenuCommand != NIL)
								{
									WhenMenuWasSelected = TickCount();
									if (Modifiers != NIL)
										{
											*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
										}
									if (Window != NIL)
										{
											*Window = GetCurrentWindow();
										}
									MenuState = eMenuSelected;
									return eMenuCommand;
								}
						}
					/* if the key doesn't correspond to a menu item, then schlep on over */
					/* to the normal keypress handler */
					MenuState = eNoMenu;
					goto FinishKeypressEvent;
					break;
				case eMenuSelected:
					/* make a small delay so that the menu is actually visible */
					while (TickCount() - WhenMenuWasSelected < MENUDELAY)
						{
							/* hideous delay loop to flash menu so user can see it. */
						}
					HiliteMenu(0);
					MenuState = eNoMenu;
					break;
			}

		/* call the event routine.  the ugly parameter decides whether a mouse is */
		/* down and uses that to speed up mouse tracking */
		WaitNextEvent(everyEvent,&MyEvent,((LastMouseDownInThisWindow != NIL)
			&& (SleepTime > VERYSMALLTIMEINTERVAL)) ? VERYSMALLTIMEINTERVAL : SleepTime,NIL);

		/* remember last event time for RelinquishCPU */
		LastEventTime = TickCount();

		/* decode the event */
	 HandleEventSwitchPoint:
		switch (MyEvent.what)
			{
				case nullEvent:
					if (TickCount() - LastCursorCheck >= CURSORUPDATEDELAY)
						{
							if (FrontWindow() != NIL)
								{
									LastCursorCheck = TickCount();
									SetPort(FrontWindow());
									GlobalToLocal(&MyEvent.where);
									if (Xloc != NIL)
										{
											*Xloc = MyEvent.where.h;
										}
									if (Yloc != NIL)
										{
											*Yloc = MyEvent.where.v;
										}
									if (Modifiers != NIL)
										{
											*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
										}
									if (Window != NIL)
										{
											*Window = GetCurrentWindow();
										}
									return eCheckCursor;
								}
							 else
								{
									SetCursor(&qd.arrow);
								}
						}
					if (FrontWindow() != NIL)
						{
							SetPort(FrontWindow());
							GlobalToLocal(&MyEvent.where);
							if (Xloc != NIL)
								{
									*Xloc = MyEvent.where.h;
								}
							if (Yloc != NIL)
								{
									*Yloc = MyEvent.where.v;
								}
						}
					if (Modifiers != NIL)
						{
							*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
						}
					if (Window != NIL)
						{
							*Window = GetCurrentWindow();
						}
					return eNoEvent;

				case mouseDown:
					switch (FindWindow(MyEvent.where,&WhichWindow))
						{
							case inSysWindow:
								EXECUTE(PRERR(AllowResume,"GetAnEvent:  FindWindow returned inSysWindow"));
								/* SystemClick(&MyEvent,WhichWindow); */
								goto LoopPoint;
							case inMenuBar:
								MenuState = eMenuPendingMouse;
								WipeMenusClean();
								if (Modifiers != NIL)
									{
										*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
									}
								if (Window != NIL)
									{
										*Window = GetCurrentWindow();
									}
								return eMenuStarting;
							case inDrag:
								if ((FrontWindow() != NIL) /* make sure there's a front window */
									&& (
										/* front window must be a document window */
										(GetWindowKind((WinType*)GetWRefCon(FrontWindow())) == eDocumentWindow)
										/* or front window must be the window in question */
										|| (FrontWindow() == WhichWindow)
										/* or command key must be down to prevent window switch */
										|| ((MyEvent.modifiers & cmdKey) != 0)))
									{
										Rect					BoundsRect;
										RgnHandle			BoundsRegion;

										BoundsRegion = GetGrayRgn();
										BoundsRect = (**BoundsRegion).rgnBBox;
										InsetRect(&BoundsRect,4,4);
										DragWindow(WhichWindow,MyEvent.where,&BoundsRect);
									}
								 else
									{
										ErrorBeep();
									}
								goto LoopPoint;
							case inContent:
							case inGrow:
								if (FrontWindow() != WhichWindow)
									{
										if ((FrontWindow() == NIL) || (GetWindowKind(
											(WinType*)GetWRefCon(FrontWindow())) == eDocumentWindow))
											{
												SelectWindow(WhichWindow);
											}
										 else
											{
												ErrorBeep();
											}
										goto LoopPoint;
									}
								LastMouseDownInThisWindow = WhichWindow;
								SetPort(WhichWindow);
								GlobalToLocal(&MyEvent.where);
								if (Modifiers != NIL)
									{
										*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
									}
								if (Xloc != NIL)
									{
										*Xloc = MyEvent.where.h;
									}
								if (Yloc != NIL)
									{
										*Yloc = MyEvent.where.v;
									}
								if (Window != NIL)
									{
										*Window = (WinType*)GetWRefCon(WhichWindow);
									}
								return eMouseDown;
							case inGoAway:
								if (TrackGoAway(WhichWindow,MyEvent.where))
									{
										if (Window != NIL)
											{
												*Window = (WinType*)GetWRefCon(WhichWindow);
											}
										return eWindowClosing;
									}
								goto LoopPoint;
							case inZoomIn:
								SetPort(WhichWindow);
								if (TrackBox(WhichWindow,MyEvent.where,inZoomIn))
									{
										ZoomWindow(WhichWindow,inZoomIn,False);
										if (Window != NIL)
											{
												*Window = (WinType*)GetWRefCon(WhichWindow);
											}
										return eWindowResized;
									}
								goto LoopPoint;
							case inZoomOut:
								SetPort(WhichWindow);
								if (TrackBox(WhichWindow,MyEvent.where,inZoomOut))
									{
										ZoomWindow(WhichWindow,inZoomOut,False);
										if (Window != NIL)
											{
												*Window = (WinType*)GetWRefCon(WhichWindow);
											}
										return eWindowResized;
									}
								goto LoopPoint;
							default:
								goto LoopPoint;
						}
					break;

				case mouseUp:
					if (LastMouseDownInThisWindow == NIL)
						{
							/* orphaned mouse-up, probably munched during RelinquishCPU */
							goto LoopPoint;
						}
					CheckPtrExistence((WinType*)GetWRefCon(LastMouseDownInThisWindow));
					SetPort(LastMouseDownInThisWindow);
					GlobalToLocal(&MyEvent.where);
					if (Modifiers != NIL)
						{
							*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
						}
					if (Xloc != NIL)
						{
							*Xloc = MyEvent.where.h;
						}
					if (Yloc != NIL)
						{
							*Yloc = MyEvent.where.v;
						}
					if (Window != NIL)
						{
							*Window = (WinType*)GetWRefCon(LastMouseDownInThisWindow);
						}
					LastMouseDownInThisWindow = NIL;
					return eMouseUp;

				case keyDown:
				case autoKey:
#if DEBUG
					if ((MyEvent.modifiers & cmdKey) && (MyEvent.modifiers & shiftKey)
						&& ((MyEvent.message & charCodeMask) == 'h'))
						{
							CheckFragmentation();
							goto LoopPoint;
						}
#endif
					if ((MyEvent.modifiers & cmdKey) != 0)
						{
							if ((MyEvent.message & charCodeMask) == '.')
								{
									if (KeyPressed != NIL)
										{
											*KeyPressed = eCancelKey;
										}
									goto KeypressCancelSkipCode;
								}
							MenuState = eMenuPendingKey;
							WipeMenusClean();
							if (Modifiers != NIL)
								{
									*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
								}
							if (Window != NIL)
								{
									*Window = GetCurrentWindow();
								}
							return eMenuStarting;
						}
					/* the menu keypress handler jumps here to handle commanded keys */
				 FinishKeypressEvent:
					if (KeyPressed != NIL)
						{
							*KeyPressed = MyEvent.message & charCodeMask;
						}
					/* jump here if *KeyPressed is eCancelKey and you don't want */
					/* to clobber it by storing something else in it */
				 KeypressCancelSkipCode:
					if (FrontWindow() != NIL)
						{
							SetPort(FrontWindow());
							GlobalToLocal(&MyEvent.where);
							if (Xloc != NIL)
								{
									*Xloc = MyEvent.where.h;
								}
							if (Yloc != NIL)
								{
									*Yloc = MyEvent.where.v;
								}
						}
					if (Modifiers != NIL)
						{
							*Modifiers = (ModifierFlags)FormModifiers(MyEvent.modifiers);
						}
					if (Window != NIL)
						{
							*Window = GetCurrentWindow();
						}
					return eKeyPressed;

				case keyUp:
					goto LoopPoint;

				case updateEvt:
					BeginUpdate((WindowPtr)MyEvent.message);
					CallWindowUpdate((WinType*)GetWRefCon((WindowPtr)MyEvent.message));
					EndUpdate((WindowPtr)MyEvent.message);
					goto LoopPoint;

				case activateEvt:
					/* we'll catch this when we test FrontWindow() */
					goto LoopPoint;

				case osEvt:
					switch ((unsigned char)((MyEvent.message >> 24) & 0x000000ff))
						{
							case suspendResumeMessage:
								if (!(MyEvent.message & resumeFlag))
									{
										/* suspend */
										JudiciousInterval = JUDICIOUSDELAYBACKGROUND;
										RunningInForeground = False;
										if (Window != NIL)
											{
												*Window = 0;
											}
									}
								 else
									{
										/* resume */
										JudiciousInterval = JUDICIOUSDELAYFOREGROUND;
										RunningInForeground = True;
										if (Window != NIL)
											{
												*Window = GetCurrentWindow();
											}
									}
								return eActiveWindowChanged;
							case mouseMovedMessage:
								break;
							default:
								break;
						}
					goto LoopPoint;

				case kHighLevelEvent:
					AEProcessAppleEvent(&MyEvent);
					goto LoopPoint;

				default:
					goto LoopPoint;
			}
	}


/* set the amount of time to wait for an event.  The default is 1/4 of a second */
/* time units are in seconds (can be fractional since they are floating point values) */
double				SetEventSleepTime(double TheSleepTime)
	{
		unsigned long			OldSleepTime;

		OldSleepTime = SleepTime;
		SleepTime = (TheSleepTime * 60) + 0.5;
		return (double)OldSleepTime / 60 + 0.5;
	}


/* used by RelinquishCPUCheckCancel.  RelinquishCPUCheckCancel continually */
/* resets it's value to VERYSMALLTIMEINVERVAL, but if it is changed, then */
/* the new delay will take effect for one call of RelinquishCPUCheckCancel. */
/* RelinquishCPUJudiciouslyCheckCancel uses this to get the processor back */
/* as soon as possible. */
static long					RelinqCPUDelay = RELINQUISHCPUNORMALDELAY;

/* relinquishes CPU for 1 tick and checks to see if the user hit escape or cmd-. */
/* (or perhaps other cancel signals) and returns True if the user is trying to */
/* cancel.  Through the use of the global variable CancelPending, cancels are */
/* "sticky" so that subsequent calls will return True as well, until GetAnEvent is */
/* called. */
MyBoolean			RelinquishCPUCheckCancel(void)
	{
		EventRecord			StupidEvent;
		WindowPtr				WhichWindow;

		if (LastMouseDownInThisWindow != NIL)
			{
				/* since we munch mouse events, we don't want to do this during a mouse */
				/* down since we'll probably lose the mouse up.  It's rather silly for the */
				/* user to hit cancel while holding the mouse down anyway... */
				return CancelPending;
			}
	 TryAgainPoint:
		WaitNextEvent(keyDownMask | keyUpMask | mDownMask | mUpMask | updateMask
			| osMask | autoKeyMask | diskMask,&StupidEvent,RelinqCPUDelay,NIL);
		LastEventTime = TickCount();
		switch (StupidEvent.what)
			{
				case updateEvt:
					BeginUpdate((WindowPtr)StupidEvent.message);
					MarkForDeferredUpdate((WinType*)GetWRefCon((WindowPtr)StupidEvent.message));
					EndUpdate((WindowPtr)StupidEvent.message);
					goto TryAgainPoint;
				case mouseDown:
					switch (FindWindow(StupidEvent.where,&WhichWindow))
						{
							default:
								break;
							case inSysWindow:
								EXECUTE(PRERR(AllowResume,"GetAnEvent:  FindWindow returned inSysWindow"));
								/* SystemClick(&StupidEvent,WhichWindow); */
								break;
							case inDrag:
								if ((FrontWindow() == WhichWindow) || ((FrontWindow() != NIL)
									&& ((GetWindowKind((WinType*)GetWRefCon(FrontWindow()))
									== eDocumentWindow) || (GetWindowKind((WinType*)GetWRefCon(
									FrontWindow())) == eModelessDialogWindow))
									&& ((StupidEvent.modifiers & cmdKey) != 0)))
									{
										Rect					BoundsRect;
										RgnHandle			BoundsRegion;

										BoundsRegion = GetGrayRgn();
										BoundsRect = (**BoundsRegion).rgnBBox;
										InsetRect(&BoundsRect,4,4);
										DragWindow(WhichWindow,StupidEvent.where,&BoundsRect);
									}
								 else
									{
										ErrorBeep();
									}
								break;
						}
					break;
				case keyDown:
					if ((((StupidEvent.message & charCodeMask) == '.')
						&& ((StupidEvent.modifiers & cmdKey) != 0)) ||
						((StupidEvent.message & charCodeMask) == 27))
						{
							CancelPending = True; /* stick */
						}
					else if (KeyboardEventBuffer != NIL)
						{
							EventRecord*		Temp;
							long						OldSize;

							CheckPtrExistence(KeyboardEventBuffer);
							/* maybe the keypress was wanted, so we should save it */
							OldSize = PtrSize((char*)KeyboardEventBuffer);
							Temp = (EventRecord*)ResizePtr((char*)KeyboardEventBuffer,
								OldSize + sizeof(EventRecord));
							if (Temp != NIL)
								{
									Temp[OldSize / sizeof(EventRecord)] = StupidEvent;
									KeyboardEventBuffer = Temp;
								}
						}
					break;
				default:
					break;
			}
		RelinqCPUDelay = RELINQUISHCPUNORMALDELAY;
		return CancelPending; /* sticky */
	}


/* similar to RelinquishCPUCheckCancel but in cooperative multitasking systems, */
/* it gives much better performance for the application by not releasing the */
/* processor nearly as often */
MyBoolean			RelinquishCPUJudiciouslyCheckCancel(void)
	{
		if (TickCount() - LastEventTime < JudiciousInterval)
			{
				return CancelPending; /* sticky */
			}
		RelinqCPUDelay = 0; /* come back right away */
		if (IsThisAPowerBook)
			{
				/* if some CPU intensive task is going on, then keep the processor */
				/* from getting sleepy */
				IdleUpdate();
			}
		return RelinquishCPUCheckCancel();
	}


/* read the system timer (in seconds).  The timer returns real time (not process */
/* time) but not relative to any known time.  The value may roll over from an */
/* undefined large number to 0. */
double				ReadTimer(void)
	{
		return ((double)TickCount()) / 60;
	}


/* find the true difference between two timer values even if one has rolled over */
double				TimerDifference(double Now, double Then)
	{
		return (double)((unsigned long)(60 * Now + 0.5)
			- (unsigned long)(60 * Then + 0.5)) / 60;
	}


/* get the current mouse position.  If there is no current window, the */
/* results are undefined.  Either of the parameters can be NIL if the user */
/* doesn't care about the result */
void					GetMousePosition(OrdType* Xloc, OrdType* Yloc)
	{
		Point			MouseLoc;

		if (FrontWindow() != NIL)
			{
				SetPort(FrontWindow());
				GetMouse(&MouseLoc);
				if (Xloc != NIL)
					{
						*Xloc = MouseLoc.h;
					}
				if (Yloc != NIL)
					{
						*Yloc = MouseLoc.v;
					}
			}
	}


/* read the state of the modifier keys on the keyboard.  On systems that don't */
/* allow this, the function may return the modifiers as they were at the last */
/* known time */
ModifierFlags	CheckModifiers(void)
	{
		EventRecord			StupidEvent;

		WaitNextEvent(0,&StupidEvent,0,NIL);
		return (ModifierFlags)FormModifiers(StupidEvent.modifiers);
	}


/* set an implementation defined version of the specified cursor */
void					SetArrowCursor(void)
	{
		SetCursor(&qd.arrow);
	}


/* set an implementation defined version of the specified cursor */
void					SetIBeamCursor(void)
	{
		CursHandle		DaCursor;

		DaCursor = GetCursor(iBeamCursor);
		HLock((Handle)DaCursor);
		SetCursor(*DaCursor);
		HUnlock((Handle)DaCursor);
	}


/* set an implementation defined version of the specified cursor */
void					SetWatchCursor(void)
	{
		CursHandle		DaCursor;

		DaCursor = GetCursor(watchCursor);
		HLock((Handle)DaCursor);
		SetCursor(*DaCursor);
		HUnlock((Handle)DaCursor);
	}


/* set an implementation defined version of the specified cursor */
void					SetCrossHairCursor(void)
	{
		CursHandle		DaCursor;

		DaCursor = GetCursor(crossCursor);
		HLock((Handle)DaCursor);
		SetCursor(*DaCursor);
		HUnlock((Handle)DaCursor);
	}


/* set the cursor tho the image and mask specified.  If the implementation's cursor */
/* is larger than 16x16, then the adjustment is implementation defined.  On the */
/* Macintosh, cursors are 16x16, so no adjustment is necessary */
/* the most significant bit of the word is leftmost */
void							SetTheCursor(short HotPointX, short HotPointY,
										unsigned short CursorImage[16], unsigned short CursorMask[16])
	{
		Cursor					TheCurs;
		int							Scan;

		ERROR((HotPointX < 0) || (HotPointX >= 16) || (HotPointY < 0) || (HotPointY >= 16),
			PRERR(AllowResume,"SetTheCursor:  hot point out of range"));
		for (Scan = 0; Scan < 16; Scan += 1)
			{
				TheCurs.data[Scan] = CursorImage[Scan];
				TheCurs.mask[Scan] = CursorMask[Scan];
			}
		TheCurs.hotSpot.h = HotPointX;
		TheCurs.hotSpot.v = HotPointY;
		SetCursor(&TheCurs);
	}


/* get the number of seconds to wait before toggling an insertion point */
double				GetCursorBlinkRate(void)
	{
		return ((double)GetCaretTime()) / 60;
	}


/* get the maximum time between clicks for which they are considered a double click */
double				GetDoubleClickInterval(void)
	{
		return ((double)GetDblTime()) / 60;
	}


/* emit a not too annoying beep to indicate an error occurred */
void					ErrorBeep(void)
	{
		if (MakeErrorBeeps)
			{
				SysBeep(BEEPDURATION);
			}
		 else
			{
				unsigned long			StartTime;

				FlashMenuBar(0);
				StartTime = TickCount();
				while (TickCount() - StartTime < MENUDELAY)
					{
						/* hideous delay loop to flash menu bar so user can see it. */
						RelinquishCPUCheckCancel();
					}
				FlashMenuBar(0);
			}
	}


/* enable or disable error beeping.  True enables it.  the old value is returned. */
MyBoolean					SetErrorBeepEnable(MyBoolean ShouldWeBeep)
	{
		MyBoolean				OldValue;

		OldValue = MakeErrorBeeps;
		MakeErrorBeeps = ShouldWeBeep;
		return OldValue;
	}


/* this routine is called when a window is dying, so any locally cached pointers */
/* to windows have to be discarded */
void					Eep_WindowDying(WinType* Window)
	{
		if ((WinType*)GetWRefCon(LastMouseDownInThisWindow) == Window)
			{
				LastMouseDownInThisWindow = NIL;
			}
		if ((WinType*)GetWRefCon(LastActiveWindow) == Window)
			{
				LastActiveWindow = NIL;
			}
	}
