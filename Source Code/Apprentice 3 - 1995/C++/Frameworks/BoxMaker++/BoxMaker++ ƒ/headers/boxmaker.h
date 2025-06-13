#pragma once
//
// This is a hack which is intended to make boxmaker compile under
// both the old and the universal headers. It probably doesn't work since
// (I don't have a _complete_ set of universal headers to check it with)
//
#if !NEW_HEADERS_AVAILABLE
	typedef EventHandlerProcPtr AEEventHandlerUPP;
	#define NewAEEventHandlerProc(entryPoint) (AEEventHandlerUPP)(entryPoint)
#endif
//
// A shell's status will normally start at 'running', and then move either
// to 'OApped' or 'finishing'. When a quit Apple event is received status
// automatically moves to 'quitting'. The actual exit is only made when
// 'EventloopHook' (a virtual function described below) returns true, and the
// status is either 'finishing' or 'quitting'.
//
typedef enum shell_status
{
	shell_is_running,		// shell is running normally
	shell_is_OApped,		// running normally, received OApp event
	shell_is_finishing,		// started by ODOC, will quit on idle
	shell_is_quitting		// Quit Apple Event received.
};

class boxmaker : private boxmakergetfile
{
	public:
		boxmaker( short dlogID = sfGetDialogID);
		~boxmaker();
		//
		// A 'normal' main for a boxmaker is:
		//
		//		void main()
		//		{
		//			myboxmaker RedBand;
		//			RedBand.run();
		//		}
		//
		// the member function 'run' is needed because the 'run' can't be executed by
		//
		// - boxmaker's constructor because myboxmaker hasn't been constructed then
		// - boxmaker's destructor because myboxmaker has already been destructed then
		//
		// Alternatively, myboxmaker could call 'run' at the end of its constructor,
		// but there is no way to enforce that, so that method does not have any
		// advantage over the one chosen here.
		//
		void run();

	protected:
		//
		// These are the actual routines to override.
		//
		// OpenDoc is the only one routine which _must_ be overridden.
		// It should use either the information in the 'theCInfoPBRec' field or the
		// information in the 'theFSSpec' field to determine the file to be processed.
		// Note: OpenDoc is called before any of the 'mayEnterFolder', 'EnterFolder',
		// 'ExitFolder', or 'CantEnterFolder' routines is called.
		//
		virtual void OpenDoc( Boolean opening) = 0L;
		//
		// mayEnterFolder should return a flag which indicates whether we should
		// enter this particular folder. Normally you will make it to always return
		// either 'true' or 'false', but you could also make it more complex.
		// Thus, one can implement what Symantec calls 'shielded folders', or process
		// all items of folders dropped upon the boxmaker, but not items in folders
		// inside those folders. If 'mayEnterFolder' returns false, the folder is not
		// entered; thus 'EnterFolder' nor 'ExitFolder' will be called for that folder.
		//
		virtual Boolean mayEnterFolder( Boolean opening) { return true;};
		//
		// EnterFolder and ExitFolder are called during a recursive descent.
		// Under some circumstances you will receive both an OpenDoc and an
		// EnterFolder message for folders encountered.
		// The difference is that 'EnterFolder' is always called, and 'OpenDoc'
		// is only called for a folder if the drop box can handle folders, as
		// specified via the 'theFlags' parameter to its constructor.
		//
		virtual void EnterFolder( Boolean opening) {};
		virtual void ExitFolder( Boolean opening) {};
		//
		// CantEnterFolder is called when we encounter a folder to which
		// we do not have read access. In that case EnterFolder and ExitFolder
		// are not called for the folder (if passFolders is set, OpenDoc will
		// be called for the folder). Note: there are two different versions
		// of read access, namely seeing files and seeing folders. You can
		// use the 'getAccessRights' member to study the access rights.
		// CantEnterFolder is called only when you have neither the see files
		// nor the see folders access rights to a folder. For more complex
		// selections you should override mayEnterFolder (see above)
		//
		virtual void CantEnterFolder( Boolean opening) {};
		//
		// OpenApp is called whenever the application receives an 'Open application'
		// Apple Event. This happens whenever somebody opens the application without
		// opening a document. Normally this is done by double-clicking the application,
		// or by selecting 'Open' in the Finder's file menu after selecting the application.
		// OpenApp is _not_ called when one drops files on the application, since the
		// application will not receive an 'Open application' Apple event then.
		// The default OpenApp simply shows the preferences dialog.
		//
		virtual void OpenApp() { ShowPreferences();};
		//
		// StartABunch and EndABunch are called at the start and end of the processing
		// of an 'ODOC' event. The difference with 'OpenDoc' is that StartABunch/EndABunch
		// are called exactly one time for every 'ODOC' Apple Event, not for every file
		// or folder specified in that Apple Event. Also, the number of items specified
		// does not include any items in subfolders, and it does not check whether the
		// items are indeed processable by your application (e.g. when the 'BNDL' and 'typs'
		// resources of your boxmaker do not match, or when the boxmaker is sent an
		// Apple Event by AppleScript). The StartABunch/EndABunch combo can for instance
		// be used to pop up a dialog requesting confirmation of a tricky operation, or
		// to display and hide a progress box.
		// The default versions of StartABunch and EndABunch are:
		// StartABunch	: set the cursor to a watch cursor, disable all menus
		// EndABunch	: reset the cursor to a NM arrow, enable all menus
		//
		virtual void StartABunch( long numTopLevelItems, Boolean opening);
		virtual void EndABunch( long numTopLevelItems, Boolean opening);
		//
		// Routines to override only when one adds menus and/or menu items:
		//
		// If one changes the menus (boxmaker installs menu bar #128 and adds
		// the desk accessories to menu #128) one should also override 'DoMenu'.
		// To assist in implementing ones own 'DoMenu', the functions 'DoAppleMenu',
		// 'SelectFile', 'ShowPreferences', and 'SendQuitToSelf' are provided.
		//
		virtual void DoMenu( long retVal);
		//
		// SelectFile by default lets one select _any_ file. Limiting the selection
		// to certain file types can be done by adding a 'typs' resource #128 to
		// the Application. For more complex selections override 'SelectFile'.
		//
		virtual void SelectFile();
		//
		// HandleDialogEvent is called whenever DialogSelect returns true.
		// Normally one should just look at 'itemHit'; theDialog can be used to
		// access the actual items, theEvent is passed because somebody may find
		// a use for it.
		// To change items from those specified in the resource (e.g. to change
		// them to reflect the saved preferences), use the constructor of
		// 'yourboxmaker'.
		// The parameter 'theDialog' usually equals 'gMainDialog'. It is included
		// for reasons of flexibility. This way, a boxmaker can display multiple
		// dialogs, and determine which dialog a event is meant for.
		//
		virtual void HandleDialogEvent( short itemHit, DialogPtr theDialog) {};
		//
		// EventloopHook is called everytime through the main event loop. As all
		// virtual members it can examine and change the 'myEvent' and 'gotEvent'
		// fields.
		// It can for instance be used in a boxmaker which does do a lengthy
		// operation on files dropped on it. The 'OpenDoc' member function would
		// then put the file or folder to be processed in a list, and 'EventloopHook'
		// would take an item of the list, and process it partially. This way one can
		// build a boxmaker which does call WaitNextEvent during processing of a file,
		// and thus does not hog the Mac it is run on.
		// (950116: Currently, the directory traversal routines do call WaitNextEvent
		// every now and then. This was done since a lengthy traversal (e.g. of a
		// one Gigabyte file server volume) already seriously hogs the Mac.
		//
		// EventloopHook is called directly after 'WaitNextEvent', before the event
		// is processed. It is passed a Boolean which indicates whether the boxmaker
		// itself would want to quit now (e.g. because the user chose 'Quit' in the
		// file menu). It should return a boolean which indicates whether your boxmaker
		// _can_ quit now. It is up to your boxmaker to decide what to do when its
		// 'EventloopHook' is called with a value of true. Possible scenarios are
		// (more or less in order of increased user-friendliness):
		//
		// - return true only when all file processing is done.
		// - abort processing of files and return true.
		// - return true and let your shell's destructor abort processing.
		// - ask the user whether to abort or continue, and return false until
		//   all processing is done.
		// - ask the user whether to abort or continue, and return false until
		//   all processing is done, and putting up a dialog explaining what is
		//   going on, with a cancel button on it.
		// When your EventloopHook does temporarily 'refuse' to quit when asked it
		// probably is a good idea to let your opendoc handler return the value
		// 'errAEEventNotHandled' after the quit request is received.
		//
		shell_status the_status;
		virtual Boolean EventloopHook() { return true;};
		//
		// If your OpenDoc handler performs a lengthy operation you might want to call
		// 'TimeForTea' every now and then in your OpenDoc handler.
		// TimeForTea does not always call WaitNextEvent; if it doesn't call it
		// it is fairly efficient, so you don't have to worry much about it slowing
		// down your program.
		//
		void TimeForTea();
		//
		// the 'myEvent' field contains the last event obtained from 'WaitNextEvent'
		// Thus it is available to all boxmaker's virtual member functions.
		// gotEvent contains the value returned by the last call of WaitNextEvent.
		// (this excludes the calls of WaitNextEvent done by TimeForTea);
		//
		EventRecord myEvent;
		short gotEvent;
		//
		// These two fields contain the information about the current file
		// whenever EnterFolder or OpenDoc is called. Both routines can use
		// them for whatever they want to use them, as long as they do not change
		// the fields ioVRefNum or ioNamePtr of 'theCInfoPBRec.hFileInfo'.
		// A safer implementation of boxmaker would make these fields private, but
		// then anybody wanting to use the information in a 'SetCatInfo' would have
		// to allocate a CInfoPBRec and copy the information.
		// When 'ExitFolder' is called the info in theCInfoPBRec does no longer
		// point to the folder we are leaving.
		//
		// If 'theCInfoPBRec' points to a directory theSubDirID is set to the ID
		// of that directory, so 'ExitFolder' may use this field to obtain the
		// directory ID of the folder.
		//
		CInfoPBRec	theCInfoPBRec;
		FSSpec		theFSSpec;
		long		theSubDirID;
		//
		// 'Utility' functions which study theCInfoPBRec:
		//
		// getAccessRights assumes that theCInfoPBRec points to a directory,
		// and returns the access rights, as documented in IM-V-391.
		//
		Boolean itsADirectory() const;
		Boolean itsVisible() const;
		
		char getAccessRights() const;
		//
		// Miscellaneous utility functions:
		//
		static void copyStr63( const Str63 fromString, Str63 toString);
		void SetUp_theFSSpec_from_theCInfoPBRec();
		//
		// itsProcessable checks whether 'theType' is member of the array 'theTypes',
		// taking into account the situation where 'numTypes' equals -1.
		// It also checks whether the file is invisible. itsProcessable can only
		// be called for files. The array 'theTypes' is initialized from the 'typs'
		// resource #128. This resource is supposed to contain the types of file the
		// boxmaker is to process.
		// This list can be restricted further by including a 'tycr' resource #128
		// If it is present it should contain type-creator pairs to accept, with
		// '****' a valid type or creator matching any type/creator.
		//
		Boolean itsProcessable() const;

		Boolean matchesTypeList() const;
		Boolean matchesTypeCreatorPairs() const;
		//
		// ErrorAlert puts up an alert with a message as specified by the
		// resource ID of a 'STR#' resource and an index into the string list.
		// It uses 'ALRT' resource # kErrorAlertID, and returns to its caller.
		// ErrorAlertQuit does not return, but does an 'ExitToShell'.
		//
		static void ErrorAlert( short errorNo,
					short stringIndexID, short stringListID = kErrStringID);
		static void ErrorAlertQuit( short errorNo,
					short stringIndexID, short stringListID = kErrStringID);
		
		DialogPtr		gMainDialog;
		DialogRecord	myDialogRecord;
		//
		// amInFront() returns true when 'we' are the current front procsess.
		//
		Boolean amInFront() const;
		//
		// 'SendQuitToSelf' does just that. It is the standard routine called when the
		// quit menu item is chosen.
		// Similarly, 'DoAppleMenu' is the standard Apple Menu routine, 'SelectFile' the
		// standard 'OpenÉ' item handler, and 'ShowPreferences' is the standard routine
		// called for the 'PreferencesÉ' menu item. They are made accessible to subclasses
		// to make overriding 'DoMenu' easier. (SelectFile is listed above; it is virtual)
		//
		static void SendQuitToSelf();
		void DoAppleMenu( short theItem);
		void ShowAbout();
		void ShowPreferences();
		//
		// errDebug invokes the Debugger when errno is unequal to noErr
		// The second form not only prints the error number, but also the
		// Pascal string specified.
		//
		static void errDebug( OSErr errno);
		static void errDebug( OSErr errno, unsigned char *errString);
		//
		// our own address; used to send events to self. It is obtained once
		// at initialization time, and stays around for the whole lifetime of
		// the boxmaker.
		//
		static const AEAddressDesc self;

		ProcessSerialNumber myPSN;
		//
		// a Handle to the standard watch cursor:
		//
		static const CursHandle theClock;

	private:
		//
		// From now (940728) on most members are made protected, so that subclasses can
		// get at them (e.g. to determine kFileMenuID when DoMenu is overridden, to study
		// theTypes and numTypes in a version of 'SelectFile', to do a SendQuitToSelf, etc).
		// We do keep some private routines and variables, though.
		//
		// The UPPs for the apple event handlers:
		//
		AEEventHandlerUPP OAPPPtr;
		AEEventHandlerUPP ODOCPtr;
		AEEventHandlerUPP PDOCPtr;
		AEEventHandlerUPP QuitPtr;
		//
		// TickCount returns a signed -> time_of_next_cup_of_tea is also signed.
		//
		long time_of_next_cup_of_tea;
		//
		// Program routines
		//
		void InitToolbox();
		void SetUpMenus();
		Boolean InitGlobals();
		void GetMainDialog();
		void DoMouseDown();
		void DoKeyDown();
		//
		// Apple Event stuff
		//
		void InitAEVTStuff();
		static OSErr GotRequiredParams( AppleEvent *theAppleEvent);

		static AEAddressDesc GetTargetToSelf();		
		static void _SendDocsToSelf( AEDescList *aliasList);
		static void SendODOCToSelf( FSSpec *theFileSpec);

		OSErr _HandleDocs( AppleEvent *theAppleEvent,
								AppleEvent *reply, Boolean opening);
		//
		// _HandleOneDoc is actually a misnomer; if the FSSpec happens to
		// indicate a folder it may end up handling multiple files.
		// Anyway, it is called repeatedly by _HandleDocs, once for every file opened.
		//
		OSErr _HandleOneDoc( const FSSpec &aFSSpec, Boolean opening);
		
		OSErr _HandleADir( long dirID, Boolean opening);

		void DoHighLevelEvent();
		//
		// GatherInfo gets info for the file specified in 'theCInfoPBRec' and the parameters
		// passed to it. The first version collects info for the file specified, the second
		// one collects info for a file in the directory specified. The volume reference
		// number is supposed to be present in 'theCInfoPBRec', already.
		//
		OSErr GatherInfo( const FSSpec &aFSSpec);
		OSErr GatherInfo( long theDirID, long index);
		OSErr _GatherInfo( long theDirID);
		//
		// FailErr displays an error alert when 'err' is not zero,
		// and always returns to its caller. It is for internal use.
		//
		static void FailErr( OSErr err);
	//
	// Apple event handles can not be member functions. Making them static should help
	// since the C++ calling format equals that of Pascal, but I haven't gotten it to work.
	// Therefore they are made friends.
	//
	friend pascal OSErr HandleOAPP( AppleEvent *theAppleEvent,
							AppleEvent *reply, long handlerRefcon);
	
	friend pascal OSErr HandleQuit( AppleEvent *theAppleEvent,
							AppleEvent *reply, long handlerRefcon);
	
	friend pascal OSErr HandleODOC( AppleEvent *theAppleEvent,
							AppleEvent *reply, long handlerRefcon);
	
	friend pascal OSErr HandlePDOC( AppleEvent *theAppleEvent,
							AppleEvent *reply, long handlerRefcon);
};

pascal OSErr HandleOAPP( AppleEvent *theAppleEvent,
						AppleEvent *reply, long handlerRefcon);

pascal OSErr HandleQuit( AppleEvent *theAppleEvent,
						AppleEvent *reply, long handlerRefcon);

pascal OSErr HandleODOC( AppleEvent *theAppleEvent,
						AppleEvent *reply, long handlerRefcon);

pascal OSErr HandlePDOC( AppleEvent *theAppleEvent,
						AppleEvent *reply, long handlerRefcon);

inline void boxmaker::FailErr( OSErr error)
{
	if( error != noErr)
	{
		ErrorAlert( error, kAEVTErr);
	}
}

inline Boolean boxmaker::itsADirectory() const
{
	return (theCInfoPBRec.hFileInfo.ioFlAttrib & ioDirMask) != 0;
}

inline Boolean boxmaker::itsVisible() const
{
	return (theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdFlags & fInvisible) == 0;
}

inline char boxmaker::getAccessRights() const
{
	//
	// for explanation on this hard-coded constant '31', see
	// the comments in boxmaker.cp, function _GatherInfo
	//
	return ((char *)(&theCInfoPBRec))[ 31];
}

inline void boxmaker::copyStr63( const Str63 fromString, Str63 toString)
{
	long *from = (long *)fromString;
	long *to   = (long *)toString;
	
	for( int i = 0; i < 16; i++)
	{
		*to++ = *from++;
	}
}

inline OSErr boxmaker::GatherInfo( const FSSpec &aFSSpec)
{
	copyStr63( aFSSpec.name, theFSSpec.name);

//	theCInfoPBRec.hFileInfo.ioNamePtr	= theFSSpec.name;	done for us by boxmaker::boxmaker
//	theCInfoPBRec.hFileInfo.ioVRefNum	= theVRef;			done for us by boxmaker::_HandleDocs
//	theCInfoPBRec.hFileInfo.ioDirID		= theDirID;			done for us by _GatherInfo
	theCInfoPBRec.hFileInfo.ioFDirIndex	= 0;

	return _GatherInfo( aFSSpec.parID);
}

inline OSErr boxmaker::GatherInfo( long theDirID, long index)
{
//	theCInfoPBRec.hFileInfo.ioNamePtr	= theFSSpec.name;	done for us by boxmaker::boxmaker
//	theCInfoPBRec.hFileInfo.ioVRefNum	= theVRef;			done for us by boxmaker::_HandleDocs
//	theCInfoPBRec.hFileInfo.ioDirID		= theDirID;			done for us by _GatherInfo
	theCInfoPBRec.hFileInfo.ioFDirIndex	= index;

	return _GatherInfo( theDirID);
}

inline void boxmaker::errDebug( OSErr errno)
{
	if( errno != noErr)
	{
		Str15 errstring;
		NumToString( errno, *(Str255 *)&errstring);
		DebugStr( errstring);
	}
}

inline void boxmaker::SetUp_theFSSpec_from_theCInfoPBRec()
{
	theFSSpec.vRefNum = theCInfoPBRec.hFileInfo.ioVRefNum;
	theFSSpec.parID   = theCInfoPBRec.hFileInfo.ioDirID;
}

inline void boxmaker::ErrorAlertQuit( short errorNo,
					short stringIndexID, short stringListID)
{
	ErrorAlert( errorNo, stringIndexID, stringListID);
	//
	// SC++ patches ExitToShell, and calls all necessary desctructors
	// (at least, I like to think it does)
	//
	ExitToShell();
}

inline Boolean boxmaker::matchesTypeList() const
{
	return boxmakergetfile::matchesTypeList(
						theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdType);
}

inline Boolean boxmaker::matchesTypeCreatorPairs() const
{
	return boxmakergetfile::matchesTypeCreatorPairs(
			theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdType,
			theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdCreator);
}
//
// These two structures are used to parse the Handle returned from GetMenuBar
// (and that is done to disable or enable all menus)
//
// I'm not 100% sure about this. The structure might have changed under PPC, but
// I think that that would be troublesome for mixed-mode programs
//
#if defined(powerc) || defined (__powerc)
	#pragma options align=mac68k
#endif

typedef struct menulistentry
{
	MenuHandle theMenu;
	short  menuLeftEdge;
};

typedef struct menulist
{
	short lastHandleOffset;	// = 6 * number of menus
	short rightEdge;
	short unused;
	
	struct menulistentry theMenus[ 1];	// actually kAnyNumber
};

#if defined(powerc) || defined (__powerc)
	#pragma options align=reset
#endif
