/* FunctionList.h */

#ifndef Included_FunctionList_h
#define Included_FunctionList_h

/* FunctionList module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* StringList */
/* Memory */
/* CodeCenter */
/* Array */
/* FunctionObject */
/* Alert */
/* DataMunging */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */
/* Files */
/* Scrap */

#include "Screen.h"
#include "EventLoop.h"
#include "MainWindowStuff.h"

struct FunctionListRec;
typedef struct FunctionListRec FunctionListRec;

/* forward declarations */
struct FunctionObjectRec;
struct MainWindowRec;
struct CodeCenterRec;
struct BufferedInputRec;
struct BufferedOutputRec;
struct FileType;

/* create a new function list */
FunctionListRec*		NewFunctionList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* delete the function list and all of the function modules it contains */
void								DisposeFunctionList(FunctionListRec* FuncList);

/* change the location of the function list in the window */
void								SetFunctionListLocation(FunctionListRec* FuncList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* redraw the list */
void								FunctionListRedraw(FunctionListRec* FuncList);

/* see if the specified coordinates falls inside the function list rectangle */
MyBoolean						FunctionListHitTest(FunctionListRec* FuncList,
											OrdType XLoc, OrdType YLoc);

/* handle a mouse down event for the function list */
void								FunctionListDoMouseDown(FunctionListRec* FuncList, OrdType XLoc,
											OrdType YLoc, ModifierFlags Modifiers);

/* called when the window becomes active */
void								FunctionListBecomeActive(FunctionListRec* FuncList);

/* called when the window becomes inactive */
void								FunctionListBecomeInactive(FunctionListRec* FuncList);

/* called when a selection is made in another list, so that this list */
/* is deselected */
void								FunctionListDeselect(FunctionListRec* FuncList);

/* check to see if there is a selection in this list */
MyBoolean						FunctionListIsThereSelection(FunctionListRec* FuncList);

/* check to see if any of the function modules contained in this list need */
/* to be saved */
MyBoolean						DoesFunctionListNeedToBeSaved(FunctionListRec* FuncList);

/* open an edit window for the selected function module */
void								FunctionListOpenSelection(FunctionListRec* FuncList);

/* create a new function module and open a window for it */
void								FunctionListNewModule(FunctionListRec* FuncList);

/* delete the selected function module */
void								FunctionListDeleteSelection(FunctionListRec* FuncList);

/* delete the explicitly specified function module */
void								FunctionListDeleteFunction(FunctionListRec* FuncList,
											struct FunctionObjectRec* TheFunctionModule);

/* the name of a function module has changed, so the name in the scrolling */
/* list must also be changed */
void								FunctionListFunctionNameChanged(FunctionListRec* FuncList,
											struct FunctionObjectRec* TheFunctionModule);

/* remove all object code for all function modules */
void								FunctionListUnbuildAll(FunctionListRec* FuncList);

/* build all functions.  returns True if successful. */
MyBoolean						FunctionListMakeUpToDate(FunctionListRec* FuncList);

/* the document's name has changed, so update all windows */
void								FunctionListGlobalNameChange(FunctionListRec* FuncList,
											char* NewFilename);

/* read function objects from a file.  returns True if completely successful. */
FileLoadingErrors		FunctionListReadData(FunctionListRec* FuncList,
											struct BufferedInputRec* Input);

/* write function objects to a file.  returns True if completely successful. */
FileLoadingErrors		FunctionListWriteData(FunctionListRec* FuncList,
											struct BufferedOutputRec* Output);

/* after a file has been saved, this is called to mark all objects as not modified. */
void								FunctionListMarkAllObjectsSaved(FunctionListRec* FuncList);

/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						FunctionListCopyObject(FunctionListRec* FuncList);

/* try to paste the clipboard in as a function object.  returns False if it failed */
/* or the clipboard did not contain a function object. */
MyBoolean						FunctionListPasteObject(FunctionListRec* FuncList);

/* try to paste a function object in from a file */
MyBoolean						FunctionListPasteFromFile(FunctionListRec* FuncList,
											struct FileType* File);

#endif
