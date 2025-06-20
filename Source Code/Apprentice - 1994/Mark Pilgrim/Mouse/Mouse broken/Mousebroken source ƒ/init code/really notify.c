/**********************************************************************\

File:		really notify.c

Purpose:	This module handles notifying the user that an error has
			occurred (through the Notification Manager); also, displaying
			the proper icon during init loading.
			

Mousebroken -=- your computer isn't truly broken until it's mousebroken
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

#include "structs.h"
#include "really notify.h"
#include "show init.h"
#include "GestaltEQU.h"

#define		DEFAULT_MESSAGE		"\pMousebroken did not open because \
an error occurred during startup.  The INIT may be damaged, \
or there may not be enough memory in the system heap."

void StartupError(int errorcode)
{
	Str255			errorText;
	Str255			errorNumber;
	unsigned char	*defaultText = DEFAULT_MESSAGE;
	unsigned char	*textPtr;
	StringPtr		str;
	int				size;
	Boolean			gHasNotification;
	OSErr			isHuman;
	unsigned long	gestalt_temp;
	
	isHuman = Gestalt(gestaltNotificationMgrAttr, &gestalt_temp);
	gHasNotification=((!isHuman) &&
		(((gestalt_temp >> gestaltNotificationPresent) & 0x01) == 1));
	if (gHasNotification)
	{
		SetZone(ApplZone);
		GetIndString(errorText, STARTUP_ERROR_STR, errorcode);
		SetZone(SysZone);
		textPtr=(errorText[0]==0) ? defaultText : errorText;		
		size = textPtr[0] + 1;
		str = (StringPtr)NewPtrSys(size);
		if(str)
		{
			BlockMove(textPtr, str, size);
			
			if(SetupNM(str))
				DisposePtr(str);
		}
	}
	
	SetZone(ApplZone);
	ShowBadICON();
	SetZone(SysZone);
}

void StartupGood(PrefHandle cdevStorage)
{
	if ((**cdevStorage).showIcon)
	{
		SetZone(ApplZone);
		ShowGoodICON();
		SetZone(SysZone);
	}
}

int SetupNM(StringPtr str)
{
	NMRec			*note;
	Handle			nmResponse;
	OSErr			isHuman;
	long			gestaltReturn;
	
	if(str == (StringPtr)0L)
		return 1;
	
	note = (NMRec*)NewPtrSys(sizeof(NMRec));
	if(!note)
	{
		return 1;
	}
	else
	{
		note->qType = nmType;
		note->nmMark = 0;
		note->nmIcon = 0L;
		note->nmSound = 0L;
		note->nmStr = str;
		
		/* NMRP ID=RESPONSE_NMRP must be System Heap, Preload */
		nmResponse = GetResource('NMRP', RESPONSE_NMRP);
		if(!nmResponse)
		{
			DisposePtr(note);
			return 1;
		}
		else
		{
			HLock(nmResponse);
			HNoPurge(nmResponse);
			DetachResource(nmResponse);
			
			note->nmResp = (ProcPtr)*nmResponse;
			
			if(NMInstall(note) == noErr)
			{
				return 0;
			}
			else
			{
				DisposePtr(note);
				return 1;
			}
		}
	}
}
