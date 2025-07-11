/*	NAME:
		CircleMouse ][.c

	WRITTEN BY:
		Mark Pilgrim
				
	MODIFIED BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a VBL task.

	NOTES:
		�	Causes the cursor to move in a circle when the mouse button is
			pressed.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			�	Source to this module was taken from Mark's MouseBroken cdev
				source. It was altered slightly for use with Extension Shell.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <Retrace.h>
#include <GestaltEQU.h>
#include "StandaloneCode.h"
#include "CodeConstants.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
void	main(void);
Boolean CapsKeyDown(void);





//=============================================================================
//		Private defines															 
//-----------------------------------------------------------------------------
#define kRadius		5				// Actual radius is (1+2+...+(kRadius-1))+(kRadius/2)
#define kTimeDelay	1





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
extern Boolean	CrsrNew		: 0x8CE;			// Low memory globals
extern Point	mTemp		: 0x828;
extern Point	RawMouse	: 0x82C;

Boolean			gAlreadyRan=false;
int				gVx,gVy;
Boolean			gXIncreasing, gYIncreasing;










//=============================================================================
//		main : Does the actual work.																 
//-----------------------------------------------------------------------------
void main(void)
{	VBLTask*		myVBL;
	
	
	
	
	// Get our globals, and save the registers.
	PatchGetGlobals();



	// Move the pointer to the VBLTask into our local variable
	asm {
		move.l d0, myVBL
	}


	
	// If we've not already been called, prime ourselves
	if (!gAlreadyRan)
		{
		gVx	= kRadius;
		gVy	= 0;
		gXIncreasing = false;
		gYIncreasing = true;
		gAlreadyRan	= true;
		}



	// If the mouse button is down, and the caps lock key isn't - rotate	
	if (Button() && !CapsKeyDown())
		{
		RawMouse.h	+= gVx;
		mTemp.h		+= gVx;
		RawMouse.v	+= gVy;
		mTemp.v		+= gVy;
		
		gVx += gXIncreasing ? 1 : -1;
		gVy += gYIncreasing ? 1 : -1;
		
		gXIncreasing = (gVx == kRadius) ? false : (gVx == -kRadius) ? true : gXIncreasing;
		gYIncreasing = (gVy == kRadius) ? false : (gVy == -kRadius) ? true : gYIncreasing;
		
		CrsrNew = true;
	}

	
	
	// Reset our delay, restore A4 and other registers, and return
	myVBL->vblCount = kTimeDelay;
	PatchUngetGlobals();
}










//=============================================================================
//		CapsKeyDown : Returns true if the Caps-lock key is down.															 
//-----------------------------------------------------------------------------
Boolean CapsKeyDown(void)
{	unsigned char	theKeys[16];
	short			theCapsKey=57;




   GetKeys((void *) theKeys);
   return((theKeys[theCapsKey >> 3] >> (theCapsKey & 7)) & 1);
}
