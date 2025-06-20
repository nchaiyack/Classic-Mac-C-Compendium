/* ========== the commmand file: ==========

	odoc.c
		
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <appleevents.h>
#include <processes.h>
#include <Aliases.h>
#include <GestaltEqu.h>

#include "nshc.h"

#include "arg_utl.proto.h"
#include "str_utl.proto.h"
#include "fss_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// prototypes

int		Pre7( void );
int		is_it_running( OSType targetSignature );
OSErr	odoc( int gotFSSpec, FSSpec *doc_fss, OSType app_sig, t_nshc_calls *nshc_calls );
int		SearchOne( short vol, OSType creator, FSSpec *app_fss );
int		SearchAllButOne( Boolean remote_search, OSType creator, FSSpec *doc_fss, FSSpec *app_fss );
int		find_app( OSType creator, FSSpec *doc_fss, FSSpec *app_fss );
OSErr	launch( int gotFSSpec, FSSpec *launchApp, FSSpec *launchDoc, t_nshc_calls *nshc_calls );

/* ======================================== */

// return true if 'sysv' is previous to 7.0

int	Pre7( void )
{
	OSErr	error;
	long	response;
	
	if ( error = Gestalt( 'sysv', &response ) )
		return(true);
		
	if ( response < 0x700 )
		return(true);
	else
		return(false);
}

/* ======================================== */

int	is_it_running( OSType targetSignature )
{
	Str255					name;
	ProcessSerialNumber		psn_p;
    ProcessInfoRec  		pi;
		    
    pi.processName       = name;
    pi.processInfoLength = sizeof(pi);
    pi.processAppSpec    = NULL;
    
	psn_p.highLongOfPSN = 0;
	psn_p.lowLongOfPSN  = kNoProcess;
	    
	while(!GetNextProcess(&psn_p))
		if(!GetProcessInformation(&psn_p,&pi))
			if ( pi.processSignature == targetSignature )
				return( 1 );

	return( 0 );
}

/* ======================================== */

OSErr odoc( int gotFSSpec, FSSpec *doc_fss, OSType app_sig, t_nshc_calls *nshc_calls )
{
	OSErr			myErr;
    FInfo			finfo;
    int				appRunning;
	AEDesc			appDesc;
	AliasHandle		docAlias;
	AppleEvent		openAE;
	AppleEvent		replyAE;
	AEDescList		docList;
	
	// initialize
	
	openAE.dataHandle = nil;
	replyAE.dataHandle = nil;
	appDesc.dataHandle = nil;
	docList.dataHandle = nil;
	
	docAlias = nil;
			
    // see if the app is running
    
    appRunning = is_it_running( app_sig );
    
    if (!appRunning) {
		nshc_calls->NSH_putStr_err("\podoc: Target process is not running\r");
		return(1);
		}

	myErr = AECreateDesc(typeApplSignature,
						 (Ptr)&app_sig,
						 sizeof(app_sig),
						 &appDesc);
	if (myErr) goto Cleanup;	
						 
	myErr = NewAlias(nil,doc_fss,&docAlias);
	if (myErr) goto Cleanup;	

	myErr = AECreateAppleEvent(kCoreEventClass,
							   kAEOpenDocuments,
							   &appDesc,
							   kAutoGenerateReturnID,
							   kAnyTransactionID,
							   &openAE);
	if (myErr) goto Cleanup;	

	myErr = AECreateList(nil,0,false,&docList);
	if (myErr) goto Cleanup;	

	HLock((Handle)docAlias);
	myErr = AEPutPtr(&docList,
					 0,
					 typeAlias,
					 (Ptr)*docAlias,
					 sizeof(AliasHandle)+(**docAlias).aliasSize);
	HUnlock((Handle)docAlias);
	if (myErr) goto Cleanup;	

	myErr = AEPutParamDesc(&openAE,keyDirectObject,&docList);
	if (myErr) goto Cleanup;
	
	myErr = AESend(&openAE,&replyAE,kAENoReply,kAENormalPriority,kAEDefaultTimeout,nil,nil);
		
Cleanup:

	if (myErr) nshc_calls->NSH_putStr_err("\podoc: Message could not be sent.\r");
	
	if (docList.dataHandle) AEDisposeDesc(&docList);
	if (openAE.dataHandle) AEDisposeDesc(&openAE);
	if (replyAE.dataHandle) AEDisposeDesc(&replyAE);
	if (appDesc.dataHandle) AEDisposeDesc(&appDesc);
	
	if (docAlias) DisposHandle((Handle)docAlias);
	
	return(myErr);
}

/* ======================================== */

OSErr launch( int gotFSSpec, FSSpec *launchApp, FSSpec *launchDoc, t_nshc_calls *nshc_calls )
{	
	AEDesc				docDesc;
	AEDesc				parmDesc;
	AEDesc				adrDesc;
	AEDescList			docList;
	AliasHandle			docAlias;
	AppleEvent			fakeEvent;
	AppParametersPtr	docParm;
	LaunchParamBlockRec	launchThis;
	OSErr				myErr;
	ProcessSerialNumber myPSN;
	
	// initialize
	
	docDesc.dataHandle = nil;
	parmDesc.dataHandle = nil;
	adrDesc.dataHandle = nil;
	docList.dataHandle = nil;
	docAlias = nil;
	fakeEvent.dataHandle = nil;

	GetCurrentProcess(&myPSN);		// used to initialize new AE things
	
	// create the address desc for the event
	
    myErr = AECreateDesc(typeProcessSerialNumber, (Ptr)&myPSN, sizeof(ProcessSerialNumber), &adrDesc);
	if (myErr) goto Cleanup;

	// stuff it in my launch parameter block
	
    launchThis.launchAppSpec = launchApp;
    
	if (launchDoc) {
	    
		// create an apple event for "open doc"
		myErr = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &adrDesc, kAutoGenerateReturnID, kAnyTransactionID, &fakeEvent);
		if (myErr) goto Cleanup;

		// create a list to hold the doc alias
    	myErr = AECreateList(nil, 0, false, &docList);
		if (myErr) goto Cleanup;
    
		// ae needs an alias to the doc
    	myErr = NewAlias(nil, launchDoc, &docAlias);
		if (myErr) goto Cleanup;
 
		// and a descriptor for the alias
    	HLock((Handle)docAlias);
    	myErr = AECreateDesc(typeAlias, (Ptr)*docAlias, GetHandleSize((Handle)docAlias), &docDesc);
    	HUnlock((Handle)docAlias);
		if (myErr) goto Cleanup;
    
		// the descriptor goes in the list
    	myErr = AEPutDesc(&docList, 0, &docDesc);
		if (myErr) goto Cleanup;
		
		// and the list goes into the event
    	myErr = AEPutParamDesc(&fakeEvent, keyDirectObject, &docList);
		if (myErr) goto Cleanup;
    
    	// but the fake event is REALLY a parameter!
		myErr = AECoerceDesc(&fakeEvent, typeAppParameters, &parmDesc);
		if (myErr) goto Cleanup;
		
		docParm = (AppParametersPtr)*(parmDesc.dataHandle);
		}
	else
		docParm = nil;
    
    // stuff the LaunchParamBlockRec and send it
    
	HLock((Handle)fakeEvent.dataHandle);
	
    launchThis.launchAppParameters = docParm;
    launchThis.launchBlockID = extendedBlock;
    launchThis.launchEPBLength = extendedBlockLen;
    launchThis.launchFileFlags = 0;
    launchThis.launchControlFlags = launchContinue + launchNoFileFlags;
    
    myErr = LaunchApplication(&launchThis);
    
	HUnlock((Handle)fakeEvent.dataHandle);
	
Cleanup:

	if (myErr) nshc_calls->NSH_putStr_err("\plaunch: Message could not be sent.\r");
	
	if ( docDesc.dataHandle ) AEDisposeDesc(&docDesc);
	if ( parmDesc.dataHandle ) AEDisposeDesc(&parmDesc);
	if ( adrDesc.dataHandle ) AEDisposeDesc(&adrDesc);
	if ( docList.dataHandle ) AEDisposeDesc(&docDesc);
	if ( fakeEvent.dataHandle )AEDisposeDesc(&fakeEvent);

	if ( docAlias ) DisposeHandle((Handle)docAlias);

	return(myErr);
}

/* ======================================== */

int SearchOne( short vol, OSType creator, FSSpec *app_fss )
{
	DTPBRec		desktopParams;
	OSErr		myErr;
	int			found;
	
	found = 0;

	desktopParams.ioVRefNum = vol;
	desktopParams.ioNamePtr = nil;
	
	myErr = PBDTGetPath(&desktopParams);
	
	if (!myErr && (desktopParams.ioDTRefNum != 0)) {
	
		desktopParams.ioIndex = 0;
		desktopParams.ioFileCreator = creator;
		desktopParams.ioNamePtr = app_fss->name;
		
		myErr = PBDTGetAPPLSync(&desktopParams);
		
		if (!myErr) {
		
			// okay, found it; fill in the application file spec
			// and set the flag indicating we're done
			
			app_fss->parID = desktopParams.ioAPPLParID;
			app_fss->vRefNum = desktopParams.ioVRefNum;
			found = 1;
			
		}
	}
	
	return( found );	
}

int SearchAllButOne( Boolean remote_search, OSType creator, FSSpec *doc_fss, FSSpec *app_fss )
{
	HParamBlockRec			hfsParams;
	FInfo					documentFInfo;
	short					volumeIndex;
	int						found;
	GetVolParmsInfoBuffer	volumeInfoBuffer;
	OSErr					myErr;
	
	found = 0;
	volumeIndex = 0;
	
	while (!found) {
		
		volumeIndex++;
			
		// convert the volumeIndex into a vRefNum
		
		hfsParams.volumeParam.ioNamePtr = nil;
		hfsParams.volumeParam.ioVRefNum = 0;
		hfsParams.volumeParam.ioVolIndex = volumeIndex;
		myErr = PBHGetVInfoSync(&hfsParams);
		
		// a nsvErr indicates that the current pass is over
		if (myErr) return( 0 );
		
		// since we handled the document volume during the documentPass,
		// skip it if we have hit that volume again
		
		if (hfsParams.volumeParam.ioVRefNum != doc_fss->vRefNum) {
		
			// call GetVolParms to determine if this volume is a server
			// (a remote volume)
			
			hfsParams.ioParam.ioBuffer = (Ptr) &volumeInfoBuffer;
			hfsParams.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
			myErr = PBHGetVolParmsSync(&hfsParams);
			
			if (myErr) return( 0 );
			
			// if the vMServerAdr field of the volume information buffer
			// is zero, this is a local volume; skip this volume
			// if it's local on a remote pass or remote on a local pass
			
			if ( ( volumeInfoBuffer.vMServerAdr == 0 ) != remote_search )
				found = SearchOne( hfsParams.volumeParam.ioVRefNum, creator, app_fss );
				
			}
				
	}
	
	return( found );
}

/* ======================================== */

int find_app( OSType creator, FSSpec *doc_fss, FSSpec *app_fss )
{
	OSErr	found;
	
	found = SearchOne( doc_fss->vRefNum, creator, app_fss );
	
	if (!found)
		found = SearchAllButOne( 0, creator, doc_fss, app_fss );

	if (!found)
		found = SearchAllButOne( 1, creator, doc_fss, app_fss );
		
	return(found);
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	FInfo	finfo;
	FSSpec	app_fss;
	FSSpec	doc_fss;
	int		gotFSSpec;
	int		result;
	OSType	creator;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	nshc_parms->action = nsh_idle;		// always one pass to this command
	gotFSSpec = fss_test();				// find out if fss is there
	
	// *** reason not to run 1 - bad included version
	
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) {
		result = NSHC_ERR_VERSION;
		goto Cleanup;
		}
	
	// *** reason not to run 2 - bad os version
	
	if ( Pre7() ) {
		nshc_calls->NSH_putStr_err("\podoc: This command requires System 7.\r");
		result = NSHC_ERR_GENERAL;
		goto Cleanup;
		}
	    
	// *** reason not to run 3 - bad parameters
	
	if (( nshc_parms->argc < 2 ) || ( nshc_parms->argc > 3 )) {
		nshc_calls->NSH_putStr_err("\pUsage: odoc doc_path [app_path]\r");
		result = NSHC_ERR_PARMS;
		goto Cleanup;
		}
		
	// *** reason not to run 4 - can't find doc
	
	result = arg_to_fss( nshc_parms, nshc_calls, 1, &doc_fss );

	if (!result)
	    result = fss_GetFInfo( gotFSSpec, &doc_fss, &finfo );

	if (result) {
		nshc_calls->NSH_putStr_err("\podoc: Document not found.\r");
		result = NSHC_ERR_PARMS;
		goto Cleanup;
		}
		
	creator = finfo.fdCreator;		// if nothing else, use doc's creator
					
	// *** reason not to run 5 - can't find app
	
	if ( nshc_parms->argc > 2 ) {
	
		result = arg_to_fss( nshc_parms, nshc_calls, 2, &app_fss );
		
		if (!result)
		    result = fss_GetFInfo( gotFSSpec, &app_fss, &finfo );
	
		if (result || ( finfo.fdType != 'APPL' ) ) {
			nshc_calls->NSH_putStr_err("\podoc: Application not found.\r");
			result = NSHC_ERR_PARMS;
			goto Cleanup;
			}
			
		creator = finfo.fdCreator;		// if available, use apps's creator
					
		}
		
	// see if we should open or launch
	
	if (is_it_running( creator )) {
		
		// *** finally try to open the thing
			
		result = odoc( gotFSSpec, &doc_fss, creator, nshc_calls );

		}
	
	else {
	
		// *** or else try to launch it
			
		if ( nshc_parms->argc < 3 )
			if ( !find_app( creator, &doc_fss, &app_fss ) ) {
				nshc_calls->NSH_putStr_err("\podoc: Document creator not found.\r");
				result = NSHC_ERR_PARMS;
				goto Cleanup;
				}
		
		result = launch( gotFSSpec, &app_fss, &doc_fss, nshc_calls );

		}

	// *** log the result and return
	
Cleanup:
			
	nshc_parms->result = result;

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#else
	;					// Think needs a ; to go with the Cleanup label
#endif
}
