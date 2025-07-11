/* Envelope.h */

#ifndef Included_Envelope_h
#define Included_Envelope_h

/* Envelope module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Frequency */

/* this type is made so it can be changed */
typedef float EnvNumberType;

/* possible patterns of the envelope transition */
typedef enum
	{
		eEnvelopeLinearInAmplitude EXECUTE(= -13241),
		eEnvelopeLinearInDecibels
	} EnvTransTypes;

/* possible ways of obtaining the new target value */
typedef enum
	{
		eEnvelopeTargetAbsolute EXECUTE(= -31125), /* target is new amplitude */
		eEnvelopeTargetScaling /* target is current amplitude * scaling factor */
	} EnvTargetTypes;

/* these are the kinds of sustain points there are */
typedef enum
	{
		eEnvelopeSustainPointSkip EXECUTE(= -2341), /* hold this point, skip here on release */
		eEnvelopeReleasePointSkip, /* don't hold this point, but skip here on release */
		eEnvelopeSustainPointNoSkip, /* hold this point, but don't skip ahead on release */
		eEnvelopeReleasePointNoSkip /* don't hold this point and don't skip ahead on release */
	} SustainTypes;

#ifdef ShowMeEnvelopeRec
typedef struct
	{
		/* transition duration (seconds) */
		EnvNumberType							Duration;
		/* destination amplitude (0..1) or scaling factor */
		EnvNumberType							EndPoint;
		/* transition type */
		EnvTransTypes							TransitionType;
		/* way of deriving the target */
		EnvTargetTypes						TargetType;
		/* adjustment factors */
		EnvNumberType							Accent1Amp;
		EnvNumberType							Accent2Amp;
		EnvNumberType							Accent3Amp;
		EnvNumberType							Accent4Amp;
		EnvNumberType							FrequencyAmpRolloff;
		EnvNumberType							FrequencyAmpNormalization;
		EnvNumberType							Accent1Rate;
		EnvNumberType							Accent2Rate;
		EnvNumberType							Accent3Rate;
		EnvNumberType							Accent4Rate;
		EnvNumberType							FrequencyRateRolloff;
		EnvNumberType							FrequencyRateNormalization;
	} EnvStepRec;
struct EnvelopeRec
	{
		/* number of envelope phases. 0 phases means the envelope produces constant value */
		long											NumPhases;
		/* list of definitions for each transition phase */
		EnvStepRec*								PhaseArray;
		/* phase at which first sustain occurs. (released by key-up) */
		/* if the value of this is N, then sustain will occur after phase N has */
		/* completed.  if it is 0, then sustain will occur after phase 0 has completed. */
		/* if it is -1, then sustain will not occur.  sustain may not be NumPhases since */
		/* that would be the end of envelope and would be the same as final value hold. */
		long											SustainPhase1;
		SustainTypes							SustainPhase1Type;
		/* phase at which second sustain occurs. */
		long											SustainPhase2;
		SustainTypes							SustainPhase2Type;
		/* phase at which note-end sustain occurs */
		long											SustainPhase3;
		SustainTypes							SustainPhase3Type;
		/* phase used to align envelope timing with other envelopes */
		long											Origin;

		EnvNumberType							OverallScalingFactor;
	};
#else
struct EnvelopeRec;
#endif
typedef struct EnvelopeRec EnvelopeRec;

/* create a new envelope record with nothing in it */
EnvelopeRec*						NewEnvelope(void);

/* dispose of an envelope record */
void										DisposeEnvelope(EnvelopeRec* Envelope);

/* find out how many frames there are in the envelope */
long										GetEnvelopeNumFrames(EnvelopeRec* Envelope);

/* set a release point.  -1 means this release point is ignored */
void										EnvelopeSetReleasePoint1(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType);
void										EnvelopeSetReleasePoint2(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType);
void										EnvelopeSetReleasePoint3(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType);

/* get the value of a release point */
long										GetEnvelopeReleasePoint1(EnvelopeRec* Envelope);
long										GetEnvelopeReleasePoint2(EnvelopeRec* Envelope);
long										GetEnvelopeReleasePoint3(EnvelopeRec* Envelope);

/* get the release point type */
SustainTypes						GetEnvelopeReleaseType1(EnvelopeRec* Envelope);
SustainTypes						GetEnvelopeReleaseType2(EnvelopeRec* Envelope);
SustainTypes						GetEnvelopeReleaseType3(EnvelopeRec* Envelope);

/* set the origin of the envelope */
void										EnvelopeSetOrigin(EnvelopeRec* Envelope, long Origin);

/* get the origin from the envelope */
long										GetEnvelopeOrigin(EnvelopeRec* Envelope);

/* set the value of an adjustment */
void										EnvelopeSetAccent1Amp(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent2Amp(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent3Amp(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent4Amp(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetFreqAmpRolloff(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetFreqAmpNormalization(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent1Rate(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent2Rate(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent3Rate(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetAccent4Rate(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetFreqRateRolloff(EnvelopeRec* Envelope,
													double Val, long Phase);
void										EnvelopeSetFreqRateNormalization(EnvelopeRec* Envelope,
													double Val, long Phase);

/* get the value of an adjustment */
EnvNumberType						GetEnvelopeAccent1Amp(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent2Amp(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent3Amp(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent4Amp(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeFreqAmpRolloff(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeFreqAmpNormalization(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent1Rate(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent2Rate(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent3Rate(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeAccent4Rate(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeFreqRateRolloff(EnvelopeRec* Envelope, long Phase);
EnvNumberType						GetEnvelopeFreqRateNormalization(EnvelopeRec* Envelope, long Phase);

/* set the overall amplitude scaling factor */
void										EnvelopeSetOverallAmplitude(EnvelopeRec* Envelope,
													double OverallAmplitude);

/* get the overall amplitude */
EnvNumberType						GetEnvelopeOverallAmplitude(EnvelopeRec* Envelope);

/* insert a new phase at the specified position.  Values are undefined */
MyBoolean								EnvelopeInsertPhase(EnvelopeRec* Envelope, long Index);

/* delete a phase from the envelope */
void										EnvelopeDeletePhase(EnvelopeRec* Envelope, long Index);

/* set a new value for the specified phase's duration */
void										EnvelopeSetPhaseDuration(EnvelopeRec* Envelope, long Index,
													double Duration);

/* get the duration from the specified envelope position */
EnvNumberType						GetEnvelopePhaseDuration(EnvelopeRec* Envelope, long Index);

/* set a new value for the specified phase's ultimate value */
void										EnvelopeSetPhaseFinalValue(EnvelopeRec* Envelope, long Index,
													double FinalValue);

/* get the phase's ultimate value */
EnvNumberType						GetEnvelopePhaseFinalValue(EnvelopeRec* Envelope, long Index);

/* set the value for a phase's transition type */
void										EnvelopeSetPhaseTransitionType(EnvelopeRec* Envelope, long Index,
													EnvTransTypes TransitionType);

/* obtain the value in a phase's transition type */
EnvTransTypes						GetEnvelopePhaseTransitionType(EnvelopeRec* Envelope, long Index);

/* set the value for a phase's target type */
void										EnvelopeSetPhaseTargetType(EnvelopeRec* Envelope, long Index,
													EnvTargetTypes TargetType);

/* get the target type for a phase */
EnvTargetTypes					GetEnvelopePhaseTargetType(EnvelopeRec* Envelope, long Index);

#endif
