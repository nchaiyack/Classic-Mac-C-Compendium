/*
	Pogo.c
	
	Boy on pogo stick for Graphic Elements demo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/10/93
*/

#include "Pogo.h"
#include "Paths.h"


//The bouncing and animation are handled as an interpreted "Path"
	
static PathEntry	pogoPathRec[11] =	{
					  					 {absMotionCmd, 3, 0, -22},
										 {repeatCmd, 10, 0, 0},
										 {relMotionCmd, 0, 0, 2},
										 {relMotionCmd, 4, 0, 0},
										 {repeatCmd, 10, 0, 0},
										 {relMotionCmd, 0, 0, 2},
										 {absMotionCmd, 5, 0, 0},
										 {absMotionCmd, 6, 0, 0},
										 {absMotionCmd, 1, 0, 0},
										 {absMotionCmd, 2, 0, 0},
										 {resetCmd, 0, 0, 0}
										 };

Boolean LoadPogoScene(GEWorldPtr world)
{
	GrafElPtr		thisElement;
	PathRecPtr		pogoPath;

	//Get walking figure
	thisElement = NewAnimatedGraphic(world, pogoID, pogoPlane, rPogoPic,
								transparent, pogoLeft, pogoTop, 6);
	if (thisElement == nil) return false;
									
	//Init path
	pogoPath = (PathRecPtr) NewPtrClear(sizeof(PathRec));
	InitPath(pogoPath);
	pogoPath->path = pogoPathRec;
	
	//Init animation
	SetAutoChange(world, pogoID, DoPogoStick, (Ptr) pogoPath, 67);
	
	thisElement->collisionPlane = 600;
	
	return true;
}

pascal void DoPogoStick(GEWorldPtr world, GrafElPtr pogo)
{
	PathRecPtr pogoPath;
	
	pogoPath = (PathRecPtr) pogo->changeData;
	GetNextStep(pogoPath);
	if (pogoPath->currFrame)
		SetFrame(world, pogo->objectID, pogoPath->currFrame);
	if (pogoPath->currYMove)
		MoveElement(world, pogo->objectID, 0, pogoPath->currYMove);
	
}