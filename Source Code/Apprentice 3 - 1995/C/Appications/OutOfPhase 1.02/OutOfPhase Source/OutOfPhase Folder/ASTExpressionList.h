/* ASTExpressionList.h */

#ifndef Included_ASTExpressionList_h
#define Included_ASTExpressionList_h

/* ASTExpressionList module depends on */
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

struct ASTExprListRec;
typedef struct ASTExprListRec ASTExprListRec;

/* all memory allocated from this module is done with TrashTracker */

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* cons an AST expression onto a list */
ASTExprListRec*			ASTExprListCons(struct ASTExpressionRec* First, ASTExprListRec* Rest,
											struct TrashTrackRec* TrashTracker);

/* type check a list of expressions.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckExprList(DataTypes* ResultingDataType,
											ASTExprListRec* ExpressionList, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* get the first expression */
struct ASTExpressionRec*	ExprListGetFirstExpr(ASTExprListRec* ExpressionList);

/* get the tail expression list */
ASTExprListRec*			ExprListGetRestList(ASTExprListRec* ExpressionList);

/* install a new first in the list */
void								ExprListPutNewFirst(ASTExprListRec* ExpressionList,
											struct ASTExpressionRec* NewFirst);

/* generate code for an expression list that is a series of sequential expressions. */
/* returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpressionListSequence(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExprListRec* ExpressionList);

/* generate code for an argument list -- all args stay on the stack. */
/* returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpressionListArguments(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExprListRec* ExpressionList);

#endif
