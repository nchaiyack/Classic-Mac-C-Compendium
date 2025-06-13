/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include <string.h>
#include <errno.h>

/* 20may95,ejo: things needed from stdio also */
#include <stdio.h>
#include "MacErrors.h"  /* ejo: for prototypes */
#include <Dialogs.h>
#include <Resources.h>
#include <TextUtils.h>
#include <Strings.h>
#include <SegLoad.h>
#ifndef CtoPstr
#include <Strings.h>
#define CtoPstr(a) c2pstr (a)
#endif

#define Alert			151
#define Warning			150
#define	ErrResType		'Errs'

char _strerr[256];

/************************************************************************************/

void  Calert( char* theMsg )
{	
	ParamText(CtoPstr(theMsg),"\p","\p","\p");

	StopAlert(Alert,NULL);
}

int  Cask( char* theMsg )
{
	ParamText(CtoPstr(theMsg),"\p","\p","\p");
	
	return( CautionAlert(Warning,NULL) );
}

void	PError( char *s )
{
	Str255 s2;
	Handle myHandle;
	
	sprintf((char*)_strerr,"%s: %s", s, strerror(errno));
	CtoPstr((char*)_strerr);
	
	if ( errno != 0 )
	{
		if((myHandle=GetResource('Errs',errno))==nil)
		{
			NumToString( (long) errno, s2 );
			ParamText((StringPtr)_strerr,"\p",s2,"\p???");
		}	
		else
			ParamText(	(StringPtr)_strerr,
						"\p",
						(StringPtr)(*myHandle),
						(StringPtr)((*myHandle)+**myHandle+1)
					);
	}
	else ParamText((StringPtr)_strerr,"\p","\p","\p");

	StopAlert(Alert,NULL);
}

/************************************************************************************/


void  theAlert( int ErrorInd,int ErrorType, OSErr theOSErr, Boolean exiting)
{
	Str255 s2,s1;
	Handle myHandle;
	
	if (ErrorType!=0) GetIndString( s1, ErrorType, ErrorInd);
	else *s1=0x00;
	
	if ( theOSErr != 0 )
	{
		if((myHandle=GetResource(ErrResType,theOSErr))==nil)
		{
			NumToString( (long) theOSErr, s2 );
			ParamText(s1,"\p",s2,"\p???");
		}	
		else
			ParamText(	s1,
						"\p",
						(StringPtr)(*myHandle),
						(StringPtr)((*myHandle)+**myHandle+1)
					);
	}
	else ParamText(s1,"\p","\p","\p");
	StopAlert(Alert,NULL);
	if ( exiting ) ExitToShell();
}

