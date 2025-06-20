/* DisassemblyWindow.h */

#ifndef Included_DisassemblyWindow_h
#define Included_DisassemblyWindow_h

/* DisassemblyWindow module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* TextEdit */
/* EventLoop */
/* Menus */
/* WindowDispatcher */
/* Memory */
/* MainWindowStuff */
/* GrowIcon */
/* Main */
/* GlobalWindowMenuList */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"

struct DisaWindowRec;
typedef struct DisaWindowRec DisaWindowRec;

/* forward declarations */
struct MainWindowRec;

/* create a new disassembly window.  the window is basically a non-editable text */
/* field containing the Data passed in (linefeed = 0x0a).  the caller is responsible */
/* for disposing of Data.  the function automatically notifies the main window */
/* that it has been created. */
DisaWindowRec*			NewDisassemblyWindow(char* Data, struct MainWindowRec* Owner);

/* dispose of the disassembly window.  this automatically notifies the main window */
/* that it has been destroyed. */
void								DisposeDisassemblyWindow(DisaWindowRec* Window);

/* event handling routines for disassembly window */
void								DisassemblyWindowDoIdle(DisaWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								DisassemblyWindowBecomeActive(DisaWindowRec* Window);
void								DisassemblyWindowBecomeInactive(DisaWindowRec* Window);
void								DisassemblyWindowJustResized(DisaWindowRec* Window);
void								DisassemblyWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, DisaWindowRec* Window);
void								DisassemblyWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, DisaWindowRec* Window);
void								DisassemblyWindowClose(DisaWindowRec* Window);
void								DisassemblyWindowUpdator(DisaWindowRec* Window);
void								DisassemblyWindowMenuSetup(DisaWindowRec* Window);
void								DisassemblyWindowDoMenuCommand(DisaWindowRec* Window,
											MenuItemType* MenuItem);

#endif
