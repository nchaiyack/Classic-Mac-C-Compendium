#ifndef __CETOOLBOX__
#define __CETOOLBOX__

extern short		gCERefNum;

#define DoGetTopPtr			9
#define TurnOnOff			10
#define	DoRegisterPtr		11
#define DoUnregisterPtr		12
#define DoFindPtr			13
#define DoOpenDA			14
#define DoCheckKey			15
#define DoDrawKey			16
#define DoGetVersion		17
#define DoMenuChoice		18
#define DoPostError			19
#define DoTheLaunch			20
#define DoFindApp			21
#define DoGetDocument		22
#define DoShutdown			23
#define DoSetFileList		24
#define DoBusyStatus		25
#define DoGetAppList		26
#define DoConfigure			27
#define DoCancelMenuChoice	28
#define DoSwitch			29
#define DoSwitchBack		30

enum {	HBFCallPending,			// 0  - This bit is set when a call is pending.  A
								//		background routine can set this bit and the
								//		routine/DA will be called at the first available
								//		moment.
		HBFWeWereSelected,		// 1  - If bit 12 is set, this bit is set when the hot key
								//		has been pressed, so that your application running
								//		in the background under MultiFinder can respond.
		HBFCallCETBApp,			// 2  - If Bit 9 is set, CEToolbox will try to switch to
								//		its application and then call HBProcPtr if MultiFinder
								//		is running and there is enough memory.
		HBFReserved3,			// 3  - Reserved for future expansion.
		HBFReserved4,			// 4  - Reserved for future expansion.
		HBFReserved5,			// 5  - Reserved for future expansion.
		HBFCallBeforeShutdown,	// 6  - The routine will be called BEFORE shutdown, when a
								//		DoShutDown driver call is made. HBItem has 0 for
								//		shutdown & -1 for restart.
		HBFHasCustomMDEF,		// 7  - Set this bit if the named routine uses a custom
								//		MDEF for its hierarchical menu.  Coupled with bit 11.
		HBFUsesClipboard,		// 8  - Set this bit if this routine needs to have the
								//		clipboard coerced on entry and exit. (Not yet
								//		implemented in version 1.6.)
		HBFCanBackground,		// 9  - Set this bit if the routine might be kept open in
								//		the background when under MultiFinder. Non-modal
								//		routines setting bit 2 may set this bit.
		HBFCallOnShutdown,		// 10 - Call this routine on Shutdown (HBItem will be -1
								//		for shutdown, -2 for Restart when the routine is
								//		called.)
		HBFHasHierarchical,		// 11 - This named routine has a pop-out hierarchical menu
								//		whose Handle is in HBMenu.  Bit 15 should also be
								//		set.
		HBFJustSetBit,			// 12 - Instead of calling a routine, set bit 1 (so an app
								//		can test it later.)
		HBFOpenDA,				// 13 - Instead of calling a routine, open the DA in
								//		HBName. This is usually used in conjunction with
								//		a hot key.
		HBFHasHotKey,			// 14 - This routine has a hot key defined in HBKeyStroke.
								//		Call HBProcPtr when it�s pressed.
		HBFAddToAppleMenu };	// 15 - Put HBName in the Apple menu.  Call HBProcPtr
								//		when it�s selected.

enum {	HBMCallPending			= 0x0001,
		HBMWeWereSelected		= 0x0002,
		HBMCallCETBApp			= 0x0004,
		HBMReserved3			= 0x0008,
		HBMReserved4			= 0x0010,
		HBMReserved5			= 0x0020,
		HBMCallBeforeShutdown	= 0x0040,
		HBMHasCustomMDEF		= 0x0080,
		HBMUsesClipboard		= 0x0100,
		HBMCanBackground		= 0x0200,
		HBMCallOnShutdown		= 0x0400,
		HBMHasHierarchical		= 0x0800,
		HBMJustSetBit			= 0x1000,
		HBMOpenDA				= 0x2000,
		HBMHasHotKey			= 0x4000,
		HBMAddToAppleMenu		= 0x8000};

typedef struct {
	struct HBQRec*	HBQueue;		// This is the pointer to the next element in the
									// queue. Should be nil when you insert into the queue.

	OSType			HBSig;			// This should be the creator of your application/INIT.
									// CEToolbox uses this creator to separate each
									// program�s data resource(s) from all others.

	short			HBSelect;		// This number is used to let the same creator register
									// multiple data structures.  Number them from 1 up. 0
									// is reserved.

	short			HBFlags;		// This is a packed array of boolean used to
									// enable/disable various features.

	long			HBKeyStroke;	// This is the keystroke to invoke the routine/DA.
									// The highword contains the key (both keynumber and
									// ascii, just as given in the EventRecord) and the
									// loword contains the event modifiers.

	Ptr				HBProcPtr;		// This points to the routine to be executed. The
									// routine will be called from SystemTask, so it may
									// allocate memory, put up dialog boxes, etc.  On
									// entry, A0 will point to the HBQRec.  All registers
									// other than A0/A1/D0/D1 must be preserved.

	char			HBName[26];		// This Pascal string is either the name to display
									// in the Apple menu , or is the name of a desk
									// accessory to open if bit 13 of HBFlags is set.
									// For DA names, initial nulls are NOT necessary.
									// Also, CEToolbox uses a �partial� match � 'DiskT'
									// would find DiskTop�, DiskTop 3.0, and DiskTools.

	MenuHandle		HBMenu;			// This is the menu to be added, if bit 11 of HBFlags
									// is set.

	short			HBItem;			// This will return the item number selected from the
									// menu if an hierarchical menu was supplied.
} HBQRec, *HBQPtr;


typedef struct {
	Str31		fName;
	long		fDir;
	short		fVRef;
	OSType		fType;
} LaunchOne, *LaunchOnePtr;


typedef struct {
	short		numDocs;
	LaunchOne	f[1];
} LaunchRec, *LaunchPtr, **LaunchHdl;


typedef struct {
	OSType		fCreator;				// -->
	StringPtr	fApplicationName;		// <-- (you know...)
	long		fDirID;					// <--
	short		fVRefNum;				// <--
} FindAppRec, *FindAppPtr, **FindAppHdl;

OSErr				CEOpenToolbox(void);
OSErr				CERegister(HBQRec *record);
OSErr				CEPostError(short errNum);
OSErr				CEFindApp(OSType creator, StringPtr name, short *vRefNum, 
						long *dirID);
OSErr				CETheLaunch(LaunchHdl launchHandle);
OSErr				CETurnOnOff(short newValue, short *oldValue);
OSErr				CECheckKey(long key, HBQPtr *otherGuy);
OSErr				CEDrawKey(long key, Rect *bounds);

#endif