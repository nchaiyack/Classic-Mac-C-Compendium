/* AlgoWaveTableWindow.h */

#ifndef Included_AlgoWaveTableWindow_h
#define Included_AlgoWaveTableWindow_h

/* AlgoWaveTableWindow module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* SampleConsts */
/* Screen */
/* EventLoop */
/* Menus */
/* MainWindowStuff */
/* AlgoWaveTableObject */
/* AlgoWaveTableList */
/* TextEdit */
/* IconButton */
/* WindowDispatcher */
/* Memory */
/* Alert */
/* Numbers */
/* GrowIcon */
/* DataMunging */
/* Main */
/* EditImages */
/* FindDialog */
/* WaveTableSizeDialog */
/* GlobalWindowMenuList */

#include "SampleConsts.h"
#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"

struct AlgoWaveTableWindowRec;
typedef struct AlgoWaveTableWindowRec AlgoWaveTableWindowRec;

/* forward declarations */
struct MainWindowRec;
struct AlgoWaveTableObjectRec;
struct AlgoWaveTableListRec;

/* create a new algorithmic wave table editing window */
AlgoWaveTableWindowRec*	NewAlgoWaveTableWindow(struct MainWindowRec* MainWindow,
											struct AlgoWaveTableObjectRec* AlgoWaveTableObject,
											struct AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType WinX, OrdType WinY, OrdType WinWidth, OrdType WinHeight);

/* write data back to the object and dispose of the window */
void								DisposeAlgoWaveTableWindow(AlgoWaveTableWindowRec* Window);

/* find out if the wave table has been modified since the last file save */
MyBoolean						HasAlgoWaveTableWindowBeenModified(AlgoWaveTableWindowRec* Window);

/* highlight a line in the formula edit box */
void								AlgoWaveTableWindowHiliteLine(AlgoWaveTableWindowRec* Window,
											long LineNumber);

/* bring the window to the top and give it the focus */
void								AlgoWaveTableWindowBringToTop(AlgoWaveTableWindowRec* Window);

/* event handling routines */
void								AlgoWaveTableWindowDoIdle(AlgoWaveTableWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								AlgoWaveTableWindowBecomeActive(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowBecomeInactive(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowJustResized(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers,AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowClose(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowUpdator(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowMenuSetup(AlgoWaveTableWindowRec* Window);
void								AlgoWaveTableWindowDoMenuCommand(AlgoWaveTableWindowRec* Window,
											MenuItemType* MenuItem);

/* get a copy of the algorithmic wave table's name */
char*								AlgoWaveTableWindowGetNameCopy(AlgoWaveTableWindowRec* Window);

/* get a copy of the algorithmic wave table's formula */
char*								AlgoWaveTableWindowGetFormulaCopy(AlgoWaveTableWindowRec* Window);

/* get the number of bits used for this algorithmic wave table */
NumBitsType					AlgoWaveTableWindowGetNumBits(AlgoWaveTableWindowRec* Window);

/* get the number of periods in the table */
long								AlgoWaveTableWindowGetNumTables(AlgoWaveTableWindowRec* Window);

/* get the number of frames in each period */
long								AlgoWaveTableWindowGetNumFrames(AlgoWaveTableWindowRec* Window);

/* the name of the file has changed, so update the title bar of the window.  the */
/* NewFilename is a non-null-terminated string which must be disposed by the caller */
void								AlgoWaveTableWindowGlobalNameChange(AlgoWaveTableWindowRec* Window,
											char* NewFilename);

/* reset the title bar name of the window even if the filename hasn't changed */
void								AlgoWaveTableWindowResetTitlebar(AlgoWaveTableWindowRec* Window);

/* write back all modified data to the object */
MyBoolean						AlgoWaveTableWindowWritebackModifiedData(AlgoWaveTableWindowRec* Window);

#endif
