/* LFOSpecifier.h */

#ifndef Included_LFOSpecifier_h
#define Included_LFOSpecifier_h

/* LFOSpecifier module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Envelope */
/* SampleSelector */

/* forward declarations */
struct EnvelopeRec;
struct SampleSelectorRec;

/* what kind of oscillator is the LFO */
typedef enum
	{
		eLFOConstant1 EXECUTE(= -12492),
		eLFOSignedSine,
		eLFOPositiveSine,
		eLFOSignedTriangle,
		eLFOPositiveTriangle,
		eLFOSignedSquare,
		eLFOPositiveSquare,
		eLFOSignedRamp,
		eLFOPositiveRamp,
		eLFOSignedLinearFuzz,
		eLFOPositiveLinearFuzz,
		eLFOWaveTable
	} LFOOscTypes;

/* ways the LFO can be used to modulate the signal */
typedef enum
	{
		eLFOAdditive EXECUTE(= -942),
		eLFOMultiplicative,
		eLFOInverseMultiplicative
	} LFOModulationTypes;

/* this is the algorithm that will be used to combine the signal & modulator */
typedef enum
	{
		eLFOArithmetic EXECUTE(= -12481), /* add signals */
		eLFOGeometric /* add base-2 logs of signals & exp */
	} LFOAdderMode;

struct LFOSpecRec;
typedef struct LFOSpecRec LFOSpecRec;

/* create a new LFO specification record */
LFOSpecRec*						NewLFOSpecifier(void);

/* dispose an LFO specification */
void									DisposeLFOSpecifier(LFOSpecRec* LFOSpec);

/* get the frequency envelope record */
struct EnvelopeRec*		GetLFOSpecFrequencyEnvelope(LFOSpecRec* LFOSpec);

/* get the amplitude envelope record */
struct EnvelopeRec*		GetLFOSpecAmplitudeEnvelope(LFOSpecRec* LFOSpec);

/* get the oscillator type for this LFO specifier */
LFOOscTypes						LFOSpecGetOscillatorType(LFOSpecRec* LFOSpec);

/* change the oscillator type */
void									SetLFOSpecOscillatorType(LFOSpecRec* LFOSpec, LFOOscTypes NewType);

/* get the oscillator modulation mode */
LFOModulationTypes		LFOSpecGetModulationMode(LFOSpecRec* LFOSpec);

/* change the oscillator modulation mode */
void									SetLFOSpecModulationMode(LFOSpecRec* LFOSpec,
												LFOModulationTypes NewType);

/* find out what the adding mode of the LFO is */
LFOAdderMode					LFOSpecGetAddingMode(LFOSpecRec* LFOSpec);

/* set a new adding mode for the LFO */
void									SetLFOSpecAddingMode(LFOSpecRec* LFOSpec,
												LFOAdderMode NewAddingMode);

/* for wave table lfo oscillators only */
struct EnvelopeRec*		GetLFOSpecWaveTableIndexEnvelope(LFOSpecRec* LFOSpec);

/* get the sample selector list */
struct SampleSelectorRec*	GetLFOSpecSampleSelector(LFOSpecRec* LFOSpec);

#endif
