/* PeepholeOptimizer.h */

#ifndef Included_PeepholeOptimizer_h
#define Included_PeepholeOptimizer_h

/* PeepholeOptimizer module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* PcodeObject */
/* Memory */

/* forward declarations */
struct PcodeRec;

/* this routine does all of the optimization. */
void								OptimizePcode(struct PcodeRec* ThePcode);

#endif
