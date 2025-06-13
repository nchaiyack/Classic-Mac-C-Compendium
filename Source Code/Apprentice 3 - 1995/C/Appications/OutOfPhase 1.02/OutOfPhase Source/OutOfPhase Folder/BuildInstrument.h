/* BuildInstrument.h */

#ifndef Included_BuildInstrument_h
#define Included_BuildInstrument_h

/* BuildInstrument module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* TrashTracker */
/* CompilerScanner */
/* InstrumentStructure */
/* OscillatorSpecifier */
/* OscillatorListSpecifier */
/* LFOSpecifier */
/* LFOListSpecifier */
/* FixedPoint */
/* DataMunging */
/* Envelope */
/* SampleSelector */
/* ModulationSpecifier */

/* syntax errors */
typedef enum
	{
		eBuildInstrNoError EXECUTE(= -17736),
		eBuildInstrOutOfMemory,
		eBuildInstrUnexpectedInput,
		eBuildInstrExpectedInstrument,
		eBuildInstrExpectedOpenParen,
		eBuildInstrExpectedCloseParen,
		eBuildInstrSomeRequiredInstrParamsMissing,
		eBuildInstrExpectedSemicolon,
		eBuildInstrExpectedInstrumentMember,
		eBuildInstrMultipleInstrLoudness,
		eBuildInstrSomeRequiredLFOParamsMissing,
		eBuildInstrSomeRequiredOscillatorParamsMissing,
		eBuildInstrExpectedNumber,
		eBuildInstrExpectedStringOrIdentifier,
		eBuildInstrExpectedLFOMember,
		eBuildInstrMultipleLFOFreqEnvelope,
		eBuildInstrSomeRequiredEnvelopeParamsMissing,
		eBuildInstrMultipleLFOAmpEnvelope,
		eBuildInstrMultipleLFOOscillatorType,
		eBuildInstrExpectedLFOOscillatorType,
		eBuildInstrMultipleLFOModulationType,
		eBuildInstrMultipleLFOAddingMode,
		eBuildInstrExpectedLFOModulationType,
		eBuildInstrExpectedOscillatorMember,
		eBuildInstrMultipleOscType,
		eBuildInstrMultipleOscSampleList,
		eBuildInstrMultipleOscModulators,
		eBuildInstrMultipleOscLoudness,
		eBuildInstrMultipleOscFreqMultiplier,
		eBuildInstrMultipleOscFreqDivisor,
		eBuildInstrMultipleOscMakeOutput,
		eBuildInstrMultipleOscLoudnessEnvelope,
		eBuildInstrMultipleOscIndexEnvelope,
		eBuildInstrExpectedOscType,
		eBuildInstrExpectedInteger,
		eBuildInstrExpectedBoolean,
		eBuildInstrExpectedEnvelopeMember,
		eBuildInstrMultipleEnvTotalScaling,
		eBuildInstrMultipleEnvPoints,
		eBuildInstrExpectedSource,
		eBuildInstrExpectedScale,
		eBuildInstrExpectedOriginadjust,
		eBuildInstrExpectedType,
		eBuildInstrExpectedModulationTypeSpecifier,
		eBuildInstrExpectedTarget,
		eBuildInstrExpectedModulationTargetSpecifier,
		eBuildInstrExpectedDelayOrOrigin,
		eBuildInstrExpectedLevelOrScale,
		eBuildInstrExpectedEnvPointMember,
		eBuildInstrExpectedIntBetween1And3,
		eBuildInstrEnvSustainPointAlreadyDefined,
		eBuildInstrMultipleEnvPointAmpAccent1,
		eBuildInstrMultipleEnvPointAmpAccent2,
		eBuildInstrMultipleEnvPointAmpAccent3,
		eBuildInstrMultipleEnvPointAmpAccent4,
		eBuildInstrMultipleEnvPointAmpFreq,
		eBuildInstrMultipleEnvPointRateAccent1,
		eBuildInstrMultipleEnvPointRateAccent2,
		eBuildInstrMultipleEnvPointRateAccent3,
		eBuildInstrMultipleEnvPointRateAccent4,
		eBuildInstrMultipleEnvPointRateFreq,
		eBuildInstrMultipleEnvPointCurveSpec,
		eBuildInstrUnresolvedOscillatorReferences,
		eBuildInstrSomeSamplesDontExist,
		eBuildInstrSomeWaveTablesDontExist,
		eBuildInstrExpectedEnvelope,
		eBuildInstrExpectedLFO,
		eBuildInstrMultipleOscStereoBias,
		eBuildInstrMultipleOscDisplacement,
		eBuildInstrMultipleOscSurroundBias,
		eBuildInserMultipleOscFreqAdder
	} BuildInstrErrors;

/* forwards */
struct InstrumentRec;
struct SampleListRec;
struct AlgoSampListRec;
struct WaveTableListRec;
struct AlgoWaveTableListRec;

/* take a block of text and parse it into an instrument definition.  it returns an */
/* error code.  if an error occurs, then *InstrOut is invalid, otherwise it will */
/* be valid.  the text file remains unaltered.  *ErrorLine is numbered from 1. */
BuildInstrErrors						BuildInstrumentFromText(char* TextFile, long* ErrorLine,
															struct InstrumentRec** InstrOut,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);

/* get a static null terminated string describing the error */
char*												BuildInstrGetErrorMessageText(BuildInstrErrors ErrorCode);

#endif
