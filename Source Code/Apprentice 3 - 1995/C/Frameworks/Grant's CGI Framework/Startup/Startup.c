/*****
 *
 *	Startup.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include <GestaltEqu.h>
#ifdef powerc
	#include <FragLoad.h>
#endif
#include <LowMem.h>
#include <Threads.h>

#include "constants.h"
#include "globals.h"

#include "AboutBox.h"
#include "AEFunc.h"
#include "AEHandlers.h"
#include "ErrorUtil.h"
#include "MemoryUtil.h"
#include "MenuFunc.h"
#include "ProcessUtil.h"
#include "SplashScreen.h"
#include "StringUtil.h"
#include "Version.h"

#include "Startup.h"


/***  LOCAL CONSTANTS ***/

#define	TrapMask		0x0800
#define kGestaltMask	1L

/* window info */
#define	kScreenBorder	4	/* pixels */
#define	kIconSize		32


/***  LOCAL PROTOTYPES ***/

	void		startupErrors			( void );
	void		startupGestalt			( void );
	void		startupEvent			( void );
	void		startupAE				( void );
	void		startupMenus 			( void );
	void		startupThreads			( void );
	void		startupWindows			( void );
	void		startupQuickDraw		( void );
	
	Boolean		startupTrapAvailable	( unsigned long );
	TrapType	startupTrapType			( unsigned long );


/***  FUNCTIONS  ***/

void
StartupApplication ( void )
{
	Handle		sacrificeHandle;

	/* Allocate a 1K memory block, force it to the top of the heap, and lock it.
	   This block is not used for anything - it is put at the top of the heap
	   as a sacrifice because sometimes MacSLIP's VBL task trashes the end of 
	   the heap.
	   from "NewsWatcher" 'newswatcher.c' */
	sacrificeHandle = MyNewHandle ( 1024, nil );
	if ( sacrificeHandle != nil )
	{
		HLockHi ( sacrificeHandle );
	}
	
	gSleepTicks		= kSleepTicks;
	gFrontProcess	= ProcessCurrentIsFront ();
	
	VersionGetShort		( 1, gVersionStr );	/* must be before splash screen */
	
	startupErrors		();
	
	SplashScreenCreate	();	/* put up splash screen */

	startupGestalt		();	/* must be before all Manager inits except toolbox and memory */
	startupEvent		();
	startupAE			();
	startupMenus		();
	startupThreads		();
	startupWindows		();	/* must be after startupMenus */
	startupQuickDraw	();
	AboutBoxInit		();
	
	SplashScreenDispose ();	/* remove splash screen */
} /* StartupApplication */


/***  Initialization Functions  ***/
#pragma mark -

/* Initialize default string for system errors */
void
startupErrors ( void )
{
	StringHandle	tempStr;
	
	/* set system error default string */
	
	gSystemErrorStr	= (StringHandle) MyNewHandle ( sizeof(Str255), nil );
	
	if ( gSystemErrorStr == nil )
	{
		return;
	}
	
	HLockHi ( (Handle)gSystemErrorStr );
	
	tempStr = GetString ( krErrSystemDefault );
	
	if ( tempStr == nil )
	{
		StringPascalCopy ( (char *)ksErrSystemDefault, (char *)(*gSystemErrorStr) );
	}
	else
	{
		HLockHi ( (Handle)tempStr );
		
		StringPascalCopy ( (char *)(*tempStr), (char *)(*gSystemErrorStr) );
		
		HUnlock			( (Handle)tempStr );
		ReleaseResource	( (Handle)tempStr );
	}
	
	HUnlock ( (Handle)gSystemErrorStr );
} /* startupErrors */


/* Determine if the Gestalt manager is available */
void
startupGestalt ( void )
{
	Boolean		gestaltAvail;
	
	/* determine whether the Gestalt call is available */
	gestaltAvail = startupTrapAvailable ( _Gestalt );
	
	if ( !gestaltAvail )
	{
		/* Gestalt require for application to work,
			inform the user and exit gracefully */
		ErrorStartup	( kerrStartupGestalt );
		ExitToShell		();
	}
} /* startupGestalt */


/* Determine if WaitNextEvent trap is available. */
void
startupEvent ( void )
{
	OSErr		theErr;
	SysEnvRec	theSysEnv;	/* system environment */
	Boolean		WNEAvail;
	
	/* what are we running on here (System 4.1 or greater) */
	theErr = SysEnvirons ( 1, &theSysEnv );
	
	if ( theErr != noErr )
	{
		/* can't figure out what the system environment is,
			inform the user and exit gracefully */
		ErrorStartup	( kerrStartupSysEnv );
		ExitToShell		();
	}
	else
	{
		/* is WaitNextEvent implemented? - from Macintosh Tech Note #158 */
		if ( theSysEnv.machineType > envMachUnknown )
		{
			WNEAvail = NGetTrapAddress(0x60, ToolTrap) != NGetTrapAddress(0x9F, ToolTrap);
		}
		else
		{
			WNEAvail = false;
		}
	}
	
	if ( !WNEAvail )
	{
		/* WaitNextEvent trap is missing - can't run */
		ErrorStartup	( kerrStartupWNE );
		ExitToShell		();
	}
} /* startupEvent */


/* Determine if Apple Event Manager is present and install handlers */
void
startupAE ( void )
{
	long	feature;
	OSErr	theErr;
	
	theErr = Gestalt ( gestaltAppleEventsAttr, &feature );

	if ( (theErr != noErr) ||
		!(feature & (kGestaltMask << gestaltAppleEventsPresent)) )
	{
		/* Apple Events not available, inform the user and exit gracefully */
		ErrorStartup	( kerrStartupAppleEvent );
		ExitToShell		();
	}

	gAEIdleUPP = NewAEIdleProc ( MyAEIdleFunc );
	
	theErr = myInstallAEHandlers ();
	
	if ( theErr != noErr )
	{
		ErrorSystem ( theErr );
	}
} /* startupAE */


/* setup menus */
void
startupMenus ( void )
{
	Handle			theMenuBar;
	
	/* store the starting menu bar height for use by window positioning routines */
	gOldMBarHeight = LMGetMBarHeight ();
	
	theMenuBar = GetNewMBar ( kmMenuBarID );
	
	if ( theMenuBar == nil )
	{
		/* can't load menu bar */
		ErrorStartup	( kerrStartupMenu );
		ExitToShell		();
	}
	
	SetMenuBar ( theMenuBar );

	gmAppleMenu	= GetMHandle ( kmAppleMenuID );
	/* add "Apple Menu Items" to the Apple menu */
	AddResMenu	( gmAppleMenu, 'DRVR' );

	gmFileMenu	= GetMHandle ( kmFileMenuID );
	gmEditMenu	= GetMHandle ( kmEditMenuID );
	
	DrawMenuBar ();
} /* startupMenus */


/* determine if thread manager is available */
void
startupThreads ( void )
{
	OSErr	theErr;
	long	feature;
	
	gHasThreadMgr = false;
	
	theErr = Gestalt ( gestaltThreadMgrAttr, &feature );
	
	if ( theErr == noErr )
	{
		gHasThreadMgr = 
			#ifdef powerc
			( feature & (1 << gestaltThreadsLibraryPresent) ) &&
			( (Ptr)NewThread != kUnresolvedSymbolAddress ) &&
			#endif /* def powerc */
			/* the feature check applies to both 68K & PPC */
			( feature & (1 << gestaltThreadMgrPresent) );
	}
} /* startupThreads */


/* Determine the space available on the main and other monitors */
void
startupWindows ( void )
{
	short 		resW;
	short		resH;
	RgnHandle	grayRgnHdl;
	Rect		grayRgnBounds;
	
	/* get screen dimensions */
	resW = qd.screenBits.bounds.right  - qd.screenBits.bounds.left;
	resH = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top;
	
	/* gScreenRect is the size of the main screen, inset by a margin */
	SetRect ( &gScreenRect, kScreenBorder, gOldMBarHeight + kScreenBorder, 
		resW - kScreenBorder, resH - kScreenBorder );
		
	/* get entire gray rgn rect (the size of all monitors) */
	grayRgnHdl		= GetGrayRgn ();			/* known as the 'gray region' */
	grayRgnBounds	= (*grayRgnHdl)->rgnBBox;	/* the bounding box for the region */
	
	resW = grayRgnBounds.right  - grayRgnBounds.left;
	resH = grayRgnBounds.bottom - grayRgnBounds.top;
	
	/* this rect encompasses all of the monitor desktop space,
		minus the menu bar and inset by a margin. We'll use this
		rectangle to limit dragging and resizing windows */
	SetRect ( &gGrayRgnRect,
		/* left */		kScreenBorder,
		/* top */		gOldMBarHeight + kScreenBorder, 
		/* right */		resW - kScreenBorder,
		/* bottom */	resH - kScreenBorder );
} /* startupWindows */


/* QuickDraw - Graphics */
void
startupQuickDraw ( void )
{
	OSErr	theErr;
	long	feature;
	long	version;
	
	gHasColorQD = false;
	
	theErr = Gestalt ( gestaltQuickdrawFeatures, &feature );
	
	if ( theErr == noErr )
	{
		theErr = Gestalt ( gestaltQuickdrawVersion, &version );
	}
	
	if ( (theErr == noErr) &&
		((feature & (kGestaltMask << gestaltHasColor)) != nil) &&
		(version >= gestalt8BitQD) )
	{
		gHasColorQD = true;
	}
} /* startupQuickDraw */


#pragma mark -

/* Determine if a trap is available */
Boolean
startupTrapAvailable ( unsigned long trap )
{
	UniversalProcPtr	theInitGrafAddr;
	UniversalProcPtr	theAA6EAddr;
	Boolean				trapAddrMatch;
	unsigned long		numToolBoxTraps;
	TrapType			theTrapType;
	unsigned long		theTrapMasked;
	
	theInitGrafAddr	= NGetTrapAddress ( _InitGraf, ToolTrap );
	theAA6EAddr		= NGetTrapAddress ( 0xAA6E, ToolTrap );
	trapAddrMatch	= theInitGrafAddr == theAA6EAddr;
	
	if ( trapAddrMatch )
	{
		numToolBoxTraps = 0x200;
	}
	else
	{
		numToolBoxTraps = 0x400;
	}
	
	theTrapType = startupTrapType ( trap );
	
	if ( theTrapType == ToolTrap )
	{
		theTrapMasked = trap & 0x07FF;	/* mask off bits from trap */
		
		if ( theTrapMasked >= numToolBoxTraps )
		{
			theTrapMasked = _Unimplemented;
		}
	}
	
	return ( NGetTrapAddress(theTrapMasked, theTrapType)
		!= NGetTrapAddress(_Unimplemented, ToolTrap) );
} /* startupTrapAvailable */


/* determine the type of the trap */
TrapType
startupTrapType ( unsigned long theTrap )
{
	/* OS traps start with A0, Tool with A8 or AA. */
	if ( (theTrap & 0x0800) == nil )
	{
		return OSTrap;
	}
	else
	{
		return ToolTrap;
	}
} /* startupTrapType */


/*****  EOF  *****/
