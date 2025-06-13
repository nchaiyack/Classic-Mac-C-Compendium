/* OscillatorSpecifier.h */

#ifndef Included_OscillatorSpecifier_h
#define Included_OscillatorSpecifier_h

/* OscillatorSpecifier module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* SampleSelector */
/* ModulationSpecifier */
/* Envelope */
/* LFOListSpecifier */

typedef float OscillatorNumType;

/* forwards */
struct SampleSelectorRec;
struct ModulationSpecRec;
struct EnvelopeRec;
struct LFOListSpecRec;
struct OscillatorListRec;

typedef enum
	{
		eOscillatorSampled EXECUTE(= -1229),
		eOscillatorWaveTable
	} OscillatorTypes;

struct OscillatorRec;
typedef struct OscillatorRec OscillatorRec;

/* create a new oscillator structure */
OscillatorRec*						NewOscillatorSpecifier(void);

/* dispose of an oscillator structure */
void											DisposeOscillatorSpecifier(OscillatorRec* Osc);

/* get the actual name of the oscillator.  don't dispose this! */
char*											OscillatorGetName(OscillatorRec* Osc);

/* put a new oscillator name in.  the object becomes owner of the name block */
void											PutOscillatorName(OscillatorRec* Osc, char* NewName);

/* set the oscillator type */
void											OscillatorSetTheType(OscillatorRec* Osc,
														OscillatorTypes WhatKindOfOscillator);

/* find out what kind of oscillator this is */
OscillatorTypes						OscillatorGetWhatKindItIs(OscillatorRec* Osc);

/* get the pitch interval --> sample mapping */
struct SampleSelectorRec*	OscillatorGetSampleIntervalList(OscillatorRec* Osc);

/* get the list of oscillators that are modulating us */
struct ModulationSpecRec*	OscillatorGetModulatorInputList(OscillatorRec* Osc);

/* get the output loudness of the oscillator */
OscillatorNumType					OscillatorGetOutputLoudness(OscillatorRec* Osc);

/* put a new output loudness in for the oscillator */
void											PutOscillatorNewOutputLoudness(OscillatorRec* Osc,
														double NewOutputLevel);

/* get the frequency multiplier factor */
OscillatorNumType					OscillatorGetFrequencyMultiplier(OscillatorRec* Osc);

/* get the frequency divisor integer */
long											OscillatorGetFrequencyDivisor(OscillatorRec* Osc);

/* get the frequency adder thing */
OscillatorNumType					OscillatorGetFrequencyAdder(OscillatorRec* Osc);

/* change the frequency adjust factors */
void											PutOscillatorNewFrequencyFactors(OscillatorRec* Osc,
														double NewMultipler, long NewDivisor);

/* put a new frequency adder value */
void											PutOscillatorFrequencyAdder(OscillatorRec* Osc,
														double NewAdder);

/* find out if output of this oscillator is to be included in final output */
MyBoolean									IncludeOscillatorInFinalOutput(OscillatorRec* Osc);

/* change whether or not the oscillator is being included in the final output */
void											PutOscillatorIncludeInOutputFlag(OscillatorRec* Osc,
														MyBoolean IncludeInOutputFlag);

/* get the loudness envelope for the oscillator */
struct EnvelopeRec*				OscillatorGetLoudnessEnvelope(OscillatorRec* Osc);

/* get the list of LFO oscillators modulating the loudness envelope output */
struct LFOListSpecRec*		OscillatorGetLoudnessLFOList(OscillatorRec* Osc);

/* get the excitation envelope for the oscillator */
struct EnvelopeRec*				OscillatorGetExcitationEnvelope(OscillatorRec* Osc);

/* get the list of LFO oscillators modulating the excitation envelope output */
struct LFOListSpecRec*		OscillatorGetExcitationLFOList(OscillatorRec* Osc);

/* resolve modulator named references to oscillators */
MyBoolean									ResolveOscillatorModulators(OscillatorRec* Osc,
														struct OscillatorListRec* ListOfOscillators);

/* get the stereo bias factor */
OscillatorNumType					OscillatorGetStereoBias(OscillatorRec* Osc);

/* put a new value for the stereo bias factor */
void											OscillatorPutStereoBias(OscillatorRec* Osc,
														OscillatorNumType NewStereoBias);

/* get the surround bias factor */
OscillatorNumType					OscillatorGetSurroundBias(OscillatorRec* Osc);

/* put a new value for the surround bias factor */
void											OscillatorPutSurroundBias(OscillatorRec* Osc,
														OscillatorNumType NewSurroundBias);

/* get the time displacement factor */
OscillatorNumType					OscillatorGetTimeDisplacement(OscillatorRec* Osc);

/* put a new value for the time displacement factor */
void											OscillatorPutTimeDisplacement(OscillatorRec* Osc,
														OscillatorNumType NewTimeDisplacement);

#endif
