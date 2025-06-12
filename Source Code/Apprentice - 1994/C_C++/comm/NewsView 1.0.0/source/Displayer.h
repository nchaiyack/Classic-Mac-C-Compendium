/* Displayer.h */

#ifndef Included_Displayer_h
#define Included_Displayer_h

/* Displayer module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* WindowDispatcher */
/* EventLoop */
/* Array */
/* StringList */
/* Files */
/* Memory */
/* Alert */
/* EditWindowRec */
/* Main */
/* DataMunging */

#include "Screen.h"
#include "EventLoop.h"

struct DisplayWindowRec;
typedef struct DisplayWindowRec DisplayWindowRec;

/* forwards */
struct MenuItemType;
struct EditWindowRec;

/* initialize the window crud */
MyBoolean						InitializeDisplayer(void);

/* clean up internal structures */
void								KillWindowStuff(void);

/* close all of the currently open things.  returns True if user didn't cancel. */
MyBoolean						DoCloseAllQuitPending(void);

/* open a new display */
void								OpenDisplayWindow(struct FileSpec* TheFile);

/* dispose of a window */
void								DisposeDisplayWindow(DisplayWindowRec* Window);

/* register a new article window */
MyBoolean						DisplayWindowNewArticleWindow(DisplayWindowRec* Window,
											struct EditWindowRec* NewWindow);

/* article window calls this when it is going bye-bye */
void								DisplayWindowArticleDead(DisplayWindowRec* Window,
											struct EditWindowRec* DeadWindow);

void								DisplayWindowDoIdle(DisplayWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								DisplayWindowBecomeActive(DisplayWindowRec* Window);
void								DisplayWindowBecomeInactive(DisplayWindowRec* Window);
void								DisplayWindowResized(DisplayWindowRec* Window);
void								DisplayWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, DisplayWindowRec* Window);
void								DisplayWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, DisplayWindowRec* Window);
void								DisplayWindowClose(DisplayWindowRec* Window);
void								DisplayWindowMenuSetup(DisplayWindowRec* Window);
void								DisplayWindowDoMenuCommand(DisplayWindowRec* Window,
											struct MenuItemType* MenuItem);
void								DisplayWindowDoUpdate(DisplayWindowRec* Window);

#endif
