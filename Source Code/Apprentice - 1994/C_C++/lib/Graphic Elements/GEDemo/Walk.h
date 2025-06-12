/*
	Walk.h
	
	Animated walking figure on balcony
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/8/93
	
*/

//Load precompiled symbols if in MPW C
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

//Resource numbers of PICTs making up balcony scene
	
#define rBalconyPic		220
#define rAnimWalk		501
#define	rSliderBkg		750
#define rSliderCtrl		751

//Position of balcony
#define balconyLeft		94
#define balconyTop		131

//Position of speed control
#define sliderLeft		48
#define sliderTop		54

//Balcony scene planes
#define walkPlane		3
#define balconyPlane	13
#define sliderPlane		4

//Balcony scene IDs

#define walkID			'CEA '
#define balconyID		'BALC'
#define sliderID		'SCTL'

#ifdef __cplusplus
extern "C" {
#endif

Boolean LoadBalconyScene(GEWorldPtr world);

//Autochange proc for walk animation
pascal void DoWalker(GEWorldPtr world, GrafElPtr walker);

//Callback proc for speed control slider
pascal void AdjustSpeed(GEWorldPtr world, short newSpeed);

#ifdef __cplusplus
}
#endif
