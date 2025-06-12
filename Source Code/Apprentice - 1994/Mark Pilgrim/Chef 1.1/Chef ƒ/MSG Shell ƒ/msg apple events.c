/**********************************************************************\

File:		msg apple events.c

Purpose:	This module handles the 4 required apple events: open
			application, open document, print document (not supported),
			and quit application.


Chef -=- convert text to Swedish chef talk
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

#include "msg apple events.h"
#include "msg environment.h"
#include "chef generic open.h"

void SetUpAppleEvents(void)
{
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
									HandleOpenAppAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
									HandleOpenDocAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
									HandlePrintDocAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
									HandleQuitAE, 0, FALSE);
	AESetInteractionAllowed(kAEInteractWithAll);
}

pascal OSErr HandleOpenAppAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon)
{
	return MyGotRequiredParams(theAppleEvent);
}

pascal OSErr HandleOpenDocAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon)
{
	OSErr			isHuman, dummy;
	FSSpec			myFSS;
	AEDescList		docList;
	long			index, itemsInList;
	Size			actualSize;
	AEKeyword		keywd;
	DescType		returnedType;
	
	isHuman=AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (isHuman==noErr)
	{
		if (MyGotRequiredParams(theAppleEvent)==noErr)
		{
			if (AECountItems(&docList, &itemsInList)==noErr)
			{
				for (index=1; index<=itemsInList; index++)
				{
					if (AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType,
						&myFSS, sizeof(myFSS), &actualSize)==noErr)
					{
						GenericOpen(&myFSS);
					}
					else
					{
						// handle error getting Nth pointer
					}
				}
			}
			else
			{
				// handle error counting items
			}
		}
		else
		{
			// handle error from MyGetRequiredParams
			dummy=AEDisposeDesc(&docList);
		}
	}
	else
	{
		// handle error getting direct parameter
	}
	
	return isHuman;
}

pascal OSErr HandlePrintDocAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon)
{
	return errAEEventNotHandled;
}

pascal OSErr HandleQuitAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon)
{
	OSErr			isHuman;
	
	isHuman=MyGotRequiredParams(theAppleEvent);
	if (isHuman==noErr)
		gDone = 1;
	
	return isHuman;
}

pascal OSErr MyGotRequiredParams(const AppleEvent *theAppleEvent)
{
	DescType		returnedType;
	Size			actualSize;
	
	return (AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
			&returnedType, 0L, 0, &actualSize)==errAEDescNotFound) ? noErr :
			errAEParamMissed;
}
