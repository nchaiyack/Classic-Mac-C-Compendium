/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSUtils.c
**
**   Description:	Utility routines that may be useful to DropBoxes
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	SCS			Stephan Somogyi
**	LDR			Leonard Rosenthol
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Author	Description
**	---------	------	---------------------------------------------
**	23 Jun 94	LDR		Added a bunch of new routines for Marshall
**	20 Feb 94	LDR		Added some new useful File System routines
**	11 Dec 93	SCS		Universal Headers/UPPs (Phoenix 68k/PPC & PPCC)
**						Skipped System 6 compatible rev of DropShell source
**						Changed GetAppName to GetMyAppName (StdCLib conflict)
**	12/09/91	LDR		Added the Apple event routines
**	11/24/91	LDR		Original Version
**
******************************************************************************/

#include <Dialogs.h>
#include <Types.h>

#include "DSGlobals.h"
#include "DSUtils.h"

/*
	This routine is used to properly center an Alert before showing.
	
	It is per Human Interface specs by putting it in the top 1/3 of screen.
	NOTE: This same technique can be used with DLOG resources as well.
*/

void CenterAlert ( short theID ) {
	short		theX, theY;
	AlertTHndl	theAlertHandle;
	Rect		screen, alrt;
	
	theAlertHandle = (AlertTHndl) GetResource ( 'ALRT', theID );
	if ( theAlertHandle != NULL ) {
		HLock ((Handle) theAlertHandle );

		alrt = (*theAlertHandle)->boundsRect;
		screen = qd.screenBits.bounds;
		
		theX = (( screen.right - screen.left ) - (alrt.right - alrt.left )) >> 1;
		theY = (( screen.bottom - screen.top ) + GetMBarHeight () - (alrt.bottom - alrt.top)) >> 1;
		theY -= ( screen.bottom - screen.top ) >> 2;	/* this moves it up for better viewing! */
		OffsetRect ( &(*theAlertHandle)->boundsRect, theX - alrt.left, theY - alrt.top );
	}
		
	SetCursor ( &qd.arrow );	// change this for code resources!
}

/*
	This routine is just a quick & dirty error reporter
*/

void ErrorAlert ( short stringListID, short stringIndexID, short errorID ) {
	#define	kAlertID	200
	Str255	param, errorStr;

	NumToString ( errorID, errorStr );
	GetIndString ( param, stringListID, stringIndexID );
	ParamText ( param,  errorStr, NULL, NULL );
	CenterAlert ( kAlertID );
	(void) Alert ( kAlertID, NULL );
}

/*** These routines use the Process Manager to give you information about yourself ***/


void GetMyAppName(Str255 appName)	{
	OSErr				err;
	ProcessInfoRec		info;
	ProcessSerialNumber	curPSN;

	err = GetCurrentProcess(&curPSN);
	
	info.processInfoLength = sizeof(ProcessInfoRec);	// ALWAYS USE sizeof!
	info.processName = appName;							// so it returned somewhere
	info.processAppSpec = NULL;							// I don't care!

	err = GetProcessInformation(&curPSN, &info);
}


void GetAppFSSpec(FSSpec *appSpec)	{
	OSErr				err;
	Str255				appName;
	ProcessInfoRec		info;
	ProcessSerialNumber	curPSN;

	err = GetCurrentProcess(&curPSN);
	
	info.processInfoLength = sizeof(ProcessInfoRec);	// ALWAYS USE sizeof!
	info.processName = appName;							// so it returned somewhere
	info.processAppSpec = appSpec;						// so it can get returned!

	err = GetProcessInformation(&curPSN, &info);
}

/* ��� File Routines begin here ��� */
/*
	This routine is used to force the Finder (as much as is possible) to update
	information about a newly changed file or folder.  
	It does this by changing the modification date of the surrounding folder.
*/
OSErr ForceFinderUpdate(FSSpec *pFSS, Boolean flush)
{
	OSErr			lErr;
	CInfoPBRec		lCBlk;
	
	if (pFSS->parID != 1)							// if it's a vol then reuse the NameStr
		lCBlk.dirInfo.ioNamePtr = 0L;
	lCBlk.dirInfo.ioVRefNum = pFSS->vRefNum;		
	lCBlk.dirInfo.ioDrDirID = pFSS->parID;
	lCBlk.dirInfo.ioFDirIndex = 0;
	lCBlk.dirInfo.ioCompletion = 0;

	lErr = PBGetCatInfoSync(&lCBlk);
	if (!lErr) {
		GetDateTime(&lCBlk.dirInfo.ioDrMdDat);
		lCBlk.dirInfo.ioDrDirID = pFSS->parID;
		lErr = PBSetCatInfoSync(&lCBlk);
	
		if ((!lErr) && (flush))
			lErr = FlushVol(nil, pFSS->vRefNum);
	}
	
	return (lErr);
}

/*	Is the file in use (busy)? */
Boolean FSpIsBusy(FSSpecPtr theFile)
{
	Boolean		isBusy = false;
	OSErr		err;
	CInfoPBRec	cipb;
	
	cipb.hFileInfo.ioCompletion	= 0L;
	cipb.hFileInfo.ioNamePtr	= theFile->name;
	cipb.hFileInfo.ioVRefNum	= theFile->vRefNum;
	cipb.hFileInfo.ioFDirIndex	= 0;
	cipb.hFileInfo.ioDirID		= theFile->parID;
	err = PBGetCatInfoSync(&cipb);
	if (!err) {
		isBusy = (cipb.hFileInfo.ioFlAttrib & 0x80) == 0x80;	// bit 7 = either fork open
	}
	return(isBusy);
}

/*	Is the �file� represented by this FSSpec really a folder? */
Boolean	FSpIsFolder (FSSpecPtr theFSSpec) 
{
	OSErr	err;
	CInfoPBRec	pb;
	Str255	fName;
	
	if (theFSSpec->parID == fsRtParID)	// it's a volume!!
		return(true);
		
	BlockMoveData (theFSSpec->name, fName, 32);
	pb.hFileInfo.ioDirID		= theFSSpec->parID;
	pb.hFileInfo.ioCompletion	= NULL;
	pb.hFileInfo.ioNamePtr		= fName;
	pb.hFileInfo.ioVRefNum		= theFSSpec->vRefNum;
	pb.hFileInfo.ioFDirIndex	= 0;
	pb.hFileInfo.ioFVersNum		= 0;
	err = PBGetCatInfoSync(&pb);
	
	if (!err) {
		if (pb.hFileInfo.ioFlAttrib & ioDirMask)
			return(true);
	}
	return(false);
}

// Creates an empty handle to which FSSpecs could be added later!
FSSpecArrayHandle	NewFSSpecList(void)
{
	return((FSSpecArrayHandle)NewHandle(0));
}

// Releases memory used by an FSSpecList when all done
void DisposeFSSpecList(FSSpecArrayHandle fsList)
{
	DisposeHandle((Handle)fsList);
}

// Adds the new FSSpec to the end of an FSSpecList
void AddToFSSpecList(FSSpec *fSpec, FSSpecArrayHandle fileList)
{
	Size	curSize, newSize;
	long	numFiles;

	if ((!fileList) || (!fSpec)) return;	// if either is bogus, get out fast
	
	curSize = GetHandleSize((Handle)fileList);
	numFiles = curSize / sizeof(FSSpec);
	newSize = curSize + sizeof(FSSpec);
	SetHandleSize((Handle)fileList, newSize);
	if (MemError())	return;		// any problems, get out
	
	BlockMove(fSpec, &(*fileList)[numFiles], sizeof(FSSpec));
}


/* ��� Apple event routines begin here ��� */

/*
	This routine will create a targetDesc for sending to self.

	We take IM VI's advice and use the typePSN form with 
	kCurrentProcess as the targetPSN.
*/
OSErr GetTargetFromSelf (AEAddressDesc *targetDesc)
{
	ProcessSerialNumber	psn;

	psn.highLongOfPSN 	= 0;
	psn.lowLongOfPSN 	= kCurrentProcess;

	return( AECreateDesc(typeProcessSerialNumber, (Ptr)&psn, sizeof(ProcessSerialNumber), targetDesc) );
}

/* This routine will create a targetDesc using the apps signature */
OSErr GetTargetFromSignature (OSType processSig, AEAddressDesc *targetDesc)
{
	return( AECreateDesc(typeApplSignature, (Ptr)&processSig, sizeof(processSig), targetDesc) );
}

/* This routine will create a targetDesc by bringing up the PPCBrowser */
OSErr GetTargetFromBrowser(Str255 promptStr, AEAddressDesc *targetDesc)
{
	OSErr		err;
	TargetID	theTarget;
	PortInfoRec	portInfo;
	
	err = PPCBrowser(promptStr, "\p", false, &theTarget.location, &portInfo, NULL, "\p");
	if (err == noErr) {
		theTarget.name = portInfo.name;
		err = AECreateDesc(typeTargetID, (Ptr)&theTarget, sizeof(TargetID), targetDesc);
	}
	return( err );
}


/*
	This routine is the low level routine used by the SendODOCToSelf
	routine.  It gets passed the list of files (in an AEDescList)
	to be sent as the data for the 'odoc', builds up the event
	and sends off the event.  

	It is broken out from SendODOCToSelf so that a SendODOCListToSelf could
	easily be written and it could then call this routine - but that is left
	as an exercise to the reader.
	
	Read the comments in the code for the order and details
*/
void _SendDocsToSelf (AEDescList *aliasList)
{
	OSErr			err;
	AEAddressDesc	theTarget;
	AppleEvent		openDocAE, replyAE;

/*
	First we create the target for the event.   We call another
	utility routine for creating the target.
*/
	err = GetTargetFromSelf(&theTarget);
	if (err == noErr) {
		/* Next we create the Apple event that will later get sent. */
		err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &theTarget, kAutoGenerateReturnID, kAnyTransactionID, &openDocAE);

		if (err == noErr) {
			/* Now add the aliasDescList to the openDocAE */
			err = AEPutParamDesc(&openDocAE, keyDirectObject, aliasList);

			if (err == noErr) {
				/*
					and finally send the event
					Since we are sending to ourselves, no need for reply.
				*/
				err = AESend(&openDocAE, &replyAE, kAENoReply + kAECanInteract, kAENormalPriority, 3600, NULL, NULL);

				/*
					NOTE: Since we are not requesting a reply, we do not need to
					need to dispose of the replyAE.  It is there simply as a 
					placeholder.
				*/
			}

		/*	
			Dispose of the aliasList descriptor
			We do this instead of the caller since it needs to be done
			before disposing the AEVT
		*/
			err = AEDisposeDesc(aliasList);
		}

	/*and of course dispose of the openDoc AEVT itself*/
		err = AEDisposeDesc(&openDocAE);
	}
}

/*
	This is the routine called by SelectFile to send a single odoc to ourselves.
	
	It calls the above low level routine to do the dirty work of sending the AEVT -
	all we do here is build a AEDescList of the file to be opened.
*/
void SendODOCToSelf (FSSpec *theFileSpec) {

	OSErr		err;
	AEDescList	aliasList;
	AEDesc		aliasDesc;
	AliasHandle	aliasH;
	
	/*Create the descList to hold the list of files*/
	err = AECreateList(NULL, 0, false, &aliasList);

	if (err == noErr) {
		/* First we setup the type of descriptor */
		aliasDesc.descriptorType = typeAlias;

		/*
			Now we add the file to descList by creating an alias and then
			adding it into the descList using AEPutDesc
		*/
		err = NewAlias(NULL, theFileSpec, &aliasH);
		aliasDesc.dataHandle = (Handle)aliasH;
		err = AEPutDesc(&aliasList, 0, &aliasDesc);
		DisposeHandle((Handle)aliasH);

		/*Now call the real gut level routine to do the dirty work*/
		_SendDocsToSelf(&aliasList);

		/*_SendDocsToSelf will dispose of aliasList for me*/
	}
}


void SendQuitToSelf (void)
{
	OSErr			err;
	AEAddressDesc	theTarget;
	AppleEvent		quitAE, replyAE;

/*
	First we create the target for the event.   We call another
	utility routine for creating the target.
*/
	err = GetTargetFromSelf(&theTarget);
	if (err == noErr) {
		/* Next we create the Apple event that will later get sent. */
		err = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &theTarget, kAutoGenerateReturnID, kAnyTransactionID, &quitAE);

		if (err == noErr) {
			/*
				and finally send the event
				Since we are sending to ourselves, no need for reply.
			*/
			err = AESend(&quitAE, &replyAE, kAENoReply + kAECanInteract, kAENormalPriority, 3600, NULL, NULL);

			/*
				NOTE: Since we are not requesting a reply, we do not need to
				need to dispose of the replyAE.  It is there simply as a 
				placeholder.
			*/
		}

		/* and of course dispose of the quit AEVT itself */
		err = AEDisposeDesc(&quitAE);
	}
}

