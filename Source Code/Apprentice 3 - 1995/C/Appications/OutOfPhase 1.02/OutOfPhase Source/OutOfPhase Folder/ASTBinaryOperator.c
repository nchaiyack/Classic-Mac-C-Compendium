/* ASTBinaryOperator.c */
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

#include "ASTBinaryOperator.h"
#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "PromotableTypeCheck.h"


struct ASTBinaryOpRec
	{
		BinaryOpType				OperationType;
		ASTExpressionRec*		LeftArg;
		ASTExpressionRec*		RightArg;
		long								LineNumber;
	};


/* create a new binary operator */
ASTBinaryOpRec*			NewBinaryOperator(BinaryOpType Operation,
											struct ASTExpressionRec* LeftArgument,
											struct ASTExpressionRec* RightArgument,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTBinaryOpRec*		BinaryOp;

		CheckPtrExistence(LeftArgument);
		CheckPtrExistence(RightArgument);
		CheckPtrExistence(TrashTracker);
		BinaryOp = (ASTBinaryOpRec*)AllocTrackedBlock(sizeof(ASTBinaryOpRec),TrashTracker);
		if (BinaryOp == NIL)
			{
				return NIL;
			}
		SetTag(BinaryOp,"ASTBinaryOpRec");

		BinaryOp->LineNumber = LineNumber;
		BinaryOp->OperationType = Operation;
		BinaryOp->LeftArg = LeftArgument;
		BinaryOp->RightArg = RightArgument;

		return BinaryOp;
	}


/* do any needed type promotion */
static CompileErrors	PromoteTypeHelper(DataTypes* LeftOperandType,
											DataTypes* RightOperandType, ASTBinaryOpRec* BinaryOperator,
											long* ErrorLineNumber, struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;

		if (CanRightBeMadeToMatchLeft(*LeftOperandType,*RightOperandType)
			&& MustRightBePromotedToLeft(*LeftOperandType,*RightOperandType))
			{
				ASTExpressionRec*		PromotedRightOperand;

				/* we must promote the right operand to become the left operand type */
				PromotedRightOperand = PromoteTheExpression(*RightOperandType/*orig*/,
					*LeftOperandType/*desired*/,BinaryOperator->RightArg,
					BinaryOperator->LineNumber,TrashTracker);
				if (PromotedRightOperand == NIL)
					{
						*ErrorLineNumber = BinaryOperator->LineNumber;
						return eCompileOutOfMemory;
					}
				BinaryOperator->RightArg = PromotedRightOperand;
				/* sanity check */
				Error = TypeCheckExpression(RightOperandType/*obtain new right type*/,
					BinaryOperator->RightArg,ErrorLineNumber,TrashTracker);
				ERROR((Error != eCompileNoError),PRERR(ForceAbort,
					"TypeCheckBinaryOperator:  type promotion caused failure"));
				ERROR(*RightOperandType != *LeftOperandType,PRERR(ForceAbort,
					"TypeCheckBinaryOperator:  after type promotion, types are no"
					" longer the same"));
			}
		else if (CanRightBeMadeToMatchLeft(*RightOperandType,*LeftOperandType)
			&& MustRightBePromotedToLeft(*RightOperandType,*LeftOperandType))
			{
				ASTExpressionRec*		PromotedLeftOperand;

				/* we must promote the left operand to become the right operand type */
				PromotedLeftOperand = PromoteTheExpression(*LeftOperandType/*orig*/,
					*RightOperandType/*desired*/,BinaryOperator->LeftArg,
					BinaryOperator->LineNumber,TrashTracker);
				if (PromotedLeftOperand == NIL)
					{
						*ErrorLineNumber = BinaryOperator->LineNumber;
						return eCompileOutOfMemory;
					}
				BinaryOperator->LeftArg = PromotedLeftOperand;
				/* sanity check */
				Error = TypeCheckExpression(LeftOperandType/*obtain new left type*/,
					BinaryOperator->LeftArg,ErrorLineNumber,TrashTracker);
				ERROR((Error != eCompileNoError),PRERR(ForceAbort,
					"TypeCheckBinaryOperator:  type promotion caused failure"));
				ERROR(*RightOperandType != *LeftOperandType,PRERR(ForceAbort,
					"TypeCheckBinaryOperator:  after type promotion, types are no"
					" longer the same"));
			}
		return eCompileNoError;
	}


/* type check the binary operator node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckBinaryOperator(DataTypes* ResultingDataType,
											ASTBinaryOpRec* BinaryOperator, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		DataTypes					LeftOperandType;
		DataTypes					RightOperandType;

		CheckPtrExistence(BinaryOperator);
		CheckPtrExistence(TrashTracker);

		Error = TypeCheckExpression(&LeftOperandType,BinaryOperator->LeftArg,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		Error = TypeCheckExpression(&RightOperandType,BinaryOperator->RightArg,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* do type checking and promotion.  return type determination is deferred */
		switch (BinaryOperator->OperationType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckBinaryOperator:  unknown opcode"));
					break;

				/* operators capable of boolean, integer, single, double, and fixed args, */
				/* which return a boolean result */
				case eBinaryEqual:
				case eBinaryNotEqual:
					if (!CanRightBeMadeToMatchLeft(LeftOperandType,RightOperandType)
						&& !CanRightBeMadeToMatchLeft(RightOperandType,LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileTypeMismatch;
						}
					ERROR((IsItAScalarType(LeftOperandType) && !IsItAScalarType(RightOperandType))
						|| (!IsItAScalarType(LeftOperandType) && IsItAScalarType(RightOperandType)),
						PRERR(ForceAbort,"TypeCheckBinaryOperator:  IsItAScalarType error"));
					if (!IsItAScalarType(LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileOperandsMustBeScalar;
						}
					/* do type promotion */
					Error = PromoteTypeHelper(&LeftOperandType,&RightOperandType,BinaryOperator,
						ErrorLineNumber,TrashTracker);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					break;

				/* operators capable of boolean, integer, and fixed, */
				/* which return the same type as the arguments */
				case eBinaryAnd:
				case eBinaryOr:
				case eBinaryXor:
					if ((LeftOperandType != eBoolean) && (LeftOperandType != eInteger)
						&& (LeftOperandType != eFixed))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileTypeMismatch;
						}
					if (LeftOperandType != RightOperandType)
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileTypeMismatch;
						}
					break;

				/* operators capable of integer, single, double, and fixed args, */
				/* where the return is the same type as args */
				case eBinaryPlus:
				case eBinaryMinus:
				case eBinaryMultiplication:
				/* FALL THROUGH! */

				/* operators capable of integer, single, double, and fixed args, */
				/* where the return is a single, double, or fixed */
				case eBinaryImpreciseDivision:
				/* FALL THROUGH! */

				/* operators capable of integer, single, double, and fixed args, */
				/* which return a boolean */
				case eBinaryLessThan:
				case eBinaryLessThanOrEqual:
				case eBinaryGreaterThan:
				case eBinaryGreaterThanOrEqual:
					if (!CanRightBeMadeToMatchLeft(LeftOperandType,RightOperandType)
						&& !CanRightBeMadeToMatchLeft(RightOperandType,LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileTypeMismatch;
						}
					ERROR((IsItAScalarType(LeftOperandType) && !IsItAScalarType(RightOperandType))
						|| (!IsItAScalarType(LeftOperandType) && IsItAScalarType(RightOperandType)),
						PRERR(ForceAbort,"TypeCheckBinaryOperator:  IsItAScalarType error"));
					if (!IsItASequencedScalarType(LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileOperandsMustBeSequencedScalar;
						}
					/* do type promotion */
					Error = PromoteTypeHelper(&LeftOperandType,&RightOperandType,BinaryOperator,
						ErrorLineNumber,TrashTracker);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					break;

				/* operators capable of integers, returning integer results */
				case eBinaryIntegerDivision:
				case eBinaryIntegerRemainder:
					if ((LeftOperandType != eInteger) || (RightOperandType != eInteger))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileOperandsMustBeIntegers;
						}
					/* no type promotion is necessary */
					break;

				/* operators where the left argument must be integer, single, double, fixed */
				/* and the right argument must be integer, and it returns the same type */
				/* as the left argument */
				case eBinaryShiftLeft:
				case eBinaryShiftRight:
					if (RightOperandType != eInteger)
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileRightOperandMustBeInteger;
						}
					if (!IsItASequencedScalarType(LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileOperandsMustBeSequencedScalar;
						}
					/* DON'T do type promotion */
					break;

				/* operators where the left argument must be an array and the right */
				/* argument must be an integer, and the array's element type is returned */
				case eBinaryArraySubscripting:
					if (RightOperandType != eInteger)
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileArraySubscriptMustBeInteger;
						}
					if (!IsItAnIndexedType(LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileArrayRequiredForSubscription;
						}
					break;

				/* operators where the arguments are double, */
				/* and which return a double result */
				case eBinaryExponentiation:
					if (!CanRightBeMadeToMatchLeft(eDouble,LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileDoubleRequiredForExponentiation;
						}
					if (!CanRightBeMadeToMatchLeft(eDouble,RightOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileDoubleRequiredForExponentiation;
						}
					/* force the promotion, if necessary */
					if (LeftOperandType != eDouble)
						{
							DataTypes			FakePromotionForcer = eDouble;

							/* promote right operand to double, so left operand is a fake double */
							Error = PromoteTypeHelper(&LeftOperandType,&FakePromotionForcer,
								BinaryOperator,ErrorLineNumber,TrashTracker);
							if (Error != eCompileNoError)
								{
									return Error;
								}
							ERROR((FakePromotionForcer != eDouble)
								|| (LeftOperandType != eDouble),PRERR(ForceAbort,
								"TypeCheckBinaryOperator:  exponent ->double promotion failed"));
						}
					if (RightOperandType != eDouble)
						{
							DataTypes			FakePromotionForcer = eDouble;

							/* promote left operand to double, so right operand is a fake double */
							Error = PromoteTypeHelper(&FakePromotionForcer,&RightOperandType,
								BinaryOperator,ErrorLineNumber,TrashTracker);
							if (Error != eCompileNoError)
								{
									return Error;
								}
							ERROR((FakePromotionForcer != eDouble)
								|| (RightOperandType != eDouble),PRERR(ForceAbort,
								"TypeCheckBinaryOperator:  exponent ->double promotion failed"));
						}
					break;

				/* operators where left must be an array type and right must be an integer */
				/* and an array of the same type is returned */
				case eBinaryResizeArray:
					if (!IsItAnIndexedType(LeftOperandType))
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileArrayRequiredForResize;
						}
					if (RightOperandType != eInteger)
						{
							*ErrorLineNumber = BinaryOperator->LineNumber;
							return eCompileIntegerRequiredForResize;
						}
					break;
			}

		/* now, figure out what the return type should be */
		switch (BinaryOperator->OperationType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckBinaryOperator:  unknown opcode"));
					break;

				/* operators capable of boolean, integer, single, double, and fixed args, */
				/* which return a boolean result */
				case eBinaryEqual:
				case eBinaryNotEqual:
					ERROR(LeftOperandType != RightOperandType,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not equivalent but should be"));
					ERROR(!IsItAScalarType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not scalar but should be"));
					*ResultingDataType = eBoolean;
					break;

				/* operators capable of boolean, integer, single, double, and fixed args, */
				/* which return the same type as the arguments */
				case eBinaryAnd:
				case eBinaryOr:
				case eBinaryXor:
					ERROR(LeftOperandType != RightOperandType,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not equivalent but should be"));
					ERROR((LeftOperandType != eBoolean) && (LeftOperandType != eInteger)
						&& (LeftOperandType != eFixed),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not what they should be"));
					*ResultingDataType = LeftOperandType;
					break;

				/* operators capable of integer, single, double, and fixed args, */
				/* where the return is the same type as args */
				case eBinaryPlus:
				case eBinaryMinus:
				case eBinaryMultiplication:
					ERROR(LeftOperandType != RightOperandType,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not equivalent but should be"));
					ERROR(!IsItASequencedScalarType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not seq scalar but should be"));
					*ResultingDataType = LeftOperandType;
					break;

				/* operators capable of integer, single, double, and fixed args, */
				/* where the return is a single, double, or fixed */
				case eBinaryImpreciseDivision:
					ERROR(LeftOperandType != RightOperandType,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not equivalent but should be"));
					ERROR(!IsItASequencedScalarType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not seq scalar but should be"));
					if (LeftOperandType != eInteger)
						{
							*ResultingDataType = LeftOperandType;
						}
					 else
						{
							*ResultingDataType = eDouble;
						}
					break;

				/* operators capable of integer, single, double, and fixed args, */
				/* which return a boolean */
				case eBinaryLessThan:
				case eBinaryLessThanOrEqual:
				case eBinaryGreaterThan:
				case eBinaryGreaterThanOrEqual:
					ERROR(LeftOperandType != RightOperandType,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not equivalent but should be"));
					ERROR(!IsItAScalarType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand types are not scalar but should be"));
					*ResultingDataType = eBoolean;
					break;

				/* operators capable of integers, returning integer results */
				case eBinaryIntegerDivision:
				case eBinaryIntegerRemainder:
					ERROR((LeftOperandType != eInteger) || (RightOperandType != eInteger),
						PRERR(ForceAbort,"TypeCheckBinaryOperator:  operands should be integers"));
					*ResultingDataType = eInteger;
					break;

				/* operators where the left argument must be integer, single, double, fixed */
				/* and the right argument must be integer, and it returns the same type */
				/* as the left argument */
				case eBinaryShiftLeft:
				case eBinaryShiftRight:
					ERROR(RightOperandType != eInteger,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  right operand should be integer"));
					ERROR(!IsItASequencedScalarType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  left operand should be seq scalar"));
					*ResultingDataType = LeftOperandType;
					break;

				/* operators where the left argument must be an array and the right */
				/* argument must be an integer, and the array's element type is returned */
				case eBinaryArraySubscripting:
					ERROR(RightOperandType != eInteger,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  right operand should be integer"));
					switch (LeftOperandType)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TypeCheckBinaryOperator:  spurious type "
									"occurred after array subscript typecheck filter"));
								break;
							case eArrayOfBoolean:
								*ResultingDataType = eBoolean;
								break;
							case eArrayOfInteger:
								*ResultingDataType = eInteger;
								break;
							case eArrayOfFloat:
								*ResultingDataType = eFloat;
								break;
							case eArrayOfDouble:
								*ResultingDataType = eDouble;
								break;
							case eArrayOfFixed:
								*ResultingDataType = eFixed;
								break;
						}
					break;

				/* operators where the arguments are double, */
				/* and which return a double result */
				case eBinaryExponentiation:
					ERROR((LeftOperandType != eDouble) || (RightOperandType != eDouble),
						PRERR(ForceAbort,"TypeCheckBinaryOperator:  operands should be double"));
					*ResultingDataType = eDouble;
					break;

				/* operators where left must be an array type and right must be an integer */
				/* and an array of the same type is returned */
				case eBinaryResizeArray:
					ERROR(!IsItAnIndexedType(LeftOperandType),PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand should be array"));
					ERROR(RightOperandType != eInteger,PRERR(ForceAbort,
						"TypeCheckBinaryOperator:  operand should be integer"));
					*ResultingDataType = LeftOperandType;
					break;
			}

		return eCompileNoError;
	}


/* find out just what kind of binary operation this is */
BinaryOpType				BinaryOperatorWhichOne(ASTBinaryOpRec* TheBinOp)
	{
		CheckPtrExistence(TheBinOp);
		return TheBinOp->OperationType;
	}


/* get the left hand side operand out of a binary operator record */
struct ASTExpressionRec*	GetLeftOperandForBinaryOperator(ASTBinaryOpRec* TheBinOp)
	{
		CheckPtrExistence(TheBinOp);
		return TheBinOp->LeftArg;
	}


/* get the right hand side operand out of a binary operator record */
struct ASTExpressionRec*	GetRightOperandForBinaryOperator(ASTBinaryOpRec* TheBinOp)
	{
		CheckPtrExistence(TheBinOp);
		return TheBinOp->RightArg;
	}


/* generate code for a binary operator.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenBinaryOperator(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTBinaryOpRec* BinaryOperator)
	{
		long							StackDepth;
		Pcodes						Opcode;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(BinaryOperator);
		StackDepth = *StackDepthParam;

		/* generate code for left operand */
		if (!CodeGenExpression(FuncCode,&StackDepth,BinaryOperator->LeftArg))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenBinaryOperator:  stack depth error on left operand"));

		/* generate code for the right operand */
		if (!CodeGenExpression(FuncCode,&StackDepth,BinaryOperator->RightArg))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 2,PRERR(ForceAbort,
			"CodeGenBinaryOperator:  stack depth error on right operand"));

		/* generate the opcode for performing the computation */
		switch (BinaryOperator->OperationType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator:  unknown opcode"));
					break;

				case eBinaryAnd:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryAnd]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryAnd]:  "
									"bad operand types"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanAnd;
								break;
							case eInteger:
								Opcode = epOperationIntegerAnd;
								break;
							case eFixed:
								Opcode = epOperationFixedAnd;
								break;
						}
					break;

				case eBinaryOr:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryOr]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryOr]:  "
									"bad operand types"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanOr;
								break;
							case eInteger:
								Opcode = epOperationIntegerOr;
								break;
							case eFixed:
								Opcode = epOperationFixedOr;
								break;
						}
					break;

				case eBinaryXor:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryXor]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryXor]:  "
									"bad operand types"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanXor;
								break;
							case eInteger:
								Opcode = epOperationIntegerXor;
								break;
							case eFixed:
								Opcode = epOperationFixedXor;
								break;
						}
					break;

				case eBinaryLessThan:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryLessThan]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryLessThan]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerLessThan;
								break;
							case eFloat:
								Opcode = epOperationFloatLessThan;
								break;
							case eDouble:
								Opcode = epOperationDoubleLessThan;
								break;
							case eFixed:
								Opcode = epOperationFixedLessThan;
								break;
						}
					break;

				case eBinaryLessThanOrEqual:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryLessThanOrEqual]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryLessThanOrEqual]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerLessThanOrEqual;
								break;
							case eFloat:
								Opcode = epOperationFloatLessThanOrEqual;
								break;
							case eDouble:
								Opcode = epOperationDoubleLessThanOrEqual;
								break;
							case eFixed:
								Opcode = epOperationFixedLessThanOrEqual;
								break;
						}
					break;

				case eBinaryGreaterThan:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryGreaterThan]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryGreaterThan]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerGreaterThan;
								break;
							case eFloat:
								Opcode = epOperationFloatGreaterThan;
								break;
							case eDouble:
								Opcode = epOperationDoubleGreaterThan;
								break;
							case eFixed:
								Opcode = epOperationFixedGreaterThan;
								break;
						}
					break;

				case eBinaryGreaterThanOrEqual:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryGreaterThanOrEqual]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryGreaterThanOrEqual]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerGreaterThanOrEqual;
								break;
							case eFloat:
								Opcode = epOperationFloatGreaterThanOrEqual;
								break;
							case eDouble:
								Opcode = epOperationDoubleGreaterThanOrEqual;
								break;
							case eFixed:
								Opcode = epOperationFixedGreaterThanOrEqual;
								break;
						}
					break;

				case eBinaryEqual:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryEqual]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryEqual]:  "
									"bad operand types"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanEqual;
								break;
							case eInteger:
								Opcode = epOperationIntegerEqual;
								break;
							case eFloat:
								Opcode = epOperationFloatEqual;
								break;
							case eDouble:
								Opcode = epOperationDoubleEqual;
								break;
							case eFixed:
								Opcode = epOperationFixedEqual;
								break;
						}
					break;

				case eBinaryNotEqual:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryNotEqual]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryNotEqual]:  "
									"bad operand types"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanNotEqual;
								break;
							case eInteger:
								Opcode = epOperationIntegerNotEqual;
								break;
							case eFloat:
								Opcode = epOperationFloatNotEqual;
								break;
							case eDouble:
								Opcode = epOperationDoubleNotEqual;
								break;
							case eFixed:
								Opcode = epOperationFixedNotEqual;
								break;
						}
					break;

				case eBinaryPlus:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryPlus]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryPlus]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerAdd;
								break;
							case eFloat:
								Opcode = epOperationFloatAdd;
								break;
							case eDouble:
								Opcode = epOperationDoubleAdd;
								break;
							case eFixed:
								Opcode = epOperationFixedAdd;
								break;
						}
					break;

				case eBinaryMinus:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryMinus]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryMinus]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerSubtract;
								break;
							case eFloat:
								Opcode = epOperationFloatSubtract;
								break;
							case eDouble:
								Opcode = epOperationDoubleSubtract;
								break;
							case eFixed:
								Opcode = epOperationFixedSubtract;
								break;
						}
					break;

				case eBinaryMultiplication:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryMultiplication]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryMultiplication]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerMultiply;
								break;
							case eFloat:
								Opcode = epOperationFloatMultiply;
								break;
							case eDouble:
								Opcode = epOperationDoubleMultiply;
								break;
							case eFixed:
								Opcode = epOperationFixedMultiply;
								break;
						}
					break;

				case eBinaryImpreciseDivision:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryImpreciseDivision]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryImpreciseDivision]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerImpreciseDivide;
								break;
							case eFloat:
								Opcode = epOperationFloatDivide;
								break;
							case eDouble:
								Opcode = epOperationDoubleDivide;
								break;
							case eFixed:
								Opcode = epOperationFixedDivide;
								break;
						}
					break;

				case eBinaryIntegerDivision:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryIntegerDivision]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryIntegerDivision]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerDivide;
								break;
						}
					break;

				case eBinaryIntegerRemainder:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg)
						!= GetExpressionsResultantType(BinaryOperator->LeftArg),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryIntegerRemainder]:  "
						"type check failure -- operands are not the same type"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryIntegerRemainder]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerModulo;
								break;
						}
					break;

				case eBinaryShiftLeft:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg) != eInteger,
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryShiftLeft]:  "
						"type check failure -- right operand isn't an integer"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryShiftLeft]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerShiftLeft;
								break;
							case eFloat:
								Opcode = epOperationFloatShiftLeft;
								break;
							case eDouble:
								Opcode = epOperationDoubleShiftLeft;
								break;
							case eFixed:
								Opcode = epOperationFixedShiftLeft;
								break;
						}
					break;

				case eBinaryShiftRight:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg) != eInteger,
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryShiftRight]:  "
						"type check failure -- right operand isn't an integer"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryShiftRight]:  "
									"bad operand types"));
								break;
							case eInteger:
								Opcode = epOperationIntegerShiftRight;
								break;
							case eFloat:
								Opcode = epOperationFloatShiftRight;
								break;
							case eDouble:
								Opcode = epOperationDoubleShiftRight;
								break;
							case eFixed:
								Opcode = epOperationFixedShiftRight;
								break;
						}
					break;

				case eBinaryArraySubscripting:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg) != eInteger,
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryArraySubscripting]:  "
						"type check failure -- right operand isn't an integer"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryArraySubscripting]:  "
									"bad operand types"));
								break;
							case eArrayOfBoolean:
								Opcode = epLoadBooleanFromArray2;
								break;
							case eArrayOfInteger:
								Opcode = epLoadIntegerFromArray2;
								break;
							case eArrayOfFloat:
								Opcode = epLoadFloatFromArray2;
								break;
							case eArrayOfDouble:
								Opcode = epLoadDoubleFromArray2;
								break;
							case eArrayOfFixed:
								Opcode = epLoadFixedFromArray2;
								break;
						}
					break;

				case eBinaryExponentiation:
					ERROR((GetExpressionsResultantType(BinaryOperator->RightArg) != eDouble)
						|| (GetExpressionsResultantType(BinaryOperator->LeftArg) != eDouble),
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryExponentiation]:  "
						"type check failure -- an argument isn't a double"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryExponentiation]:  "
									"bad operand types"));
								break;
							case eDouble:
								Opcode = epOperationDoublePower;
								break;
						}
					break;

				case eBinaryResizeArray:
					ERROR(GetExpressionsResultantType(BinaryOperator->RightArg) != eInteger,
						PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryResizeArray]:  "
						"type check failure -- right operand isn't an integer"));
					switch (GetExpressionsResultantType(BinaryOperator->LeftArg))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenBinaryOperator[eBinaryResizeArray]:  "
									"bad operand types"));
								break;
							case eArrayOfBoolean:
								Opcode = epResizeBooleanArray2;
								break;
							case eArrayOfInteger:
								Opcode = epResizeIntegerArray2;
								break;
							case eArrayOfFloat:
								Opcode = epResizeFloatArray2;
								break;
							case eArrayOfDouble:
								Opcode = epResizeDoubleArray2;
								break;
							case eArrayOfFixed:
								Opcode = epResizeFixedArray2;
								break;
						}
					break;
			}
		if (!AddPcodeInstruction(FuncCode,Opcode,NIL))
			{
				return False;
			}
		StackDepth -= 1;
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenBinaryOperator:  post operator stack size is screwed up"));

		*StackDepthParam = StackDepth;
		return True;
	}
