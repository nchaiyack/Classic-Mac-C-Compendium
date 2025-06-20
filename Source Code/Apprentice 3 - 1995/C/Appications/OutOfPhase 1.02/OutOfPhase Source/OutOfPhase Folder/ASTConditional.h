/* ASTConditional.h */

#ifndef Included_ASTConditional_h
#define Included_ASTConditional_h

/* ASTConditional module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* ASTExpression */
/* TrashTracker */
/* Memory */
/* PcodeObject */
/* CompilerRoot */
/* PromotableTypeCheck */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTCondRec;
typedef struct ASTCondRec ASTCondRec;

/* all memory allocated in the module is done through TrashTracker */

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a new if node.  the Alternate can be NIL. */
ASTCondRec*					NewConditional(struct ASTExpressionRec* Conditional,
											struct ASTExpressionRec* Consequent,
											struct ASTExpressionRec* Alternate,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the conditional node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckConditional(DataTypes* ResultingDataType,
											ASTCondRec* Conditional, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for a conditional.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenConditional(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTCondRec* Conditional);

#endif
