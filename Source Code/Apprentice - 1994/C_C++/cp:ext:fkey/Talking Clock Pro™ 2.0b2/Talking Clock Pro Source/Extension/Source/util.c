/*
 * util.c
 */

#include <Types.h>
#include <GestaltEqu.h>
#include <Events.h>

#include "util.h"
#include "speech.h"


Boolean
SpeechAvailable ( void ) {

static Boolean cache = 0 , init = 0 ;

	if ( ! init ) {
		cache = CheckGestaltBit ( gestaltSpeechAttr , gestaltSpeechMgrPresent ) ;
		init = 1 ;
	}
	return cache ;
}


Boolean
ColorAvailable ( void ) {

static Boolean cache = 0 , init = 0 ;
long gResp ;

	if ( ! init ) {
		cache = ! ( Gestalt ( gestaltQuickdrawVersion , & gResp ) ||
		( gResp < 0x100 ) ) ;
		init = 1 ;
	}
	return cache ;
}


Boolean
OffscreenAvailable ( void ) {

static Boolean cache = 0 , init = 0 ;

	if ( ! init ) {
		cache = CheckGestaltBit ( gestaltQuickdrawFeatures , gestaltHasDeepGWorlds ) ;
		init = 1 ;
	}
	return cache ;
}


Boolean
KeyIsDown ( unsigned short keyCode ) {

unsigned char keyMap [ 16 ] ;

	GetKeys ( ( void * ) keyMap ) ;
	return 1 & ( keyMap [ keyCode >> 3 ] >> ( keyCode & 7 ) ) ;
}


Boolean
CheckGestaltBit ( long selector , long bitNumber ) {

long resp ;

	if ( Gestalt ( selector , & resp ) || ! ( resp & ( 1 << bitNumber ) ) ) {
		return 0 ;
	}
	return 1 ;
}


OSErr
SysSpeakString ( unsigned char * str ) {

OSErr ret ;

	SetZone ( SystemZone ( ) ) ; /* SysZone */
	ret = SpeakString ( str ) ;
	SetZone ( ApplicZone ( ) ) ;
	return ret ;
}
