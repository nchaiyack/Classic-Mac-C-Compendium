/*****
 *
 *	Quit.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "globals.h"

#include "WindowInt.h"

#include "Quit.h"


/***  INTERFACE  ***/

/* Handle an interface or scripting call to quit the applciation */
void
doQuitApp ( void )
{
	/* quit with user interaction allowed */
	QuitPrepare ( true );
} /* doQuitApp */


/***  FUNCTIONS  ***/
#pragma mark -

/* Confirm that the application can quit, prompt the user if necessary.
	Clean up, if the application can quit.
	Return true if application is set to quit, false if it can't quit. */
Boolean
QuitPrepare ( Boolean allowUserInteract )
{
	Boolean	success;
	
	/* Should add check for outstanding events and handle them as appropriate.
		Especially important is to check for any queued high level events. */
	
	/* get rid of document windows */
	success = WindowCloseAll ( allowUserInteract );
	
	if ( success || !allowUserInteract )
	{
		/* can quit if successful or user is not allowed to interact */
		gQuit = true;
	}
	
	return gQuit;
} /* QuitPrepare */


/* force the application to quit, doing necessary cleanup. */
void
ForceQuit ( void )
{
	/* quit with no user interaction */
	QuitPrepare ( false );
	
	ExitToShell ();
} /* ForceQuit */


/*****  EOF  *****/
