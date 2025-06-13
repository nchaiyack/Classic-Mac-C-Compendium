/* LFOListSpecifier.h */

#ifndef Included_LFOListSpecifier_h
#define Included_LFOListSpecifier_h

/* LFOListSpecifier module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* LFOSpecifier */

/* forwards */
struct LFOSpecRec;

struct LFOListSpecRec;
typedef struct LFOListSpecRec LFOListSpecRec;

/* allocate a new LFO spec list */
LFOListSpecRec*				NewLFOListSpecifier(void);

/* dispose of an LFO spec list and all the specs inside of it */
void									DisposeLFOListSpecifier(LFOListSpecRec* LFOListSpec);

/* get a LFOSpecRec out of the list */
struct LFOSpecRec*		LFOListSpecGetLFOSpec(LFOListSpecRec* LFOListSpec, long Index);

/* create a new LFO spec entry in the list */
MyBoolean							LFOListSpecAppendNewEntry(LFOListSpecRec* LFOListSpec,
												struct LFOSpecRec* NewEntry);

/* get the number of elements in the list */
long									LFOListSpecGetNumElements(LFOListSpecRec* LFOListSpec);

#endif
