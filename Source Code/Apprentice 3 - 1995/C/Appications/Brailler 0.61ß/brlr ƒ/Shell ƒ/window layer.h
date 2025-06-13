#ifndef __WINDOW_LAYER_H__
#define __WINDOW_LAYER_H__

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
	kActivateWindow				= true,
	kDeactivateWindow			= false
};

#if powerc
#pragma options align=mac68k
#endif
struct ExtendedWindowRec
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
	Boolean			isPrintable;			/* window can print */
	Boolean			draggable;				/* window supports drag manager */
	Boolean			wasVisible;				/* window was visible before suspending */
	Point			initialTopLeft;			/* initial window bounds when opened */
	Rect			windowBounds;			/* on screen rectangle of window content */
	ControlHandle	vScroll;				/* window's vertical scroll bar */
	ControlHandle	hScroll;				/* window's horizontal scroll bar */
	TEHandle		hTE;					/* window's TextEdit handle */
	Str63			title;					/* window title */
	FSSpec			fs;						/* FSSpec for disk file associated w/window data */
	RgnHandle		hiliteRgn;				/* used by Drag Manager */
	TEClickLoopUPP	oldClickLoopProc;		/* default click proc */
	short			lastFindPosition;		/* last find position if window is searchable */
};
#if powerc
#pragma options align=reset
#endif

typedef struct ExtendedWindowRec ExtendedWindowRec, *ExtendedWindowPtr;

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
	
	kApplicationFloaterKind = 7
};

#define GetWindowKind(w)				(((WindowPeek)w)->windowKind)
#define GetWindowVisible(w)				(((ExtendedWindowPtr)w)->theWindow.visible)
#define GetWindowHilite(w)				(((ExtendedWindowPtr)w)->theWindow.hilited)
#define GetNextWindow(w)				(WindowRef)(((ExtendedWindowPtr)w)->theWindow.nextWindow)
#define GetContentRegion(w)				(((ExtendedWindowPtr)w)->theWindow.contRgn)
#define GetStructureRegion(w)			(((ExtendedWindowPtr)w)->theWindow.strucRgn)
#define GetWindowDefProc(w)				(((ExtendedWindowPtr)w)->theWindow.windowDefProc)
#define GetWasVisible(w)				(((ExtendedWindowPtr)w)->wasVisible)

#define GetWindowAttributes(w)			(((ExtendedWindowPtr)w)->attrs)
#define GetWindowMagic(w)				(((ExtendedWindowPtr)w)->magic)
#define GetWindowIndex(w)				(((ExtendedWindowPtr)w)->windowIndex)
#define GetWindowWidth(w)				(((ExtendedWindowPtr)w)->windowWidth)
#define GetWindowHeight(w)				(((ExtendedWindowPtr)w)->windowHeight)
#define GetWindowDepth(w)				(((ExtendedWindowPtr)w)->windowDepth)
#define GetWindowMaxDepth(w)			(((ExtendedWindowPtr)w)->maxDepth)
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
#define GetWindowHiliteRgn(w)			(((ExtendedWindowPtr)w)->hiliteRgn)
#define GetWindowOldClickLoopProc(w)	(((ExtendedWindowPtr)w)->oldClickLoopProc)
#define WindowIsDraggableQQ(w)			(((ExtendedWindowPtr)w)->draggable)
#define WindowIsActiveQQ(w)				(((ExtendedWindowPtr)w)->active)
#define GetWindowLastFindPosition(w)	(((ExtendedWindowPtr)w)->lastFindPosition)
#define WindowDrawGrowIconLinesQQ(w)	(((ExtendedWindowPtr)w)->drawGrowIconLines)
#define WindowIsPrintableQQ(w)			(((ExtendedWindowPtr)w)->isPrintable)

#define SetWindowKind(w,x)				{ ((WindowPeek)w)->windowKind=x; }
#define SetWindowVisible(w,x)			{ ((ExtendedWindowPtr)w)->theWindow.visible=x; }
#define SetWindowHilite(w,x)			{ ((ExtendedWindowPtr)w)->theWindow.hilited=x; }
#define SetNextWindow(w,x)				{ ((ExtendedWindowPtr)w)->theWindow.nextWindow=(WindowPeek)x; }
#define SetWasVisible(w,x)				{ ((ExtendedWindowPtr)w)->wasVisible=x; }

#define SetWindowAttributes(w,x)		{ ((ExtendedWindowPtr)w)->attrs=x; }
#define SetWindowMagic(w,x)				{ ((ExtendedWindowPtr)w)->magic=x; }
#define SetWindowIndex(w,x)				{ ((ExtendedWindowPtr)w)->windowIndex=x; }
#define SetWindowWidth(w,x)				{ ((ExtendedWindowPtr)w)->windowWidth=x; }
#define SetWindowHeight(w,x)			{ ((ExtendedWindowPtr)w)->windowHeight=x; }
#define SetWindowDepth(w,x)				{ ((ExtendedWindowPtr)w)->windowDepth=x; }
#define SetWindowMaxDepth(w,x)			{ ((ExtendedWindowPtr)w)->maxDepth=x; }
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
#define SetWindowHiliteRgn(w,x)			{ ((ExtendedWindowPtr)w)->hiliteRgn=x; }
#define SetWindowOldClickLoopProc(w,x)	{ ((ExtendedWindowPtr)w)->oldClickLoopProc=x; }
#define SetWindowIsDraggable(w,x)		{ ((ExtendedWindowPtr)w)->draggable=x; }
#define SetWindowIsActive(w,x)			{ ((ExtendedWindowPtr)w)->active=x; }
#define SetWindowLastFindPosition(w,x)	{ ((ExtendedWindowPtr)w)->lastFindPosition=x; }
#define SetWindowDrawGrowIconLines(w,x)	{ ((ExtendedWindowPtr)w)->drawGrowIconLines=x; };
#define SetWindowIsPrintable(w,x)		{ ((ExtendedWindowPtr)w)->isPrintable=x; }

#define IndWindowExistsQQ(x)		(GetIndWindowPtr(x)!=0L)
#define WindowExistsQQ(w)			(w!=0L)
#define FrontDocumentWindow()		GetFrontDocumentWindow()
#define WindowIsModal(w)			WindowIsModalQQ(w)
#define FrontNonFloatingWindow()	GetFrontDocumentWindow()
#define LastFloatingWindow()		GetLastFloatingWindow()

#ifdef __cplusplus
extern "C" {
#endif

extern	pascal WindowRef MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, WindowAttributes attributes, WindowRef behind, long refCon, Boolean isFloat);
extern	pascal void MyDisposeWindow(WindowRef windowReference);
extern	pascal Boolean MySelectWindow(WindowRef windowToSelect);
extern	pascal void MyShowWindow(WindowRef windowToShow);
extern	pascal void MyHideWindow(WindowRef windowToHide);
extern	pascal void MyDragWindow(WindowRef windowToDrag, Point startPoint, const Rect *draggingBounds);
extern	pascal WindowRef GetFrontDocumentWindow(void);
extern	pascal WindowRef GetLastFloatingWindow(void);
extern	pascal Boolean WindowIsModalQQ(WindowRef windowReference);
extern	pascal void DeactivateFloatersAndFirstDocumentWindow(void);
extern	pascal void ActivateFloatersAndFirstDocumentWindow(void);
extern	pascal void HideOneFloatingWindow(WindowRef floatingWindow);
extern	pascal void ShowOneFloatingWindow(WindowRef floatingWindow);
extern	pascal void SuspendFloatingWindows(void);
extern	pascal void ResumeFloatingWindows(void);
extern	pascal void ValidateWindowList(void);
extern	pascal void GetWindowPortRect(WindowRef windowReference, Rect *portRect);
extern	pascal Boolean WindowHasLayer(WindowRef theWindow);
extern	pascal Boolean WindowIsFloat(WindowRef theWindow);
extern	pascal WindowRef GetIndWindowPtr(short index);

#ifdef __cplusplus
}
#endif

#endif
