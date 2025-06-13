/* AlgoSampWindow.h */

#ifndef Included_AlgoSampWindow_h
#define Included_AlgoSampWindow_h

/* AlgoSampWindow module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* Menus */
/* MainWindowStuff */
/* AlgoSampObject */
/* TextEdit */
/* IconButton */
/* WindowDispatcher */
/* Memory */
/* Alert */
/* Numbers */
/* GrowIcon */
/* DataMunging */
/* Main */
/* AlgoSampList */
/* SampleConsts */
/* EditImages */
/* FindDialog */
/* GlobalWindowMenuList */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"
#include "SampleConsts.h"

struct AlgoSampWindowRec;
typedef struct AlgoSampWindowRec AlgoSampWindowRec;

/* forward declarations */
struct MainWindowRec;
struct AlgoSampObjectRec;
struct AlgoSampListRec;

/* allocate a new algorithmic sample window */
AlgoSampWindowRec*	NewAlgoSampWindow(struct MainWindowRec* MainWindow,
											struct AlgoSampObjectRec* AlgoSampObject,
											struct AlgoSampListRec* AlgoSampList, OrdType WinX, OrdType WinY,
											OrdType WinWidth, OrdType WinHeight);

/* write back modified data and dispose of the window */
void								DisposeAlgoSampWindow(AlgoSampWindowRec* Window);

/* return True if the data in the window has been changed since the last */
/* time the file was saved. */
MyBoolean						HasAlgoSampWindowBeenModified(AlgoSampWindowRec* Window);

/* highlight the specified line number in the formula window. */
void								AlgoSampWindowHiliteLine(AlgoSampWindowRec* Window,
											long LineNumber);

/* bring the window to the top and give it the focus */
void								AlgoSampWindowBringToTop(AlgoSampWindowRec* Window);

/* event handling routines */
void								AlgoSampWindowDoIdle(AlgoSampWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								AlgoSampWindowBecomeActive(AlgoSampWindowRec* Window);
void								AlgoSampWindowBecomeInactive(AlgoSampWindowRec* Window);
void								AlgoSampWindowJustResized(AlgoSampWindowRec* Window);
void								AlgoSampWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, AlgoSampWindowRec* Window);
void								AlgoSampWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers,AlgoSampWindowRec* Window);
void								AlgoSampWindowClose(AlgoSampWindowRec* Window);
void								AlgoSampWindowUpdator(AlgoSampWindowRec* Window);
void								AlgoSampWindowMenuSetup(AlgoSampWindowRec* Window);
void								AlgoSampWindowDoMenuCommand(AlgoSampWindowRec* Window,
											MenuItemType* MenuItem);

/* get a copy of the name of the sample. */
char*								AlgoSampWindowGetNameCopy(AlgoSampWindowRec* Window);

/* get a copy of the sample's formula */
char*								AlgoSampWindowGetFormulaCopy(AlgoSampWindowRec* Window);

/* get the origin point of the sample */
long								AlgoSampWindowGetOrigin(AlgoSampWindowRec* Window);

/* get the first loop start point of the sample */
long								AlgoSampWindowGetLoopStart1(AlgoSampWindowRec* Window);

/* get the second loop start point of the sample */
long								AlgoSampWindowGetLoopStart2(AlgoSampWindowRec* Window);

/* get the third loop start point of the sample */
long								AlgoSampWindowGetLoopStart3(AlgoSampWindowRec* Window);

/* get the first loop end point of the sample */
long								AlgoSampWindowGetLoopEnd1(AlgoSampWindowRec* Window);

/* get the second loop end point of the sample */
long								AlgoSampWindowGetLoopEnd2(AlgoSampWindowRec* Window);

/* get the third loop end point of the sample */
long								AlgoSampWindowGetLoopEnd3(AlgoSampWindowRec* Window);

/* get the sampling rate */
long								AlgoSampWindowGetSamplingRate(AlgoSampWindowRec* Window);

/* get the natural pitch for playback conversion */
double							AlgoSampWindowGetNaturalFrequency(AlgoSampWindowRec* Window);

/* get the number of bits that the sample uses */
NumBitsType					AlgoSampWindowGetNumBits(AlgoSampWindowRec* Window);

/* get the number of channels that the sample uses */
NumChannelsType			AlgoSampWindowGetNumChannels(AlgoSampWindowRec* Window);

/* the name of the document has changed, so change the title bar.  the caller is */
/* responsible for disposing of the string, which must be non-null-terminated */
void								AlgoSampWindowGlobalNameChange(AlgoSampWindowRec* Window,
											char* NewFilename);

/* update the name in the title bar even if the document name hasn't changed. */
void								AlgoSampWindowResetTitlebar(AlgoSampWindowRec* Window);

/* write back all modified data to the algosamp object */
MyBoolean						AlgoSampWindowWritebackModifiedData(AlgoSampWindowRec* Window);

#endif
