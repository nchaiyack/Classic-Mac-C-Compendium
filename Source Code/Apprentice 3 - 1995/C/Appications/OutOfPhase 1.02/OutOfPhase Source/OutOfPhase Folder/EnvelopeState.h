/* EnvelopeState.h */

#ifndef Included_EnvelopeState_h
#define Included_EnvelopeState_h

/* EnvelopeState module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* FastFixedPoint */
/* Envelope */
/* Memory */
/* LinearTransition */
/* Frequency */
/* FloatingPoint */

#include "FastFixedPoint.h"

struct EvalEnvelopeRec;
typedef struct EvalEnvelopeRec EvalEnvelopeRec;

/* forwards */
struct EnvelopeRec;

/* flush cached envelope state records */
void								FlushEvalEnvelopeStateRecords(void);

/* dispose of an envelope state record */
void								DisposeEnvelopeStateRecord(EvalEnvelopeRec* State);

/* create a new envelope state record.  Accent factors have no effect with a value */
/* of 1, attenuate at smaller values, and amplify at larger values. */
EvalEnvelopeRec*		NewEnvelopeStateRecord(struct EnvelopeRec* Template,
											float Accent1, float Accent2, float Accent3, float Accent4,
											float FrequencyHertz, float Loudness, float HurryUp,
											float TicksPerSecond, long* PreOriginTime);

/* when all envelopes have been computed, then the total (i.e. largest) pre-origin */
/* time will be known and we can tell all envelopes how long they must wait */
/* before starting */
void								EnvelopeStateFixUpInitialDelay(EvalEnvelopeRec* State,
											long MaximumPreOriginTime);

/* perform a single cycle of the envelope and return the amplitude for it's */
/* point.  should be called at key-down to obtain initial amplitude. */
FastFixedType				EnvelopeUpdate(EvalEnvelopeRec* State);

/* find out if envelope has reached the end */
MyBoolean						IsEnvelopeAtEnd(EvalEnvelopeRec* State);

/* create key-up impulse.  call this before calling EnvelopeUpdate during a */
/* given cycle.  this call preserves the current level of the envelope but */
/* skips to the phase after the particular sustain. */
void								EnvelopeKeyUpSustain1(EvalEnvelopeRec* State);
void								EnvelopeKeyUpSustain2(EvalEnvelopeRec* State);
void								EnvelopeKeyUpSustain3(EvalEnvelopeRec* State);

/* retrigger envelopes from the origin point */
void								EnvelopeRetriggerFromOrigin(EvalEnvelopeRec* State,
											float Accent1, float Accent2, float Accent3, float Accent4,
											float FrequencyHertz, float Loudness, float HurryUp,
											float TicksPerSecond, MyBoolean ActuallyRetrigger);

#endif
