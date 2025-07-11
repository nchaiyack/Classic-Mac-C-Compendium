/* ExecuteSynthesis.h */

#ifndef Included_ExecuteSynthesis_h
#define Included_ExecuteSynthesis_h

/* ExecuteSynthesis module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* FixedPoint */
/* BinaryCodedDecimal */
/* Memory */
/* MainWindowStuff */
/* LinearTransition */
/* PlayTrackInfoThang */
/* Array */
/* WaveTableOscControl */
/* EnvelopeState */
/* LFOGenerator */
/* InstrList */
/* InstrObject */
/* TrackObject */
/* SampleOscControl */
/* ModulationOscControl */
/* Fractions */
/* FrameObject */
/* DeterminedNoteStructure */
/* TempoController */
/* OscBankPlayer */
/* DataMunging */
/* Alert */
/* CheckNameUniqueness */
/* ErrorDaemon */

/* this module is the entrypoint for actually doing synthesis. */

#include "FixedPoint.h"
#include "BinaryCodedDecimal.h"

/* forwards */
struct MainWindowRec;
struct CodeCenterRec;
struct FunctionListRec;
struct SampleListRec;
struct AlgoSampListRec;
struct WaveTableListRec;
struct AlgoWaveTableListRec;
struct InstrListRec;
struct ArrayRec;
struct TrackListRec;
struct TrackObjectRec;
struct ErrorDaemonRec;

typedef enum
	{
		eSynthDone EXECUTE(= -4325),
		eSynthNoMemory, /* not enough memory to build internal structures for playback */
		eSynthUserCancelled, /* user cancelled via the periodic callback */
		/* eSynthProgramError, */ /* an inconsistency exists in the track */
		eSynthPrereqError, /* the sample/func/instrs couldn't be built */
		eSynthUndefinedInstrumentError, /* the named instrument does not exist */
		eSynthDataSubmitError, /* some error (perhaps disk error) occurred during callback */
		eSynthDuplicateNames /* some objects have the same name */
	} SynthErrorCodes;

/* This routine does all of the work. */
/* The DataOutCallback is called every time a block of data is */
/* ready to be sent to the target device; this is provided so that data can be */
/* redirected to a file or postprocessed in some way before playback. */
/* the KeyTrack and FrameToStartAt provide a reference point indicating where */
/* playback should occur.  if KeyTrack is NIL, then playback begins at the beginning. */
/* the rate parameters are in operations per second. */
SynthErrorCodes				Synthesizer(struct MainWindowRec* MainWindow,
												MyBoolean (*DataOutCallback)(void* Refcon,
													largefixedsigned* DataBlock, long NumFrames,
													MyBoolean* AbortPlaybackFlagOut),
												void* DataOutRefcon, struct ArrayRec* ListOfTracks,
												struct TrackObjectRec* KeyTrack, long FrameToStartAt,
												long SamplingRate, long EnvelopeRate, MyBoolean UseStereo,
												LargeBCDType DefaultBeatsPerMinute,
												LargeBCDType OverallVolumeScalingReciprocal,
												MyBoolean InterpOverTime, MyBoolean InterpAcrossWaves,
												LargeBCDType ScanningGap, struct ErrorDaemonRec* ErrorDaemon);

#endif
