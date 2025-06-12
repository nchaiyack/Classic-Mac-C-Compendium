/*
	Dragon.h
	
	Interface to Dragon.c
*/

#pragma once

#include	"AppleEventQueue.h"
#include	"Preferences.h"
#include	"FileUtils.h"

#define	prefDragonPrefs	0				// This class's preferences number (see Preferences.c)

// Structure of the 'DrPr' 128 resource
typedef struct {
	short	miscFlags;
	short	reserved;
	long		sleep[4];
	short	depthLim;
} DragonPrefsRec;

// Masks for use with miscFlags
enum {
	maskFilesOnly = 1,
	maskResolveAliases = 2,
	maskFollowAliasChain = 4,
	maskAutoQuit = 8
};

// Possible values for runState Ñ are we in the foreground or background, and are we idle or are we busy (i.e., processing docs)?
enum {
	kFGIdle,		// binary É0000
	kBGIdle,		// binary É0001
	kFGBusy,	// binary É0010
	kBGBusy		// binary É0011
};

// Masks for use with runState
//	If we're in the background, runState & maskInBG != FALSE
//	If we're busy, runState & maskBusy != FALSE
enum {
	maskInBG = 1,	// binary É0001
	maskBusy = 2		// binary É0010
};

class Dragon: indirect {

	protected:
		FSSpec			*curDocFSS;		// Pointer to an FSSpec to the current file or folder (or volume)
		PBRecUnion		*curDocPB;		// Pointer to a multi-purpose param block containing info about the current doc
		short			dirDepthLimit;		// Recursively open folders and volumes to this level (0 == don't open
										//	them, -1 == go down one level, etc.)
		short			curDirDepth;		// Where are we now? Ñ 0 == at top level, -1 == down one, etc.
		Boolean			filesOnly;			// Do we ignore folders and volumes that appear at the lowest level?
										//	 (i.e., when curDirDepth == dirDepthLimit)
		Boolean			resolveAliases;	// Resolve any aliases we end up with after opening folders and disks?
		Boolean			followAliasChain;	// Resolve them all the way, or just one step of the way?
		Boolean			useCustomFilter;	// Should we use the CustomFilterOne method to check each FSSpec?
		TypeListHndl		acceptableTypes;	// List (derived from our 'FREF' resources) of all the types of things we
										//	can digest

		OSType			prefsFileType;		// Our prefs file's type
		Preferences		*preferences;		// Preferences-managing object
		DragonPrefsRec	**dragonPrefs;	// Handle to the resource containing preferences used by Dragon
										//	(provided so that subclasses can let the user change them)
		short			appResFork;		// The application file's resource fork refNum
		FSSpec			appFile;			// An FSSpec that identifies the dragon's application file
		
		OSType			signature;		// The dragon application's signature
		short			runState;			// What state are we in Ñ processing docs?  in the background?
		long				sleepTime;		// Maximum number of ticks to yield to WaitNextEvent
		long				sleepValue[4];		// One sleepTime value for each of the 4 states (see the enum above)
		CursHandle		busyCursor;		// The cursor we show when we're busy
		RgnHandle		cursorRgn;		// For WaitNextEvent
		Boolean			running;			// Are we running?
		Boolean			abortProcessing;	// Should we stop processing docs?
		Boolean			autoQuit;			// Should we automatically quit after the first 'oapp' or 'odoc'?
		Boolean			menusInstalled;	// Do we have working menus?
		
		AppleEventQueue	*aeQueue;		// Queue object used to manage suspended Apple events
		short			numAEsPending;	// Number of Apple events that we've received but not yet processed
		
		MenuHandle		appleMenu;
		MenuHandle		fileMenu;
		MenuHandle		editMenu;
		
	public:
						Dragon (void);		// Constructor
		virtual void		Start (void);
		virtual void		Run (void);
		virtual OSErr		DoOapp (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
		virtual OSErr		DoOdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
		virtual OSErr		DoPdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
		virtual OSErr		DoQuit (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
		virtual Boolean	WaitIdle (EventRecord *theEvent, long *sleep, RgnHandle *mouseRgn);
		virtual void		StopRunning (OSErr err);
		virtual void		Finish (void);

	protected:
		virtual void		BeginProcessing (void);
		virtual void		EndProcessing (void);
		virtual void		StopProcessing (OSErr err);

		virtual Boolean	CanProcessDoc (void);
		virtual Boolean	CustomFilterDoc (void);
		
		virtual OSErr		ProcessDroppings (AEDescList *docList);
		virtual void		ProcessDoc (void);
		virtual void		ProcessFile (void);
		virtual void		ProcessDirectory (void);
		virtual void		ProcessDocsInDirectory (short vRefNum, long dirID);
		virtual void		ProcessOwnedFile (void);
		virtual void		SaveDocInfo (Boolean refreshFinder);
		
		virtual void		DoBusy (void);
		virtual void		ShowProgress (void);
		virtual void		AdjustMenusBusy (void);
		virtual void		AdjustMenusIdle (void);
		virtual void		CursorBusy (void);
		virtual void		CursorIdle (void);
		
		virtual OSErr		SuspendAEvent (AppleEvent *event, AppleEvent *reply, AEHandlerFunc *handler, long refcon);
		virtual void		ResumeAEvent (void);
		virtual void		FlushAEventQueue (void);
		
		virtual void		DoEvent (EventRecord *event);
		virtual void		DoMouseDown (EventRecord *theEvent);
		virtual void		DoMouseUp (EventRecord *theEvent);
		virtual void		DoKeyDown (EventRecord *theEvent);
		virtual void		DoActivate (EventRecord *theEvent);
		virtual void		DoDeactivate (EventRecord *theEvent);
		virtual void		DoUpdateEvent (EventRecord *theEvent);
		virtual void		DoDiskInsert (EventRecord *theEvent);
		virtual void		DoOSEvent (EventRecord *theEvent);
		virtual void		DoHighLevelEvent (EventRecord *theEvent);
		virtual void		DoOtherHLEvent (EventRecord *theEvent);
		virtual void		DoIdle (void);
		virtual void		DoSuspend (void);
		virtual void		DoResume (void);
		
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoAppleMenu (short itemNum);
		virtual void		DoAbout (void);
		virtual void		DoFileMenu (short itemNum);
		virtual void		DoEditMenu (short itemNum);
		
		virtual void		InitMac (void);
		virtual void		InitMilieu (void);
		virtual void		InitMem (void);
		virtual void		InitPrefs (void);
		virtual Preferences *MakePrefsObject (void);
		virtual Boolean	MakePrefsFile (FSSpec *fss);
		virtual Boolean	FindPrefsFile (FSSpec *fss);
		virtual Boolean	IsPrefsFile (FSSpec *fss);
		virtual void		ReadPrefs (void);
		virtual void		SetUpMenus (void);
		virtual void		CallMoreMasters (void);
		virtual void		InitAppleEvents (void);
		virtual AppleEventQueue   *MakeAEQObject (void);
		virtual Boolean	InteractWithUser (long timeOut);
		virtual void		Abort (short errNum);
		virtual void		Error (short errNum);
		
};

// Now we declare some macros for convenience' sake, so we can write
//	curFileType = 'TEXT';
// instead of
//	curDocPB->cinfo.hFileInfo.ioFlFndrInfo.fdType = 'TEXT';

// These macros are valid for files and directories Ñ
	#define	curDocVRefNum		curDocFSS->vRefNum
	#define	curDocParID			curDocFSS->parID
	// This would also work:			curDocPB->c.hFileInfo.ioFlParID [or .dirInfo.ioDrParID, same thing]
	#define	curDocName			curDocFSS->name					// This is a pointer to a Pascal string, remember
	#define	curDocCreated		curDocPB->c.hFileInfo.ioFlCrDat
	#define	curDocModified		curDocPB->c.hFileInfo.ioFlMdDat
// These macros can't be assigned to (of course) Ñ
	#define	curDocIsFile			!(curDocPB->c.hFileInfo.ioFlAttrib & ioDirMask)
	#define	curDocIsDirectory		(curDocPB->c.hFileInfo.ioFlAttrib & ioDirMask)
	#define	curDocIsVolume		(curDocFSS->parID == fsRtParID)
	
// These macros are valid only for files Ñ
	#define	curFileType			curDocPB->c.hFileInfo.ioFlFndrInfo.fdType
	#define	curFileCreator			curDocPB->c.hFileInfo.ioFlFndrInfo.fdCreator
	#define	curFileDataForkSize	curDocPB->c.hFileInfo.ioFlLgLen
	#define	curFileResForkSize		curDocPB->c.hFileInfo.ioFlRLgLen
	#define	curFileFlags			curDocPB->c.hFileInfo.ioFlFndrInfo.fdFlags
	
extern Dragon		*gDragon;

// ----------------		Error codes			----------------

	enum {
		eNormalQuit = noErr,			// Normal quitting (user chose Quit or autoQuit == TRUE) is not an error
		eNoAppleEvents = 1001,
		eCouldntInstallAppleEvents,
		eInsufficientSystem = 1003,		// The current System does not have all the features this program needs
		eInitializationFailed,			// Initialization failed for some miscellaneous reason
		ePreferenceNotFound,			// We couldn't read a needed preference resource
		eForcedQuit					// We were forced to quit (i.e., we received a Quit event while processing)
	};

	#define	eLastDragonError	eForcedQuit
		
// ----------------		Resource constants	----------------

	#define	rErrorAlert		129
	#define	kPrefsFileType	'pref'
	#define	rPrefsFileName	128		// ID of the 'STR ' resource containing the name of our prefs file
	#define	kGstCheckType	'GChk'
	#define	rGstChecklist		128
	
// ----------------		Menu constants		----------------
enum {
	mApple = 128,
	mFile,
	mEdit
};

enum {			// Apple menu
	iAbout = 1,
	iLine1
};


// ----------------		Misc. constants		----------------
#define	kWatchCursor		4

// ----------------		Function prototypes	----------------

	void main (void);
	pascal OSErr HandleOapp (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
	pascal OSErr HandleOdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
	pascal OSErr HandlePdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
	pascal OSErr HandleQuit (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
	pascal OSErr ReturnEventNotHandled (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon);
	pascal Boolean CallWaitIdle (EventRecord *theEvent, long *sleep, RgnHandle *mouseRgn);

	Dragon *CreateGDragon (void);		// This function is NOT defined in Dragon.c Ñ you must define it elsewhere
									//	so that it returns a new dragon of your subclass (not of Dragon!)

// Same data as a regular EventRecord, redone for High-Level events
typedef struct {
	short	what;
	long		eventClass;
	long		when;
	long		eventID;
	short	modifiers;
} HLEventRecord, *HLEventPtr;

