/**********************************************************************\

File:		msg main.c

Purpose:	This module handles the event loop and event dispatching.


Ghost -=- a classic word-building challenge
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
#include "msg sounds.h"
#include "msg prefs.h"
#include "msg environment.h"
#include "ghost.h"
#include "AppleEvents.h"
#include "EPPC.h"

void main(void)
{
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
	InitSounds();
	PrefsError(PreferencesInit());
	InitEnvironment();	
	InitMSGGraphics();
	InitGame();
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
		
		if(gMainWindow)
			SetPort(gMainWindow);
		
		if (WaitNextEvent(everyEvent, &theEvent, 0, 0L))
			DispatchEvents(theEvent);
		else
			CheckForComputerPlayer();
	}
}

void DispatchEvents(EventRecord theEvent)
{
	int				i;
	OSErr			isHuman;
	Point			thisPoint;
	
	switch (theEvent.what)
	{
		case nullEvent:
			CheckForComputerPlayer();
			break;
		case mouseDown:
			HandleMouseDown(theEvent);
			break;
		case keyDown:
		case autoKey:
			if(theEvent.modifiers & cmdKey)
			{
				AdjustMenus();
				HandleMenu(MenuKey((char)(theEvent.message & charCodeMask)));
			}
			else if (gMainWindow)
				GameKeyEvent((char)(theEvent.message & charCodeMask));
			break;
		case diskEvt:
			if (HiWord(theEvent.message)!=noErr)
			{
				DILoad();
				SetPt(&thisPoint, 120, 120);
				isHuman=DIBadMount(thisPoint, theEvent.message);
				DIUnload();
			}
			break;
		case updateEvt:
			BeginUpdate((WindowPtr)theEvent.message);
			
			if((WindowPtr)theEvent.message == gMainWindow)
				UpdateBoard();
			
			for (i=0; i<NUM_HELP; i++)
				if ((WindowPtr)theEvent.message == gHelp[i])
					UpdateHelp(i);
			
			EndUpdate((WindowPtr)theEvent.message);
			break;
		case kHighLevelEvent:
			if (gHasAppleEvents)
				AEProcessAppleEvent(&theEvent);
			break;
	}
}

void HandleMouseDown(EventRecord theEvent)
{
	WindowPtr			theWindow;
	int					windowCode;
	long				windSize;
	GrafPtr				oldPort;
	int					i;
	Rect				sizeRect;
	
	windowCode=FindWindow(theEvent.where, &theWindow);
	switch (windowCode)
	{
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent.where));
			break;
		case inContent:
			if(FrontWindow() != theWindow)
				SelectWindow(theWindow);
			else
				if(theWindow == gMainWindow)
					GameEvent();
			break;
		case inSysWindow:
			SystemClick(&theEvent, theWindow);
			break;
		case inDrag:
			DragWindow(theWindow, theEvent.where, &gDragRect);
			if(theWindow == gMainWindow)
				gMainWindowBounds = (*(((WindowPeek)gMainWindow)->contRgn))->rgnBBox;
			break;
		case inGoAway:
			if (TrackGoAway(theWindow, theEvent.where))
			{
				for (i=0; i<NUM_HELP; i++)
					if (theWindow == gHelp[i])
						gHelp[i]=0L;
				
				if(theWindow == gMainWindow)
					CloseMainWindow();
				else
					DisposeWindow(theWindow);
				
				AdjustMenus();
			}
			break;
		case inGrow:
			sizeRect = screenBits.bounds;
			OffsetRect(&sizeRect, sizeRect.left, sizeRect.top);
			
			windSize = GrowWindow(theWindow, theEvent.where, &sizeRect);
			if(windSize != 0)
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				EraseRect(&theWindow->portRect);
				SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			
			if(theWindow == gMainWindow)
				gMainWindowBounds = (*(((WindowPeek)gMainWindow)->contRgn))->rgnBBox;
			
			break;
		case inZoomIn:
		case inZoomOut:
			if(TrackBox(theWindow, theEvent.where, windowCode))
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				ZoomWindow(theWindow, windowCode, FALSE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			
			if(theWindow == gMainWindow)
				gMainWindowBounds = (*(((WindowPeek)gMainWindow)->contRgn))->rgnBBox;
			
			break;
	}
}

void ShutDownEnvironment(void)
{
	ShutDownGame();
	ShutDownMSGGraphics();
	CloseSounds();	
}
