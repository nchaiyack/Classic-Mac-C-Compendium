/*
	SynchGraphic.c
	
	Graphics for Graphic Elements SynchTest
	
	1/29/94
	
	Al Evans
	
*/

#include "SynchGraphic.h"
#include "Motion.h"

short	objUpdateIntrvl;
short	objMoveDist;
Boolean	objLinearMotion;
Boolean objCollision;
short	objSeparation;
short	numObjs;

short RangedRdm(short min, short max)
{
	short rdm;
	
	rdm = Random();
	if (rdm < 0) 
		rdm = -rdm;
	rdm = rdm % (max - min);
	rdm += min;
	return rdm;
}


GrafElPtr MakeAnObject(GEWorldPtr world, OSType id)
{
	GrafElPtr anObject;
	
	anObject = FindElementByID(world, id);
	if (anObject == nil) {
		anObject = NewBasicPICT(world, id, testObjPlane, rTestObj, transparent, 0, 0);
		if (anObject == nil) return nil;
	}
	SetElementPlane(world, anObject->objectID, testObjPlane);
	if (objLinearMotion) {
		SetAutoChange(world, id, DoObjectLinear, nil, objUpdateIntrvl);
		SetCollision(world, id, nil, 0);
	}
	else {
		SetAutoChange(world, id, DoObjectRandom, nil, objUpdateIntrvl);
	}
	
	return anObject;
}

Boolean InitObjectGraphics(GEWorldPtr world)
{
	GrafElPtr		thisElement;
	
	objUpdateIntrvl = 33;
	objMoveDist = 4;
	objSeparation = 4;
	objLinearMotion = true;
	objCollision = false;
	thisElement = MakeAnObject(world, firstObjID);
	if (thisElement == nil) return false;
	ShowElement(world, firstObjID, false);
}

void HideObjects(GEWorldPtr world)
{
	GrafElPtr	thisObj;
	OSType		objID = firstObjID;
	
	while (thisObj = FindElementByID(world, objID)){
		ShowElement(world, objID, false);
		objID++;
	}
}

short GetMoveDistance(void)
{
	return objMoveDist;
}

Boolean MotionIsLinear(void)
{
	return objLinearMotion;
}

void SetObjMotionLinear(Boolean linearMotion)
{
	objLinearMotion = linearMotion;
}

Boolean ObjCollisionActive(void)
{
	return objCollision;
}

void SetObjCollision(GEWorldPtr world, Boolean collide)
{
	OSType		objID = firstObjID;
	GrafElPtr	anObject;

	if (objLinearMotion) return; //no collision possible
	
	objCollision = collide;
	while (anObject = FindElementByID(world, objID)){
		if (objCollision)
			SetCollision(world, objID, DoObjectCollide, testObjPlane);
		else
			SetCollision(world, objID, nil, 0);
		objID++;
	}
}

void SetMoveDistance(GEWorldPtr world, short newMove)
{
	OSType		objID = firstObjID;
	GrafElPtr	anObject;
	short		objMove;
	
	objMoveDist = newMove;
	
	while (anObject = FindElementByID(world, objID)){
		if (objLinearMotion) {
		objMove = (short) ((long) anObject->changeData);
		if (objMove >= 0)
			objMove = newMove;
		else
			objMove = -newMove;
		anObject->changeData = (Ptr) ((long) objMove);
		}
		else {
			anObject->changeData = (Ptr) ((long) RangedRdm(1, 2 * objMoveDist) << 16) + RangedRdm(1, 2 * objMoveDist);
		}
		objID++;
	}
}

short GetUpdateInterval(void)
{
	return objUpdateIntrvl;
}
void SetUpdateInterval(GEWorldPtr world, short newInterval)
{
	OSType		objID = firstObjID;
	GrafElPtr	anObject;
	
	objUpdateIntrvl = newInterval;
	while (anObject = FindElementByID(world, objID)){
		anObject->changeIntrvl = newInterval;
		objID++;
	}
}

short GetNumberOfObjects(void)
{
	return numObjs;
}

void SetNumberOfObjects(GEWorldPtr world, short newNumber)
{
	OSType		objID = firstObjID;
	GrafElPtr	anObject;
	Rect		objRect;
	short		objCount;
	short		hOffset, vOffset;
	
	numObjs = newNumber;
	HideObjects(world);
	anObject = MakeAnObject(world, objID);
	objRect = anObject->animationRect;
	RectOffset(&objRect, -objRect.left + 4, -objRect.top + 16);
	hOffset = objRect.right - objRect.left + objSeparation;
	vOffset = objRect.bottom -objRect.top + objSeparation;
	if (objLinearMotion) {
		for (objCount = 0; objCount < newNumber; objCount++) {
			anObject= MakeAnObject(world, objID);
			if (anObject == nil) return;
			MoveElementTo(world, anObject->objectID, objRect.left, objRect.top);
			anObject->changeData = (Ptr) ((long) objMoveDist);
			ShowElement(world, anObject->objectID, true);
			RectOffset(&objRect, hOffset, 0);
			if (objRect.right > world->animationRect.right) 
				RectOffset(&objRect, -objRect.left + 4, vOffset);
			objID++;
		}
	}
	else {
		for (objCount = 0; objCount < newNumber; objCount++) {
			anObject= MakeAnObject(world, objID);
			if (anObject == nil) return;
			MoveElementTo(world, anObject->objectID, 
				RangedRdm(4, world->animationRect.right - hOffset), 
				RangedRdm(16, world->animationRect.bottom - vOffset));
			anObject->changeData = (Ptr) ((long) RangedRdm(1, 2 * objMoveDist) << 16) + RangedRdm(1, 2 * objMoveDist);
			ShowElement(world, anObject->objectID, true);
			objID++;
		}
	}
	
}

short GetObjSeparation(void)
{
	return objSeparation;
}

void SetObjSeparation(GEWorldPtr world, short newSeparation)
{
#pragma unused(world)
	objSeparation = newSeparation;
}


pascal void DoObjectLinear(GEWorldPtr world, GrafElPtr obj)
{
	GEDirection	collisionDir;
	short currXMove = (short) ((long) obj->changeData);
	short currYMove = 0;
	register Rect *objRect = &obj->animationRect;
	
	collisionDir = CheckLimits(objRect, &world->animationRect);
	switch (collisionDir) {
		case right:
		case left:
			obj->changeData = (Ptr) ((long) -currXMove);
			currXMove = -currXMove;
			currYMove = (objRect->bottom - objRect->top + objSeparation) >> 2;
			if (collisionDir == right) {
				currXMove -= objRect->right - world->animationRect.right;
				SetElementPlane(world, obj->objectID, testObjPlane + 1);
			}
			else {
				currXMove += world->animationRect.left - objRect->left;
				SetElementPlane(world, obj->objectID, testObjPlane);
			}
			break;
		case down:
			PtrMoveElementTo(world, obj, 4, 16);
			return;
	}
	PtrMoveElement(world, obj, currXMove, currYMove);		
}

void BounceObj(GEDirection direction, short *xMove, short *yMove)
{
	switch (direction) {
		case right:
			if (*xMove > 0);
				*xMove = -*xMove;
			break;
		case left:
			if (*xMove < 0);
				*xMove = -*xMove;
			break;
		case up:
			if (*yMove < 0)
				*yMove = -*yMove;
			break;
		case down:
			if (*yMove > 0)
				*yMove = -*yMove;
			break;
	}
}

pascal void DoObjectRandom(GEWorldPtr world, GrafElPtr obj)
{
	GEDirection collisionDir;
	register short *currXMove = (short *) &obj->changeData + 1;
	register short *currYMove = (short *) &obj->changeData;
	
	collisionDir = CheckLimits(&obj->animationRect, &world->animationRect);
	if (collisionDir != none)
		BounceObj(collisionDir, currXMove, currYMove);
	PtrMoveElement(world, obj, *currXMove, *currYMove);
	
}

pascal void DoObjectCollide(GEWorldPtr world, GrafElPtr obj, GEDirection dir, GrafElPtr objHit)
{
#pragma unused (world, objHit)

	register short *currXMove = (short *) &obj->changeData + 1;
	register short *currYMove = (short *) &obj->changeData;
	
	
	switch (dir) {
		case left:
			if (*currXMove < 0)
				*currXMove = -*currXMove;
			break;
		case right:
			if (*currXMove > 0)
				*currXMove = -*currXMove;
			break;
		case up:
			if (*currYMove < 0)
				*currYMove = -*currYMove;
			break;
		case down:
			if (*currYMove > 0)
				*currYMove = -*currYMove;
			break;
		case upLeft:
			if (*currYMove < 0)
				*currYMove = -*currYMove;
			if (*currXMove < 0)
				*currXMove = -*currXMove;
			break;
		case upRight:
			if (*currYMove < 0)
				*currYMove = -*currYMove;
			if (*currXMove > 0)
				*currXMove = -*currXMove;
			break;
		case downLeft:
			if (*currYMove > 0)
				*currYMove = -*currYMove;
			if (*currXMove < 0)
				*currXMove = -*currXMove;
			break;
		
		case downRight:
			if (*currYMove > 0)
				*currYMove = -*currYMove;
			if (*currXMove > 0)
				*currXMove = -*currXMove;
			break;
	}
}
