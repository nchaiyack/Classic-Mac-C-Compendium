/*****
 *
 *	SplashScreen.c
 *
 *	If you don't want a splash screen for your CGI, remove this file and the two
 *	SplashScreen calls made in the function 'StartupApplication' in "Startup.c".
 *	Also remove the resources: cicn #128, ICON #128, dctb #401, DITL #401, DLOG #401
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "globals.h"

#include "DebugUtil.h"

#include "SplashScreen.h"


/***  LOCAL VARIABLES ***/

static	DialogPtr	vSplashScreenDlog;
static	GrafPtr		vSplashScreenSavePort;


/***  LOCAL CONSTANTS ***/

#define kSplashScreenDLOG	401


/***  FUNCTIONS  ***/

/*  */
void
SplashScreenCreate ( void )
{
	ParamText ( gVersionStr, nil, nil, nil );
	
	/* get dialog ptr from resource file */
	vSplashScreenDlog = GetNewDialog ( kSplashScreenDLOG, nil, (WindowPtr)-1L );
	
	if ( vSplashScreenDlog != nil )
	{
		/* dialog load successful */
		SetWRefCon ( vSplashScreenDlog, (long)kSplashScreenDLOG );
		
		/* save the current port, whatever it is */
		GetPort ( &vSplashScreenSavePort );
		
		/* now make it visible */
		ShowWindow		( vSplashScreenDlog );
		SelectWindow	( vSplashScreenDlog );
		DrawDialog		( vSplashScreenDlog );
	}
} /* SplashScreenCreate */


/*  */
void
SplashScreenDispose ( void )
{
	if ( vSplashScreenDlog != nil )
	{
		my_assert ( vSplashScreenSavePort != nil, "\pSplashScreenDispose: vSplashScreenSavePort is nil" );
		
		DisposeDialog	( vSplashScreenDlog );
		SetPort			( vSplashScreenSavePort );
	}
} /* SplashScreenDispose */


/*****  EOF  *****/
