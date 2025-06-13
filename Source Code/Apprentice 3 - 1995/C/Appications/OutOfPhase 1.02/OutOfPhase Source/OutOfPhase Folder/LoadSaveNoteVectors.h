/* LoadSaveNoteVectors.h */

#ifndef Included_LoadSaveNoteVectors_h
#define Included_LoadSaveNoteVectors_h

/* LoadSaveNoteVectors module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MainWindowStuff */
/* Array */
/* FrameObject */
/* NoteObject */
/* Memory */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "MainWindowStuff.h"

/* forwards */
struct BufferedInputRec;
struct BufferedOutputRec;
struct ArrayRec;

/* this reads in notes from the file and the tie matrix and builts a note */
/* vector from the information. */
FileLoadingErrors		ReadNoteVector(struct ArrayRec** FrameArrayOut,
											struct BufferedInputRec* Input);

/* this writes out the information for each note and then writes the tie matrix */
/* to the file. */
FileLoadingErrors		WriteNoteVector(struct ArrayRec* ArrayOfFrames,
											struct BufferedOutputRec* Output);

#endif
