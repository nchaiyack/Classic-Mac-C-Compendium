/*
	Meter.h
	
	A "meter" Graphic Element
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/5/93
*/

//Load precompiled headers if in MPW C
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#include "GraphElements.h"

//Resource numbers of PICTs making up meter

#define rMeterBkg	701
#define rMeterInd	700

//Meter position

#define meterLeft	174
#define meterTop	310
#define indLeft		meterLeft + 2
#define indTop		meterTop + 10
#define indHeight	10

//Meter planes

#define meterPlane	rMeterBkg
#define indPlane	meterPlane - 1

//Meter IDs

#define meterBkgID	'MTRB'
#define meterIndID	'MTRI'

#ifdef __cplusplus
extern "C" {
#endif

Boolean LoadUsageMeterScene(GEWorldPtr world);

pascal void SetMeterReading(GEWorldPtr world, short percent);

#ifdef __cplusplus
}
#endif
