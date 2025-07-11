/* ASTErrorForm.h */

#ifndef Included_ASTErrorForm_h
#define Included_ASTErrorForm_h

/* ASTErrorForm module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* TrashTracker */
/* Memory */
/* ASTExpression */
/* PcodeObject */
/* CompilerRoot */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTErrorFormRec;
typedef struct ASTErrorFormRec ASTErrorFormRec;

/* all memory allocated in this module is through TrashTracker */

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a new AST error form */
ASTErrorFormRec*		NewErrorForm(struct ASTExpressionRec* Expression, char* String,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the error message node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckErrorForm(DataTypes* ResultingDataType,
											ASTErrorFormRec* ErrorMessage, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for an error thing.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenErrorForm(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTErrorFormRec* ErrorForm);

#endif
