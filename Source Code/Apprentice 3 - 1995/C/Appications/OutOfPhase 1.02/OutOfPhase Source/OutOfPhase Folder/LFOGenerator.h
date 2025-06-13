/* LFOGenerator.h */

#ifndef Included_LFOGenerator_h
#define Included_LFOGenerator_h

/* LFOGenerator module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* EnvelopeState */
/* FastFixedPoint */
/* LFOSpecifier */
/* LFOListSpecifier */
/* FloatingPoint */
/* RandomNumbers */
/* Frequency */
/* Multisampler */
/* 64BitMath */
/* SampleConsts */

#include "FastFixedPoint.h"

struct LFOGenRec;
typedef struct LFOGenRec LFOGenRec;

/* forwards */
struct LFOListSpecRec;

typedef enum
	{
		eLFOArithAdditive EXECUTE(= -12431),
		eLFOArithGeometric,
		eLFOArithDefault
	} LFOArithSelect;

/* flush cached LFO generator records */
void								FlushLFOGeneratorRecords(void);

/* create a new LFO generator based on a list of specifications */
/* it returns the largest pre-origin time for all of the envelopes it contains */
/* AmplitudeScaling and FrequencyScaling are provided primarily for frequency */
/* LFO control; other LFOs are controlled through the accent parameters, and */
/* should supply 1 (no scaling) for these parameters. */
LFOGenRec*					NewLFOGenerator(struct LFOListSpecRec* LFOListSpec,
											long* MaxPreOriginTime, float Accent1, float Accent2,
											float Accent3, float Accent4, float FrequencyHertz,
											float HurryUp, float TicksPerSecond, float AmplitudeScaling,
											float FrequencyScaling, LFOArithSelect ModulationMode,
											float FreqForMultisampling);

/* fix up the origin time so that envelopes start at the proper times */
void								LFOGeneratorFixEnvelopeOrigins(LFOGenRec* LFOGen,
											long ActualPreOriginTime);

/* this function computes one LFO cycle and returns the value from the LFO generator. */
/* it should be called on the envelope clock. */
FastFixedType				LFOGenUpdateCycle(LFOGenRec* LFOGen, FastFixedType OriginalValue);

/* pass the key-up impulse on to the envelopes contained inside */
void								LFOGeneratorKeyUpSustain1(LFOGenRec* LFOGen);
void								LFOGeneratorKeyUpSustain2(LFOGenRec* LFOGen);
void								LFOGeneratorKeyUpSustain3(LFOGenRec* LFOGen);

/* retrigger envelopes from the origin point */
void								LFOGeneratorRetriggerFromOrigin(LFOGenRec* LFOGen, float Accent1,
											float Accent2, float Accent3, float Accent4, float FrequencyHertz,
											float HurryUp, float TicksPerSecond, float AmplitudeScaling,
											float FrequencyScaling, MyBoolean ActuallyRetrigger);

/* dispose the LFO generator */
void								DisposeLFOGenerator(LFOGenRec* LFOGen);

#endif
