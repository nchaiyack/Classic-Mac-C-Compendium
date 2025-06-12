/**********************************************************************\

File:		msg main.c

Purpose:	This module handles the event loop and event dispatching.


Menu Fixer -=- synchronize menu IDs and menu resource IDs
Copyright (C) 1993 Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "msg main.h"
#include "msg integrity.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg environment.h"
#include "fix.h"
#include "fix files.h"
#include "AppleEvents.h"
#include "EPPC.h"

void main(void)
{
	gDone = FALSE;	
	MaxApplZone();	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	GetDateTime(&randSeed);
	
	CheckEnvironment();	
	DoIntegrityCheck();	
	InitEnvironment();	
	InitProgram();
	EventLoop();	
	ShutDownEnvironment();
	ExitToShell();
}

void EventLoop(void)
{
	EventRecord		theEvent;
	
	while (!gDone)
	{
		SetCursor(&arrow);
		HiliteMenu(0);
		
		if (WaitNextEvent(everyEvent, &theEvent, 30, 0L))
			DispatchEvents(theEvent);
	}
}

void DispatchEvents(EventRecord gTheEvent)
{
	int				i;
	OSErr			isHuman;
	Point			thisPoint;
	
	switch (gTheEvent.what)
	{
		case mouseDown:
			HandleMouseDown(gTheEvent);
			break;
		case keyDown:
		case autoKey:
			if(gTheEvent.modifiers & cmdKey)
			{
				AdjustMenus();
				HandleMenu(MenuKey((char)(gTheEvent.message & charCodeMask)));
			}
			break;
		case diskEvt:
			if (HiWord(gTheEvent.message)!=noErr)
			{
				DILoad();
				SetPt(&thisPoint, 120, 120);
				isHuman=DIBadMount(thisPoint, gTheEvent.message);
				DIUnload();
			}
			break;
		case updateEvt:
			BeginUpdate((WindowPtr)gTheEvent.message);
			
			for (i=0; i<NUM_HELP; i++)
				if ((WindowPtr)gTheEvent.message == gHelp[i])
					UpdateHelp(i);
			
			EndUpdate((WindowPtr)gTheEvent.message);
			break;
		case kHighLevelEvent:
			if (gHasAppleEvents)
				AEProcessAppleEvent(&gTheEvent);
			break;
	}
}

void HandleMouseDown(EventRecord gTheEvent)
{
	WindowPtr	theWindow;
	int			windowCode;
	long		windSize;
	GrafPtr		oldPort;
	int			i;
	
	windowCode=FindWindow(gTheEvent.where, &theWindow);
	switch (windowCode)
	{
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(gTheEvent.where));
			break;
		case inContent:
			if(FrontWindow() != theWindow)
				SelectWindow(theWindow);
			break;
		case inSysWindow:
			SystemClick(&gTheEvent, theWindow);
			break;
		case inDrag:
			DragWindow(theWindow, gTheEvent.where, &gDragRect);
			break;
		case inGoAway:
			if (TrackGoAway(theWindow, gTheEvent.where))
			{
				for (i=0; i<NUM_HELP; i++)
					if (theWindow == gHelp[i])
						gHelp[i]=0L;
				
				DisposeWindow(theWindow);
				AdjustMenus();
			}
			break;
		case inGrow:
			gSizeRect = screenBits.bounds;
			OffsetRect(&gSizeRect, gSizeRect.left, gSizeRect.top);
			
			windSize = GrowWindow(theWindow, gTheEvent.where, &gSizeRect);
			if(windSize != 0)
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				EraseRect(&theWindow->portRect);
				SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			break;
		case inZoomIn:
		case inZoomOut:
			if(TrackBox(theWindow, gTheEvent.where, windowCode))
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				ZoomWindow(theWindow, windowCode, FALSE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			break;
	}
}

void ShutDownEnvironment(void)
{
}
