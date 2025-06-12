/*
 * clockrec.h
 */

#pragma once
#ifndef clockrec_h
# define clockrec_h

# ifndef window_h
#  include "window.h"
# endif

extern DefWindowRec clockRec ;

extern OSErr ClockCr ( WindowPtr , Handle * , FSSpec * ) ;
extern OSErr ClockDe ( WindowPtr , Handle ) ;
extern OSErr ClockUp ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockMD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockMU ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockKD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockAK ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockAc ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockSw ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClockId ( WindowPtr , Handle , long * ) ;
extern OSErr ClockPr ( WindowPtr , Handle ) ;
extern OSErr ClockCo ( WindowPtr , Handle , short , short , unsigned char * ) ;
extern OSErr ClockAE ( WindowPtr , Handle , AppleEvent * , AppleEvent * ) ;

extern Boolean FindPSN ( OSType type , OSType creator ,
	ProcessSerialNumber * psn ) ;

#endif /* clockrec_h */
