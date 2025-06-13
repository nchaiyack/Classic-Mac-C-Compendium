#if 0

Usage notes:
- MyNewWindow or MyNewCWindow with all the standard parameters to create a
  new window in the document layer.
- MyNewFloatWindow or MyNewCFloatWindow with all the standard parameters to
  create a new window in the floating layer.
- MySelectWindow(theWindowPtr) to select a window in any layer
- MyDisposeWindow(theWindowPtr) to dispose of a window in any layer

- Call InitTheWindowLayer(void) once at program startup before creating any
  windows in any layer.
- New windows are automatically visible and frontmost in their layer.  Storage
  must be allocated (sizeof(ExtendedWindowRec)) and passed to any window creation
  routine.
- You should only dispose of the frontmost window in a layer.

#endif


#ifndef __WINDOW_LAYER_H__
#define __WINDOW_LAYER_H__

typedef struct
{
	WindowRecord	docWindow;				/* window record */
	unsigned long	magic;					/* magic number to see if window is ours */
	short			layer;					/* layer number */
	WindowPtr		nextWindow;				/* next window in layer (further from front) */
	WindowPtr		previousWindow;			/* previous window in layer (closer to front) */
	short			windowIndex;			/* index of window in program's window list */
	short			windowWidth;			/* width of window content, in pixels */
	short			windowHeight;			/* height of window content, in pixels */
	short			windowType;				/* type of window */
	short			windowDepth;			/* current pixel depth of window */
	short			maxDepth;				/* maximum pixel depth of window */
	Boolean			hasCloseBox;			/* window has a close box */
	Boolean			autoCenter;				/* auto center window in main screen when opened */
	Boolean			isFloat;				/* is floating window? */
	Boolean			modified;
	Point			initialTopLeft;			/* initial window bounds when opened */
	Rect			windowBounds;			/* on screen rectangle of window content */
	ControlHandle	vScroll;				/* window's vertical scroll bar */
	ControlHandle	hScroll;				/* window's horizontal scroll bar */
	TEHandle		hTE;					/* window's TextEdit handle */
	Str63			title;					/* window title */
	FSSpec			fs;
} ExtendedWindowRec, *ExtendedWindowPtr;

#define GetWindowIndex(w)				(((ExtendedWindowPtr)w)->windowIndex)
#define GetWindowWidth(w)				(((ExtendedWindowPtr)w)->windowWidth)
#define GetWindowHeight(w)				(((ExtendedWindowPtr)w)->windowHeight)
#define GetWindowType(w)				(((ExtendedWindowPtr)w)->windowType)
#define GetWindowDepth(w)				(((ExtendedWindowPtr)w)->windowDepth)
#define GetWindowMaxDepth(w)			(((ExtendedWindowPtr)w)->maxDepth)
#define WindowHasCloseBoxQQ(w)			(((ExtendedWindowPtr)w)->hasCloseBox)
#define WindowAutoCenterQQ(w)			(((ExtendedWindowPtr)w)->autoCenter)
#define WindowIsFloatQQ(w)				(((ExtendedWindowPtr)w)->isFloat)
#define WindowIsModifiedQQ(w)			(((ExtendedWindowPtr)w)->modified)
#define GetWindowTopLeft(w)				(((ExtendedWindowPtr)w)->initialTopLeft)
#define GetWindowBounds(w)				(((ExtendedWindowPtr)w)->windowBounds)
#define GetWindowVScrollBar(w)			(((ExtendedWindowPtr)w)->vScroll)
#define GetWindowHScrollBar(w)			(((ExtendedWindowPtr)w)->hScroll)
#define GetWindowTE(w)					(((ExtendedWindowPtr)w)->hTE)
#define GetWindowTitle(w)				(((ExtendedWindowPtr)w)->title)
#define GetWindowFS(w)					(((ExtendedWindowPtr)w)->fs)

#define SetWindowIndex(w,x)				{ ((ExtendedWindowPtr)w)->windowIndex=x; }
#define SetWindowWidth(w,x)				{ ((ExtendedWindowPtr)w)->windowWidth=x; }
#define SetWindowHeight(w,x)			{ ((ExtendedWindowPtr)w)->windowHeight=x; }
#define SetWindowType(w,x)				{ ((ExtendedWindowPtr)w)->windowType=x; }
#define SetWindowDepth(w,x)				{ ((ExtendedWindowPtr)w)->windowDepth=x; }
#define SetWindowMaxDepth(w,x)			{ ((ExtendedWindowPtr)w)->maxDepth=x; }
#define SetWindowHasCloseBox(w,x)		{ ((ExtendedWindowPtr)w)->hasCloseBox=x; }
#define SetWindowAutoCenter(w,x)		{ ((ExtendedWindowPtr)w)->autoCenter=x; }
#define SetWindowIsFloat(w,x)			{ ((ExtendedWindowPtr)w)->isFloat=x; }
#define SetWindowIsModified(w,x)		{ ((ExtendedWindowPtr)w)->modified=x; }
#define SetWindowTopLeft(w,x)			{ ((ExtendedWindowPtr)w)->initialTopLeft=x; }
#define SetWindowBounds(w,x)			{ ((ExtendedWindowPtr)w)->windowBounds=x; }
#define SetWindowVScrollBar(w,x)		{ ((ExtendedWindowPtr)w)->vScroll=x; }
#define SetWindowHScrollBar(w,x)		{ ((ExtendedWindowPtr)w)->hScroll=x; }
#define SetWindowTE(w,x)				{ ((ExtendedWindowPtr)w)->hTE=x; }
#define SetWindowTitle(w,x)				{ BlockMove(x, (((ExtendedWindowPtr)w)->title), x[0]+1); }
#define SetWindowFS(w,x)				{ ((ExtendedWindowPtr)w)->fs=x; }

#define IndWindowExistsQQ(x)		(GetIndWindowPtr(x)!=0L)
#define WindowExistsQQ(w)			(w!=0L)
#define FrontDocumentWindow()		GetFrontDocumentWindow()

extern	Boolean			gIgnoreNextActivateEvent;

/* necessary routines */
void InitTheWindowLayer(void);
void ShutDownTheWindowLayer(void);
WindowPtr MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon);
WindowPtr MyNewCWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon);
WindowPtr MyNewFloatWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon);
WindowPtr MyNewFloatCWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon);
void MyDisposeWindow(WindowPtr window);
Boolean MySelectWindow(WindowPtr window);

/* useful abstract routines for working with windows */
Boolean WindowHasLayer(WindowPtr window);
Boolean WindowIsFloat(WindowPtr window);
WindowPtr GetFrontDocumentWindow(void);
WindowPtr GetIndWindowPtr(short index);

/* useful patches */
void InstallHilitePatch(void);
void RemoveHilitePatch(void);

#endif
