/*
	Motion.c
	
	Motion routines for Graphic Elements
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/3/93
*/


#include "Motion.h"

void InitMotion(MParamPtr motionRec, short percentFriction, short percentElasticity)
{
	motionRec->currMotion.v = motionRec->currMotion.h = 0;
	motionRec->friction = (((long) percentFriction) << 16) / 100;
	motionRec->frictAcc = 0;
	motionRec->elasticity = (((long) percentElasticity) << 16) / 100;
}

GEDirection CheckLimits(Rect *objRect, Rect *limitRect)
{
	if (objRect->right >= limitRect->right)
		return right;
	if (objRect->left <= limitRect->left)
		return left;
	if (objRect->top <= limitRect->top)
		return up;
	if (objRect->bottom >= limitRect->bottom)
		return down;
	return none;
}

void DoFriction(MParamPtr motion)
{
	short temp;
	if (motion->currMotion.h) {
		temp = motion->currMotion.h;
		if (temp < 0)
			temp = -temp;
		motion->frictAcc += temp * motion->friction;
		temp = (motion->frictAcc >> 16);						//integer part
		motion->frictAcc -= (motion->frictAcc & 0xFFFF0000);	//keep fractional part
		if (motion->currMotion.h > 0)
			motion->currMotion.h -= temp;
		else
			motion->currMotion.h += temp;
	}
}


void DoBounce(VHSelect direction, MParamPtr motion)
{
	long temp, compare;
	
	if (direction == v)
		temp = motion->currMotion.v;
	else
		temp = motion->currMotion.h;
	temp = -temp;
	compare = temp;
	temp = (temp * motion->elasticity) >> 16;
	if (compare == temp) {
		if (temp > 0)
			temp -= 1;
		else
			temp += 1;
	}
	if (direction == v)
		motion->currMotion.v = temp;
	else
		motion->currMotion.h = temp;
}


