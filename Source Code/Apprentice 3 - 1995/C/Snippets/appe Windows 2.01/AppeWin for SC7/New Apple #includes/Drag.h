/*
	File:		Drag.h

	Contains:	C Interface to the Drag Manager

	Copyright:	� 1992-1994 by Apple Computer, Inc., all rights reserved.

*/

#ifndef __DRAG__
#define __DRAG__

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif


/* Gestalt Selector and Response Constants (will move to GestaltEqu.h) */

#define	gestaltDragMgrAttr			'drag'		// Drag Manager attributes
#define	gestaltDragMgrPresent		0			// Drag Manager is present
#define gestaltDragMgrFloatingWind	1			// Drag Manager supports floating windows
#define gestaltPPCDragLibPresent	2			// Drag Manager PPC DragLib is present

#define	gestaltTEAttr				'teat'		// TextEdit attributes
#define gestaltTEHasGetHiliteRgn	0			// TextEdit has TEGetHiliteRgn


/* Flavor Flags */

enum {
	flavorSenderOnly				= 0x00000001L,	// flavor is available to sender only
	flavorSenderTranslated			= 0x00000002L,	// flavor is translated by sender
	flavorNotSaved					= 0x00000004L,	// flavor should not be saved
	flavorSystemTranslated			= 0x00000100L	// flavor is translated by system
};

typedef unsigned long FlavorFlags;


/* Drag Attributes */

enum {
	dragHasLeftSenderWindow			= 0x00000001L,	// drag has left the source window since TrackDrag
	dragInsideSenderApplication		= 0x00000002L,	// drag is occurring within the sender application
	dragInsideSenderWindow			= 0x00000004L	// drag is occurring within the sender window
};

typedef unsigned long DragAttributes;


/* Special Flavor Types */

#define flavorTypeHFS				'hfs '		// flavor type for HFS data
#define flavorTypePromiseHFS		'phfs'		// flavor type for promised HFS data
#define flavorTypeDirectory			'diry'		// flavor type for AOCE directories


/* Drag Tracking Handler Messages */

enum {
	dragTrackingEnterHandler		= 1,		// drag has entered handler
	dragTrackingEnterWindow			= 2,		// drag has entered window
	dragTrackingInWindow			= 3,		// drag is moving within window
	dragTrackingLeaveWindow			= 4,		// drag has exited window
	dragTrackingLeaveHandler		= 5			// drag has exited handler
};

typedef short DragTrackingMessage;


/* Drag Drawing Procedure Messages */

enum {
	dragRegionBegin					= 1,		// initialize drawing
	dragRegionDraw					= 2,		// draw drag feedback
	dragRegionHide					= 3,		// hide drag feedback
	dragRegionIdle					= 4,		// drag feedback idle time
	dragRegionEnd					= 5			// end of drawing
};

typedef short DragRegionMessage;


/* Zoom Acceleration */

enum {
	zoomNoAcceleration				= 0,		// use linear interpolation
	zoomAccelerate					= 1,		// ramp up step size
	zoomDecelerate					= 2			// ramp down step size
};

typedef short ZoomAcceleration;


/* Drag Manager Data Types */

typedef unsigned long DragReference;
typedef unsigned long ItemReference;

typedef ResType FlavorType;


/* Result Codes - (will move to Errors.h) */

enum {
	badDragRefErr				= -1850,	// unknown drag reference
	badDragItemErr				= -1851,	// unknown drag item reference
	badDragFlavorErr			= -1852,	// unknown flavor type
	duplicateFlavorErr			= -1853,	// flavor type already exists
	cantGetFlavorErr			= -1854,	// error while trying to get flavor data
	duplicateHandlerErr			= -1855,	// handler already exists
	handlerNotFoundErr			= -1856,	// handler not found
	dragNotAcceptedErr			= -1857		// drag was not accepted by receiver
};

/* HFS Flavors */

struct HFSFlavor {
	OSType			fileType;					// file type
	OSType			fileCreator;				// file creator
	unsigned short	fdFlags;					// Finder flags
	FSSpec			fileSpec;					// file system specification
};
typedef struct HFSFlavor HFSFlavor;

struct PromiseHFSFlavor {
	OSType			fileType;					// file type
	OSType			fileCreator;				// file creator
	unsigned short	fdFlags;					// Finder flags
	FlavorType		promisedFlavor;				// promised flavor containing an FSSpec
};
typedef struct PromiseHFSFlavor PromiseHFSFlavor;

#ifdef __cplusplus
extern "C" {
#endif

/* Application-Defined Drag Handler Routines */

typedef pascal OSErr (*DragTrackingHandler) (DragTrackingMessage message, WindowPtr theWindow,
											 void *handlerRefCon, DragReference theDragRef);

typedef pascal OSErr (*DragReceiveHandler) (WindowPtr theWindow, void *handlerRefCon,
											DragReference theDragRef);


/* Application-Defined Routines */

typedef pascal OSErr (*DragSendDataProc) (FlavorType theType, void *dragSendRefCon,
										  ItemReference theItemRef, DragReference theDragRef);

typedef pascal OSErr (*DragInputProc) (Point *mouse, short *modifiers, void *dragInputRefCon,
									   DragReference theDragRef);

typedef pascal OSErr (*DragDrawingProc) (DragRegionMessage message,
										 RgnHandle showRegion, Point showOrigin,
										 RgnHandle hideRegion, Point hideOrigin,
										 void *dragDrawingRefCon, DragReference theDragRef);


/*
 *	Drag Manager Routines
 */


/* Installing and Removing Drag Handlers */

pascal OSErr InstallTrackingHandler
								(DragTrackingHandler trackingHandler,
								 WindowPtr theWindow,
								 void *handlerRefCon)
	= { 0x7001, 0xABED };

pascal OSErr InstallReceiveHandler
								(DragReceiveHandler receiveHandler,
								 WindowPtr theWindow,
								 void *handlerRefCon)
	= { 0x7002, 0xABED };

pascal OSErr RemoveTrackingHandler
								(DragTrackingHandler trackingHandler,
								 WindowPtr theWindow)
	= { 0x7003, 0xABED };

pascal OSErr RemoveReceiveHandler
								(DragReceiveHandler receiveHandler,
								 WindowPtr theWindow)
	= { 0x7004, 0xABED };


/* Creating and Disposing Drag References */

pascal OSErr NewDrag			(DragReference *theDragRef)
	= { 0x7005, 0xABED };

pascal OSErr DisposeDrag		(DragReference theDragRef)
	= { 0x7006, 0xABED };


/* Adding Drag Item Flavors */

pascal OSErr AddDragItemFlavor	(DragReference theDragRef,
								 ItemReference theItemRef,
								 FlavorType theType,
								 void *dataPtr,
								 Size dataSize,
								 FlavorFlags theFlags)
	= { 0x7007, 0xABED };

pascal OSErr SetDragItemFlavorData
								(DragReference theDragRef,
								 ItemReference theItemRef,
								 FlavorType theType,
								 const void *dataPtr,
								 Size dataSize,
								 unsigned long dataOffset)
	= { 0x7009, 0xABED };


/* Providing Drag Callback Procedures */

pascal OSErr SetDragSendProc	(DragReference theDragRef,
								 DragSendDataProc sendProc,
								 void *dragSendRefCon)
	= { 0x700A, 0xABED };

pascal OSErr SetDragInputProc	(DragReference theDragRef,
								 DragInputProc inputProc,
								 void *dragInputRefCon)
	= { 0x700B, 0xABED };

pascal OSErr SetDragDrawingProc	(DragReference theDragRef,
								 DragDrawingProc drawingProc,
								 void *dragDrawingRefCon)
	= { 0x700C, 0xABED };


/* Performing a Drag */

pascal OSErr TrackDrag			(DragReference theDragRef,
								 const EventRecord *theEvent,
								 RgnHandle theRegion)
	= { 0x700D, 0xABED };


/* Getting Drag Item Information */

pascal OSErr CountDragItems		(DragReference theDragRef,
								 unsigned short *numItems)
	= { 0x700E, 0xABED };

pascal OSErr GetDragItemReferenceNumber
								(DragReference theDragRef,
								 unsigned short index,
								 ItemReference *theItemRef)
	= { 0x700F, 0xABED };

pascal OSErr CountDragItemFlavors
								(DragReference theDragRef,
								 ItemReference theItemRef,
								 unsigned short *numFlavors)
	= { 0x7010, 0xABED };

pascal OSErr GetFlavorType		(DragReference theDragRef,
								 ItemReference theItemRef,
								 unsigned short index,
								 FlavorType *theType)
	= { 0x7011, 0xABED };

pascal OSErr GetFlavorFlags		(DragReference theDragRef,
								 ItemReference theItemRef,
								 FlavorType theType,
								 FlavorFlags *theFlags)
	= { 0x7012, 0xABED };

pascal OSErr GetFlavorDataSize	(DragReference theDragRef,
								 ItemReference theItemRef,
								 FlavorType theType,
								 Size *dataSize)
	= { 0x7013, 0xABED };

pascal OSErr GetFlavorData		(DragReference theDragRef,
								 ItemReference theItemRef,
								 FlavorType theType,
								 void *dataPtr,
								 Size *dataSize,
								 unsigned long dataOffset)
	= { 0x7014, 0xABED };

pascal OSErr GetDragItemBounds	(DragReference theDragRef,
								 ItemReference theItemRef,
								 Rect *itemBounds)
	= { 0x7015, 0xABED };

pascal OSErr SetDragItemBounds	(DragReference theDragRef,
								 ItemReference theItemRef,
								 const Rect *itemBounds)
	= { 0x7016, 0xABED };

pascal OSErr GetDropLocation	(DragReference theDragRef,
								 AEDesc *dropLocation)
	= { 0x7017, 0xABED };

pascal OSErr SetDropLocation	(DragReference theDragRef,
								 const AEDesc *dropLocation)
	= { 0x7018, 0xABED };


/* Getting Information About a Drag */

pascal OSErr GetDragAttributes	(DragReference theDragRef,
								 DragAttributes *flags)
	= { 0x7019, 0xABED };

pascal OSErr GetDragMouse		(DragReference theDragRef,
								 Point *mouse,
								 Point *pinnedMouse)
	= { 0x701A, 0xABED };

pascal OSErr SetDragMouse		(DragReference theDragRef,
								 Point pinnedMouse)
	= { 0x701B, 0xABED };

pascal OSErr GetDragOrigin		(DragReference theDragRef,
								 Point *initialMouse)
	= { 0x701C, 0xABED };

pascal OSErr GetDragModifiers	(DragReference theDragRef,
								 short *modifiers,
								 short *mouseDownModifiers,
								 short *mouseUpModifiers)
	= { 0x701D, 0xABED };


/* Drag Highlighting */

pascal OSErr ShowDragHilite		(DragReference theDragRef,
								 RgnHandle hiliteFrame,
								 Boolean inside)
	= { 0x701E, 0xABED };

pascal OSErr HideDragHilite		(DragReference theDragRef)
	= { 0x701F, 0xABED };

pascal OSErr DragPreScroll		(DragReference theDragRef,
								 short dH,
								 short dV)
	= { 0x7020, 0xABED };

pascal OSErr DragPostScroll		(DragReference theDragRef)
	= { 0x7021, 0xABED };

pascal OSErr UpdateDragHilite	(DragReference theDragRef,
								 RgnHandle updateRgn)
	= { 0x7022, 0xABED };


/* Drag Manager Utilities */

pascal Boolean WaitMouseMoved	(Point initialMouse)
	= { 0x7023, 0xABED };

pascal OSErr ZoomRects			(const Rect *fromRect,
								 const Rect *toRect,
								 short zoomSteps,
								 ZoomAcceleration acceleration)
	= { 0x7024, 0xABED };

pascal OSErr ZoomRegion			(RgnHandle region,
								 Point zoomDistance,
								 short zoomSteps,
								 ZoomAcceleration acceleration)
	= { 0x7025, 0xABED };

// Will move to TextEdit.h
pascal OSErr TEGetHiliteRgn		(RgnHandle region, TEHandle hTE)
	= { 0x3F3C, 0x000F, 0xA83D };

#ifdef __cplusplus
}
#endif

#endif
