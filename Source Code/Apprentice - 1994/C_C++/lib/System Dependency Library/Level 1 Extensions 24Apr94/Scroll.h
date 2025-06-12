/* Scroll.h */

#ifndef Included_Scroll_h
#define Included_Scroll_h

/* Scroll module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* EventLoop */

#include "Screen.h"
#include "EventLoop.h"

struct ScrollRec;
typedef struct ScrollRec ScrollRec;

/* what direction does the scrollbar go */
typedef enum {eVScrollBar, eHScrollBar} ScrollBarType;

/* these opcodes are passed to the scroll hook procedure */
typedef enum
	{
		eScrollToPosition,
		eScrollPageMinus,
		eScrollPagePlus,
		eScrollLineMinus,
		eScrollLinePlus
	} ScrollType;

/* create a new scroll bar, returning a Ptr to the private data structure. */
/* Length is the total number of pixels long the scroll bar is */
/* The scrollbar is initially in a DISABLED state. */
ScrollRec*			NewScrollBar(WinType* TheWindow, ScrollBarType Kind,
									OrdType X, OrdType Y, OrdType Length);

/* dispose the scroll bar record allocated by NewScrollBar */
void						DisposeScrollBar(ScrollRec* TheBar);

/* get the position on the screen of the scroll bar */
OrdType					GetScrollXPosition(ScrollRec* TheBar);
OrdType					GetScrollYPosition(ScrollRec* TheBar);
OrdType					GetScrollLength(ScrollRec* TheBar);

/* set the position of the scroll bar on the screen */
void						SetScrollLocation(ScrollRec* TheBar, OrdType NewX, OrdType NewY,
									OrdType NewLength);

/* get the maximum number of indices the scroll bar can have */
long						GetMaxScrollIndex(ScrollRec* TheBar);

/* get the current index for the scroll bar's position */
long						GetCurrentScrollIndex(ScrollRec* TheBar);

/* set the maximum number of indices the scroll bar can have */
void						SetMaxScrollIndex(ScrollRec* TheBar, long NewMaxScrollIndex);

/* set the current index; NewIndex may be out of range */
void						SetScrollIndex(ScrollRec* TheBar, long NewIndex);

/* enable or disable the scroll bars (for inactive windows) */
void						EnableScrollBar(ScrollRec* TheBar);
void						DisableScrollBar(ScrollRec* TheBar);

/* redraw the scroll bar normally */
void						RedrawScrollBar(ScrollRec* TheBar);

/* call this when the mouse goes down in the scroll bar, providing the */
/* information which came with the mouse down event.  ScrollHook is a routine */
/* which actually performs the scrolling: */
/* If How == eScrollToPosition, then scroll and redraw the image at the index */
/* represented by Parameter */
/* If How == eScrollPageMinus or eScrollPagePlus, then scroll as much of the */
/* image is necessary to scroll a page worth */
void						ScrollHitProc(ScrollRec* TheBar, ModifierFlags Modifiers,
									OrdType X, OrdType Y, void* Refcon,
									void (*ScrollHook)(long Parameter, ScrollType How, void* Refcon));

/* see if the position is in the scrollbar box */
MyBoolean				ScrollHitTest(ScrollRec* TheBar, OrdType X, OrdType Y);

#endif
