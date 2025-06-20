/*
** File:		AEHandler.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright � 1995 Nikol Software
** All rights reserved.
*/



#include "AEHandler.h"
#include "Failure.h"
#include "MacWT.h"
#include "StringUtils.h"
#include "FileUtils.h"


#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct AEHandler{
	AEEventClass			theEventClass;
	AEEventID				theEventID;
	AEEventHandlerProcPtr	theHandler;
	AEEventHandlerUPP		theUPP;
};
typedef struct AEHandler AEHandler;
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif


static AEHandler keywordsToInstall[] = {
	{ kCoreEventClass,	kAEOpenApplication,	(AEEventHandlerProcPtr)OpenApplicationEvent,	nil },
	{ kCoreEventClass,	kAEOpenDocuments,	(AEEventHandlerProcPtr)OpenDocumentEvent,		nil },
	{ kCoreEventClass,	kAEQuitApplication,	(AEEventHandlerProcPtr)QuitApplicationEvent,	nil }
};

#define kNumKeywords (sizeof(keywordsToInstall) / sizeof(AEHandler))


static OSErr	GotRequiredParams(AppleEvent *);




/*****************************************************************************/




void	InitAppleEvents(void)
{
	OSErr	err;
	short	i;

	for (i = 0; i < kNumKeywords; ++i)
		{
		if (!keywordsToInstall[i].theUPP)
			keywordsToInstall[i].theUPP = NewAEEventHandlerProc(keywordsToInstall[i].theHandler);

		err = AEInstallEventHandler(
			keywordsToInstall[i].theEventClass,	/* What class to install.  */
			keywordsToInstall[i].theEventID,	/* Keywords to install.    */
			keywordsToInstall[i].theUPP,		/* The AppleEvent handler. */
			0L,									/* Unused refcon.		   */
			false								/* Only for our app.	   */
			);

		if (err)
			{
			Fail(err, __FILE__, __LINE__, TRUE);
			return;
			}
		}
	
}




/*****************************************************************************/




void DoAppleEvent(EventRecord *event)
{
	OSErr err;
	
	err = AEProcessAppleEvent(event);
	if ( err )
		Fail(err, __FILE__, __LINE__, FALSE);
}




/*****************************************************************************/




static OSErr GotRequiredParams(AppleEvent *theAE)
{
	OSErr		err;
	DescType	returnedType;
	Size		actualSize;

	err = AEGetAttributePtr(theAE, keyMissedKeywordAttr, typeWildCard, &returnedType, nil, 0, &actualSize);
	if ( err == errAEDescNotFound )
		return noErr;
	else if ( err == noErr )
		return errAEParamMissed;
}
	
	
	

/*****************************************************************************/




pascal OSErr OpenDocumentEvent(AppleEvent *theAE, AppleEvent *reply, long handlerRefcon)
{
	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err;
	long		itemsInList;
	Size		actualSize;
	AEKeyword	keyword;
	DescType	returnedType;
	
	err = AEGetParamDesc(theAE, keyDirectObject, typeAEList, &docList);
	if ( err == noErr )
		{
		err = GotRequiredParams(theAE);
		if ( err == noErr )
			{
			err = AECountItems(&docList, &itemsInList);
			if ( err == noErr )
				{
				err = AEGetNthPtr(&docList, 1, typeFSS, &keyword, &returnedType, (Ptr)&myFSS, sizeof(myFSS), &actualSize);
				if ( err == noErr )
					{
					PathNameFromDirID(myFSS.vRefNum, myFSS.parID, gWorldFile);
					pcat(gWorldFile, myFSS.name);
					}
				}
			}
		err = AEDisposeDesc(&docList);
		}
	return err;
}




/*****************************************************************************/




pascal OSErr OpenApplicationEvent(AppleEvent *theAE, AppleEvent *reply, long handlerRefcon)
{
	OSErr	err;

	err = GotRequiredParams(theAE);
	if (!err)
		{
		}

	return err;
}




/*****************************************************************************/




pascal OSErr QuitApplicationEvent(AppleEvent *theAE, AppleEvent *reply, long handlerRefcon)
{
	OSErr err;
	extern Boolean quitting;

	err = GotRequiredParams(theAE);
	if ( !err )
		quitting = TRUE;
	
	return err;
}
