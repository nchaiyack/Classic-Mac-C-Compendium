/* ExportAIFFSample.h */

#ifndef Included_ExportAIFFSample_h
#define Included_ExportAIFFSample_h

/* ExportAIFFSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* SampleConsts */
/* Files */
/* SampleStorageActual */
/* BufferedFileOutput */
/* Alert */

/* forwards */
struct SampleStorageActualRec;

/* this routine saves the data in the provided sample storage object as an AIFF-C */
/* formatted file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportAIFFSample(struct SampleStorageActualRec* TheSample,
											long SamplingRate);

#endif
