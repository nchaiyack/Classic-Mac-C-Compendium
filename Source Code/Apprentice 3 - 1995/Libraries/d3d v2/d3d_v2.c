// d3d_v2.c
// Darrell Anderson

#include "d3d_v2.h"
#include <math.h>

void d3dMakeIdentity( d3dMatrix *X ) {
	short i,j;

	for( i=0 ; i<4 ; i++ ) {
		for( j=0 ; j<4 ; j++ ) {
			X->M[i][j] = 0.0;
		}
		X->M[i][i] = 1.0;
	}
}

void d3dGetTranslation( d3dMatrix *X, double x, double y, double z ) {
	d3dMakeIdentity(X);
	X->M[0][3] = x;
	X->M[1][3] = y;
	X->M[2][3] = z;
}

// el, az, rl must be in radians (= degrees * PI / 180.0)
void d3dGetRotation( d3dMatrix *X, double el, double az, double rl ) {
	double cos_x, cos_y, cos_z, sin_x, sin_y, sin_z;
	
	cos_x = cos(el);
	cos_y = cos(az);
	cos_z = cos(rl);
	sin_x = sin(el);
	sin_y = sin(az);
	sin_z = sin(rl);
	
	d3dMakeIdentity(X);
	
	X->M[0][0] =  cos_y * cos_z;							// 0 // coodinates in the 
	X->M[0][1] = -cos_x * sin_z + sin_x * sin_y * cos_z;	// 3 // flock transformation
	X->M[0][2] =  sin_x * sin_z + cos_x * sin_y * cos_z;	// 6 // matrix, transposed
	X->M[1][0] =  cos_y * sin_z;							// 1 // b/c we want col vectors.
	X->M[1][1] =  cos_x * cos_z + sin_x * sin_y * sin_z;	// 4
	X->M[1][2] = -sin_x * cos_z + cos_x * sin_y * sin_z;	// 7
	X->M[2][0] = -sin_y;									// 2
	X->M[2][1] =  sin_x * cos_y;							// 5
	X->M[2][2] =  cos_x * cos_y;							// 8
}

// math operations.  Answer gets stuck in first param.

void d3dMultMatrixMatrix( d3dMatrix *AxB, d3dMatrix *A, d3dMatrix *B ) {
	short i,j,k;
	double temp_real;

	for( i=0 ; i<4 ; i++ ) {
		for( j=0 ; j<4 ; j++ ) {
			temp_real = 0;
			for( k=0 ; k<4 ; k++ ) {
				temp_real += A->M[i][k] * B->M[k][j];
			}
			AxB->M[i][j] = temp_real;
		}
	}
}

void d3dMultMatrixPoint( d3dPoint *AxP, d3dMatrix *A, d3dPoint *P ) {
	AxP->x = (A->M[0][0]*P->x) + (A->M[0][1]*P->y) + (A->M[0][2]*P->z) + A->M[0][3];
	AxP->y = (A->M[1][0]*P->x) + (A->M[1][1]*P->y) + (A->M[1][2]*P->z) + A->M[1][3];
	AxP->z = (A->M[2][0]*P->x) + (A->M[2][1]*P->y) + (A->M[2][2]*P->z) + A->M[2][3];
}

void d3dGetInverse( d3dMatrix *XI, d3dMatrix *X ) {
	double det, adj;
	short i, i1, i2, j, j1, j2;
	
	// calculate the determinant
	det = d3dDeterminant(X);
	
	// make sure it's not a singular matrix! (ie, det != 0)
	if( det < EPSILON ){
		// panic!  well, just make it the identity for now..
		d3dMakeIdentity(X);
	} else {
		// otherwise take the adjoint matrix / determinant
		for( i=0 ; i<4 ; i++ ) {
			i1 = (i%3)+1;
			i2 = (i1%3)+1;
			for( j=0 ; j<4 ; j++ ) {
				j1 = (j%3)+1;
				j2 = (j1%3)+1;
				adj = X->M[i1][j1] * X->M[i2][j2] - X->M[i1][j2] * X->M[i2][j1];
				XI->M[j][i] = adj/det;
			}
		}
	}
}

double d3dDeterminant( d3dMatrix *X ) {
	return(	X->M[0][0] * (X->M[1][1] * X->M[2][2] - X->M[1][2] * X->M[2][1]) +
			X->M[0][1] * (X->M[1][2] * X->M[2][0] - X->M[1][0] * X->M[2][2]) +
			X->M[0][2] * (X->M[1][0] * X->M[2][1] - X->M[1][1] * X->M[2][0]) );
}

// where would a point be if observed from a different reference frame?
void d3dObserve( d3dPoint *Pfinal, d3dPoint *P, d3dPoint *pos, d3dPoint *ort ) {
	d3dMatrix Q, posI, ortI; //, temp;
	
	// Pfinal = (ortI)(posI) P

	d3dGetTranslation( &posI, -pos->x, -pos->y, -pos->z );
	d3dGetRotation( &ortI, -ort->x, -ort->y, -ort->z );
	
	d3dMultMatrixMatrix( &Q, &ortI, &posI );
	d3dMultMatrixPoint( Pfinal, &Q, P );
}

void d3dObserveMany( d3dPoint *Pfinal, d3dPoint *P, d3dPoint *pos, d3dPoint *ort, short howMany ) {
	d3dMatrix Q, posI, ortI; 
	short i;
	
	// Pfinal = (ortI)(posI) P

	d3dGetTranslation( &posI, -pos->x, -pos->y, -pos->z );
	d3dGetRotation( &ortI, -ort->x, -ort->y, -ort->z );
	
	d3dMultMatrixMatrix( &Q, &ortI, &posI );
	
	for( i=0 ; i<howMany ; i++ ) 
		d3dMultMatrixPoint( &(Pfinal[i]), &Q, &(P[i]) );
}

// drawing routines, map 3d -> 2d, with perspective

void d3dMoveTo( d3dPoint *P ) {
	// protect against divide by zero errors
	if( (P->z < EPSILON) && (P->z > -EPSILON) ) 
		P->z = EPSILON;
		 
	MoveTo( 
		(P->x / P->z * SCALE) + (qd.thePort->portRect.bottom / 2), 
		(qd.thePort->portRect.bottom / 2) - (P->y / P->z * SCALE) );
}

void d3dLineTo( d3dPoint *P ) {
	// protect against divide by zero errors
	if( (P->z < EPSILON) && (P->z > -EPSILON) ) 
		P->z = EPSILON;
		 
	LineTo( 
		(P->x / P->z * SCALE) + (qd.thePort->portRect.bottom / 2), 
		(qd.thePort->portRect.bottom / 2) - (P->y / P->z * SCALE) );
}