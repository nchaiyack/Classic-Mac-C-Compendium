// d3d_v2.h
// Darrell Anderson

#ifndef __d3d_v2__
#define __d3d_v2__

#define SCALE 100.0

typedef struct d3dPoint {
	double x,y,z; // or el, az, rl
} d3dPoint;

typedef struct d3dMatrix {
	double M[4][4];
	// 00,01,02,03
	// 10,11,12,13
	// 20,21,22,23
	// 30,31,32,33
} d3dMatrix;

// a number darn close to zero (save us from precision/rounding error)
#define EPSILON 0.00001

// pi
#define PI 3.1415926536

// make the identity matrix
void d3dMakeIdentity( d3dMatrix *X );

// build standard transformation matrices.  Answer gets stuck in first param.

void d3dGetTranslation( d3dMatrix *X, double x, double y, double z );
// el, az, rl must be in radians (= degrees * PI / 180.0)
void d3dGetRotation( d3dMatrix *X, double el, double az, double rl );

// math operations.  Answer gets stuck in first param.

void d3dMultMatrixMatrix( d3dMatrix *AxB, d3dMatrix *A, d3dMatrix *B );
void d3dMultMatrixPoint( d3dPoint *AxP, d3dMatrix *A, d3dPoint *P );
void d3dGetInverse( d3dMatrix *XI, d3dMatrix *X ); 
double d3dDeterminant( d3dMatrix *X );

// where would a point be if observed from a different reference frame?
void d3dObserve( d3dPoint *Pfinal, d3dPoint *P, d3dPoint *pos, d3dPoint *ort );

void d3dObserveMany( d3dPoint *Pfinal, d3dPoint *P, d3dPoint *pos, d3dPoint *ort, short howMany );

// drawing routines, map 3d -> 2d, with perspective
void d3dMoveTo( d3dPoint *P );
void d3dLineTo( d3dPoint *P );

#endif

