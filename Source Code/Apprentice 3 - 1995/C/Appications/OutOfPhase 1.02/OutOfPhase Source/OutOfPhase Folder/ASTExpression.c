/* ASTExpression.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "ASTArrayDeclaration.h"
#include "ASTAssignment.h"
#include "ASTBinaryOperator.h"
#include "ASTConditional.h"
#include "ASTExpressionList.h"
#include "ASTFuncCall.h"
#include "ASTLoop.h"
#include "ASTOperand.h"
#include "ASTUnaryOperator.h"
#include "ASTVariableDeclaration.h"
#include "ASTErrorForm.h"
#include "ASTWaveGetter.h"
#include "SymbolTableEntry.h"


struct ASTExpressionRec
	{
		ExprTypes						ElementType;
		DataTypes						WhatIsTheExpressionType;
		long								LineNumber;
		union
			{
				ASTArrayDeclRec*		ArrayDeclaration;
				ASTAssignRec*				Assignment;
				ASTBinaryOpRec*			BinaryOperator;
				ASTCondRec*					Conditional;
				ASTExprListRec*			ExpressionList;
				ASTFuncCallRec*			FunctionCall;
				ASTLoopRec*					Loop;
				ASTOperandRec*			Operand;
				ASTUnaryOpRec*			UnaryOperator;
				ASTVarDeclRec*			VariableDeclaration;
				ASTErrorFormRec*		ErrorForm;
				ASTWaveGetterRec*		WaveGetter;
			} u;
	};


/* construct a generic expression around an array declaration */
ASTExpressionRec*		NewExprArrayDecl(struct ASTArrayDeclRec* TheArrayDeclaration,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheArrayDeclaration);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprArrayDecl");

		Expr->ElementType = eExprArrayDeclaration;
		Expr->u.ArrayDeclaration = TheArrayDeclaration;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around an assignment statement */
ASTExpressionRec*		NewExprAssignment(struct ASTAssignRec* TheAssignment,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheAssignment);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprAssignment");

		Expr->ElementType = eExprAssignment;
		Expr->u.Assignment = TheAssignment;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a binary operator */
ASTExpressionRec*		NewExprBinaryOperator(struct ASTBinaryOpRec* TheBinaryOperator,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheBinaryOperator);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprBinaryOperator");

		Expr->ElementType = eExprBinaryOperator;
		Expr->u.BinaryOperator = TheBinaryOperator;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a conditional. */
ASTExpressionRec*		NewExprConditional(struct ASTCondRec* TheConditional,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheConditional);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprConditional");

		Expr->ElementType = eExprConditional;
		Expr->u.Conditional = TheConditional;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a list of expressions. */
ASTExpressionRec*		NewExprSequence(struct ASTExprListRec* TheExpressionList,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		if (TheExpressionList != NIL)
			{
				CheckPtrExistence(TheExpressionList);
			}
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprSequence");

		Expr->ElementType = eExprExpressionList;
		Expr->u.ExpressionList = TheExpressionList;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a function call */
ASTExpressionRec*		NewExprFunctionCall(struct ASTFuncCallRec* TheFunctionCall,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheFunctionCall);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprFunctionCall");

		Expr->ElementType = eExprFunctionCall;
		Expr->u.FunctionCall = TheFunctionCall;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a loop */
ASTExpressionRec*		NewExprLoop(struct ASTLoopRec* TheLoop,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheLoop);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprLoop");

		Expr->ElementType = eExprLoop;
		Expr->u.Loop = TheLoop;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around an operand */
ASTExpressionRec*		NewExprOperand(struct ASTOperandRec* TheOperand,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheOperand);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprOperand");

		Expr->ElementType = eExprOperand;
		Expr->u.Operand = TheOperand;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a unary operator */
ASTExpressionRec*		NewExprUnaryOperator(struct ASTUnaryOpRec* TheUnaryOperator,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheUnaryOperator);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprUnaryOperator");

		Expr->ElementType = eExprUnaryOperator;
		Expr->u.UnaryOperator = TheUnaryOperator;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a variable declaration */
ASTExpressionRec*		NewExprVariableDeclaration(struct ASTVarDeclRec* TheVariableDecl,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheVariableDecl);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprVariableDeclaration");

		Expr->ElementType = eExprVariableDeclaration;
		Expr->u.VariableDeclaration = TheVariableDecl;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around an error form */
ASTExpressionRec*		NewExprErrorForm(struct ASTErrorFormRec* TheErrorForm,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheErrorForm);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprErrorForm");

		Expr->ElementType = eExprErrorForm;
		Expr->u.ErrorForm = TheErrorForm;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* construct a generic expression around a wave getter */
ASTExpressionRec*		NewExprWaveGetter(struct ASTWaveGetterRec* TheWaveGetter,
											struct TrashTrackRec* TrashTracker, long TheLineNumber)
	{
		ASTExpressionRec*	Expr;

		CheckPtrExistence(TheWaveGetter);
		CheckPtrExistence(TrashTracker);
		Expr = (ASTExpressionRec*)AllocTrackedBlock(sizeof(ASTExpressionRec),TrashTracker);
		if (Expr == NIL)
			{
				return NIL;
			}
		SetTag(Expr,"ASTExpressionRec: NewExprWaveGetter");

		Expr->ElementType = eExprWaveGetter;
		Expr->u.WaveGetter = TheWaveGetter;
		Expr->LineNumber = TheLineNumber;

		return Expr;
	}


/* type check an expression.  returns eCompileNoError and the resulting value */
/* type if it checks correctly. */
CompileErrors				TypeCheckExpression(DataTypes* ResultTypeOut,
											ASTExpressionRec* TheExpression, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			ReturnValue;

		CheckPtrExistence(TheExpression);
		CheckPtrExistence(TrashTracker);

		switch (TheExpression->ElementType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckExpression:  unknown AST node type"));
					break;
				case eExprArrayDeclaration:
					ReturnValue = TypeCheckArrayConstruction(ResultTypeOut,
						TheExpression->u.ArrayDeclaration,ErrorLineNumber,TrashTracker);
					break;
				case eExprAssignment:
					ReturnValue = TypeCheckAssignment(ResultTypeOut,TheExpression->u.Assignment,
						ErrorLineNumber,TrashTracker);
					break;
				case eExprBinaryOperator:
					ReturnValue = TypeCheckBinaryOperator(ResultTypeOut,
						TheExpression->u.BinaryOperator,ErrorLineNumber,TrashTracker);
					break;
				case eExprConditional:
					ReturnValue = TypeCheckConditional(ResultTypeOut,TheExpression->u.Conditional,
						ErrorLineNumber,TrashTracker);
					break;
				case eExprExpressionList:
					/* this needs to be done specially */
					if (TheExpression->u.ExpressionList == NIL)
						{
							*ErrorLineNumber = TheExpression->LineNumber;
							ReturnValue = eCompileVoidExpressionIsNotAllowed;
						}
					 else
						{
							ReturnValue = TypeCheckExprList(ResultTypeOut,
								TheExpression->u.ExpressionList,ErrorLineNumber,TrashTracker);
						}
					break;
				case eExprFunctionCall:
					ReturnValue = TypeCheckFunctionCall(ResultTypeOut,TheExpression->u.FunctionCall,
						ErrorLineNumber,TrashTracker);
					break;
				case eExprLoop:
					ReturnValue = TypeCheckLoop(ResultTypeOut,TheExpression->u.Loop,ErrorLineNumber,
						TrashTracker);
					break;
				case eExprOperand:
					ReturnValue = TypeCheckOperand(ResultTypeOut,TheExpression->u.Operand,
						ErrorLineNumber,TrashTracker);
					break;
				case eExprUnaryOperator:
					ReturnValue = TypeCheckUnaryOperator(ResultTypeOut,
						TheExpression->u.UnaryOperator,ErrorLineNumber,TrashTracker);
					break;
				case eExprVariableDeclaration:
					ReturnValue = TypeCheckVariableDeclaration(ResultTypeOut,
						TheExpression->u.VariableDeclaration,ErrorLineNumber,TrashTracker);
					break;
				case eExprErrorForm:
					ReturnValue = TypeCheckErrorForm(ResultTypeOut,TheExpression->u.ErrorForm,
						ErrorLineNumber,TrashTracker);
					break;
				case eExprWaveGetter:
					ReturnValue = TypeCheckWaveGetter(ResultTypeOut,TheExpression->u.WaveGetter,
						ErrorLineNumber,TrashTracker);
					break;
			}

		TheExpression->WhatIsTheExpressionType = *ResultTypeOut;
		return ReturnValue;
	}


/* get a symbol table entry out of an expression.  this is used for getting */
/* function generation stuff. */
CompileErrors				ExpressionGetFunctionCallSymbol(struct SymbolRec** SymbolOut,
											ASTExpressionRec* TheExpression)
	{
		SymbolRec*				FunctionThing;

		CheckPtrExistence(TheExpression);
		if (TheExpression->ElementType != eExprOperand)
			{
				return eCompileFunctionIdentifierRequired;
			}
		if (!IsOperandASymbol(TheExpression->u.Operand))
			{
				return eCompileFunctionIdentifierRequired;
			}
		FunctionThing = GetSymbolFromOperand(TheExpression->u.Operand);
		CheckPtrExistence(FunctionThing);
		if (eSymbolFunction != WhatIsThisSymbol(FunctionThing))
			{
				return eCompileFunctionIdentifierRequired;
			}
		*SymbolOut = FunctionThing;
		return eCompileNoError;
	}


/* find out if the expression is a valid lvalue */
MyBoolean						IsExpressionValidLValue(ASTExpressionRec* TheExpression)
	{
		CheckPtrExistence(TheExpression);

		/* to be a valid lvalue, the expression must be one of */
		/*  - variable */
		/*  - array subscription operation */
		switch (TheExpression->ElementType)
			{
				default:
					return False;

				case eExprBinaryOperator:
					return (BinaryOperatorWhichOne(TheExpression->u.BinaryOperator)
						== eBinaryArraySubscripting);

				case eExprOperand:
					if (IsOperandASymbol(TheExpression->u.Operand))
						{
							SymbolRec*			TheOperandThing;

							TheOperandThing = GetSymbolFromOperand(TheExpression->u.Operand);
							return WhatIsThisSymbol(TheOperandThing) == eSymbolVariable;
						}
					 else
						{
							return False;
						}
			}
		EXECUTE(PRERR(ForceAbort,"IsExpressionValidLValue:  control reached end"));
	}


/* find out what kind of expression it is */
ExprTypes						WhatKindOfExpressionIsThis(ASTExpressionRec* TheExpression)
	{
		CheckPtrExistence(TheExpression);
		return TheExpression->ElementType;
	}


/* get the operand from the generic expression */
struct ASTOperandRec*	GetOperandOutOfExpression(ASTExpressionRec* TheExpression)
	{
		CheckPtrExistence(TheExpression);
		ERROR(TheExpression->ElementType != eExprOperand,PRERR(ForceAbort,
			"GetOperandOutOfExpression:  expression isn't an operand"));
		return TheExpression->u.Operand;
	}


/* get the binary operator out of the generic expression */
struct ASTBinaryOpRec*	GetBinaryOperatorOutOfExpression(ASTExpressionRec* TheExpression)
	{
		CheckPtrExistence(TheExpression);
		ERROR(TheExpression->ElementType != eExprBinaryOperator,PRERR(ForceAbort,
			"GetBinaryOperatorOutOfExpression:  expression isn't an operand"));
		return TheExpression->u.BinaryOperator;
	}


/* get the type of value that is returned by this expression */
DataTypes						GetExpressionsResultantType(ASTExpressionRec* TheExpression)
	{
		CheckPtrExistence(TheExpression);
		return TheExpression->WhatIsTheExpressionType;
	}


/* generate code for an expression.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpression(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExpressionRec* Expression)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(Expression);
		StackDepth = *StackDepthParam;

		switch (Expression->ElementType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenExpression:  unknown expression type"));
					break;
				case eExprArrayDeclaration:
					if (!CodeGenArrayConstruction(FuncCode,&StackDepth,
						Expression->u.ArrayDeclaration))
						{
							return False;
						}
					break;
				case eExprAssignment:
					if (!CodeGenAssignment(FuncCode,&StackDepth,Expression->u.Assignment))
						{
							return False;
						}
					break;
				case eExprBinaryOperator:
					if (!CodeGenBinaryOperator(FuncCode,&StackDepth,Expression->u.BinaryOperator))
						{
							return False;
						}
					break;
				case eExprConditional:
					if (!CodeGenConditional(FuncCode,&StackDepth,Expression->u.Conditional))
						{
							return False;
						}
					break;
				case eExprExpressionList:
					if (!CodeGenExpressionListSequence(FuncCode,&StackDepth,
						Expression->u.ExpressionList))
						{
							return False;
						}
					break;
				case eExprFunctionCall:
					if (!CodeGenFunctionCall(FuncCode,&StackDepth,Expression->u.FunctionCall))
						{
							return False;
						}
					break;
				case eExprLoop:
					if (!CodeGenLoop(FuncCode,&StackDepth,Expression->u.Loop))
						{
							return False;
						}
					break;
				case eExprOperand:
					if (!CodeGenOperand(FuncCode,&StackDepth,Expression->u.Operand))
						{
							return False;
						}
					break;
				case eExprUnaryOperator:
					if (!CodeGenUnaryOperator(FuncCode,&StackDepth,Expression->u.UnaryOperator))
						{
							return False;
						}
					break;
				case eExprVariableDeclaration:
					if (!CodeGenVarDecl(FuncCode,&StackDepth,Expression->u.VariableDeclaration))
						{
							return False;
						}
					break;
				case eExprErrorForm:
					if (!CodeGenErrorForm(FuncCode,&StackDepth,Expression->u.ErrorForm))
						{
							return False;
						}
					break;
				case eExprWaveGetter:
					if (!CodeGenWaveGetter(FuncCode,&StackDepth,Expression->u.WaveGetter))
						{
							return False;
						}
					break;
			}
		ERROR(
			(((Expression->ElementType != eExprVariableDeclaration)
				&& (Expression->ElementType != eExprArrayDeclaration))
				&& (StackDepth != *StackDepthParam + 1))
			||
			(((Expression->ElementType == eExprVariableDeclaration)
				|| (Expression->ElementType == eExprArrayDeclaration))
				&& (StackDepth != *StackDepthParam + 2)),
			PRERR(ForceAbort,"CodeGenExpression:  stack depth error"));

		*StackDepthParam = StackDepth;
		return True;
	}
