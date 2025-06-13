/* FunctionWindow.h */

#ifndef Included_FunctionWindow_h
#define Included_FunctionWindow_h

/* FunctionWindow module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* Menus */
/* MainWindowStuff */
/* FunctionObject */
/* TextEdit */
/* WindowDispatcher */
/* Memory */
/* GrowIcon */
/* DataMunging */
/* Main */
/* DisassemblyWindow */
/* Alert */
/* FunctionList */
/* FindDialog */
/* GlobalWindowMenuList */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"

struct FunctionWindowRec;
typedef struct FunctionWindowRec FunctionWindowRec;

/* forward declarations */
struct MainWindowRec;
struct FunctionObjectRec;
struct FunctionListRec;

/* create a new function editing window */
FunctionWindowRec*	NewFunctionWindow(struct MainWindowRec* MainWindow,
											struct FunctionObjectRec* FunctionObject,
											struct FunctionListRec* FunctionList, OrdType WinX, OrdType WinY,
											OrdType WinWidth, OrdType WinHeight);

/* dispose of the function editing window */
void								DisposeFunctionWindow(FunctionWindowRec* Window);

/* dispatch callback for handling idle events */
void								FunctionWindowDoIdle(FunctionWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);

/* dispatch callback that's called when our window comes to the top */
void								FunctionWindowBecomeActive(FunctionWindowRec* Window);

/* dispatch callback that's called when our window is no longer on top */
void								FunctionWindowBecomeInactive(FunctionWindowRec* Window);

/* dispatch callback to handle window resized event (change sizes of things */
/* that the window contains) */
void								FunctionWindowJustResized(FunctionWindowRec* Window);

/* dispatch callback to handle mouse down events */
void								FunctionWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, FunctionWindowRec* Window);

/* dispatch callback to handle key down events */
void								FunctionWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, FunctionWindowRec* Window);

/* dispatch callback to handle click in the close box */
void								FunctionWindowClose(FunctionWindowRec* Window);

/* screen callback for redrawing window */
void								FunctionWindowUpdator(FunctionWindowRec* Window);

/* menu callback for enabling menu items */
void								FunctionWindowMenuSetup(FunctionWindowRec* Window);

/* dispatch callback for executing a menu item the user has chosen */
void								FunctionWindowDoMenuCommand(FunctionWindowRec* Window,
											MenuItemType* MenuItem);

/* utility routine to hilite a text line on which a compile error has occurred */
void								FunctionWindowHiliteLine(FunctionWindowRec* Window, long ErrorLine);

/* utility routine to bring this window to the top. */
void								FunctionWindowBringToTop(FunctionWindowRec* Window);

/* check to see if data in text edit boxes has been altered */
MyBoolean						HasFunctionWindowBeenModified(FunctionWindowRec* Window);

/* get a copy of the name edit */
char*								FunctionWindowGetNameCopy(FunctionWindowRec* Window);

/* get a copy of the source text edit */
char*								FunctionWindowGetSourceCopy(FunctionWindowRec* Window);

/* the name of the document has changed, so change the name of the window */
void								FunctionWindowGlobalNameChange(FunctionWindowRec* Window,
											char* NewFilename);

/* refresh the titlebar of the window */
void								FunctionWindowResetTitlebar(FunctionWindowRec* Window);

/* force the function window to write back to the object any data that has changed */
MyBoolean						FunctionWindowWritebackModifiedData(FunctionWindowRec* Window);

#endif
