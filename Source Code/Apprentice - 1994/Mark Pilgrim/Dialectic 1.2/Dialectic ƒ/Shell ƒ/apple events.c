/**********************************************************************\

File:		apple events.c

Purpose:	This module handles the 4 required apple events: open
			application, open document, print document (not supported),
			and quit application.

\**********************************************************************/

#include "apple events.h"
#include "environment.h"
#include "dialectic generic open.h"

/*-----------------------------------------------------------------------------------*/
/* internal stuff for apple events.c                                                 */
/* (need to be declared here because SetUpAppleEvents() references them              */

pascal OSErr HandleOpenAppAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon);
pascal OSErr HandleOpenDocAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon);
pascal OSErr HandlePrintDocAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon);
pascal OSErr HandleQuitAE(const AppleEvent *theAppleEvent, const AppleEvent *reply,
	long refcon);
pascal OSErr MyGotRequiredParams(const AppleEvent *theAppleEvent);

void SetUpAppleEvents(void)
/* called at program initialization, AFTER checking if apple events exist at all.
   This is the magic linker that makes the procedures below get called when we
   say AEProcessAppleEvent(&theEvent) in response to a kHighLevelEvent (see main.c). */

{
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
									HandleOpenAppAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
									HandleOpenDocAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
									HandlePrintDocAE, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
									HandleQuitAE, 0, FALSE);
	AESetInteractionAllowed(kAEInteractWithAll);	/* don't know what this does, sorry */
}

/* Don't ask about the rest of this code.  It is a straight port from the pascal
   code given in IM Interapplication Communication; not even the variable names
   have been changed to protect the innocent.  I don't pretend to understand this,
   but it works well enough to (1) not crash, (2) open and close the application
   properly, and (3) get an FSSpec on open/print which we can then work on
   directly (see the generic open.c file). */

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
						GenericOpen(&myFSS);		/* in rc generic open.c */
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
		gDone=TRUE;	/* our global program-done flag, so we will exit our event loop */
					/* do NOT under any circumstances call ExitToShell() here; */
					/* be patient and wait to get back to the event loop, which */
					/* will exit the next time around now than gDone=TRUE */
	
	return isHuman;
}

pascal OSErr MyGotRequiredParams(const AppleEvent *theAppleEvent)
{
	DescType		returnedType;
	Size			actualSize;
	
	/* yeah, whatever */
	return (AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
			&returnedType, 0L, 0, &actualSize)==errAEDescNotFound) ? noErr :
			errAEParamMissed;
}
