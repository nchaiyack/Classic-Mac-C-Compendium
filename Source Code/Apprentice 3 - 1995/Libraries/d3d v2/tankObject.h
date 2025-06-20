// tankObject.h
// Darrell Anderson

#ifndef __tankObject__
#define __tankObject__

#include "d3d_v2.h"

#define NUM_TANK_VERTICES 4

typedef struct TankObj {
	// the tank position and orientation
	d3dPoint pos, ort;
	short color;
} TankObj;

void CreateMasterTank(void);

// draw a tank, given it's frame and our frame.
void DrawTank( TankObj *them, TankObj *us );

#endif