/* error_dlog.c 
 * 
 * 5/4/94 by Darrell Anderson
 * brings up a window with a string and a number (OSErr, typically).
 */
 
 #include <Packages.h>
 #include <string.h>
 #include "error_dlog.h"
 
 #define k_dlogH 100
 #define k_dlogV 100
 #define k_dlogWidth 450
 #define k_dlogHeight 72
 
 void ReportError( char *errMsg, short errNum )
 //
 // bring up a window to display an error message in.  wait for a mouseclick, then
 // exitToShell();
 //
 // an option to continue would be nice..
 //
 {
 	WindowPtr		errWinPtr;
 	Rect			winRect;
 	GrafPtr			oldPort;
 	Str255			errNumStr;
 	Str255 			errTextStr;
 	short			strlength;
 	short 			fontNum;
 	
 	// convert the c-string to a pascal string:
 	
 	// get the length of the string in characters
 	strlength = strlen( errMsg );
 	
 	// truncate anything over 255 characters
 	if( strlength > 255 )
 		strlength = 255;
 	
 	// set the length byte	
 	*((char *)errTextStr) = (unsigned char) strlength;
 	
 	// copy the string over
 	strncpy( (char *)(errTextStr+1), errMsg, strlength );
 	
 	GetPort( &oldPort );
 	
 	// what rect should the window occupy? (in global coords)
 	SetRect( &winRect, k_dlogH, k_dlogV, k_dlogH + k_dlogWidth, k_dlogV + k_dlogHeight );
 	
 	errWinPtr = NewWindow( nil, &winRect, "\pErrorWindow", false, dBoxProc, 
 							(WindowPtr)-1L, false, 0L );
 	
 	ShowWindow( errWinPtr );
 	SetPort( errWinPtr );

	MoveTo( 13,23 );
	
	TextFace( bold );
	DrawString( "\pError: " );
	TextFace( normal );
	
	// write the error message
	DrawString( errTextStr );
	
	// write the associated OSErr number (or whatever was passed..)
	MoveTo( 13,40 );
	NumToString( errNum, &errNumStr );
	TextFace( bold );
	DrawString( "\pErrNum: " );
	TextFace( normal );
	
	// write the actual number
	DrawString( errNumStr );
	
	MoveTo( 13, 57 );
	DrawString( "\pPress the mouse button to exit." );
	
	while( !Button() )
		;
	
	SetPort( oldPort );
	
	HideWindow( errWinPtr );
	DisposeWindow( errWinPtr );
	
	ExitToShell();
}
	