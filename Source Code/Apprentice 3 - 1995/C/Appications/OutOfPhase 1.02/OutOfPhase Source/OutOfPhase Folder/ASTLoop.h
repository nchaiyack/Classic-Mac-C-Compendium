/* ASTLoop.h */

#ifndef Included_ASTLoop_h
#define Included_ASTLoop_h

/* ASTLoop module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* ASTExpression */
/* TrashTracker */
/* Memory */
/* PcodeObject */
/* CompilerRoot */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTLoopRec;
typedef struct ASTLoopRec ASTLoopRec;

/* all memory allocated in this module is from TrashTracker */

typedef enum
	{
		eLoopWhileDo EXECUTE(= -32541),
		eLoopUntilDo,
		eLoopDoWhile,
		eLoopDoUntil
	} LoopTypes;

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a new loop node */
ASTLoopRec*					NewLoop(LoopTypes LoopType, struct ASTExpressionRec* ControlExpr,
											struct ASTExpressionRec* BodyExpr,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the loop node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckLoop(DataTypes* ResultingDataType,
											ASTLoopRec* Loop, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for a loop. returns True if successful, or False if it fails. */
MyBoolean						CodeGenLoop(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTLoopRec* Loop);

#endif
