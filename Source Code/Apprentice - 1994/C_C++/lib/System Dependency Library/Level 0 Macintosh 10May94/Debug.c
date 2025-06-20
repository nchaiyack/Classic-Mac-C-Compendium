/* Debug.c */
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
#include "Definitions.h"
#include "Debug.h"
#include "Audit.h"

#pragma options(pack_enums)
#include <Quickdraw.h>
#include <Windows.h>
#include <Memory.h>
#include <Controls.h>
#include <OSEvents.h>
#include <SegLoad.h>
#include <TextEdit.h>
#pragma options(!pack_enums)


/* debugging function, use it to present message */
#define MAINWINDOWCOORDS  70,40,512-70,342-120
#define TEXTBOXCOORDS  8,24,512-70-70-8,342-120-40-8
#define RESUMEBUTTONCOORDS  8,342-120-40-8-20,96+8,342-120-40-8
#define QUITBUTTONCOORDS  120,342-120-40-8-20,96+128,342-120-40-8

/* if AbortFlag == True, then it is impossible to recover from error and */
/* the application must be forced to quit */
/* 'Message' is a C-String [null terminated] */
#define EmergencyCacheSize (8192)


EXECUTE(static MyBoolean	Inited = False;)

static Handle							Emergency = NIL;


/* turn off any weird options */
#pragma options(!profile)
#pragma options(!mc68020)


void							PRERR(int AbortFlag, char* Message)
	{
		WindowRecord		ErrorWindow;
		Rect						Bounds;
		Rect						TextRect;
		EventRecord			MyEvent;
		WindowPtr				WhichWindow;
		ControlHandle		ResumeButton;
		ControlHandle		QuitButton;
		ControlHandle		WhichOne;
		long						NumChars;
		short						CommandKeyDown;

		if (Emergency != NIL)
			{
				DisposHandle(Emergency);
				Emergency = NIL;
			}
		APRINT(("+PRERR: '%t'",Message));
		#if ALWAYSRESUME
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
															APRINT(("-PRERR: abort"));
															ENDAUDIT();
															if (CommandKeyDown)
																{
																	goto SkipBrk1Point;
																}
															#if DEBUGGER_PRESENT
																#if DEBUG
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
							DrawString("\pAn Internal Program Error Occurred:");
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
		Emergency = NewHandle(EmergencyCacheSize);
		#if DEBUGGER_PRESENT
			#if DEBUG
				Debugger(); /* break point */
			#endif
		#endif
	 SkipBrk2Point:
		return;
	}


void							Eep_InitPRERR(void)
	{
		ERROR(Inited,PRERR(ForceAbort,"InitPRERR called more than once."));
		Emergency = NewHandle(EmergencyCacheSize);
		EXECUTE(Inited = True;)
	}


void							Eep_ShutdownPRERR(void)
	{
		ERROR(!Inited,PRERR(ForceAbort,"ShutdownPRERR:  not initialized"));
		if (Emergency != NIL)
			{
				DisposHandle(Emergency);
				Emergency = NIL;
			}
	}
