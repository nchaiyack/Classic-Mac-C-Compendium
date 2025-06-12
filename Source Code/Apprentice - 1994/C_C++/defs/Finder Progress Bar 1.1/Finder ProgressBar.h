/*****************************************************************************************************
*                                                                                                    *
* Finder ProgressBar.h - Copyright 1993 - 1994 Chris Larson, All rights reserved                     *
*                                              (cklarson@engr.ucdavis.edu)                           *
*                                                                                                    *
* This is the header file for a CDEF which mimics the progress bar used by the Finder. This file and *
* compiled derivatives may be freely used within any freeware/shareware/postcardware/beerware/É as   *
* long as you mention my name in your credits. Neither this source nor its compiled derivatives are  *
* in the public domain and may not be use in any form in public domain software. Neither this source *
* nor its compiled derivatives may be used in any form in a commercial product without the expressed,*
* written consent of the author (me).                                                                *
*                                                                                                    *
* Version 1.1 -- April 28, 1994.                                                                     *
*                                                                                                    *
*****************************************************************************************************/

// Constants

#define kNoColorQD	0x4000	// If this bit in ROM85 is set, color QD is not available.

#define	cBlack		{ 0x0000, 0x0000, 0x0000 }
#define cWhite		{ 0xFFFF, 0xFFFF, 0xFFFF }
#define cGray		{ 0x4444, 0x4444, 0x4444 }
#define cBlue		{ 0xCCCC, 0xCCCC, 0xFFFF }

// Inline Functions

// ----------
// NOTE: These two routines are not intended as a general replacement of the SetUpA4() and
//		 RestoreA4() functions. THEY ARE NOT GENERAL; THEY WILL NOT WORK ANYWHERE BUT HERE.
// ----------

void MySetUpA4(void)
	= {
		0x2F0C,   //	move.l	a4,-(a7)	; Push a4
		0x2848	  //	movea.l	a0,a4		; Load address of code resource into a4
	  };

void MyRestoreA4(void)
	= {
		0x285F    //	move.l	(a7)+,a4	; Pop a4
	  };

// ----------
// Notes about CalculateBarBoundry():
//
// This function calculates the location of the edge of the progress bar given the current
// control settings. The calculation corresponds to the following equation:
//
//		                   ( boxRight - boxLeft ) * ( controlValue - controlMinimum )
//		result = boxLeft + ----------------------------------------------------------
//		                              ( controlMaximum - controlMinimum )
//
// Correct operation of this function requires the following conditions:
//		(1)	controlMax >=ÊcontrolValue >= controlMinimum
//		(2)	boxRight >= boxLeft
//
// Both of these sould be met in all normal curcumstances (i.e. control maxima should always
// be greater than control minima; control values should always be between the control's maximum
// and minimum; and the left coordinate of the control's rectangle should be less than the right
// edge (otherwise the rectangle is empty). Imposing these conditions allows me to calculate
// the correct answer no matter what values these variables contain. For example, controlMax and
// controlMin are both short integers (16 bits) limited to the range [-32768,32767]. Without the
// restriction that controlMin <= controlMax, the difference ( controlMax - controlMin ) could
// be anywhere within the range [-65535,65535] which is a 17 bit signed quantity. With the
// restriction the difference is limited to the range [0,65535] which fits into an unsigned
// 16 bit word. Big deal; registers are 32 bits. Why not just use 32 bit signed quantities?
// Well, then when I computed the product in the numerator I would have to use a quad word (64 bits)
// to store it. The quad word multiplication routines are implemented in software on the 68000
// and require inclusion of a library that is larger than this code resource. I wanted to avoid
// needing this library, if possible, without requiring a 68020. Further, normal use of the control
// obeys all of my conditions anyway so I felt it was well worthwhile.
//
// As it stands, the algorithm computes the two differences in the numerator, storing each into a
// 16 bit unsigned word. If the control value and control minimum are the same, there is no progress
// to display, so the left edge is returned (actually it was given in d0 and d0 has not been altered
// so I only have to exit). Otherwise, the two unsigned 16 bit words are multiplied together to
// yield a 32 bit unsigned long word product. The difference in the denominator is now calculated and
// placed into a 16 bit unsigned word. Note that no check is made to see if a divide by zero will
// occur. This is owing to the conditions outlined above: if the max and min are equal (the only
// way to get a zero in the denominator) then the control value must be equal to both. This case
// was detected in the earlier check so I need not repeat it. The division is carried out, resulting
// in a 16 bit unsigned quotient. This quotient is then added to boxLeft to yield the 16 bit
// signed result value.
//
// The routine is coded in inline assembly because I couldnÕt get this #@!%ing compiler to recognize
// and use the mulu.w and divu.w instructions properly (it was using the library even though it was
// not strictly necessary).
// ----------

#pragma parameter __D0 CalculateBarBoundry(__D0, __D1, __A0)
short CalculateBarBoundry(short boxLeft, short boxRight, ControlPtr theControl)
	= {
		0x9240,			//			sub.w	d0,d1			; Box width to d1
		0x3268, 0x0014,	//			movea.w	$0014(a0),a1	; Control minimum to a1
		0x3428, 0x0012,	//			move.w	$0012(a0),d2	; Control value to d2
		0x9449,			//			sub.w	a1,d2			; Normalized control setting to d2
		0x670C,			//			beq		@done			; Value == minimum so weÕre done
		0xC2C2,			//			mulu.w	d2,d1			; ( width * normalized setting ) to d1
		0x3428, 0x0016,	//			move.w	$0016(a0),d2	; Control maximum to d2
		0x9449,			//			sub.w	a1,d2			; Control range to d2
		0x82C2,			//			divu.w	d2,d1			; (width*normalized setting)/range to d1
		0xD041,			//			add.w	d1,d0			; result to d0
						// @done:
	  };

// Function Prototypes

pascal long main(short varCode, ControlHandle theControlHandle, short message, long param);
void DrawProgressBar(ControlPtr theControl);
