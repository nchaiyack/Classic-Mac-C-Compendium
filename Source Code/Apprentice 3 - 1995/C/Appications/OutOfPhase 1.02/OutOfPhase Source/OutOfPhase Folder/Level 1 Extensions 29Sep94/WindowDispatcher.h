/* WindowDispatcher.h */

#ifndef Included_WindowDispatcher_h
#define Included_WindowDispatcher_h

/* WindowDispatcher module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* Menus */
/* Array */
/* Memory */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"

struct GenericWindowRec;
typedef struct GenericWindowRec GenericWindowRec;

/* initialize internal data structures for the window dispatch handler */
MyBoolean						InitializeWindowDispatcher(void);

/* dispose of internal data structures for the window dispatch handler */
void								ShutdownWindowDispatcher(void);

/* check in a new window, install callback routines, and obtain a unique identifier */
GenericWindowRec*		CheckInNewWindow(WinType* ScreenID, void* Reference,
											void (*WindowDoIdle)(void* Reference, MyBoolean CheckCursorFlag,
														OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers),
											void (*WindowBecomeActive)(void* Reference),
											void (*WindowBecomeInactive)(void* Reference),
											void (*WindowResized)(void* Reference),
											void (*WindowDoMouseDown)(OrdType XLoc, OrdType YLoc,
														ModifierFlags Modifiers, void* Reference),
											void (*WindowDoKeyDown)(unsigned char KeyCode,
														ModifierFlags Modifiers, void* Reference),
											void (*WindowClose)(void* Reference),
											void (*WindowMenuSetup)(void* Reference),
											void (*WindowDoMenuCommand)(void* Reference,
														MenuItemType* MenuItem));

/* notify the window dispatcher that this window is no longer with us */
void								CheckOutDyingWindow(GenericWindowRec* Window);

/* get the identifier for the currently active window.  returns NIL */
/* if there is no active window */
GenericWindowRec*		GetCurrentWindowID(void);

/* dispatch one of the specified events to the currently active window */
/* Pass NIL for Window if there is no active window */
void								DispatchDoIdle(WinType* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);
void								DispatchActiveWindowJustChanged(WinType* NewWindow);
void								DispatchWindowJustResized(WinType* Window);
void								DispatchDoMouseDown(WinType* Window, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								DispatchDoKeyDown(WinType* Window, unsigned char KeyCode,
											ModifierFlags Modifiers);
void								DispatchCloseWindow(WinType* Window);
void								DispatchMenuStarting(WinType* Window);
void								DispatchProcessMenuCommand(WinType* Window, MenuItemType* MenuItem);

struct IdleTaskSignature;
typedef struct IdleTaskSignature IdleTaskSignature;

/* install an idle task to be executed whenever an idle event occurs */
IdleTaskSignature*	DispatchInstallIdleTask(void (*Proc)(void* Refcon), void* Refcon);

/* remove an idle task */
void								DispatchRemoveIdleTask(IdleTaskSignature* Signature);

/* dispatch a callback to all installed idle tasks */
void								DispatchIdleTaskCallback(void);

#endif
