/* ASTFuncCall.h */

#ifndef Included_ASTFuncCall_h
#define Included_ASTFuncCall_h

/* ASTFuncCall module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* ASTExpressionList */
/* TrashTracker */
/* Memory */
/* ASTExpression */
/* PcodeObject */
/* CompilerRoot */
/* SymbolTableEntry */
/* SymbolList */
/* PromotableTypeCheck */
/* ASTOperand */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTFuncCallRec;
typedef struct ASTFuncCallRec ASTFuncCallRec;

/* all memory allocated from this module is done through TrashTracker */

/* forwards */
struct TrashTrackRec;
struct ASTExprListRec;
struct ASTExpressionRec;

/* create a new function call node.  the argument list can be NIL if there are */
/* no arguments. */
ASTFuncCallRec*			NewFunctionCall(struct ASTExprListRec* ArgumentList,
											struct ASTExpressionRec* FunctionGeneratorExpression,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the function call node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckFunctionCall(DataTypes* ResultingDataType,
											ASTFuncCallRec* FunctionCall, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for a function call. returns True if successful, or False if it fails. */
MyBoolean						CodeGenFunctionCall(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTFuncCallRec* FunctionCall);

#endif
