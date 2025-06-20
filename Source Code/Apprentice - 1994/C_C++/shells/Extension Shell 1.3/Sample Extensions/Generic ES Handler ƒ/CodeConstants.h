/*	NAME:
		CodeConstants.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Header file for various Extension specific constants

	NOTES:
		�	The defines in this file serve as indexes into the code details array
			that's embedded in a ESParamBlock, for interpreting the value of
			errorIndex, and for indexing into an address table.
		
		�	It's also a good idea to use this file to define the various resource
			types/IDs that your Extension uses.
		
	___________________________________________________________________________
*/
#ifndef __CODECONSTANTS__
#define __CODECONSTANTS__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
#define kCodeThingOne				1							// Some code resource
#define kCodeThingTwo				2							// Some code resource
#define kCodeThingThree				3							// Some code resource
#define kCodeThingFour				4							// Some code resource
#define kCodeThingFive				5							// Some code resource
#define kCodeThingSix				6							// Some code resource

#define kThingOneResType			'CODE'						// Resource type of some code
#define kThingTwoResType			'CODE'						// Resource type of some code
#define kThingThreeResType			'CODE'						// Resource type of some code
#define kThingFourResType			'CODE'						// Resource type of some code
#define kThingFiveResType			'CODE'						// Resource type of some code
#define kThingSixResType			'CODE'						// Resource type of some code
#define kThingOneResID				128							// Resource ID of some code
#define kThingTwoResID				129							// Resource ID of some code
#define kThingThreeResID			130							// Resource ID of some code
#define kThingFourResID				131							// Resource ID of some code
#define kThingFiveResID				132							// Resource ID of some code
#define kThingSixResID				133							// Resource ID of some code



#endif
