/* InstrWindow.h */

#ifndef Included_InstrWindow_h
#define Included_InstrWindow_h

/* InstrWindow module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* InstrObject */
/* MainWindowStuff */
/* InstrList */
/* TextEdit */
/* WindowDispatcher */
/* Memory */
/* GrowIcon */
/* Main */
/* FindDialog */
/* DataMunging */
/* GlobalWindowMenuList */

#include "Screen.h"
#include "EventLoop.h"

struct InstrWindowRec;
typedef struct InstrWindowRec InstrWindowRec;

/* forward declarations */
struct InstrObjectRec;
struct MainWindowRec;
struct InstrListRec;
struct MenuItemType;

/* create a new instrument specification editing window */
InstrWindowRec*			NewInstrWindow(struct InstrObjectRec* InstrObj,
											struct MainWindowRec* MainWindow,
											struct InstrListRec* InstrList, short WinX, short WinY,
											short WinWidth, short WinHeight);

/* write back modified data and dispose of the instrument window */
void								DisposeInstrWindow(InstrWindowRec* Window);

/* bring the window to the top and give it the focus */
void								BringInstrWindowToFront(InstrWindowRec* Window);

/* returns True if the data has been modified since the last file save. */
MyBoolean						HasInstrWindowBeenModified(InstrWindowRec* Window);

/* highlight the line in the instrument specification edit */
void								InstrWindowHilightLine(InstrWindowRec* Window, long LineNumber);

/* event handling routines */
void								InstrWindowDoIdle(InstrWindowRec* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);
void								InstrWindowBecomeActive(InstrWindowRec* Window);
void								InstrWindowBecomeInactive(InstrWindowRec* Window);
void								InstrWindowResized(InstrWindowRec* Window);
void								InstrWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, InstrWindowRec* Window);
void								InstrWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, InstrWindowRec* Window);
void								InstrWindowClose(InstrWindowRec* Window);
void								InstrWindowMenuSetup(InstrWindowRec* Window);
void								InstrWindowDoMenuCommand(InstrWindowRec* Window,
											struct MenuItemType* MenuItem);
void								InstrWindowUpdate(InstrWindowRec* Window);

/* get a copy of the instrument name */
char*								InstrWindowGetNameCopy(InstrWindowRec* Window);

/* get a copy of the instrument definition text */
char*								InstrWindowGetDefinitionCopy(InstrWindowRec* Window);

/* the filename has changed, so update the window title bar.  NewFilename is a */
/* non-null-terminated string which must be disposed by the caller. */
void								InstrWindowGlobalNameChange(InstrWindowRec* Window, char* NewFilename);

/* update the title bar of the window even if the filename hasn't changed */
void								InstrWindowResetTitlebar(InstrWindowRec* Window);

/* write back all modified data to the instrument object */
MyBoolean						InstrWindowWritebackModifiedData(InstrWindowRec* Window);

#endif
