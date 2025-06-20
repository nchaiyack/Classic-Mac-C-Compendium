/*
*********************************************************************
*	
*	pstr.c
*	Pascal string handling routines
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*	
*/
	
/*
******************************* Prototypes ***************************
*/


#include "pstr.h"

/**************************************
*	Copies Pascal string from s2 to s1
*	Return value:	Pointer to s1
*/

StringPtr pstrcpy( StringPtr s1, StringPtr s2 )	
{
	register short	length;
	StringPtr	   t=s1;
	
	for (length= *s2; length >= 0; --length)
		*s1++ = *s2++;
	return(t);
}

/**************************************
*	Appends Pascal string s2 to s1
*	Return value:	Pointer to s1
*/

StringPtr pstrcat(StringPtr s1, StringPtr s2)
{
	register StringPtr	p;
	register short			len, i;
	StringPtr				t=s1;
	
	if (*s1 + *s2 <= 255) {
		p = *s1 + s1 + 1;
		*s1 += (len = *s2++);
	}
	else {
		*s1 = 255;
		p = s1 + 256 - (len = *s2++);
	}
	for (i=len; i; --i) *p++ = *s2++;
	
	return(t);
}

/**************************************
*	Lexically compares Pascal strings s1 and s2
*	Return values:	0, if equal
*						>0, if s1 > s2
*						<0, if s2 > s1
*/

short pstrcmp(StringPtr s1,StringPtr s2)
{
	register short	len;
	short				len1,len2;
	
	len1= *s1;
	len2= *s2;
	len= (len1 <= len2)? len1 : len2;

	for(++s1,++s2;len;--len,++s1,++s2) {
		if(*s1 != *s2)
			return(*s1 - *s2);
	}
	if(len1 == len2)
		return(0);
	else if(len1 > len2)
		return(1);
	else return(-1);
}

/**************************************
*	Lexically compares the first n characters of Pascal strings s1 and s2
*	Return values:	0, if equal
*						>0, if s1 > s2
*						<0, if s2 > s1
*/

short pstrncmp(StringPtr s1,StringPtr s2,short n)
{
	register short	len;
	short				len1,len2;
	Boolean			shortstrings=TRUE;
	
	len1= *s1;
	len2= *s2;
	len= (len1 <= len2)? len1 : len2;
	if(len >= n) {
		len=n;
		shortstrings=FALSE;
	}
	
	for(++s1,++s2;len;--len,++s1,++s2) {
		if(*s1 != *s2)
			return(*s1 - *s2);
	}
	
	if(!shortstrings || len1 == len2)
		return(0);
	else if(len1 > len2)
		return(1);
	else return(-1);
}

/**************************************
*	Returns length of Pascal string
*	Return values:	first byte of the string as a short.
*/

short pstrlen(StringPtr s)
{
	if ( s == NULL )
		return(0);
		
	return(*(unsigned char *)s);
}