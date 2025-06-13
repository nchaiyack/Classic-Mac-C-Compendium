/* OscillatorListSpecifier.h */

#ifndef Included_OscillatorListSpecifier_h
#define Included_OscillatorListSpecifier_h

/* OscillatorListSpecifier module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* OscillatorSpecifier */

/* forwards */
struct OscillatorRec;

struct OscillatorListRec;
typedef struct OscillatorListRec OscillatorListRec;

/* create a new array of oscillators */
OscillatorListRec*			NewOscillatorListSpecifier(void);

/* dispose of oscillator list */
void										DisposeOscillatorListSpecifier(OscillatorListRec* OscList);

/* append a new oscillator */
MyBoolean								AppendOscillatorToList(OscillatorListRec* OscList,
													struct OscillatorRec* NewOscillator);

/* get one of the oscillators from the list */
struct OscillatorRec*		GetOscillatorFromList(OscillatorListRec* OscList, long Index);

/* find out how many oscillators there are in the list */
long										GetOscillatorListLength(OscillatorListRec* OscList);

/* resolve all modulation references.  returns False if it couldn't be done */
MyBoolean								ResolveOscillatorListModulators(OscillatorListRec* OscList);

#endif
