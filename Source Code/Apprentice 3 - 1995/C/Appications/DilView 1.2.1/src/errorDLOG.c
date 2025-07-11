/* errorDLOG.c */

#include <Dialogs.h>
#include <stdlib.h>
#include <string.h>

#define k_alert_ID 999

void ErrorDLOG( char *errMsg, Boolean fatal );

void ErrorDLOG( char *errMsg, Boolean fatal )
{
 	GrafPtr			oldPort;
 	Str255 			errTextStr;
 	short			strlength;
 	short			itemSelected;
 	DialogPtr		errDLOG;
 	short i;
 	
 	// convert the c-string to a pascal string:
 	 	
 	// get the length of the string in characters
 
 	//strlength = strlen( errMsg ); // Sorry, no ANSI ):
 	strlength = 0;
 	while( errMsg[strlength] != 0 )
 		strlength++;
 	
 	// truncate anything over 255 characters
 	if( strlength > 255 )
 		strlength = 255;
 	
 	// set the length byte	
 	*((char *)errTextStr) = (unsigned char) strlength;
 	
 	// copy the string over
 	
 	// strncpy( (char *)(errTextStr+1), errMsg, strlength ); // Sorry, no ANSI
 	for( i=0 ; i<strlength ; i++ )
 		((char *)errTextStr)[i+1] = errMsg[i];
 	
 	// use it as ^0 in the DLOG 
 	ParamText( errTextStr, 0, 0, 0 );

 	// bring up the dialog
 	GetPort( &oldPort );
	SysBeep(0);

	errDLOG = GetNewDialog( k_alert_ID, nil, (WindowPtr)-1L );
	ShowWindow( errDLOG );
	ModalDialog( nil, &itemSelected );
	DisposeDialog( errDLOG );

	SetPort( oldPort );
	
	// handle the button choice.
	switch( itemSelected )
	{
		case -1: 	// dialog not drawn!!
					break;
		default:	// don't do anything.. we only have one button!
					break;
	}
	
	if( fatal )
		ExitToShell();
}
	
	
	