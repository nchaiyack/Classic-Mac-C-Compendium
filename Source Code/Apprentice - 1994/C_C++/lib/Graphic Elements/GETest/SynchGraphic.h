/*
	SynchGraphic.h
	
	Graphics for Graphic Elements SynchTest
	
	1/29/94
	
	Al Evans
	
*/

#ifndef GRAPHELEMENTS
#include "GraphElements.h"
#endif

#define	rTestObj		401
#define testObjPlane	400
#define firstObjID		'TST\0'

#ifdef __cplusplus
extern "C" {
#endif

Boolean InitObjectGraphics(GEWorldPtr world);

short GetMoveDistance(void);
void SetMoveDistance(GEWorldPtr world, short newMove);

short GetUpdateInterval(void);
void SetUpdateInterval(GEWorldPtr world, short newInterval);

short GetNumberOfObjects(void);
void SetNumberOfObjects(GEWorldPtr world, short newNumber);

short GetObjSeparation(void);
void SetObjSeparation(GEWorldPtr world, short newSeparation);

Boolean ObjMotionIsLinear(void);
void SetObjMotionLinear(Boolean linearMotion);

Boolean ObjCollisionActive(void);
void SetObjCollision(GEWorldPtr world, Boolean collide);

//Object's autochange procs
pascal void DoObjectLinear(GEWorldPtr world, GrafElPtr obj);
pascal void DoObjectRandom(GEWorldPtr world, GrafElPtr obj);

//Object's collision proc
pascal void DoObjectCollide(GEWorldPtr world, GrafElPtr obj, GEDirection dir, GrafElPtr objHit);
#ifdef __cplusplus
}
#endif
