/* AlgoSampList.h */

#ifndef Included_AlgoSampList_h
#define Included_AlgoSampList_h

/* AlgoSampList module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */
/* StringList */
/* Array */
/* Memory */
/* AlgoSampObject */
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

struct AlgoSampListRec;
typedef struct AlgoSampListRec AlgoSampListRec;

/* forward declarations */
struct MainWindowRec;
struct CodeCenterRec;
struct AlgoSampObjectRec;
struct BufferedInputRec;
struct BufferedOutputRec;
struct FileType;

/* create a new algorithmic sample list */
AlgoSampListRec*		NewAlgoSampList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* delete the algorithmic sample list and all of the samples it contains */
void								DisposeAlgoSampList(AlgoSampListRec* AlgoSampList);

/* change the location of the algorithmic sample list in the window */
void								SetAlgoSampListLocation(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* redraw the list */
void								AlgoSampListRedraw(AlgoSampListRec* AlgoSampList);

/* see if the specified coordinates falls inside the sample list rectangle */
MyBoolean						AlgoSampListHitTest(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc);

/* handle a mouse down event for the algorithmic sample list */
void								AlgoSampListDoMouseDown(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);

/* called when the window becomes active */
void								AlgoSampListBecomeActive(AlgoSampListRec* AlgoSampList);

/* called when the window becomes inactive */
void								AlgoSampListBecomeInactive(AlgoSampListRec* AlgoSampList);

/* called when a selection is made in another list, so that this list */
/* is deselected */
void								AlgoSampListDeselect(AlgoSampListRec* AlgoSampList);

/* check to see if there is a selection in this list */
MyBoolean						AlgoSampListIsThereSelection(AlgoSampListRec* AlgoSampList);

/* check to see if any of the algo samples contained in this list need */
/* to be saved */
MyBoolean						DoesAlgoSampListNeedToBeSaved(AlgoSampListRec* AlgoSampList);

/* open an edit window for the selected algorithmic sample */
void								AlgoSampListOpenSelection(AlgoSampListRec* AlgoSampList);

/* create a new algorithmic sample and open a window for it */
void								AlgoSampListNewAlgoSamp(AlgoSampListRec* AlgoSampList);

/* delete the selected algorithmic sample */
void								AlgoSampListDeleteSelection(AlgoSampListRec* AlgoSampList);

/* delete the explicitly specified algorithmic sample */
void								AlgoSampListDeleteAlgoSamp(AlgoSampListRec* AlgoSampList,
											struct AlgoSampObjectRec* TheAlgoSamp);

/* the name of a algorithmic sample has changed, so the name in the scrolling */
/* list must also be changed */
void								AlgoSampListAlgoSampNameChanged(AlgoSampListRec* AlgoSampList,
											struct AlgoSampObjectRec* TheAlgoSamp);

/* look for a specified algorithmic sample.  returns NIL if not found.  the name is */
/* NOT null terminated */
struct AlgoSampObjectRec*	AlgoSampListLookupNamedAlgoSamp(
											AlgoSampListRec* AlgoSampList, char* Name);

/* remove all data arrays for all algorithmic samples */
void								AlgoSampListUnbuildAll(AlgoSampListRec* AlgoSampList);

/* build all algorithmic sample tables.  returns True if successful. */
MyBoolean						AlgoSampListMakeUpToDate(AlgoSampListRec* AlgoSampList);

/* document's name changed, so we have to update the windows */
void								AlgoSampListGlobalNameChange(AlgoSampListRec* AlgoSampList,
											char* NewFilename);

/* read algorithmic sample objects from a file.  returns True if completely successful. */
FileLoadingErrors		AlgoSampListReadData(AlgoSampListRec* AlgoSampList,
											struct BufferedInputRec* Input);

/* write algorithmic sample objects to a file.  returns True if completely successful. */
FileLoadingErrors		AlgoSampListWriteData(AlgoSampListRec* AlgoSampList,
											struct BufferedOutputRec* Output);

/* after a file has been saved, this is called to mark all objects as not modified. */
void								AlgoSampListMarkAllObjectsSaved(AlgoSampListRec* AlgoSampList);

/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						AlgoSampListCopyObject(AlgoSampListRec* AlgoSampList);

/* try to paste the clipboard in as an algorithmic sample object.  returns False if */
/* it failed or the clipboard did not contain an algorithmic sample object. */
MyBoolean						AlgoSampListPasteObject(AlgoSampListRec* AlgoSampList);

/* try to paste an algorithmic sample object from the file */
MyBoolean						AlgoSampListPasteFromFile(AlgoSampListRec* AlgoSampList,
											struct FileType* File);

/* find out how many algorithmic samples there are in this list */
long								AlgoSampListHowMany(AlgoSampListRec* AlgoSampList);

/* get an indexed algorithmic sample from the list */
struct AlgoSampObjectRec*	AlgoSampListGetIndexedAlgoSamp(AlgoSampListRec* AlgoSampList,
											long Index);

#endif
