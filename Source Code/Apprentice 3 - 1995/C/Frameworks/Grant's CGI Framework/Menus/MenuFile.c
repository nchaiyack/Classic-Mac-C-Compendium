/*****
 *
 *	MenuFile.c
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

#include "MenuFunc.h"
#include "Quit.h"
#include "WindowInt.h"

#include "MenuFile.h"


/***  FUNCTIONS  ***/

/*  */
void
doFileMenu ( short theItem, short modifiers )
{
	switch ( theItem ) 
	{
		case kmiClose :
			WindowMenuClose ( modifiers );
			break;
		
		case kmiQuit :
			doQuitApp ();
			break;
		
		default :
			/* invalid item */
			break;
	}
	
} /* doFileMenu */


/*  */
void
adjustFileMenu ( window_type theWindowType )
{
	switch ( theWindowType )
	{
		case Window_about :
		case Window_DA :
			EnableItem ( gmFileMenu, kmiClose );
			EnableItem ( gmFileMenu, kmiQuit );
			break;
		
		case Window_none :
			DisableItem ( gmFileMenu, kmiClose );
			EnableItem ( gmFileMenu, kmiQuit );
			break;
		
		case Window_UNKNOWN :
		case Window_dlgModal :
		case Window_dlgMoveableModal :
		default :
			DisableItem ( gmFileMenu, kmiClose );
			DisableItem ( gmFileMenu, kmiQuit );
			break;
	}
	
	EnableItem (gmFileMenu, kmTheWholeMenu);
} /* adjustFileMenu */


/*****  EOF  *****/
