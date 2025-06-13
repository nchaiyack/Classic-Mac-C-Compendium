/* AlgoWaveTableObject.h */

#ifndef Included_AlgoWaveTableObject_h
#define Included_AlgoWaveTableObject_h

/* AlgoWaveTableObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* SampleConsts */
/* AlgoWaveTableList */
/* Memory */
/* DataMunging */
/* CodeCenter */
/* PcodeStack */
/* PcodeSystem */
/* FixedPoint */
/* MainWindowStuff */
/* Alert */
/* Numbers */
/* AlgoWaveTableWindow */
/* WaveTableStorage */
/* CompilerRoot */
/* FunctionCode */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "SampleConsts.h"
#include "MainWindowStuff.h"

struct AlgoWaveTableObjectRec;
typedef struct AlgoWaveTableObjectRec AlgoWaveTableObjectRec;

/* forward declarations */
struct CodeCenterRec;
struct MainWindowRec;
struct AlgoWaveTableListRec;
struct BufferedInputRec;
struct BufferedOutputRec;

/* allocate and create a new algorithmic wave table */
AlgoWaveTableObjectRec*	NewAlgoWaveTableObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoWaveTableListRec* AlgoWaveTableList);

/* dispose of all data structures associated with wave table */
void									DisposeAlgoWaveTableObject(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* find out if any changes have been made to it */
MyBoolean							HasAlgoWaveTableObjectBeenModified(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* rebuild the wave table and return True if successful */
MyBoolean							AlgoWaveTableObjectBuild(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* remove the wave table data */
void									AlgoWaveTableObjectUnbuild(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* rebuild the wave table if it hasn't been built and return success flag */
MyBoolean							AlgoWaveTableObjectMakeUpToDate(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* get a copy of the name of this object */
char*									AlgoWaveTableObjectGetNameCopy(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* install a new name on the object.  the object becomes owner of the name, so */
/* the caller should not dispose of it. */
void									AlgoWaveTableObjectNewName(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												char* Name);

/* get a copy of the formula that computes the algorithmic wave table */
char*									AlgoWaveTableObjectGetFormulaCopy(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* install a new formula for the algorithmic wave table.  the object becomes the */
/* owner of the memory block so the caller should not dispose of it. */
void									AlgoWaveTableObjectNewFormula(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												char* Formula);

/* find out how many bits are in each sample frame */
NumBitsType						AlgoWaveTableObjectGetNumBits(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* install a new number of bits in the object. */
void									AlgoWaveTableObjectPutNumBits(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												NumBitsType NewNumBits);

/* find out how many frames there are per table */
long									AlgoWaveTableObjectGetNumTables(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* set the number of tables in the wave table */
void									AlgoWaveTableObjectPutNumTables(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long NumTables);

/* get number of frames in each table */
long									AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* set the number of frames in each table */
void									AlgoWaveTableObjectPutNumFrames(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long NumFrames);

/* get a raw slice of data from the wave table.  if the wave table is modified, */
/* this pointer becomes invalid. */
char*									AlgoWaveTableObjectGetRawSlice(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long AlgoWaveTableIndex);

/* make the wave table open it's editor window */
MyBoolean							AlgoWaveTableObjectOpenWindow(AlgoWaveTableObjectRec* AlgoWaveTableObj);

/* notify the object that the editor window is closing.  the object should */
/* not take any action. */
void									AlgoWaveTableObjectClosingWindowNotify(
												AlgoWaveTableObjectRec* AlgoWaveTableObj,
												short NewX, short NewY, short NewWidth, short NewHeight);

/* the document's name has changed, so we need to update the window */
void									AlgoWaveTableObjectGlobalNameChange(AlgoWaveTableObjectRec*
												AlgoWaveTableObj, char* NewFilename);

/* read in an object from the file. */
FileLoadingErrors			AlgoWaveTableObjectNewFromFile(AlgoWaveTableObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoWaveTableListRec* AlgoWaveTableList);

/* write the object out to the file. */
FileLoadingErrors			AlgoWaveTableObjectWriteDataOut(
												AlgoWaveTableObjectRec* AlgoWaveTableObj,
												struct BufferedOutputRec* Output);

/* mark algorithmic wave table object as saved */
void									AlgoWaveTableObjectMarkAsSaved(AlgoWaveTableObjectRec* AlgoWaveTableObj);

#endif
