/**********************************************************
 * typedefs
 **********************************************************/

typedef pascal short (*SEProcPtr) (EventRecord *);
typedef pascal void (*ICProcPtr) (void);
typedef pascal void (*DMBProcPtr) (void);
typedef pascal void (*ERProcPtr) (Rect *);
typedef pascal void (*EOProcPtr) (Rect *);
typedef pascal void (*EGProcPtr) (RgnHandle);
typedef pascal void (*STProcPtr) (void);

typedef struct {
        char    privates[76];
        long    randSeed;
        BitMap  screenBits;
        Cursor  arrow;
        Pattern dkGray;
        Pattern ltGray;
        Pattern black;
        Pattern white;
        GrafPtr thePort;
        long    qdend;
} QDGlobals;

typedef struct PatchGlobals {
	short			pgVersion;			// our version number
	SEProcPtr		pgOldSE;			// address of old SystemEvent
	ICProcPtr		pgOldIC;			// address of old InitCursor
	DMBProcPtr		pgOldDMB;			// address of old DrawMenuBar
	ERProcPtr		pgOldER;			// address of old EraseRect
	EOProcPtr		pgOldEO;			// address of old EraseOval
	EGProcPtr		pgOldEG;			// address of old EraseRgn
	STProcPtr		pgOldST;			// address of old SystemTask
	Boolean			pgSaverOn,			// Are we asleep?
					pgInSleepRect,		// Is the mouse in the sleep rect?
					pgMustSleep,		// Do we need to fall asleep now?
					pgMustWake,			// Do we need to wake up now?
					pgBouncingClock,	// Should there be a bouncing clock?
					pgMenubarKluge,		// Should we play with the menu bar?
					pgMustSave;			// Are we turned on?
	short			pgSleepRect,		// index number of the sleep rect
					pgWakeRect,			// index number of the wake rect
					pgOldHeight,		// old menu bar height
					pgIntervalTime;		// time between ST maintenance runs
	long			pgRefreshTime,		// ticks between refreshes
					pgLastAction,		// time of last user action
					pgLastCheck,		// time of last maintenance check
					pgIdleTicks,		// idle ticks before sleep
					pgLastRefresh;		// time of last refresh
	Point			pgLastMouse;		// point where mouse was last seen
	Rect			pgCorners[4];		// corners of the main screen (rects)
	Pattern			pgForePat,			// foreground pattern
					pgBackPat;			// background pattern
	PicHandle		pgClockBg;			// clock background picture
} PatchGlobals, *PatchGlobalsPtr;

typedef struct PrefStructure {
	short	version;					// our version number
	char	sleepNowCorner,				// sleep now corner
			sleepNeverCorner,			// sleep never corner
			idleTime;					// idle time (seconds)
	Boolean	bounceClock,				// show the clock?
			saverOn;					// screen saver on?
	char	refreshSeconds;				// refresh time (seconds)
	Boolean	showIcon,					// show the startup icon?
			fadeToWhite,				// fade to white?
			zeroBar,					// zero menubar?
			eraseRectPatch,				// patch flags...
			eraseOvalPatch,
			eraseRgnPatch,
			drawMenuBarPatch,
			initCursorPatch;
} PrefStructure, *PrefStructurePtr, **PrefStructureHandle;