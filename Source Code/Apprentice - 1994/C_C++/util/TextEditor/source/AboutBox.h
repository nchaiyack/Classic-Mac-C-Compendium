/* AboutBox.h */

#ifndef Included_AboutBox_h
#define Included_AboutBox_h

/* AboutBox depends on */
/* MiscInfo.h */
/* Debug */
/* Audit */
/* Definitions */
/* Screen */
/* EventLoop */
/* Menus */
/* WindowDispatcher */
/* Memory */
/* DataMunging */
/* Main */

#include "Screen.h"
#include "EventLoop.h"

struct AboutBoxWindowRec;
typedef struct AboutBoxWindowRec AboutBoxWindowRec;

/* forward declaration */
struct MenuItemType;

void					ShowAboutBox(void);
void					HideAnyAboutBoxes(void);

void					AboutBoxWindowDoIdle(AboutBoxWindowRec* Window,
								MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc);
void					AboutBoxWindowBecomeActive(AboutBoxWindowRec* Window);
void					AboutBoxWindowBecomeInactive(AboutBoxWindowRec* Window);
void					AboutBoxWindowJustResized(AboutBoxWindowRec* Window);
void					AboutBoxWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
								ModifierFlags Modifiers, AboutBoxWindowRec* Window);
void					AboutBoxWindowDoKeyDown(unsigned char KeyCode,
								ModifierFlags Modifiers, AboutBoxWindowRec* Window);
void					AboutBoxWindowClose(AboutBoxWindowRec* Window);
void					AboutBoxWindowUpdator(AboutBoxWindowRec* Window);
void					AboutBoxWindowMenuSetup(AboutBoxWindowRec* Window);
void					AboutBoxWindowDoMenuCommand(AboutBoxWindowRec* Window,
								struct MenuItemType* MenuItem);

#endif
