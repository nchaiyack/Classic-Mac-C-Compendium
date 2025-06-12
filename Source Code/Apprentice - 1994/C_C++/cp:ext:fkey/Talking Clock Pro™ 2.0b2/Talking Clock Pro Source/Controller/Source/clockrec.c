/*
 * clockrec.c
 */

#include <AERegistry.h>
#include <AEObjects.h>

#include <PlotIconSuite.h>
#include "TalkConstants.h"

#include "apprec.h"
#include "clockrec.h"
#include "x.h"
#include "menu.h"
#include "window.h"
#include "util.h"
#include "str.h"


#define DAEMON_TYPE 'appe'

Boolean hasController ;

DefWindowRec clockRec = {

	128 , NULL ,

	ClockCr ,
	ClockDe ,
	ClockUp ,
	ClockMD ,
	ClockMU ,
	ClockKD ,
	ClockAK ,
	ClockAc ,
	ClockSw ,
	ClockId ,
	ClockPr ,
	ClockCo ,
	ClockAE

} ;


typedef struct ClockRec {
	ProcessSerialNumber	psn ; /* The extension's process */
	short				turnaround ;
	Boolean				optionKeyTalk ;
	Boolean				controlKeyTalk ;
	Boolean				commandKeyTalk ;
	Boolean				shiftKeyTalk ;
	Boolean				capsLockKeyTalk ;
	Boolean				fiveMinuteTalk ;
} ClockRec , * ClockPtr ;


typedef struct Switch {
	short				xPos ;
	short				yPos ;
	short				xSize ;
	short				ySize ;
	short				nameIndex ;
	OSType				prop ;
} Switch , * SwitchPtr ;


SwitchPtr switches = NULL ;
int NUM_SWITCHES = 0 ;



static void
ReadSwitches ( void ) {

Handle h ;
int ix ;

	NUM_SWITCHES = Count1Resources ( 'SWTC' ) ;
	switches = ( SwitchPtr ) NewPtrClear ( sizeof ( Switch ) * NUM_SWITCHES ) ;
	for ( ix = 0 ; ix < NUM_SWITCHES ; ix ++ ) {
		h = Get1IndResource ( 'SWTC' , ix + 1 ) ;
		HLock ( h ) ;
		BlockMove ( * h , switches + ix , sizeof ( Switch ) ) ;
		ReleaseResource ( h ) ;
	}
}


static void
DrawSwitch ( SwitchPtr sw , Boolean b ) {

Rect r ;
Str63 name ;

	PenNormal ( ) ;
	TextFont ( GetAppFont ( ) ) ;
	TextSize ( sw -> ySize ) ;
	SetRect ( & r , sw -> xPos , sw -> yPos , sw -> xPos + sw -> ySize ,
		sw -> yPos + sw -> ySize ) ;
	FrameRect ( & r ) ;
	if ( b ) {
		MoveTo ( r . left , r . top ) ;
		Line ( sw -> ySize - 1 , sw -> ySize - 1 ) ;
		MoveTo ( r . left , r . bottom - 1 ) ;
		Line ( sw -> ySize - 1 , 1 - sw -> ySize ) ;
	}
	MoveTo ( sw -> xPos + 2 * sw -> ySize , sw -> yPos + sw -> ySize * 4 / 5 + 1 ) ;
	GetIndString ( name , 132 , sw -> nameIndex ) ;
	DrawString ( name ) ;
}


static Boolean
TrackSwitch ( SwitchPtr sp ) {

Rect r , box ;
Boolean in = 0 ;
Boolean toIn = 1 ;
Point p ;

	SetRect ( & r , sp -> xPos , sp -> yPos , sp -> xPos + sp -> xSize ,
		sp -> yPos + sp -> ySize ) ;
	box = r ;
	box . right = box . left + sp -> ySize ;
	InsetRect ( & box , 1 , 1 ) ;
	PenMode ( patXor ) ;
	while ( StillDown ( ) ) {
		GetMouse ( & p ) ;
		toIn = PtInRect ( p , & r ) ;
		if ( toIn != in ) {
			FrameRect ( & box ) ;
			in = toIn ;
		}
	}
	if ( in ) {
		FrameRect ( & box ) ;
	}
	PenNormal ( ) ;

	return in ;
}


Boolean
FindPSN ( OSType type , OSType creator , ProcessSerialNumber * psn ) {

ProcessInfoRec prInfo ;

	psn -> highLongOfPSN = 0 ;
	psn -> lowLongOfPSN = kNoProcess ;
	while ( ! GetNextProcess ( psn ) ) {
		prInfo . processInfoLength = sizeof ( ProcessInfoRec ) ;
		prInfo . processName = NULL ;
		prInfo . processAppSpec = NULL ;
		FailErr ( GetProcessInformation ( psn , & prInfo ) ) ;
		if ( prInfo . processType == type &&
			prInfo . processSignature == creator ) {
			return 1 ;
		}
	}
	return 0 ;
}


static Boolean
GetRecData ( ClockPtr ptr , DescType theKey ) {

AEDesc aed ;
AERecord rec ;
AppleEvent aevt ;
AppleEvent reply = { 0 , 0 } ;
DescType type ;
Boolean b ;
long theSize ;

	FailErr ( AECreateDesc ( typeProcessSerialNumber , ( Ptr ) & ( ptr -> psn ) ,
		sizeof ( ProcessSerialNumber ) , & aed ) ) ;
	FailErr ( AECreateAppleEvent ( kAECoreSuite , kAEGetData , & aed ,
		kAutoGenerateReturnID , kAnyTransactionID , & aevt ) ) ;
	FailErr ( AEDisposeDesc ( & aed ) ) ;
	FailErr ( AECreateList ( NULL , 0L , 1 , & rec ) ) ;
	type = cProperty ;
	FailErr ( AEPutKeyPtr ( & rec , keyAEDesiredClass , typeType ,
		( Ptr ) & type , sizeof ( type ) ) ) ;
	type = theKey ;
	FailErr ( AEPutKeyPtr ( & rec , keyAEKeyData , typeType ,
		( Ptr ) & type , sizeof ( type ) ) ) ;
	FailErr ( AEPutParamDesc ( & aevt , keyDirectObject , & rec ) ) ;
	FailErr ( AEDisposeDesc ( & rec ) ) ;
	type = typeBoolean ;
	FailErr ( AEPutParamPtr ( & aevt , keyAERequestedType , typeType ,
		( Ptr ) & type , sizeof ( type ) ) ) ;
	FailErr ( AESend ( & aevt , & reply , kAEWaitReply , kAEHighPriority ,
		20L , NULL , NULL ) ) ;
	FailErr ( AEDisposeDesc ( & aevt ) ) ;
	FailErr ( AEGetParamPtr ( & reply , keyAEResult , typeBoolean , & type ,
		( Ptr ) & b , sizeof ( b ) , & theSize ) ) ;
	FailErr ( AEDisposeDesc ( & reply ) ) ;

	return b ;
}


static void
SetRecData ( ClockPtr ptr , DescType theKey , Boolean b ) {

AEDesc aed = { 0 , 0 } ;
AERecord rec = { 0 , 0 } ;
AppleEvent aevt = { 0 , 0 } ;
AppleEvent reply = { 0 , 0 } ;
DescType type ;

	TRY {
		FailErr ( AECreateDesc ( typeProcessSerialNumber , ( Ptr ) & ( ptr -> psn ) ,
			sizeof ( ProcessSerialNumber ) , & aed ) ) ;
		FailErr ( AECreateAppleEvent ( kAECoreSuite , kAESetData , & aed ,
			kAutoGenerateReturnID , kAnyTransactionID , & aevt ) ) ;
		FailErr ( AECreateList ( NULL , 0L , 1 , & rec ) ) ;
		type = cProperty ;
		FailErr ( AEPutKeyPtr ( & rec , keyAEDesiredClass , typeType ,
			( Ptr ) & type , sizeof ( type ) ) ) ;
		type = theKey ;
		FailErr ( AEPutKeyPtr ( & rec , keyAEKeyData , typeType ,
			( Ptr ) & type , sizeof ( type ) ) ) ;
		FailErr ( AEPutParamDesc ( & aevt , keyDirectObject , & rec ) ) ;
		FailErr ( AEPutParamPtr ( & aevt , keyAEData , typeBoolean ,
			( Ptr ) & b , sizeof ( b ) ) ) ;
		FailErr ( AESend ( & aevt , & reply , kAENoReply , kAEHighPriority ,
			20L , NULL , NULL ) ) ;
	} CLEANUP {
		FailErr ( AEDisposeDesc ( & rec ) ) ;
		FailErr ( AEDisposeDesc ( & aed ) ) ;
		FailErr ( AEDisposeDesc ( & aevt ) ) ;
		FailErr ( AEDisposeDesc ( & reply ) ) ;
		if ( __err == errAETimeout ) {
			NO_PROPAGATE ;
		}
	} DONE ;
}


static void
FillRec ( ClockPtr ptr ) {

	ptr -> optionKeyTalk = GetRecData ( ptr , pOptionKeyTalk ) ;
	ptr -> commandKeyTalk = GetRecData ( ptr , pCommandKeyTalk ) ;
	ptr -> controlKeyTalk = GetRecData ( ptr , pControlKeyTalk ) ;
	ptr -> shiftKeyTalk = GetRecData ( ptr , pShiftKeyTalk ) ;
	ptr -> capsLockKeyTalk = GetRecData ( ptr , pCapsLockKeyTalk ) ;
	ptr -> fiveMinuteTalk = GetRecData ( ptr , pFiveMinuteTalk ) ;
}


static void
SetRec ( ClockPtr ptr ) {

	SetRecData ( ptr , pOptionKeyTalk , ptr -> optionKeyTalk ) ;
	SetRecData ( ptr , pCommandKeyTalk , ptr -> commandKeyTalk ) ;
	SetRecData ( ptr , pControlKeyTalk , ptr -> controlKeyTalk ) ;
	SetRecData ( ptr , pShiftKeyTalk , ptr -> shiftKeyTalk ) ;
	SetRecData ( ptr , pCapsLockKeyTalk , ptr -> capsLockKeyTalk ) ;
	SetRecData ( ptr , pFiveMinuteTalk , ptr -> fiveMinuteTalk ) ;
}


/*	A window is being created or opened. Allocate data and		*/
/*	select the window									*/
OSErr
ClockCr ( WindowPtr wp , Handle * data , FSSpec * file ) {

ClockPtr ptr ;

	if ( ! NUM_SWITCHES ) {
		ReadSwitches ( ) ;
	}
	* data = NewHandle ( sizeof ( ClockRec ) ) ;
	FailNil ( * data ) ;
	HLockHi ( * data ) ;
	ptr = ( ClockPtr ) * * data ;
	if ( ! FindPSN ( DAEMON_TYPE , 'O\'Cl' , & ( ptr -> psn ) ) ) {
		Alert ( 130 , NULL ) ;
		hasController = 0 ;
		return 1 ;
	} else {
		FillRec ( ptr ) ;
		hasController = 1 ;
	}
	return noErr ;
}


/*	Window is being destroyed. Put up a warning dialog, and		*/
/*	return errCancel if the user cancels closing - else call	*/
/*	DisposeWindow here											*/
OSErr
ClockDe ( WindowPtr wp , Handle data ) {

ProcessSerialNumber psn ;

	if ( hasController ) {
		SetRec ( ( ClockPtr ) * data ) ;
		hasController = 0 ;
		if ( FindPSN ( 'FNDR' , 'MACS' , & psn ) ) {
			FailErr ( SetFrontProcess ( & psn ) ) ;
		}
	}
	DisposeHandle ( data ) ;
	DisposeWindow ( wp ) ;
	return noErr ;
}


/*	Update the window - BeginUpdate is already called			*/
OSErr
ClockUp ( WindowPtr wp , Handle data , EventRecord * event ) {

short ix ;
Handle h ;
ClockPtr ptr = ( ClockPtr ) * data ;
Rect r = { 10 , 10 , 42 , 42 } ;
Str255 s ;

	FailErr ( GetIconSuite ( & h , 128 , -1L ) ) ;
	EraseRect ( & ( wp -> portRect ) ) ;
	FailErr ( PlotIconSuite ( & r , atNone , ttNone , h ) ) ;
	FailErr ( DisposeIconSuite ( h , FALSE ) ) ;

	TextFont ( GetAppFont ( ) ) ;
	TextSize ( GetDefFontSize ( ) ) ;

	SetRect ( & r , 52 , 10 , wp -> portRect . right - 8 , 40 ) ;
	GetIndString ( s , 131 , 1 ) ;
	TextBox ( & s [ 1 ] , s [ 0 ] , & r , teFlushDefault ) ;

	TextSize ( GetDefFontSize ( ) * 4 / 5 ) ;

	SetRect ( & r , 10 , 52 , wp -> portRect . right - 8 , 80 ) ;
	GetIndString ( s , 131 , 2 ) ;
	TextBox ( & s [ 1 ] , s [ 0 ] , & r , teFlushDefault ) ;

	SetRect ( & r , 10 , wp -> portRect . bottom - 35 ,
		wp -> portRect . right - 8 , wp -> portRect . bottom ) ;
	GetIndString ( s , 131 , 3 ) ;
	TextBox ( & s [ 1 ] , s [ 0 ] , & r , teFlushDefault ) ;

	for ( ix = 0 ; ix < NUM_SWITCHES ; ix ++ ) {
		DrawSwitch ( & switches [ ix ] , !! ( & ( ptr -> optionKeyTalk ) ) [ ix ] ) ;
	}
	return noErr ;
}


/*	The user clicked in your window. The window port is set and	*/
/*	event -> where is translated into local coordinates now		*/
OSErr
ClockMD ( WindowPtr wp , Handle data , EventRecord * event ) {

short ix ;
Rect r ;
SwitchPtr sp = switches ;
ClockPtr ptr = ( ClockPtr ) * data ;

	for ( ix = 0 ; ix < NUM_SWITCHES ; ix ++ ) {
		SetRect ( & r , sp -> xPos , sp -> yPos , sp -> xPos + sp -> xSize ,
			sp -> yPos + sp -> ySize ) ;
		if ( PtInRect ( event -> where , & r ) ) {
			break ;
		}
		sp ++ ;
	}
	if ( ix < NUM_SWITCHES ) {
		if ( TrackSwitch ( sp ) ) {
			( & ( ptr -> optionKeyTalk ) ) [ ix ] = ! ( & ( ptr -> optionKeyTalk ) ) [ ix ] ;
			SetPort ( wp ) ;
			InvalRect ( & r ) ;
		}
	}
	return noErr ;
}


/*	MouseUp in a window - you probably should do nothing		*/
OSErr
ClockMU ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	The user typed in the window - command keys are already		*/
/*	taken care of and don't come here							*/
OSErr
ClockKD ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	You probably just want to call the KeyDown handler here		*/
OSErr
ClockAK ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	Activate event - highlight/dehighlight controls & caret		*/
OSErr
ClockAc ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	You should probably build a suitable activate event and		*/
/*	send to the activate handling for MultiFinder switches		*/
OSErr
ClockSw ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	Idle time - set the sleep parameter if it's too large for	*/
/*	your needs. GetCaretTime() is a good value for text editors	*/
OSErr
ClockId ( WindowPtr wp , Handle data , long * sleep ) {

	return AppId ( wp , app . data , sleep ) ;
}


/*	This is called when menus need to be updated.				*/
OSErr
ClockPr ( WindowPtr wp , Handle data ) {

	FailErr ( AppPr ( wp , app . data ) ) ;
	EnableCmd ( FILE_MENU , CLOSE_ITEM ) ;
	return noErr ;
}


/*	A menu selection was made - take appropriate action			*/
OSErr
ClockCo ( WindowPtr wp , Handle data , short menu , short item ,
	unsigned char * itemStr ) {

	if ( menu == FILE_MENU && item == CLOSE_ITEM ) {
		DestroyWindow ( wp ) ;
	} else {
		FailErr ( AppCo ( wp , app . data , menu , item , itemStr ) ) ;
	}

	return noErr ;
}


/*	An AppleEvent was received with this window in front		*/
OSErr
ClockAE ( WindowPtr wp , Handle data , AppleEvent * event ,
	AppleEvent * reply ) {

	return noErr ;
}

