/*************************************************************
*                                                            *
* File:  complex.h -- contains routines for complex numbers. *
*                                                            *
*************************************************************/

#ifndef COMPLEX_H
#define COMPLEX_H
/** all header stuff goes after this line */

#include "mathtype.h"

typedef struct {
    MathType i,          /* imaginary part */
          r,             /* real part */
          mag;           /* magnitude of complex number */
     } ImagPt;

extern void    AddC(ImagPt *a, ImagPt *b, ImagPt *result);
extern void    SubC(ImagPt *a, ImagPt *b, ImagPt *result);
extern void   MultC(ImagPt *a, ImagPt *b, ImagPt *result);
extern void SquareC(ImagPt *a, ImagPt *result);
extern void   CubeC(ImagPt *a, ImagPt *result);

/* all header stuff goes above this line */
#endif