/*	NAME:
		MenuSelect.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a Trap Patch.

	NOTES:
		�	Causes the Mac to beep when About... is selected from the Apple
			menu.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			�	First publicly distributed version.

		(Mar 1994, dg)
			�	Extended address table, uses code block to play sound.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include "StandaloneCode.h"
#include "BTAddrsTable.h"
#include "ESConstants.h"
#include "CodeConstants.h"





//=============================================================================
//		Private defines																 
//-----------------------------------------------------------------------------
#define AppleSymbol				0xF0				// The Apple symbol





//=============================================================================
//		Global Variables																 
//-----------------------------------------------------------------------------
pascal long	(*gPrevMenuSelect) (Point pt);
pascal void	(*gPlaySound) (void);
Boolean		gAlreadyRan=false;





//=============================================================================
//		Private function prototypes							 
//-----------------------------------------------------------------------------
pascal	long main(Point pt);
short	IsAppleMenu(MenuHandle menuH);










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	We call the original MenuSelect, then do some post processing.
//				The net effect is to make the Mac beep when you select
//				About.. from the Apple menu.
//-----------------------------------------------------------------------------
pascal long main(Point pt)
{	long			retVal;					// Long returned with menu id and item number
	MenuHandle		menuH;
	short			menuIDNum;
	BTAddressTable	*theAddressTable;
	



	// Get access to our globals, and save off A4
	PatchGetGlobals();

	
	
	// If we've not already been called, call the Gestalt selector to get
	// the address of the old MenuSelect and the routine to play the
	// sound. Cast the table entry to a long first so we get what's pointed
	// to by it, rather than a pointer to it.
	if (!gAlreadyRan)
		{
		Gestalt(kBellTestAddressTable, &theAddressTable);		
		gPrevMenuSelect	= (ProcPtr) ((long) theAddressTable->theTable[kMenuSelect]);
		gPlaySound		= (ProcPtr) ((long) theAddressTable->theTable[kPlaySound]);
		gAlreadyRan		= true;
		}

	
	
	// Call the original MenuSelect()
	retVal = (*gPrevMenuSelect)(pt);


	
	// If the user made a choice (menu ID != 0)
	if (menuIDNum = HiWord(retVal))
		{
		// Get a handle to the menu.
		menuH = GetMHandle(menuIDNum);
		
	
		
		// If it's the Apple menu, and it's the first item,
		// then call our block to play the sound.
		if (IsAppleMenu(menuH) && (LoWord(retVal) == 1))
			(*gPlaySound)();
		}
	
	

	// Restore A4 for our caller and return
	PatchUngetGlobals();
	return(retVal);
}










//=============================================================================
//		IsAppleMenu : Is a given menu the Apple menu?
//-----------------------------------------------------------------------------
short IsAppleMenu(MenuHandle menuH)
{	




	// Quick check
	if (!menuH)
		return(false);
	
	
	
	// Real check
	if (((*menuH)->menuData[0] == 1) &&
			(((*menuH)->menuData[1] == 0x14) || ((*menuH)->menuData[1] == AppleSymbol)))
		return(true);
	else
		return(false);
}
