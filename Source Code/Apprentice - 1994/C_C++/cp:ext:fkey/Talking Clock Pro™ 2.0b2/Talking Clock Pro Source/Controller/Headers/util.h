/*
 * util.h
 */

#pragma once
#ifndef util_h
# define util_h

Boolean SpeechAvailable ( void ) ;
Boolean ColorAvailable ( void ) ;
Boolean OffscreenAvailable ( void ) ;
Boolean KeyIsDown ( unsigned short keyCode ) ;
Boolean CheckGestaltBit ( long selector , long bitNumber ) ;

OSErr SysSpeakString ( unsigned char * str ) ;

#endif /* util_h */
