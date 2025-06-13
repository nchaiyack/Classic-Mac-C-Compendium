/* AboutBox.h */

#ifndef Included_AboutBox_h
#define Included_AboutBox_h

/* AboutBox depends on */
/* MiscInfo.h */
/* Definitions */
/* Debug */
/* Audit */
/* Screen */
/* EventLoop */
/* Menus */
/* TextEdit */
/* WindowDispatcher */
/* Memory */
/* DataMunging */
/* Main */

#include "Screen.h"
#include "Menus.h"
#include "EventLoop.h"

struct AboutBoxWindowRec;
typedef struct AboutBoxWindowRec AboutBoxWindowRec;

/* show the about box.  this creates a new box if there isn't one, or raises the */
/* window to the top if there already is one. */
void					ShowAboutBox(void);

/* close the about box.  does nothing if no about box is open */
void					HideAnyAboutBoxes(void);

#endif
