/*
 * str.h
 */

#pragma once
#ifndef str_h
# define str_h

void StrGet ( short id , unsigned char * str , short maxLen ) ;
void CopyC2P ( char * cStr , unsigned char * pStr ) ;
void ClearMem ( void * mem , int size ) ;
void ConcatPStrings ( unsigned char * dest , unsigned char * src ) ;
void MemCopy ( void * src , void * dst , int len ) ;
void CopyPString ( unsigned char * src , unsigned char * dst ) ;

#endif /* str_h */
