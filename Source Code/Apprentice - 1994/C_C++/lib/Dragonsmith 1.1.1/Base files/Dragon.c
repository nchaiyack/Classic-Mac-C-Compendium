/*
	Dragon.c
	
	Dragonsmith 1.1.1
	
	Simple framework for a drag-and-drop application.  A "dragon" does something meaningful (one
	would hope) with the Finder objects that were drag-and-dropped onto it.  Most dragons will
	stick around after the initial launch unless there are no settings for the user to change.
	
	This class does nothing by itself; simple subclasses will need to override ProcessFile —
	more powerful subclasses may need to do much more (e.g., override event handling methods).
	
	Developed using THINK C 5.0 and 6.0, Resorcerer 1.1.1 and ResEdit 2.1.1
	
	Copyright © 1992–1994 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.
	
	For a complete description of the contents of this file, see the "Dragonsmith Programmer’s Manual"
	
*/

#include	"Dragon.h"

#include	<Folders.h>
#include	"AppleEventUtils.h"
#include	"EventUtils.h"
#include	"GestaltUtils.h"
#include	"HandleUtils.h"
#include	"MenuUtils.h"
#include	"StringUtils.h"
#include	"ProcessUtils.h"
#include	"TrapUtils.h"

//	-------------------------------------------	P u b l i c     m e t h o d s	----------------------------------------------

Dragon::Dragon (void)
{
	FSSpec	appFile;
	Handle	h;
	
	// We use unrelocatable blocks for curDocPB and curDocFSS in order to avoid having to HLock the dragon object
	//	to prevent its instance variables from moving during calls to (for example) ResolveAliasFile, etc.
	
	curDocFSS = (FSSpec *) NewPtr (sizeof (FSSpec));
// NOTE bug-fix 1.1.1 — call NewPtrClear here instead of NewPtr to clear unused parameters
	curDocPB = (PBRecUnion *) NewPtrClear (sizeof (PBRecUnion));	
	if (curDocPB == NULL || curDocFSS == NULL)
		Abort (memFullErr);
		
	curDocPB->h.fileParam.ioCompletion = NULL;			// All PB calls will be synchronous
	curDocPB->h.fileParam.ioNamePtr = curDocFSS->name;	// This only has to be set once, since curDocFSS won't move
	curDocFSS->name[0] = 0;	// Just in case

	dirDepthLimit = 0;			// Change this instance variable to a largish negative number (say, -100) in your
							//	dragon's constructor if you want to recursively open all folders and volumes to
							//	process what's inside.  Or change it to -1 if you just want to go down 1 level
	curDirDepth = 0;			// Start out at the top level of processing (of course)
	filesOnly = FALSE;			// If we get FSSpecs to folders to volumes, go ahead and use them as is
							//	— set this to TRUE if dirDepthLimit contains a small negative value (i.e., a value
							//	near zero) and your ProcessDroppings is designed to work only on files
							// If you don't want to get folders and volumes at all, just say "no" — don't put
							//	'fold' and 'disk' in the FREF
	resolveAliases = TRUE;		// Resolve any aliases you end up getting? (i.e., by opening folders — the Finder
							//	kindly resolves any aliases that are dragged-and-dropped, so this instance
							//	variable is meaningless unless dirDepthLimit < 0)
	followAliasChain = TRUE;	// Always resolve aliases to the original file (not to an intermediate alias)?
	useCustomFilter = FALSE;	// Don't do any special filtering of docs
	acceptableTypes = NULL;	// The types of documents which we can process (e.g., 'TEXT', '****', 'disk', etc.)
	
	preferences = NULL;
	prefsFileType = kPrefsFileType;	// Subclasses should probably not change this — the System 7 Finder displays
								//	a rather nice icon for files of this type
	dragonPrefs = NULL;
	
	appResFork = CurResFile ();	// Keep a refNum to the application's resource fork
	
	// Now use it to get an FSSpec to our app's file — we use a local copy because calling RefNumToFSSpec 
	//	may move memory
	if (RefNumToFSSpec (appResFork, &appFile, NULL) != noErr)
		appFile.name[0] = 0;		// This should never never be executed, but then you never never know…
	this->appFile = appFile;			// Copy local variable to instance variable
	
	h = Get1Resource ('BNDL', 128);
	signature = (h ? **((OSType **) h) : '????');		// '????' is the least obnoxious default I could think of

	acceptableTypes = FREFTypes (appResFork);	// File types this dragon will accept (from its 'FREF' resources)

	// These hard-coded default sleep values will be overridden by ones from the Dragon preferences resource
	//	('DrPr' 128) — if everything goes well in InitPrefs, that is
	sleepValue[kFGIdle] = 14;					// Allow TEIdle to work OK (15 would PROBABLY be safe, too)
	sleepValue[kBGIdle] = 60 * 60;				// Laziest state — check once every minute
	sleepValue[kFGBusy] = 3;					// Give the user the CPU's (almost) undivided attention
	sleepValue[kBGBusy] = 60;					// Process about one doc per second while in the background
// BEGIN bug-fix 1.1.1 — Set a default value for sleepTime
	sleepTime = 14;							// We'll check reality later (in Start) — for now, assume we're
											//	starting in the foreground (we know there's nothing to do yet)
// END bug-fix 1.1.1
	
	running = TRUE;							// Setting running = FALSE later will cause the dragon to quit
	abortProcessing = FALSE;					// Nothing's gone wrong yet
	busyCursor = GetCursor (kWatchCursor);		// Get the watch cursor
	if (busyCursor == NULL)						// Make sure we actually got it
		Abort (eInitializationFailed);				//	(not that this will ever happen, but…)
	HLockHi ((Handle) busyCursor);				// Lock it down for carefree dereferencing later
	cursorRgn = NULL;							// This is for WaitNextEvent

	aeQueue = NULL;
	numAEsPending = 0;
	
	autoQuit = FALSE;			// A subclass's constructor (which will be called AFTER this one) should set autoQuit
							//	to TRUE instead if it wants to hang around after starting up — and should set
							//	the corresponding bit in 'DrPr' 128
	menusInstalled = FALSE;	// No menus yet
	appleMenu = NULL;
	fileMenu = NULL;
	editMenu = NULL;
}

void Dragon::Start (void)
{
	OSErr	err;
	Boolean	inForeground;

	InitMem ();
	InitMac ();
	InitMilieu ();
	InitPrefs ();
	if (!autoQuit)				// Don't bother with menus if we're just going to quit right away
		SetUpMenus ();
	FlushEvents (everyEvent, 0);

// BEGIN bug-fix 1.1.1 — This was formerly done in Dragon::Dragon, but then inForeground was always being set to FALSE,
//	even if the dragon was started in the foreground; consequently, drop-launch processing was slowed down considerably
	// We hope to start in the foreground, but we may not — the Finder launches applications in the foreground, but
	//	somebody else might launch us in the background.  Don't fight it, just grin and bear it.  If something goes
	//	wrong, the safer assumption is that we're in the background
	err = StartAppInForeground (&inForeground);
	if (err != noErr)
		inForeground = FALSE;
		
	runState = (inForeground ? kFGIdle : kBGIdle);	// We're idle until we get an 'odoc' event
	sleepTime = sleepValue[runState];				// Make sure sleepTime is set correctly — sleepValue[0..3] will
											//	have been set correctly by this time (namely, in InitPrefs)
// END bug-fix
}

void Dragon::Run (void)
{
	EventRecord	event;
	
	while (running) {
		if (WaitNextEvent (everyEvent, &event, sleepTime, NULL))
			DoEvent (&event);
		else if (numAEsPending > 0)		// This code won't get called if we're busy, so we don't
			ResumeAEvent ();			//	need to check for runState & maskBusy here
		else
			DoIdle ();
	}
}

void Dragon::Finish (void)
{
	if (preferences != NULL)
		delete preferences;
		
	FlushAEventQueue ();
	delete aeQueue;
	
	/* What else is there to do?
		Release any temporary memory
		Delete any temporary files
		Remove any trap patches
		etc. (look at CApplication.c in the THINK Class Library for some ideas)
	*/
}

//	-------------------------------------------	I n i t i a l i z a t i o n     m e t h o d s	----------------------------------------------

void Dragon::InitMem (void)
{	
	// If you want to increase the stack size, right here (before MaxApplZone) is where to do it
	MaxApplZone ();
	CallMoreMasters ();
}

void Dragon::CallMoreMasters (void)
{
	short		mmCalls, **mmCallsHandle;
	
	// Call MoreMasters the number of times specified in the 'MoMa' resource — each call gives
	//	us an extra block of master pointers
	mmCallsHandle = (short **) GetResource ('MoMa', 128);
	if (mmCallsHandle != NULL) {
		for (mmCalls = **mmCallsHandle; mmCalls > 0; mmCalls--)
			MoreMasters ();
		HPurge ((Handle) mmCallsHandle);
	}
}

void Dragon::InitMac (void)
{
	InitGraf (&thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (NULL);
	InitCursor ();
}

void Dragon::InitMilieu (void)
{
	OSErr		err;
	long			result;
	Boolean		hasAppleEvents;
	GstCheckList	**checkList;
	
	if (!TrapAvailable (0xA1AD))		// 0xA1AD == _Gestalt
		Abort (eInsufficientSystem);
	
	checkList = (GstCheckList **) Get1Resource (kGstCheckType, rGstChecklist);
	err = GestaltBatchCheck (checkList);
	if (err != noErr)
		Abort (err);
	
	HPurge ((Handle) checkList);
	InitAppleEvents ();
}

void Dragon::InitAppleEvents (void)
{
	OSErr	err;
	
	// Make the Apple event queue object, which stores suspended Apple events for handling later
	aeQueue = MakeAEQObject ();
	if (aeQueue == NULL)
		Abort (memFullErr);
	
	// Install Apple event handler functions — abort if we get a non-zero result from AEInstallEventHandler
	if (AEInstallEventHandler (kCoreEventClass, kAEOpenApplication, HandleOapp, 0, FALSE)
		|| AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments, HandleOdoc, 0, FALSE)
		|| AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments, HandlePdoc, 0, FALSE)
		|| AEInstallEventHandler (kCoreEventClass, kAEQuitApplication, HandleQuit, 0, FALSE)
	)
		Abort (eCouldntInstallAppleEvents);
}

AppleEventQueue *Dragon::MakeAEQObject (void)
{
	return new AppleEventQueue;
}

//	-------------------------------------------	P r o c e s s i n g     m e t h o d s	----------------------------------------------

OSErr Dragon::ProcessDroppings (AEDescList *docList)
{
	// We've received an 'odoc' event and are ready to process the docs (files, folders and volumes) in the event
	
	// NOTE:	If you are upgrading from Dragonsmith 1.0b2, do NOT override this method! — read the documentation for
	//		instructions (you need to override ProcessFile and/or ProcessDirectory instead)
	
	long			numDocs, i, actualSize;
	Boolean		wasAlias;
	OSErr		err;
	AEKeyword	keyword;
	DescType	returnedType;
	
	err = AECountItems (docList, &numDocs);
	if (err == noErr) {
		BeginProcessing ();
// NOTE bug-fix 1.1.1 — added test of running to for-loop condition
		for (i = 1; i <= numDocs && running && !abortProcessing; i++) {
			err = AEGetNthPtr (docList, i, typeFSS, &keyword, &returnedType,
														(Ptr) curDocFSS, sizeof (FSSpec), &actualSize);
			if (err == noErr) {
				err = FSpToPBCatInfo (curDocPB, curDocFSS, resolveAliases, followAliasChain, &wasAlias);
				if (err == noErr)
					ProcessDoc ();
			}
		}
		err = noErr;
		EndProcessing ();
	}
	
	return err;
}

void Dragon::BeginProcessing (void)
{
	// Override if you need to do anything special before processing any docs — for example, to interact with the user.
	//	Remember, you can always bail out by calling StopProcessing
	
	// NOTE:	Your subclass's method should call inherited::BeginProcessing (or duplicate its functionality)
	
	// Default behavior —
	//	1.	Turn on the busy flag in runState and adjust sleepTime accordingly
	//	2.	Disable menus
	//	3.	Put up a busy cursor
	
	curDirDepth = 0;				// We haven't yet delved down into any folders or disks
	abortProcessing = FALSE;		// Nothing's gone wrong yet…
	
	runState |= maskBusy;
	sleepTime = sleepValue[runState & (maskInBG | maskBusy)];
	if (menusInstalled) {
		AdjustMenusBusy ();
		DrawMenuBar ();			// Disabling an entire menu requires us to call DrawMenuBar afterwards
	}
	CursorBusy ();
}

void Dragon::EndProcessing (void)
{
	// Surely someone will think of something else to do here!

	// NOTE:	Your subclass's method should call inherited::EndProcessing
	
	// Default behavior —
	//	1.	Turn on the busy flag in runState and adjust sleepTime accordingly
	//	2.	Enable menus
	//	3.	Restore the arrow cursor
	
	curDirDepth = 0;				// Reset, just to be cautious …
	
	runState &= ~maskBusy;
	sleepTime = sleepValue[runState & (maskInBG | maskBusy)];	// Mask out any bits other than the bg and busy ones, for safety
	if (menusInstalled) {
		AdjustMenusIdle ();
		DrawMenuBar ();			// Enabling an entire menu requires us to call DrawMenuBar afterwards
	}
	CursorIdle ();
}

void Dragon::AdjustMenusBusy (void)
{
	// We're about to start processing docs — disable any menu items that should not be selected (generally speaking,
	//	this means everything but the Apple and Edit menus)
	
	DisableItem (appleMenu, 1);		// Disable the About… item
	DisableItem (fileMenu, 0);		// Disable the entire File menu
}

void Dragon::AdjustMenusIdle (void)
{
	// Enable menu items that were disabled when we started processing docs
	
	EnableItem (appleMenu, 1);		// Enable the About… item
	EnableItem (fileMenu, 0);		// Enable what we disabled in AdjustMenusBusy — this will NOT enable items that
								//	were disabled to begin with
}

void Dragon::CursorBusy (void)
{
	// Put up a cursor to show the user we're working on it (only if we're in the foreground)
	if (!(runState & maskInBG))
		SetCursor (*busyCursor);
}

void Dragon::CursorIdle (void)
{
	// Restore the arrow cursor
	InitCursor ();
}

void Dragon::ShowProgress (void)
{
	// This is a good place to let the user know you're working on things — spin a cursor, show a progress bar (hard to
	//	do if your dragon looks inside folders, I'm afraid), etc.
}

void Dragon::DoBusy (void)
{
	// Override this to do periodic actions while we're processing docs — it'll be called once before each doc.  This is the
	//	"busy" counterpart to DoIdle

	// Default behavior —
	//	1.	Call ShowProgress to let the user know we're hard at work
	//	2.	Call WaitNextEvent — yield some time to background processes, let the user cancel the docs processing,
	//			switch to another process, or launch something from the Apple menu (if it exists), etc. Note that this
	//			may result in an Apple event being generated (including 'quit', which lands us in StopRunning)
	
	EventRecord	event;
	Boolean		canAbort;
	
	ShowProgress ();
	
	if (WaitNextEvent (everyEvent, &event, sleepTime, NULL))		// At this point, sleepTime should be a very low value
		DoEvent (&event);
	
	// Note the absence of an else-clause here (as contrasted with the call to WaitNextEvent in the Run method) — we don't
	//	want to call DoIdle here (after all, we're not idle, we're busy!)
}

void Dragon::ProcessDoc (void)
{
	// If you override this method, make sure you call CanProcessDoc first and bail out if it returns FALSE
	// You might, for example, want to treat volumes and folders differently — you could add a separate method
	//	ProcessVolume that's called if curDocIsVolume == TRUE
	
	DoBusy ();
	
	// We have to check running and abortProcessing before continuing — DoBusy may call StopRunning
	if (running && !abortProcessing && CanProcessDoc ()) {
		if (curDocIsFile) {
			if (curFileCreator == signature)
				ProcessOwnedFile ();
			else
				ProcessFile ();
		} else {
			// It's a directory (either a folder or a volume) — look inside it only if we haven't already gone down as
			//	far as we're allowed (remember, directory levels are zero or negative, never positive)
			if (dirDepthLimit < curDirDepth) {
				curDirDepth--;		// We're going down a level
				ProcessDocsInDirectory (curDocPB->h.fileParam.ioVRefNum, curDocPB->h.fileParam.ioDirID);
				curDirDepth++;	// Pop back up to the level we were at before
			} else if (!filesOnly)
				ProcessDirectory ();
		}
	}
}

Boolean Dragon::CanProcessDoc (void)
{
	OSType	type;
	
	// We have to get a little tricky here — the macro curFileType is only valid for files
	type = curDocIsFile ? curFileType : (curDocIsVolume ? 'disk' : 'fold');
	if (OpenableType (type, acceptableTypes))
		return useCustomFilter ? CustomFilterDoc () : TRUE;
	else
		return FALSE;
}

Boolean Dragon::CustomFilterDoc (void)
{
	return TRUE;		// Default is to not filter out anything
}

void Dragon::ProcessDocsInDirectory (short vRefNum, long dirID)
{
	// NOTE:	The way things are set up in Dragon, this method is called only by ProcessDoc.  If your dragon calls it from one
	//		of its methods, make sure you set up curDirDepth (and dirDepthLimit) correctly beforehand!
	
	// Be very careful when overriding this method — things are set up here to make sure that the values in
	//	*curDocPB and *curDocFSS are valid any time another method might be called
	
	OSErr		err;
	short		i;
	Boolean		wasAliasFile;
		
	// NOTE:	curDocPB->h.hFileInfo.ioNamePtr was set to curDocFSS->name in Dragon::Dragon
	//		above and would only need to be reset if it was changed at some point later on
	
	// Loop through each file/folder in a directory
	for (i = 1, err = noErr; err != fnfErr && !abortProcessing; i++) {
	
		// The loop ends only when we get an fnfErr — any other error simply causes us to skip to the next iteration
		//	(i.e., the next file/folder in the given folder/volume)
		
		// Set up the three fields that together identify the file or folder we want information about — they're
		//	liable to have changed since the last time we were at this point in the loop
		curDocPB->h.fileParam.ioVRefNum = vRefNum;
		curDocPB->h.fileParam.ioDirID = dirID;
		curDocPB->h.fileParam.ioFDirIndex = i;
		
		// Convert curDocPB to curDocFSS, filling in curDocPB and (if appropriate) resolving aliases along the way
		err = PBToFSpCatInfo (curDocPB, curDocFSS, resolveAliases, followAliasChain, &wasAliasFile);
		if (err == noErr)
			ProcessDoc ();
	}
	
	// According to Tech Note #68: Searching All Directories on an HFS Volume, "… stopp[ing] the whole search when
	//	we [encounter] an error … cause[s] trouble with privilege errors on AppleShare volumes"
	// This would seem to indicate we have to set abortProcessing = FALSE here (in case it was set to TRUE somewhere
	//	along the line) — but it doesn't make sense to me, I don't like it, and I won't do it!
}

void Dragon::ProcessFile (void)
{
	// NOTE:	This method, if overridden, can do ANYTHING to *curDocFSS and *curDocPB, UNLESS you've
	//		done some indiscreet overriding of Dragon::ProcessDroppings or Dragon::ProcessDocsInDirectory
	//		You do NOT have to restore any values in *curDocFSS or *curDocPB!!!

	// Of course, you will normally override this method
}

void Dragon::ProcessDirectory (void)
{
	// NOTE:	This method, if overridden, can do ANYTHING to *curDocFSS and *curDocPB, UNLESS you've
	//		done some indiscreet overriding of Dragon::ProcessDroppings or Dragon::ProcessDocsInDirectory
	//		You do NOT have to restore any values in *curDocFSS or *curDocPB!!!

	// The directory ID of the folder or volume we're processing is in the .ioDirID field
}

void Dragon::ProcessOwnedFile (void)
{
	// NOTE:	This method will be called whenever a file created by this dragon is dropped on it or double-clicked.
	//		This is a handy way to change preferences quickly (if one is sneaky, it can be done in the middle of a
	//		dropped batch)
	
	if (curFileType == prefsFileType)
		if (preferences->UseFile (curDocFSS))
			ReadPrefs ();
}

void Dragon::StopProcessing (OSErr err)
{
	// Your class should override this method if it wants to beep, display an error message, etc. — just make sure your
	//	method calls inherited::StopProcessing
	
	abortProcessing = TRUE;
}

void Dragon::SaveDocInfo (Boolean refreshFinder)
{
	/* Call this method right after you change one or more of the following values —
	
				curFileType	curFileCreator		curFileFlags	curDocCreated	curDocModified
				
	or any other field in *curDocPB that PBSetCatInfo takes as input.  To rename the current document, you have to call
	PBHRename rather than just changing *curDocName
	
	WARNING:	If your dragon makes any low-level calls (PB____) that change any other values in *curDocPB that
				PBSetCatInfo takes as input, then it should do so AFTER calling this method, or save and restore
				those values, or refrain from calling UpdateCatInfo at all.  For a PARTIAL list of low-level File Manager
				calls that leave garbage in fields that PBSetCatInfo uses, see the Dragonsmith Programmer's Manual
	*/

	long		ioDirIDWas;
	short	ioFDirIndexWas;
	OSErr	err;
	
	// Save field values
	ioDirIDWas = curDocPB->c.hFileInfo.ioDirID;		// File number or directory ID, returned by PBGetCatInfo
	ioFDirIndexWas = curDocPB->c.hFileInfo.ioFDirIndex;		
	
	// Give them correct values for PBSetCatInfo
	curDocPB->c.hFileInfo.ioDirID = curDocPB->c.hFileInfo.ioFlParID;
	curDocPB->c.hFileInfo.ioFDirIndex = 0;
	
	// Call PBSetCatInfo for the current document
	err = PBSetCatInfoSync ((CInfoPBPtr) curDocPB);
	
	// Restore the original values
	curDocPB->c.hFileInfo.ioDirID = ioDirIDWas;
	curDocPB->c.hFileInfo.ioFDirIndex = ioFDirIndexWas;
	
	// Refresh the Finder's display of the current document, if desired — we use the FSpRefreshFinderDisplay
	//	function from FileUtils.c rather than duplicating its functionality here

	if (refreshFinder && err == noErr)
		FSpRefreshFinderDisplay (curDocFSS);
}

//	-------------------------------------------	E v e n t s     m e t h o d s	----------------------------------------------

void Dragon::DoEvent (EventRecord *event)
{
	switch (event->what) {
		case mouseDown:
			DoMouseDown (event);
			break;
		case mouseUp:
			DoMouseUp (event);
			break;
		case keyDown:
		case autoKey:
			DoKeyDown (event);
			break;
		case activateEvt:
			if (event->modifiers & activeFlag)
				DoActivate (event);
			else
				DoDeactivate (event);
			break;
		case updateEvt:
			DoUpdateEvent (event);
			break;
		case diskEvt:
			DoDiskInsert (event);
			break;
		case osEvt:				// Suspend, resume, and mouse-moved events
			DoOSEvent (event);
			break;
		case kHighLevelEvent:
			DoHighLevelEvent (event);
			break;
	}
}

void Dragon::DoMouseDown (EventRecord *theEvent)
{
	WindowPtr	whichWindow;
	short		domain;
	long			menuItemCode;
	
	domain = FindWindow (theEvent->where, &whichWindow);
	switch (domain) {
		case inSysWindow:
			SystemClick (theEvent, whichWindow);
			break;
		case inMenuBar:
			// Watch out for stray mouseDowns in non-existent menus …
			if (menusInstalled) {
				menuItemCode = MenuSelect (theEvent->where);
				if (menuItemCode > 0x0000FFFF) {
					DoMenu (menuItemCode);
					ShowMenuAction ();
				}
			}
			break;
		default:
			break;
	}
}

void Dragon::DoMouseUp (EventRecord *theEvent)
{
	// Do nothing
}

void Dragon::DoKeyDown (EventRecord *theEvent)
{
	long		menuItemCode;
	
	if (runState & maskBusy && IsCancelEvent (theEvent))
		StopProcessing (userCanceledErr);
	else if ((theEvent->modifiers & cmdKey) && menusInstalled) {		// If we have menus, handle
		menuItemCode = MenuKey (theEvent->message);			// 	cmd-key combos
		if (menuItemCode > 0x0000FFFF) {							// The high word of menuItemCode will be
			DoMenu (menuItemCode);							//	zero (and the low word undefined) if
			ShowMenuAction ();								//	no menu item was chosen
		}
	}
}

void Dragon::DoActivate (EventRecord *theEvent)
{
	// Null method — override if you have windows
}

void Dragon::DoDeactivate (EventRecord *theEvent)
{
	// Null method — override if you have windows
}

void Dragon::DoUpdateEvent (EventRecord *theEvent)
{
	// Null method — override if you have windows
}

void Dragon::DoDiskInsert (EventRecord *theEvent)
{
	Point	pt = {100, 100};		// Would { -1, -1 } give us a centered dialog??
	
	if ((short) (theEvent->message >> 16) != noErr)		// If the disk isn't formatted (or has problems),
		(void) DIBadMount (pt, theEvent->message);	// 	give the user the opportunity to initialize it
}

void Dragon::DoOSEvent (EventRecord *theEvent)
{
	switch ((theEvent->message >> 24) & 0x00FF) {		// High byte tells us what kind of event it is
		case suspendResumeMessage:			
			if (theEvent->message & resumeFlag)
				DoResume ();
			else
				DoSuspend ();
			break;
		case mouseMovedMessage:
			break;
		default:
			break;
	}
}

void Dragon::DoHighLevelEvent (EventRecord *theEvent)
{
	OSErr	err;
	
	// Don't assume all high level events are Apple Events
	if (((HLEventPtr) theEvent)->eventClass == kCoreEventClass)
		err = AEProcessAppleEvent (theEvent);
	else
		DoOtherHLEvent (theEvent);

			//  NOTE:	When running in the THINK C Debugger, you have to be careful not to set any
			//		breakpoints between the call to WaitNextEvent and the call to AEProcessAppleEvent
			//		which dispatches to the Apple event handlers (HandleOdoc etc.).  If you forget this,
			//		the call to AEProcessAppleEvent will return noOutstandingHLE (== -608)

}

void Dragon::DoOtherHLEvent (EventRecord *theEvent)
{
	// Subclasses may want to handle other high-level events
}

void Dragon::DoSuspend (void)
{
	// Turn on the in-background bit in runState and adjust sleepTime accordingly
	runState |= maskInBG;
	sleepTime = sleepValue[runState & (maskInBG | maskBusy)];
	
	// If we're busy, stop the busy cursor (boring watch, spinning cursor, back-flipping dogcow, whatever)
	if (runState & maskBusy)
		CursorIdle ();
}

void Dragon::DoResume (void)
{
	// Turn off the in-background bit in runState and adjust sleepTime accordingly
	runState &= ~maskInBG;
	sleepTime = sleepValue[runState & (maskInBG | maskBusy)];
	
	// If we're busy, put the "busy" cursor back up
	if (runState & maskBusy)
		CursorBusy ();
}

void Dragon::DoIdle (void)
{
	// Override this method to do periodic actions while we're sitting around waiting for something to happen.  This is the
	//	"idle" counterpart to DoBusy (which is called for each doc we try to process) — it'll never be called while we're
	//	processing docs
}

void Dragon::StopRunning (OSErr err)
{
	if (runState & maskBusy)			// If we're processing docs, we have to set abortProcessing = TRUE or
		StopProcessing (eForcedQuit);	//	else we'll blithely continue to process 'em (bad idea!)
	running = FALSE;
}

//	-------------------------------------------	M e n u     m e t h o d s	----------------------------------------------

void Dragon::SetUpMenus (void)
{
	// NOTE:	Your sub-class of Dragon probably won't need to call SetUpMenus from "outside" (meaning anywhere
	//		but in an overridden SetUpMenus method).  If it does, make sure you DON'T call it unless menusInstalled
	//		== FALSE.  Otherwise, you'll have extra menus (or a cute little picture of a bomb…)
	
	// Override this method if you have any menus in addition to the Apple, File, and Edit menus
	// Make sure your method calls inherited::SetUpMenus at the beginning and DrawMenuBar at the end, so that —
	//	1.	Dragon's menus get initialized
	//	2.	The menu bar gets drawn

	appleMenu = GetMenu (mApple);		// Standard Apple Menu
	AddResMenu (appleMenu, 'DRVR');
	InsertMenu (appleMenu, 0);
	
	fileMenu = GetMenu (mFile);
	InsertMenu (fileMenu, 0);
	
	editMenu = GetMenu (mEdit);
	InsertMenu (editMenu, 0);
	
	DrawMenuBar ();
	menusInstalled = TRUE;
}

void Dragon::DoMenu (long menuItemCode)
{
	// Override this method if you need more than just the Apple, File, and Edit menus
	
	short	menuID, itemNum;

	menuID = menuItemCode >> 16;
	itemNum = menuItemCode & 0xFFFF;

	switch (menuID) {
		case mApple:
			DoAppleMenu (itemNum);
			break;
		case mFile:
			DoFileMenu (itemNum);
			break;
		case mEdit:
			DoEditMenu (itemNum);
			break;
		default:
			break;
	}
}

void Dragon::DoAppleMenu (short itemNum)
{
	// Implement standard Apple menu functionality
	
	Str255	itemStr;
	
	if (itemNum == iAbout)
		DoAbout ();
	else {
		GetItem (appleMenu, itemNum, itemStr);
		OpenDeskAcc (itemStr);
	}
}

void Dragon::DoAbout (void)
{
	// Override this method if you want an About… window
}

void Dragon::DoFileMenu (short itemNum)
{
	if (itemNum == CountMItems (fileMenu))		// This will work fine no matter how many items you have
										//	in your File menu, as long as Quit is at the end
// BEGIN bug-fix 1.1.1 — Trigger a 'quit' event instead of just setting a flag
		(void) SendToSelf (kCoreEventClass, kAEQuitApplication, NULL, NULL, kAEAlwaysInteract | kAENoReply);
// END bug-fix 1.1.1
}

void Dragon::DoEditMenu (short itemNum)
{
	// SystemEdit does undo, cut, copy, paste, and clear for us
	if (itemNum <= 6)
		(void) SystemEdit (itemNum - 1);
}

OSErr Dragon::DoOapp (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	// Override this method if you want your dragon to do something special when it's launched
	//	by double-click in the Finder (like put up a status window, start a directory scan, etc.)
	
	if (autoQuit)			// Should we quit immediately?
		StopRunning (eNormalQuit);

	return noErr;
}

OSErr Dragon::DoOdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	// You should't need to override this method — it calls ProcessDroppings with info gleaned from the 'odoc' event
	
	register OSErr		err;
	AEDescList		docList;
	
	// If we're already processing an Apple event, call SuspendAEvent instead
	if (runState & maskBusy)
		return SuspendAEvent (theAppleEvent, theReply, &HandleOdoc, refcon);
	
	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err == noErr) {
		err = GotRequiredParams (theAppleEvent);
		if (err == noErr)
			err = ProcessDroppings (&docList);
		(void) AEDisposeDesc (&docList);
	}
		
	if (autoQuit)				// Should we quit immediately after handling this event?
		StopRunning (eNormalQuit);
		
	return err;
}

OSErr Dragon::SuspendAEvent (AppleEvent *event, AppleEvent *reply, AEHandlerFunc *handler, long refcon)
{
	// Postpone processing of the current Apple event till we're done with the current one (and any previously
	//	postponed ones)
	
	OSErr	err;
	
	err = aeQueue->Put (event, reply, handler, refcon);
	if (err == noErr) {
		numAEsPending++;
		err = AESuspendTheCurrentEvent (event);
	}
	
	return err;
}

void Dragon::ResumeAEvent (void)
{
	AppleEvent		event, reply;
	AEHandlerFunc	handler;
	long				refcon;
	OSErr			err;
	
	err = aeQueue->Get (&event, &reply, &handler, &refcon);
	if (err == noErr && handler != NULL)
		err = AEResumeTheCurrentEvent (&event, &reply, handler, refcon);
		
	if (--numAEsPending < 0)
		numAEsPending = 0;
}

void Dragon::FlushAEventQueue (void)
{
	AppleEvent		event, reply;
	AEHandlerFunc	handler;
	long				refcon;
	OSErr			err;
	
	for (err = noErr; err != noOutstandingHLE; ) {
		err = aeQueue->Get (&event, &reply, &handler, &refcon);
		if (err == noErr && handler != NULL)
			// "Resume" here means we call ReturnEventNotHandled, which just returns errAEEventNotHandled
			(void) AEResumeTheCurrentEvent (&event, &reply, &ReturnEventNotHandled, refcon);
	}
}

OSErr Dragon::DoPdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	if (autoQuit)						// Should we quit immediately after handling this event?
		StopRunning (eNormalQuit);
		
	return errAEEventNotHandled;		// Most subclasses won't need to override this method
}

OSErr Dragon::DoQuit (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	StopRunning (eNormalQuit);			// This'll keep us from going through the main event loop again
	return noErr;
}

//	-------------------------------------------	P r e f e r e n c e s     m e t h o d s	----------------------------------------------

void Dragon::InitPrefs (void)
{
	FSSpec	prefsFile;
	
	// NOTE:	The application's resource fork must be the most-recently-used one at this
	//		point — you won't have to worry about this unless you do some heavy-duty
	//		overriding of Dragon methods (::InitPrefs, ::Start, ::Dragon)
	// TIP:	Always treat Get1Resource and Count1Resources (etc.) with great respect!
	
	preferences = MakePrefsObject ();
	if (preferences == NULL)
		Abort (memFullErr);
	else {
		if (!preferences->Init (appResFork, signature, prefsFileType))
			Abort (eInitializationFailed);				// Abort if preferences initialization failed completely
		if (FindPrefsFile (&prefsFile))
			(void) preferences->UseFile (&prefsFile);	// Ignore the returned value — we'll call ReadPrefs regardless
		ReadPrefs ();
	}
}

void Dragon::ReadPrefs (void)
{
	// Read in any needed prefs resources
	// If your subclass overrides this method, it must call inherited::ReadPrefs
	// When overriding, check for NULL handles — do the same in any other method that might rely on a prefs
	//	resource being in memory.  A screwed-up preferences file should never cause big problems (i.e., crash).
	//	You'll notice that this is the only method in this file that uses the dragonPrefs instance variable — the only
	//	reason it's provided as an instance variable is so that a subclass can change the Dragon preferences in
	//	the preferences files it uses

	// Note that you should NOT reference any preferences resource handles that may have been in memory
	//	— they will not be valid handles now because the file they were from has by this time been closed
	
	short	flags, i;

	if (dragonPrefs != NULL)
		preferences->ReleasePrefResource (prefDragonPrefs);
		
	dragonPrefs = (DragonPrefsRec **) preferences->GetPrefResource (prefDragonPrefs);
	if (dragonPrefs != NULL) {
		// Read Boolean settings from the miscFlags field of the Dragon preferences resource
		flags = (*dragonPrefs)->miscFlags;
		filesOnly = flags & maskFilesOnly;
		resolveAliases = flags & maskResolveAliases;
		followAliasChain = flags & maskFollowAliasChain;
		autoQuit = flags & maskAutoQuit;
		
		// Read the 4 possible sleepTime values from the Dragon preferences resource
		for (i = 0; i <= 3; i++)
			sleepValue[i] = (*dragonPrefs)->sleep[i];
		sleepTime = sleepValue[runState & (maskInBG | maskBusy)];
		
		// Read dirDepthLimit from the Dragon preferences resource
		dirDepthLimit = (*dragonPrefs)->depthLim;
	}
}

Preferences *Dragon::MakePrefsObject (void)
{
	// Override this method if you write your own subclass of Preferences and want to use it instead
	return new Preferences;
}

Boolean Dragon::FindPrefsFile (FSSpec *fss)
{
	OSErr			err;
	Str63			fileName;
	unsigned short	len;
	Handle			h = NULL;
	short			saveResFork;
	
	saveResFork = CurResFile ();
	UseResFile (appResFork);
	h = Get1Resource ('STR ', rPrefsFileName);
	UseResFile (saveResFork);
	if (h != NULL) {
		len = **((unsigned char **) h);
		if (len != 0 && len <= 63) {	
			HLock (h);
			CopyPStr ((unsigned char *) *h, fss->name);
			HUnlock (h);
			HPurge (h);
	
			fss->vRefNum = appFile.vRefNum;					// First look in the directory that the application is in
			fss->parID = appFile.parID;
			if (IsPrefsFile (fss))
				return TRUE;
				
			err = FSpFindFolder (kPreferencesFolderType, fss);	// Then look in the Preferences folder
			if (IsPrefsFile (fss))								// If there's a preferences file there,
				return TRUE;								//	then we're done
			else											// Otherwise,
				return MakePrefsFile (fss);					//	we create one there
		}
	}
	
	if (h != NULL)
		HPurge (h);
	
	return FALSE;			// If we encountered any problems, then we'll rely on the app file for our preferences resources
}

Boolean Dragon::MakePrefsFile (FSSpec *fss)
{
	OSErr	err;
	
	FSpCreateResFile (fss, signature, prefsFileType, smSystemScript);
	return (ResError () == noErr);
}

Boolean Dragon::IsPrefsFile (FSSpec *fss)
{
	// This method returns TRUE if the file designated by fss is a preferences file for this dragon
	// NOTE:	IsPrefsFile may alter *fss, since we tell FSpToPBCatInfo to resolve aliases.  This is a feature, not a bug!

	OSErr		err;
	Boolean		isFolder, wasAliasFile;
	PBRecUnion	pb;
	Str63		name;
	
	pb.c.hFileInfo.ioCompletion = NULL;
	pb.c.hFileInfo.ioNamePtr = (StringPtr) &name;
	
	err = FSpToPBCatInfo (&pb, fss, TRUE, TRUE, &wasAliasFile);
	return (err == noErr && PBIsFile (&pb) && PBFileCreator (&pb) == signature && PBFileType (&pb) == prefsFileType);
}

//	-------------------------------------------	M i s c e l l a n e o u s	----------------------------------------------

Boolean Dragon::InteractWithUser (long timeOut)
{
	// Call this method any time during the handling of an Apple event (I don't know what will happen
	//	if there's no current Apple event) if you need to come to the foreground (to pose a dialog, etc.)
	
	return AEInteractWithUser (timeOut, NULL, &CallWaitIdle) == noErr;
}

Boolean Dragon::WaitIdle (EventRecord *theEvent, long *sleep, RgnHandle *mouseRgn)
{
	// This method is called whenever we get an event while AEInteractWithUser is in process
	// The system will give us only null, OS, activate, and deactivate events
	
	*sleep = sleepTime;			// These two assignments tell AEInteractWithUser what to
	*mouseRgn = cursorRgn;	//	pass to WaitNextEvent on our behalf
	
	DoEvent (theEvent);
	
	return FALSE;				// Don't abort AEInteractWithUser unless it times out
}

void Dragon::Abort (short errNum)
{
	Error (errNum);
	ExitToShell ();
}

void Dragon::Error (short errNum)
{
	Str255	string;
	
	NumToString (errNum, string);
	ParamText (string, "\p", "\p", "\p");
	Alert (rErrorAlert, NULL);
}

//	-------------------------------------------	F u n c t i o n s			----------------------------------------------

pascal OSErr HandleOapp (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return gDragon->DoOapp (theAppleEvent, theReply, refcon);
}

pascal OSErr HandleOdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return gDragon->DoOdoc (theAppleEvent, theReply, refcon);
}

pascal OSErr HandlePdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return gDragon->DoPdoc (theAppleEvent, theReply, refcon);
}

pascal OSErr HandleQuit (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return gDragon->DoQuit (theAppleEvent, theReply, refcon);
}

pascal OSErr ReturnEventNotHandled (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return errAEEventNotHandled;
}

pascal Boolean CallWaitIdle (EventRecord *theEvent, long *sleep, RgnHandle *mouseRgn)
{
	// Idle function to be used as parameter to AEInteractWithUser
	
	return gDragon->WaitIdle (theEvent, sleep, mouseRgn);
}

