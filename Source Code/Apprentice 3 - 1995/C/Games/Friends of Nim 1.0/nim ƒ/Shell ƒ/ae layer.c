#include "ae layer.h"
#include "generic open.h"
#include "environment.h"
#include "error.h"
#include "program init.h"

static OSErr GotRequiredParameters(const AppleEvent *theAppleEvent);
pascal OSErr HandleOAppEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleDocEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleQuitEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);

static	AEEventHandlerUPP	gHandleOAppUPP;
static	AEEventHandlerUPP	gHandleDocUPP;
static	AEEventHandlerUPP	gHandleQuitUPP;

static OSErr GotRequiredParameters(const AppleEvent *theAppleEvent)
{
	OSErr			myErr;
	DescType		returnedType;
	Size			actualSize;
	
	myErr=AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType,
				0L, 0, &actualSize);
	if (myErr==errAEDescNotFound)
		return noErr;
	else if (myErr==noErr)
		return errAEParamMissed;
	else
		return myErr;
}

pascal OSErr HandleOAppEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	
	theError=GotRequiredParameters(theEvent);
	return theError;
}

pascal OSErr HandleDocEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply)
	
	OSErr			theError;
	AEDescList		docList;
	long			itemsInList;
	long			index;
	AEKeyword		keyword;
	DescType		returnedType;
	FSSpec			theFileSpec;
	Size			actualSize;
	
	theError=AEGetParamDesc(theEvent, keyDirectObject, typeAEList,  &docList);
	if (theError==noErr)
	{
		theError=GotRequiredParameters(theEvent);
		if (theError==noErr)
		{
			theError=AECountItems(&docList, &itemsInList);
			if (theError==noErr)
			{
				for (index=1; index<=itemsInList; index++)
				{
					theError=AEGetNthPtr(&docList, index, typeFSS, &keyword, &returnedType,
								(Ptr)&theFileSpec, sizeof(theFileSpec), &actualSize);
					if (theError==noErr)
					{
						if (refCon==kAEOpenDocuments)
							OpenTheFile(&theFileSpec);
						else
							PrintTheFile(&theFileSpec);
					}
					else
					{
					}
				}
			}
		}
		
		AEDisposeDesc(&docList);
	}
	return theError;
}

pascal OSErr HandleQuitEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	
	theError=GotRequiredParameters(theEvent);
	if (theError==noErr)
	{
		gDone=ShutDownTheProgram();
		if (!gDone)
			theError=userCanceledErr;
	}
	return theError;
}

OSErr InstallRequiredAppleEvents(void)
{
	OSErr result;
	
	gHandleOAppUPP=NewAEEventHandlerProc(HandleOAppEvent);
	FailNilUPP((UniversalProcPtr)gHandleOAppUPP);
	gHandleDocUPP=NewAEEventHandlerProc(HandleDocEvent);
	FailNilUPP((UniversalProcPtr)gHandleDocUPP);
	gHandleQuitUPP=NewAEEventHandlerProc(HandleQuitEvent);
	
	result=AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
				gHandleOAppUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
			gHandleDocUPP, kAEOpenDocuments, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
			gHandleDocUPP, kAEPrintDocuments, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
			gHandleQuitUPP, 0, FALSE);
	
	return result;
}
