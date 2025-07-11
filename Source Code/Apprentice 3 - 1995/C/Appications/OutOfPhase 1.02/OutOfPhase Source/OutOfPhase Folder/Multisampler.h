/* Multisampler.h */

#ifndef Included_Multisampler_h
#define Included_Multisampler_h

/* Multisampler module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* SampleSelector */
/* SampleObject */
/* AlgoSampObject */
/* WaveTableObject */
/* AlgoWaveTableObject */
/* SampleConsts */

#include "SampleConsts.h"

struct MultiSampleRec;
typedef struct MultiSampleRec MultiSampleRec;

/* forwards */
struct SampleSelectorRec;

/* create a new multisampling object based on a list of wave tables */
/* the named references in the Selector should have been resolved, but it is */
/* the parser's job to do that. */
MultiSampleRec*				NewMultisampleWaveTable(struct SampleSelectorRec* Selector);

/* create a new multisampling object based on a list of samples */
/* the named references in the Selector should have been resolved, but it is */
/* the parser's job to do that. */
MultiSampleRec*				NewMultisampleSample(struct SampleSelectorRec* Selector);

/* dispose of a multisampling object */
void									DisposeMultisample(MultiSampleRec* Multisample);

/* obtain a data reference & info for a wave table.  returns False if there is */
/* no wave table corresponding to the supplied frequency */
MyBoolean							GetMultisampleReferenceWaveTable(MultiSampleRec* Multisample,
												float FrequencyHertz, void*** TwoDimensionalVecOut,
												long* NumFramesOut, long* NumTablesOut,
												NumBitsType* NumBitsOut);

/* obtain a data reference & info for a sample.  returns False if there is no */
/* sample corresponding to the supplied frequency. */
MyBoolean							GetMultisampleReferenceSample(MultiSampleRec* Multisample,
												float FrequencyHertz, void** DataOut, long* NumFramesOut,
												NumBitsType* NumBitsOut, NumChannelsType* NumChannelsOut,
												long* Loop1StartOut, long* Loop1EndOut, long* Loop2StartOut,
												long* Loop2EndOut, long* Loop3StartOut, long* Loop3EndOut,
												long* OriginOut, double* NaturalFreqOut, long* SamplingRateOut);
#endif
