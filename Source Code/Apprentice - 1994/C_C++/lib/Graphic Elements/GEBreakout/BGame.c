/*
	BGame.c
	
	Game running code for Breakout Demo
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	2/24/94
	
*/

#include "BGame.h"
#include "Motion.h"

short	gBOBrickCount;
short	gBOBallCount;
short	gBOBallSpeed;
long	gBOScore;
Str255	gBOScoreStr;

GrafElPtr MakeABrick(GEWorldPtr world, OSType id)
{
	GrafElPtr	aBrick;
	
	aBrick = FindElementByID(world, id);
	if (aBrick == nil) {
		aBrick = NewBasicPICT(world, id, brickPlane, rBrick, transparent, 0, 0);
	}
	return aBrick;
}

//Setup
Boolean LoadBreakoutGame(GEWorldPtr world)
{
	GrafElPtr	thisElement;
	MParamPtr 	ballMotion;
	RGBColor 	scoreColor;
	short		scoreFNum;
	
	//Get background
	thisElement = NewTiledGraphic(world, bkgID, bkgPlane, rBkg, srcCopy, world->animationRect);
	if (thisElement == nil) return false;
	
	//Get paddle
	thisElement = NewBasicPICT(world, paddleID, paddlePlane, rPaddle, transparent, 0, world->animationRect.bottom - 16);
	if (thisElement == nil) return false;
	//Set paddle's autochange proc
	SetAutoChange(world, paddleID, DoPaddle, nil, 33);
	
	//Get ball
	thisElement = NewBasicPICT(world, ballID, ballPlane, rBall, transparent, 0, 0);
	if (thisElement == nil) return false;
	//Hide it for now
	ShowElement(world, ballID, false);
	//Set ball's autochange and collision procs
	ballMotion = (MParamPtr) NewPtrClear(sizeof(MotionParams));
	InitMotion(ballMotion, 0, 125);
	ballMotion->limitRect = world->animationRect;
	SetAutoChange(world, ballID, DoBall, (Ptr) ballMotion, 17);
	SetCollision(world, ballID, DoBallHit, brickPlane);  //brickPlane == paddlePlane
	
	//Get a brick just to make sure it's available
	thisElement = MakeABrick(world, firstBrickID);
	if (thisElement == nil) return false;
	//Hide it
	ShowElement(world, firstBrickID, false);
	
	//Make the scoreboard
	scoreColor.red = 240 << 8;
	scoreColor.green = 240 << 8;
	scoreColor.blue = 46 << 8;
	GetFNum("\pChicago", &scoreFNum);
	thisElement = NewTextGraphic(world, scoreID, scorePlane, 20, 10, srcOr, 
						scoreFNum, bold, 12, scoreColor, gBOScoreStr);
	if (thisElement == nil) return false;
	SetAutoChange(world, scoreID, DoScore, nil, 200);  //Update score 5 times/second
	
	return true;
}

void MakeNewBricks(GEWorldPtr world)
{
	GrafElPtr	thisBrick;
	OSType		currBrickID = firstBrickID;
	short		rowCount, colCount;
	
	gBOBrickCount = 0;
	for (rowCount = 0; rowCount < 5; rowCount++) 
		for (colCount = 0; colCount < 10; colCount++) {
		thisBrick = MakeABrick(world, currBrickID);
		if (thisBrick == nil) return; 				//Oops
		MoveElementTo(world, currBrickID, 2 + colCount * 34, 36 + rowCount * 18);
		ShowElement(world, currBrickID, true);
		currBrickID++;
		gBOBrickCount++;
	}
}

void StartNewBall(GEWorldPtr world)
{
	GrafElPtr	ball;
	MParamPtr	ballMotion;
	
	ball = FindElementByID(world, ballID);
	if (ball == nil) return;						//Oops again
	MoveElementTo(world, ballID, 0, 200);
	ShowElement(world, ballID, true);
	ballMotion = (MParamPtr) ball->changeData;
	ballMotion->currMotion.v = gBOBallSpeed;
	ballMotion->currMotion.h = gBOBallSpeed - 2;
}

void NewBreakoutGame(GEWorldPtr world)
{
	
	gBOScore = 0;
	gBOScoreStr[0] = 0;
	gBOBallCount = 4;
	gBOBallSpeed = 4;
	MakeNewBricks(world);
	StartNewBall(world);
}

//Return position of mouse in GEWorld coordinates
short	GetPlayerMove(GEWorldPtr world)
{
	GrafPtr		savePort;
	Point		saveFocus;
	Point		mousePt;
	short		move;


	GetPort(&savePort);
	SetPort((GrafPtr) world->gEWWindow);
	GetGEWorldFocus(world, &saveFocus);
	FocusOnGEWorld(world);

	GetMouse(&mousePt);
	move = mousePt.h;
	SetGEWorldFocus(world, saveFocus);
	SetPort(savePort);
	return move;
}

pascal void DoPaddle(GEWorldPtr world, GrafElPtr paddle)
{
	short paddleX;
	short limit = world->animationRect.right - (paddle->animationRect.right - paddle->animationRect.left);
	
	paddleX = GetPlayerMove(world);
	if (paddleX < 0) paddleX = 0;
	if (paddleX > limit) paddleX = limit;
	PtrMoveElementTo(world, paddle, paddleX, paddle->animationRect.top);
}


pascal void DoBall(GEWorldPtr world, GrafElPtr ball)
{
	MParamPtr	motion = (MParamPtr) ball->changeData;
	
		switch (CheckLimits(&ball->animationRect, &motion->limitRect)) {
			case up:
				if (motion->currMotion.v < 0)
					motion->currMotion.v = -motion->currMotion.v;
				break;
			case left:
				if (motion->currMotion.h < 0)
					motion->currMotion.h = -motion->currMotion.h;
				break;
			case down:
				ShowElement(world, ball->objectID, false);
				//Do something gamey here
				if (--gBOBallCount > 0)
					StartNewBall(world);
				break;
			case right:
				if (motion->currMotion.h > 0)
					motion->currMotion.h = -motion->currMotion.h;
				break;
		}
		
		PtrMoveElement(world, ball, motion->currMotion.h, motion->currMotion.v);

}

pascal void DoBallHit(GEWorldPtr world, GrafElPtr ball, GEDirection dir, GrafElPtr objHit)
{
	MParamPtr	motion = (MParamPtr) ball->changeData;
	short 		hBump;
	
	if (objHit->objectID == paddleID) {			//Ball hit the paddle
		hBump = ball->animationRect.left - objHit->animationRect.left + 6;
		hBump = hBump - 20;
		hBump = hBump / 4;
		motion->currMotion.h += hBump;
	}
	else { 										//Ball hit a brick
		ShowElement(world, objHit->objectID, false);
		gBOScore += 50;
		--gBOBrickCount;
		if (gBOBrickCount == 0) {
			gBOBallSpeed++;
			MakeNewBricks(world);
			StartNewBall(world);
		}
	}	
	//Either way, bounce ball
	switch (dir) {
		case left:
		case upLeft:
		case downLeft:
			if (motion->currMotion.h < 0)
				motion->currMotion.h = -motion->currMotion.h;
			if (dir != left)
				motion->currMotion.v = -motion->currMotion.v;
			break;
		case right:
		case upRight:
		case downRight:
			if (motion->currMotion.h > 0)
				motion->currMotion.h = -motion->currMotion.h;
			if (dir != right)
				motion->currMotion.v = -motion->currMotion.v;
			break;
		case up:
			if (motion->currMotion.v < 0)
				motion->currMotion.v = -motion->currMotion.v;
			break;
		case down:
			if (motion->currMotion.v > 0)
				motion->currMotion.v = -motion->currMotion.v;
			break;
	}
			

}

pascal void DoScore(GEWorldPtr world, GrafElPtr scoreBoard)
{
	static long scoreShown = 0;
	
	if (gBOScore != scoreShown) {
		scoreShown = gBOScore;
		NumToString(scoreShown, gBOScoreStr);
		SetTextGraphicText(world, scoreBoard->objectID, gBOScoreStr);
	}
}
