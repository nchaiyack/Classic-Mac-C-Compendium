/*****
 *
 *	MenuApple.c
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

#include "AboutBox.h"
#include "MenuFunc.h"
#include "WindowInt.h"

#include "MenuApple.h"


/***  FUNCTIONS  ***/

void
doAppleMenu ( short itemNumber, short modifiers )
{
	Str255		itemName;
	
	if ( itemNumber == kmiAbout )
	{
		 AboutBoxOpen ();
	}
	else
	{
		/* desk accessory or other Apple menu item */
		GetItem		( gmAppleMenu, itemNumber, itemName );
		OpenDeskAcc	( itemName );
	}
} /* doAppleMenu */


/*  */
void
adjustAppleMenu ( window_type theWindowType )
{
	EnableItem ( gmAppleMenu, kmiAbout );
	EnableItem ( gmAppleMenu, kmTheWholeMenu );
} /* adjustAppleMenu */


/*****  EOF  *****/
