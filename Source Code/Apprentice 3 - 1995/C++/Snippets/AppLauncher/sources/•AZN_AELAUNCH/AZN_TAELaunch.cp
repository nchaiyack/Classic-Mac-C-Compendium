/*
*	AZN_TAELaunch.cp
*
*	Static Methods for 'TAELaunch' class
*	A class to locate and launch from the Finder
*	application(s) of the desired kind
*	( Based on Pascal code by Peter Lewis, [ peter@kagi.com ] )
*	( Converted to C++ by Gilles Dignard [ gdignard@hookup.net ] )
*	This version by Andrew Nemeth [ azn@nemeng.mpx.com.au ]
*
*	File Created:		21 Feb 95
*	File Ammended:		21 Feb;
*					4�9, 12�14 Mar 95.
*/

#include	"AZN_Debug.h"									//	debugging utilites

#include	"AZN_TAELaunch.h"								//	class to remotely launch apps
#include	"FinderRegistry.h"								//	finder objects

#include	<AppleEvents.h>
#include	<AEPackObject.h>								//	the object specifier building utilities
#include	<AERegistry.h>
#include	<AEObjects.h>									//	mac OSL constants
#include	<Processes.h>									//	mac process manager

#include	<Aliases.h>
#include	<Folders.h>
#include	<Gestalt.h>


//	FILE DEFINES�
//
const OSType	kostypeApp 		= 'APPL',
			kostypeCdev 		= 'cdev',
			kostypeCreatorMac 	= 'MACS',
			kostypeFinder 		= 'FNDR';


//	FILE FUNCTIONS�
//
static void		buildVisibilityOSPEC( Str63, AEDesc * );
inline Boolean	bTST( long lgIn, short shBit )
				{
					return( (lgIn >> shBit) & 1 );
				}




OSErr	TAELaunch::findApplication( OSType ostypeCreator, FSSpecPtr ptrfsspecA )
//
//	Given an application's creator OSType,
//	search all mounted volumes (by looking in 
//	desktop database) until a matching app on
//	disc is found.  Then load file's FSSpec into
//	FSSpecPtr provided.  Return OSErr as to
//	success or otherwise
//
{
	short 		i = 0;
	VolumeParam	vparamRec;
	DTPBRec 		pbdt;
	long 		lgBytesFree = 0L;
	OSErr 		myErr = noErr;
	Boolean 		boolFoundIt = FALSE;
	long 		lgFeature = 0L;
	
//														idiot filters
	ASSERT( ptrfsspecA != NULL );
	ASSERT( ostypeCreator > 0L );
	SALT_MEMORY( &vparamRec, sizeof( VolumeParam ) );
	SALT_MEMORY( &pbdt, sizeof( DTPBRec ) );
//														check for system 7
	myErr = ::Gestalt( gestaltSystemVersion, &lgFeature );
	if ( myErr == noErr && ( lgFeature >= 0x0700 ) ) 
		{
		vparamRec.ioCompletion = NULL;
		vparamRec.ioNamePtr = ptrfsspecA->name;
//														cycle thru all mounted vols
		do {
			vparamRec.ioVRefNum = 0;
			vparamRec.ioVolIndex = ++i;
	
			myErr = ::PBGetVInfoSync( (ParmBlkPtr)&vparamRec );
//														grab desktop database
			if ( myErr == noErr ) 
				{
				ptrfsspecA->name[0] = 0;
				pbdt.ioNamePtr = ptrfsspecA->name;
				pbdt.ioVRefNum = vparamRec.ioVRefNum;
				myErr = ::PBDTGetPath( &pbdt );
//														search desktop for app
				if ( myErr == noErr ) 
					{
					pbdt.ioIndex = 0;
					pbdt.ioFileCreator = ostypeCreator;
					myErr = ::PBDTGetAPPLSync( &pbdt );
					if ( myErr == noErr )
						boolFoundIt = TRUE;
					}
				myErr = noErr;
				}
			} while ( ! boolFoundIt && ( myErr == noErr ) );
		}
//														store data in fsspec
	if ( boolFoundIt ) 
		{
		myErr = noErr;
		ptrfsspecA->vRefNum = pbdt.ioVRefNum;
		ptrfsspecA->parID = pbdt.ioAPPLParID;
		//	N.B. 'name' was set above during assignment
		//	'pbdt.ioNamePtr = ptrfsspecA->name'
		}
//														report error condition
	else
		{
		myErr = afpItemNotFound;
		ptrfsspecA->vRefNum = 0;
		ptrfsspecA->parID = 2;
		ptrfsspecA->name[0] = 0;
		}

	return( myErr );
}



Boolean	TAELaunch::openItemInFinder( const FSSpec & fsspecITEM, 
								Boolean 		boolToFront )
//
///	Send in an FSSpec of an item you
//	wish the finder to launch.  Set
//	'boolToFront' to bring it to the front(!)
//	Returns TRUE if everything went okay
//
{
	AEDesc				aedescTarget 		= { typeNull, NULL };
	AEDescList			aedesclistFiles 	= { typeNull, NULL };
	AppleEvent			aeEvent 			= { typeNull, NULL },
						aeReply 			= { typeNull, NULL };
	AESendMode			aesendMode;
	long					lgFeature 		= 0L;
	FSSpec				fsspecOrig, 
						fsspecFinder;
	OSType				ostypeSig 		= 0L;
	EventRecord			erRec;
	ProcessSerialNumber		psnProcess, 
						psnFndr;
	OSErr				myErr 			= noErr;
	
//														use Gestalt to check for OSL compliance
	myErr = ::Gestalt( gestaltFinderAttr, &lgFeature );
	if ( ( myErr == noErr ) && bTST( lgFeature, gestaltOSLCompliantFinder ) )
		NULL;
	else
		return( FALSE );
//														if in background�
	if ( ! boolToFront )
		{
		switch( scanProcesses( fsspecITEM, &ostypeSig, &psnProcess ) )
			{
			case kenumError:
				return( FALSE );
				break;
//														grab FSSpec of CURRENT application
			case kenumNone:
				if ( ! grabCurrentFSSpec( &fsspecOrig ) )
					return( FALSE );
				break;
//														open item in already running process
			case kenumSimilar:
				return( openDocInApp( ostypeSig, fsspecITEM, boolToFront ) );
				break;
//														do nothing, already running!
			case kenumIdentical:
				return( TRUE );
				break;
			}
		}
//														grab process for finder
	if ( findProcess( kostypeCreatorMac, kostypeFinder, &psnFndr, &fsspecFinder ) )
		{
//														prepare 'odoc' AE for finder
		myErr = ::AECreateDesc( typeProcessSerialNumber,
							&psnFndr, sizeof( ProcessSerialNumber ),
							&aedescTarget );
		if (myErr == noErr)
			myErr = ::AECreateAppleEvent(	kCoreEventClass,
									kAEOpenDocuments,
									&aedescTarget,
									kAutoGenerateReturnID,
									kAnyTransactionID,
									&aeEvent );
//														prepare FSSpec's to send to finder
		if ( myErr == noErr )
			myErr = ::AECreateList( nil, 0, FALSE, &aedesclistFiles );
		
		if ( myErr == noErr )
			addFSSToAEList( &aedesclistFiles, 1, fsspecITEM );
		
		if ( myErr == noErr )
			myErr = ::AEPutParamDesc( &aeEvent, keyDirectObject, &aedesclistFiles );
//														send AE to finder
		if (myErr == noErr) 
			{
			aesendMode = kAENoReply + kAECanSwitchLayer;
			aesendMode += ( boolToFront ) ? kAEAlwaysInteract : kAENeverInteract;

			myErr = ::AESend(	&aeEvent,
							&aeReply,
							aesendMode,
							kAENormalPriority,
							kAEDefaultTimeout,
							nil, nil);
			ASSERT( myErr == noErr );
			}
//														tidy up
		::AEDisposeDesc( &aeEvent );
		::AEDisposeDesc( &aeReply );
		::AEDisposeDesc( &aedesclistFiles );
		::AEDisposeDesc( &aedescTarget );			
//														make sure new process fires up
		::EventAvail( osEvt, &erRec );
//														make new process foreground/ background�
		if ( myErr == noErr )
			{
			if ( boolToFront ) 
				{
//														need to find PSN of process just launched!
				switch( scanProcesses( fsspecITEM, &ostypeSig, &psnProcess ) )
					{
					case kenumError:
						return( FALSE );
						break;
//														control panel?, put finder in front!
					case kenumNone:
						::SetFrontProcess( &psnFndr );
						break;
//														bring anything else to front
					case kenumIdentical:
					case kenumSimilar:
						::SetFrontProcess( &psnProcess );
						break;
					}
				}
//	if required, send target app to back
//	by bringing back original application!
//	A kludge, but it works!
			else
				return( openItemInFinder( fsspecOrig, TRUE ) );
			}
		}

	return( myErr == noErr );
}



Boolean	TAELaunch::openDocInApp( OSType 			ostypeSig, 
							const FSSpec &	fsspecDoc, 
							Boolean			boolToFront )
//
//	ASSUMES THAT APP IS ALREADY RUNNING!
//
//	Send in the OSType of an app. and
//	the FSSpec of a document you wish to
//	open in it, return TRUE if success
//
{
	const long			klgHeapNeeded = 4096L;

	AEDesc 				aedescTarget 		= { typeNull, NULL };
	AppleEvent 			aeEvent 			= { typeNull, NULL },
						aeReply 			= { typeNull, NULL };
	AEDescList 			aedesclistFiles 	= { typeNull, NULL };
	AESendMode 			aesendMode;
	ProcessSerialNumber 	psnProcess;
	FSSpec 				fsspecApp;
	long 				lgFeature = 0L,
						lgTotHeapSize = 0L,
						lgContigHeapSize = 0L;
	OSErr 				myErr = noErr;

//														idiot filter
	ASSERT( ostypeSig > 0L );
//														make sure we've got enough heap to play with!
	::PurgeSpace( &lgTotHeapSize, &lgContigHeapSize );
	if ( lgContigHeapSize <= klgHeapNeeded )
		return( FALSE );
//														check gestalt to see if we can launch
	myErr = ::Gestalt( gestaltOSAttr, &lgFeature );
	if ( ( myErr == noErr ) && bTST( lgFeature, gestaltLaunchControl ) )
		NULL;
	else
		return( FALSE );
//														find app. process and launch with doc
	if ( findProcess( ostypeSig, kostypeApp, &psnProcess, &fsspecApp ) ) 
		{
//														check to see if item isn't already open
		if ( docOpenable( fsspecDoc ) )
			{
//														create AE for target application
			myErr = ::AECreateDesc( typeProcessSerialNumber, &psnProcess, 
								sizeof(psnProcess), &aedescTarget );
	
			myErr = ::AECreateAppleEvent(	kCoreEventClass,
									kAEOpenDocuments,
									&aedescTarget,
									kAutoGenerateReturnID,
									kAnyTransactionID,
									&aeEvent );
//														prepare AEDesc(s) for doc to be launched
			myErr = ::AECreateList( nil, 0, false, &aedesclistFiles );
			addFSSToAEList( &aedesclistFiles, 1, fsspecDoc );
			myErr = ::AEPutParamDesc( &aeEvent, keyDirectObject, &aedesclistFiles );
			aesendMode = kAENoReply;
			if ( ! boolToFront )
				aesendMode += kAENeverInteract;
//														send 'odoc' AE to application
			myErr = ::AESend(	&aeEvent,
							&aeReply,
							aesendMode,
							kAEHighPriority,
							kNoTimeOut,
							nil, nil);
			ASSERT( myErr == noErr );
//														tidy up
			myErr = ::AEDisposeDesc( &aedescTarget );
			myErr = ::AEDisposeDesc( &aeEvent );
			myErr = ::AEDisposeDesc( &aeReply );
			myErr = ::AEDisposeDesc( &aedesclistFiles );
			}
//														if required, bring app to front
		if ( boolToFront )
			myErr = ::SetFrontProcess( &psnProcess );
		}
//														application is not running!
	else
		myErr = afpItemNotFound;

	return( myErr == noErr );
}



void		TAELaunch::quitRemoteApp( OSType ostypeSig )
//
//	Send quit AE to a running application
//
{
	const long			klgTimeOut = 5L * 60L;

	AEAddressDesc 			aedescTarget 		= { typeNull, NULL };
	AppleEvent 			aeEvent 			= { typeNull, NULL },
			 			aeReply 			= { typeNull, NULL };
	ProcessSerialNumber 	psnProcess;
	FSSpec 				fsspecApp;
	OSErr 				myErr = noErr;
	
//														idiot filter
	ASSERT( ostypeSig > 0L );
//														find app and shut it down
	if ( findProcess( ostypeSig, kostypeApp, &psnProcess, &fsspecApp ) ) 
		{
		myErr = ::AECreateDesc(	typeProcessSerialNumber,
								&psnProcess,
								sizeof( psnProcess ),
								&aedescTarget );
		myErr = ::AECreateAppleEvent(	kCoreEventClass,
								kAEQuitApplication,
								&aedescTarget,
								kAutoGenerateReturnID,
								kAnyTransactionID,
								&aeEvent );
//														send 'quit' AE
		myErr = ::AESend(	&aeEvent,
						&aeReply,
						kAENoReply,
						kAEHighPriority,
						klgTimeOut,
						nil, nil );
		ASSERT( myErr == noErr );
//														tidy up
		myErr = ::AEDisposeDesc(&aedescTarget);
		myErr = ::AEDisposeDesc( &aeEvent );
		myErr = ::AEDisposeDesc( &aeReply );
		}
}



Boolean	TAELaunch::hideItemInFinder( OSType ostypeSig )
//
//	Send a boolean kAESetData AE to the finder to
//	hide a running process of signature 'ostypeSig'
//
{
	AEDesc 				aedescOSPEC		= { typeNull, NULL };
	AEAddressDesc			aedescFinderAddr 	= { typeNull, NULL };
	AppleEvent			aeEvent 			= { typeNull, NULL },
						aeReply 			= { typeNull, NULL };
	ProcessSerialNumber		psnProc;
	FSSpec				fsspecProc;
	long					lgFeature			= 0L;
	Boolean				boolVisible		= FALSE;
	OSErr				myErr 			= noErr;

//														idiot filter
	ASSERT( ostypeSig > 0L );
//														use Gestalt to check for OSL compliance
	myErr = ::Gestalt( gestaltFinderAttr, &lgFeature );
	if ( ( myErr == noErr ) && bTST( lgFeature, gestaltOSLCompliantFinder ) )
		NULL;
	else
		return( FALSE );
//														find name of process
	if ( ! findProcess( ostypeSig, kostypeApp, &psnProc, &fsspecProc ) )
		return( FALSE );
//														create object specifier
	buildVisibilityOSPEC( fsspecProc.name, &aedescOSPEC );
//														get PSN of finder
	findProcess( kostypeCreatorMac, kostypeFinder, &psnProc, &fsspecProc );
//														create AE targeted at finder
	myErr = ::AECreateDesc( typeProcessSerialNumber, &psnProc,
						sizeof( ProcessSerialNumber ), &aedescFinderAddr );
		
	myErr = ::AECreateAppleEvent( kAECoreSuite, kAESetData,
							&aedescFinderAddr, kAutoGenerateReturnID,
							kAnyTransactionID, &aeEvent );

	ASSERT( myErr == noErr );	
//														place OSPEC in AE	
	myErr = ::AEPutParamDesc( &aeEvent, keyDirectObject, &aedescOSPEC );

	ASSERT( myErr == noErr );	
//														place boolean in AE
	myErr = ::AEPutParamPtr( &aeEvent, keyAEData, typeBoolean,
						(Ptr)&boolVisible, sizeof( Boolean ) );

	ASSERT( myErr == noErr );	
//														send AE on its merry way
	myErr = ::AESend( &aeEvent, &aeReply, 
					kAENoReply + kAENeverInteract,
					kAENormalPriority, 
					kAEDefaultTimeout, 
					nil, nil );

	ASSERT( myErr == noErr );	
//														tidy up
	::AEDisposeDesc( &aedescOSPEC );
	::AEDisposeDesc( &aedescFinderAddr );
	::AEDisposeDesc( &aeEvent );
	::AEDisposeDesc( &aeReply );

	return( TRUE );	
}


void		buildVisibilityOSPEC( Str63 	str63Name, 
						AEDesc 	*ptraedescOSPEC )
//
//	Build an object specifier for the visibility
//	property of a process called 'str63Name'
//	running in the finder (phew!)
//
{
	const Boolean	kboolDisposeInputs = TRUE;

	AEDesc 			nullContainer 		= { typeNull, NULL },
					ospecProc 		= { typeNull, NULL },
					aedescName		= { typeNull, NULL },
					aedescVisi 		= { typeNull, NULL };
	long				lgVisiProperty 	= pVisible;
	OSErr			myErr			= noErr;

//														idiot filters
	ASSERT( str63Name[0] > 0 );
	ASSERT( ptraedescOSPEC != NULL );
//														create AEDesc for process name
	myErr = ::AECreateDesc( typeChar, &str63Name[1], 
						str63Name[0], &aedescName );

	ASSERT( myErr == noErr );

//	create an ospec to indicate that we are playing with
//	a process called 'str63Name' of class 'cProcess'
//	running in the finder

	myErr = ::CreateObjSpecifier( cProcess, &nullContainer, 
							formName, &aedescName, 
							kboolDisposeInputs, &ospecProc );
	
	ASSERT( myErr == noErr );
//														create AEDesc for visibility property
	myErr = AECreateDesc( typeType, &lgVisiProperty, 
						sizeof( long ), &aedescVisi );
	
	ASSERT( myErr == noErr );

//	now build a specifier representing the 'pVisible' property
//	of the target process, using 'ospecProc' as its container...

	myErr = CreateObjSpecifier( cProperty, &ospecProc,
						formPropertyID, &aedescVisi, 
						kboolDisposeInputs, ptraedescOSPEC );
	
	ASSERT( myErr == noErr );
}



Boolean	TAELaunch::openControlPanel( OSType ostypeCreator )
//
//	Given the OSType creator of a control
//	panel, fire it up and return TRUE if
//	everything went okay
//
{
	FSSpec 		fsspecCPanel;
	HFileParam	hfpRec;
	short		i = 0;
	OSErr		myErr = noErr;

//														idiot filters
	ASSERT( ostypeCreator > 0L );
	SALT_MEMORY( &hfpRec, sizeof( HFileParam ) );
//														locate 'Control Panels' folder
	myErr = ::FindFolder( kOnSystemDisk, kControlPanelFolderType, FALSE,
						&(fsspecCPanel.vRefNum), 
						&(fsspecCPanel.parID) );
	if ( myErr == noErr ) 
		{
		hfpRec.ioNamePtr = fsspecCPanel.name;
		hfpRec.ioVRefNum = fsspecCPanel.vRefNum;
//														cycle thru all files in folder until match
		do {
			hfpRec.ioDirID = fsspecCPanel.parID;
			hfpRec.ioFDirIndex = ++i;

			myErr = ::PBHGetFInfoSync( (HParmBlkPtr)&hfpRec );
			if ( myErr == noErr ) 
				{
				if ( ( hfpRec.ioFlFndrInfo.fdType == kostypeCdev ) &&
					( hfpRec.ioFlFndrInfo.fdCreator == ostypeCreator ) )
					//	the fsspec provided by user is already
					//	filled with the elements of the control panel,
					//	thus there's nothing more to do!
//														now open control panel!
				return( openItemInFinder( fsspecCPanel, TRUE ) );
				}
			} while ( myErr == noErr );
		}

	return( FALSE );
}



TAELaunch::EProcess		TAELaunch::scanProcesses( const FSSpec 		& fsspecITEM,
											OSType			*ptrostypeSig,
											ProcessSerialNumber	*ptrPSN)
//
//	Given the FSSpec of an item
//	about to be launched, check
//	current processes to see if one
//	of the same kind is already running.
//	If something of the same kind is found,
//	then dumps its process signature & PSN
//	into ptrs provided
//
//	Returns (TAELaunch::EProcess) :
//	kenumError		something bad happened�
//	kenumNone			no equivalent process running
//	kenumSimilar		same creator running, but filetype
//					of FSSpec sent in is �
//	kenumIdentical		creator & filetype of FSSpec provided
//					is identical to a process already running
//					(hence no need to do anything)
//
{
	FInfo			finfoRec;
	ProcessInfoRec 	procirRec;
	FSSpec			fsspecProc;
	long 			lgFeature = 0L;
	OSErr			myErr = noErr;
	EProcess			eResult = kenumNone;

//														idiot filters
	ASSERT( ptrostypeSig != NULL );
	ASSERT( ptrPSN != NULL );
	SALT_MEMORY( &finfoRec, sizeof( FInfo ) );
	SALT_MEMORY( &procirRec, sizeof( ProcessInfoRec ) );
//														check gestalt to see if we can launch
	myErr = ::Gestalt( gestaltOSAttr, &lgFeature );
	if ( ( myErr == noErr ) && bTST( lgFeature, gestaltLaunchControl ) )
		NULL;
	else
		return( kenumError );
//														get OSTypes of fsspecITEM item
	myErr = ::FSpGetFInfo( (FSSpecPtr)&fsspecITEM, &finfoRec );
	if ( myErr != noErr )
		return( kenumError );
//														prepare to examine running processes
	ptrPSN->highLongOfPSN = 0;
	ptrPSN->lowLongOfPSN = kNoProcess;
	procirRec.processInfoLength = sizeof( ProcessInfoRec );
	procirRec.processName = nil;
	procirRec.processAppSpec = &fsspecProc;
//														step thru all active processes until match
	while ( ::GetNextProcess( ptrPSN ) == noErr ) 
		{
		if ( ::GetProcessInformation( ptrPSN, &procirRec ) == noErr ) 
			{
			 if ( procirRec.processSignature == finfoRec.fdCreator )
			 	{
				eResult = kenumSimilar;
				if ( procirRec.processType == long( finfoRec.fdType ) )
					eResult = kenumIdentical;
//														assign data of matching process
				*ptrostypeSig = procirRec.processSignature;
				//	ptrPSN has already been filled!
				break;
				}
			}
		}
	
	return( eResult );
}



Boolean	TAELaunch::docOpenable( const FSSpec & fsspecFile )
//
//	Check to see if a document is not already open.
//	Do this by trying to open data fork of file 
//	with write permission.  
//	Return TRUE if attempt is successful.
//
{
	short		shRefNum = 0;
	OSErr		myErr = noErr;
	Boolean		boolDatOpenable = FALSE;
//														now try to open the data fork
	myErr = ::FSpOpenDF( (FSSpecPtr)&fsspecFile, 
					fsWrPerm, 
					&shRefNum );
//														can open file, close data fork
	if ( myErr == noErr )
		{
		::FSClose( shRefNum );
		boolDatOpenable = TRUE;
		}
//														check if user supplied dumb FSSpec
//														let calling function handle it!
	else if ( myErr == fnfErr )
		return( FALSE );

	return( boolDatOpenable );
}



Boolean	TAELaunch::findProcess( OSType 				ostypeCreator, 
							OSType 				ostypeProcType, 
							ProcessSerialNumber *	ptrPSN, 
							FSSpecPtr				ptrfsspecApp )
//
//	Step thru all active processes until
//	one is found which matches the creator & 
//	file type OSType's given.  Load PSN &
//	FSSpec of process into ptr's provided.
//	Return TRUE if success.
//
{
	ProcessInfoRec 	procirRec;
	long 			lgFeature = 0L;
	Boolean 			boolFoundIt = FALSE;
	OSErr			myErr = noErr;

//														idiot filters
	ASSERT( ostypeCreator > 0L );
	ASSERT( ostypeProcType > 0L );
	ASSERT( ptrPSN != NULL );
	ASSERT( ptrfsspecApp != NULL );
	SALT_MEMORY( &procirRec, sizeof( ProcessInfoRec ) );
//														check gestalt to see if we can launch
	myErr = ::Gestalt( gestaltOSAttr, &lgFeature );
	if ( ( myErr == noErr ) && bTST( lgFeature, gestaltLaunchControl ) )
		NULL;
	else
		return( FALSE );
	
	ptrPSN->highLongOfPSN = 0;
	ptrPSN->lowLongOfPSN = kNoProcess;
	procirRec.processInfoLength = sizeof( ProcessInfoRec );
	procirRec.processName = nil;
	procirRec.processAppSpec = ptrfsspecApp;
//														step thru all active processes until match
	while ( ::GetNextProcess( ptrPSN ) == noErr ) 
		{
		if ( ::GetProcessInformation( ptrPSN, &procirRec ) == noErr ) 
			{
			if ( ( procirRec.processType == long( ostypeProcType ) ) &&
			 	( procirRec.processSignature == ostypeCreator ) ) 
			 	{
				boolFoundIt = TRUE;
				break;
				}
			}
		}
	
	return( boolFoundIt );
}




Boolean	TAELaunch::grabCurrentFSSpec( FSSpecPtr ptrfsspecAPP )
//
//	Dump the FSSpec of the current foreground process
//	into the FSSpecPtr provided.  Return TRUE if
//	no problems
//
{
	ProcessSerialNumber		psnOriginal;
	ProcessInfoRec 		procirRec;
	OSErr				myErr = noErr;

//														idiot filter
	ASSERT( ptrfsspecAPP );

	myErr = ::GetCurrentProcess( &psnOriginal );
	if ( myErr != noErr )
		return( FALSE );

	procirRec.processInfoLength = sizeof( ProcessInfoRec );
	procirRec.processName = nil;
	procirRec.processAppSpec = ptrfsspecAPP;

	myErr = ::GetProcessInformation( &psnOriginal, &procirRec );

	return( myErr == noErr );
}



void		TAELaunch::addFSSToAEList( AEDescList 	*ptrDescList, 
							short 		shRow, 
							const FSSpec 	& fsspecFile )
//
//	Given a FSSpec, add it to a AEDescList at
//	row 'shRow'
//
{
	AliasHandle	haliasFile;
	OSErr		myErr = noErr;
	
//														idiot filters
	ASSERT( ptrDescList != NULL );
	ASSERT( shRow >= 0 );
//														create alias handle & insert into AEDescList
	myErr = ::NewAlias( nil, &fsspecFile, &haliasFile );
	if (myErr == noErr) 
		{
		::HLock((Handle) haliasFile);
		myErr = ::AEPutPtr( ptrDescList, shRow, typeAlias,
						(Ptr)*haliasFile, (*haliasFile)->aliasSize );
		::DisposeHandle( (Handle) haliasFile );
		}
}
