/* ASTExpression.h */

#ifndef Included_ASTExpression_h
#define Included_ASTExpression_h

/* ASTExpression module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* TrashTracker */
/* Memory */
/* PcodeObject */
/* CompilerRoot */
/* ASTArrayDeclaration */
/* ASTAssignment */
/* ASTBinaryOperator */
/* ASTConditional */
/* ASTExpressionList */
/* ASTFuncCall */
/* ASTLoop */
/* ASTOperand */
/* ASTUnaryOperator */
/* ASTVariableDeclaration */
/* SymbolTableEntry */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTExpressionRec;
typedef struct ASTExpressionRec ASTExpressionRec;

/* all memory allocated in this module is through TrashTracker */

typedef enum
	{
		eExprArrayDeclaration EXECUTE(= -9769),
		eExprAssignment,
		eExprBinaryOperator,
		eExprConditional,
		eExprExpressionList,
		eExprFunctionCall,
		eExprLoop,
		eExprOperand,
		eExprUnaryOperator,
		eExprVariableDeclaration,
		eExprErrorForm,
		eExprWaveGetter
	} ExprTypes;

/* forwards */
struct TrashTrackRec;
struct ASTArrayDeclRec;
struct ASTAssignRec;
struct ASTBinaryOpRec;
struct ASTCondRec;
struct ASTExprListRec;
struct ASTFuncCallRec;
struct ASTLoopRec;
struct ASTOperandRec;
struct ASTUnaryOpRec;
struct ASTVarDeclRec;
struct ASTErrorFormRec;
struct ASTWaveGetterRec;
struct SymbolRec;

/* construct a generic expression around an array declaration */
ASTExpressionRec*		NewExprArrayDecl(struct ASTArrayDeclRec* TheArrayDeclaration,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around an assignment statement */
ASTExpressionRec*		NewExprAssignment(struct ASTAssignRec* TheAssignment,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a binary operator */
ASTExpressionRec*		NewExprBinaryOperator(struct ASTBinaryOpRec* TheBinaryOperator,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a conditional. */
ASTExpressionRec*		NewExprConditional(struct ASTCondRec* TheConditional,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a list of expressions. */
ASTExpressionRec*		NewExprSequence(struct ASTExprListRec* TheExpressionList,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a function call */
ASTExpressionRec*		NewExprFunctionCall(struct ASTFuncCallRec* TheFunctionCall,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a loop */
ASTExpressionRec*		NewExprLoop(struct ASTLoopRec* TheLoop,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around an operand */
ASTExpressionRec*		NewExprOperand(struct ASTOperandRec* TheOperand,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a unary operator */
ASTExpressionRec*		NewExprUnaryOperator(struct ASTUnaryOpRec* TheUnaryOperator,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a variable declaration */
ASTExpressionRec*		NewExprVariableDeclaration(struct ASTVarDeclRec* TheVariableDecl,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around an error form */
ASTExpressionRec*		NewExprErrorForm(struct ASTErrorFormRec* TheErrorForm,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);

/* construct a generic expression around a wave getter */
ASTExpressionRec*		NewExprWaveGetter(struct ASTWaveGetterRec* TheWaveGetter,
											struct TrashTrackRec* TrashTracker, long TheLineNumber);


/* type check an expression.  returns eCompileNoError and the resulting value */
/* type if it checks correctly. */
CompileErrors				TypeCheckExpression(DataTypes* ResultTypeOut,
											ASTExpressionRec* TheExpression, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* get a symbol table entry out of an expression.  this is used for getting */
/* function generation stuff. */
CompileErrors				ExpressionGetFunctionCallSymbol(struct SymbolRec** SymbolOut,
											ASTExpressionRec* TheExpression);

/* find out if the expression is a valid lvalue */
MyBoolean						IsExpressionValidLValue(ASTExpressionRec* TheExpression);

/* find out what kind of expression it is */
ExprTypes						WhatKindOfExpressionIsThis(ASTExpressionRec* TheExpression);

/* generate code for any expression.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpression(struct PcodeRec* FuncCode, long* StackDepthParam,
											ASTExpressionRec* Expression);

/* get the operand from the generic expression */
struct ASTOperandRec*	GetOperandOutOfExpression(ASTExpressionRec* TheExpression);

/* get the binary operator out of the generic expression */
struct ASTBinaryOpRec*	GetBinaryOperatorOutOfExpression(ASTExpressionRec* TheExpression);

/* get the type of value that is returned by this expression */
DataTypes						GetExpressionsResultantType(ASTExpressionRec* TheExpression);

/* generate code for an expression.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpression(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExpressionRec* Expression);

#endif
