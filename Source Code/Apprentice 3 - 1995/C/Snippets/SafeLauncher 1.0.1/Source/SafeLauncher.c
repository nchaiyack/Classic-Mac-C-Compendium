//
// ProperLauncher.c
//
//	Written by Nick Triantos
//
//	Copyright © 1994 by Nick Triantos.  All rights reserved.
//
//	This program will launch the one named in the 'STR ' resource if and only
//	if we're in 24-bit addressing mode.  Prevents crashes for some old great
//	games.
//
//	Modification history: (most recent first)
//
//		Date:		Chg:	Description:
//		10/11/94	0000	Initial coding.  Make checks for 32- and 24-bit addr,
//							use rsrc. for editability, Sys 7 dependent
//		10/11/94	0001	Add check for Quadra's cache.  Some apps need for it
//							to be off.

#define kDialogID				128
#define kDialogItemNo			2
#define kDialogFont				1
#define kDialogSize				10
#define kFilename				128
// Error strings
#define kFail24BitAddrID		129
#define kFail32BitAddrID		130
#define kFailQuadCacheOffID		131
#define kFailBadDepthID			132
#define kFailFileNotFoundID		133
#define kLPrpID					0
// Bits read from the 'LPrp' (Launch Properties) resource
#define bitCheck32BitAddr		0x80
#define bitCheck24BitAddr		0x40
#define bitDisableQuadCache		0x20
#define bitSetDepth				0x10


typedef struct LaunchProperties {
	unsigned char		chkBits ;
	unsigned char		depth ;
} LaunchProperties, *LaunchPropPtr, **LaunchPropHdl ;


#include <GestaltEqu.h>			// To check for sys info for flags
#include <Processes.h>			// To launch the app
#include <Palettes.h>			// For depth-setting routines
#ifndef __TRAPS__
#include <Traps.h>
#endif
#ifndef __MWERKS__
pascal OSErr SetDialogDefaultItem (DialogPtr theDialog,
		short newItem) = {0x303C,0x0304,0xAA68};
#endif



// local functions
void InitToolbox( void ) ;
pascal void myItem( WindowPtr theWindow, short theItem ) ;
void DisplayFailAlert( short id ) ;
void LaunchApp( void ) ;
Boolean HWPrivAvailable( void ) ;
// external functions
Boolean TrapAvailable(short theTrap) ;


Rect		gItemRect ;
short		gStrID ;


void InitToolbox( void )
{
	InitGraf( &qd.thePort ) ;
	InitFonts() ;
	InitWindows() ;
	InitMenus() ;
	TEInit() ;
	InitDialogs( nil ) ;
	InitCursor() ;
}

main()
{
	OSErr			theErr ;
	long			result ;
	LaunchPropHdl	lpHdl ;
	LaunchPropPtr	lpPtr ;
	
	MaxApplZone() ;
	InitToolbox() ;
	lpHdl = (LaunchPropHdl)GetResource( 'LPrp', kLPrpID ) ;
	if (lpHdl == nil)
		return 0 ;
	HLock( (Handle)lpHdl ) ;
	lpPtr = *lpHdl ;
	if ( lpPtr->chkBits & (bitCheck32BitAddr|bitCheck24BitAddr) ) {
		theErr = Gestalt( gestaltAddressingModeAttr, &result ) ;
		if (theErr != noErr)
			return theErr ;
		// Check if we're in 24-bit addressing mode
		if ((lpPtr->chkBits & bitCheck24BitAddr) &&
			(BitTst(&result, 31-gestalt32BitAddressing)==TRUE) ) {
			DisplayFailAlert( kFail24BitAddrID ) ;
			goto exit ;
		}
		// Check if we're in 32-bit addressing mode
		if ((lpPtr->chkBits & bitCheck32BitAddr) &&
			(BitTst(&result, 31-gestalt32BitAddressing)==FALSE) ) {
			DisplayFailAlert( kFail32BitAddrID ) ;
			goto exit ;
		}
	}
	// Check if the Quadra cache is off
	if (lpPtr->chkBits & bitDisableQuadCache) {
		Boolean		oldState ;
		Boolean		tempState = TRUE ;		// True or False, it makes no difference.
		
		if (!HWPrivAvailable())
			goto exit ;
		// Data cache
		oldState = SwapDataCache( tempState ) ;	// Get the state in "oldState"
		if (oldState != tempState)
			(void)SwapDataCache( oldState ) ;	// Restore state if we changed it
		if (oldState == TRUE) {
			DisplayFailAlert( kFailQuadCacheOffID ) ;
			goto exit ;
		}
		// instruction cache
		oldState = SwapInstructionCache( tempState ) ;	// Get the state in "oldState"
		if (oldState != tempState)
			(void)SwapInstructionCache( oldState ) ;	// Restore state if we changed it
		if (oldState == TRUE) {
			DisplayFailAlert( kFailQuadCacheOffID ) ;
			goto exit ;
		}
	}
	// Set bit depth if we need to
	if (lpPtr->chkBits & bitSetDepth) {
		short whichFlags=0, flags=0 ;
		short depthID = HasDepth( GetGDevice(), lpPtr->depth, 0, 0 ) ;
		
		if (depthID == 0) {
			DisplayFailAlert( kFailBadDepthID ) ;
			goto exit ;
		}
		else {
			if (lpPtr->depth == 1)
				whichFlags = 1 ;	// See "SetDepth" in THINK Ref...
			SetDepth( GetGDevice(), lpPtr->depth, whichFlags, flags ) ;
		}
	}
	LaunchApp() ;
exit:
	HUnlock( (Handle)lpHdl ) ;
	ReleaseResource( (Handle)lpHdl ) ;
	return 0 ;
}


pascal void myItem( WindowPtr theWindow, short theItem )
{
	StringHandle strHdl ;
	StringPtr theStr ;
	
	strHdl = GetString( gStrID ) ;
	if (strHdl == NULL)
		return ;
	HLock( (Handle)strHdl ) ;
	theStr = *strHdl ;
	TextFont( kDialogFont ) ;
	TextSize( kDialogSize ) ;
	#ifdef __MWERKS__
		TETextBox( &theStr[1], theStr[0], &gItemRect, teFlushDefault ) ;
	#else
		TextBox( &theStr[1], theStr[0], &gItemRect, teFlushDefault ) ;
	#endif
	HUnlock( (Handle)strHdl ) ;
	ReleaseResource( (Handle)strHdl ) ;
}

void DisplayFailAlert( short id )
{
	DialogPtr theDlg ;
	short result ;
	short iType ;
	Rect iRect ;
	Handle iHandle ;
	
	gStrID = id ;
	theDlg = GetNewDialog( kDialogID, NULL, (WindowPtr)-1 ) ;
	GetDItem( theDlg, kDialogItemNo, &iType, &iHandle, &iRect ) ;
	gItemRect = iRect ;
	SetDItem( theDlg, kDialogItemNo, iType, (Handle)myItem, &iRect ) ;
	SetDialogDefaultItem( theDlg, ok ) ;
	ShowWindow( (WindowPtr)theDlg ) ;
	ModalDialog( NULL, &result ) ;
	DisposeDialog( theDlg ) ;
}


void LaunchApp( void ) 
{
	LaunchParamBlockRec		myLaunchParams ;
	ProcessSerialNumber		launchedProcessSN ;
	OSErr					launchErr ;
	long					prefSize ;
	long					minSize ;
	long					availSize ;
	FSSpec					sfFile ;
	short					vRefNum ;
	long					dirID ;
	StringHandle			theStr ;
	StringPtr				filename ;
	
	theStr = GetString( kFilename ) ;
	if (theStr == nil)
		return ;
	HLock( (Handle)theStr ) ;
	filename = *theStr ;
	HGetVol( nil, &vRefNum, &dirID ) ;
	launchErr = FSMakeFSSpec( 0, dirID, filename, &sfFile ) ;
	HUnlock( (Handle)theStr ) ;
	ReleaseResource( (Handle)theStr ) ;
	myLaunchParams.launchBlockID = extendedBlock ;
	myLaunchParams.launchEPBLength = extendedBlockLen ;
	myLaunchParams.launchFileFlags = 0 ;
	myLaunchParams.launchControlFlags = launchContinue + launchNoFileFlags ;
	myLaunchParams.launchAppSpec = &sfFile ;
	myLaunchParams.launchAppParameters = nil ;

	launchErr = LaunchApplication( &myLaunchParams ) ;

	if (launchErr == fnfErr)
		DisplayFailAlert( kFailFileNotFoundID ) ;
	// These aren't used, but what the hey...they're neat to have.
	prefSize = myLaunchParams.launchPreferredSize ;
	minSize = myLaunchParams.launchMinimumSize ;
	if ( !launchErr )
		launchedProcessSN = myLaunchParams.launchProcessSN ;
	else if ( launchErr == memFullErr )
		availSize = myLaunchParams.launchAvailableSize ;
}


// This function is needed to call SwapDataCache/SwapInstructionCache
Boolean HWPrivAvailable(void)
{
	return TrapAvailable(_HWPriv);
}


