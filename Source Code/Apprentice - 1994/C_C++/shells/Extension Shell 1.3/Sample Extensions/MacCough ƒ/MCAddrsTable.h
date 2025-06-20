/*	NAME:
		MCAddrsTable.h

	WRITTEN BY:
		Dair Grant
				 
	DESCRIPTION:
		Header file for MCAddrsTable.c. Contains the definition of an extended
		AddressTable, for use with the MacCough Extension.

	___________________________________________________________________________
*/
#ifndef __MACCOUGHADDRSTABLE__
#define __MACCOUGHADDRSTABLE__
//=============================================================================
//		Include files																 
//-----------------------------------------------------------------------------
#include "ESConstants.h"





//=============================================================================
//		Structures																 
//-----------------------------------------------------------------------------
// Our extended address table is one ProcPtr, and a Handle
typedef struct {
	ProcPtr			theTable[2];			// Runs from 1..1
	Handle			theSound;				// The noise to play
} MCAddressTable;



#endif
