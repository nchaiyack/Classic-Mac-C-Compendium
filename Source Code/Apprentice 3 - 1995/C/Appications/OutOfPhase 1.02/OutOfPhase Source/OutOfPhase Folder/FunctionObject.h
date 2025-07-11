/* FunctionObject.h */

#ifndef Included_FunctionObject_h
#define Included_FunctionObject_h

/* FunctionObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* DataMunging */
/* CodeCenter */
/* Numbers */
/* Alert */
/* Array */
/* PcodeDisassembly */
/* CompilerRoot */
/* FunctionCode */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "MainWindowStuff.h"

struct FunctionObjectRec;
typedef struct FunctionObjectRec FunctionObjectRec;

/* forward declarations */
struct CodeCenterRec;
struct MainWindowRec;
struct FunctionListRec;
struct BufferedInputRec;
struct BufferedOutputRec;

/* create a new empty function object */
FunctionObjectRec*	NewFunctionObject(struct CodeCenterRec* CodeCenter,
											struct MainWindowRec* MainWindow,
											struct FunctionListRec* FuncList);

/* get rid of the function object */
void								DisposeFunctionObject(FunctionObjectRec* FuncObj);

/* inquire whether the function has been modified */
MyBoolean						HasFunctionObjectBeenModified(FunctionObjectRec* FuncObj);

/* build the function module code.  returns True if successful. */
MyBoolean						FunctionObjectBuild(FunctionObjectRec* FuncObj);

/* unconditionally unbuild the functions */
void								FunctionObjectUnbuild(FunctionObjectRec* FuncObj);

/* build the functions if necessary.  return True if successful. */
MyBoolean						FunctionObjectMakeUpToDate(FunctionObjectRec* FuncObj);

/* return a text block containing a disassembly of the function module */
char*								FunctionObjectDisassemble(FunctionObjectRec* FuncObj);

/* get a copy of the name of this function module, no null termination */
char*								FunctionObjectGetNameCopy(FunctionObjectRec* FuncObj);

/* open the window for this function */
MyBoolean						FunctionObjectOpenWindow(FunctionObjectRec* FuncObj);

/* get the text for this function */
char*								FunctionObjectGetSourceCopy(FunctionObjectRec* FuncObj);

/* install new data in the object.  this is used when the editor window closes, */
/* to put the new data into this object.  the object becomes the owner of Data, */
/* so the caller should not release it. */
void								FunctionObjectNewSource(FunctionObjectRec* FuncObj, char* Data);

/* give function module a new name.  the object becomes the owner of Name, so */
/* the caller should not release it. */
void								FunctionObjectNewName(FunctionObjectRec* FuncObj, char* Name);

/* this is called when the window is closing.  it notifies the object.  the */
/* object should not take any action. */
void								FunctionObjectClosingWindowNotify(FunctionObjectRec* FuncObj,
											short NewWindowX, short NewWindowY, short NewWindowWidth,
											short NewWindowHeight);

/* the document's name has changed, so we need to update the window */
void								FunctionObjectGlobalNameChange(FunctionObjectRec* FuncObj,
											char* NewFilename);

/* create a new object initialized with data read in from a file. */
FileLoadingErrors		FunctionObjectNewFromFile(FunctionObjectRec** ObjectOut,
											struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
											struct MainWindowRec* MainWindow,
											struct FunctionListRec* FuncList);

/* write the data in an object out to disk */
FileLoadingErrors		FunctionObjectWriteOutData(FunctionObjectRec* FuncObj,
											struct BufferedOutputRec* Output);

/* mark all items in the function object as not changed */
void								FunctionObjectMarkAsSaved(FunctionObjectRec* FuncObj);

#endif
