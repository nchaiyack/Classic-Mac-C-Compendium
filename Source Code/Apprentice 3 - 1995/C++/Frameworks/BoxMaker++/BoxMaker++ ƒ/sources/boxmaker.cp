#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <Packages.h>
#include <StandardFile.h>
#include <Traps.h>
#include <Files.h>
#include <Aliases.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include <Fonts.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Desk.h>
#include <Errors.h>

#include "boxmaker constants.h"

#include "standardgetfile.h"
#include "boxmakergetfile.h"
#include "boxmaker.h"

const AEAddressDesc boxmaker::self = GetTargetToSelf();

const CursHandle boxmaker::theClock = GetCursor( watchCursor);

Boolean boxmaker::itsProcessable() const
{
	Boolean result = false;

	if( itsVisible() || passInvisibles)
	{
		//
		// First check against the 'typs' resource, if one was present
		//
		result = matchesTypeList();

		if( result == true)
		{
			//
			// We passed the 'typs' resource check; check against
			// the 'tycr' resource.
			//
			result = matchesTypeCreatorPairs();
		}
	}
	return result;
}

void boxmaker::InitToolbox() 
{	
	InitGraf( &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L);
	InitCursor();
	FlushEvents( everyEvent, 0);
	MoreMasters();
	MoreMasters();
}

Boolean boxmaker::InitGlobals() 
{
	the_status	= shell_is_running;
	gMainDialog	= NULL;

	long aLong;
	return (Gestalt( gestaltAppleEventsAttr, &aLong) == noErr);
}

void boxmaker::SetUpMenus()
{
	Handle theMenus = GetNewMBar( kMBarID);
	
	SetMenuBar( theMenus);

	MenuHandle theAppleMenu = GetMenuHandle( kAppleMenuID);
	AppendResMenu( theAppleMenu, 'DRVR');
	DrawMenuBar();
}
//
//	This routine is called during startup to display a splash screen.
//	
//	This was recommend by the Blue Team HI person, John Sullivan, who
//	feels that all apps should display something so that users can easily
//	tell what is running, and be able to switch by clicking.  Thanks John!
//
void boxmaker::GetMainDialog()
{
	if( gMainDialog == 0)
	{
		gMainDialog = GetNewDialog( kSettingsDialogID,
									&myDialogRecord, (WindowPtr)-1L);
	}
}

void boxmaker::ShowPreferences()
{
	if( gMainDialog != 0)
	{
		ShowWindow( gMainDialog);
	}
}

void boxmaker::ShowAbout()
{
	(void)Alert( kAboutAlertID, NULL);
}

void boxmaker::DoAppleMenu( short itemID)
{
	if( itemID == 1)
	{
		ShowAbout();
	} else {
		Str255 itemStr;
		GetMenuItemText( GetMenuHandle( kAppleMenuID), itemID, itemStr);
		OpenDeskAcc( itemStr);
	}
}

void boxmaker::DoMenu( long retVal)
{
//	const short menuID = HiWord( retVal);
//	const short itemID = LoWord( retVal);
	const short menuID = retVal >> 16;
	const short itemID = retVal & 0x00000FFFF;

	switch( menuID)
	{
		case kAppleMenuID:
			DoAppleMenu( itemID);
			break;
			
		case kFileMenuID:
			switch( itemID)
			{
				case kSelectFileItem:
					SelectFile();
					break;
			
				case kPrefsItem:
					ShowPreferences();
					break;
					
				case kQuitItem:
					SendQuitToSelf();
					break;
			}
			break;
		
		default:
			break;			
	}
	HiliteMenu( 0);
}

void boxmaker::DoMouseDown()
{
	WindowPtr	whichWindow;
	short whichPart = FindWindow( myEvent.where, &whichWindow);
	switch( whichPart)
	{
		case inMenuBar:
			DoMenu( MenuSelect( myEvent.where));
			break;
		
		case inSysWindow:
			SystemClick( &myEvent, whichWindow);
			break;
		
		case inDrag:
			DragWindow( whichWindow, myEvent.where, &qd.screenBits.bounds);
			break;

		case inGoAway:
			if( TrackGoAway( whichWindow, myEvent.where))
			{
				HideWindow( whichWindow);
			}
			break;
		//
		// We do not handle zooming or resizing. It seems simple, but one would
		// need to add a couple of virtual functions to make it practical, so
		// we do not do it.
		//
		default:
			break;
	}
}

void boxmaker::DoKeyDown()
{
	if( myEvent.modifiers & cmdKey)
	{
		DoMenu( MenuKey( (char)myEvent.message & charCodeMask));
	}
}

boxmaker::boxmaker( short dlogID) : boxmakergetfile( dlogID)
{
	InitToolbox();
	if( !InitGlobals())
	{
		ErrorAlertQuit( kErrStringID, kCantRunErr, 0);
	}
	InitAEVTStuff();
	SetUpMenus();
	GetMainDialog();

	theCInfoPBRec.hFileInfo.ioNamePtr = theFSSpec.name;
	
	GetCurrentProcess( &myPSN);
	time_of_next_cup_of_tea = TickCount() + kFirstSleepInterval;
}

boxmaker::~boxmaker()
{
	//
	// 950621: two users complain that �Zak crashes their Mac at exit time.
	// This appears to be a problem with the control panel 'ApplWindows'.
	// Fortunately, it can be easily prevented by disposing of the preferences
	// dialog before exiting. We fix this inside boxmaker, and not in
	// muzakshell::~muzakshell because of the following:
	//
	// - Logically, this code should be part of boxmaker::~boxmaker,
	// - the bug in ApplWindows may bite other users of boxmaker.
	//
	if( gMainDialog != 0)
	{
		CloseDialog( gMainDialog);
	}
}

void boxmaker::run()
{
	Boolean canQuitNow = false;
	while( !canQuitNow)
	{
		const short currentSleep = amInFront() ? kForeSleepValue : kBackSleepValue;
	
		gotEvent = WaitNextEvent( everyEvent, &myEvent, currentSleep, NULL);
		//
		// We use 'EventloopHook( �) && �' rather than '� && EventloopHook( �)'
		// since we always want to call 'EventloopHook'.
		//
		canQuitNow = EventloopHook() && (the_status >= shell_is_finishing);
	
		if( gotEvent)
		{
			const int gotCmdKey = (myEvent.what == keyDown) && (myEvent.modifiers & cmdKey);
	
			if( !gotCmdKey && IsDialogEvent( &myEvent))
			{
				short itemHit;
				DialogPtr theDialog;
				if( DialogSelect( &myEvent, &theDialog, &itemHit))
				{
					HandleDialogEvent( itemHit, theDialog);
				}
			} else {
				switch( myEvent.what)
				{
					case kHighLevelEvent:
						DoHighLevelEvent();
						break;
						
					case mouseDown:
						DoMouseDown();
						break;
						
					case keyDown:
					case autoKey:
						DoKeyDown();
						break;
						
					default:
						break;
				}
			}
		}
	}
}

void boxmaker::InitAEVTStuff()
{
	OAPPPtr = NewAEEventHandlerProc( HandleOAPP);
	ODOCPtr = NewAEEventHandlerProc( HandleODOC);
	PDOCPtr = NewAEEventHandlerProc( HandlePDOC);
	QuitPtr = NewAEEventHandlerProc( HandleQuit);

	OSErr aevtErr =

		AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
					 OAPPPtr, (long)this, false)

		|| AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
					(AEEventHandlerUPP) ODOCPtr, (long)this, false)

		|| AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
					(AEEventHandlerUPP) PDOCPtr, (long)this, false)

		|| AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
					(AEEventHandlerUPP) QuitPtr, (long)this, false);
	FailErr( aevtErr);
}

OSErr boxmaker::GotRequiredParams( AppleEvent *theAppleEvent)
{
	DescType	typeCode;
	Size		actualSize;
	OSErr		result;

	OSErr err = AEGetAttributePtr( theAppleEvent, keyMissedKeywordAttr,
					typeWildCard, &typeCode, NULL, 0, &actualSize);
	
	if( err == errAEDescNotFound)	// we got all the required params: all is ok
	{
		result = noErr;
	} else if( err == noErr) {
		result = errAEEventNotHandled;
	} else {
		result = err;
	}
	return result;
}
//
//	This routine is the handler for the oapp (Open Application) event.
//
//	It first checks the number of parameters to make sure we got them all
//	(even though we don't want any) and then calls the OpenApp userProc.
//	Finally it checks to see if the caller wanted a reply & sends one,
//	setting any error.
//
pascal OSErr HandleOAPP( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	OSErr result = boxmaker::GotRequiredParams( theAppleEvent);

	boxmaker *me = (boxmaker *)handlerRefcon;
	me->the_status = shell_is_OApped;
	
	me->OpenApp();		// pass it on to the app specific routine

	if( reply->dataHandle != NULL )	//	a reply is sought
	{
		const char error_message[] = "Opening";
		const long error_length = sizeof( error_message) - 1;	// don't include zero terminator
		result = AEPutParamPtr( reply, 'errs', 'TEXT', error_message, error_length);
	}
	return result;
}
//
//	This routine is the handler for the quit (Quit Application) event.
//
//	It first checks the number of parameters to make sure we got them all 
//	(even though we don't want any) and then calls the QuitApp userProc.
//	Finally it checks to see if the caller wanted a reply & sends one,
//	setting any error.
//
pascal OSErr HandleQuit( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	OSErr result = boxmaker::GotRequiredParams( theAppleEvent);

	boxmaker *me = (boxmaker *)handlerRefcon;
	
	me->the_status = shell_is_quitting;

	if( reply->dataHandle != NULL )	//	a reply is sought
	{
		const char error_message[] = "Quitting";
		const long error_length = sizeof( error_message) - 1;	// don't include zero terminator
		result = AEPutParamPtr( reply, 'errs', 'TEXT', error_message, error_length);
	}
	return result;
}
//	
//	This routine is the low level processing routine for both the 
//	odoc (Open Document) and pdoc (Print Document) events.
//
//	This routine is the key one, since this is how we get the list of
//	files/folders/disks to process.  The first thing to do is the get the
//	list of files, and then make sure that's all the parameters (should be!).
//	We then process each file in the list by calling _HandleOneDoc.
//
OSErr boxmaker::_HandleDocs( AppleEvent *theAppleEvent,
								AppleEvent *reply, Boolean opening)
{
	AEDescList	docList;
	long		itemsInList;

	OSErr result =
		AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &docList);

	if( result == noErr)
	{
		result = GotRequiredParams( theAppleEvent);
		if( result == noErr)
		{
			result = AECountItems( &docList, &itemsInList);
		}
	}
	StartABunch( itemsInList, opening);
	for( int index = 1; (index <= itemsInList) && (result == noErr); index++)
	{
		FSSpec		aFSSpec;
		Size		actualSize;
		AEKeyword	keywd;
		DescType	typeCode;
		
		result = AEGetNthPtr( &docList, index, typeFSS, &keywd,
						&typeCode, (Ptr)&aFSSpec, sizeof( aFSSpec ), &actualSize);

		if( result == noErr)
		{
			//
			// Fill in the volume reference number
			// (it will not change during a recursive descent)
			// But the different dropped items might be on different volumes,
			// so we do it inside the 'itemsInList' loop.
			// This is true since we do not 'follow aliases' during the descent.
			// (implementing that would also mean having to check for endless loops,
			// and thus would not be trivial)
			//
			theCInfoPBRec.hFileInfo.ioVRefNum = aFSSpec.vRefNum;
	
			result = _HandleOneDoc( aFSSpec, opening);
		}
	}
	EndABunch( itemsInList, opening);
	if( (the_status == shell_is_running) && opening)
	{
		//
		//	The reason we also check for 'opening' is based on a recommendation
		//	in the Apple event Registry which specifically states that you should
		//	NOT quit on a 'pdoc' as the Finder will send you a 'quit' when it is
		//	ready for you to do so.
		//
		the_status = shell_is_finishing;
	}
	const OSErr dispose_result = AEDisposeDesc( &docList);
	if( result == noErr)
	{
		result = dispose_result;
	}
	return result;
}
//
// _HandleOneDoc is called repeatedly by _HandleDocs
//
OSErr boxmaker::_HandleOneDoc( const FSSpec &aFSSpec, Boolean opening)
{
	OSErr result = GatherInfo( aFSSpec);

	if( result == noErr)
	{
		if( itsADirectory())
		{
			result = _HandleADir( theSubDirID, opening);
		} else {
			if( itsProcessable())
			{
				SetUp_theFSSpec_from_theCInfoPBRec();
				OpenDoc( opening);
			}
		}
	}
	return result;
}
//
// _HandleADir is the low-level recursive directory handler.
//
OSErr boxmaker::_HandleADir( long dirID, Boolean opening)
{
	OSErr result = noErr;

	if( itsVisible() || passInvisibles)
	{
		if( passFolders)
		{
			SetUp_theFSSpec_from_theCInfoPBRec();
			OpenDoc( opening);
		}
		if( enterFolders && (itsVisible() || enterInvisibles))
		{
			SetUp_theFSSpec_from_theCInfoPBRec();
			if( mayEnterFolder( opening))
			{
				const char theAccessRights = getAccessRights();
				//
				// We must enter the folder whenever we have
				// either read or search privileges, or both
				//
				if( (theAccessRights & 3) == 3)
				{
					CantEnterFolder( opening);
				} else {
					EnterFolder( opening);
					int index = 1;
					do
					{
						result = GatherInfo( dirID, index);
						
						if( result == noErr)
						{
							if( itsADirectory())
							{
								result = _HandleADir( theSubDirID, opening);
							} else {
								if( itsProcessable())
								{
									SetUp_theFSSpec_from_theCInfoPBRec();
									OpenDoc( opening);
								}
							}
						}
						index += 1;
					} while( result == noErr);
					if( result == fnfErr)	// fnfErr simply indicates end of directory
					{
						result = noErr;
					}
					ExitFolder( opening);
				}
			}
		}
	}
	return result;
}
//
//	This routine is the handler for the odoc (Open Document) event.
//	
//	The odoc event simply calls the common _HandleDocs routines, which will
//	do the dirty work of parsing the AEVT & calling the userProcs.
//
pascal OSErr HandleODOC( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	boxmaker *me = (boxmaker *)handlerRefcon;
	return me->_HandleDocs( theAppleEvent, reply, true);
}
//
//	This routine is the handler for the pdoc (Print Document) event.
//
//	The pdoc event like the odoc simply calls the common _HandleDocs routines
//
pascal OSErr HandlePDOC( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	boxmaker *me = (boxmaker *)handlerRefcon;
	return me->_HandleDocs( theAppleEvent, reply, false);
}
//
//	This is the routine called by the main event loop, when a high level
//	event is found.  Since we only deal with Apple events, and not other
//	high level events, we just pass everything onto the AEM via AEProcessAppleEvent
//
void boxmaker::DoHighLevelEvent()
{
	FailErr( AEProcessAppleEvent( &myEvent));
}

void boxmaker::SelectFile()
{
	doIt();
	if( sfGood)
	{
		SendODOCToSelf( &sfFile);
	}
}
//
//	A quick & dirty error reporter
//
void boxmaker::ErrorAlert( short errorNo, short stringIndexID, short stringListID)
{
	Str255	param;
	Str15	errorStr;

	NumToString( errorNo, errorStr);
	GetIndString( param, stringListID, stringIndexID);
	ParamText( param, errorStr, NULL, NULL);
	(void)Alert( kErrorAlertID, NULL);
}

AEAddressDesc boxmaker::GetTargetToSelf()
{
	ProcessSerialNumber	psn;

	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN  = kCurrentProcess;

	AEAddressDesc result;
	(void)AECreateDesc( typeProcessSerialNumber, (Ptr)&psn,
								sizeof(ProcessSerialNumber), &result);
	return result;
}
//
//	This routine is the low level routine used by the SendODOCToSelf
//	routine.  It gets passed the list of files (in an AEDescList)
//	to be sent as the data for the 'odoc', builds up the event
//	and sends off the event.  
//
//	It is broken out from SendODOCToSelf so that a SendODOCListToSelf could
//	easily be written and it could then call this routine - but that is left
//	as an exercise to the reader.
//
void boxmaker::_SendDocsToSelf( AEDescList *aliasList)
{
	AppleEvent openDocAE;
	OSErr err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
				&self, kAutoGenerateReturnID, kAnyTransactionID, &openDocAE);

	if( err == noErr)
	{
		err = AEPutParamDesc( &openDocAE, keyDirectObject, aliasList);

		if( err == noErr)
		{
			//
			//	Since we are sending to ourselves, there is no need for reply.
			//
			AppleEvent replyAE;
			err = AESend( &openDocAE, &replyAE, kAENoReply | kAECanInteract,
												kAENormalPriority, 3600, NULL, NULL);
			//
			//	NOTE: Since we are not requesting a reply, we do not need to
			//	need to dispose of the replyAE.  It is there simply as a 
			//	placeholder.
			//
		}
	}
	err = AEDisposeDesc( &openDocAE);
}
//
//	This is the routine called by SelectFile to send a single odoc to ourselves.
//
//	It calls the above low level routine to do the dirty work of sending the AEVT -
//	all we do here is build a AEDescList of the file to be opened.
//
void boxmaker::SendODOCToSelf( FSSpec *theFileSpec)
{
	AEDescList	aliasList;

	const OSErr error = AECreateList( NULL, 0, false, &aliasList);

	if( error == noErr)
	{
		AliasHandle	aliasH;

		(void)NewAlias( NULL, theFileSpec, &aliasH);

		AEDesc aliasDesc;
		aliasDesc.descriptorType = typeAlias;
		aliasDesc.dataHandle     = (Handle)aliasH;
		
		(void)AEPutDesc( &aliasList, 0, &aliasDesc);

		DisposeHandle( (Handle)aliasH);

		_SendDocsToSelf( &aliasList);

		(void)AEDisposeDesc( &aliasList);
	}
}

Boolean boxmaker::amInFront() const
{
	ProcessSerialNumber frontProcess;
	Boolean result = false;
	if( GetFrontProcess( &frontProcess) == noErr)
	{
		(void)SameProcess( &myPSN, &frontProcess, &result);
	}
	return result;
}

void boxmaker::SendQuitToSelf (void)
{
	AppleEvent quitAE;
	OSErr error = AECreateAppleEvent( kCoreEventClass, kAEQuitApplication,
					&self, kAutoGenerateReturnID, kAnyTransactionID, &quitAE);

	if( error == noErr)
	{
		AppleEvent replyAE;
		error = AESend( &quitAE, &replyAE, kAENoReply | kAECanInteract,
										kAENormalPriority, 3600, NULL, NULL);
		//
		//	NOTE: Since we are not requesting a reply, we do not need to
		//	need to dispose of the replyAE.  It is there simply as a 
		//	placeholder.
		//
	}
	(void)AEDisposeDesc( &quitAE);
}

void boxmaker::errDebug( OSErr errno, unsigned char *errString)
{
	if( errno != noErr)
	{
		Str63 debugString;
		NumToString( errno, *(Str255 *)&debugString);

		const int numberLength = debugString[ 0];
		//
		// the '+ 1' is for the colon after the error number
		//
		const int numFitting = 63 - (numberLength + 1);
		const int errStringLength = errString[ 0];
		const int numToCopy =
			(errStringLength < numFitting) ? errStringLength : numFitting;

		unsigned char *firstafter = &debugString[ numberLength + 1];
		*firstafter++ = ':';
		const unsigned char *currentMessageChar = &errString[ 1];

		for( int i = 0; i < numToCopy; i++)
		{
			*firstafter++ = *currentMessageChar++;
		}
		debugString[ 0] = numberLength + 1 + errStringLength;
		DebugStr( debugString);
	}
}

OSErr boxmaker::_GatherInfo( long theDirID)
{
	theCInfoPBRec.hFileInfo.ioDirID = theDirID;
	//
	// Note: According to IM, the field 'ioACUser' of the parameter block
	// is unmodified when GetCatInfo is called for a non-network directory.
	// (well, that is if one interprete a lack of an output arrow on page IV-153
	// of IM-IV as meaning that the field in question is left untouched).
	// However, with the arrival of 'File Manager extensions in a shared Environment'
	// users can no longer assume that the field is unmodified, so one could suppose
	// that GetCatInfo now _always_ returns something in that field. To be compatible
	// with non-shared environments I set the field to a certainly good value before
	// calling PBGetCatInfo. We use zero, which translates to 'everybody may do anything',
	// which is a logical value in a non-shared environment.
	// Also, I do not have the most recent header files, so I hack my way to the
	// field in question.
	//
	((char *)(&theCInfoPBRec))[ 31] = (char)0;
	const OSErr result = PBGetCatInfoSync( &theCInfoPBRec);
	
	TimeForTea();
	//
	// ioDirID is both in- and output for GetCatInfo. If the item for which info
	// was requested is a directory it is changed to the ID of that directory.
	// In that case we have to set it back to the right value before we can do
	// a 'SetCatInfo'. 'OpenDoc' assumes that theCInfoPBRec _can_ be used to set
	// file information, so we reset it immediately. We do keep the ID of the item
	// itself in 'theSubDirID'.
	//
	theSubDirID = theCInfoPBRec.hFileInfo.ioDirID;
	theCInfoPBRec.hFileInfo.ioDirID = theDirID;
	
	return result;
}

void boxmaker::StartABunch( long numTopLevelItems, Boolean opening)
{
	SetCursor( *theClock);
	//
	// Disable all menus:
	//
	Handle theMenuBar = GetMenuBar();
	HLock( theMenuBar);

	const menulist *theList = (const menulist *)*theMenuBar;

	const int numMenus = theList->lastHandleOffset / 6;
	
	for( int i = 0; i < numMenus; i++)
	{
		const MenuHandle currentMenu = theList->theMenus[ i].theMenu;
		DisableItem( currentMenu, 0);
	}
	DrawMenuBar();
	DisposeHandle( theMenuBar);
}

void boxmaker::EndABunch( long numTopLevelItems, Boolean opening)
{
	InitCursor();
	//
	// Enable all menus:
	//
	Handle theMenuBar = GetMenuBar();
	HLock( theMenuBar);

	const menulist *theList = (const menulist *)*theMenuBar;

	const int numMenus = theList->lastHandleOffset / 6;
	
	for( int i = 0; i < numMenus; i++)
	{
		const MenuHandle currentMenu = theList->theMenus[ i].theMenu;
		EnableItem( currentMenu, 0);
	}
	DrawMenuBar();
	DisposeHandle( theMenuBar);
}

void boxmaker::TimeForTea()
{
	//
	// 950116: In an attempt to make dropboxes built with boxmaker++ more
	// MultiFinder-friendly (anybody know a more modern name for that?)
	// we call WaitNextEvent every now and then during a directory traversal.
	// We do not want to get any further 'ODOC' events here, though, so we
	// don't handle any events. This is only a rudimentary solution to the
	// problem, but it does work (sort of).
	//
	if( TickCount() > time_of_next_cup_of_tea)
	{
		const Boolean inFront = amInFront();
		const short currentSleep    = inFront ? kForeSleepValue    : kBackSleepValue;
		const short currentInterval = inFront ? kForeSleepInterval : kBackSleepInterval;

		EventRecord junkEvent;
		(void)WaitNextEvent( 0, &junkEvent, currentSleep, nil);

		if( inFront)
		{
			SetCursor( *theClock);
		}
		FlushEvents( mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);
		
		time_of_next_cup_of_tea = TickCount() + currentInterval;
	}
}
