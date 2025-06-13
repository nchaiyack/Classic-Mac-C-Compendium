/* InstrList.h */

#ifndef Included_InstrList_h
#define Included_InstrList_h

/* InstrList module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* StringList */
/* Array */
/* Memory */
/* Alert */
/* DataMunging */
/* InstrObject */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */
/* Files */
/* Scrap */

#include "Screen.h"
#include "EventLoop.h"
#include "MainWindowStuff.h"

struct InstrListRec;
typedef struct InstrListRec InstrListRec;

/* forward declarations */
struct MainWindowRec;
struct CodeCenterRec;
struct InstrObjectRec;
struct BufferedInputRec;
struct BufferedOutputRec;
struct FileType;

/* create a new instrument list */
InstrListRec*				NewInstrList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* delete the instrument list and all of the instruments it contains */
void								DisposeInstrList(InstrListRec* InstrList);

/* change the location of the instrument list in the window */
void								SetInstrListLocation(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* redraw the list */
void								InstrListRedraw(InstrListRec* InstrList);

/* see if the specified coordinates falls inside the instrument list rectangle */
MyBoolean						InstrListHitTest(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc);

/* handle a mouse down event for the instrument list */
void								InstrListDoMouseDown(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);

/* called when the window becomes active */
void								InstrListBecomeActive(InstrListRec* InstrList);

/* called when the window becomes inactive */
void								InstrListBecomeInactive(InstrListRec* InstrList);

/* called when a selection is made in another list, so that this list */
/* is deselected */
void								InstrListDeselect(InstrListRec* InstrList);

/* check to see if there is a selection in this list */
MyBoolean						InstrListIsThereSelection(InstrListRec* InstrList);

/* check to see if any of the instruments contained in this list need to be saved */
MyBoolean						DoesInstrListNeedToBeSaved(InstrListRec* InstrList);

/* open an edit window for the selected instrument */
void								InstrListOpenSelection(InstrListRec* InstrList);

/* create a new instrument and open a window for it */
void								InstrListNewInstr(InstrListRec* InstrList);

/* delete the selected instrument */
void								InstrListDeleteSelection(InstrListRec* InstrList);

/* delete the explicitly specified instrument */
void								InstrListDeleteInstr(InstrListRec* InstrList,
											struct InstrObjectRec* TheInstr);

/* the name of an instrument has changed, so the name in the scrolling */
/* list must also be changed */
void								InstrListInstrNameChanged(InstrListRec* InstrList,
											struct InstrObjectRec* TheInstr);

/* look for a specified instrument.  returns NIL if not found.  the name is */
/* NOT null terminated */
struct InstrObjectRec*	InstrListLookupNamedInstr(InstrListRec* InstrList, char* Name);

/* document's name has changed, so we need to update the windows */
void								InstrListGlobalNameChange(InstrListRec* InstrList,
											char* NewFilename);

/* compile all of the instruments definitions */
MyBoolean						InstrListMakeUpToDate(InstrListRec* InstrList);

/* dispose of all of the objects from the instrument definitions */
void								InstrListUnbuildAll(InstrListRec* InstrList);

/* read instrument definitions from a file.  returns True if completely successful. */
FileLoadingErrors		InstrListReadData(InstrListRec* InstrList,
											struct BufferedInputRec* Input);

/* write instrument definitions to a file.  returns True if completely successful. */
FileLoadingErrors		InstrListWriteData(InstrListRec* InstrList,
											struct BufferedOutputRec* Output);

/* after a file has been saved, this is called to mark all objects as not modified. */
void								InstrListMarkAllObjectsSaved(InstrListRec* InstrList);

/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						InstrListCopyObject(InstrListRec* InstrList);

/* try to paste the clipboard in as an instrument.  returns False if it failed or the */
/* clipboard did not contain an instrument. */
MyBoolean						InstrListPasteObject(InstrListRec* InstrList);

/* try to paste the instrument object in from the file */
MyBoolean						InstrListPasteFromFile(InstrListRec* InstrList,
											struct FileType* File);

/* find out how many instruments there are in this list */
long								InstrListHowMany(InstrListRec* InstrList);

/* get an indexed instruments from the list */
struct InstrObjectRec*	InstrListGetIndexedInstr(InstrListRec* InstrList, long Index);

#endif
