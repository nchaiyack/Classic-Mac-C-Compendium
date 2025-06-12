/*
	BGame.h
	
	Game running code for Breakout Demo
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	2/24/94
	
*/

/*
	A very simple game written with Graphic Elements.
*/


//To load precompiled symbol table under MPW C
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#include "GraphElements.h"

//Resource numbers of game element PICTs

#define rPaddle		200
#define rBall		300
#define rBkg		400
#define rBrick		500

//IDs of game elements

#define paddleID		'PADL'
#define ballID			'BALL'
#define bkgID			'BKGD'
#define firstBrickID	'BRK\0'
#define scoreID			'SCOR'

//Planes of game elements

#define paddlePlane		200
#define ballPlane		300
#define brickPlane		200
#define bkgPlane		1
#define scorePlane		900

//Game window size
#define gWindWidth		342
#define gWindHeight		370

#ifdef __cplusplus
extern "C" {
#endif

//Setup
Boolean LoadBreakoutGame(GEWorldPtr world);

//Start a game
void NewBreakoutGame(GEWorldPtr world);

//Autochange procs

pascal void DoPaddle(GEWorldPtr world, GrafElPtr paddle);
pascal void DoBall(GEWorldPtr world, GrafElPtr ball);
pascal void DoScore(GEWorldPtr world, GrafElPtr scoreBoard);

//Ball's collision proc
pascal void DoBallHit(GEWorldPtr world, GrafElPtr ball, GEDirection dir, GrafElPtr objHit);



#ifdef __cplusplus
}
#endif
