/* WindowStuff.h */

#ifndef Included_WindowStuff_h
#define Included_WindowStuff_h

/* WindowStuff module depends on */
/* MiscInfo.h */
/* Debug */
/* Audit */
/* Definitions */
/* Screen */
/* EventLoop */
/* WindowDispatcher */
/* TextEdit */
/* Files */
/* Array */
/* Memory */
/* Alert */
/* Menus */
/* DataMunging */
/* Main */
/* GrowIcon */
/* Numbers */
/* NumberDialog */
/* FindDialog */
/* StringDialog */

#include "Screen.h"
#include "EventLoop.h"

struct MyWinRec;
typedef struct MyWinRec MyWinRec;

/* forward declarations */
struct MenuItemType;
struct FileSpec;

MyBoolean						InitWindowStuff(void);
void								KillWindowStuff(void);

MyWinRec*						OpenDocument(struct FileSpec* Where);
MyBoolean						CloseDocument(MyWinRec* Window);
MyBoolean						DoCloseAllQuitPending(void);
MyBoolean						Save(MyWinRec* Window);
MyBoolean						SaveAs(MyWinRec* Window);
char*								GetProperStaticLineFeed(MyWinRec* Window);

void								WindowDoIdle(MyWinRec* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);
void								WindowBecomeActive(MyWinRec* Window);
void								WindowBecomeInactive(MyWinRec* Window);
void								WindowResized(MyWinRec* Window);
void								WindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, MyWinRec* Window);
void								WindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, MyWinRec* Window);
void								WindowClose(MyWinRec* Window);
void								WindowUpdate(MyWinRec* Window);
void								WindowMenuSetup(MyWinRec* Window);
void								WindowDoMenuCommand(MyWinRec* Window, struct MenuItemType* MenuItem);

void								FindAgain(MyWinRec* Window);
MyBoolean						Replace(MyWinRec* Window);
MyWinRec*						WhichWindowMenuItem(struct MenuItemType* TheItem);

#endif
