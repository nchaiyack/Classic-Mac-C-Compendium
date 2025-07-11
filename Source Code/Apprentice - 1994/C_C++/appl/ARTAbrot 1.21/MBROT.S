/************************************************************************/
/*																		*/
/*	FILE:	MBROT.S -- MANDELBROT DSP3210 PROGRAM						*/
/*																		*/
/*	PURPOSE:	DSP3210 routines to generate Mandelbrot image data.		*/
/*																		*/
/*	AUTHOR:	George T. Warner											*/
/*																		*/
/*	NOTES:	Variables at param block must be initialized				*/
/*			before setting Startflag to allow the program to run.  The	*/
/*			results are stored in RAM starting at address OutputData.	*/
/*			Program completes by writing a nonzero value to Doneflag.	*/
/*																		*/
/*	REVISIONS:															*/
/*	10/22/93	GTW	Increased maximum line length to 1280 pixels.		*/
/*	08/22/93	GTW	Converted from VCOS to ARTA code.					*/
/*	02/14/92	GTW	Cleaned up.											*/
/*	12/23/91	GTW	First working DSP3210 version.						*/
/*																		*/
/************************************************************************/

#include "DSP3210.h"

/* Macro for PC relative addressing. */
#define AddressPR(LAB) pc + LAB - (.+8)

NewModule(MBrot, kdspSmoothModule, kdspAutoCache, cmain)
	long	100, 4, 655		/* I don't know about this. */

NewCachedProgramSection(cmain, MBrot)
NewParameterSection(brobuf, kdspAppSpecificData, MBrot)

AppendSection(cmain)
	*sp++ = r5
	*sp++ = r6
	*sp++ = r8
	*sp++ = r10
	*sp++ = r11
	*sp++ = r13
	*sp++ = r14
	*sp++ = r18					/* Push r18 onto stack. */

	GetSectionAddress(r5, brobuf)	/* Get address for brobuf and put in r5.	*/
	/* Once I get the section base address in a register (r5), I can make all	*/
	/* future accesses by register relative addressing.							*/

	r2 = r5 + (Inited - Startflag)		// r2 = Inited

	r1 = *r2
	nop
	if(ne) pcgoto Wait_for_go	/* Go ahead if initialized. */
	r1 = (short) 1

	r3 = r5 + (Startflag - Startflag)		// r3 = Startflag
	*r3 = r0					/* Clear startflag. */
	r4 = r5 + (Doneflag - Startflag)		// r4 = Doneflag
	*r4 = r0					/* Clear doneflag. */
	*r2 = r1					/* Set inited flag. */

Wait_for_go:
	r4 = r5 + (Startflag - Startflag)		// r4 = Startflag
	r3 = *r4
	nop
	if(eq) pcgoto exit			/* Be a "good citizen" and return. */
	nop

	*r4 = r0					/* Clear Startflag. */

	r13 = r0					/* Zero point counter. */
	r10 = r5 + (OutputData - Startflag)		// r10 = OutputData			/* Set pointer to output data. */
	r14 = r5 + (Maxiter - Startflag)		// r14 = Maxiter
	r14 = *r14					/* Get the maximum number of iterations. */
	r8 = AddressPR(Scrfl)					/* Set pointer to temp storage. */
	
	r2 = r5 + (Xstart - Startflag)		// r2 = Xstart					/* Convert IEEE format. */
	r3 = AddressPR(myXstart)
	/* Use these to convert from IEEE to DSP3210 format. */
	*r3++ = a0 = dsp(*r2++)
	*r3++ = a0 = dsp(*r2++)
	*r3 = a0 = dsp(*r2)
								/* Compute y coordinate. */
	/* r11 - iteration count. */
	/* r13 - point counter. */
	/* r14 - max iterations. */
	/* r8  - pointer to temp storage. */
Mloop:
	r2 = AddressPR(Scrlong)		/* Set pointer to temp storage. */
	*r2 = r13					/* Store in memory. */
	r3 = AddressPR(myDelta)		/* Set pointer to delta x. */
	a0 = float32(*r2)			/* Convert count to floating point. */
	r6 = AddressPR(myXstart)	/* Set pointer to base x. */
	r11 = AddressPR(Zero)
	a2 = *r6 + a0 * *r3			/* Coordinate=base+(pointnum * deltaX) */
	r3 = AddressPR(myYstart)
	a3 = *r3					/* Get imaginary cooridinate (y). */
	a0 = *r11					/* Zero the accumulator. */
	a1 = *r11					/* Zero the accumulator. */
	*r8++ = a2 = a2				/* Save real c. */
	*r8-- = a3 = a3				/* Save imaginary c. */
	r11 = r0					/* Zero iteration count. */

Mandel: 
	pccall TestMag(r18)
	r11 = r11 + 1				/* Increment iteration count. */

	if(ne) pcgoto Pointdone		/* This point diverged; it's done. */
	nop

	r11 - r14					/* Compare to max. num. of iterations. */
	if(ge) pcgoto Pointdone		/* Return if greater than or equal to. */
	nop


	/* Now the famous z = z^2 + C function */
DoTheBrot:
	a2 = a0 * a1
	a3 = a0 * a0
	a0 = a3 - a1 * a1			/* This gives the real part of z squared. */
	a1 = a2 + a2				/* This gives the imaginary part of z squared. */
	a0 = a0 + *r8++				/* This is the +C (real) part of the equation. */
	pcgoto Mandel
	a1 = a1 + *r8--				/* This is the +C (img.) part of the equation. */

Pointdone:
	*r10++ = (short)r11			/* Save result. */
	r2 = r5 + (Points - Startflag)		// r2 = Points
	r2 = *r2					/* Get the number of points to do. */
	r13 = r13 + 1
	r2 - r13					/* Are there more points to do? */
	if(gt) pcgoto Mloop			/* If so, go do next point. */
	nop 

Linedone:
	r14 = (short)0x69			/* Just an arbitrary non-zero number. */
	r4 = r5 + (Doneflag - Startflag)	// r4 = Doneflag
	*r4 = r14					/* Indicate that DSP is done. */

exit:
	r4 = sp - 4
	r18 = *r4--					/* Pull r18 off stack. */
	r14 = *r4--					/* Pull r14 off stack. */
	r13 = *r4--					/* Pull r13 off stack. */
	r11 = *r4--					/* Pull r11 off stack. */
	r10 = *r4--					/* Pull r10 off stack. */
	r8 = *r4--					/* Pull r8 off stack. */
	r6 = *r4--					/* Pull r6 off stack. */
	r5 = *r4--					/* Pull r5 off stack. */
	sp = sp - 32				/* Restore stack pointer. */
	return(r18)
	nop

/************************************************************************/
/* TestMag -- subroutine to compare magnitude of complex number in a0	*/
/*		and a1 to 2.0.  If greater, r1 = 1 upon exiting; if				*/
/*		less, r1 = 0.													*/
/************************************************************************/
TestMag:
	a3 = a0 * a0				/* Square real part. */
	a3 = a3 + a1 * a1			/* Add to img. part squared. */
	r2 = AddressPR(Four)
	a3 = a3 - *r2				/* Compare to 4.0. */
	nop
	nop
	nop
	if(alt) pcgoto Donef		/* If negative result, goto Done. */
	r1 = r0
	r1 = (short)1				/* Result did not diverge. */
Donef:
	return(r18)
	r1 - 0						/* Set flags based on result. */

Four:		float 4.0			/* Number for magnitude comparisons. */
Zero:		float 0.0
Scrfl:		2 * float			/* Scratch pad. */
Scrlong:	long				/* Scratch pad. */
myXstart:	float	0.0
myDelta:	float	0.0
myYstart:	float	0.0


AppendSection(brobuf)
/* Note: Xstart, Delta, and Ystart all hold IEEE representations of	*/
/* floating point values.  Hence, they have been declared as longs	*/
/* in order that they not be confused with DSP floats.				*/

Startflag:	int		0
Doneflag:	int		0
Points:		long	640		/* Number of points. */
Maxiter:	long	63		/* Maximum number of iterations. */
Xstart:		long	80		/* Starting real (x) coordinate. */
Delta:		long	80		/* Delta x. */
Ystart:		long	80		/* y coordinate. */
Inited:		long	0		/* Initialization flag. */
OutputData:
			1280 * short
