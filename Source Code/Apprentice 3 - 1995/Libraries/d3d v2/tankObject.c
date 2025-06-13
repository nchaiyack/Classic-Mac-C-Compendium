// tankObject.c
// Darrell Anderson

#include "tankObject.h"
#include "d3d_v2.h"

d3dPoint masterTank[4];

void CreateMasterTank(void) {
	masterTank[0].x = 0;	
	masterTank[0].y = 0;	
	masterTank[0].z = 1;
	
	masterTank[1].x = 1;	
	masterTank[1].y = 0;	
	masterTank[1].z = -1;
	
	masterTank[2].x = 0;	
	masterTank[2].y = 1;	
	masterTank[2].z = -2;

	masterTank[3].x = -1;	
	masterTank[3].y = 0;	
	masterTank[3].z = -1;
}

void DrawTank( TankObj *them, TankObj *us ) {
	d3dPoint tank1[4], tank2[4];
	short i;
	d3dMatrix ortM, posM, Q;
	Boolean behind = false;
		
	// put them in universe frame
	d3dGetRotation( &ortM, them->ort.x, them->ort.y, them->ort.z );
	d3dGetTranslation( &posM, them->pos.x, them->pos.y, them->pos.z );
	d3dMultMatrixMatrix( &Q, &posM, &ortM );
	for( i=0 ; i<4 ; i++ )
		d3dMultMatrixPoint(&(tank1[i]), &Q, &(masterTank[i]));

	// transform to our frame
	d3dGetRotation( &ortM, -us->ort.x, -us->ort.y, -us->ort.z );
	d3dGetTranslation( &posM, -us->pos.x, -us->pos.y, -us->pos.z );
	d3dMultMatrixMatrix( &Q, &ortM, &posM );
	for( i=0 ; i<4 ; i++ )
		d3dMultMatrixPoint(&(tank2[i]), &Q, &(tank1[i]));

	// tank2 if now in our frame	
	
	// figure out if it's behind us (any vertex)
	for( i=0 ; i<4 ; i++ ) {
		if( tank2[i].z <= 0 )
			behind = true;
	}
	
	// only draw it if it's not behind us, then continue
	if( !behind ) {
		ForeColor(them->color);
		d3dMoveTo(&(tank2[0]));
		d3dLineTo(&(tank2[1]));
		d3dLineTo(&(tank2[2]));
		d3dLineTo(&(tank2[3]));
		d3dLineTo(&(tank2[0]));
		d3dLineTo(&(tank2[2]));
	}
}