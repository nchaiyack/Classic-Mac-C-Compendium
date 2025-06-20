/* AlgoWaveTableList.h */

#ifndef Included_AlgoWaveTableList_h
#define Included_AlgoWaveTableList_h

/* AlgoWaveTableList module depends on */
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
/* AlgoWaveTableObject */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */
/* Files */
/* Scrap */

#include "Screen.h"
#include "EventLoop.h"
#include "MainWindowStuff.h"

struct AlgoWaveTableListRec;
typedef struct AlgoWaveTableListRec AlgoWaveTableListRec;

/* forward declarations */
struct MainWindowRec;
struct CodeCenterRec;
struct AlgoWaveTableObjectRec;
struct BufferedInputRec;
struct BufferedOutputRec;
struct FileType;

/* create a new algorithmic wave table list */
AlgoWaveTableListRec*	NewAlgoWaveTableList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* delete the algorithmic wave table list and all of the wave tables it contains */
void								DisposeAlgoWaveTableList(AlgoWaveTableListRec* AlgoWaveTableList);

/* change the location of the algorithmic wave table list in the window */
void								SetAlgoWaveTableListLocation(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height);

/* redraw the list */
void								AlgoWaveTableListRedraw(AlgoWaveTableListRec* AlgoWaveTableList);

/* see if the specified coordinates falls inside the wave table list rectangle */
MyBoolean						AlgoWaveTableListHitTest(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc);

/* handle a mouse down event for the algorithmic wave table list */
void								AlgoWaveTableListDoMouseDown(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers);

/* called when the window becomes active */
void								AlgoWaveTableListBecomeActive(AlgoWaveTableListRec* AlgoWaveTableList);

/* called when the window becomes inactive */
void								AlgoWaveTableListBecomeInactive(AlgoWaveTableListRec* AlgoWaveTableList);

/* called when a selection is made in another list, so that this list */
/* is deselected */
void								AlgoWaveTableListDeselect(AlgoWaveTableListRec* AlgoWaveTableList);

/* check to see if there is a selection in this list */
MyBoolean						AlgoWaveTableListIsThereSelection(AlgoWaveTableListRec* AlgoWaveTableList);

/* check to see if any of the algorithmic wave tables contained in this list need */
/* to be saved */
MyBoolean						DoesAlgoWaveTableListNeedToBeSaved(AlgoWaveTableListRec* AlgoWaveTableList);

/* open an edit window for the selected algorithmic wave table */
void								AlgoWaveTableListOpenSelection(AlgoWaveTableListRec* AlgoWaveTableList);

/* create a new algorithmic wave table and open a window for it */
void								AlgoWaveTableListNewAlgoWaveTable(AlgoWaveTableListRec* AlgoWaveTableList);

/* delete the selected algorithmic wave table */
void								AlgoWaveTableListDeleteSelection(AlgoWaveTableListRec* AlgoWaveTableList);

/* delete the explicitly specified algorithmic wave table */
void								AlgoWaveTableListDeleteAlgoWaveTable(AlgoWaveTableListRec* AlgoWaveTableList,
											struct AlgoWaveTableObjectRec* TheAlgoWaveTable);

/* the name of a algorithmic wave table has changed, so the name in the scrolling */
/* list must also be changed */
void								AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableListRec* AlgoWaveTableList,
											struct AlgoWaveTableObjectRec* TheAlgoWaveTable);

/* look for a specified algorithmic wave table.  returns NIL if not found.  the name is */
/* NOT null terminated */
struct AlgoWaveTableObjectRec*	AlgoWaveTableListLookupNamedAlgoWaveTable(
											AlgoWaveTableListRec* AlgoWaveTableList, char* Name);

/* remove all data arrays for all algorithmic wave tables */
void								AlgoWaveTableListUnbuildAll(AlgoWaveTableListRec* AlgoWaveTableList);

/* build all algorithmic wave tables.  returns True if successful. */
MyBoolean						AlgoWaveTableListMakeUpToDate(AlgoWaveTableListRec* AlgoWaveTableList);

/* the document's name has changed, so we need to update the windows */
void								AlgoWaveTableListGlobalNameChange(AlgoWaveTableListRec*
											AlgoWaveTableList, char* NewFilename);

/* read algorithmic wave table objects from a file.  returns True if successful. */
FileLoadingErrors		AlgoWaveTableListReadData(AlgoWaveTableListRec* AlgoWaveTableList,
											struct BufferedInputRec* Input);

/* write algorithmic wave table objects to a file.  returns True if successful. */
FileLoadingErrors		AlgoWaveTableListWriteData(AlgoWaveTableListRec* AlgoWaveTableList,
											struct BufferedOutputRec* Output);

/* after a file has been saved, this is called to mark all objects as not modified. */
void								AlgoWaveTableListMarkAllObjectsSaved(
											AlgoWaveTableListRec* AlgoWaveTableList);

/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						AlgoWaveTableListCopyObject(AlgoWaveTableListRec* AlgoWaveTableList);

/* try to paste the clipboard in as an algorithmic wave table object.  returns False if */
/* it failed or the clipboard did not contain an algorithmic wave table object. */
MyBoolean						AlgoWaveTableListPasteObject(AlgoWaveTableListRec* AlgoWaveTableList);

/* try to paste the algorithmic wave table object in from the file */
MyBoolean						AlgoWaveTableListPasteFromFile(AlgoWaveTableListRec* AlgoWaveTableList,
											struct FileType* File);

/* find out how many algorithmic wave tables there are in this list */
long								AlgoWaveTableListHowMany(AlgoWaveTableListRec* AlgoWaveTableList);

/* get an indexed algorithmic wave tables from the list */
struct AlgoWaveTableObjectRec*	AlgoWaveTableListGetIndexedAlgoWaveTable(
											AlgoWaveTableListRec* AlgoWaveTableList, long Index);

#endif
