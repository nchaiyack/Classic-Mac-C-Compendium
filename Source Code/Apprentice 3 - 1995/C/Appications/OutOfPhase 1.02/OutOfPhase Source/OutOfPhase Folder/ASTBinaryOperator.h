/* ASTBinaryOperator.h */

#ifndef Included_ASTBinaryOperator_h
#define Included_ASTBinaryOperator_h

/* ASTBinaryOperator module depends on */
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

struct ASTBinaryOpRec;
typedef struct ASTBinaryOpRec ASTBinaryOpRec;

/* all memory allocated in this module is through TrashTracker */

/* binary operator operations */
typedef enum
	{
		eBinaryAnd EXECUTE(= -6623),
		eBinaryOr,
		eBinaryXor,
		eBinaryLessThan,
		eBinaryLessThanOrEqual,
		eBinaryGreaterThan,
		eBinaryGreaterThanOrEqual,
		eBinaryEqual,
		eBinaryNotEqual,
		eBinaryPlus,
		eBinaryMinus,
		eBinaryMultiplication,
		eBinaryImpreciseDivision,
		eBinaryIntegerDivision,
		eBinaryIntegerRemainder,
		eBinaryShiftLeft,
		eBinaryShiftRight,
		eBinaryArraySubscripting,
		eBinaryExponentiation,
		eBinaryResizeArray
	} BinaryOpType;

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a new binary operator */
ASTBinaryOpRec*			NewBinaryOperator(BinaryOpType Operation,
											struct ASTExpressionRec* LeftArgument,
											struct ASTExpressionRec* RightArgument,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the binary operator node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckBinaryOperator(DataTypes* ResultingDataType,
											ASTBinaryOpRec* BinaryOperator, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* find out just what kind of binary operation this is */
BinaryOpType				BinaryOperatorWhichOne(ASTBinaryOpRec* TheBinOp);

/* get the left hand side operand out of a binary operator record */
struct ASTExpressionRec*	GetLeftOperandForBinaryOperator(ASTBinaryOpRec* TheBinOp);

/* get the right hand side operand out of a binary operator record */
struct ASTExpressionRec*	GetRightOperandForBinaryOperator(ASTBinaryOpRec* TheBinOp);

/* generate code for a binary operator.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenBinaryOperator(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTBinaryOpRec* BinaryOperator);

#endif
