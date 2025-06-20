#ifndef __FABCURSORBALLOON__
#define __FABCURSORBALLOON__

/* this distinguishes a normal window from a window with objects */

enum windowclasses {
kFabWindowClass = 10
};

/* this is my window record, containing the standard Dialog Manager record,
the procedures normally called when handling events (if you do not want to
undertake special actions i.e. for dragging, pass nil), the number of
objects, and a Handle to the objects */

typedef struct RgnBalloonCurs RgnBalloonCurs;
typedef RgnBalloonCurs *RgnBalloonCursPtr;
typedef RgnBalloonCursPtr *RgnBalloonCursHandle;

struct FabWindowRecord {
	DialogRecord	w;
	void (*activateProc)(EventRecord *, WindowPtr, Boolean);
	void (*updateProc)(WindowPtr);
	void (*dragProc)(WindowPtr);
	void (*growProc)(WindowPtr, EventRecord *);
	void (*zoomProc)(WindowPtr);
	void (*goAwayProc)(WindowPtr);
	void (*contentProc)(WindowPtr, EventRecord *);
	void (*getDragHiliteRectProc)(WindowPtr, RectPtr);
	Boolean (*canIAcceptDrag)(const HFSFlavor *);
	OSErr (*dragReaction)(WindowPtr, const HFSFlavor *);
	unsigned long	objCount;
	DragTrackingHandlerUPP	trackUPP;
	DragReceiveHandlerUPP	recUPP;
	RgnBalloonCursHandle	myZones;
	};

typedef struct FabWindowRecord FabWindowRecord;
typedef FabWindowRecord *FabWindowPtr;

/* this is a generic object belonging to a window */

struct RgnBalloonCurs {
	void (*recalcRgnProc)(FabWindowPtr, RgnBalloonCursPtr);
	RgnHandle	zoneLocal;
	RgnHandle	zoneGlobal;
	CursHandle	curs;
	unsigned long	myBalloon;
	short		balloonVariant;
	short		itemID;
	};


OSErr InstallRgnHandler(FabWindowPtr w, RgnHandle whichRgn, void (*recalcProc)(FabWindowPtr, RgnBalloonCursPtr),
						CursHandle cursor,
						unsigned long balloon, short ballnVariant, short iID);
void RecalcMouseRegion(WindowPtr w, Point mouse);
void ResizeObjects(FabWindowPtr w);
void RecalcGlobalCoords(FabWindowPtr w);
void DisposFabWindow(FabWindowPtr w);
void ForceMouseMovedEvent(void);
void InitFabWindow(FabWindowPtr w);

/* useful macros */

#define toBalloon(m, i)		(((long)m << 16) + i)

#define Zones(w)			(((FabWindowPtr)w)->myZones)

/* macros for setting the event handlers for the window, and
prototypes of the handlers */

#define SetActivate(w, p)	((FabWindowPtr)w)->activateProc = p
#define SetUpdate(w, p)		((FabWindowPtr)w)->updateProc = p
#define SetDrag(w, p)		((FabWindowPtr)w)->dragProc = p
#define SetGrow(w, p)		((FabWindowPtr)w)->growProc = p
#define SetZoom(w, p)		((FabWindowPtr)w)->zoomProc = p
#define SetGoAway(w, p)		((FabWindowPtr)w)->goAwayProc = p
#define SetContent(w, p)	((FabWindowPtr)w)->contentProc = p
#define SetGetDragRect(w, p)	((FabWindowPtr)w)->getDragHiliteRectProc = p
#define SetCanIAcceptDrag(w, p)	((FabWindowPtr)w)->canIAcceptDrag = p
#define SetDragReaction(w, p)	((FabWindowPtr)w)->dragReaction = p

#define NumObjects(w)		(((FabWindowPtr)w)->objCount)
#define OneMoreObject(w)	(((FabWindowPtr)w)->objCount++)

#define IsFabWindow(w)		(w && (((WindowPeek)w)->windowKind == kFabWindowClass || (isMovableModal(w) && GetPtrSize((Ptr)w) > sizeof(DialogRecord))))

extern RgnHandle	mouseRgn, wideOpenRgn;

#endif

