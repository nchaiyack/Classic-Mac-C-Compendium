/**********************************************************************\

File:		msg shell main.c

Purpose:	This module handles the event loop and event dispatching.


MSG Prepare 1.0 -- minimal integrity check preparation program
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
#include "msg menus.h"
#include "MSG Prepare.h"

EventRecord		gTheEvent;

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
	CheckDropKick();
	if (gDone)
		ExitToShell();
	InitEnvironment();	
	EventLoop();	
	ExitToShell();
}

void InitEnvironment(void)
{
	Handle		MBARHandle;

	MBARHandle = GetNewMBar(400);
	SetMenuBar(MBARHandle);
	gAppleMenu = GetMHandle(appleMenu);
	gFileMenu = GetMHandle(fileMenu);
	gEditMenu = GetMHandle(editMenu);

	AddResMenu(gAppleMenu, 'DRVR');
	AdjustMenus();
}

void EventLoop(void)
{
	while(!gDone)
	{
		SetCursor(&arrow);
		HiliteMenu(0);
		if (WaitNextEvent(everyEvent, &gTheEvent, 30, 0L))
			DispatchEvents();
	}
}

void DispatchEvents(void)
{
	int			i;
	
	switch (gTheEvent.what)
	{
		case nullEvent:
			break;
		case mouseDown:
			HandleMouseDown();
			break;
		case keyDown:
		case autoKey:
			if(gTheEvent.modifiers & cmdKey)
			{
				AdjustMenus();
				HandleMenu(MenuKey((char)(gTheEvent.message & charCodeMask)));
			}
			break;
	}
}

void HandleMouseDown(void)
{
	WindowPtr	theWindow;
	int			windowCode = FindWindow(gTheEvent.where, &theWindow);
	
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
	}
}
