/* ModulationOscControl.h */

#ifndef Included_ModulationOscControl_h
#define Included_ModulationOscControl_h

/* ModulationOscControl module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* FixedPoint */
/* WaveTableOscControl */
/* ModulationSpecifier */
/* SampleOscControl */
/* FastModulation */
/* OscillatorSpecifier */
/* Array */
/* Memory */
/* LFOListSpecifier */
/* LFOGenerator */
/* EnvelopeState */
/* Envelope */
/* ErrorDaemon */

#include "FixedPoint.h"

struct ModOscTemplateRec;
typedef struct ModOscTemplateRec ModOscTemplateRec;

struct ModOscStateRec;
typedef struct ModOscStateRec ModOscStateRec;

/* forwards */
struct ArrayRec;
struct ErrorDaemonRec;

/* get rid of all cached memory for state or template records */
void									FlushModOscControl(void);

/* perform one envelope update cycle */
void									UpdateModOscEnvelopes(ModOscStateRec* State);

/* dispose of the modulation oscillator state record */
void									DisposeModOscState(ModOscStateRec* State);

/* dispose of the modulation oscillator information template */
void									DisposeModOscTemplate(ModOscTemplateRec* Template);

/* create a new modulation oscillator template */
ModOscTemplateRec*		NewModOscTemplate(struct ArrayRec* OscillatorList,
												float EnvelopeTicksPerSecond, long SamplingRate,
												MyBoolean Stereo, MyBoolean TimeInterp, MyBoolean WaveInterp,
												struct ErrorDaemonRec* ErrorDaemon);

/* create a new modulation oscillator state object. */
ModOscStateRec*				NewModOscState(ModOscTemplateRec* Template,
												float FreqForMultisampling, float Accent1, float Accent2,
												float Accent3, float Accent4, float Loudness, float HurryUp,
												long* PreOriginTimeOut, float StereoPosition,
												float InitialFrequency);

/* fix up pre-origin time for the modulation oscillator state object */
void									FixUpModOscStatePreOrigin(ModOscStateRec* State,
												long ActualPreOrigin);

/* set a new frequency for a modulation oscillator state object.  used for */
/* portamento and modulation of frequency (vibrato) */
void									ModOscStateNewFrequency(ModOscStateRec* State,
												float NewFrequencyHertz);

/* send a key-up signal to one of the oscillators */
void									ModOscKeyUpSustain1(ModOscStateRec* State);
void									ModOscKeyUpSustain2(ModOscStateRec* State);
void									ModOscKeyUpSustain3(ModOscStateRec* State);

/* restart a modulation oscillator oscillator.  this is used for tie continuations */
void									RestartModOscState(ModOscStateRec* State,
												float NewFreqMultisampling, float NewAccent1, float NewAccent2,
												float NewAccent3, float NewAccent4, float NewLoudness,
												float NewHurryUp, MyBoolean RetriggerEnvelopes,
												float NewStereoPosition, float NewInitialFrequency);

/* generate a sequence of samples (called for each envelope clock) */
void									ModOscGenSamples(ModOscStateRec* State,
												long SampleCount, largefixedsigned* RawBuffer);

/* find out if the modulation oscillator has finished */
MyBoolean							ModOscIsItFinished(ModOscStateRec* State);

#endif
