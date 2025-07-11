/* DiskFileOutput.h */

#ifndef Included_DiskFileOutput_h
#define Included_DiskFileOutput_h

/* DiskFileOutput module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MainWindowStuff */
/* FixedPoint */
/* BinaryCodedDecimal */
/* Files */
/* Memory */
/* Alert */
/* ExecuteSynthesis */
/* ClipWarnDialog */
/* SynthProgressWindow */
/* BufferedFileOutput */
/* ErrorDaemon */

#include "MainWindowStuff.h"
#include "FixedPoint.h"
#include "BinaryCodedDecimal.h"

/* forwards */
struct MainWindowRec;
struct ArrayRec;
struct TrackObjectRec;

/* this routine opens a file and dumps the data to it. */
void									SynthToAIFFFile(struct MainWindowRec* MainWindow,
												struct ArrayRec* ListOfTracks, struct TrackObjectRec* KeyTrack,
												long FrameToStartAt, long SamplingRate, long EnvelopeRate,
												MyBoolean UseStereo, LargeBCDType DefaultBeatsPerMinute,
												LargeBCDType OverallVolumeScalingReciprocal,
												MyBoolean InterpOverTime, MyBoolean InterpAcrossWaves,
												LargeBCDType ScanningGap, OutputNumBitsType NumBitsOut,
												MyBoolean ClipWarn);

#endif
