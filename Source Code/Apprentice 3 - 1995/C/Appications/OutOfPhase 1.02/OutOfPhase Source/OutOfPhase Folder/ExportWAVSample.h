/* ExportWAVSample.h */

#ifndef Included_ExportWAVSample_h
#define Included_ExportWAVSample_h

/* ExportWAVSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* SampleStorageActual */
/* Memory */
/* Files */
/* Alert */
/* BufferedFileOutput */
/* SampleConsts */

/* forwards */
struct SampleStorageActualRec;

/* this routine saves the data in the provided sample storage object as a WAVE */
/* formatted file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportWAVSample(struct SampleStorageActualRec* TheSample,
											long SamplingRate);

#endif
