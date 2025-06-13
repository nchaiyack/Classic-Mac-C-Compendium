/* AlgoSampObject.h */

#ifndef Included_AlgoSampObject_h
#define Included_AlgoSampObject_h

/* AlgoSampObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* AlgoSampList */
/* Memory */
/* DataMunging */
/* CodeCenter */
/* PcodeStack */
/* PcodeSystem */
/* MainWindowStuff */
/* Alert */
/* Numbers */
/* SampleStorageActual */
/* AlgoSampWindow */
/* SampleConsts */
/* CompilerRoot */
/* FunctionCode */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "SampleConsts.h"
#include "MainWindowStuff.h"

struct AlgoSampObjectRec;
typedef struct AlgoSampObjectRec AlgoSampObjectRec;

/* forward declarations */
struct CodeCenterRec;
struct MainWindowRec;
struct AlgoSampListRec;
struct BufferedFileInput;
struct BufferedFileOutput;

/* allocate and create a new empty algorithmic sample object, with reasonable defaults */
AlgoSampObjectRec*		NewAlgoSampObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoSampListRec* AlgoSampList);

/* dispose of an algorithmic sample object */
void									DisposeAlgoSampObject(AlgoSampObjectRec* AlgoSampObj);

/* find out if algorithmic sample object has been modified */
MyBoolean							HasAlgoSampObjectBeenModified(AlgoSampObjectRec* AlgoSampObj);

/* build the algorithmic sample.  returns True if successful. */
MyBoolean							AlgoSampObjectBuild(AlgoSampObjectRec* AlgoSampObj);

/* unconditionally unbuild the algorithmic sample */
void									AlgoSampObjectUnbuild(AlgoSampObjectRec* AlgoSampObj);

/* build the algorithmic sample if necessary.  return True if successful. */
MyBoolean							AlgoSampObjectMakeUpToDate(AlgoSampObjectRec* AlgoSampObj);

/* get a copy of the algorithmic sample's name. */
char*									AlgoSampObjectGetNameCopy(AlgoSampObjectRec* AlgoSampObj);

/* set the algorithmic sample's name.  the object becomes the owner of the Name */
/* block so the caller should not release it. */
void									AlgoSampObjectNewName(AlgoSampObjectRec* AlgoSampObj, char* Name);

/* get a copy of the formula that generates the algorithmic sample */
char*									AlgoSampObjectGetFormulaCopy(AlgoSampObjectRec* AlgoSampObj);

/* install a new formula into the algorithmic sample.  the object becomes the owner */
/* of the formula, so the caller should not release it */
void									AlgoSampObjectNewFormula(AlgoSampObjectRec* AlgoSampObj,
												char* Formula);

/* get the number of bits in a channel for the algorithmic sample */
NumBitsType						AlgoSampObjectGetNumBits(AlgoSampObjectRec* AlgoSampObj);

/* change the number of bits in a channel for the algorithmic sample */
void									AlgoSampObjectPutNumBits(AlgoSampObjectRec* AlgoSampObj,
												NumBitsType NewNumBits);

/* get the number of channels in the algorithmic sample */
NumChannelsType				AlgoSampObjectGetNumChannels(AlgoSampObjectRec* AlgoSampObj);

/* change the number of channels for the algorithmic sample */
void									AlgoSampObjectPutNumChannels(AlgoSampObjectRec* AlgoSampObj,
												NumChannelsType NewNumChannels);

/* get the number of data frames in the algorithmic sample */
long									AlgoSampObjetGetNumFrames(AlgoSampObjectRec* AlgoSampObj);

/* get the loop control values.  if the loop end and loop start are the same, */
/* then there is no loop.  Note that these values are NOT bounds checked in any */
/* way so it is the callers responsibility to deal with nonsensical values. */
long									AlgoSampObjectGetOrigin(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopStart1(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopStart2(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopStart3(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopEnd1(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopEnd2(AlgoSampObjectRec* AlgoSampObj);
long									AlgoSampObjectGetLoopEnd3(AlgoSampObjectRec* AlgoSampObj);

/* get algorithmic sample attributes */
long									AlgoSampObjectGetSamplingRate(AlgoSampObjectRec* AlgoSampObj);
double								AlgoSampObjectGetNaturalFrequency(AlgoSampObjectRec* AlgoSampObj);

/* get a pointer to the raw data for the sample.  this pointer is the */
/* actual data, not a copy, so don't dispose of it.  if any operations are performed */
/* on the sample, this pointer may become invalid.  format of raw data: */
/*  - mono, 8-bit:  array of signed bytes */
/*  - stereo, 8-bit:  array of signed bytes, grouped in pairs.  the one lower in */
/*    memory is the left channel */
/*  - mono, 16-bit:  array of signed short integers (either 2 or 4 bytes) */
/*  - stereo, 16-bit:  array of signed short integers, grouped in pairs.  the one */
/*    lower in memory is the left channel */
/* if it returns NIL, then the thing couldn't be built */
char*									AlgoSampObjectGetRawData(AlgoSampObjectRec* AlgoSampObj);

/* change the loop control values and attributes */
void									AlgoSampObjectPutOrigin(AlgoSampObjectRec* AlgoSampObj,
												long Origin);
void									AlgoSampObjectPutLoopStart1(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart);
void									AlgoSampObjectPutLoopStart2(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart);
void									AlgoSampObjectPutLoopStart3(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart);
void									AlgoSampObjectPutLoopEnd1(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd);
void									AlgoSampObjectPutLoopEnd2(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd);
void									AlgoSampObjectPutLoopEnd3(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd);
void									AlgoSampObjectPutSamplingRate(AlgoSampObjectRec* AlgoSampObj,
												long SamplingRate);
void									AlgoSampObjectPutNaturalFrequency(AlgoSampObjectRec* AlgoSampObj,
												double NaturalFrequency);

/* call which makes object open its editor window */
MyBoolean							AlgoSampObjectOpenWindow(AlgoSampObjectRec* AlgoSampObj);

/* this is called by the window when it is closing to notify the object. */
/* the object should not take any action. */
void									AlgoSampObjectClosingWindowNotify(AlgoSampObjectRec* AlgoSampObj,
												short NewX, short NewY, short NewWidth, short NewHeight);

/* the document's name has changed, so the windows need to be updated */
void									AlgoSampObjectGlobalNameChange(AlgoSampObjectRec* AlgoSampObj,
												char* NewFilename);

/* read from the file and create a new algorithmic sample object from it. */
FileLoadingErrors			AlgoSampObjectNewFromFile(AlgoSampObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoSampListRec* AlgoSampList);

/* write the object out to the specified file. */
FileLoadingErrors			AlgoSampObjectWriteOutData(AlgoSampObjectRec* AlgoSampObj,
												struct BufferedOutputRec* Output);

/* mark the object as saved */
void									AlgoSampObjectMarkAsSaved(AlgoSampObjectRec* AlgoSampObj);

#endif
