//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains portable fixed-point routines
//|
//| This code was created by Greg Ferrar and various other
//| contributors.  There are no restrictions on the use
//| or distribution of this library, except that you may not
//| restrict its use or distribution.  Any project which is
//| created from this code, or any significant modification of
//| this code, may be distributed in any way you choose; you may
//| even sell it or its source code.  However, any limitations
//| placed on the distribution or use of such a project or
//| modification may not in any way limit the distribution or use
//| of this original code.
//| Modified: MAART: Changed type to Fixed3D to enable use aside from
//|                  the Real type
//|________________________________________________________________


#include "Fixed3D.h"


//  Sine and Cosine tables are in Real.cc
extern unsigned long CosTab[];
extern unsigned long SinTab[];



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure Fixed3DMult
//|
//| Purpose: This procedure computes a fixed point product
//|
//| Parameters: a, b: the multiplier and multiplicand
//|             returns the product
//|
//| Created by Greg Ferrar, February 15, 1995
//| Based on code posted to the comp.graphics.algorithms newsgroup
//|________________________________________________________________

inline Fixed3D Fixed3DMult(register Fixed3D a, register Fixed3D b)
{

   register unsigned int d = (unsigned int) a&65535;
   register unsigned int f = (unsigned int) b&65535;
   register long c = (long) a>>16;
   register long e = (long) b>>16;
 
   return ((long) f*c+d*e+((unsigned long) f*d>>16)+((long) c*e<<16));

}  //==== Fixed3DMult() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure FixedDiv
//|
//| Purpose: This procedure computes a fixed point quotient a/b
//|
//| Parameters: a: the number to divide
//|             b: the number to divide by
//|             returns the quotient a/b
//|
//| Created by Greg Ferrar, February 15, 1995
//|________________________________________________________________

inline Fixed3D Fixed3DDiv(register Fixed3D a, register Fixed3D b)
{

  //  NOTE: this is really a lousy way to do this; it loses 
  //  precision all over the place.  I don't have a better generic
  //  algorithm available, though.  It is DEFINITELY best, when 
  //  porting this library, to create a machine-specific version
  //  of this routine (perhaps using assembly language).

  // Prevent divide-by-zero (something of a hack; it doesn't
  // even preserve the sign...).
  if ((b >> 10) == 0)
    return 0x7FFFFFFF;
  else
    return ((a << 6) / (b >> 10));

}  //==== Fixed3DDiv() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure Fixed3DSin
//|
//| Purpose: This procedure computes the fixed point sine function
//|
//| Parameters: angle: the angle to take the sine of, in radians
//|             returns the sine of angle.
//|
//| Created by Greg Ferrar, February 15, 1995
//|________________________________________________________________

inline Fixed3D Fixed3DSin(register Fixed3D angle)
{

  /* angle is expected to be in the range 0..1024.  402 scales
     the range 0..2*pi to 0..1024. */
  
  return SinTab[(angle/402) & (MAX_TRIG - 1)];

}



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure Fixed3DCos
//|
//| Purpose: This procedure computes the fixed point cosine function
//|
//| Parameters: angle: the angle to take the cosine of, in radians
//|             returns the cosine of angle.
//|
//| Created by Greg Ferrar, February 15, 1995
//|________________________________________________________________

inline Fixed3D Fixed3DCos(register Fixed3D angle)
{

  /* angle is expected to be in the range 0..1024.  402 scales
     the range 0..2*pi to 0..1024. */

  return CosTab[(angle/402) & (MAX_TRIG - 1)];

}  // Fixed3DCos