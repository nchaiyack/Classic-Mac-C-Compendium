/**********************************************************************\

File:		graphics.h

Purpose:	This is the header file for graphics.c

\**********************************************************************/

#include "QDOffscreen.h"

typedef struct
{
	int					windowIndex;			/* index of window in program's window list */
	int					windowWidth;			/* width of window content, in pixels */
	int					windowHeight;			/* height of window content, in pixels */
	int					windowType;				/* type of window (see IM Essentials, 4-80) */
	Boolean				hasCloseBox;			/* window has a close box */
	Boolean				offscreenNeedsUpdate;	/* TRUE if offscreen bitmap needs redrawing */
	Point				initialTopLeft;			/* initial window bounds when opened */
	Rect				windowBounds;			/* on screen rectangle of window content */
	Str31				windowTitle;			/* pascal string, title of window */
} WindowDataRec, *WindowDataPtr, **WindowDataHandle;

typedef int (*dispatchProcPtr)(WindowDataHandle theData, int theMessage, unsigned long misc);

typedef struct									/* exactly the same as WindowDataRec */
{												/* + dispatchProc at end */
	int					windowIndex;
	int					windowWidth;
	int					windowHeight;
	int					windowType;
	Boolean				hasCloseBox;
	Boolean				offscreenNeedsUpdate;
	Point				initialTopLeft;
	Rect				windowBounds;
	Str31				windowTitle;
	dispatchProcPtr		dispatchProc;			/* called with message of windowish event */
} ExtendedWindowDataRec, *ExtendedWindowDataPtr, **ExtendedWindowDataHandle;

enum					/* messages passed to window's dispatch procedure */
{
	kNull=0,			/* on null event when window is active, frgrnd/bkgrnd */
	kStartup,			/* on program startup */
	kShutdown,			/* on program shutdown */
	kInitialize,		/* just before window is created & shown */
	kOpen,				/* just after window is created & shown */
	kUpdate,			/* during window update -- draw contents to current grafport */
	kClose,				/* just before window is closed -- this can cancel close */
	kDispose,			/* just after window is closed/disposed */
	kActivate,			/* on window activate event */
	kDeactivate,		/* on window deactivate event */
	kSuspend,			/* on program suspension (switched into background) */
	kResume,			/* on program resuming (switching into foreground) */
	kKeydown,			/* on keydown event when window is active & in foreground */
	kMousedown			/* on mousedown event in window content when active & in frgrnd */
};

enum					/* return codes from window dispatch procedure */
{
	kSuccess=0,			/* message handled, no further processing please */
	kFailure,			/* message not handled, use default action if any */
	kCancel				/* message refused, cancel action (only good with kClose) */
};

/***************************************************************************************/

enum					/* window indices in gTheWindow[] and gTheWindowData[] lists */
{
	kAbout=0,			/* about box */
	kAboutMSG,			/* "About MSG" splash screen */
	kHelp,				/* help window */
	kClipboard			/* clipboard window */
};

#define		NUM_WINDOWS				4		/* total number of windows (see above enum) */

extern	WindowPtr		gTheWindow[NUM_WINDOWS];
extern	ExtendedWindowDataHandle
						gTheWindowData[NUM_WINDOWS];

Boolean InitTheGraphics(void);
void ShutDownTheGraphics(void);
void OpenTheWindow(int index);
void GetMainScreenBounds(void);
int GetWindowDepth(ExtendedWindowDataHandle theData);
void UpdateTheWindow(ExtendedWindowDataHandle theData);
Boolean CloseTheWindow(ExtendedWindowDataHandle theData);
void DrawThePicture(PicHandle *thePict, int whichPict, int x, int y);
void ReleaseThePict(PicHandle *thePict);
