/* LinearTransition.h */

#ifndef Included_LinearTransition_h
#define Included_LinearTransition_h

/* LinearTransition module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

struct LinearTransRec;
typedef struct LinearTransRec LinearTransRec;

/* open a new linear transition state record */
LinearTransRec*			NewLinearTransition(long Start, long Destination, long TicksToReach);

/* refill a linear transition with new state information */
void								RefillLinearTransition(LinearTransRec* TransRec, long Start,
											long Destination, long TicksToReach);

/* get rid of a linear transition state record */
void								DisposeLinearTransition(LinearTransRec* TransRec);

/* get rid of all cached transition state records */
void								FlushLinearTransitionRecords(void);

/* execute one cycle and return the value */
long								LinearTransitionUpdate(LinearTransRec* TransRec);

/* execute multiple cycles and return the value */
long								LinearTransitionUpdateMultiple(LinearTransRec* TransRec,
											long NumCycles);

#endif
