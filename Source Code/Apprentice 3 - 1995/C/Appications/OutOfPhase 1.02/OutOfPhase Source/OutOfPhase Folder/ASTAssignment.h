/* ASTAssignment.h */

#ifndef Included_ASTAssignment_h
#define Included_ASTAssignment_h

/* ASTAssignment module depends on */
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
/* ASTOperand */
/* SymbolTableEntry */
/* ASTBinaryOperator */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTAssignRec;
typedef struct ASTAssignRec ASTAssignRec;

/* all memory allocated in this module is through TrashTracker */

/* forward */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a new assignment node */
ASTAssignRec*				NewAssignment(struct ASTExpressionRec* LeftValue,
											struct ASTExpressionRec* RightValue,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the assignment node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckAssignment(DataTypes* ResultingDataType,
											ASTAssignRec* Assignment, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for an assignment.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenAssignment(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTAssignRec* Assignment);

#endif
