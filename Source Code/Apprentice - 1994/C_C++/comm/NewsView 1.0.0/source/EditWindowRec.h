/* EditWindowRec.h */

#ifndef Included_EditWindowRec_h
#define Included_EditWindowRec_h

/* EditWindowRec module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* TextEdit */
/* Memory */
/* GrowIcon */
/* WindowDispatcher */
/* Menus */
/* Displayer */
/* DataMunging */
/* Numbers */
/* FindDialog */
/* NumberDialog */
/* Files */
/* Alert */

#include "Screen.h"
#include "EventLoop.h"

struct EditWindowRec;
typedef struct EditWindowRec EditWindowRec;

/* forward declarations */
struct MenuItemType;
struct DisplayWindowRec;

/* initialize internal static structures for edit window */
MyBoolean							InitializeEditWindow(void);

/* dispose of internal static structures for edit window */
void									ShutdownEditWindow(void);

/* create a new editing window */
EditWindowRec*				NewEditWindow(char* DataBlock, char* LineFeed, char* WindowName,
												struct DisplayWindowRec* Owner);

/* dispose of the editing window */
void									DisposeEditWindow(EditWindowRec* Window);

void									EditWindowDoIdle(EditWindowRec* Window,
												MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
												ModifierFlags Modifiers);
void									EditWindowBecomeActive(EditWindowRec* Window);
void									EditWindowBecomeInactive(EditWindowRec* Window);
void									EditWindowResized(EditWindowRec* Window);
void									EditWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
												ModifierFlags Modifiers, EditWindowRec* Window);
void									EditWindowDoKeyDown(unsigned char KeyCode,
												ModifierFlags Modifiers, EditWindowRec* Window);
void									EditWindowClose(EditWindowRec* Window);
void									EditWindowMenuSetup(EditWindowRec* Window);
void									EditWindowDoMenuCommand(EditWindowRec* Window,
												struct MenuItemType* MenuItem);
void									EditWindowDoUpdate(EditWindowRec* Window);

#endif
