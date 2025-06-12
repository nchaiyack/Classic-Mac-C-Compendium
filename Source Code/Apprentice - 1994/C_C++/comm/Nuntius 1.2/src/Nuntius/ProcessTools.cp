// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ProcessTools.cp

#include "ProcessTools.h"

#include <Errors.h>

#pragma segment MyTools

Boolean FindApplFromSignature(OSType sig, FSSpec &spec)
{
	// lidt modificeret udgave af LaunchBySignature i MacAppGlobals
	ProcessSerialNumber psn;
	OSErr err;
	short sysVRefNum;
	short vRefNum;
	short index;
	Boolean hasDesktopDB;

	// First see if it's already running:
	err = FindProcessBySignature(sig, psn, &spec);
	if (err == noErr)
		return true;

	// Well, it's not running. Let's have a look around:

	FailOSErr(GetSysVolume(sysVRefNum));
	vRefNum = sysVRefNum;						// Start search with boot volume
	index = 0;
	do
	{
		if (index == 0 || vRefNum != sysVRefNum)
		{
			err = VolHasDesktopDB(vRefNum, hasDesktopDB);
			if (err != noErr)
				continue;
			if (hasDesktopDB)
			{
				// If volume has a desktop DB,
				err = FindAppOnVolume(sig, vRefNum, spec);// ask it to find app
				if (err == noErr)
				{
					return true;
				}
			}
		}
		err = GetIndVolume(++index, vRefNum);	// Else go to next volume
	} while (err == noErr);						// Keep going until we run out of vols
	return false;
}

void LaunchApplication(FSSpec &applSpec, Boolean toFront)
{
	LaunchFlags launchControlFlags = launchContinue + launchNoFileFlags + launchAllow24Bit + launchUseMinimum;
	if (!toFront)
		launchControlFlags |= launchDontSwitch;

	static LaunchParamBlockRec glaunchPB;
	static FSSpec gLauchSpec = applSpec;

	glaunchPB.launchBlockID = extendedBlock;
	glaunchPB.launchEPBLength = extendedBlockLen;
	glaunchPB.launchFileFlags = launchNoFileFlags;
	glaunchPB.launchControlFlags = launchControlFlags;
	glaunchPB.launchAppSpec = &gLauchSpec;
	glaunchPB.launchAppParameters = nil;

	FailOSErr(LaunchApplication(glaunchPB));

	gApplication->PollToolboxEvent(false);
	gApplication->PollToolboxEvent(false);
	gApplication->PollToolboxEvent(false);
	gApplication->PollToolboxEvent(false);
}


void OpenApplicationDocument(FSSpec &applSpec, AliasHandle aliasH, Boolean toFront)
{
	static FSSpec gLauchSpec;
	static LaunchParamBlockRec glaunchPB;
	FailInfo fi;
	if (fi.Try())
	{
		AppleEvent event;
		AEDesc addr;
		ProcessSerialNumber psn;

//@@
#if 0
		FailOSErr(LaunchBySignature(applSignature, psn, &gLauchSpec, nil, true,
				launchContinue + launchNoFileFlags + launchDontSwitch + launchAllow24Bit));
		EventRecord toolEvent;
		for (short i = 0; i <= 25; i++)
//			EventAvail(everyEvent, toolEvent);
			EventAvail(0, toolEvent);
		if (FindProcessBySignature(applSignature, psn, &gLauchSpec) != noErr)
		{
#if qDebug
			DebugStr("Editor is not running as expected (I just launched it!)");
#endif
			fi.Success();
			return;
		}
#endif
//@@

		FInfo info;
		FailOSErr(FSpGetFInfo(applSpec, info));
		Boolean applIsRunning = FindProcessBySignature(info.fdCreator, psn, &gLauchSpec) == noErr;

#if qDebugLauchEditor
			fprintf(stderr, "ODOC: Appl is%s running\n", applIsRunning ? "" : " not");
#endif
		if (applIsRunning)
			FailOSErr(AECreateDesc(typeProcessSerialNumber, Ptr(&psn), sizeof(ProcessSerialNumber), addr));
		else
		{
				addr.descriptorType = typeNull; // in LaunchWithDoc, addr is undefined
				addr.dataHandle = nil;
		}
		
		FailOSErr(AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, addr, kAutoGenerateReturnID, kAnyTransactionID, event));
	
		AEDescList theList;
		AEDesc docDesc;
		FailOSErr(AECreateList(nil, 0, false, theList));
		HLock(Handle(aliasH));
		FailOSErr(AECreateDesc(typeAlias, Ptr(*aliasH), GetHandleSize(Handle(aliasH)), docDesc));
		HUnlock(Handle(aliasH));

		FailOSErr(AEPutDesc(theList, 0, docDesc));
		FailOSErr(AEPutParamDesc(event, keyDirectObject, theList));

		if (applIsRunning)
		{
			if (toFront)
			{
				FailOSErr(SetFrontProcess(psn));
				gApplication->PollToolboxEvent(false);
				gApplication->PollToolboxEvent(false);
				gApplication->PollToolboxEvent(false);
			}
			// editor is running, throw the apple-event to it
			AppleEvent dummyReply;
			FailOSErr(AESend(event, dummyReply, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer,
				kAENormalPriority, 60, nil, nil));
			fi.Success();
			return;
		}
		// we must launch the appl with an apple-event
		static LaunchParamBlockRec glaunchPB;
		gLauchSpec = applSpec;
		glaunchPB.launchAppSpec = &gLauchSpec;
		AEDesc launchDesc;
		AECoerceDesc(event, typeAppParameters, launchDesc);
		HLock(Handle(event.dataHandle));
		glaunchPB.launchAppParameters = (AppParametersPtr)*(launchDesc.dataHandle);
		glaunchPB.launchBlockID = extendedBlock;
		glaunchPB.launchEPBLength = extendedBlockLen;
		glaunchPB.launchFileFlags = 0;
		glaunchPB.launchControlFlags = launchContinue + launchNoFileFlags + launchAllow24Bit + launchUseMinimum;
		if (!toFront)
			glaunchPB.launchControlFlags += launchDontSwitch;
		FailOSErr(LaunchApplication(glaunchPB));
		gApplication->PollToolboxEvent(false);
		gApplication->PollToolboxEvent(false);
		gApplication->PollToolboxEvent(false);

		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}			
}
