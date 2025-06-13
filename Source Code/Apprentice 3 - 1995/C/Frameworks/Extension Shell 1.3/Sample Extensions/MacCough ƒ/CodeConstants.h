/*	NAME:
		CodeConstants.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Header file for various Extension specific constants

	NOTES:
		¥	The defines in this file serve as indexes into the code details array
			that's embedded in a ESParamBlock, for interpreting the value of
			errorIndex, and for indexing into an address table.
		
		¥	It's also a good idea to use this file to define the various resource
			types/IDs that your Extension uses.
		
	___________________________________________________________________________
*/
#ifndef __CODECONSTANTS__
#define __CODECONSTANTS__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
#define kTimeTask					1							// Index into arrays

#define kMacCoughAddressTable		'TiTa'						// Address table selector
#define kTimeTaskResType			'CODE'						// Resource type of task
#define kTimeTaskResID				128							// Resource id of task



#endif
