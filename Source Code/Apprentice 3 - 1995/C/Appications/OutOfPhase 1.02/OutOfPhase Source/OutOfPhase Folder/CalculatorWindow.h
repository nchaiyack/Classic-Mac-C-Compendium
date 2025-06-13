/* CalculatorWindow.h */

#ifndef Included_CalculatorWindow_h
#define Included_CalculatorWindow_h

/* CalculatorWindow module depends on */
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
/* SimpleButton */
/* Alert */
/* CodeCenter */
/* PcodeStack */
/* PcodeSystem */
/* DataMunging */
/* Numbers */
/* FixedPoint */
/* GlobalWindowMenuList */
/* FunctionCode */
/* CompilerRoot */
/* PcodeDisassembly */
/* DisassemblyWindow */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"

struct CalcWindowRec;
typedef struct CalcWindowRec CalcWindowRec;

/* forward declarations */
struct CodeCenterRec;
struct MainWindowRec;


/* create a new calculator window.  the caller is responsible for registering the */
/* new calculator with the main window. */
CalcWindowRec*			NewCalculatorWindow(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter);

/* dispose of a calculator window.  the calculator notifies the main window that */
/* owns it. */
void								DisposeCalculatorWindow(CalcWindowRec* Window);

/* event handling routines for the calculator */
void								CalculatorWindowDoIdle(CalcWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								CalculatorWindowBecomeActive(CalcWindowRec* Window);
void								CalculatorWindowBecomeInactive(CalcWindowRec* Window);
void								CalculatorWindowJustResized(CalcWindowRec* Window);
void								CalculatorWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, CalcWindowRec* Window);
void								CalculatorWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, CalcWindowRec* Window);
void								CalculatorWindowClose(CalcWindowRec* Window);
void								CalculatorWindowUpdator(CalcWindowRec* Window);
void								CalculatorWindowMenuSetup(CalcWindowRec* Window);
void								CalculatorWindowDoMenuCommand(CalcWindowRec* Window,
											MenuItemType* MenuItem);

/* perform evaluation.  if there is a selection, then evaluate */
/* the selection, otherwise evaluate from last point */
void								CalculatorWindowDoCalculation(CalcWindowRec* Window);

#endif
