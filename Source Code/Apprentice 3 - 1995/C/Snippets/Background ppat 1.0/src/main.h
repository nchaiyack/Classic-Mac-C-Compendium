/* ----------------------------------------------------------------------
defines
---------------------------------------------------------------------- */
#define	NIL					0L
#define STD_BUTTON			0

#define MENU_BASE_ID		100

#define	MENU_APPLE_ID		100
#define	MENU_FILE_ID		200

#define	APPLE_ABOUT			1

#define	FILE_QUIT			1

#define DLOG_ABOUT			100
#define DLOG_ABOUT_OK		1
#define DLOG_ABOUT_PICT		2

#define SYSTEM_PPAT			16

#define	WNE_TRAP_NUM		0x60
#define	UNIMPL_TRAP_NUM		0x9F
#define	SUSPEND_RESUME_BIT	0x0001
#define	RESUMING			1
#define	MIN_SLEEP			60L

/* ----------------------------------------------------------------------
globals
---------------------------------------------------------------------- */
extern Boolean				gDone,
							gWNEImplemented,
							gInBackground;
extern EventRecord			gTheEvent;
extern MenuHandle			gAppleMenu,
							gFileMenu,
							gEditMenu;
extern WindowPtr			gTheWindow;
extern PixPatHandle			gPixPat;
extern ProcessSerialNumber	gMailApp,
							gNewsApp,
							gFTPApp,
							gWebApp,
							gTelnetApp;


/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

/* main.c */
extern void main();
extern void InitToolBox();
extern void InitMenuBar();
extern void MainLoop();
extern void Do();
extern void DoUpdate();
extern void DoMouseDown();
extern void DoMenu(long int menuChoice);
