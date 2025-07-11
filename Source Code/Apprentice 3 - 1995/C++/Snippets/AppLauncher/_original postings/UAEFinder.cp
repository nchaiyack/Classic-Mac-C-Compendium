//-------------------------------------------------------------------------------
// UAEFinder.cp			Originally by Peter N Lewis (peter@mail.peter.com.au)
//						Converted to C++ by Gilles Dignard (gdignard@hookup.net)
//-------------------------------------------------------------------------------

#include "UAEFinder.h"

Boolean
UAEFinder::TellFinderToLaunch (FSSpec& ioSpec, Boolean inToFront)
{
	ProcessSerialNumber		process;
	OSErr					err;
	OSErr					junk;
	AEDesc					targetAddress;
	AEDescList				fileList;
	AppleEvent				theEvent;
	AppleEvent				theReply;
	AESendMode				sendmode;
	long					gv;
	FSSpec					finder_fs;
	
	err = -1;
	if ((::Gestalt(gestaltFinderAttr, &gv) == noErr) 
					&& BTST(gv, gestaltOSLCompliantFinder)) {
		if (FindProcess ('MACS', 'FNDR', process, finder_fs)) {

			err = ::AECreateDesc(	typeProcessSerialNumber,
									&process, sizeof(process),
									&targetAddress);
			
			if (err == noErr)
				err = ::AECreateAppleEvent(	kCoreEventClass,
											kAEOpenDocuments,
											&targetAddress,
											kAutoGenerateReturnID,
											kAnyTransactionID,
											&theEvent);
			
			if (err == noErr)
				err = ::AECreateList(nil, 0, false, &fileList);
			
			if (err == noErr)
				AddFSSToAEList(fileList, 1, ioSpec);
			
			if (err == noErr)
				err = ::AEPutParamDesc(&theEvent, keyDirectObject, &fileList);
			
			if (err == noErr) {
				sendmode = kAENoReply;
				if (!inToFront)
					sendmode = sendmode + kAENeverInteract;
				
				err = ::AESend(	&theEvent,
								&theReply,
								sendmode,
								kAEHighPriority,
								kNoTimeOut,
								nil, nil);
			}
			
			if ((err == noErr) && (inToFront))
				junk = ::SetFrontProcess(&process);

			::AEDisposeDesc(&theEvent);
			::AEDisposeDesc(&theReply);
			::AEDisposeDesc(&fileList);
			::AEDisposeDesc(&targetAddress);			

		}	// FindProcess
	}	// Gestalt
	return (err == noErr);
}

Boolean
UAEFinder::OpenControlPanel (OSType inCreator)
{
	Boolean theResult = false;
	
	FSSpec theSpec;
	
	if (FindControlPanel(inCreator, theSpec) == noErr)
		theResult = TellFinderToLaunch(theSpec, true);
	
	return theResult;
}


void
UAEFinder::LaunchWithDocument (OSType inCreator, OSType inType, FSSpec& inSpec, Boolean inToFront)
{
	ProcessSerialNumber psn;
	AEDesc targetAddress;
	AppleEvent theEvent;
	AppleEvent theReply;
	AEDescList fileList;
	FSSpec app_fs;
	OSErr oe;
	long gv;
	AESendMode sendmode;
	long t;
	long c;
	
	::PurgeSpace(&t, &c);
	if ((::Gestalt(gestaltOSAttr, &gv) == noErr)
	 && (BTST(gv, gestaltLaunchControl))
	 && (c > 4096)) {
		if (FindProcess(inCreator, inType, psn, app_fs)) {
			oe = ::AECreateDesc(typeProcessSerialNumber, &psn, sizeof(psn), &targetAddress);
			oe = ::AECreateAppleEvent(	kCoreEventClass,
										kAEOpenDocuments,
										&targetAddress,
										kAutoGenerateReturnID,
										kAnyTransactionID,
										&theEvent);
			oe = ::AEDisposeDesc(&targetAddress);
			oe = ::AECreateList(nil, 0, false, &fileList);
			AddFSSToAEList(fileList, 1, inSpec);
			oe = ::AEPutParamDesc(&theEvent, keyDirectObject, &fileList);
			sendmode = kAENoReply;
			if (!inToFront)
				sendmode += kAENeverInteract;
			oe = ::AESend(	&theEvent,
							&theReply,
							sendmode,
							kAEHighPriority,
							kNoTimeOut,
							nil, nil);
			oe = ::AEDisposeDesc(&theEvent);
			oe = ::AEDisposeDesc(&theReply);
			oe = ::AEDisposeDesc(&fileList);
			if (!inToFront)
				oe = ::SetFrontProcess(&psn);
		}
	}
}


void
UAEFinder::LaunchApp (OSType inCreator, OSType inType, Boolean inToFront)
{
	ProcessSerialNumber psn;
	FSSpec app_fs;
	OSErr oe;
	long gv;
	
	if ((::Gestalt(gestaltOSAttr, &gv) == noErr) && (BTST(gv, gestaltLaunchControl))) {
		if (FindProcess(inCreator, inType, psn, app_fs)) {
			if (inToFront)
				oe = ::SetFrontProcess(&psn);
		} else {
			if (FindApplication(inCreator, app_fs) == noErr)
				LaunchFSSpec(app_fs, inToFront);
		}
	}
}


void
UAEFinder::LaunchFSSpec (FSSpec& ioSpec, Boolean inToFront)
{
	OSErr oe;
	FInfo fi;
	AEDesc targetAddress;
	AppleEvent theEvent;
	long gv;
	LaunchParamBlockRec launchThis;
	AEDesc launchDesc;
	
	if ((::Gestalt(gestaltOSAttr, &gv) == noErr) && (BTST(gv, gestaltLaunchControl))) {
		oe = ::FSpGetFInfo(&ioSpec, &fi);
		oe = ::AECreateDesc(	typeApplSignature,
								&fi.fdCreator,
								sizeof(fi.fdCreator),
								&targetAddress);
		oe = ::AECreateAppleEvent(	kCoreEventClass,
									kAEOpenApplication,
									&targetAddress,
									kAutoGenerateReturnID,
									kAnyTransactionID,
									&theEvent);
		oe = ::AEDisposeDesc(&targetAddress);
		launchThis.launchAppSpec = &ioSpec;
		PrepareToLaunch(theEvent, inToFront, launchDesc, launchThis);
		oe = ::LaunchApplication(&launchThis);
		oe = ::AEDisposeDesc(&theEvent);
	}
}

void
UAEFinder::QuitApplication (OSType inCreator, OSType inType)
{
	ProcessSerialNumber process;
	AEAddressDesc targetAddress;
	AppleEvent theEvent;
	AppleEvent theReply;
	FSSpec fs;
	OSErr oe;
	
	if (FindProcess(inCreator, inType, process, fs)) {
		oe = ::AECreateDesc(	typeProcessSerialNumber,
								&process,
								sizeof(process),
								&targetAddress);
		oe = ::AECreateAppleEvent(	kCoreEventClass,
									kAEQuitApplication,
									&targetAddress,
									kAutoGenerateReturnID,
									kAnyTransactionID,
									&theEvent);
		oe = ::AEDisposeDesc(&targetAddress);
		oe = ::AESend(	&theEvent,
						&theReply,
						kAENoReply,
						kAEHighPriority,
						5 * 60,
						nil, nil);
		oe = ::AEDisposeDesc(&theEvent);
		oe = ::AEDisposeDesc(&theReply);
	}
}


OSErr
UAEFinder::FindApplication (OSType inCreator, FSSpec& ioSpec)
{
	short i;
	DTPBRec pbdt;
	long freebytes;
	OSErr oe;
	Boolean found;
	long gv;
	
	found = false;
	if (::Gestalt(gestaltSystemVersion, &gv) && (gv >= 0x0700)) {	// has system 7
		i = 1;
		do {
			ioSpec.vRefNum = 0;
			oe = ::GetVInfo(i, ioSpec.name, &ioSpec.vRefNum, &freebytes);
			i++;
			if (oe == noErr) {
				ioSpec.name[0] = 0;
				pbdt.ioNamePtr = ioSpec.name;
				oe = ::PBDTGetPath(&pbdt);
				if (oe == noErr) {
					pbdt.ioIndex = 0;
					pbdt.ioFileCreator = inCreator;
					oe = ::PBDTGetAPPLSync(&pbdt);
					if (oe == noErr)
						found = true;
				}
				oe = noErr;
			}
		} while ((!found) && (oe == noErr));
	}
	if (found) {
		oe = noErr;
		ioSpec.parID = pbdt.ioAPPLParID;
	} else {
		oe = afpItemNotFound;
		ioSpec.vRefNum = 0;
		ioSpec.parID = 2;
		ioSpec.name[0] = 0;
	}
	return oe;
}


Boolean
UAEFinder::FindProcess (OSType inCreator, OSType inType,
			 ProcessSerialNumber& ioProcess, FSSpec& ioSpec)
{
	ProcessInfoRec info;
	long gv;
	
	Boolean processWasFound = false;
	
	if ((::Gestalt(gestaltOSAttr, &gv) == noErr) && (BTST(gv, gestaltLaunchControl))) {
		ioProcess.highLongOfPSN = 0;
		ioProcess.lowLongOfPSN = kNoProcess;
		info.processInfoLength = sizeof(ProcessInfoRec);
		info.processName = nil;
		info.processAppSpec = &ioSpec;
		while (::GetNextProcess(&ioProcess) == noErr) {
			if (::GetProcessInformation(&ioProcess, &info) == noErr) {
				if ((info.processType == (long) inType)
				 && (info.processSignature == inCreator)) {
					processWasFound = true;
					break;
				}
			}
		}
	}
	
	return processWasFound;
}


OSErr
UAEFinder::FindControlPanel (OSType inCreator, FSSpec& ioSpec)
{
	HParamBlockRec	pb;
	short			i;
	OSErr			theErr;
	
	theErr = ::FindFolder(kOnSystemDisk, kControlPanelFolderType, false,
							&ioSpec.vRefNum, &ioSpec.parID);
	if (theErr == noErr) {
		i = 1;
		do {
			pb.fileParam.ioNamePtr = ioSpec.name;
			pb.fileParam.ioVRefNum = ioSpec.vRefNum;
			pb.fileParam.ioFRefNum = ioSpec.parID;
			pb.fileParam.ioFDirIndex = i;
			i++;
			theErr = ::PBHGetFInfoSync(&pb);
			if (theErr == noErr) {
				if (   (pb.fileParam.ioFlFndrInfo.fdType == 'cdev')
					&& (pb.fileParam.ioFlFndrInfo.fdCreator == inCreator))
					break;
			}
		} while (theErr == noErr);
	}
	return theErr;
}


Boolean
UAEFinder::BTST (long inLong, short inBitNo)
{
	return ((inLong >> inBitNo) & 1);	// ####
}

void
UAEFinder::PrepareToLaunch (AppleEvent &ioEvent, Boolean toFront,
				 AEDesc &ioLaunchDesc, LaunchParamBlockRec &ioLaunchThis)
{
	OSErr oe;
	
	oe = ::AECoerceDesc(&ioEvent, typeAppParameters, &ioLaunchDesc);
	HLock((Handle) ioEvent.dataHandle);
	ioLaunchThis.launchAppParameters = (AppParametersPtr) *(ioLaunchDesc.dataHandle);
	ioLaunchThis.launchBlockID = extendedBlock;
	ioLaunchThis.launchEPBLength = extendedBlockLen;
	ioLaunchThis.launchFileFlags = 0;
	ioLaunchThis.launchControlFlags = launchContinue + launchNoFileFlags;
	if (!toFront)
		ioLaunchThis.launchControlFlags += launchDontSwitch;
}


void
UAEFinder::AddFSSToAEList (AEDescList& ioList, short inRow, FSSpec& ioSpec)
{
	AliasHandle	fileAlias;
	OSErr		theErr;
	
	theErr = ::NewAlias(nil, &ioSpec, &fileAlias);
	if (theErr == noErr) {
		::HLock((Handle) fileAlias);
		theErr = ::AEPutPtr(&ioList, inRow, typeAlias,
							(Ptr) *fileAlias, (*fileAlias)->aliasSize);
		::DisposeHandle((Handle) fileAlias);
	}
}
