/*
	Walk.c
	
	Animated walking figure on balcony
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/8/93
	
*/

#include "Walk.h"
#include "Motion.h"

Boolean LoadBalconyScene(GEWorldPtr world)
{
	GrafElPtr		thisElement;
	Rect			balconyBox;			//Walking figure is positioned relative to balcony
	short			elemHeight;
	MParamPtr		walkMotion;
	
	//Get railing of balcony
	thisElement = NewBasicPICT(world, balconyID, balconyPlane, rBalconyPic,
								transparent, balconyLeft, balconyTop);
	if (thisElement == nil) return false;
	balconyBox = thisElement->animationRect;
	
	//Get walking figure
	thisElement = NewAnimatedGraphic(world, walkID, walkPlane, rAnimWalk,
								transparent, 0, 0, 10);
	if (thisElement == nil) return false;

	//Position figure relative to balcony
	elemHeight = thisElement->graphRect.bottom - thisElement->graphRect.top;
	MoveElementTo(world, walkID, balconyBox.left + 20, balconyBox.bottom - 10 - elemHeight);
	
	//Initialize motion fields -- never collide with top or bottom
	walkMotion = (MParamPtr) NewPtrClear(sizeof(MotionParams));
	InitMotion(walkMotion, 100, 100);
	walkMotion->currMotion.h = 6;
	walkMotion->limitRect.top  = 0;
	walkMotion->limitRect.left = balconyBox.left + 5;
	walkMotion->limitRect.bottom = 1000;
	walkMotion->limitRect.right = balconyBox.right - 16;
	
	//Initialize figure's walking action
	SetAutoChange(world, walkID, DoWalker, (Ptr) walkMotion, 133);
	
	//And set animation style to "loop"
	((SeqGraphicPtr) thisElement)->seq = loop;
	
	//Load speed control slider
	thisElement = NewSliderSensor(world, sliderID, sliderPlane, rSliderBkg, 
					sliderLeft, sliderTop, hSlideSensor, rSliderCtrl);
	if (thisElement == nil) return false;
	SetSliderPercent(world, sliderID, 50);
	SetSensorAction(world, sliderID, AdjustSpeed);
	
	return true;
}


pascal void DoWalker(GEWorldPtr world, GrafElPtr walker)
{
	MParamPtr	motion;
	GEDirection	collisionDir;
	
	motion = (MParamPtr) walker->changeData;
	collisionDir = CheckLimits(&walker->animationRect, &motion->limitRect);
	if ((collisionDir == left) || (collisionDir == right)) {
		motion->currMotion.h = -motion->currMotion.h;
		SetMirroring(world, walker->objectID, (collisionDir == right), false);
	}
	MoveElement(world, walker->objectID, motion->currMotion.h, motion->currMotion.v);
	BumpFrame(world, walker->objectID);
}

pascal void AdjustSpeed(GEWorldPtr world, short newSpeed)
{
	GrafElPtr walker;
	long newIntrvl;
	
	walker = FindElementByID(world, walkID);
	if (walker) {
		newIntrvl = (newSpeed * 240) / 100;
		if (newIntrvl > 0) {
			newIntrvl = 240 - newIntrvl;
			if (newIntrvl < 16)
				newIntrvl = 16;
		}
		walker->changeIntrvl = newIntrvl;
	}
}
