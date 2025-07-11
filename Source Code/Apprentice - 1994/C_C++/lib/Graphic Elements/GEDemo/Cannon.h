/*
	Cannon.h
	
	Cannon scene for GEDemo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/3/93
	
*/

//Load precomiled headers if in MPW
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#include "GraphElements.h"
#include "Sensors.h"

//Resource numbers of PICTs making up cannon scene
#define rBallPic	600
#define rCannonPic	601
#define rSmokePic	602
#define rFBtnPic	650

//Cannon position
#define cannonLeft	390
#define cannonTop	210

//Cannon scene planes
#define cannonPlane	rCannonPic
#define ballPlane	cannonPlane - 1
#define smokePlane	cannonPlane + 1
//Button is just in front of background
#define btnPlane	4

//Cannon scene IDs
#define cannonID	'CANN'
#define ballID 		'BALL'
#define smokeID		'SMOK'
#define fBtnID		'FBtn'

#ifdef __cplusplus
extern "C" {
#endif

Boolean LoadCannonScene(GEWorldPtr world);

//Cannonball autochange proc
pascal void DoCannonBall(GEWorldPtr world, GrafElPtr ball);

//Cannonball collision proc
pascal void DoBallHit(GEWorldPtr world, GrafElPtr ball, GEDirection dir, GrafElPtr objHit);

//Call to start cannon scene animation
pascal void ShootCannon(GEWorldPtr world, short fireIt);

#ifdef __cplusplus
}
#endif
