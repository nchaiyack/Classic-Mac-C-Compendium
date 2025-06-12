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
#define kMenuSelect					1							// Index into arrays
#define kPlaySound					2							// Index into arrays

#define kBellTestAddressTable		'BeLl'						// Address table selector
#define kMenuSelectResType			'CODE'						// Resource type of patch
#define kPlaySoundResType			'CODE'						// Resource type of code block
#define kMenuSelectResID			128							// Resource id of patch
#define kPlaySoundResID				129							// Resource id of code block



#endif
