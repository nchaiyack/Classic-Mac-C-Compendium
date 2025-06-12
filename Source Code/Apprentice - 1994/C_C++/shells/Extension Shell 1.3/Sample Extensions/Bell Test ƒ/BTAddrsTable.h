/*	NAME:
		BTAddrsTable.h

	WRITTEN BY:
		Dair Grant
				 
	DESCRIPTION:
		Header file for AddrsTable.c. Contains the definition of an extended
		AddressTable, for use with the Bell Test Extension.

	___________________________________________________________________________
*/
#ifndef __BELLTESTADDRSTABLE__
#define __BELLTESTADDRSTABLE__
//=============================================================================
//		Include files																 
//-----------------------------------------------------------------------------
#include "ESConstants.h"





//=============================================================================
//		Structures																 
//-----------------------------------------------------------------------------
// Our extended address table is two ProcPtrs, and a Handle
typedef struct {
	ProcPtr			theTable[3];			// Runs from 1..2
	Handle			theSound;				// The noise to play
} BTAddressTable;



#endif
