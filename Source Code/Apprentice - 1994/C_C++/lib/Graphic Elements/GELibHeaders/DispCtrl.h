/*
	DispCtrl.h
	
	Display Controller routines for Graphic Elements
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	Release version 1.0b1
	
	3/7/94

*/

#ifndef DISPCTRL
#define DISPCTRL

#include "Defs.h"


#ifdef __cplusplus
extern "C" {
#endif


//Creation of new GEWorld

/*
	Returns pointer to a new GEWorld. This world covers the area of worldWindow
	specified by animationRect (in window coordinates).  If it is not nil, worldColors
	must be a handle to a color table appropriate for an 8-bit depth.
	
*/

GEWorldPtr NewGEWorld(CWindowPtr worldWindow, Rect *worldRect, CTabHandle worldColors);


//Destruction of a GEWorld

/*
	Disposes of a world and everything in it EXCEPT! memory allocated by application
	and stored in GrafElPtrs
*/

void DisposeGEWorld(GEWorldPtr world);


//Management of a GEWorld

/*
	Set default loader function for world.  If no loader is specified when a new
	Graphic Element is created, this function is used.
	
	In a newly-created world, this loader function is one which creates an offscreen
	GWorld and loads one or more PICTS from an open resource fork into it.
*/

void SetDefaultLoadFunc(GEWorldPtr world, GraphicLoadFunc loader);

/*

	Activate or deactivate an individual world
*/

void ActivateWorld(GEWorldPtr world, Boolean turnItOn);

/*
	Get and set projection rate
	These are NOT very useful, but MAY improve performance in some situations on
	slower machines.
*/

void SetProjectionRate(GEWorldPtr world, short newMSPerFrame);

short GetProjectionRate(GEWorldPtr world);

/*
	Control GEWorld's timer
*/

void StartGETimer(GEWorldPtr world);

void StopGETimer(GEWorldPtr world);


//Returns current time in world
unsigned long CurrentGETime(GEWorldPtr world);

/*	Set the number of timer milliseconds that pass per "real"
	millisecond. newRate is treated as a fixed-point number,
	with a two-byte positive integer and a two byte fraction,
	such that a value of 00010000 will give a 1-to-1
	correspondence between Graphic Elements "milliseconds"
	and real milliseconds.
*/

void SetGETimerRate(GEWorldPtr world, unsigned long newRate);

//Returns current "rate" of timer in world
unsigned long GetGETimerRate(GEWorldPtr world);

/*
	Manipulate world coordinate system;
*/

/*
	GetGEWorldFocus and SetGEWorldFocus should be used to "bracket" calls to
	FocusOnGEWorld, in order to save and restore the previous focus.
*/

void GetGEWorldFocus(GEWorldPtr world, Point *currFocus);
void SetGEWorldFocus(GEWorldPtr world, Point newFocus);

/*
	Sets focus, i.e. coordinate system, to that of world
*/

void FocusOnGEWorld(GEWorldPtr world);

/*
	Move an entire GEWorld relative to its window
*/

void MoveGEWorld(GEWorldPtr world, short dh, short dv);
void MoveGEWorldTo(GEWorldPtr world, short h, short v);

/*
	Generate a frame showing the world in its current state and display it on the screen.
	Should be called frequently from application program, for example once each time
	through the main event loop.
	
	If invalidate is true, the entire world is redrawn. Otherwise, only the changed
	portions of world are updated.
*/

void DoWorldUpdate(GEWorldPtr world, Boolean invalidate);

/*
	Call from event handler when mouse down in the window containing world.
	Returns true if mouseDown was handled by a sensor, false otherwise.
*/

Boolean MouseDownInSensor(GEWorldPtr world, Point gMousePt);


//Services provided by Display Controller for Graphic Elements

//Creation and destruction

/* 
	Low-level element creation. This function allocates an element, calls its
	GraphicLoadFunc proc (if any), and links it into the world. In itself, it 
	initializes only the fields element->objectID and element-<drawPlane (in
	addition to those initialized by the GraphicLoadFunc).
	
	It is normally called only from a higher-level creation function.
*/

GrafElPtr NewGrafElement(GEWorldPtr world, OSType id, short plane, short elemSize,
							GraphicLoadFunc loadProc, short resStart, short nRsrcs);
							
/*
	Unlinks a graphic element from its world and disposes of it. Note: if the element is
	part of a higher-level struct, i.e. if fields have been added, the contents of these
	fields must be disposed of before calling DisposeGrafElement.
*/

void DisposeGrafElement(GEWorldPtr world, OSType objectID);

//Access

/*
	Returns individual element with ID objectID
*/

GrafElPtr FindElementByID(GEWorldPtr world, OSType objectID);

/*
	Returns first element with drawPlane == plane. For collision checking, etc.
*/

GrafElPtr ElementsInPlane(GEWorldPtr world, short plane);

//Control

/*
	ShowElement shows or hides a graphic element, depending on the value of showIt
*/

void ShowElement(GEWorldPtr world, OSType elementID, Boolean showIt);

/*
	Move graphic elements
*/

void MoveElement(GEWorldPtr world, OSType elementID, short dh, short dv);
void MoveElementTo(GEWorldPtr world, OSType elementID, short h, short v);

//Alternate interface: avoid lookup when GrafElPtr already available, for example
//in Autochange procedures

void PtrMoveElement(GEWorldPtr world, GrafElPtr element, short dh, short dv);
void PtrMoveElementTo(GEWorldPtr world, GrafElPtr element, short h, short v);

/*
	Sets element's plane to newPlane
*/

void SetElementPlane(GEWorldPtr world, OSType elementID, short newPlane);

/*
	Set element's autochange parameters
*/

void SetAutoChange(GEWorldPtr world, OSType elementID, AutoChangeProc changeProc,
						Ptr changeData, short changeIntrvl);
						
/*
	Set element's collision parameters. No collision if collisionPlane == 0
*/

void SetCollision(GEWorldPtr world, OSType elementID, CollisionProc collideProc, 
						short collidePlane);

//Miscellaneous sensor services

//Add sensor to world's sensor list and make it active
void AddSensorToList(GEWorldPtr world, OSType id, Rect *sensorRect);

//Remove sensor from world's sensor list and make it inactive
void RemoveSensorFromList(GEWorldPtr world, OSType id);

//Set sensor's actionProc to newAction
void SetSensorAction(GEWorldPtr world, OSType sensorID, SensorAction newAction);

//Get element's entry in world->sensorList, if any
SListEntryPtr FindSensorListEntry(GEWorldPtr world, OSType sensorID);

/*
	If anything in a world is changed from OUTSIDE the Graphic Elements system,
	call this procedure with the affected rectangle.
*/

void ChangedRect(GEWorldPtr world, Rect *chgdRect);


/*
	GraphicLoadFunc which:
	
	1)	Searches to see if the required graphics are already available in this world, 
		and copies their parameters if so,
	2) 	Otherwise, creates a new offscreen GWorld of an appropriate size, and
		loads nResources PICTs from an open resource file into it, starting
		at the PICT with resource number startResNum
		
	This is the default loaderFunc of a newly created GEWorld.
*/

pascal Boolean LoadPICTElement(GEWorldPtr world, GrafElPtr element,
										short startResNum, short nResources);


/*
	GraphicLoadFunc which does nothing except initialize the appropriate fields.
	For Graphic Elements which have no permanent offscreen representations, e.g.
	strings or fills.
*/

pascal Boolean NoLoader(GEWorldPtr world, GrafElPtr element,
										short startResNum, short nResources);



#ifdef __cplusplus
}
#endif


#endif