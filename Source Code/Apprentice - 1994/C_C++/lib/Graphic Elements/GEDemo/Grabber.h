/*
	Grabber.h
	
	Special Graphic Element to manipulate other Graphic Elements
	Written to demonstrate the versatility of the Graphic Elements system
	and the ease of creating new types of Graphic Elements
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/29/93
	
	NOTICE:
	
	Because it acts as a "meta-element", this grabber utilizes knowledge of the
	low-level implementation details of graphic elements and sensors. 
	
*/

//Include precompiled headers if we're in MPW
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#else
#include <QDOffscreen.h>
#endif

#include "GraphElements.h"

#define	grabberID 'GRBR'
#define grabberPlane	32767		//The grabber wants to be in front!!

typedef struct {
	GrafElement	baseGraphic;
	short		slaveChgIntrvl;		//For restoration upon deselection
	short		minPlane;			//Only grab elements with drawPlane >= minPlane
	short		maxPlane;			//and <= maxPlane
}	Grabber, *GrabberPtr;

#ifdef __cplusplus
extern "C" {
#endif

/*
	Create the grabber
*/
GrafElPtr MakeGrabber(GEWorldPtr world);

//Activate or deactivate grabber
void ActivateGrabber(GEWorldPtr world, Boolean activate);

//Grabber will pick up elements with planes >= minPlane and <= maxPlane
void SetGrabberDepth(GEWorldPtr world, short newMinPlane, short newMaxPlane);

//Returns true if grabber is currently active
Boolean GrabberActive(GEWorldPtr world);

//Returns pointer to element currently selected by grabber
GrafElPtr  CurrentGrabberSelection(GEWorldPtr world);

#ifdef __cplusplus
}
#endif
