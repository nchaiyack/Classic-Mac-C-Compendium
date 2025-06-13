//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Fixed3D.h
//|
//| This file contains declarations for the fixed point routines. 
//|
//| Created: Maarten Gribnau
//|
//|________________________________________________________________

#ifndef _Fixed3D_h_
#define _Fixed3D_h_


#include <limits.h>

typedef long Fixed3D;

#define MAX_TRIG 1024

const FIXED_POINT = 16;
const HALF_FIXED_POINT = FIXED_POINT / 2;

const Fixed3D FIXED_ONE = (1 << FIXED_POINT);

#define FIXED_SMALLNUM	100
#define FIXED_MAX		LONG_MAX
#define FIXED_MIN		LONG_MIN

// From file Fixed3D.hpp
inline Fixed3D Fixed3DMult(register Fixed3D a, register Fixed3D b);
inline Fixed3D Fixed3DDiv(register Fixed3D a, register Fixed3D b);
inline Fixed3D Fixed3DSin(register Fixed3D angle);
inline Fixed3D Fixed3DCos(register Fixed3D angle);

// From file Fixed3D.cpp
Fixed3D Fixed3DSqrt(register Fixed3D x);

#endif  // _Fixed3D_h_
