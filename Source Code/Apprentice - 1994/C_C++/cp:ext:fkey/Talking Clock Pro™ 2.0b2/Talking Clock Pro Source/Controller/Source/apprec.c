/*
 * apprec.c
 */

#include "apprec.h"
#include "menu.h"
#include "x.h"
#include "clockrec.h"
#include "clwinrec.h"


DefWindowRec appRec = {

	128 , NULL ,

	AppCr ,
	AppDe ,
	AppUp ,
	AppMD ,
	AppMU ,
	AppKD ,
	AppAK ,
	AppAc ,
	AppSw ,
	AppId ,
	AppPr ,
	AppCo ,
	AppAE

} ;


long startedTicks = 60000 ;

#define CURRENT_VER 3

typedef struct AppData {

	FSSpec			theFile ;
	Handle			theWindows ;

} AppData , * AppPtr ;


static void
AppAboutItem ( ) {
	Alert ( 129 , NULL ) ;
}


static void
AppAppleMenu ( short item , unsigned char * itemStr ) {

	if ( item == ABOUT_ITEM ) {
		AppAboutItem ( ) ;
	} else {
		OpenDeskAcc ( itemStr ) ;
	}
}


void
ClWinAdd ( WindowPtr wp , Handle data ) {

AppPtr ptr = ( AppPtr ) * data ;

	PtrAndHand ( ( Ptr ) & wp , ptr -> theWindows , sizeof ( WindowPtr ) ) ;
}


void
ClWinRemove ( WindowPtr wp , Handle data ) {

AppPtr ptr = ( AppPtr ) * data ;
WindowPtr * wpp = ( WindowPtr * ) * ( ptr -> theWindows ) ;
int num = GetHandleSize ( ptr -> theWindows ) / sizeof ( WindowPtr ) ;
Boolean del = 0 ;

	while ( num -- ) {
		if ( del ) {
			wpp [ -1 ] = * wpp ;
		} else {
			if ( * wpp == wp ) {
				del = 1 ;
			}
		}
		wpp ++ ;
	}
	SetHandleSize ( ptr -> theWindows , GetHandleSize ( ptr -> theWindows ) -
		sizeof ( WindowPtr ) ) ;
}


static void
AddPrefResource ( AppPtr ptr ) {

volatile Handle h = NULL ;
volatile short refNum ;

	TRY {
		h = GetResource ( 'STR ' , -16397 ) ; /* Magic prefs string */
		FailNil ( h ) ;
		DetachResource ( h ) ;
		FSpCreateResFile ( & ( ptr -> theFile ) , 'pref' , 'pref' , smSystemScript ) ;
		refNum = FSpOpenResFile ( & ( ptr -> theFile ) , fsRdWrPerm ) ;
		if ( refNum < 1 ) {
			FailErr ( ResError ( ) ) ;
		}
		AddResource ( h , 'STR ' , -16397 , "\p" ) ;
		h = NULL ;
		FailErr ( ResError ( ) ) ;
	} CLEANUP {
		if ( refNum > 0 ) {
			CloseResFile ( refNum ) ;
			refNum = 0 ;
		}
		if ( h ) {
			DisposeHandle ( h ) ;
		}
	} DONE ;
}


void
AppSaveWindows ( Handle data ) {

AppPtr ptr = ( AppPtr ) * data ;
WindowPtr * wpp = ( WindowPtr * ) * ( ptr -> theWindows ) ;
short num = GetHandleSize ( ptr -> theWindows ) / sizeof ( WindowPtr ) ;
short refNum = 0 , temp , ver ;
long size ;

	TRY {
		HLock ( ptr -> theWindows ) ;
		FSpCreate ( & ( ptr -> theFile ) , 'pref' , 'pref' , smSystemScript ) ;
		FailErr ( FSpOpenDF ( & ( ptr -> theFile ) , fsRdWrPerm , & refNum ) ) ;
		FailErr ( SetEOF ( refNum , 0L ) ) ;
		ver = CURRENT_VER ;
		size = sizeof ( ver ) ;
		FailErr ( FSWrite ( refNum , & size , ( Ptr ) & ver ) ) ;
		size = sizeof ( num ) ;
		FailErr ( FSWrite ( refNum , & size , ( Ptr ) & num ) ) ;
		while ( num -- ) {
			ClWinWrite ( refNum , * wpp ) ;
			wpp ++ ;
		}
	} CLEANUP {
		HUnlock ( ptr -> theWindows ) ;
		if ( refNum ) {
			FSClose ( refNum ) ;
			refNum = 0 ;
		}
	} DONE ;
	AddPrefResource ( ptr ) ;
}


static void
AppFileMenu ( short item , Handle data ) {

WindowPtr wp ;
extern Boolean hasController ;

	switch ( item ) {
	case NEW_ITEM :
		wp = MakeWindow ( NULL , & clwinRec ) ;
		if ( wp ) {
			SelectWindow ( wp ) ;
			ShowWindow ( wp ) ;
		}
		break ;
	case OPEN_ITEM :
		if ( ! hasController ) {
			wp = MakeWindow ( NULL , & clockRec ) ;
			SelectWindow ( wp ) ;
			ShowWindow ( wp ) ;
		}
		break ;
	case QUIT_ITEM :
		quit = 1 ;
		break ;
	}
}


static void
ReadWindows ( FSSpec * fss ) {

volatile short refNum = 0 ;
long size ;
short num , ver ;
WindowPtr wp ;
volatile Boolean del = 0 ;

	TRY {
		if ( ! FSpOpenDF ( fss , fsRdPerm , ( short * ) & refNum ) ) {
			size = sizeof ( ver ) ;
			FailErr ( FSRead ( refNum , & size , ( Ptr ) & ver ) ) ;
			if ( ver != CURRENT_VER ) {
				Alert ( 132 , NULL ) ;
				del = 1 ;
			}
			if ( ! del ) {
				size = sizeof ( num ) ;
				FailErr ( FSRead ( refNum , & size , ( Ptr ) & num ) ) ;
				while ( num -- ) {
					wp = MakeWindow ( NULL , & clwinRec ) ;
					ClWinRead ( refNum , wp ) ;
					if ( wp ) {
						SelectWindow ( wp ) ;
						ShowWindow ( wp ) ;
					}
				}
			}
		}
	} CLEANUP {
		if ( refNum ) {
			FSClose ( refNum ) ;
			refNum = 0 ;
		}
	} DONE ;
	if ( del ) {
		FailErr ( FSpDelete ( fss ) ) ;
	}
}


/*	This function will be called with a preferences file spec	*/
/*	The name of the prefs file is specified in STR id 1			*/
OSErr
AppCr ( WindowPtr wp , Handle * data , FSSpec * file ) {

	startedTicks = TickCount ( ) + 180L ;
	* data = NewHandle ( sizeof ( AppData ) ) ;
	FailNil ( * data ) ;
	HLockHi ( * data ) ;
	( ( AppPtr ) * * data ) -> theFile = * file ;
	( ( AppPtr ) * * data ) -> theWindows = NewHandle ( 0L ) ;

	ReadWindows ( file ) ;

	return noErr ;
}


/*	This is a good place to write back your preferences; it is	*/
/*	called right before the application quits					*/
OSErr
AppDe ( WindowPtr wp , Handle data ) {

	return noErr ;
}


/*	Update events for windows that don't handle them themselves	*/
/*	are sent here.												*/
OSErr
AppUp ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	MouseDown events in window content that are not handled by	*/
/*	windows are sent here										*/
OSErr
AppMD ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	MouseUp events in window content that are not handled by	*/
/*	windows are sent here										*/
OSErr
AppMU ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	KeyDown events in window content that are not handled by	*/
/*	windows are sent here										*/
OSErr
AppKD ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	AutoKey events in window content that are not handled by	*/
/*	windows are sent here										*/
OSErr
AppAK ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	Activate events for windows that are not handled by the		*/
/*	window are sent here										*/
OSErr
AppAc ( WindowPtr wp , Handle data , EventRecord * event ) {
	return noErr ;
}


/*	MultiFinder events for windows that are not handled by the	*/
/*	window are sent here										*/
OSErr
AppSw ( WindowPtr wp , Handle data , EventRecord * event ) {

extern Boolean hasController ;

	if ( ( ! FrontWindow ( ) ) || ( TickCount ( ) > startedTicks ) ) {
		if ( ( event -> message & 1 ) && ! hasController ) {
			wp = MakeWindow ( NULL , & clockRec ) ;
			SelectWindow ( wp ) ;
			ShowWindow ( wp ) ;
		}
	}
	return noErr ;
}


/*	When the front window does not handle idling, this is		*/
/*	called														*/
OSErr
AppId ( WindowPtr wp , Handle data , long * sleep ) {

AppPtr ptr = ( AppPtr ) * data ;
WindowPtr * wpp = ( WindowPtr * ) * ( ptr -> theWindows ) ;
int num = GetHandleSize ( ptr -> theWindows ) / sizeof ( WindowPtr ) ;
DateTimeRec dtr ;
unsigned long now ;

	GetDateTime ( & now ) ;
	Secs2Date ( now , & dtr ) ;
	HLock ( ptr -> theWindows ) ;
	TRY {
		while ( num -- ) {
			if ( * wpp ) {
				ClWinRunClock ( * wpp , sleep , dtr . minute , dtr . second ) ;
			}
			wpp ++ ;
		}
	} CLEANUP {
		HUnlock ( ptr -> theWindows ) ;
	} DONE ;

	return noErr ;
}


/*	This is called when menus need to be updated.				*/
OSErr
AppPr ( WindowPtr wp , Handle data ) {

extern Boolean hasController ;

	EnableCmd ( APPLE_MENU , ABOUT_ITEM ) ;

	EnableCmd ( FILE_MENU , NEW_ITEM ) ;
	if ( ! hasController ) {
		EnableCmd ( FILE_MENU , OPEN_ITEM ) ;
	}
	EnableCmd ( FILE_MENU , QUIT_ITEM ) ;

	return noErr ;
}


/*	This is called when the front window does not handle		*/
/*	commands													*/
OSErr
AppCo ( WindowPtr wp , Handle data , short menu , short item ,
	unsigned char * itemStr ) {
	switch ( menu ) {
	case APPLE_MENU :
		AppAppleMenu ( item , itemStr ) ;
		break ;
	case FILE_MENU :
		AppFileMenu ( item , data ) ;
		break ;
	}
	return noErr ;
}


/*	This is called when the front window does not handle		*/
/*	AppleEvents													*/
OSErr
AppAE ( WindowPtr wp , Handle data , AppleEvent * event ,
	AppleEvent * reply ) {
	return noErr ;
}


