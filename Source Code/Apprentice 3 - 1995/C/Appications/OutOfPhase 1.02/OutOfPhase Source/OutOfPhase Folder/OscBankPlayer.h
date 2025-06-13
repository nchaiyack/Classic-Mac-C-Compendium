/* OscBankPlayer.h */

#ifndef Included_OscBankPlayer_h
#define Included_OscBankPlayer_h

/* OscBankPlayer module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* FixedPoint */
/* LFOGenerator */
/* BinaryCodedDecimal */
/* IncrementalParameterUpdator */
/* LFOSpecifier */
/* LFOListSpecifier */
/* Memory */
/* Array */
/* OscillatorListSpecifier */
/* OscillatorSpecifier */
/* InstrumentStructure */
/* GenInstrTopology */
/* SampleOscControl */
/* WaveTableOscControl */
/* ModulationOscControl */
/* DeterminedNoteStructure */
/* NoteObject */
/* FloatingPoint */
/* Frequency */
/* ErrorDaemon */

#include "FixedPoint.h"
#include "BinaryCodedDecimal.h"

struct OscBankTemplateRec;
typedef struct OscBankTemplateRec OscBankTemplateRec;

struct OscStateBankRec;
typedef struct OscStateBankRec OscStateBankRec;

/* forwards */
struct InstrumentRec;
struct IncrParamUpdateRec;
struct NoteObjectRec;
struct FrozenNoteRec;
struct ErrorDaemonRec;

/* flush all cached oscillator state bank records */
void									FlushCachedOscStateBankRecords(void);

/* construct an oscillator bank template record.  various parameters are passed in */
/* which are needed for synthesis.  ParameterUpdator is the parameter information */
/* record for the whole track of which this is a part. */
OscBankTemplateRec*		NewOscBankTemplate(struct InstrumentRec* InstrumentDefinition,
												MyBoolean StereoFlag, LargeBCDType OverallVolumeScalingReciprocal,
												long SamplingRate, float EnvelopeRate, MyBoolean TimeInterp,
												MyBoolean WaveInterp, struct IncrParamUpdateRec* ParameterUpdator,
												struct ErrorDaemonRec* ErrorDaemon);

/* dispose of the template */
void									DisposeOscBankTemplate(OscBankTemplateRec* Template);

/* construct a new oscillator bank state object based on the note.  the note is */
/* assumed to start "now" in terms of the parameters in the ParameterUpdator.  */
/* *WhenToStartPlayingOut returns a negative number that represents how much */
/* before "now" the note should start playing. */
/*     <already played>       |    <scanning gap>     |    <not yet analyzed> */
/*   time ---->    time ---->    time ---->    time ---->    time ---->   time ----> */
/*                            ^A                      ^B     */
/* point A is the back edge of the scanning gap.  as this edge moves forward in time, */
/*   oscillator bank state objects are removed from the queue and playback is commenced */
/*   for them. */
/* point B is the front edge of the scanning gap.  as this edge moves forward in time, */
/*   notes are extracted from the track and state bank objects are created for them. */
/*   ParameterUpdator always reflects parameters at this point in time. */
OscStateBankRec*			NewOscBankState(OscBankTemplateRec* Template,
												long* WhenToStartPlayingOut, struct NoteObjectRec* Note,
												float EnvelopeTicksPerDurationTick);

/* this is used for resetting a note for a tie */
/* the FrozenNote object is NOT disposed */
MyBoolean							ResetOscBankState(OscStateBankRec* State,
												struct FrozenNoteRec* FrozenNote,
												float EnvelopeTicksPerDurationTick);

/* get rid of a state bank */
void									DisposeOscStateBank(OscStateBankRec* State);

/* get the reference to the note that this bank ties to.  NIL if it doesn't */
struct NoteObjectRec*	GetOscStateTieTarget(OscStateBankRec* State);

/* perform one envelope clock cycle on a state bank.  this returns True if the */
/* state bank is done and should be retired.  (it will return false if it is a */
/* tie source.) */
MyBoolean							UpdateOscStateBank(OscStateBankRec* State, long NumFrames,
												largefixedsigned* OutputData);

#endif
