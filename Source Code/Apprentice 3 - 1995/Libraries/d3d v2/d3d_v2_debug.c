// d3d_v2_debug.c
// Darrell Anderson

#include "d3d_v2_debug.h"
#include <stdio.h>

void PrintMatrix( d3dMatrix *M ) {
	short i,j;
	
	for( i=0 ; i<4 ; i++ ) {
		for( j=0 ; j<4 ; j++ ) {
			printf("%f ",(float)M->M[i][j]);
		} 
		printf("\n");
	}
}

void PrintPoint( d3dPoint *P ) {
	printf( "(%f, %f, %f)\n", (float)P->x, (float)P->y, (float)P->z );
}
