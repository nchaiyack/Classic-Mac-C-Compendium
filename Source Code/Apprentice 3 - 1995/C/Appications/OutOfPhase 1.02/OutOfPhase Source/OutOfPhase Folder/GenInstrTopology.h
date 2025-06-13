/* GenInstrTopology.h */

#ifndef Included_GenInstrTopology_h
#define Included_GenInstrTopology_h

/* GenInstrTopology module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Array */
/* OscillatorSpecifier */
/* ModulationSpecifier */
/* Memory */

/* GenInstrTopology examines oscillator dependencies and generates schedules for */
/* ordering oscillator evaluation */

/* forwards */
struct EvalEnvRec;
struct ArrayRec;
struct OscillatorRec;

/* the following oscillator nodes can occur: */
/*   1.  independent oscillator.  this one does not depend on any input and it's */
/*       output is not depended on by anyone. */
/*   2.  source oscillator.  this one's output is used by someone.  this oscillator */
/*       can still generate it's output in one shot, but the output must be saved */
/*       for future use. */
/*   3.  target oscillator.  this one requires input from other oscillators.  it */
/*       can still generate it's output in one shot, but requires a precomputed array */
/*       of input for modulation. */
/*   4.  cyclic complex.  this is a series of interdependent oscillators which */
/*       arises due to feedback loops.  all oscillators here must be evaluated in */
/*       lockstep fashion, and may require input vectors or may need to produce */
/*       output vectors. */
/* instances 2 and 3 are special cases of instance 4 which can be optimized to */
/* run more efficiently.  this implementation does NOT use such optimizations. */

/* this routine takes an array of oscillators and builds a series of independent */
/* oscillator groups based on which oscillators depend on others for modulation */
/* input.  the result is an array containing arrays of OscillatorRec's. */
/* for the moment, the ordering is undefined.  if this turns out to matter, or I */
/* implement optimized non-cyclic modulation, then this will need to be changed */
/* by putting them in the proper order and generating dependency annotations. */
struct ArrayRec*			BuildOscillatorLists(struct OscillatorRec** OscillatorArray,
												long NumOscillators);

#endif
