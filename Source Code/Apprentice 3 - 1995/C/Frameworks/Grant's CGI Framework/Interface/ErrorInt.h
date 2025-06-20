#pragma once
/*****
 *
 *	ErrorInt.h
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

/***  CONSTANT DECLARATIONS  ***/

#define krErrorDialogSystem		300
#define krErrorDialogStartup	301


/***  FUNCTION PROTOTYPES  ***/

	void	ErrorAlertSystemString	( OSErr, StringHandle, Str255 * );
	void	ErrorAlertStartupString	( short, Str255 * );


/***** EOF *****/
