/*****
 *
 *	MenuEdit.c
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

#include "MenuFunc.h"
#include "WindowInt.h"

#include "MenuEdit.h"


/***  FUNCTIONS  ***/

void
doEditMenu ( short theItem, short modifiers )
{
	if ( !(SystemEdit (theItem)) )	/* SystemEdit returns true if desk accessory is front */
	{
		/* support for edit menus here used in application */
		switch (theItem)
		{
			case kmiUndo :
				break;
			
			case kmiCut :
				break;
			
			case kmiCopy :
				break;
			
			case kmiPaste :
				break;
			
			case kmiClear :
				break;
			
			case kmiSelectAll :
				break;
			
			default :
				break;
		}
	}
} /* doEditMenu */


void
adjustEditMenu ( window_type theWindowType )
{
	switch ( theWindowType )
	{
		case Window_DA :
		case Window_dlgModal :
		case Window_dlgMoveableModal :
			EnableItem (gmEditMenu, kmiUndo);
			EnableItem (gmEditMenu, kmiCut);
			EnableItem (gmEditMenu, kmiCopy);
			EnableItem (gmEditMenu, kmiPaste);
			EnableItem (gmEditMenu, kmiClear);
			DisableItem (gmEditMenu, kmiSelectAll);
			break;

		case Window_UNKNOWN :
		case Window_about :
		case Window_none :
		default :
			DisableItem (gmEditMenu, kmiUndo);
			DisableItem (gmEditMenu, kmiCut);
			DisableItem (gmEditMenu, kmiCopy);
			DisableItem (gmEditMenu, kmiPaste);
			DisableItem (gmEditMenu, kmiClear);
			DisableItem (gmEditMenu, kmiSelectAll);
	}
	
	EnableItem ( gmEditMenu, kmTheWholeMenu );
} /* adjustEditMenu */


/*****  EOF  *****/
