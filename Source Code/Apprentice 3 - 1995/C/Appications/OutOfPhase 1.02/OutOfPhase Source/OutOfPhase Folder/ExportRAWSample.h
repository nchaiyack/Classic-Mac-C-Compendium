/* ExportRAWSample.h */

#ifndef Included_ExportRAWSample_h
#define Included_ExportRAWSample_h

/* ExportRAWSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* RadioButton */
/* SimpleButton */
/* Memory */
/* Alert */
/* EventLoop */
/* Files */
/* BufferedFileOutput */
/* SampleStorageActual */
/* SampleConsts */

/* forwards */
struct SampleStorageActualRec;

/* this routine saves the data in the provided sample storage object as a raw */
/* data file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportRAWSample(struct SampleStorageActualRec* TheSample);

#endif
