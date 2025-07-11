/*	NAME:
		AddrsTable.h

	WRITTEN BY:
		Dair Grant
				 
	DESCRIPTION:
		Header file for AddrsTable.c. Contains the definition of a standard
		AddressTable for use with Extension Shell.

	___________________________________________________________________________
*/
#ifndef __ADDRSTABLE__
#define __ADDRSTABLE__
//=============================================================================
//		Include files																 
//-----------------------------------------------------------------------------
#include "ESConstants.h"





//=============================================================================
//		Structures																 
//-----------------------------------------------------------------------------
// An AddressTable is an array of ProcPtrs.
typedef struct {
	ProcPtr			theTable[kMaxNumCodeResources+1];
} AddressTable;


#endif
