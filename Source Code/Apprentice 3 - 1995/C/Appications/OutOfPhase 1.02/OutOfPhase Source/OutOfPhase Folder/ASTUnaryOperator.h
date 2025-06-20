/* ASTUnaryOperator.h */

#ifndef Included_ASTUnaryOperator_h
#define Included_ASTUnaryOperator_h

/* ASTUnaryOperator module depends on */
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

struct ASTUnaryOpRec;
typedef struct ASTUnaryOpRec ASTUnaryOpRec;

/* all memory allocated from this module is created from TrashTracker */

typedef enum
	{
		eUnaryNegation EXECUTE(= -1252),
		eUnaryNot,
		eUnarySine,
		eUnaryCosine,
		eUnaryTangent,
		eUnaryArcSine,
		eUnaryArcCosine,
		eUnaryArcTangent,
		eUnaryLogarithm,
		eUnaryExponentiation,
		eUnaryCastToBoolean,
		eUnaryCastToInteger,
		eUnaryCastToSingle,
		eUnaryCastToDouble,
		eUnaryCastToFixed,
		eUnarySquare,
		eUnarySquareRoot,
		eUnaryAbsoluteValue,
		eUnaryTestNegative,
		eUnaryGetSign,
		eUnaryGetArrayLength
	} UnaryOpType;

/* forwards */
struct TrashTrackRec;
struct ASTExpressionRec;

/* create a unary operator node */
ASTUnaryOpRec*	NewUnaryOperator(UnaryOpType WhatOperation,
									struct ASTExpressionRec* Argument, struct TrashTrackRec* TrashTracker,
									long LineNumber);

/* type check the unary operator node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors		TypeCheckUnaryOperator(DataTypes* ResultingDataType,
									ASTUnaryOpRec* UnaryOperator, long* ErrorLineNumber,
									struct TrashTrackRec* TrashTracker);

/* generate code for a unary operator. returns True if successful, or False if it fails. */
MyBoolean				CodeGenUnaryOperator(struct PcodeRec* FuncCode,
									long* StackDepthParam, ASTUnaryOpRec* UnaryOperator);

#endif
