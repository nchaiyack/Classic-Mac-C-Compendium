/*
 * window.h
 */


#pragma once
#ifndef window_h
# define window_h


# define errCancel 1

typedef struct defWindowRec {

	short				windID ;
	Handle				data ;

	OSErr				( * OnCreate ) ( WindowPtr , Handle * , FSSpec * ) ;
	OSErr				( * OnDestroy ) ( WindowPtr , Handle ) ;

	OSErr				( * DoUpdate ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoMouseDown ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoMouseUp ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoKeyDown ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoAutoKey ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoActivate ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoSwitch ) ( WindowPtr , Handle , EventRecord * ) ;
	OSErr				( * DoIdle ) ( WindowPtr , Handle , long * ) ;
	OSErr				( * PrepareMenus ) ( WindowPtr , Handle ) ;
	OSErr				( * DoCommand ) ( WindowPtr , Handle , short menu , short item ,
							unsigned char * itemStr ) ;
	OSErr				( * DoAppleEvent ) ( WindowPtr , Handle , AppleEvent * , AppleEvent * ) ;

} DefWindowRec ;


extern Boolean quit ;
extern long gSleep ;
extern RgnHandle gMouse ;
extern EventRecord gEvent ;
extern Rect gDragLimit ;
extern Rect gSizeLimit ;
extern DefWindowRec app ;


WindowPtr MakeWindow ( FSSpec * file , DefWindowRec * ) ;
OSErr DestroyWindow ( WindowPtr window ) ;

void DriveWindow ( void ) ;
void HandleEvent ( EventRecord * event ) ;
void IdleFrontWindow ( void ) ;

DefWindowRec * GetFrontWindow ( WindowPtr * wp ) ;
DefWindowRec * RecFromWindow ( WindowPtr window ) ;

Boolean InFront ( void ) ;
void Notify ( void ) ; 
void DeNotify ( void ) ;

#endif /* window_h */

