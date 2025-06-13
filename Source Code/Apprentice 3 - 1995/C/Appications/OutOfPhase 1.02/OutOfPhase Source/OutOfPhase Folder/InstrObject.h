/* InstrObject.h */

#ifndef Included_InstrObject_h
#define Included_InstrObject_h

/* InstrObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* BinaryCodedDecimal */
/* InstrWindow */
/* InstrList */
/* Memory */
/* DataMunging */
/* BuildInstrument */
/* InstrumentStructure */
/* DataMunging */
/* Numbers */
/* Alert */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "BinaryCodedDecimal.h"
#include "MainWindowStuff.h"

struct InstrObjectRec;
typedef struct InstrObjectRec InstrObjectRec;

/* forward declarations */
struct CodeCenterRec;
struct MainWindowRec;
struct InstrListRec;
struct InstrumentRec;

/* create a new empty instrument object */
InstrObjectRec*				NewInstrObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow, struct InstrListRec* InstrList);

/* dispose of instrument object and all the crud it contains */
void									DisposeInstrObject(InstrObjectRec* InstrObj);

/* find out if the object has been changed */
MyBoolean							HasInstrObjectBeenModified(InstrObjectRec* InstrObj);

/* indicate that object has been modified */
void									InstrObjHasBeenModified(InstrObjectRec* InstrObj);

/* unbuild the instrument */
void									UnbuildInstrObject(InstrObjectRec* InstrObj);

/* build the instrument.  returns True if successful */
MyBoolean							BuildInstrObject(InstrObjectRec* InstrObj);

/* make instrument up to date */
MyBoolean							MakeInstrObjectUpToDate(InstrObjectRec* InstrObj);

/* find out if the instrument object is up to date */
MyBoolean							IsIntrumentObjectUpToDate(InstrObjectRec* InstrObj);

/* get the definition of the instrument */
struct InstrumentRec*	GetInstrObjectRawData(InstrObjectRec* InstrObj);

/* get a copy of the instrument's name */
char*									InstrObjectGetNameCopy(InstrObjectRec* InstrObj);

/* install a new name.  the object becomes the owner of the name */
void									InstrObjectPutName(InstrObjectRec* InstrObj, char* Name);

/* get the text definition of the object */
char*									InstrObjectGetDefinitionCopy(InstrObjectRec* InstrObj);

/* put a new definition of the instrument.  it becomes owner of the block */
void									InstrObjectPutDefinition(InstrObjectRec* InstrObj, char* NewDef);

/* display the editor window for this object or bring it to the top */
MyBoolean							InstrObjectOpenWindow(InstrObjectRec* InstrObj);

/* notify the object that the window has disappeared.  The object does not */
/* perform any actions */
void									InstrObjectWindowCloseNotify(InstrObjectRec* InstrObj,
												short NewX, short NewY, short NewWidth, short NewHeight);

/* the document's name changed, so we need to update the window */
void									InstrObjectGlobalNameChange(InstrObjectRec* InstrObj,
												char* NewFilename);

/* create a new object from data in the file. */
FileLoadingErrors			InstrObjectNewFromFile(InstrObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow, struct InstrListRec* InstrList);

/* write the data in the object to the file. */
FileLoadingErrors			InstrObjectWriteDataOut(InstrObjectRec* InstrObj,
												struct BufferedOutputRec* Output);

/* mark the instrument object as saved */
void									InstrObjectMarkAsSaved(InstrObjectRec* InstrObj);

#endif
