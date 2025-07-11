/* InstrumentStructure.h */

#ifndef Included_InstrumentStructure_h
#define Included_InstrumentStructure_h

/* InstrumentStructure module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* LFOListSpecifier */
/* OscillatorListSpecifier */

/* this is abstracted so that we can somewhat easily change it if we need to */
typedef double InstrNumberType;

/* forwards */
struct LFOListSpecRec;
struct OscillatorListRec;

struct InstrumentRec;
typedef struct InstrumentRec InstrumentRec;

/* create a new instrument specification record */
InstrumentRec*						NewInstrumentSpecifier(void);

/* dispose of the instrument specification record */
void											DisposeInstrumentSpecification(InstrumentRec* Instr);

/* get the overall loudness of the instrument */
InstrNumberType						GetInstrumentOverallLoudness(InstrumentRec* Instr);

/* put a new value for overall loudness */
void											InstrumentSetOverallLoudness(InstrumentRec* Instr,
														double NewLoudness);

/* get the instrument's frequency LFO list */
struct LFOListSpecRec*		GetInstrumentFrequencyLFOList(InstrumentRec* Instr);

/* get the instrument's oscillator list */
struct OscillatorListRec*	GetInstrumentOscillatorList(InstrumentRec* Instr);

/* resolve oscillator name references in modulation lists. returns False if there */
/* are unresolvable names */
MyBoolean									ResolveInstrOscillatorModulations(InstrumentRec* Instr);

#endif
