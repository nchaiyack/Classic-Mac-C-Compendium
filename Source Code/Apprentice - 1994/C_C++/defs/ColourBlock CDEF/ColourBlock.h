/*	NAME:
		ColourBlock.h

	WRITTEN BY:
		Dair Grant
		grantd@dcs.gla.ac.uk
				
	DESCRIPTION:
		Header file for ColourBlock.c.

	___________________________________________________________________________
*/
#ifndef __COLOURBLOCK__
#define __COLOURBLOCK__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
#ifndef PackRGB
#define PackRGB(col, r, g, b)		col.red=r; col.green=g; col.blue=b
#endif


// Pushes most registers onto the stack, and puts a copy of the address of main into A4.
// Should be called at start of stand-alone code resource to get access to it's globals.
#define GetGlobals()		asm { 													\
								move.l 		A4, -(SP)		; save old A4			\
								lea 		main, A4 		; get globals			\
								}


// The converse of GetGlobals(). Restore the value of A4						
#define UngetGlobals()		asm {													\
								move.l 		(SP)+, A4		; restore A4 from stack	\
								}



//=============================================================================
//		Structures																 
//-----------------------------------------------------------------------------
#pragma once
typedef struct {
	RGBColor	blockColour;
} CDEFStruct, *CDEFStructPtr, **CDEFStructHnd;


#endif
