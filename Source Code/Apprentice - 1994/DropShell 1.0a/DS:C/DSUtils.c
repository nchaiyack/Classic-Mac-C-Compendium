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
**	LDR			Leonard Rosenthol
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	12/09/91			LDR		Added the Apple event routines
**	11/24/91			LDR		Original Version
**
******************************************************************************/

#include "DSGlobals.h"
#include "DSUtils.h"

/*
	This routine is used to properly center an Alert before showing.
	
	It is per Human Interface specs by putting it in the top 1/3 of screen.
	NOTE: This same technique can be used with DLOG resources as well.
*/
#pragma segment Main
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
#pragma segment Main
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

#pragma segment Main
void GetAppName(Str255 appName)	{
	OSErr				err;
	ProcessInfoRec		info;
	ProcessSerialNumber	curPSN;

	err = GetCurrentProcess(&curPSN);
	
	info.processInfoLength = sizeof(ProcessInfoRec);	// ALWAYS USE sizeof!
	info.processName = appName;							// so it returned somewhere
	info.processAppSpec = NULL;							// I don't care!

	err = GetProcessInformation(&curPSN, &info);
}

#pragma segment Main
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
		DisposHandle((Handle)aliasH);

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

