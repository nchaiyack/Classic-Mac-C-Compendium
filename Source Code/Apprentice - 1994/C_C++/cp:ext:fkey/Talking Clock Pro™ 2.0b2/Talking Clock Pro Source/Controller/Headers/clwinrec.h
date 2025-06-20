/*
 * clwinrec.h
 */

#pragma once
#ifndef clwinrec_h
# define clwinrec_h

# ifndef window_h
#  include "window.h"
# endif

extern DefWindowRec clwinRec ;

extern OSErr ClwinCr ( WindowPtr , Handle * , FSSpec * ) ;
extern OSErr ClwinDe ( WindowPtr , Handle ) ;
extern OSErr ClwinUp ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinMD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinMU ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinKD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinAK ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinAc ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinSw ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr ClwinId ( WindowPtr , Handle , long * ) ;
extern OSErr ClwinPr ( WindowPtr , Handle ) ;
extern OSErr ClwinCo ( WindowPtr , Handle , short , short , unsigned char * ) ;
extern OSErr ClwinAE ( WindowPtr , Handle , AppleEvent * , AppleEvent * ) ;

void ClWinWrite ( short refNum , WindowPtr wp ) ;
void ClWinRead ( short refNum , WindowPtr wp ) ;
void ClWinRunClock ( WindowPtr wp , long * sleep , short nowMin , short nowSec ) ;

#endif /* clwinrec_h */
