/* mac_event.c */

#include "mac_event.h"
#include <Sound.h>


Boolean								ReceivedOpenEventFlag = false;
char									FakeKeyBuffer[MAXKEYS];
int										KeyBufPtr = 0;

Boolean								QuitPending = false;

/* parameters controlling the synthesis, with handy default values. */
short									AntiAliasing = true;
short									StereoOn = false;
unsigned short				SamplingRate = 22254;
short									NumRepeats = 1;
short									Speed = 50;
short									StereoMix = 0;
short									Loudness = 48;
short									NumBits = 8; /* this is new in version 1.10! */

int										RecalibratePlayer = false; /* set when settings change */


FSSpec								GlobalFileSpec;

ProcessSerialNumber		WhoLaunchedUs;


/* function to check to see that all required parameters have been gotten */
OSErr		MyGotRequiredParams(AppleEvent* theAppleEvent)
	{
		DescType		ReturnedType;
		Size				ActualSize;
		OSErr				Error;

		Error = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
			&ReturnedType,NULL,0,&ActualSize);
		if (Error == errAEDescNotFound)
			{
				return noErr;  /* we got all the params, since no more were found */
			}
		 else
			{
				if (Error == noErr)
					{
						BUG("\pDidn't get all required parameters from apple event");
						return errAEEventNotHandled;  /* missed some, so it failed */
					}
				 else
					{
						BUG("\pAEGetAttributePtr failed in MyGotRequiredParams");
						return Error;  /* AEGetAttributePtr failed, so we return why */
					}
			}
	}


pascal OSErr MyHandleODoc(AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefcon)
	{
		OSErr				Error;
		FSSpec			MyFSS;  /* place to put file info */
		long				Index,ItemsInList;
		AEDescList	DocList;
		Size				ActualSize;
		AEKeyword		Keywd;
		DescType		ReturnedType;

		/* get the direct parameter--a descriptor list--and put it into DocList */
		Error = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &DocList);
		if (Error != noErr)
			{
				BUG("\pCouldn't get file list");
			}
		if (Error != noErr) return Error;

		/* now we read all the additional parameters that MIGHT be present */
		/* determining how to do the synthesis.  They won't be present if the special */
		/* interface program was not used to launch us.  In that case the defaults */
		/* will take over. */

		Error = AEGetParamPtr(theAppleEvent,keyAntiAliasing,typeShortInteger,
			&ReturnedType,(void*)&AntiAliasing,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyAntiAliasing"); /* for debugging--don't worry about it */
			}

		Error = AEGetParamPtr(theAppleEvent,keyStereoOn,typeShortInteger,
			&ReturnedType,(void*)&StereoOn,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyStereoOn");
			}

		Error = AEGetParamPtr(theAppleEvent,keySamplingRate,typeShortInteger,
			&ReturnedType,(void*)&SamplingRate,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keySamplingRate");
			}

		Error = AEGetParamPtr(theAppleEvent,keyNumRepeats,typeShortInteger,
			&ReturnedType,(void*)&NumRepeats,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyNumRepeats");
			}

		Error = AEGetParamPtr(theAppleEvent,keySpeed,typeShortInteger,
			&ReturnedType,(void*)&Speed,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keySpeed");
			}

		Error = AEGetParamPtr(theAppleEvent,keyStereoMix,typeShortInteger,
			&ReturnedType,(void*)&StereoMix,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyStereoMix");
			}

		Error = AEGetParamPtr(theAppleEvent,keyLoudness,typeShortInteger,
			&ReturnedType,(void*)&Loudness,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyLoudness");
			}

		Error = AEGetParamPtr(theAppleEvent,keyNumBits,typeShortInteger,
			&ReturnedType,(void*)&NumBits,sizeof(short),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keyNumBits");
			}

		Error = AEGetParamPtr(theAppleEvent,keySenderPSN,typeProcessSerialNumber,
			&ReturnedType,(void*)&WhoLaunchedUs,sizeof(ProcessSerialNumber),&ActualSize);
		if (Error != noErr)
			{
				BUG("\pBad keySenderPSN");
			}

		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		if (Error != noErr)
			{
				BUG("\Not all required params handled");
			}
		if (Error != noErr) return Error;
		/* count the number of descriptor records in the list */
		Error = AECountItems(&DocList,&ItemsInList);
		/* now get each descriptor record from the list, coerce the returned data */
		/* to an FSSpec record, and open the associated file */
		for (Index=1; Index <= ItemsInList; Index += 1)
			{
				Error = AEGetNthPtr(&DocList,Index,typeFSS,&Keywd,&ReturnedType,
					(void*)&MyFSS,sizeof(FSSpec),&ActualSize);
				if (Error == noErr)
					{
						GlobalFileSpec = MyFSS;
					}
				 else
					{
						BUG("\pCouldn't get FSSpec out of type list in MyHandleODoc");
					}
			}
		Error = AEDisposeDesc(&DocList);

		ReceivedOpenEventFlag = true;

		return Error;
	}


/* this receives a simulated keypress from the other system.  The idea */
/* was that we could simulate the user pressing '>', '<', and others by */
/* sending apple events.  Unfortunately, since things are precomputed as far */
/* ahead as we have memory for, the commands don't show up for many seconds. */
/* If I ever get around to making this interrupt driven, well, the mechanism */
/* is here ready and waiting. */
pascal OSErr MyHandleKey(AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefcon)
	{
		OSErr				Error;
		short				KeyPressed = 0; /* default key does nothing */
		long				ActualSize;
		DescType		ReturnedType;

		Error = AEGetParamPtr(theAppleEvent,keyKeyPressCharacter,typeShortInteger,
			&ReturnedType,(void*)&KeyPressed,sizeof(short),&ActualSize);

		Error = AEGetParamPtr(theAppleEvent,keyAntiAliasing,typeShortInteger,
			&ReturnedType,(void*)&AntiAliasing,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keyStereoOn,typeShortInteger,
			&ReturnedType,(void*)&StereoOn,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keySamplingRate,typeShortInteger,
			&ReturnedType,(void*)&SamplingRate,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keyNumRepeats,typeShortInteger,
			&ReturnedType,(void*)&NumRepeats,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keySpeed,typeShortInteger,
			&ReturnedType,(void*)&Speed,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keyStereoMix,typeShortInteger,
			&ReturnedType,(void*)&StereoMix,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keyLoudness,typeShortInteger,
			&ReturnedType,(void*)&Loudness,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = AEGetParamPtr(theAppleEvent,keyNumBits,typeShortInteger,
			&ReturnedType,(void*)&NumBits,sizeof(short),&ActualSize);
		if (Error == noErr)
			{
				RecalibratePlayer = true;
			}

		Error = noErr;

		if (Error == noErr)
			{
				Error = MyGotRequiredParams(theAppleEvent);
				if (Error != noErr)
					{
						return Error;
					}
				if ((KeyBufPtr < MAXKEYS - 1) && (KeyPressed != 0))
					{
						FakeKeyBuffer[KeyBufPtr] = KeyPressed;
						KeyBufPtr += 1;
					}
				return noErr;
			}
		 else
			{
				return Error;
			}
	}


/* forced quit event on system shutdown or user stoppage */
pascal OSErr MyHandleQuit(AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefcon)
	{
		OSErr			Error;

		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		if (Error != noErr) return Error;
		QuitPending = true;
		discard_buffer();
		return noErr;
	}


/* this is a simple event loop.  Not having an interface, we don't pay any */
/* attention to anything but apple events.  This also gives time to other */
/* applications.  One improvement: adding checks for keyboard events might */
/* be handy when the program is run without the interface.  They could just */
/* be queued like they are done in MyHandleKey. */
void			WaitForEvent(long SleepTime)
	{
		EventRecord				MyEvent;
		OSErr							Error;

		WaitNextEvent(everyEvent,&MyEvent,SleepTime,NULL);
		switch (MyEvent.what)
			{
				case kHighLevelEvent:
					Error = AEProcessAppleEvent(&MyEvent);
					break;
				default:
					break;
			}
	}


/* whip off a message to the interface application explaining why the */
/* song sounds like crap! */
void				FatalError(short ErrorID)
	{
		short							Error;
		AppleEvent				Event;
		AEAddressDesc			AddressDescriptor;
		AppleEvent				Reply;

		Error = AECreateDesc(typeProcessSerialNumber,(void*)&WhoLaunchedUs,
			sizeof(ProcessSerialNumber),&AddressDescriptor);
		Error = AECreateAppleEvent(ControlEventClass,ErrorEvent,&AddressDescriptor,
			kAutoGenerateReturnID,kAnyTransactionID,&Event);
		Error = AEPutParamPtr(&Event,keyErrorIDNum,typeShortInteger,
			(void*)&ErrorID,sizeof(short));
		Error = AESend(&Event,&Reply,kAENoReply,kAENormalPriority,kNoTimeOut,NULL,NULL);
	}


int					RegisterEventHandlers(void)
	{
		OSErr				Error;

		/* installing open document handler */
		Error = AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,&MyHandleODoc,0,false);
		if (Error != noErr)
			{
				return false;
			}

		Error = AEInstallEventHandler(ControlEventClass,ControlEvent,&MyHandleKey,0,false);
		if (Error != noErr)
			{
				return false;
			}

		Error = AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,&MyHandleQuit,0,false);
		if (Error != noErr)
			{
				return false;
			}
		return true;
	}
