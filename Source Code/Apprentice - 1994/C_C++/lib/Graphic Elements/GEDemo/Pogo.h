/*
	Pogo.h
	
	Boy on pogo stick for Graphic Elements demo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/10/93
*/

//Load precompiled symbols if in MPW
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#include "GraphElements.h"

//Starting resource number of PICT sequence

#define rPogoPic	140

//Starting position of graphic

#define pogoLeft	98
#define pogoTop		218

//Plane of graphic

#define pogoPlane	650

//Graphic ID

#define pogoID		'POGO'

#ifdef __cplusplus
extern "C" {
#endif

Boolean LoadPogoScene(GEWorldPtr world);

//Autochange proc for pogo stick animation
pascal void DoPogoStick(GEWorldPtr world, GrafElPtr pogo);

#ifdef __cplusplus
}
#endif
