#ifndef __WINDOW_LAYER_H__
#define __WINDOW_LAYER_H__

#include "offscreen layer.h"
#include "program globals.h"
#if USE_DRAG
#include <Drag.h>
#endif

enum WindowAttributes
{
	kHasCloseBoxMask = 				0x00000001,
	kHasZoomBoxMask = 				0x00000002,
	kHasGrowBoxMask = 				0x00000004,
	kHasModalBorderMask = 			0x00000010,
	kHasThickDropShadowMask = 		0x00000020,
	kHasDocumentTitlebarMask = 		0x00001000,
	kHasPaletteTitlebarMask = 		0x00002000,
	kHasRoundedTitlebarMask = 		0x00004000,

	kWindowGadgetsMask =			0x0000000F,
	kWindowAdornmentsMask =			0x00000FF0,
	kWindowTitlebarMask =			0x000FF000
};

typedef unsigned long WindowAttributes;

enum
{
	kActivateWindow				= TRUE,
	kDeactivateWindow			= FALSE
};

enum
{
	kAlwaysOpenNew				= TRUE,
	kOpenOldIfPossible			= FALSE
};

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct ExtendedWindowStruct
{
	WindowRecord	theWindow;				/* original window record */
	unsigned long	magic;					/* magic number to see if window is ours */
	WindowAttributes	attrs;				/* attributes for window, see above enums */
	short			windowIndex;			/* index of window in program's window list */
	short			windowWidth;			/* width of window content, in pixels */
	short			windowHeight;			/* height of window content, in pixels */
	short			windowDepth;			/* current pixel depth of window */
	short			maxDepth;				/* maximum pixel depth of window */
	Boolean			active;					/* window is active */
	Boolean			autoCenter;				/* auto center window in main screen when opened */
	Boolean			isFloat;				/* is floating window? */
	Boolean			modified;				/* data in window has been modified since save? */
	Boolean			drawGrowIconLines;		/* draw lines when drawing grow icon */
#if USE_PRINTING
	Boolean			isPrintable;			/* window can print */
#else
	Boolean			printFiller;
#endif
#if USE_DRAG
	Boolean			draggable;				/* window supports drag manager */
#else
	Boolean			dragFiller;
#endif
	Boolean			wasVisible;				/* window was visible before suspending */
	Point			initialTopLeft;			/* initial window bounds when opened */
	Rect			windowBounds;			/* on screen rectangle of window content */
	ControlHandle	vScroll;				/* window's vertical scroll bar */
	ControlHandle	hScroll;				/* window's horizontal scroll bar */
	TEHandle		hTE;					/* window's TextEdit handle */
	Str63			title;					/* window title */
	FSSpec			fs;						/* FSSpec for disk file associated w/window data */
	TEClickLoopUPP	oldClickLoopProc;		/* default click proc */
	short			lastFindPosition;		/* last find position if window is searchable */
	Boolean			hasPermanentOffscreenWorld;	/* TRUE if window has never-disposed offscreen world */
	Boolean			offscreenNeedsUpdate;	/* permanent offscreen world is stale? */
	Boolean			isColor;				/* color or grayscale */
	Boolean			editable;				/* window contains editable text fields? */
	Boolean			zoomable;				/* window is zoomable? */
	Boolean			isZoomed;				/* window is currently zoomed? */
	MyOffscreenPtr	permanentOffscreenWorldPtr;	/* ptr to offscreen world saved between updates */
#if USE_DRAG
	RgnHandle		hiliteRgn;				/* used by Drag Manager */
	DragTrackingHandlerUPP	dragTrackingUPP;
	DragReceiveHandlerUPP	dragReceiveUPP;
#endif
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct ExtendedWindowStruct ExtendedWindowStruct, *ExtendedWindowRef;

enum	/* wdef id's */
{
	rStandardDocumentWindowProc =	0,
	rRoundedWindowProc =			1,
	rFloatingWindowProc =			128
};

enum	/* proc id's */
{
	kStandardDocumentWindowProc = rStandardDocumentWindowProc * 16,
	kRoundedWindowProc = rRoundedWindowProc * 16,
	kFloatingWindowProc = rFloatingWindowProc * 16,
	
	kApplicationFloaterKind = 7,
	kApplicationDummyKind = 23
};

//#define GetWindowKind(w)				(((WindowPeek)w)->windowKind)
#define GetWindowVisible(w)				(((ExtendedWindowRef)w)->theWindow.visible)
#define GetWindowHilite(w)				(((ExtendedWindowRef)w)->theWindow.hilited)
//#define GetNextWindow(w)				(WindowRef)(((ExtendedWindowRef)w)->theWindow.nextWindow)
#define GetContentRegion(w)				(((ExtendedWindowRef)w)->theWindow.contRgn)
#define GetStructureRegion(w)			(((ExtendedWindowRef)w)->theWindow.strucRgn)
#define GetWindowDefProc(w)				(((ExtendedWindowRef)w)->theWindow.windowDefProc)
#define GetWasVisible(w)				(((ExtendedWindowRef)w)->wasVisible)

#define GetWindowAttributes(w)			(((ExtendedWindowRef)w)->attrs)
#define GetWindowMagic(w)				(((ExtendedWindowRef)w)->magic)
#define GetWindowIndex(w)				(((ExtendedWindowRef)w)->windowIndex)
#define GetWindowWidth(w)				(((ExtendedWindowRef)w)->windowWidth)
#define GetWindowHeight(w)				(((ExtendedWindowRef)w)->windowHeight)
#define GetWindowDepth(w)				(((ExtendedWindowRef)w)->windowDepth)
#define GetWindowMaxDepth(w)			(((ExtendedWindowRef)w)->maxDepth)
#define WindowAutoCenterQQ(w)			(((ExtendedWindowRef)w)->autoCenter)
#define WindowIsFloatQQ(w)				(((ExtendedWindowRef)w)->isFloat)
#define WindowIsModifiedQQ(w)			(((ExtendedWindowRef)w)->modified)
#define GetWindowTopLeft(w)				(((ExtendedWindowRef)w)->initialTopLeft)
#define GetWindowBounds(w)				(((ExtendedWindowRef)w)->windowBounds)
#define GetWindowVScrollBar(w)			(((ExtendedWindowRef)w)->vScroll)
#define GetWindowHScrollBar(w)			(((ExtendedWindowRef)w)->hScroll)
#define GetWindowTE(w)					(((ExtendedWindowRef)w)->hTE)
#define GetWindowTitle(w)				(((ExtendedWindowRef)w)->title)
#define GetWindowFS(w)					(((ExtendedWindowRef)w)->fs)
#define GetWindowOldClickLoopProc(w)	(((ExtendedWindowRef)w)->oldClickLoopProc)
#define WindowIsActiveQQ(w)				(((ExtendedWindowRef)w)->active)
#define GetWindowLastFindPosition(w)	(((ExtendedWindowRef)w)->lastFindPosition)
#define WindowDrawGrowIconLinesQQ(w)	(((ExtendedWindowRef)w)->drawGrowIconLines)
#if USE_PRINTING
#define WindowIsPrintableQQ(w)			(((ExtendedWindowRef)w)->isPrintable)
#endif
#define WindowHasPermanentOffscreenWorldQQ(w)	(((ExtendedWindowRef)w)->hasPermanentOffscreenWorld)
#define GetWindowPermanentOffscreenWorld(w)	(((ExtendedWindowRef)w)->permanentOffscreenWorldPtr)
#define WindowOffscreenNeedsUpdateQQ(w)	(((ExtendedWindowRef)w)->offscreenNeedsUpdate)
#define WindowIsColorQQ(w)				(((ExtendedWindowRef)w)->isColor)
#define WindowIsEditableQQ(w)			(((ExtendedWindowRef)w)->editable)
#define WindowIsZoomableQQ(w)			(((ExtendedWindowRef)w)->zoomable)
#define WindowIsZoomedQQ(w)				(((ExtendedWindowRef)w)->isZoomed)
#if USE_DRAG
#define GetWindowHiliteRgn(w)			(((ExtendedWindowRef)w)->hiliteRgn)
#define WindowIsDraggableQQ(w)			(((ExtendedWindowRef)w)->draggable)
#define GetWindowDragTrackingHandler(w)	(((ExtendedWindowRef)w)->dragTrackingUPP)
#define GetWindowDragReceiveHandler(w)	(((ExtendedWindowRef)w)->dragReceiveUPP)
#endif

//#define SetWindowKind(w,x)				{ ((WindowPeek)w)->windowKind=x; }
#define SetWindowVisible(w,x)			{ ((ExtendedWindowRef)w)->theWindow.visible=x; }
#define SetWindowHilite(w,x)			{ ((ExtendedWindowRef)w)->theWindow.hilited=x; }
#define SetNextWindow(w,x)				{ ((ExtendedWindowRef)w)->theWindow.nextWindow=(WindowPeek)x; }
#define SetWasVisible(w,x)				{ ((ExtendedWindowRef)w)->wasVisible=x; }

#define SetWindowAttributes(w,x)		{ ((ExtendedWindowRef)w)->attrs=x; }
#define SetWindowMagic(w,x)				{ ((ExtendedWindowRef)w)->magic=x; }
#define SetWindowIndex(w,x)				{ ((ExtendedWindowRef)w)->windowIndex=x; }
#define SetWindowWidth(w,x)				{ ((ExtendedWindowRef)w)->windowWidth=x; }
#define SetWindowHeight(w,x)			{ ((ExtendedWindowRef)w)->windowHeight=x; }
#define SetWindowDepth(w,x)				{ ((ExtendedWindowRef)w)->windowDepth=x; }
#define SetWindowMaxDepth(w,x)			{ ((ExtendedWindowRef)w)->maxDepth=x; }
#define SetWindowAutoCenter(w,x)		{ ((ExtendedWindowRef)w)->autoCenter=x; }
#define SetWindowIsFloat(w,x)			{ ((ExtendedWindowRef)w)->isFloat=x; }
#define SetWindowIsModified(w,x)		{ ((ExtendedWindowRef)w)->modified=x; }
#define SetWindowTopLeft(w,x)			{ ((ExtendedWindowRef)w)->initialTopLeft=x; }
#define SetWindowBounds(w,x)			{ ((ExtendedWindowRef)w)->windowBounds=x; }
#define SetWindowVScrollBar(w,x)		{ ((ExtendedWindowRef)w)->vScroll=x; }
#define SetWindowHScrollBar(w,x)		{ ((ExtendedWindowRef)w)->hScroll=x; }
#define SetWindowTE(w,x)				{ ((ExtendedWindowRef)w)->hTE=x; }
#define SetWindowTitle(w,x)				{ BlockMove(x, (((ExtendedWindowRef)w)->title), x[0]+1); }
#define SetWindowFS(w,x)				{ ((ExtendedWindowRef)w)->fs=x; }
#define SetWindowOldClickLoopProc(w,x)	{ ((ExtendedWindowRef)w)->oldClickLoopProc=x; }
#define SetWindowIsActive(w,x)			{ ((ExtendedWindowRef)w)->active=x; }
#define SetWindowLastFindPosition(w,x)	{ ((ExtendedWindowRef)w)->lastFindPosition=x; }
#define SetWindowDrawGrowIconLines(w,x)	{ ((ExtendedWindowRef)w)->drawGrowIconLines=x; };
#if USE_PRINTING
#define SetWindowIsPrintable(w,x)		{ ((ExtendedWindowRef)w)->isPrintable=x; }
#endif
#define SetWindowHasPermanentOffscreenWorld(w,x)	{ ((ExtendedWindowRef)w)->hasPermanentOffscreenWorld=x; }
#define SetWindowPermanentOffscreenWorld(w,x)	{ ((ExtendedWindowRef)w)->permanentOffscreenWorldPtr=x; }
#define SetWindowOffscreenNeedsUpdate(w,x)	{ ((ExtendedWindowRef)w)->offscreenNeedsUpdate=x; }
#define SetWindowIsColor(w,x)			{ ((ExtendedWindowRef)w)->isColor=x; }
#define SetWindowIsEditable(w,x)		{ ((ExtendedWindowRef)w)->editable=x; }
#define SetWindowIsZoomable(w,x)		{ ((ExtendedWindowRef)w)->zoomable=x; }
#define SetWindowIsZoomed(w,x)			{ ((ExtendedWindowRef)w)->isZoomed=x; }
#if USE_DRAG
#define SetWindowHiliteRgn(w,x)			{ ((ExtendedWindowRef)w)->hiliteRgn=x; }
#define SetWindowIsDraggable(w,x)		{ ((ExtendedWindowRef)w)->draggable=x; }
#define SetWindowDragTrackingHandler(w,x)	{ ((ExtendedWindowRef)w)->dragTrackingUPP=x; }
#define SetWindowDragReceiveHandler(w,x)	{ ((ExtendedWindowRef)w)->dragReceiveUPP=x; }
#endif

#define IndWindowExistsQQ(x)		(GetIndWindowRef(x)!=0L)
#define WindowExistsQQ(w)			(w!=0L)
#define FrontDocumentWindow()		GetFrontDocumentWindow()
#define WindowIsModal(w)			WindowIsModalQQ(w)
#define FrontNonFloatingWindow()	GetFrontDocumentWindow()
#define LastFloatingWindow()		GetLastFloatingWindow()

#ifdef __cplusplus
extern "C" {
#endif

extern	WindowRef MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, WindowAttributes attributes, WindowRef behind, long refCon, Boolean isFloat,
	Boolean isDummy);
extern	void MyDisposeWindow(WindowRef windowReference);
extern	Boolean MySelectWindow(WindowRef windowToSelect);
extern	void MyShowWindow(WindowRef windowToShow);
extern	void MyHideWindow(WindowRef windowToHide);
extern	void MyDragWindow(WindowRef windowToDrag, Point startPoint, const Rect *draggingBounds);
extern	void MyMoveWindow(WindowRef theWindow, short h, short v, Boolean selectNow);
extern	void MyGrowWindow(WindowRef theWindow, Point theGlobalPoint);
extern	void MySizeWindow(WindowRef theWindow, short width, short height, Boolean updateNow);
extern	void MyZoomWindow(WindowRef theWindow, short windowCode);
extern	Boolean MySendBehind(WindowRef window, WindowRef behind);
extern	WindowRef GetFrontDocumentWindow(void);
extern	WindowRef GetLastFloatingWindow(void);
extern	Boolean WindowIsModalQQ(WindowRef windowReference);
extern	void DeactivateFloatersAndFirstDocumentWindow(void);
extern	void ActivateFloatersAndFirstDocumentWindow(void);
extern	void HideOneFloatingWindow(WindowRef floatingWindow);
extern	void ShowOneFloatingWindow(WindowRef floatingWindow);
extern	void SuspendFloatingWindows(void);
extern	void ResumeFloatingWindows(void);
extern	void ValidateWindowList(void);
extern	void GetWindowPortRect(WindowRef windowReference, Rect *portRect);
extern	Boolean WindowHasLayer(WindowRef theWindow);
extern	Boolean WindowIsFloat(WindowRef theWindow);
extern	WindowRef GetIndWindowRef(short index);
extern	OSErr OpenTheIndWindow(short index, Boolean newStatus);
extern	OSErr UpdateTheWindow(WindowRef theWindow);
extern	Boolean CloseTheWindow(WindowRef theWindow);
extern	void CloseAllDocumentWindows(void);
extern	void GetMainScreenBounds(Rect *screenRect);
extern	short GetBiggestDeviceDepth(WindowRef theWindow);
extern	short GetWindowRealDepth(WindowRef theWindow);
extern	Boolean GetWindowReallyIsColor(WindowRef theWindow);
extern	void ArrangeWindows(void);

#ifdef __cplusplus
}
#endif

#endif
