/*
 * main.c
 */

#include "window.h"
#include "apprec.h"
#include "clockrec.h"
#include "x.h"
#include "str.h"
#include "menu.h"
#include "util.h"
#include "TalkConstants.h"


/* This will be used as a breathing hole for the error reporting routine */
static Handle err_safeguard = NULL ;

void InstallAEHandlers ( void ) ;



static void
InitMac ( void ) {

	MaxApplZone ( ) ;
	MoreMasters ( ) ;

	InitGraf ( & qd . thePort ) ;
	InitFonts ( ) ;
	InitWindows ( ) ;
	InitMenus ( ) ;
	TEInit ( ) ;
	InitDialogs ( NULL ) ;

	InitCursor ( ) ;
}


static void
InitApp ( DefWindowRec * theApp ) {

FSSpec prefFile ;
short vRefNum ;
long parID ;
Str63 name ;

	gMouse = NewRgn ( ) ;
	FailNil ( gMouse ) ;
	CopyRgn ( GetGrayRgn ( ) , gMouse ) ;
	gDragLimit = ( * GetGrayRgn ( ) ) -> rgnBBox ;
	InsetRect ( & gDragLimit , 3 , 3 ) ;
	gSizeLimit = gDragLimit ;
	OffsetRect ( & gSizeLimit , - gSizeLimit . left , - gSizeLimit . top ) ;
	gSizeLimit . top = 100 ;
	gSizeLimit . left = 150 ;
	gSizeLimit . bottom -= GetMBarHeight ( ) ;
	if ( gSizeLimit . right > 512 ) {
		gSizeLimit . right -= 32 ;
	}

	MakeMenus ( ) ;
	StrGet ( 128 , name , 63 ) ;
	FailErr ( FindFolder ( kOnSystemDisk , kPreferencesFolderType , kCreateFolder ,
		& vRefNum , & parID ) ) ;
	FSMakeFSSpec ( vRefNum , parID , name , & prefFile ) ;
	app = * theApp ;
	( * app . OnCreate ) ( NULL , & app . data , & prefFile ) ;
}


static void
QuitApp ( void ) {

	( * app . OnDestroy ) ( NULL , app . data ) ;
}


static void
ReportError ( short errCode , Boolean toQuit ) {

StringHandle h = GetString ( errCode ) ;
Str15 code ;
StringHandle num ;
Str31 fileStr ;
Str15 lineStr ;

	DisposeHandle ( err_safeguard ) ;
	if ( ! h ) {
		h = GetString ( 129 ) ; /* What do we do if this fails !? */
	}
	HLock ( ( Handle ) h ) ;
	NumToString ( errCode , code ) ;
	num = GetString ( 130 ) ;
	if ( num ) {
		PtrAndHand ( ( Ptr ) & code [ 1 ] , ( Handle ) num , code [ 0 ] ) ;
		* * num += code [ 0 ] ;
		HLock ( ( Handle ) num ) ;
	}

#ifdef FAILINFO
	NumToString ( __err_line , lineStr ) ;
	CopyC2P ( __err_file , fileStr ) ;
#else
	lineStr [ 0 ] = fileStr [ 0 ] = 0 ;
#endif

	ParamText ( h ? * h : NULL , num ? * num : NULL , fileStr , lineStr ) ;
	if ( num ) {
		ReleaseResource ( ( Handle ) num ) ;
	}
	if ( h ) {
		ReleaseResource ( ( Handle ) h ) ;
	}
	if ( ! InFront ( ) ) {
		Notify ( ) ;
	}
	Alert ( 128 , NULL ) ;
	HiliteMenu ( 0L ) ; /* For failures during menu selections */
	ParamText ( NULL , NULL , NULL , NULL ) ;
	err_safeguard = NewHandle ( 1024 ) ;
}


static void
CheckVersions ( void ) {

#if 1
/* These are the typical System-7 checks (we assume System 6 which means
 * WaitNextEvent is always implemented etc.)
 */
Boolean fine = 1 ;

	fine = fine && CheckGestaltBit ( gestaltFindFolderAttr , gestaltFindFolderPresent ) ;
	fine = fine && CheckGestaltBit ( gestaltAppleEventsAttr , gestaltAppleEventsPresent ) ;
	fine = fine && CheckGestaltBit ( gestaltAliasMgrAttr , gestaltAliasMgrPresent ) ;
	fine = fine && CheckGestaltBit ( gestaltOSAttr , gestaltLaunchControl ) ;
	fine = fine && CheckGestaltBit ( gestaltOSAttr , gestaltRealTempMemory ) ;
	fine = fine && CheckGestaltBit ( gestaltOSAttr , gestaltTempMemTracked ) ;
	fine = fine && SpeechAvailable ( ) ;
	if ( ! fine ) {
		Alert ( 131 , NULL ) ;
		ExitToShell ( ) ;
	}
#endif
}


void
main ( ) {

WindowPtr wp ;

	InitMac ( ) ;
	CheckVersions ( ) ;
	err_safeguard = NewHandle ( 1024 ) ;
	InstallAEHandlers ( ) ;
	TRY {
		InitApp ( & appRec ) ;
	runAgain :
		while ( ! quit ) {
			TRY {
				DriveWindow ( ) ;
				if ( InFront ( ) ) {
					DeNotify ( ) ;
				}
			} CATCH {
				ReportError ( __err , 0 ) ;
				NO_PROPAGATE ;
			} DONE ;
		}
		AppSaveWindows ( app . data ) ;
		while ( wp = FrontWindow ( ) ) {
		short err ;
			if ( err = DestroyWindow ( wp ) ) {
				quit = 0 ;
				ReportError ( err , 0 ) ;
				goto runAgain ;
			}
		}
		QuitApp ( ) ;
	} CATCH {
		ReportError ( __err , 1 ) ;
		NO_PROPAGATE ;
	} DONE ;
}


static pascal OSErr
AEOpenApplication ( AppleEvent * event , AppleEvent * reply , long refCon ) {

ProcessSerialNumber psn ;
extern Boolean hasController ;

	if ( FrontWindow ( ) ) {
		if ( FindPSN ( 'FNDR' , 'MACS' , & psn ) ) {
			FailErr ( SetFrontProcess ( & psn ) ) ;
		}
	} else if ( ! hasController ) {
	WindowPtr wp = MakeWindow ( NULL , & clockRec ) ;
		SelectWindow ( wp ) ;
		ShowWindow ( wp ) ;
	}
	return noErr ;
}


static pascal OSErr
AEQuitApplication ( AppleEvent * event , AppleEvent * reply , long refCon ) {

	quit = 1 ;
	return noErr ;
}


static void
InstallAEHandlers ( void ) {

	FailErr ( AEInstallEventHandler ( kCoreEventClass , kAEOpenApplication ,
		AEOpenApplication , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kCoreEventClass , kAEQuitApplication ,
		AEQuitApplication , 0L , 0 ) ) ;
}
