#include <Aliases.h>
#include "SendFinderOpen.h"

#define kFinderSig			'FNDR'
#define kAEFinderEvents		'FNDR'
#define kSystemType			'MACS'

#define	kAEOpenSelection	'sope'
#define keySelection		'fsel'

OSErr SendFinderOpenAE(FSSpec *theDoc)
{
AppleEvent			aeEvent;
AEDesc				myAddressDesc;
AEDesc				aeDirDesc;
AEDesc				listElem;
AEDescList			fileList;
FSSpec				dirSpec;
AliasHandle			dirAlias;
AliasHandle			fileAlias;
ProcessSerialNumber	process;
OSErr				myErr;
OSType			FndrType = 'MACS';
	
	/*
	 * Get the psn of the Finder and create the target address for the AE
	 */
	
	if ( FindAProcess( kFinderSig, kSystemType, &process) )
		return( procNotFound );

	myErr = AECreateDesc( typeProcessSerialNumber, (Ptr)&process, 
							sizeof(process), &myAddressDesc);
	if (myErr) return(myErr);
	
	/*
	 * Create an empty Apple Event
	 */
	
	myErr = AECreateAppleEvent( kAEFinderEvents, kAEOpenSelection, &myAddressDesc,
								kAutoGenerateReturnID, kAnyTransactionID, &aeEvent);
	
	if (myErr) return(myErr);
	
	/*
	 * Make and FSSpec for the parent folder and an alias of the file.
	 */
	
	FSMakeFSSpec( theDoc->vRefNum, theDoc->parID, 0L, &dirSpec);
	NewAlias( nil, &dirSpec, &dirAlias);
	NewAlias( nil, theDoc, &fileAlias);
	
	
	/*
	 * Create the File list.
	 *
	 */
	
	HLockHi( (Handle)dirAlias);
	AECreateDesc( typeAlias, (Ptr)*dirAlias, GetHandleSize( (Handle)dirAlias), &aeDirDesc);
	HUnlock( (Handle)dirAlias);
	DisposeHandle( (Handle)dirAlias);
	
	if ( (myErr=AEPutParamDesc(&aeEvent, keyDirectObject, &aeDirDesc)) == noErr )
	{
		AEDisposeDesc( &aeDirDesc );
		
		AECreateList(nil, 0, FALSE, &fileList);
		HLockHi( (Handle)fileAlias);
		myErr = AECreateDesc( typeAlias, (Ptr)*fileAlias, 
									GetHandleSize( (Handle)fileAlias), &listElem);
		HUnlock( (Handle)fileAlias);

		DisposeHandle( (Handle)fileAlias);
	
		myErr = AEPutDesc( &fileList, 0L, &listElem);
		
		if (myErr) return(myErr);
		
	}
	if (myErr) return(myErr);
	
	AEDisposeDesc(&listElem);
	
	myErr = AEPutParamDesc(&aeEvent, keySelection, &fileList);
	if (myErr) return(myErr);

	myErr = AEDisposeDesc( &fileList);
	if (myErr) return(myErr);
	
	myErr = AESend( &aeEvent, 0L, kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
					kAENormalPriority, kAEDefaultTimeout, 0L, 0L);\
				
	AEDisposeDesc(&aeEvent);
	
	return(myErr);
}

/*
 * Search though the current process list to find the given application. 
 *
 */

OSErr FindAProcess( OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN)
{
ProcessInfoRec			tempInfo;
FSSpec					procSpec;
Str31					processName;
OSErr					myErr = noErr;

	/*
	 * Start at begining of process list
	 */
	
	processSN->lowLongOfPSN = kNoProcess;
	processSN->highLongOfPSN = kNoProcess;
	
	/*
	 * Init the process information record.
	 *
	 */
	
	tempInfo.processInfoLength = sizeof(ProcessInfoRec);
	tempInfo.processName = (StringPtr)&processName;
	tempInfo.processAppSpec = &procSpec;
	
	while ( (tempInfo.processSignature != creatorToFind || tempInfo.processType != typeToFind)
				|| myErr != noErr)
	{
		myErr = GetNextProcess(processSN);
		if (myErr == noErr)
			GetProcessInformation( processSN, &tempInfo);
	}
		
	return(myErr);
}
 
