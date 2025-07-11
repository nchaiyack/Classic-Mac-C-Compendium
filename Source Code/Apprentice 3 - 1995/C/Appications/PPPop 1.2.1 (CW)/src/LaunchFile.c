/* PPPop, verison 1.2  June 6, 1995  Rob Friefeld

   Launch a file using Apple Events
   Think C 6.0 source code. From Think Reference 2.0
   The application must set the High Level Event Aware flag in its size resource.
   Thanks to Richard Buckle for sharing his code for this.
*/

#include "LaunchFile.h"

static FSSpec	MacTCPSpec;
static FSSpec	ConfigPPPSpec;

void SetUpLaunch()
{
	short		vRefNum;
	long		dirID;
	OSErr		result;
	long		gestaltAnswer;
	long		found;
	short		i;

	for (i = 0; i < 3; i++)		// This array is used to enable and disable menu items
		gLaunchOK[i] = 0;		// Initialize elements to FALSE
	
	result = Gestalt(gestaltAppleEventsAttr , &gestaltAnswer);
	if (result != noErr)
		return;
	if (!BitTst(&gestaltAnswer,31- gestaltAppleEventsPresent)) 
		return;

	gLaunchOK[0] = true;		// Return to Finder will work.

	result = FindFolder(kOnSystemDisk, kControlPanelFolderType, kDontCreateFolder,
				&vRefNum, &dirID);
	if (result != noErr)
		return;
		
	found =	FindControlPanel( &ConfigPPPSpec, kMacPPPType,	vRefNum, dirID );
	if (found == 1)
		gLaunchOK[1] = true;	// Open Config PPP will work
			
	found =	FindControlPanel( &MacTCPSpec, kMacTCPType,	vRefNum, dirID );
	if (found == 1)
		gLaunchOK[2] = true;	// Open MacTCP will work
}



void OpenConfigPPP()
{
	OSErr	result;
	
	result = ScheduleFinder();
	if (result == noErr)
		result = OpenSelection(&ConfigPPPSpec);
	if (result != noErr)
		SysBeep(5);
}


void OpenMacTCP()
{
	OSErr	result;
	
	result = ScheduleFinder();
	if (result == noErr)
		result = OpenSelection(&MacTCPSpec);
	if (result != noErr)
		SysBeep(5);

}



OSErr OpenSelection(FSSpecPtr theDoc)
{
	AppleEvent	aeEvent;	// event to create
	AEDesc		myAddressDesc;
	AEDesc		aeDirDesc;
	AEDesc		listElem;
	AEDesc		fileList;
	FSSpec		dirSpec;
	AliasHandle	dirAlias;
	AliasHandle	fileAlias;
	ProcessSerialNumber	process;
	OSErr		myErr;

	if (FindAProcess(kFinderSig, kSystemType, &process))
		return procNotFound;
	myErr = AECreateDesc(typeProcessSerialNumber, (Ptr) &process,
				sizeof(process), &myAddressDesc);
	if (myErr) return myErr;

	myErr = AECreateAppleEvent(kAEFinderEvents, kAEOpenSelection,
				&myAddressDesc, kAutoGenerateReturnID, kAnyTransactionID,
				&aeEvent);
	if (myErr) return myErr;

	FSMakeFSSpec(theDoc->vRefNum, theDoc->parID, nil, &dirSpec);
	NewAlias(nil, &dirSpec, &dirAlias);
	NewAlias(nil, theDoc, &fileAlias);

	if (myErr = AECreateList(nil, 0, false, &fileList))
		return myErr;

	HLock((Handle)dirAlias);
	AECreateDesc(typeAlias, (Ptr) *dirAlias, GetHandleSize((Handle) dirAlias),
				&aeDirDesc);
	HUnlock((Handle)dirAlias);
	DisposHandle((Handle)dirAlias);

	if ((myErr = AEPutParamDesc(&aeEvent, keyDirectObject, &aeDirDesc)) ==
				noErr) {
		AEDisposeDesc(&aeDirDesc);
		HLock((Handle)fileAlias);

		AECreateDesc(typeAlias, (Ptr) *fileAlias, GetHandleSize((Handle)fileAlias),
				&listElem);
		HUnlock((Handle)fileAlias);
		DisposHandle((Handle)fileAlias);
		myErr = AEPutDesc(&fileList, 0, &listElem);
	}

	if (myErr) return myErr;
	AEDisposeDesc(&listElem);

	if (myErr = AEPutParamDesc(&aeEvent, keySelection, &fileList))
		return myErr;

	myErr = AEDisposeDesc(&fileList);

	myErr = AESend(&aeEvent, nil, kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
				kAENormalPriority, kAEDefaultTimeout, nil, nil);

	AEDisposeDesc(&aeEvent);
	
	return myErr;
}



OSErr ScheduleFinder( void )
{
	ProcessSerialNumber finderPSN;          // the finder's psn
    OSErr               myErr = noErr; 

    // Get the psn of the Finder
    if( FindAProcess(kFinderSig,kSystemType,&finderPSN) )
    	return procNotFound;

    // Wake the Finder up
    myErr = WakeUpProcess( &finderPSN );
    if(myErr) return myErr;

    // Queue the Finder for activation
    myErr = SetFrontProcess( &finderPSN );
    if(myErr) return myErr;

    return myErr;
}



OSErr FindAProcess(OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN)
{
	ProcessInfoRec	tempInfo;
	OSErr			myErr = noErr;
	Str31			processName;
	FSSpec			procSpec;
	
	processSN->lowLongOfPSN = kNoProcess;
	processSN->highLongOfPSN = kNoProcess;

	tempInfo.processInfoLength = sizeof(ProcessInfoRec);
	tempInfo.processName = (StringPtr)&processName;
	tempInfo.processAppSpec = &procSpec;

	while ((tempInfo.processSignature != creatorToFind ||
				tempInfo.processType != typeToFind) ||
				myErr != noErr) {
		myErr = GetNextProcess(processSN);
		if (myErr ==noErr)
			GetProcessInformation(processSN, &tempInfo);
	}
	return(myErr);
}

long FindControlPanel( FSSpec *resultSpec, OSType theType, short vRefNum, long dirID )
{
	OSErr			iErr;
	CSParam			pb;
	CInfoPBRec		criteria1, criteria2;
	Ptr						myBuffer;

	myBuffer = NewPtr( kBufferSize );

	// no fileName
	criteria1.hFileInfo.ioNamePtr =	criteria2.hFileInfo.ioNamePtr =	NIL;
	criteria1.hFileInfo.ioFlParID =	criteria2.hFileInfo.ioFlParID =	dirID;

	/* only	match files	(not directories) */
	criteria1.hFileInfo.ioFlAttrib = 0x00;
	criteria2.hFileInfo.ioFlAttrib = 0x10;

	// search for creator
	criteria1.hFileInfo.ioFlFndrInfo.fdCreator = theType;
	criteria1.hFileInfo.ioFlFndrInfo.fdType	= kcdevType;
	criteria2.hFileInfo.ioFlFndrInfo.fdCreator = (OSType)0xffffffff;
	criteria2.hFileInfo.ioFlFndrInfo.fdType	= (OSType)0xffffffff;

	/* zero	all	other FInfo	fields */
	criteria1.hFileInfo.ioFlFndrInfo.fdFlags = 0;
	criteria1.hFileInfo.ioFlFndrInfo.fdLocation.v =	0;
	criteria1.hFileInfo.ioFlFndrInfo.fdLocation.h =	0;
	criteria1.hFileInfo.ioFlFndrInfo.fdFldr	= 0;

	criteria2.hFileInfo.ioFlFndrInfo.fdFlags = 0;
	criteria2.hFileInfo.ioFlFndrInfo.fdLocation.v =	0;
	criteria2.hFileInfo.ioFlFndrInfo.fdLocation.h =	0;
	criteria2.hFileInfo.ioFlFndrInfo.fdFldr	= 0;

	pb.ioCompletion	= NIL;
	pb.ioNamePtr = NIL;
	pb.ioVRefNum = vRefNum;
	pb.ioMatchPtr =	resultSpec;
	pb.ioReqMatchCount = 1;
	pb.ioSearchBits	= fsSBFlAttrib + fsSBFlFndrInfo	+ fsSBFlParID;
	pb.ioSearchInfo1 = &criteria1;
	pb.ioSearchInfo2 = &criteria2;
	pb.ioSearchTime	= 0;			// exhaustive search
	pb.ioCatPosition.initialize	= 0;
	if(	myBuffer ) {
		pb.ioOptBuffer = myBuffer;
		pb.ioOptBufSize	= kBufferSize;
	}
	else {
		pb.ioOptBuffer = NIL;
		pb.ioOptBufSize	= 0L;
	}

	iErr = PBCatSearch(	&pb, FALSE );

	if(	myBuffer ) DisposePtr( myBuffer	);

	return pb.ioActMatchCount;
}
