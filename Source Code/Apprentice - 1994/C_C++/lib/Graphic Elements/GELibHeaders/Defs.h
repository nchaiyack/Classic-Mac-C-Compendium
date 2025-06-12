/*
	Defs.h
	
	Type definitions for Graphic Elements release version 1.0b1
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	
*/

#ifndef GEDEFS
#define GEDEFS

//Get Toolbox symbols if we're compiling in MPW
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

#include "List.h"

//All offscreen graphics are 8 bits deep
#define	offscrnDepth	8

//Some forward declarations
typedef struct GrafElement* GrafElPtr;
typedef struct GEWorld* GEWorldPtr;

//Built-in directions
typedef enum {none, up, left, down, right, upLeft, upRight, downLeft, downRight} GEDirection;

/* 
	Prototype of function to initialize a graphic element from a resource or series
	of resources. This function is responsible for creating an offscreen GWorld
	for the element if required, and rendering the resource(s) into it.
	
	This function is responsible for setting the fields element->resID, element->graphRect,
	and element->graphWorld.
*/

typedef pascal Boolean (*GraphicLoadFunc) (GEWorldPtr world, GrafElPtr element,
							short startResNum, short nResources);

/*
	Prototype for general-purpose bit copy function type-compatible with CopyBits.
	BitCopyProcs are normally used for offscreen-to-offscreen copying only.
	NOTE THAT THE PARAMETER maskRgn IS ALMOST NEVER A MASK REGION!  It is passed
	from the drawData field of the graphic record. It should normally be nil, but
	can be used for data required by a specific custom bitcopying procedure.
	
	If the "real" CopyBits is used directly for offscreen-to-offscreen drawing, care must be
	taken that this field is nil or contains a real region handle.
*/

typedef pascal void (*BitCopyProc) (const BitMap *srcBits,const BitMap *dstBits,const Rect *srcRect,
    const Rect *dstRect, short mode, RgnHandle maskRgn);
	
/*
	Prototype for autochange procedure, which periodically moves graphic,
	changes frames, scrolls graphic, etc.
*/

typedef pascal void (*AutoChangeProc) (GEWorldPtr world, GrafElPtr element);

/*
	Prototype for interact procedure, which handles collisions
*/

typedef pascal void (*CollisionProc)(GEWorldPtr world, GrafElPtr element, 
											GEDirection dir, GrafElPtr hitElement);
	
/*
	Prototype for rendering procedure to draw element into offscreen "stage" world
*/

typedef pascal void (*RenderProc)(GrafElPtr element, GWorldPtr destGWorld);

/*
	Prototype for sensor-tracking procedure, called when mouse button is pressed
	in a graphic element with sensor properties
*/

typedef pascal Boolean (*SensorTrack)(GEWorldPtr world, GrafElPtr sensor);

/*
	Prototype for action procedure to be called by sensor
*/

typedef pascal void (*SensorAction)(GEWorldPtr world, short sensorState);

/*
	Defined values for flags field of Graphic Element. Bits 5-13 are reserved
	for future system use, bits 16-31 are available for use in defining
	custom Graphic Elements.
*/

enum {	geShown =		0x00000001L,
		geChanged =		0x00000002L,
		geHit =			0x00000004L,
		geSensor =		0x00000008L,
		geForward =		0x00000010L,
		geMirrored =	0x00004000L,
		geInverted =	0x00008000L
	};
	
/*
	The structure of an entry in a world's sensorList
*/

typedef struct SensorListEntry *SListEntryPtr;

typedef struct SensorListEntry {
	SListEntryPtr	nextEntry;
	Rect			sensorRect;
	OSType			sensorID;
} SensorListEntry;

/*
	The variable-rate timer for a GEWorld
*/

//Timer "rate" for 1 ms world time/ms real time
#define geTimerStdRate 0x00010000

typedef struct {
	TMTask			aTMTask;
	unsigned long	currentTime;
	long 			currentRate;
	long			timeAccum;
	Boolean			running;
} GETMgrRec, *GETMgrRecPtr;


	
/*
	A basic Graphic Element
*/

typedef struct GrafElement{

	//Bookkeeping and access
	GrafElPtr		nextByPlane;		//Link in world's drawList
	GrafElPtr		nextByID;			//Link in world's idList
	GrafElPtr		masterGrafEl;		//Master (if any) controls movement of this GrafEl
	GrafElPtr		slaveGrafEl;		//This GrafEl controls movement of slave (if any) 
	OSType			objectID;			//This element's "name"
	short			drawPlane;			//"Level" of object, higher number == closer to front
	short			resID;				//# of resource from which element was derived, if any
	long			flags;				//See definitions above
	
	//Basic graphics data
	GWorldPtr		graphWorld;			//Ptr to offscreen graphic, if any
	Rect			graphRect;			//Source rectangle used by RenderProc
	Rect			animationRect;		//Total dest rectangle, GEWorld coordinates
	RenderProc		renderIt;			//Element's rendering procedure
	BitCopyProc		drawIt;				//Lowlevel bit copier, if any
	Ptr				drawData;			//Data passed to drawIt, if any
	Rect			drawRect;			//Rect to update on this cycle
	short			copyMode;			//Bit transfer mode

	//For automatic periodic changes
	short			changeIntrvl;		//Interval between automatic changes
	unsigned long	lastChangeTime;		//Last time autochange proc was called
	AutoChangeProc	changeIt;			//Function for automatic changes
	Ptr				changeData;			//Extra data for automatic changes
	
	//For collisions
	CollisionProc	doCollision;		//Proc called when element collides
	short			collisionPlane;		//Collides only with elements on this plane
	
	//For direct user interactions
	short			sensorType;			//Used to distinguish variant types
	SensorTrack		trackingProc;		//Called when mousedown in sensor's rectangle
	SensorAction	actionProc;			//Called by trackingProc when sensor is "activated"
	short			sensorState;		//For whatever a sensor needs to save across calls

} GrafElement;
	
/*
	A world of Graphic Elements
*/

typedef struct GEWorld {

	//Objects maintained by this world
	GrafElPtr			drawList;		//Head of linked list of GrafElements in draw order
	GrafElPtr			idList;			//Head of linked list of GrafElements by ID

	//Graphic environment of this world
	CWindowPtr			gEWWindow;		//Onscreen window for this world
	GDHandle			windGDevice;	//Device for this window
	GWorldPtr			stageGWorld;	//Offscreen construction "stage" for this world
	Rect				animationRect;	//Total dest rect of this world
	CTabHandle			worldCTable;	//Offscreen color table, should == onscreen cTable
	Point				worldFocus;		//Offset of world from window top left
	
	//Current state of this world
	GraphicLoadFunc		defaultLoader;	//Loader used if no other is specified
	LHeaderPtr			activeRectList;	//Two lists of update rects, so that animation and
	LHeaderPtr			safeRectList;	//screen updating can be concurrent
	LHeaderPtr			sensorList;		//Sensors in this world, see Sensors.h
	GETMgrRec			worldTime;		//Timer record for this world
	unsigned long		lastFrameTime;	//last time screen was updated
	short				msPerFrame;		//minimum "projection" interval, ms
	Boolean				active;			//True if this animation is running
	Boolean				changed;		//True if at least 1 object needs drawing
	
} GEWorld;

#endif