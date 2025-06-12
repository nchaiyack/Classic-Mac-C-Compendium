/*
	Cannon.c
	
	Cannon scene for GEDemo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/3/93
	
*/

#include "Cannon.h"
#include "Motion.h"

static MotionParams	ballMotion;

Boolean LoadCannonScene(GEWorldPtr world)
{
	GrafElPtr		cannon, thisElement;
	short			smokeWidth, smokeHeight;

	//Get cannon picture
	cannon = NewBasicPICT(world, cannonID, cannonPlane, rCannonPic, 
									transparent, cannonLeft, cannonTop);
	if (cannon == nil) return false;
	
	//Get cannonball picture
	thisElement = NewBasicPICT(world, ballID, ballPlane, rBallPic,
									transparent, cannonLeft - 8, cannonTop - 8);
	if (thisElement == nil) return false;

	//Initialize cannonball's motion
	SetAutoChange(world, ballID, DoCannonBall, (Ptr) &ballMotion, 33);
	
	//Initialize motion parameters
	ballMotion.limitRect = world->animationRect;
	RectOffset(&ballMotion.limitRect, 0, -80);
	InitMotion(&ballMotion, 2, 90);
	
	//Initialize cannonball's collision params
	SetCollision(world, ballID, DoBallHit, 650); //Plane of pogo stick figure
	
	
	//Cannonball is initially invisible
	ShowElement(world, ballID, false);
	
	//Get animated smoke picture
	thisElement = NewAnimatedGraphic(world, smokeID, smokePlane, rSmokePic,
									transparent, 0, 0, 3);
	if (thisElement == nil) return false;
	
	//Position smoke in relation to cannon
	smokeWidth = thisElement->graphRect.right - thisElement->graphRect.left;
	smokeHeight = thisElement->graphRect.bottom - thisElement->graphRect.top;
	MoveElementTo(world, smokeID, cannonLeft - smokeWidth + 16, cannonTop - smokeHeight + 16);
	
	//Set smoke animation
	AnimateGraphic(world, smokeID, 170, oneshot);
	
	//Smoke is initially invisible
	ShowElement(world, smokeID, false);
	
	//Smoke is attached to cannon
	cannon->slaveGrafEl = thisElement;
	
	//Get "FIRE" button
	thisElement = NewButtonSensor(world, fBtnID, btnPlane, rFBtnPic, 434, 193);
	if (thisElement == nil) return false;
	SetSensorAction(world, fBtnID, ShootCannon);
	
	return true;
}

pascal void DoCannonBall(GEWorldPtr world, GrafElPtr ball)
{
	MParamPtr	motion;
	
	motion = (MParamPtr) ball->changeData;
	
	if ((motion->currMotion.v == 0) && (motion->currMotion.h == 0)) {
		ShowElement(world, ball->objectID, false);
		return;
	}
	MoveElement(world, ball->objectID, motion->currMotion.h, motion->currMotion.v);
	switch (CheckLimits(&ball->animationRect, &motion->limitRect)) {
	case down:
		if (motion->currMotion.v > 0) 
			DoBounce(v, motion);
		DoFriction(motion);
		break;
	case right:
		if (motion->currMotion.h > 0) 
			DoBounce(h, motion);
		break;
	case left:
		if (motion->currMotion.h < 0) 
			DoBounce(h, motion);
		break;
	default:
		motion->currMotion.v++;
		break;
	}
}

pascal void DoBallHit(GEWorldPtr world, GrafElPtr ball, GEDirection dir, GrafElPtr objHit)
{
#pragma unused (world, objHit)

	MParamPtr	motion;
	
	motion = (MParamPtr) ball->changeData;
	switch (dir) {
		case left:
			if (motion->currMotion.h < 0)
				DoBounce(h, motion);
			break;
		case right:
			if (motion->currMotion.h > 0)
				DoBounce(h, motion);
			break;
		case up:
			if (motion->currMotion.v < 0)
				DoBounce(v, motion);
			break;
		case down:
			if (motion->currMotion.v > 0)
				DoBounce(v, motion);
			break;
		case upLeft:
			if (motion->currMotion.v < 0)
				DoBounce(v, motion);
			if (motion->currMotion.h < 0)
				DoBounce(h, motion);
			break;
		case downLeft:
			if (motion->currMotion.v > 0)
				DoBounce(v, motion);
			if (motion->currMotion.h < 0)
				DoBounce(h, motion);
			break;
		case upRight:
			if (motion->currMotion.v < 0)
				DoBounce(v, motion);
			if (motion->currMotion.h > 0)
				DoBounce(h, motion);
			break;
		case downRight:
			if (motion->currMotion.v > 0)
				DoBounce(v, motion);
			if (motion->currMotion.h > 0)
				DoBounce(h, motion);
			break;
		
	}
}

pascal void ShootCannon(GEWorldPtr world, short fireIt)
{
	GrafElPtr cannon;
#pragma unused (fireIt)

	cannon = FindElementByID(world, cannonID);
	if (cannon) {
		ballMotion.currMotion.h = -20;
		ballMotion.currMotion.v = -20;
		ballMotion.frictAcc = 0;
		//Move ball into place at mouth of cannon
		MoveElementTo(world, ballID, cannon->animationRect.left - 8, cannon->animationRect.top - 8);
		ShowElement(world, ballID, true);
		//Show smoke to start animation -- it will hide itself automatically
		ShowElement(world, smokeID, true);
	}
}

