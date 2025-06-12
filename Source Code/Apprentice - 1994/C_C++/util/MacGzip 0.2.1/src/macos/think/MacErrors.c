/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include <string.h>
#include <errno.h>

#define Alert			151
#define Warning			150
#define	ErrResType		'Errs'

char _strerr[256];

/************************************************************************************/

void  Calert( char* theMsg )
{	
	CtoPstr(theMsg);
	ParamText(theMsg,"\p","\p","\p");
	PtoCstr(theMsg);
	StopAlert(Alert,NULL);
}

int  Cask( char* theMsg )
{
	CtoPstr(theMsg);
	ParamText(theMsg,"\p","\p","\p");
	return( CautionAlert(Warning,NULL) );
}

void	PError( char *s )
{
	Str255 s2,s1;
	Handle myHandle;
	
	sprintf((char*)_strerr,"%s: %s", s, strerror(errno));
	CtoPstr((char*)_strerr);
	
	if ( errno != 0 )
	{
		if((myHandle=GetResource('Errs',errno))==nil)
		{
			NumToString( (long) errno, s2 );
			ParamText(_strerr,"\p",s2,"\p???");
		}	
		else
			ParamText(_strerr,"\p",*myHandle,(*myHandle)+**myHandle+1);
	}
	else ParamText(_strerr,"\p","\p","\p");

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
			ParamText(s1,"\p",*myHandle,(*myHandle)+**myHandle+1);
	}
	else ParamText(s1,"\p","\p","\p");
	StopAlert(Alert,NULL);
	if ( exiting ) ExitToShell();
}

