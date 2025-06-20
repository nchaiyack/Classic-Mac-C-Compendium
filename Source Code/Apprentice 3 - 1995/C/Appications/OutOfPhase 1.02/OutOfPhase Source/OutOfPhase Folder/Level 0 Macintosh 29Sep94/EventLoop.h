/* EventLoop.h */

#ifndef Included_EventLoop_h
#define Included_EventLoop_h

/* EventLoop module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Menus */
/* Screen */
/* Memory */
/* MyMalloc (for debugging) */

#include "Screen.h"

/* these are the possible events that can occur.  note that only some of the */
/* parameters may be valid for certain events */
typedef enum
	{
		eNoEvent EXECUTE(= -4412), /* Window, X, Y, Modifiers valid; Window == active or NIL */
		eKeyPressed, /* Window, X, Y, Modifiers, KeyPressed valid */
		eMouseDown, /* Window, X, Y, Modifiers valid */
		eMouseUp, /* Window, X, Y, Modifiers valid */
		eCheckCursor, /* Window, X, Y, Modifiers valid; Window == active window or NIL */
		eMenuStarting, /* Window, Modifiers valid */
		eMenuCommand, /* Window, Modifiers, MenuCommand valid */
		eWindowClosing, /* Window valid */
		eWindowResized, /* Window valid */
		eActiveWindowChanged /* Window valid */
	} EventType;

/* keyboard modifier bits */
typedef enum
	{
		eNoModifiers = 0,
		eShiftKey = 1,
		eControlKey = 2,
		eCommandKey = 4,
		eOptionKey = 8,
		eCapsLockKey = 16,
		eMouseDownFlag = 32
	} ModifierFlags;

/* special key return values.  these values must not change */
#define eCancelKey (0x1b)
#define eLeftArrow (0x1c)
#define eRightArrow (0x1d)
#define eUpArrow (0x1e)
#define eDownArrow (0x1f)

/* forwards */
struct MenuItemType;

/* initialize internal event loop data structures */
MyBoolean					Eep_InitEventLoop(void);

/* dispose of any internal event loop structures */
void							Eep_ShutdownEventLoop(void);

/* Fetch an event from the event queue and return it.  Only some of the parameters */
/* returned may be valid; see the enumeration comments above for EventType to see */
/* which.  Mouse coordinates are always local to the current window, or undefined */
/* if there is no current window.  If there are no events, the routine will return */
/* after some amount of time.  This routine may call the Menu manager, so menus */
/* should be initialized before this routine is called.  Any parameter may be passed */
/* as NIL if the user doesn't care about the result.  Window changes do not occur */
/* if the mouse is down.  If the current window is a dialog box, then a window */
/* change will never be returned for another window.  Mouse up events are always */
/* returned with the same window as the mosue down event, even if the mouse is no */
/* longer in the window. */
EventType					GetAnEvent(OrdType* Xloc, OrdType* Yloc, ModifierFlags* Modifiers,
										WinType** Window, struct MenuItemType** MenuCommand, char* KeyPressed);

/* set the amount of time to wait for an event.  The default is 1/4 of a second */
/* time units are in seconds. */
double						SetEventSleepTime(double TheSleepTime);

/* allow other processes to run in a cooperative environment and test for the */
/* user cancel signal.  A return value of True means the user wants to cancel */
MyBoolean					RelinquishCPUCheckCancel(void);

/* allow other processes to run, but only a little bit.  Return value of True */
/* means the user wants to cancel */
MyBoolean					RelinquishCPUJudiciouslyCheckCancel(void);

/* read the system timer (in seconds).  The timer returns real time (not process */
/* time) but not relative to any known time.  The value may roll over from an */
/* undefined large number to 0 */
double						ReadTimer(void);

/* find the true difference between two timer values even if one has rolled over */
double						TimerDifference(double Now, double Then);

/* get the current mouse position, relative to top-left of current window.  If there */
/* is no current window, the results are undefined.  Either of the parameters can */
/* be NIL if the user doesn't care about the result. */
void							GetMousePosition(OrdType* Xloc, OrdType* Yloc);

/* read the state of the modifier keys on the keyboard.  On systems that don't */
/* allow this, the function may return the modifiers as they were at the last */
/* known time */
ModifierFlags			CheckModifiers(void);

/* set an implementation defined version of the specified cursor */
void							SetArrowCursor(void);
void							SetIBeamCursor(void);
void							SetWatchCursor(void);
void							SetCrossHairCursor(void);

/* set the cursor to the image and mask specified.  If the implementation's cursor */
/* is larger than 16x16, then the adjustment is implementation defined.  On the */
/* Macintosh, cursors are 16x16, so no adjustment is necessary */
/* the most significant bit of the word is leftmost */
void							SetTheCursor(short HotPointX, short HotPointY,
										unsigned short CursorImage[16], unsigned short CursorMask[16]);

/* get the number of seconds to wait before toggling an insertion point */
double						GetCursorBlinkRate(void);

/* get the maximum time between clicks for which they are considered a double click */
double						GetDoubleClickInterval(void);

/* emit a not too annoying beep to indicate an error occurred */
void							ErrorBeep(void);

/* enable or disable error beeping.  True enables it.  the old value is returned. */
MyBoolean					SetErrorBeepEnable(MyBoolean ShouldWeBeep);

/* this is an internal routine -- don't use */
void							Eep_WindowDying(WinType* Window);

#endif
