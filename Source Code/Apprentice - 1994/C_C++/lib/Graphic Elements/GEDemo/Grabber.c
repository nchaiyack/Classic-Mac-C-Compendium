/*
	Grabber.c
	
	Special Graphic Element to manipulate other Graphic Elements
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/29/93
	
	NOTICE:
	
	Because it acts as a "meta-element", this grabber utilizes knowledge of the
	low-level implementation details of graphic elements and sensors. 
	
*/

#include "Grabber.h"

pascal void RenderGrabber(GrafElPtr graphic, GWorldPtr destGWorld)
{
#pragma unused (destGWorld)
	if (graphic->slaveGrafEl)
		FrameRect(&graphic->animationRect);
}

void DropSelection(GEWorldPtr world, GrabberPtr grabber)
{
	GrafElPtr element = (GrafElPtr) grabber;
	
	if (element->slaveGrafEl) {
		ChangedRect(world, &element->slaveGrafEl->animationRect);
		element->slaveGrafEl->changeIntrvl = grabber->slaveChgIntrvl;
		element->slaveGrafEl = nil;
		RectZero(&element->animationRect);
	}
}

Boolean GetNewSelection(GEWorldPtr world, GrabberPtr grabber, Point mousePt)
{
	GrafElPtr	newSelection = nil;
	GrafElPtr	testElement;
	Rect*		testRect;
	
	testElement = world->drawList;
	
	//Find TOPMOST element which contains mousePt
	while (testElement) {
		testRect = &testElement->animationRect;
		if ((mousePt.v > testRect->top) && (mousePt.v < testRect->bottom) &&
			(mousePt.h > testRect->left) && (mousePt.h < testRect->right) &&
			(testElement->drawPlane >= grabber->minPlane))
			newSelection = testElement;
		testElement = testElement->nextByPlane;
	}
	if (newSelection) {
		((GrafElPtr) grabber)->slaveGrafEl = newSelection;
		grabber->slaveChgIntrvl = newSelection->changeIntrvl;
		newSelection->changeIntrvl = 0;	//Stop autochanges
		((GrafElPtr) grabber)->animationRect = newSelection->animationRect;
		ChangedRect(world, &newSelection->animationRect);
		return true;
	}
	else {
	
		return false;
	}
}

void ActivateGrabber(GEWorldPtr world, Boolean activate)
{
	GrabberPtr	grabber;
	
	
	grabber = (GrabberPtr) FindElementByID(world, grabberID);
	if (grabber) {
		if (activate) {
			((GrafElPtr) grabber)->slaveGrafEl = nil;
			grabber->slaveChgIntrvl = 0;
			AddSensorToList(world, grabberID, &world->animationRect);
			ShowElement(world, grabberID, true);
		}
		else {
			DropSelection(world, grabber);
			RemoveSensorFromList(world, grabberID);
			ShowElement(world, grabberID, false);
		}
	}
}

pascal Boolean TrackGrabber(GEWorldPtr world, GrafElPtr sensor)
/*
	Is mousePoint inside selection?
		No:  Deselect current selection
			 Make new selection
			 If new selection == nil return
	While mouseDown
		Drag selection
*/
{
	Point	localPt, newPt;
	short	dx;
	short	dy;
	OSType	sensorID;
	
	
	GetMouse(&localPt);
	if (!PtInRect(localPt, &sensor->animationRect)) {
		DropSelection(world, (GrabberPtr) sensor);
		if (!GetNewSelection(world, (GrabberPtr) sensor, localPt))
			return true;
	}
	dx = localPt.h - ((GrafElPtr) sensor)->animationRect.left;
	dy = localPt.v - ((GrafElPtr) sensor)->animationRect.top;
	sensorID = sensor->objectID;
	while(StillDown()) {
		GetMouse(&newPt);
		if (*((long *) &localPt) != *((long *) &newPt)) {
			MoveElementTo(world, sensor->slaveGrafEl->objectID, newPt.h - dx, newPt.v - dy);
			sensor->animationRect = sensor->slaveGrafEl->animationRect;
			localPt = newPt;
		}
		DoWorldUpdate(world, false);
	}
	return true;
}

GrafElPtr MakeGrabber(GEWorldPtr world)
{
	GrafElPtr	grabber;
	
	grabber = NewGrafElement(world, grabberID, grabberPlane, sizeof(Grabber), NoLoader, 0, 0);
	if (grabber) {
		grabber->copyMode = srcCopy;
		grabber->flags = 0;
		grabber->renderIt = RenderGrabber;
		grabber->drawIt = nil;
		grabber->changeIntrvl = 0;
		grabber->trackingProc = TrackGrabber;
		grabber->actionProc = nil;
		((GrabberPtr) grabber)->slaveChgIntrvl = 0;
		((GrabberPtr) grabber)->minPlane = 2;
		((GrabberPtr) grabber)->maxPlane = 32000;
		
		
	}
	return grabber;
	
}

Boolean GrabberActive(GEWorldPtr world)
{
	GrafElPtr	grabber;
	grabber = FindElementByID(world, grabberID);
	if	(grabber)
		return (grabber->flags & geShown);
	else
		return false;
}

void SetGrabberDepth(GEWorldPtr world, short newMinPlane, short newMaxPlane)
{
	GrabberPtr	grabber;
	grabber = (GrabberPtr) FindElementByID(world, grabberID);
	if (grabber) {
		grabber->minPlane = newMinPlane;
		grabber->maxPlane = newMaxPlane;
	}
}

GrafElPtr  CurrentGrabberSelection(GEWorldPtr world)
{
	GrafElPtr	grabber;
	
	grabber = FindElementByID(world, grabberID);
	if (grabber)
		return grabber->slaveGrafEl;
	else
		return nil;
}
