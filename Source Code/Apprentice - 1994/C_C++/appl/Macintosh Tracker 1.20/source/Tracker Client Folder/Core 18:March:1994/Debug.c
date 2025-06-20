/* Debug.c */

#include "Debug.h"
#include "MiscInfo.h"
#include "Memory.h"

/* debugging function, use it to present message */
#define MAINWINDOWCOORDS  70,40,512-70,342-120
#define TEXTBOXCOORDS  8,24,512-70-70-8,342-120-40-8
#define RESUMEBUTTONCOORDS  8,342-120-40-8-20,96+8,342-120-40-8
#define QUITBUTTONCOORDS  120,342-120-40-8-20,96+128,342-120-40-8
/* if AbortFlag == True, then it is impossible to recover from error and */
/* the application must be forced to quit */
/* 'Message' is a C-String [null terminated] */
#define EmergencyCacheSize (4096)

static Handle		Emergency;

static void (*ErrorFunction)(void) = NIL; /* when system error occurs, call this */

#if __option(mc68020)
	#define CodeFor68020
#else
	#define CodeFor68000
#endif
#pragma options(!mc68020) /* this code works no matter what */


/* conditionally defined range checking procedures */
#ifdef DEBUG
void			HRNGCHK(void* TheHandle, void* EffectiveAddress, signed long AccessSize)
	{
		signed long			HSize;
		signed long			Difference;

		StackSizeTest();
		HSize = HandleSize((Handle)TheHandle);
		Difference = (char*)EffectiveAddress - *(Handle)TheHandle;
		if ((Difference < 0) || (Difference + AccessSize > HSize))
			{
				PRERR(ForceAbort,"Handle access out of range.");
			}
	}

void			PRNGCHK(void* ThePointer, void* EffectiveAddress, signed long AccessSize)
	{
		signed long			PSize;
		signed long			Difference;

		StackSizeTest();
		PSize = PtrSize((Ptr)ThePointer);
		Difference = (char*)EffectiveAddress - (Ptr)ThePointer;
		if ((Difference < 0) || (Difference + AccessSize > PSize))
			{
				PRERR(ForceAbort,"Pointer access out of range.");
			}
	}
#endif

pascal void		MyResumeProc(void)
	{
		if (ErrorFunction != NIL)
			{
				(*ErrorFunction)();
			}
		ExitToShell(); /* abort immediately */
	}


void		SetErrorFunction(void (*Erf)(void))
	{
		ErrorFunction = Erf;
	}


void		PRERR(short AbortFlag, void* Message)
	{
		WindowRecord	ErrorWindow;
		Rect					Bounds;
		Rect					TextRect;
		EventRecord		MyEvent;
		WindowPtr			WhichWindow;
		ControlHandle	ResumeButton;
		ControlHandle	QuitButton;
		ControlHandle	WhichOne;
		long					NumChars;
		short					CommandKeyDown;

		CheckHeap();
		StackSizeTest();
		if (Emergency != NIL)
			{
				DisposHandle(Emergency);
				Emergency = NIL;
			}
		APRINT(("+PRERR: '%t'",Message));
		#ifdef ALWAYSRESUME
			AbortFlag = AllowResume; /* while we're debugging, we can resume */
		#endif
		SetRect(&Bounds,MAINWINDOWCOORDS);
		NewWindow(&ErrorWindow,&Bounds,"\p",True,1,(void*)-1,False,0);
		SelectWindow((WindowPtr)&ErrorWindow);  /* force it to be on top */
		SetRect(&Bounds,RESUMEBUTTONCOORDS);
		ResumeButton = NewControl((WindowPtr)&ErrorWindow,&Bounds,
			"\pResume",True,0,0,0,pushButProc,0);
		SetRect(&Bounds,QUITBUTTONCOORDS);
		QuitButton = NewControl((WindowPtr)&ErrorWindow,&Bounds,
			"\pQuit",True,0,0,0,pushButProc,0);
		if (AbortFlag == ForceAbort)
			{
				HiliteControl(ResumeButton,255);  /* make it inactive */
			}
		FlushEvents(everyEvent,0);
		while (True)
			{
				GetNextEvent(everyEvent,&MyEvent);
				CommandKeyDown = ((MyEvent.modifiers & cmdKey) != 0);
				switch (MyEvent.what)
					{
						case mouseDown:
							GlobalToLocal(&MyEvent.where);
							if (FindControl(MyEvent.where,(WindowPtr)&ErrorWindow,&WhichOne) != 0)
								{
									if (WhichOne != NIL)
										{
											if (WhichOne == QuitButton)
												{
													if (TrackControl(QuitButton,MyEvent.where,NIL) != 0)
														{
															if (ErrorFunction != NIL)
																{
																	(*ErrorFunction)(); /* call error function */
																}
															APRINT(("-PRERR: abort"));
															ENDAUDIT();
															if (CommandKeyDown)
																{
																	goto SkipBrk1Point;
																}
															#ifdef THINKC_DEBUGGER
																#ifdef DEBUG
																	Debugger(); /* break point */
																#endif
															#endif
														 SkipBrk1Point:
															ExitToShell(); /* exit from the program */
														}
												}
											if (WhichOne == ResumeButton)
												{
													if (TrackControl(ResumeButton,MyEvent.where,NIL) != 0)
														{
															goto ExitPoint;
														}
												}
										}
								}
							break;
						case updateEvt:
							BeginUpdate((WindowPtr)&ErrorWindow);
							SetPort(&ErrorWindow.port);
							TextFont(0);  /* system font */
							TextFace(0);  /* normal */
							TextMode(srcOr);
							TextSize(12);  /* 12 point */
							SpaceExtra(0);  /* no extra space */
							MoveTo(8,16);
							DrawString("\pA Program Error Occurred:");
							SetRect(&TextRect,TEXTBOXCOORDS);
							/* finding length of message string */
							NumChars = 0;
							while ( ((char*)Message)[NumChars] != 0)
								{
									NumChars += 1;
								}
							TextBox(Message,NumChars,&TextRect,teJustLeft);
							DrawControls((WindowPtr)&ErrorWindow);
							EndUpdate((WindowPtr)&ErrorWindow);
							break;
						default:
							break;
					}
			}
	 ExitPoint:
		CloseWindow((WindowPtr)&ErrorWindow);
		APRINT(("-PRERR"));
		GetNextEvent(activMask,&MyEvent); /* clear activate event for underlying window */
		if (CommandKeyDown)
			{
				goto SkipBrk2Point;
			}
		#ifdef THINKC_DEBUGGER
			#ifdef DEBUG
				Debugger(); /* break point */
			#endif
		#endif
	 SkipBrk2Point:
		return;
	}

EXECUTE(static MyBoolean	Inited = False;)
void	InitPRERR(void)
	{
		ERROR(Inited,PRERR(ForceAbort,"InitPRERR called more than once."));
		Emergency = NewHandle(EmergencyCacheSize);
		EXECUTE(Inited = True;)
	}

#ifdef CodeFor68020
	#pragma options(mc68020) /* turn it back on if necessary */
#endif
