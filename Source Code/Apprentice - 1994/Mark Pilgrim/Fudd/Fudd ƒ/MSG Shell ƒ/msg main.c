/**********************************************************************\

File:		msg main.c

Purpose:	This module handles the event loop and event dispatching.


Fudd -=- convert text to Elmer Fudd talk
Copyright ©1994, Mark Pilgrim

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
#include "msg about.h"
#include "msg help.h"
#include "msg menus.h"
#include "msg prefs.h"
#include "msg environment.h"
#include "fudd.h"
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
	InitMSGGraphics();
	InitHelp();
	PrefsError(PreferencesInit());
	InitEnvironment();	
	InitProgram();
	EventLoop();	
	ShutDownEnvironment();
	ExitToShell();
}

void EventLoop(void)
{
	EventRecord		theEvent;
	int				i;
	
	while (!gDone)
	{
		SetCursor(&arrow);
		HiliteMenu(0);
		
		if (FrontWindow()!=0L)
		{
			for (i=0; i<NUM_WINDOWS; i++)
			{
				if (FrontWindow()==gTheWindow[i])
				{
					SetPort(gTheWindow[i]);
					i=NUM_WINDOWS;
				}
			}
		}
			
		if (WaitNextEvent(everyEvent, &theEvent, gIsInBackground ? 100 :
			(((gTheWindow[kAboutMSG]!=0L) && (FrontWindow()==gTheWindow[kAboutMSG])) ?
			0 : 30), 0L))
			DispatchEvents(theEvent);
		
		if ((!gIsInBackground) && (gTheWindow[kAboutMSG]!=0L) &&
			(FrontWindow()==gTheWindow[kAboutMSG]))
			DoTheMSGThing();
	}
}

void DispatchEvents(EventRecord theEvent)
{
	int				i;
	OSErr			isHuman;
	Point			thisPoint;
	int				index;
	char			thisChar;
	unsigned long	dummy;
	WindowPtr		theWindow;
	
	index=-1;
	if ((theWindow=FrontWindow())!=0L)
	{
		for (i=0; (i<NUM_WINDOWS) && (index==-1); i++)
			if (theWindow==gTheWindow[i])
				index=i;
	}
	
	switch (theEvent.what)
	{
		case nullEvent:
			break;
		case mouseDown:
			HandleMouseDown(theEvent);
			break;
		case keyDown:
		case autoKey:
			thisChar=(char)(theEvent.message & charCodeMask);
			if(theEvent.modifiers & cmdKey)
			{
				AdjustMenus();
				HandleMenu(MenuKey(thisChar));
			}
			else
				switch (index)
				{
					case kAbout:
						CloseTheWindow(index);
						break;
					case kAboutMSG:
						UpdateTheWindow(index);
						Delay(30, &dummy);
						CloseTheWindow(index);
						break;
					case kHelp:
						HelpKeyEvent(thisChar);
						break;
				}
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
			
			for (i=0; i<NUM_WINDOWS; i++)
				if ((WindowPtr)theEvent.message == gTheWindow[i])
					UpdateTheWindow(i);
					
			EndUpdate((WindowPtr)theEvent.message);
			break;
		case activateEvt:
			if (((WindowPtr)theEvent.message==gTheWindow[kAboutMSG]) &&
				((theEvent.modifiers&activeFlag)==0))
				UpdateTheWindow(kAboutMSG);
			break;
		case osEvt:
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				if ((gIsInBackground) && ((gTheWindow[kAboutMSG]!=0L) &&
					(FrontWindow()==gTheWindow[kAboutMSG])))
					UpdateTheWindow(kAboutMSG);
			}
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
	Boolean				gotone;
	int					index;
	unsigned long		dummy;
	
	windowCode=FindWindow(theEvent.where, &theWindow);
	index=-1;
	for (i=0; (i<NUM_WINDOWS) && (index==-1); i++)
		if (theWindow==gTheWindow[i])
			index=i;
	
	switch (windowCode)
	{
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent.where));
			break;
		case inContent:
			if(FrontWindow() != theWindow)
			{
				if (FrontWindow()==gTheWindow[kAboutMSG])
					UpdateTheWindow(kAboutMSG);
				SelectWindow(theWindow);
			}
			else
				switch (index)
				{
					case kAbout:
						CloseTheWindow(index);
						break;
					case kAboutMSG:
						UpdateTheWindow(index);
						Delay(30, &dummy);
						CloseTheWindow(index);
						break;
					case kHelp:
						HelpEvent();
						break;
				}
			break;
		case inSysWindow:
			SystemClick(&theEvent, theWindow);
			break;
		case inDrag:
			DragWindow(theWindow, theEvent.where, &gDragRect);
			gWindowBounds[index] = (*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			break;
		case inGoAway:
			if (TrackGoAway(theWindow, theEvent.where))
			{
				gotone=FALSE;
				for (i=0; (i<NUM_WINDOWS) && (!gotone); i++)
					gotone=(theWindow==gTheWindow[i]);
					
				if (gotone)
					CloseTheWindow(i-1);
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
			
			gWindowBounds[index] = (*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			
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
			
			gWindowBounds[index] = (*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			
			break;
	}
}

void ShutDownEnvironment(void)
{
	ShutDownProgram();
	ShutDownMSGGraphics();
	ShutDownHelp();
}
