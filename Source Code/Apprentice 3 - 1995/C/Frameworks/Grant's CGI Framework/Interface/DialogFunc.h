#pragma once
/*****
 *
 *	DialogFunc.h
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

/***  CONSTANT DECLARATIONS  ***/

#define kDefaultButtonID		1	/* used in all dialogs: button 1 is OK */


/***  FUNCTION PROTOTYPES  ***/

	pascal Boolean	defaultAlert1ButtonEventFilter	( DialogPtr, EventRecord *, short * );


/***** EOF *****/
