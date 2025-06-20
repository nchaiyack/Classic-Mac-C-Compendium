/*
 * apprec.h
 */

#pragma once
#ifndef apprec_h
# define apprec_h

# ifndef window_h
#  include "window.h"
# endif

extern DefWindowRec appRec ;

extern OSErr AppCr ( WindowPtr , Handle * , FSSpec * ) ;
extern OSErr AppDe ( WindowPtr , Handle ) ;
extern OSErr AppUp ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppMD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppMU ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppKD ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppAK ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppAc ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppSw ( WindowPtr , Handle , EventRecord * ) ;
extern OSErr AppId ( WindowPtr , Handle , long * ) ;
extern OSErr AppPr ( WindowPtr , Handle ) ;
extern OSErr AppCo ( WindowPtr , Handle , short , short , unsigned char * ) ;
extern OSErr AppAE ( WindowPtr , Handle , AppleEvent * , AppleEvent * ) ;

void ClWinAdd ( WindowPtr wp , Handle data ) ;
void ClWinRemove ( WindowPtr wp , Handle data ) ;
void AppSaveWindows ( Handle data ) ;
void ClWinWrite ( short ref , WindowPtr wp ) ;

#endif
