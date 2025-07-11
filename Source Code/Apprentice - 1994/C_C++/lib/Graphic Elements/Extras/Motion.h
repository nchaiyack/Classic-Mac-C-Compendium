/*
	Motion.h
	
	Simple motion routines for Graphic Elements
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/3/93
*/

/*
	These routines are just some quick hacks to show how "laws of motion" can
	be applied to Graphic Elements.
	
	They should NOT be used in real software!
*/

#ifndef MOTIONPROCS
#define MOTIONPROCS

#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#include "DispCtrl.h"

typedef struct {
	Rect	limitRect;
	Point	currMotion;
	long	friction;	//coeff. of friction, %
	long	frictAcc;	//Actually fixed-point
	long	elasticity; //coeff. of elasticity
} MotionParams, *MParamPtr;

#ifdef __cplusplus
extern "C" {
#endif

//Initialize MotionParams record
void InitMotion(MParamPtr motionRec, short percentFriction, short percentElasticity);

//Returns direction (right, left, up, down only) of collision with limitRect
GEDirection CheckLimits(Rect *spriteRect, Rect *limitRect);

//Modifies motion->currMotion to reflect motion->friction
void DoFriction(MParamPtr motion);

//Modifies motion->currMotion to reflect bounce in direction, taking into account motion->elasticity
void DoBounce(VHSelect direction, MParamPtr motion);

#ifdef __cplusplus
}
#endif

#endif