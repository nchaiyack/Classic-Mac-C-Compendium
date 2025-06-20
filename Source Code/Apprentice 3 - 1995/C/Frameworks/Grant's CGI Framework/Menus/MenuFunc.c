/*****
 *
 *	MenuFunc.c
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

#include "MenuApple.h"
#include "MenuFile.h"
#include "MenuEdit.h"
#include "WindowInt.h"

#include "MenuFunc.h"


/***  FUNCTIONS  ***/

/* Handle a menu selection */
void 
doMenu ( long menuResult, short modifiers  )
{
	short	menuID;
	short	itemNumber;
	
	/* determine which menu */
	menuID		= HiWord ( menuResult );
	/* determine which menu item */
	itemNumber	= LoWord ( menuResult );
	
	switch ( menuID ) 
	{
		case kmAppleMenuID :
			doAppleMenu ( itemNumber, modifiers );
			break;
			
		case kmFileMenuID :
			doFileMenu ( itemNumber, modifiers );
			break;
			
		case kmEditMenuID :
			doEditMenu ( itemNumber, modifiers );
			break;
	}
	
	HiliteMenu ( nil );
} /* doMenu */


/*  */
void
adjustMenus ( void )
{
	WindowPtr		theWindow;
	window_type		theWindowType;
	
	theWindow		= FrontWindow ();
	theWindowType	= WindowType ( theWindow );
	
	adjustAppleMenu	( theWindowType );
	adjustFileMenu	( theWindowType );
	adjustEditMenu	( theWindowType );
} /* MenusAdjust */


/*****  EOF  *****/
