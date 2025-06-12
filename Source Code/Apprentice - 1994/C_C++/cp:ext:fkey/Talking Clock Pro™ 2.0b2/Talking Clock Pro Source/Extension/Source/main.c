/*
 * main.c
 */

#include <Types.h>
#include <appleevents.h>
#include <speech.h>
#include <aeregistry.h>
#include <aeobjects.h>
#include <Memory.h>
#ifdef applec
#include <SysEqu.h>
#endif
#include <Resources.h>
#include <ToolUtils.h>
#include <Folders.h>
#include <Errors.h>
#include <Packages.h>
#include <GestaltEqu.h>
#include <Desk.h>

#include "x.h"
#include "util.h"
#include "str.h"
#include "notify.h"
#include "TalkConstants.h"

/* FBA? */
#define appe


/* This will be used as a breathing hole for the error reporting routine */
Handle err_safeguard = NULL ;


void InstallHandlers ( void ) ;

FSSpec prefFile ;
Boolean quit = 0 ;

typedef struct pref {
	short	turnoverPeriod ;
	char		talkOptionKey ;
	char		pad1 ;
	char		talkControlKey ;
	char		pad2 ;
	char		talkCommandKey ;
	char		pad3 ;
	char		talkShiftKey ;
	char		pad4 ;
	char		talkCapsLockKey ;
	char		pad5 ;
	char		talkConstantly ;
	char		pad6 ;
} Pref ;

Pref gPrefStruct ;


static void
InitMac ( void ) {

	if ( ( * ( long * ) 0x908 ) - 6000L < ( long ) GetApplLimit ( ) ) {
		SetApplLimit ( ( void * ) ( ( * ( long * ) 0x908 ) - 6000L ) ) ; /* CurStackBase */
	}
	MaxApplZone ( ) ;
	MoreMasters ( ) ;

#ifndef appe
	InitGraf ( & qd . thePort ) ;
	InitFonts ( ) ;
	InitWindows ( ) ;
	InitMenus ( ) ;
	TEInit ( ) ;
	InitDialogs ( NULL ) ;
#endif
}


static void
CreatePrefsFile ( FSSpec * file ) {

volatile short refNum = 0 , refTemp ;
long len ;

	TRY {
		FailErr ( FSpCreate ( file , 'pref' , 'pref' , smRoman ) ) ;
		FailErr ( FSpOpenDF ( file , fsWrPerm , ( short * ) & refNum ) ) ;
		len = sizeof ( gPrefStruct ) ;
		FailErr ( SetEOF ( refNum , len ) ) ;
		FailErr ( FSWrite ( refNum , & len , ( Ptr ) & gPrefStruct ) ) ;
		refTemp = refNum ;
		refNum = 0 ;
		FailErr ( FSClose ( refTemp ) ) ;
	} CATCH {
		if ( refNum ) {
			FSClose ( refNum ) ;
		}
		FSpDelete ( file ) ;
	} DONE ;
}


static void
ReadPrefsFile ( FSSpec * file ) {

volatile short refNum = 0 , refTemp ;
long len ;

	TRY {
		FailErr ( FSpOpenDF ( file , fsRdPerm , ( short * ) & refNum ) ) ;
		len = sizeof ( gPrefStruct ) ;
		FailErr ( FSRead ( refNum , & len , ( Ptr ) & gPrefStruct ) ) ;
		refTemp = refNum ;
		refNum = 0 ;
		FailErr ( FSClose ( refTemp ) ) ;
	} CATCH {
		if ( refNum ) {
			FSClose ( refNum ) ;
		}
	} DONE ;
}


static void
WritePrefsFile ( FSSpec * file ) {

volatile short refNum = 0 , refTemp ;
long len ;

	TRY {
		FailErr ( FSpOpenDF ( file , fsWrPerm , ( short * ) & refNum ) ) ;
		len = sizeof ( gPrefStruct ) ;
		FailErr ( SetEOF ( refNum , len ) ) ;
		FailErr ( FSWrite ( refNum , & len , ( Ptr ) & gPrefStruct ) ) ;
		refTemp = refNum ;
		refNum = 0 ;
		FailErr ( FSClose ( refTemp ) ) ;
	} CATCH {
		if ( refNum ) {
			FSClose ( refNum ) ;
		}
	} DONE ;
}


static void
InitApp ( ) {

short vRefNum ;
long parID ;
Str63 name ;

Handle h ;

	err_safeguard = NewHandle ( 1024 ) ;
	FailNil ( err_safeguard ) ;

	h = GetResource ( 'Pref' , 128 ) ;
	FailNil ( h ) ;
	BlockMove ( * h , & gPrefStruct , sizeof ( gPrefStruct ) ) ;
	ReleaseResource ( h ) ;

	GetIndString ( name , 128 , 1 ) ;
	FailErr ( FindFolder ( kOnSystemDisk , kPreferencesFolderType , kCreateFolder ,
		& vRefNum , & parID ) ) ;
	if ( fnfErr == FSMakeFSSpec ( vRefNum , parID , name , & prefFile ) ) {
		CreatePrefsFile ( & prefFile ) ;
	}
	ReadPrefsFile ( & prefFile ) ;

	InstallHandlers ( ) ;
}


static void
QuitApp ( void ) {

static Str255 quitStr ;
unsigned long tick = TickCount ( ) + 300L ;

	GetIndString ( quitStr , 128 , 11 ) ;
	while ( SpeechBusy ( ) && TickCount ( ) < tick ) {
		SystemTask ( ) ;
	}
	FailErr ( SysSpeakString ( quitStr ) ) ;
	WritePrefsFile ( & prefFile ) ;
	tick = TickCount ( ) + 300L ;
	while ( SpeechBusy ( ) && TickCount ( ) < tick ) {
		SystemTask ( ) ;
	}
}


static void
ReportError ( short errCode , Boolean toQuit ) {

static Str15 codeStr ;
static Str15 lineStr ;
static Str63 fileStr ;
static Str255 s ;
static Str63 medStr ;

	DisposeHandle ( err_safeguard ) ;
	err_safeguard = NULL ;

#ifdef FAILINFO
	NumToString ( __err_line , lineStr ) ;
	CopyC2P ( __err_file , fileStr ) ;
#else
	lineStr [ 0 ] = 0 ;
	fileStr [ 0 ] = 0 ;
#endif

	NumToString ( errCode , codeStr ) ;

	GetIndString ( s , 128 , 6 ) ;
	ConcatPStrings ( s , codeStr ) ;
	GetIndString ( medStr , 128 , 7 ) ;
	ConcatPStrings ( s , medStr ) ;
	ConcatPStrings ( s , fileStr ) ;
	GetIndString ( medStr , 128 , 8 ) ;
	ConcatPStrings ( s , medStr ) ;
	ConcatPStrings ( s , lineStr ) ;
	GetIndString ( medStr , 128 , 9 ) ;
	ConcatPStrings ( s , medStr ) ;

	NotifyStr ( s ) ;

	err_safeguard = NewHandle ( 1024 ) ;
}


static Boolean
CheckVersions ( void ) {

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
	return fine ;
}


static void
FailVersions ( void ) {

static Str255 s ;

	GetIndString ( s , 128 , 10 ) ;
	NotifyStr ( s ) ;
	ExitToShell ( ) ;
}


typedef struct talkTime {
	short	hour ;
	short	minutePeriod ;
} TalkTime ;


static void
MakeTalkTime ( TalkTime * new ) {

unsigned long now ;
DateTimeRec dtr ;

	GetDateTime ( & now ) ;
	Secs2Date ( now , & dtr ) ;
	new -> hour = dtr . hour ;
	new -> minutePeriod = dtr . minute / 5 ;
	if ( ! dtr . minute ) {
		new -> minutePeriod = 0 ;
	} else if ( new -> minutePeriod > gPrefStruct . turnoverPeriod ) {
		new -> hour += 1 ;
	}
}


static Boolean
TimeTask ( TalkTime * old , TalkTime * new ) {

Boolean ret = 0 ;

	MakeTalkTime ( new ) ;
	if ( gPrefStruct . talkConstantly ) {
		if ( new -> minutePeriod != old -> minutePeriod ) {
			ret = 1 ;
		}
		if ( new -> hour != old -> hour ) {
			ret = 1 ;
		}
	}
	return ret ;
}


static Boolean
KeyCheck ( void ) {
	if ( gPrefStruct . talkOptionKey && KeyIsDown ( 0x3a ) ) {
		return 1 ;
	}
	if ( gPrefStruct . talkControlKey && KeyIsDown ( 0x3b ) ) {
		return 1 ;
	}
	if ( gPrefStruct . talkCommandKey && KeyIsDown ( 0x37 ) ) {
		return 1 ;
	}
	if ( gPrefStruct . talkShiftKey && KeyIsDown ( 0x38 ) ) {
		return 1 ;
	}
	if ( gPrefStruct . talkCapsLockKey && KeyIsDown ( 0x39 ) ) {
		return 1 ;
	}
	return 0 ;
}


static void
SpeakDate ( void ) {

static Str255 date ;
unsigned long now ;

	GetDateTime ( & now ) ;
	IUDateString ( now , longDate , date ) ;
	while ( SpeechBusy ( ) ) {
		SystemTask ( ) ;
	}
	FailErr ( SysSpeakString ( date ) ) ;
}


static void
Talk ( TalkTime * time ) {

static Str255 s ;
static Str63 middle ;

	if ( ! time -> minutePeriod ) {
		GetIndString ( s , 129 , time -> hour + 1 ) ;
		if ( ! SpeechBusy ( ) ) {
			FailErr ( SysSpeakString ( s ) ) ;
			if ( ! time -> hour ) {
				SpeakDate ( ) ;
			}
		}
	} else {
		GetIndString ( s , 128 , 2 ) ;
		GetIndString ( middle , 131 , time -> minutePeriod ) ;
		ConcatPStrings ( s , middle ) ;
		GetIndString ( middle , 130 , ( time -> hour % 24 ) + 1 ) ;
		ConcatPStrings ( s , middle ) ;
		GetIndString ( middle , 128 , 3 ) ;
		ConcatPStrings ( s , middle ) ;
		FailErr ( SysSpeakString ( s ) ) ;
	}
}


static void
Idle ( void ) {

EventRecord er ;
static TalkTime lastTalk = { 47 , 111 } , talk ;
Boolean doTalk = 0 ;

	if ( WaitNextEvent ( -1 , & er , 120L , NULL ) ) {
		if ( er . what == kHighLevelEvent ) {
			FailErr ( AEProcessAppleEvent ( & er ) ) ;
		}
	}
	doTalk = TimeTask ( & lastTalk , & talk ) ;
	doTalk |= KeyCheck ( ) ;
	if ( doTalk ) {
		Talk ( & talk ) ;
		lastTalk = talk ;
	}
}


void
main ( void ) {

	InitMac ( ) ;
	if ( ! CheckVersions ( ) ) {
		FailVersions ( ) ;
	}
	TRY {
		InitApp ( ) ;
	runAgain :
		while ( ! quit ) {
			TRY {
				Idle ( ) ;
			} CATCH {
				ReportError ( __err , 0 ) ;
				NO_PROPAGATE ;
			} DONE ;
		}
		QuitApp ( ) ;
	} CATCH {
		ReportError ( __err , 1 ) ;
		NO_PROPAGATE ;
	} DONE ;
}


static pascal OSErr
OpenApplication ( AppleEvent * event , AppleEvent * reply , long refCon ) {
	return noErr ;
}


static pascal OSErr
QuitApplication ( AppleEvent * event , AppleEvent * reply , long refCon ) {
	quit = 1 ;
	return noErr ;
}


static OSErr
AddReplyBool ( AppleEvent * event , AppleEvent * reply , Boolean val ) {

OSErr err = 0 ;
DescType theType , aType ;
long theSize ;
AEDesc list = { 0 , 0 } ;

	if ( AEGetKeyDesc ( event , keyAERequestedType , typeAEList , & list ) ) {
		theType = typeBest ;
	}
	if ( ! err ) {
		err = errAECantSupplyType ;
		if ( list . dataHandle ) {

		long numItems ;
		long theItem ;
		AEKeyword foom ;

			err = AECountItems ( & list , & numItems ) ;
			for ( theItem = 1 ; theItem <= numItems && ! err ; theItem ++ ) {
				err = AEGetNthPtr ( & list , theItem , typeType , & foom ,
					& aType , ( void * ) & theType , sizeof ( theType ) , & theSize ) ;
				switch ( theType ) {
				case typeChar :
					theItem = numItems ;
					break ;
				case typeBoolean :
					theItem = numItems ; /* break out */
					break ;
				case typeBest :
					theType = typeBoolean ;
					theItem = numItems ; /* break out */
					break ;
				}
			}
		}
	}
	if ( ! err ) {
	static Str63 s ;
		switch ( theType ) {
		case typeBoolean :
			err = AEPutParamPtr ( reply , keyAEResult , typeBoolean , ( void * ) & val ,
				sizeof ( val ) ) ;
			break ;
		case typeChar :
			GetIndString ( s , 127 , val ? 2 : 1 ) ;
			err = AEPutParamPtr ( reply , keyAEResult , typeChar , ( void * ) & s [ 1 ] ,
				s [ 0 ] ) ;
			break ;
		default :
			err = errAECantSupplyType ;
			break ;
		}
	}
	AEDisposeDesc ( & list ) ;
	return err ;
}


static OSErr
AddReplyLong ( AppleEvent * event , AppleEvent * reply , long val ) {

OSErr err = 0 ;
DescType theType , aType ;
long theSize ;
AEDesc list = { 0 , 0 } ;

	if ( AEGetKeyDesc ( event , keyAERequestedType , typeAEList , & list ) ) {
		theType = typeBest ;
	}
	if ( ! err ) {
		err = errAECantSupplyType ;
		if ( list . dataHandle ) {

		long numItems ;
		long theItem ;
		AEKeyword foom ;

			err = AECountItems ( & list , & numItems ) ;
			for ( theItem = 1 ; theItem <= numItems && ! err ; theItem ++ ) {
				err = AEGetNthPtr ( & list , theItem , typeType , & foom ,
					& aType , ( void * ) & theType , sizeof ( theType ) , & theSize ) ;
				switch ( theType ) {
				case typeChar :
				case typeMagnitude :
				case typeLongInteger :
				case typeBest :
					theType = typeInteger ;
					theItem = numItems ; /* break out */
					break ;
				}
			}
		}
	}
	if ( ! err ) {
		switch ( theType ) {
		case typeInteger :
			err = AEPutParamPtr ( reply , keyAEResult , typeInteger , ( void * ) & val ,
				sizeof ( val ) ) ;
			break ;
		default :
			err = errAECantSupplyType ;
			break ;
		}
	}
	AEDisposeDesc ( & list ) ;
	return err ;
}


static pascal OSErr
GetData ( AppleEvent * event , AppleEvent * reply , long refCon ) {

AERecord theSpec = { 0 , 0 } ;
DescType theType , aType ;
long theSize ;
OSErr err ;

	TRY {
		err = AEGetKeyDesc ( event , keyDirectObject , typeAERecord , & theSpec ) ;
		if ( ! err ) {
			err = AEGetKeyPtr ( & theSpec , keyAEDesiredClass , typeType , & aType ,
				( void * ) & theType , sizeof ( theType ) , & theSize ) ;
		}
		if ( ! err ) {
			if ( cProperty != theType ) {
				err = errAENotAnElement ;
			} else {
				err = AEGetKeyPtr ( & theSpec , keyAEKeyData , typeType , & aType ,
					( void * ) & theType , sizeof ( theType ) , & theSize ) ;
			}
		}
		if ( ! err ) {
			switch ( theType ) {
			case pTurnover :
				err = AddReplyLong ( event , reply , gPrefStruct . turnoverPeriod ) ;
				break ;
			case pOptionKeyTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkOptionKey ) ;
				break ;
			case pControlKeyTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkControlKey ) ;
				break ;
			case pCommandKeyTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkCommandKey ) ;
				break ;
			case pShiftKeyTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkShiftKey ) ;
				break ;
			case pCapsLockKeyTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkCapsLockKey ) ;
				break ;
			case pFiveMinuteTalk :
				err = AddReplyBool ( event , reply , gPrefStruct . talkConstantly ) ;
				break ;
			default :
				err = errAENotAnElement ;
			}
		}
		AEDisposeDesc ( & theSpec ) ;
	} CATCH {
		AEDisposeDesc ( & theSpec ) ;
		err = __err ;
		NO_PROPAGATE ;
	} DONE ;
	return err ;
}


static OSErr
GetVal ( AppleEvent * event , DescType type , void * data , long size ) {

DescType theType ;
long theSize ;
OSErr err ;

	err = AEGetParamPtr ( event , keyAEData , type , & theType , ( void * ) data ,
		size , & theSize ) ;
	if ( ! err ) {
		if ( type != theType ) {
			DebugStr ( "\pReal type error" ) ;
			err = errAETypeError ;
		}
	}
	return err ;
}


static pascal OSErr
SetData ( AppleEvent * event , AppleEvent * reply , long refCon ) {

AERecord theSpec = { 0 , 0 } ;
DescType theType , aType ;
long theSize ;
OSErr err ;

	TRY {
		err = AEGetKeyDesc ( event , keyDirectObject , typeAERecord , & theSpec ) ;
		if ( ! err ) {
			err = AEGetKeyPtr ( & theSpec , keyAEDesiredClass , typeType , & aType ,
				( void * ) & theType , sizeof ( theType ) , & theSize ) ;
		}
		if ( ! err ) {
			if ( cProperty != theType ) {
				err = errAENotAnElement ;
			} else {
				err = AEGetKeyPtr ( & theSpec , keyAEKeyData , typeType , & aType ,
					( void * ) & theType , sizeof ( theType ) , & theSize ) ;
			}
		}
		if ( ! err ) {
		Pref savePref = gPrefStruct ;
		long l ;
		Boolean b ;
			switch ( theType ) {
			case pTurnover :
				err = GetVal ( event , typeInteger , & l , sizeof ( l ) ) ;
				if ( ! err ) gPrefStruct . turnoverPeriod = l ;
				break ;
			case pOptionKeyTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkOptionKey = b ;
				break ;
			case pControlKeyTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkControlKey = b ;
				break ;
			case pCommandKeyTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkCommandKey = b ;
				break ;
			case pShiftKeyTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkShiftKey = b ;
				break ;
			case pCapsLockKeyTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkCapsLockKey = b ;
				break ;
			case pFiveMinuteTalk :
				err = GetVal ( event , typeBoolean , & b , sizeof ( b ) ) ;
				if ( ! err ) gPrefStruct . talkConstantly = b ;
				break ;
			default :
				err = errAENotAnElement ;
			}
			if ( err ) {
				gPrefStruct = savePref ;
			}
		}
		AEDisposeDesc ( & theSpec ) ;
	} CATCH {
		AEDisposeDesc ( & theSpec ) ;
		err = __err ;
		NO_PROPAGATE ;
	} DONE ;
	return err ;
}


static pascal OSErr
AESpeakTime ( AppleEvent * event , AppleEvent * reply , long refCon ) {

TalkTime tt ;
short err = 0 ;

	TRY {
		MakeTalkTime ( & tt ) ;
		Talk ( & tt ) ;
	} CATCH {
		err = __err ;
		NO_PROPAGATE ;
	} DONE ;

	return err ;
}


static pascal OSErr
AESpeakDate ( AppleEvent * event , AppleEvent * reply , long refCon ) {

short err = 0 ;

	TRY {
		SpeakDate ( ) ;
	} CATCH {
		err = __err ;
		NO_PROPAGATE ;
	} DONE ;

	return err ;
}


static pascal OSErr
AETextToBool ( DescType typeCode , Ptr dataPtr , Size dataSize ,
	DescType toType , long refCon , AEDesc * result ) {

static Str63 s ;
static Str63 com ;
Boolean b = 0 ;

	if ( dataSize > 63 ) {
		return errAECantSupplyType ;
	}
	if ( typeCode == typeChar && toType == typeBoolean ) {
		com [ 0 ] = dataSize ;
		BlockMove ( dataPtr , & com [ 1 ] , com [ 0 ] ) ;
		GetIndString ( s , 127 , 1 ) ;
		if ( ! IUEqualString ( s , com ) ) {
			b = 0 ;
			return AECreateDesc ( typeBoolean , ( Ptr ) & b , sizeof ( b ) , result ) ;
		}

		GetIndString ( s , 127 , 2 ) ;
		if ( ! IUEqualString ( s , com ) ) {
			b = 1 ;
			return AECreateDesc ( typeBoolean , ( Ptr ) & b , sizeof ( b ) , result ) ;
		}
	}
	return errAECantSupplyType ;
}


static void
InstallHandlers ( void ) {

	FailErr ( AEInstallEventHandler ( kCoreEventClass , kAEOpenApplication ,
		( void * ) OpenApplication , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kCoreEventClass , kAEQuitApplication ,
		( void * ) QuitApplication , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kAECoreSuite , kAEGetData ,
		( void * ) GetData , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kAECoreSuite , kAESetData ,
		( void * ) SetData , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kAETalkingClockSuite , kAESayTime ,
		( void * ) AESpeakTime , 0L , 0 ) ) ;
	FailErr ( AEInstallEventHandler ( kAETalkingClockSuite , kAESayDate ,
		( void * ) AESpeakDate , 0L , 0 ) ) ;

	FailErr ( AEInstallCoercionHandler ( typeChar , typeBoolean , AETextToBool ,
		0L , 0 , 0 ) ) ;
}
