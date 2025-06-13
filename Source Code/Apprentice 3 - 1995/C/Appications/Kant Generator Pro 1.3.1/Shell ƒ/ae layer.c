#include "ae layer.h"
#include "generic open.h"
#include "environment.h"
#include "error.h"
#include "program init.h"
#include "kant event class dispatch.h"

static OSErr GotRequiredParameters(const AppleEvent *theAppleEvent);
pascal OSErr HandleOAppEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleDocEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleQuitEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPResolveEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPInsertEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPNewEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPUseModuleEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPGetTextEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPOptionsEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPCloseEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPSpeedEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPMusicEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPSaveEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPRememberEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon);
pascal OSErr HandleKGPRestoreEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPSpeakEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);
pascal OSErr HandleKGPSetVoiceEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon);

static	AEEventHandlerUPP	gHandleOAppUPP;
static	AEEventHandlerUPP	gHandleDocUPP;
static	AEEventHandlerUPP	gHandleQuitUPP;
static	AEEventHandlerUPP	gHandleKGPResolveUPP;
static	AEEventHandlerUPP	gHandleKGPInsertUPP;
static	AEEventHandlerUPP	gHandleKGPNewUPP;
static	AEEventHandlerUPP	gHandleKGPUseModuleUPP;
static	AEEventHandlerUPP	gHandleKGPGetTextUPP;
static	AEEventHandlerUPP	gHandleKGPOptionsUPP;
static	AEEventHandlerUPP	gHandleKGPCloseUPP;
static	AEEventHandlerUPP	gHandleKGPSpeedUPP;
static	AEEventHandlerUPP	gHandleKGPMusicUPP;
static	AEEventHandlerUPP	gHandleKGPSaveUPP;
static	AEEventHandlerUPP	gHandleKGPRememberUPP;
static	AEEventHandlerUPP	gHandleKGPRestoreUPP;
static	AEEventHandlerUPP	gHandleKGPSpeakUPP;
static	AEEventHandlerUPP	gHandleKGPSetVoiceUPP;

#define kKGPEventClass		'Kant'

enum /* KGP class event IDs */
{
	kKGPResolve			='Rslv',
	kKGPInsert			='Inst',
	kKGPNew				='New ',
	kKGPUseModule		='Modl',
	kKGPGetText			='GetT',
	kKGPOptions			='Optn',
	kKGPClose			='Clos',
	kKGPSpeed			='Sgs ',
	kKGPMusic			='Play',
	kKGPSave			='SDoc',
	kKGPRemember		='RemO',
	kKGPRestore			='ResO',
	kKGPSpeak			='Spek',
	kKGPSetVoice		='SetV'
};

#define kKGPSaveEnum		'savo'
#define kKGPSettingEnum		'Sett'
#define kKGPSpeedEnum		'Sped'
#define kKGPMusicEnum		'Mopt'

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

pascal OSErr HandleKGPResolveEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	
	theError=GotRequiredParameters(theEvent);
	if (theError==noErr)
	{
		KGPResolve();
	}
	return theError;
}

pascal OSErr HandleKGPInsertEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	AEDescList		docList;
	long			itemsInList;
	long			index;
	AEKeyword		keyword;
	DescType		returnedType;
	Size			actualSize;
	Str255			theStr;
	
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
					theError=AEGetNthPtr(&docList, index, typeChar, &keyword, &returnedType,
								(Ptr)&theStr[1], 255, &actualSize);
					if (theError==noErr)
					{
						theStr[0]=actualSize;
						KGPInsert(theStr);
					}
				}
			}
		}
		
		AEDisposeDesc(&docList);
	}
	return theError;
}

pascal OSErr HandleKGPNewEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	
	theError=GotRequiredParameters(theEvent);
	if (theError==noErr)
	{
		KGPNew();
	}
	return theError;
}

pascal OSErr HandleKGPUseModuleEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(refCon)
	
	OSErr			theError;
	Str255			theModuleStr;
	Size			actualSize;
	DescType		returnedType, moduleResult;
	
	if ((theError=AEGetParamPtr(theEvent, keyDirectObject, typeChar, &returnedType,
		&theModuleStr[1], 256, &actualSize))==noErr)
	{
		theModuleStr[0]=actualSize;
		theError=GotRequiredParameters(theEvent);
	}

	if (theError==noErr)
	{
		moduleResult=KGPUseModule(theModuleStr, TRUE);
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &moduleResult, sizeof(DescType));
	}
	
	return theError;
}

pascal OSErr HandleKGPGetTextEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(refCon)
	
	OSErr			theError;
	Ptr				theTextPtr;
	unsigned short	theTextLength;
	
	theError=GotRequiredParameters(theEvent);
	if (theError==noErr)
	{
		KGPGetText(&theTextPtr, &theTextLength);
		theError=AEPutParamPtr(reply, keyDirectObject, typeChar, theTextPtr, theTextLength);
	}
	return theError;
}

pascal OSErr HandleKGPOptionsEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	Size			actualSize;
	DescType		returnedType;
	DescType		optionType, settingType;
	
	if ((theError=AEGetParamPtr(theEvent, keyDirectObject, typeEnumerated, &returnedType,
		&optionType, sizeof(DescType), &actualSize))==noErr)
	{
		if (!IsOkayOptionTypeQQ(optionType))
			return errAEDescNotFound;
		
		if ((theError=AEGetKeyPtr((AERecord*)theEvent, kKGPSettingEnum, typeEnumerated, &returnedType,
			&settingType, sizeof(DescType), &actualSize))==noErr)
		{
			theError=GotRequiredParameters(theEvent);
		}
	}
	
	if (theError==noErr)
	{
		KGPSetOption(optionType, settingType);
	}
	
	return theError;
}

pascal OSErr HandleKGPCloseEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	DescType		saveType;
	Size			actualSize;
	DescType		returnedType;
	DescType		closeResult;
	
	saveType=0;
	if ((theError=GotRequiredParameters(theEvent))!=noErr)
	{
		if ((theError=AEGetKeyPtr((AERecord*)theEvent, kKGPSaveEnum, typeEnumerated,
			&returnedType, &saveType, sizeof(DescType), &actualSize))==noErr)
		{
			theError=GotRequiredParameters(theEvent);
		}
	}

	if (theError==noErr)
	{
		closeResult=KGPCloseDocument(saveType);
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &closeResult, sizeof(DescType));
	}
	
	return theError;
}

pascal OSErr HandleKGPSpeedEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	DescType		speedType;
	Size			actualSize;
	DescType		returnedType;
	
	speedType=0;
	if ((theError=AEGetKeyPtr((AERecord*)theEvent, kKGPSpeedEnum, typeEnumerated,
		&returnedType, &speedType, sizeof(DescType), &actualSize))==noErr)
	{
		theError=GotRequiredParameters(theEvent);
	}

	if (theError==noErr)
	{
		KGPSpeed(speedType);
	}
	
	return theError;
}

pascal OSErr HandleKGPMusicEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	DescType		musicType;
	Size			actualSize;
	DescType		returnedType;
	
	musicType=0;
	if ((theError=AEGetKeyPtr((AERecord*)theEvent, kKGPMusicEnum, typeEnumerated,
		&returnedType, &musicType, sizeof(DescType), &actualSize))==noErr)
	{
		theError=GotRequiredParameters(theEvent);
	}

	if (theError==noErr)
	{
		KGPMusic(musicType);
	}
	
	return theError;
}

pascal OSErr HandleKGPSaveEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	FSSpec			fs;
	Size			actualSize;
	DescType		returnedType;
	Boolean			gotFS;
	DescType		saveResult;
	
	theError=AEGetParamPtr(theEvent, keyDirectObject, typeFSS,  &returnedType, &fs,
		sizeof(FSSpec), &actualSize);
	gotFS=(theError==noErr);
	if ((theError=GotRequiredParameters(theEvent))==noErr)
	{
		saveResult=KGPSave(&fs, gotFS);
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &saveResult, sizeof(DescType));
	}
	
	return theError;
}

pascal OSErr HandleKGPRememberEvent(const AppleEvent *theEvent, const AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	
	theError=GotRequiredParameters(theEvent);
	if (theError==noErr)
	{
		KGPRemember();
	}
	return theError;
}

pascal OSErr HandleKGPRestoreEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	DescType		restoreResult;
	
	if ((theError=GotRequiredParameters(theEvent))==noErr)
	{
		restoreResult=KGPRestore();
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &restoreResult, sizeof(DescType));
	}
	
	return theError;
}

pascal OSErr HandleKGPSpeakEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(reply, refCon)
	
	OSErr			theError;
	DescType		speakResult;
	
	if ((theError=GotRequiredParameters(theEvent))==noErr)
	{
		speakResult=KGPSpeech(TRUE);
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &speakResult, sizeof(DescType));
	}
	
	return theError;
}

pascal OSErr HandleKGPSetVoiceEvent(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	#pragma unused(refCon)
	
	OSErr			theError;
	Str255			theVoiceStr;
	Size			actualSize;
	DescType		returnedType, voiceResult;
	
	if ((theError=AEGetKeyPtr((AERecord*)theEvent, keyDirectObject, typeChar,
		&returnedType, &theVoiceStr[1], 256, &actualSize))==noErr)
	{
		theVoiceStr[0]=actualSize;
		theError=GotRequiredParameters(theEvent);
	}

	if (theError==noErr)
	{
		voiceResult=KGPSetVoice(theVoiceStr);
		theError=AEPutParamPtr(reply, keyDirectObject, typeEnumerated, &voiceResult, sizeof(DescType));
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
	
	FailNilUPP((UniversalProcPtr)gHandleQuitUPP);
	gHandleKGPResolveUPP=NewAEEventHandlerProc(HandleKGPResolveEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPResolveUPP);
	gHandleKGPInsertUPP=NewAEEventHandlerProc(HandleKGPInsertEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPInsertUPP);
	gHandleKGPNewUPP=NewAEEventHandlerProc(HandleKGPNewEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPNewUPP);
	gHandleKGPUseModuleUPP=NewAEEventHandlerProc(HandleKGPUseModuleEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPUseModuleUPP);
	gHandleKGPGetTextUPP=NewAEEventHandlerProc(HandleKGPGetTextEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPGetTextUPP);
	gHandleKGPOptionsUPP=NewAEEventHandlerProc(HandleKGPOptionsEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPOptionsUPP);
	gHandleKGPCloseUPP=NewAEEventHandlerProc(HandleKGPCloseEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPCloseUPP);
	gHandleKGPSpeedUPP=NewAEEventHandlerProc(HandleKGPSpeedEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPSpeedUPP);
	gHandleKGPMusicUPP=NewAEEventHandlerProc(HandleKGPMusicEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPMusicUPP);
	gHandleKGPSaveUPP=NewAEEventHandlerProc(HandleKGPSaveEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPSaveUPP);
	gHandleKGPRememberUPP=NewAEEventHandlerProc(HandleKGPRememberEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPRememberUPP);
	gHandleKGPRestoreUPP=NewAEEventHandlerProc(HandleKGPRestoreEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPRestoreUPP);
	gHandleKGPSpeakUPP=NewAEEventHandlerProc(HandleKGPSpeakEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPSpeakUPP);
	gHandleKGPSetVoiceUPP=NewAEEventHandlerProc(HandleKGPSetVoiceEvent);
	FailNilUPP((UniversalProcPtr)gHandleKGPSetVoiceUPP);
	
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
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPResolve,
			gHandleKGPResolveUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPInsert,
			gHandleKGPInsertUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPNew,
			gHandleKGPNewUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPUseModule,
			gHandleKGPUseModuleUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPGetText,
			gHandleKGPGetTextUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPOptions,
			gHandleKGPOptionsUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPClose,
			gHandleKGPCloseUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPSpeed,
			gHandleKGPSpeedUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPMusic,
			gHandleKGPMusicUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPSave,
			gHandleKGPSaveUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPRemember,
			gHandleKGPRememberUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPRestore,
			gHandleKGPRestoreUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPSpeak,
			gHandleKGPSpeakUPP, 0, FALSE);
	if (result==noErr)
		result=AEInstallEventHandler(kKGPEventClass, kKGPSetVoice,
			gHandleKGPSetVoiceUPP, 0, FALSE);
	
	return result;
}
