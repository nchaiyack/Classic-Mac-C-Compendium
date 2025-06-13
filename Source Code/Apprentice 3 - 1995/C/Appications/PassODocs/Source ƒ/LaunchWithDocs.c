/*
	LaunchWithDocs.c
	
	Code to handle opening documents using AppleEvents.
	
	This code is loosely based upon the LaunchWithDoc2 code from DTS, but I shuffled things
	around to provide a more robust way to launch documents.  (See the Readme.DTS file for
	their info.
*/

#include <Aliases.h>

#include "LaunchWithDocs.h"

/*
	Send an odoc to the creator of a specified file.
*/
OSErr OpenSpecifiedDocumentByCreator(const FSSpec * docSpec){
	OSErr retCode;
	FInfo docInfo;
	
	// verify the document file exists and get its creator type
	
	retCode = FSpGetFInfo(docSpec, &docInfo);
	
	// if no errors, have the creator open the document...
	
	if (retCode == noErr)
		retCode=OpenSpecifiedDocumentWithCreator(docSpec,docInfo.fdCreator);
	
	return retCode;
}

/*
	OpenSpecifiedDocumentWithCreator
	
	Opens the specified document with a specified application.
*/
OSErr OpenSpecifiedDocumentWithCreator(const FSSpec * docSpec,OSType creator){
	ProcessSerialNumber psn;
	ProcessInfoRec pir;
	FSSpec appSpec;
	FInfo docInfo;
	Boolean found;
	OSErr retCode;
	
	// verify the document file exists
	retCode = FSpGetFInfo(docSpec, &docInfo);
	
	// check the current processes to see if the creator app is already
	// running, and get its process serial number (as psn)
	
	found=CreatorToProcessID(creator,&psn);
	
	// if the creator is running, send it an OpenDocuments Apple event
	// since there is no need to launch it
	
	if (found){
		retCode = SendOpenDocumentEventToProcess(&psn, docSpec, 1);
	} else {
		// else if the creator is not running, find it on disk and launch
		// it with the OpenDocuments event included as a part of the
		// launch parameters
	
		retCode = FindApplicationFromCreator(creator, &appSpec);
		
		if (retCode == noErr)
			retCode = LaunchApplicationWithDocument(&appSpec,docSpec);
	}
	
	return retCode;
}

/*
	Check the currently running processes to determine if the creator is already running.
	If the creator is found, it's process serial number is used and true is returned.  Otherwise
	false is returned indicating that the process could not be found.
*/
Boolean CreatorToProcessID(OSType creator,ProcessSerialNumber* psn){
	ProcessInfoRec pir;
	FSSpec appSpec;
	
	psn->lowLongOfPSN = kNoProcess;
	psn->highLongOfPSN = 0;
	
	pir.processInfoLength = sizeof(ProcessInfoRec);
	pir.processName = nil;
	pir.processAppSpec = &appSpec;
	
	while (GetNextProcess(psn) == noErr) {
		if (GetProcessInformation(psn, &pir) == noErr) {
			if (pir.processSignature == creator) {
				return true;
			}
		}
	}
	
	return false;
}

OSErr LaunchApplicationWithDocument(const FSSpec* appSpec,const FSSpec* docSpec){
	return LaunchApplicationWithDocuments(appSpec,docSpec,1);
}

//----------------------------------------------------------------------------
// LaunchApplicationWithDocuments
//
// given an application and any number of documents, 
// LaunchApplicationWithDocument launches the application and passes the 
// application an OpenDocuments event for the document(s)
//----------------------------------------------------------------------------
OSErr LaunchApplicationWithDocuments(const FSSpec* appSpec,const FSSpec* specArr,const short numSpecs){
	OSErr retCode;
	LaunchParamBlockRec lpb;
	AppleEvent ae;
	AEDesc descLaunch;
	ProcessSerialNumber target;
	
	if (numSpecs<1)
		return paramErr;
	
	// to simplify cleanup, ensure that handles are nil to start
	lpb.launchAppParameters= nil;
	ae.dataHandle= nil;
	descLaunch.dataHandle= nil;
	
	if (specArr != nil) {

		// because 'odoc' events require a address descriptor, I just 
		// grab the PSN for the current process.  It doesn't matter what
		// it is, because it's never used.
		(void) GetCurrentProcess(&target);
		
		// build an 'odoc' event given the array of FSSpecs and the ProcessSerialNumber
		retCode = BuildOpenDocumentEvent(&target, specArr, numSpecs, &ae);
		
		if (retCode == noErr) {
		
			// coerce the AppleEvent into app parameters, for _LaunchApplication
			retCode = AECoerceDesc(&ae, typeAppParameters, &descLaunch);
			
			if (retCode != noErr)
				goto Bail;
			
			// fill in the launch parameter block, including the
			// Apple event, and make the launch call
			HLock((Handle) descLaunch.dataHandle);
			lpb.launchAppParameters = (AppParametersPtr) *(descLaunch.dataHandle);
		}
	}

	lpb.launchBlockID = extendedBlock;
	lpb.launchEPBLength = extendedBlockLen;
	lpb.launchFileFlags = launchNoFileFlags;
	lpb.launchControlFlags = launchContinue;
	lpb.launchAppSpec = (FSSpecPtr)appSpec;

	retCode = LaunchApplication(&lpb);

Bail:
	// dispose of everything that was allocated
	if (ae.dataHandle != nil)
		(void) AEDisposeDesc(&ae);
	
	if (descLaunch.dataHandle != nil)
		(void) AEDisposeDesc(&descLaunch);
	
	return retCode;

}


//----------------------------------------------------------------------------
// SendOpenDocumentEventToProcess
//
// given an application's serial number and any number of documents, 
// SendOpenDocumentEventToProcess passes 
// the application an OpenDocuments event for the document
//----------------------------------------------------------------------------
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber* target,const FSSpec* specArr,const short numSpecs){
	OSErr retCode;
	AppleEvent ae, reply;
	
	ae.dataHandle = nil;
	retCode = BuildOpenDocumentEvent(target, specArr, numSpecs, &ae);
	
	if (retCode == noErr)
		retCode = AESend(&ae,&reply,kAENoReply,kAENormalPriority,kAEDefaultTimeout,nil,nil);
	
	// dispose of the AppleEvent if it was allocated	
	if (ae.dataHandle != nil)  
		(void) AEDisposeDesc(&ae);
	
	return retCode;
}


// FindApplicationFromDocument uses the Desktop Database to
// locate the creator application for the given document
//
// this routine will first check the desktop database of the disk
// containing the document, then the desktop database of all local
// disks, then the desktop databases of all server volumes
// (so up to three passes will be made)

OSErr FindApplicationFromDocument(const FSSpec* docSpec,FSSpecPtr appSpec){
	OSErr err=noErr;
	FInfo docInfo;
	
	// verify the document file exists and get its creator type
	
	err = FSpGetFInfo(docSpec, &docInfo);
	if (err== noErr)
		err=FindApplicationFromCreator(docInfo.fdCreator,appSpec);
	
	return err;
}

OSErr FindApplicationFromCreator(OSType creator,FSSpecPtr appSpec){
	enum { documentPass, localPass, remotePass, donePass } volumePass;
	
	DTPBRec deskParms;
	HParamBlockRec hfsParms;
	short index;
	Boolean found;
	GetVolParmsInfoBuffer vib;
	OSErr retCode;
	
	volumePass = documentPass;
	index = 0;
	found=false;
	
	while ((found == false)&&(volumePass != donePass)){
		
		// first, find the vRefNum of the volume whose Desktop Database
		// we're checking this time
		
		// find the vRefNum of the next volume appropriate
		// for this pass
		
		index++;
		
		// convert the volumeIndex into a vRefNum
		
		hfsParms.volumeParam.ioNamePtr = nil;
		hfsParms.volumeParam.ioVRefNum = 0;
		hfsParms.volumeParam.ioVolIndex = index;
		retCode = PBHGetVInfoSync(&hfsParms);
		
		// a nsvErr indicates that the current pass is over
		if (retCode == nsvErr)
			goto SkipThisVolume;
		
		if (retCode != noErr)
			goto Bail;
		
		// call GetVolParms to determine if this volume is a server
		// (a remote volume)
		
		hfsParms.ioParam.ioBuffer = (Ptr) &vib;
		hfsParms.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
		retCode = PBHGetVolParmsSync(&hfsParms);
		
		if (retCode != noErr)
			goto Bail;
		
		// if the vMServerAdr field of the volume information buffer
		// is zero, this is a local volume; skip this volume
		// if it's local on a remote pass or remote on a local pass
		
		if ((vib.vMServerAdr != 0)!=(volumePass == remotePass))
			goto SkipThisVolume;
		
		// okay, now we've found the vRefNum for our desktop database call
		
		deskParms.ioVRefNum = hfsParms.volumeParam.ioVRefNum;
		
		// find the path refNum for the desktop database for
		// the volume we're interested in
		
		deskParms.ioNamePtr = nil;
		
		retCode = PBDTGetPath(&deskParms);
		
		if ((retCode == noErr)&&(deskParms.ioDTRefNum != 0)){
		
			// use the GetAPPL call to find the preferred application
			// for opening any document with this one's creator
			
			deskParms.ioIndex = 0;
			deskParms.ioFileCreator = creator;
			deskParms.ioNamePtr = appSpec->name;
			
			retCode = PBDTGetAPPLSync(&deskParms);
			
			if (retCode == noErr) {
			
				// okay, found it; fill in the application file spec
				// and set the flag indicating we're done
				
				appSpec->parID = deskParms.ioAPPLParID;
				appSpec->vRefNum = deskParms.ioVRefNum;
				found = true;
				
			}
		}
		
	SkipThisVolume:
	
		// if retCode indicates a no such volume error or if this
		// was the first pass, it's time to move on to the next pass
		
		if ((retCode == nsvErr)||(volumePass == documentPass)){
			volumePass++;
			index = 0;
		}
		
	};
	
Bail:
	return retCode;
}


//----------------------------------------------------------------------------
// BuildOpenDocumentsEvent
//
// General utility to turn a ProcessSerialNumber and a list of FSSpecs into
// an 'odoc' AppleEvent with the ProcessSerialNumber as the target
// application.  Used by SendOpenDocumentEventToProcess, and
// LaunchApplicationWithDocument.
//----------------------------------------------------------------------------
OSErr BuildOpenDocumentEvent(ProcessSerialNumber* target,const FSSpec* specArr,const short numSpecs,AppleEvent* odoc){
	OSErr retCode;
	AppleEvent ae;
	AEDesc descTarget, docDesc;
	AEDescList docList;
	AliasHandle docAlias;
	short counter;
	FSSpecPtr iter;

	// to simplify cleanup, ensure that handles are nil to start
	descTarget.dataHandle= nil;
	ae.dataHandle= nil;
	docList.dataHandle= nil;
	docDesc.dataHandle= nil;
	docAlias= nil;

	// create an address descriptor based on the serial number of
	// the target process
	retCode = AECreateDesc(typeProcessSerialNumber,(Ptr)target,sizeof(ProcessSerialNumber), &descTarget);
	
	if (retCode != noErr)
		goto Bail;
	
	// make a descriptor list containing just a descriptor with an
	// alias to the document
	retCode = AECreateList(nil, 0, false, &docList);
	
	if (retCode != noErr)
		goto Bail;

	// start at the beginning of the FSSpec list, and start adding
	// them to the document list descriptor

	// NOTE: we need to make sure we dispose of the aliases and the
	// AE descriptor in this loop, otherwise there will be memory
	// leaks.
	iter = (FSSpecPtr)specArr;
	for (counter = 0; counter < numSpecs; counter ++) {	

		retCode = NewAlias(nil, &iter[counter], &docAlias);
		if (retCode != noErr)
			goto Bail;
		
		HLock((Handle) docAlias);
		
		retCode = AECreateDesc(typeAlias, (Ptr) *docAlias,InlineGetHandleSize((Handle) docAlias), &docDesc);
		
		HUnlock((Handle) docAlias);
		
		if (retCode != noErr)
			goto Bail;
		
		// the alias is now in the AEDescriptor, so dispose of it
		DisposeHandle((Handle)docAlias);
		docAlias = nil;
		
		retCode = AEPutDesc(&docList, 0, &docDesc);
		
		if (retCode != noErr)
			goto Bail;

		// the alias is now in the AE document list, so dispose of it
		retCode = AEDisposeDesc(&docDesc);
		
		if (retCode != noErr)
			goto Bail;
	}
	
	// now make the 'odoc' AppleEvent descriptor and insert the 
	// document descriptor list as the direct object
	retCode = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,&descTarget, kAutoGenerateReturnID, kAnyTransactionID,&ae);
	
	if (retCode != noErr)
		goto Bail;
	
	retCode = AEPutParamDesc(&ae, keyDirectObject, &docList);
	
	if (retCode != noErr)
		goto Bail;
	
	*odoc = ae;
	
Bail:
	// dispose of everything that was allocated, except the return AE
	if (descTarget.dataHandle != nil)
		(void) AEDisposeDesc(&descTarget);
	
	if (docList.dataHandle != nil)
		(void) AEDisposeDesc(&docList);
	
	if (docDesc.dataHandle != nil)
		(void) AEDisposeDesc(&docDesc);
	
	if (docAlias != nil)
		DisposeHandle((Handle) docAlias);
	
	return retCode;
}
