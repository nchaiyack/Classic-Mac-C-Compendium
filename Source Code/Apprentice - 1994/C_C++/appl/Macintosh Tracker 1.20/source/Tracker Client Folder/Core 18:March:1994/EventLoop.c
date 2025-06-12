/* EventLoop.c */

#define COMPILING_EVENTLOOP_C
#include "EventLoop.h"
#include "CMyApplication.h"
#include "CMyScrap.h"
#include "MenuController.h"
#include "CWindow.h"
#include "CViewRect.h"
#include "CSack.h"
#include "Compatibility.h"
#include "Memory.h"


#define RELINQUISHSLEEPTIME (1)
#define JudiciousDelay (5)

ulong				SleepTime = DefaultSleepTime;
CSack*			ListOfIdlers;
CSack*			ListOfIdlerWannabees;
Rect				BoundsRect;
ulong				LastEventTime = 0;
MyBoolean		QuitPending = False;
ulong				LastCursorTime;
CSack*			DeadIdlersList;
ulong				LastIdleTime = 0;
MyBoolean		WeAreActiveApplication = True;
MyBoolean		ModalWindowTerminationFlag = False;
ulong				LastJudiciousRelease = 0;

CViewRect*	LastMouseDownViewRect;
CViewRect*	KeyReceiverViewRect;
CViewRect*	LastKeyDownViewRect;

short				LastModifiers = 0;
ulong				LastModifiersTime = 0;


/* initialize data structures */
void		InitMyEventLoop(void)
	{
		BoundsRect = screenBits.bounds;
		InsetRect(&BoundsRect,4,4);
		ListOfIdlers = new CSack;
		ListOfIdlers->ISack(sizeof(CIdle*),128);
		ListOfIdlerWannabees = new CSack;
		ListOfIdlerWannabees->ISack(sizeof(CIdle*),4);
		DeadIdlersList = new CSack;
		DeadIdlersList->ISack(sizeof(CIdle*),4);
		ActiveWindow = NIL;
		KeyReceiverViewRect = NIL;
		LastCursorTime = TickCount();
	}


/* dispose any variables */
void		ShutDownMyEventLoop(void)
	{
		delete ListOfIdlers;
		delete ListOfIdlerWannabees;
		delete DeadIdlersList;
	}


void		TheEventLoop(CWindow* TheModalWindow)
	{
		EventRecord		MyEvent;
		WindowPtr			WhichWindow;
		MyEventRec		SpiffEvent;
		short					MenuCommand;
		ulong					MenuCommandStart;

	 Loop:
		if (QuitPending || ModalWindowTerminationFlag)
			{
				ModalWindowTerminationFlag = False;
				return;
			}
		ERROR((ActiveWindow==NIL)&&(KeyReceiverViewRect!=NIL),PRERR(ForceAbort,
			"There is no active window, yet there is a key receiver view rect."));
		APRINT((""));
//	if (((signed long)TickCount() - LastIdleTime > SleepTime)
//		&& ((signed long)TickCount() - LastIdleTime > (3*DefaultSleepTime/2)))
//		{
//			SendIdleToAll();
//			APRINT((""));
//		}
		WaitNextEvent(everyEvent,&MyEvent,SleepTime,NIL);
		if (MyEvent.what != nullEvent)
			{
				LastEventTime = MyEvent.when;
			}
		/* "Stress-testing" idea taken from ResEdit. */
		EXECUTE(PurgeMem(0x007fffff);) /* purge all blocks from zone */
		EXECUTE(CompactMem(0x007fffff);) /* compact whole zone */
		CheckHeap();
		SpiffEvent.Where.x = MyEvent.where.h;
		SpiffEvent.Where.y = MyEvent.where.v;
		SpiffEvent.When = MyEvent.when;
		SpiffEvent.Modifiers = MyEvent.modifiers;
		SpiffEvent.Message = MyEvent.message;
		LastModifiers = MyEvent.modifiers;
		LastModifiersTime = MyEvent.when;
		switch (MyEvent.what)
			{
				case nullEvent:
					SendIdleToAll();
					if (((signed long)TickCount() - LastCursorTime > DefaultSleepTime)
						&& WeAreActiveApplication)
						{
							LastCursorTime = TickCount();
							if ((FindWindow(MyEvent.where,&WhichWindow) != inContent)
								|| ((CWindow*)GetWRefCon(WhichWindow) != ActiveWindow)
								|| (!((CWindow*)GetWRefCon(WhichWindow))
								->DoMouseMoved(SpiffEvent)))
								{
									SetCursor(&arrow);
								}
						}
					break;
				case mouseDown:
					APRINT(("Mousedown"));
					switch (FindWindow(MyEvent.where,&WhichWindow))
						{
							case inSysWindow:
								SystemClick(&MyEvent,WhichWindow);
								break;
							case inMenuBar:
								MenuCommand = MenuMouseDown(&MyEvent);
								if (  (KeyReceiverViewRect == NIL) || ( (KeyReceiverViewRect != NIL)
									&& (!KeyReceiverViewRect->DoMenuCommand(MenuCommand)) )  )
									{
										if (ActiveWindow != NIL)
											{
												ActiveWindow->DoMenuCommand(MenuCommand);
											}
									}
								HiliteMenu(0);
								break;
							case inDrag:
								if ((TheModalWindow == NIL) || (TheModalWindow
									== (CWindow*)GetWRefCon(WhichWindow)))
									{
										((CWindow*)GetWRefCon(WhichWindow))->DoDrag(MyEvent.where);
									}
								 else
									{
										SysBeep(20);
									}
								break;
							case inContent:
							 InContentPoint:
								if ((TheModalWindow == NIL) || (TheModalWindow
									== (CWindow*)GetWRefCon(WhichWindow)))
									{
										((CWindow*)GetWRefCon(WhichWindow))->DoMouseDown(SpiffEvent);
									}
								 else
									{
										SysBeep(20);
									}
								break;
							case inGrow:
								if (((CWindow*)GetWRefCon(WhichWindow))->Growable)
									{
										((CWindow*)GetWRefCon(WhichWindow))->DoGrow(MyEvent.where);
									}
								 else
									{
										goto InContentPoint;
									}
								break;
							case inGoAway:
								((CWindow*)GetWRefCon(WhichWindow))->DoGoAway(MyEvent.where);
								break;
							case inZoomIn:
								((CWindow*)GetWRefCon(WhichWindow))->DoZoomIn(MyEvent.where);
								break;
							case inZoomOut:
								((CWindow*)GetWRefCon(WhichWindow))->DoZoomOut(MyEvent.where);
								break;
							default:
								break;
						}
					break;
				case mouseUp:
					APRINT(("MouseUp"));
					if (LastMouseDownViewRect != NIL)
						{
							LastMouseDownViewRect->DoMouseUp(SpiffEvent);
						}
					break;
				case keyDown:
					APRINT(("KeyDown"));
					if ((MyEvent.modifiers & cmdKey) != 0)
						{
							MenuCommand = MenuKeyDown(&MyEvent);
							if (MenuCommand == 0)
								{
									goto OtherKeyDown; /* if there is no menu */
								}
							MenuCommandStart = TickCount();
							if (  (KeyReceiverViewRect == NIL) || ( (KeyReceiverViewRect != NIL)
								&& (!KeyReceiverViewRect->DoMenuCommand(MenuCommand)) )  )
								{
									if (ActiveWindow != NIL)
										{
											ActiveWindow->DoMenuCommand(MenuCommand);
										}
								}
							while (TickCount() - MenuCommandStart < MENUMINFLASH)
								{
								}
							HiliteMenu(0);
						}
					 else
						{
							if (KeyReceiverViewRect != NIL)
								{
									LastKeyDownViewRect = KeyReceiverViewRect;
									if (!KeyReceiverViewRect->DoKeyDown(SpiffEvent))
										{
											goto OtherKeyDown;
										}
								}
							 else
								{
								 OtherKeyDown:
									if (ActiveWindow != NIL)
										{
											ActiveWindow->DoKeyDown(SpiffEvent);
										}
								}
						}
					break;
				case keyUp:
					APRINT(("KeyUp"));
					if (LastKeyDownViewRect != NIL)
						{
							LastKeyDownViewRect->DoKeyUp(SpiffEvent);
						}
					break;
				case autoKey:
					APRINT(("AutoKey"));
					if (LastKeyDownViewRect != NIL)
						{
							LastKeyDownViewRect->DoKeyDown(SpiffEvent);
						}
					break;
				case updateEvt:
					APRINT(("UpdateEvent"));
					((CWindow*)GetWRefCon((WindowPtr)MyEvent.message))->DoUpdate();
					break;
				case activateEvt:
					APRINT(("ActivateEvent"));
					if ((MyEvent.modifiers & activeFlag) != 0)
						{
							/* activate */
							/* ERROR(ActiveWindow!=NIL,PRERR(ForceAbort,
								"Activate event Received while a window was active.")); */
							if (ActiveWindow != NIL)
								{
									ActiveWindow->DoSuspend();
								}
							ActiveWindow = ((CWindow*)GetWRefCon((WindowPtr)MyEvent.message));
							ActiveWindow->DoResume();
						}
					 else
						{
							/* deactivate */
							/* ERROR(ActiveWindow==NIL,PRERR(ForceAbort,
								"Deactivate event Received while no window was active.")); */
							if (ActiveWindow != NIL)
								{
									ActiveWindow->DoSuspend();
									ActiveWindow = NIL;
								}
						}
					break;
				case osEvt:
					APRINT(("OSEvent"));
					switch ((MyEvent.message >> 24) & 0x000000ff)
						{
							case suspendResumeMessage:
								if (!(MyEvent.message & resumeFlag))
									{
										/* suspend */
										WeAreActiveApplication = False;
										if (ActiveWindow != NIL)
											{
												ActiveWindow->DoSuspend();
											}
										/* ActiveWindow remains valid, so that it can be reactivated later */
										if (MyEvent.message & convertClipboardFlag)
											{
												Scrap->ExportScrap();
											}
									}
								 else
									{
										/* resume */
										WeAreActiveApplication = True;
										ActiveWindow = (CWindow*)GetWRefCon(FrontWindow());
										if (ActiveWindow != NIL)
											{
												ActiveWindow->DoResume();
											}
										 else
											{
												ActiveWindow = Application;
												Application->DoResume();
											}
										if (MyEvent.message & convertClipboardFlag)
											{
												Scrap->ImportScrap();
											}
									}
								break;
							case mouseMovedMessage:
								break;
						}
					break;
				case kHighLevelEvent:
					if (TheModalWindow == NIL)
						{
							/* don't accept high level events when there's a modal window up */
							APRINT(("HighLevelEvent"));
							Application->ReceiveHighLevelEvent(&MyEvent);
						}
					break;
				default:
					APRINT(("UnknownEvent"));
					break;
			}
		goto Loop;
	}


/********************************************************************************/
/* Idle Tracking Stuff */


/* send an idle event to all idlers */
/* be very careful with this function since it can cause big time recursion */
/* if you call it from something that can be reached by the idle handler */
void		SendIdleToAll(void)
	{
		CIdle*		Item;
		long			TimeSinceLastEvent;
		CIdle*		DeadOne;
		EXECUTE(long					Sizer;)
		EXECUTE(static long		AuditLastIdleTime;)
		EXECUTE(long					TickTemp;)

		StackSizeTest();

		EXECUTE(TickTemp = TickCount();)
		APRINT(("%l between idles",TickTemp - AuditLastIdleTime));
		EXECUTE(AuditLastIdleTime = TickTemp;)

		TimeSinceLastEvent = TickCount() - LastEventTime;

		/* traversing list of idlers and calling their idle handlers */
		ListOfIdlers->ResetScan();
		while (ListOfIdlers->GetNext(&Item))
			{
				if (DeadIdlersList->NumElements() != 0)
					{
						DeadIdlersList->ResetScan();
						while (DeadIdlersList->GetNext(&DeadOne))
							{
								if (DeadOne == Item)
									{
										goto Escape; /* if idler died while in outer loop, can't call it */
									}
							}
					}
				CheckHandleExistence((Handle)Item);
				EXECUTE(Sizer = HandleSize((Handle)Item));
				ERROR(Sizer < 0,PRERR(ForceAbort,"CIdle object found to have negative size."));
				Item->DoIdle(TimeSinceLastEvent);
			 Escape:
				;
			}

		/* removing idlers from list.  Must be done first in case a new idler picked */
		/* up the same handle as a dead one. */
		while (DeadIdlersList->NumElements() != 0)
			{
				DeadIdlersList->ResetScan();
				DeadIdlersList->GetNext(&DeadOne);
				ListOfIdlers->KillElement(&DeadOne);
				DeadIdlersList->KillElement(&DeadOne);

				/* finding new lowest sleep time. */
				SleepTime = DefaultSleepTime;
				ListOfIdlers->ResetScan();
				while (ListOfIdlers->GetNext(&Item))
					{
						if (SleepTime > Item->MinSleepTime)
							{
								SleepTime = Item->MinSleepTime;
							}
					}
			}

		/* adding idlers to the list.  We have to shadow adds to the list in case */
		/* some things decide to add themselves as idlers as the previous loop */
		/* is in progress */
		while (ListOfIdlerWannabees->NumElements() != 0)
			{
				ListOfIdlerWannabees->ResetScan();
				ListOfIdlerWannabees->GetNext(&Item);
				ListOfIdlerWannabees->KillElement(&Item);
				ListOfIdlers->KillElement(&Item);
				if (SleepTime > Item->MinSleepTime)
					{
						SleepTime = Item->MinSleepTime;
					}
				ListOfIdlers->PushElement(&Item);
			}

		LastIdleTime = TickCount();

		/* at this point we have a problem:  say the idle loop was invoked and */
		/* one of the idle routines created a dialog box, so the idle loop gets */
		/* invoked again.  This means the internal pointer for ListOfIdlers */
		/* gets messed up, and may even be invalid when the routine returns to */
		/* the first idle loop.  To prevent that, we just resent the internal */
		/* pointer to the beginning of the list, making this routine reentrant. */
		ListOfIdlers->ResetScan();

		EXECUTE(TickTemp = TickCount();)
		APRINT(("%l in idle",TickTemp-AuditLastIdleTime));
		EXECUTE(AuditLastIdleTime = TickTemp;)
	}


/* add an object to the idle-time task list */
void		RegisterIdler(CIdle* TheIdler, long MinSleepTime)
	{
		TheIdler->MinSleepTime = MinSleepTime;
		ListOfIdlerWannabees->PushElement(&TheIdler);
		EXECUTE(TheIdler->Installed = True;)
	}


/* remove an object from the idle-time task list */
void		DeregisterIdler(CIdle* TheIdler)
	{
		DeadIdlersList->PushElement(&TheIdler);
		ListOfIdlerWannabees->KillElement(&TheIdler);
		EXECUTE(TheIdler->Installed = False;)
	}


/* the idle's function */
void		CIdle::DoIdle(long TimeSinceLastEvent)
	{
	}

#ifdef DEBUG
	
/* */		CIdle::CIdle()
	{
		Installed = False;
	}

/* */ 	CIdle::~CIdle()
	{
		ERROR(Installed,PRERR(ForceAbort,"Deleting an installed idler"));
	}

#endif


/********************************************************************************/


/* certain race conditions or crashes can occur when using this function. */
/* - do not call this from the Scrap object since it may cause the scrap */
/*   to be imported or exported */
/* - this calls update, suspend, and resume, so any window structures must */
/*   be completely initialized if you want to call this */
/* Finally, it returns True if the user cancelled and False if not. */
MyBoolean			RelinquishCPU(void)
	{
		EventRecord			StupidEvent;
		MyBoolean				CancelFlag;

		CancelFlag = False;
	 TryAgainPoint:
		WaitNextEvent(updateMask | keyDownMask | keyUpMask | mDownMask | mUpMask
			| osMask | autoKeyMask | diskMask,&StupidEvent,RELINQUISHSLEEPTIME,NIL);
		LastModifiers = StupidEvent.modifiers;
		LastModifiersTime = StupidEvent.when;
		switch (StupidEvent.what)
			{
				case keyDown:
					if ((((StupidEvent.message & charCodeMask) == '.')
						&& ((StupidEvent.modifiers & cmdKey) != 0)) ||
						(((StupidEvent.message & charCodeMask) == 3)
						&& ((StupidEvent.modifiers & controlKey) != 0)))
						{
							CancelFlag = True;
						}
					break;
				case updateEvt:
					APRINT(("UpdateEvent"));
					/* this helps "live" scrolling when part of the region is not */
					/* on the screen.  Since the scrollbars always call RelinquishCPU */
					/* frequently, this works well here. */
					((CWindow*)GetWRefCon((WindowPtr)StupidEvent.message))->DoUpdate();
					/* since the purpose of this routine is to give other processes */
					/* a chance to run, if we update, we don't do that so we go and */
					/* try another event. */
					goto TryAgainPoint;
				case osEvt:
					APRINT(("OSEvent"));
					switch ((StupidEvent.message >> 24) & 0x000000ff)
						{
							case suspendResumeMessage:
								if (!(StupidEvent.message & resumeFlag))
									{
										/* suspend */
										WeAreActiveApplication = False;
										if (ActiveWindow != NIL)
											{
												ActiveWindow->DoSuspend();
											}
										/* ActiveWindow remains valid, so that it can be reactivated later */
										if (StupidEvent.message & convertClipboardFlag)
											{
												Scrap->ExportScrap();
											}
									}
								 else
									{
										/* resume */
										WeAreActiveApplication = True;
										ActiveWindow = (CWindow*)GetWRefCon(FrontWindow());
										if (ActiveWindow != NIL)
											{
												ActiveWindow->DoResume();
											}
										 else
											{
												ActiveWindow = Application;
												Application->DoResume();
											}
										if (StupidEvent.message & convertClipboardFlag)
											{
												Scrap->ImportScrap();
											}
									}
								break;
							case mouseMovedMessage:
								break;
						}
					break;
				default:
					break;
			}
		LastJudiciousRelease = TickCount();
		return CancelFlag;
	}


/* see notes on RelinquishCPU about possible problems */
/* returns True if the user cancelled, and False if not */
MyBoolean 		RelinquishCPUJudiciously(void)
	{
		if (TickCount() - LastJudiciousRelease > JudiciousDelay)
			{
				return RelinquishCPU();
			}
		 else
			{
				return False;
			}
	}


/* be very careful with this function since it can cause big time recursion */
/* if you call it from something that can be reached by the idle handler */
void					SendIdleToAllJudiciously(void)
	{
		if (TickCount() - LastIdleTime > ((SleepTime > 2) ? SleepTime : 2))
			{
				SendIdleToAll();
			}
	}
