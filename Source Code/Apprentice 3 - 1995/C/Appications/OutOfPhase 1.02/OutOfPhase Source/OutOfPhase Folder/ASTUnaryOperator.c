/* ASTUnaryOperator.c */
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

#include "ASTUnaryOperator.h"
#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "PromotableTypeCheck.h"


struct ASTUnaryOpRec
	{
		UnaryOpType					Operation;
		ASTExpressionRec*		Argument;
		long								LineNumber;
	};


/* create a unary operator node */
ASTUnaryOpRec*	NewUnaryOperator(UnaryOpType WhatOperation,
									struct ASTExpressionRec* Argument, struct TrashTrackRec* TrashTracker,
									long LineNumber)
	{
		ASTUnaryOpRec*	UnaryOp;

		CheckPtrExistence(Argument);
		CheckPtrExistence(TrashTracker);
		UnaryOp = (ASTUnaryOpRec*)AllocTrackedBlock(sizeof(ASTUnaryOpRec),TrashTracker);
		if (UnaryOp == NIL)
			{
				return NIL;
			}
		SetTag(UnaryOp,"ASTUnaryOpRec");

		UnaryOp->LineNumber = LineNumber;
		UnaryOp->Operation = WhatOperation;
		UnaryOp->Argument = Argument;

		return UnaryOp;
	}


/* type check the unary operator node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors		TypeCheckUnaryOperator(DataTypes* ResultingDataType,
									ASTUnaryOpRec* UnaryOperator, long* ErrorLineNumber,
									struct TrashTrackRec* TrashTracker)
	{
		CompileErrors	Error;
		DataTypes			ArgumentType;

		CheckPtrExistence(UnaryOperator);
		CheckPtrExistence(TrashTracker);

		/* check the argument and find out what type it is */
		Error = TypeCheckExpression(&ArgumentType,UnaryOperator->Argument,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* do type checking and promotion; return type determination is deferred... */
		switch (UnaryOperator->Operation)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckUnaryOperator:  unknown opcode"));
					break;

				/* operators which take an integer, single, double, or fixed and */
				/* return a boolean */
				case eUnaryTestNegative:
					/* FALL THROUGH */

				/* operators which take an integer, single, double, or fixed and */
				/* return an integer */
				case eUnaryGetSign:
					/* FALL THROUGH */

				/* operators capable of integer, single, double, fixed arguments */
				/* which return the same type as the argument */
				case eUnaryNegation:
				case eUnaryAbsoluteValue:
					if (!IsItASequencedScalarType(ArgumentType))
						{
							*ErrorLineNumber = UnaryOperator->LineNumber;
							return eCompileOperandsMustBeSequencedScalar;
						}
					break;

				/* operators capable of boolean or integer arguments which */
				/* return the same type as the argument */
				case eUnaryNot:
					if ((ArgumentType != eBoolean) && (ArgumentType != eInteger))
						{
							*ErrorLineNumber = UnaryOperator->LineNumber;
							return eCompileOperandMustBeBooleanOrInteger;
						}
					break;

				/* operators capable of double arguments which return doubles */
				case eUnarySine:
				case eUnaryCosine:
				case eUnaryTangent:
				case eUnaryArcSine:
				case eUnaryArcCosine:
				case eUnaryArcTangent:
				case eUnaryLogarithm:
				case eUnaryExponentiation:
				case eUnarySquare:
				case eUnarySquareRoot:
					if (!CanRightBeMadeToMatchLeft(eDouble,ArgumentType))
						{
							*ErrorLineNumber = UnaryOperator->LineNumber;
							return eCompileOperandMustBeDouble;
						}
					if (MustRightBePromotedToLeft(eDouble,ArgumentType))
						{
							ASTExpressionRec*		PromotedOperand;

							/* we must promote the right operand to become the left operand type */
							PromotedOperand = PromoteTheExpression(ArgumentType/*orig*/,
								eDouble/*desired*/,UnaryOperator->Argument,
								UnaryOperator->LineNumber,TrashTracker);
							if (PromotedOperand == NIL)
								{
									*ErrorLineNumber = UnaryOperator->LineNumber;
									return eCompileOutOfMemory;
								}
							UnaryOperator->Argument = PromotedOperand;
							/* sanity check */
							Error = TypeCheckExpression(&ArgumentType/*obtain new right type*/,
								UnaryOperator->Argument,ErrorLineNumber,TrashTracker);
							ERROR((Error != eCompileNoError),PRERR(ForceAbort,
								"TypeCheckUnaryOperator:  type promotion caused failure"));
							ERROR(eDouble != ArgumentType,PRERR(ForceAbort,
								"TypeCheckUnaryOperator:  after type promotion, types are no"
								" longer the same"));
						}
					break;

				/* operands which take a boolean, integer, single, double, or fixed */
				/* and return an integer */
				case eUnaryCastToInteger:
					/* FALL THROUGH */

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a single */
				case eUnaryCastToSingle:
					/* FALL THROUGH */

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a double */
				case eUnaryCastToDouble:
					/* FALL THROUGH */

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a fixed */
				case eUnaryCastToFixed:
					/* FALL THROUGH */

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a boolean */
				case eUnaryCastToBoolean:
					if (!IsItAScalarType(ArgumentType))
						{
							*ErrorLineNumber = UnaryOperator->LineNumber;
							return eCompileOperandsMustBeScalar;
						}
					break;

				/* operators which take an array and return an integer */
				case eUnaryGetArrayLength:
					if (!IsItAnIndexedType(ArgumentType))
						{
							*ErrorLineNumber = UnaryOperator->LineNumber;
							return eCompileArrayRequiredForGetLength;
						}
					break;
			}

		/* figure out the return type */
		switch (UnaryOperator->Operation)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckUnaryOperator:  unknown opcode"));
					break;

				/* operators which take an integer, single, double, or fixed and */
				/* return a boolean */
				case eUnaryTestNegative:
					ERROR(!IsItASequencedScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be seq scalar but isn't"));
					*ResultingDataType = eBoolean;
					break;

				/* operators which take an integer, single, double, or fixed and */
				/* return an integer */
				case eUnaryGetSign:
					ERROR(!IsItASequencedScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be seq scalar but isn't"));
					*ResultingDataType = eInteger;
					break;

				/* operators capable of integer, single, double, fixed arguments */
				/* which return the same type as the argument */
				case eUnaryNegation:
				case eUnaryAbsoluteValue:
					ERROR(!IsItASequencedScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be seq scalar but isn't"));
					*ResultingDataType = ArgumentType;
					break;

				/* operators capable of boolean or integer arguments which */
				/* return the same type as the argument */
				case eUnaryNot:
					ERROR((ArgumentType != eBoolean) && (ArgumentType != eInteger),
						PRERR(ForceAbort,"TypeCheckUnaryOperator:  arg should be int or bool"));
					*ResultingDataType = ArgumentType;
					break;

				/* operators capable of double arguments which return doubles */
				case eUnarySine:
				case eUnaryCosine:
				case eUnaryTangent:
				case eUnaryArcSine:
				case eUnaryArcCosine:
				case eUnaryArcTangent:
				case eUnaryLogarithm:
				case eUnaryExponentiation:
				case eUnarySquare:
				case eUnarySquareRoot:
					ERROR(ArgumentType != eDouble,PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be double but isn't"));
					*ResultingDataType = eDouble;
					break;

				/* operands which take a boolean, integer, single, double, or fixed */
				/* and return an integer */
				case eUnaryCastToInteger:
					ERROR(!IsItAScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be scalar but isn't"));
					*ResultingDataType = eInteger;
					break;

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a single */
				case eUnaryCastToSingle:
					ERROR(!IsItAScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be scalar but isn't"));
					*ResultingDataType = eFloat;
					break;

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a double */
				case eUnaryCastToDouble:
					ERROR(!IsItAScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be scalar but isn't"));
					*ResultingDataType = eDouble;
					break;

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a fixed */
				case eUnaryCastToFixed:
					ERROR(!IsItAScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be scalar but isn't"));
					*ResultingDataType = eFixed;
					break;

				/* operators which take a boolean, integer, single, double, or fixed */
				/* and return a boolean */
				case eUnaryCastToBoolean:
					ERROR(!IsItAScalarType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be scalar but isn't"));
					*ResultingDataType = eBoolean;
					break;

				/* operators which take an array and return an integer */
				case eUnaryGetArrayLength:
					ERROR(!IsItAnIndexedType(ArgumentType),PRERR(ForceAbort,
						"TypeCheckUnaryOperator:  arg should be indexable but isn't"));
					*ResultingDataType = eInteger;
					break;
			}

		return eCompileNoError;
	}


/* generate code for a unary operator. returns True if successful, or False if it fails. */
MyBoolean				CodeGenUnaryOperator(struct PcodeRec* FuncCode,
									long* StackDepthParam, ASTUnaryOpRec* UnaryOperator)
	{
		long					StackDepth;
		Pcodes				Opcode;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(UnaryOperator);
		StackDepth = *StackDepthParam;

		/* evaluate the argument */
		if (!CodeGenExpression(FuncCode,&StackDepth,UnaryOperator->Argument))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenUnaryOperator:  stack depth error after evaluating argument"));

		/* generate the operation code */
		switch (UnaryOperator->Operation)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator:  bad opcode"));
					break;

				case eUnaryNegation:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryNegation]:  "
									"bad type"));
								break;
							case eInteger:
								Opcode = epOperationIntegerNegation;
								break;
							case eFloat:
								Opcode = epOperationFloatNegation;
								break;
							case eDouble:
								Opcode = epOperationDoubleNegation;
								break;
							case eFixed:
								Opcode = epOperationFixedNegation;
								break;
						}
					break;

				case eUnaryNot:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryNot]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanNot;
								break;
							case eInteger:
								Opcode = epOperationIntegerNot;
								break;
						}
					break;

				case eUnarySine:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnarySine]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleSin;
								break;
						}
					break;

				case eUnaryCosine:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCosine]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleCos;
								break;
						}
					break;

				case eUnaryTangent:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryTangent]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleTan;
								break;
						}
					break;

				case eUnaryArcSine:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryArcSine]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleAsin;
								break;
						}
					break;

				case eUnaryArcCosine:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryArcCosine]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleAcos;
								break;
						}
					break;

				case eUnaryArcTangent:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryArcTangent]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleAtan;
								break;
						}
					break;

				case eUnaryLogarithm:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryLogarithm]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleLn;
								break;
						}
					break;

				case eUnaryExponentiation:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryExponentiation]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleExp;
								break;
						}
					break;

				case eUnarySquare:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnarySquare]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleSqr;
								break;
						}
					break;

				case eUnarySquareRoot:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnarySquareRoot]:  "
									"bad type"));
								break;
							case eDouble:
								Opcode = epOperationDoubleSqrt;
								break;
						}
					break;

				case eUnaryCastToBoolean:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCastToBoolean]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epNop;
								break;
							case eInteger:
								Opcode = epOperationIntegerToBoolean;
								break;
							case eFloat:
								Opcode = epOperationFloatToBoolean;
								break;
							case eDouble:
								Opcode = epOperationDoubleToBoolean;
								break;
							case eFixed:
								Opcode = epOperationFixedToBoolean;
								break;
						}
					break;

				case eUnaryCastToInteger:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCastToInteger]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanToInteger;
								break;
							case eInteger:
								Opcode = epNop;
								break;
							case eFloat:
								Opcode = epOperationFloatToInteger;
								break;
							case eDouble:
								Opcode = epOperationDoubleToInteger;
								break;
							case eFixed:
								Opcode = epOperationFixedToInteger;
								break;
						}
					break;

				case eUnaryCastToSingle:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCastToSingle]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanToFloat;
								break;
							case eInteger:
								Opcode = epOperationIntegerToFloat;
								break;
							case eFloat:
								Opcode = epNop;
								break;
							case eDouble:
								Opcode = epOperationDoubleToFloat;
								break;
							case eFixed:
								Opcode = epOperationFixedToFloat;
								break;
						}
					break;

				case eUnaryCastToDouble:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCastToDouble]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanToDouble;
								break;
							case eInteger:
								Opcode = epOperationIntegerToDouble;
								break;
							case eFloat:
								Opcode = epOperationFloatToDouble;
								break;
							case eDouble:
								Opcode = epNop;
								break;
							case eFixed:
								Opcode = epOperationFixedToDouble;
								break;
						}
					break;

				case eUnaryCastToFixed:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryCastToFixed]:  "
									"bad type"));
								break;
							case eBoolean:
								Opcode = epOperationBooleanToFixed;
								break;
							case eInteger:
								Opcode = epOperationIntegerToFixed;
								break;
							case eFloat:
								Opcode = epOperationFloatToFixed;
								break;
							case eDouble:
								Opcode = epOperationDoubleToFixed;
								break;
							case eFixed:
								Opcode = epNop;
								break;
						}
					break;

				case eUnaryAbsoluteValue:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryAbsoluteValue]:  "
									"bad type"));
								break;
							case eInteger:
								Opcode = epOperationIntegerAbs;
								break;
							case eFloat:
								Opcode = epOperationFloatAbs;
								break;
							case eDouble:
								Opcode = epOperationDoubleAbs;
								break;
							case eFixed:
								Opcode = epOperationFixedAbs;
								break;
						}
					break;

				case eUnaryTestNegative:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryTestNegative]:  "
									"bad type"));
								break;
							case eInteger:
								Opcode = epOperationTestIntegerNegative;
								break;
							case eFloat:
								Opcode = epOperationTestFloatNegative;
								break;
							case eDouble:
								Opcode = epOperationTestDoubleNegative;
								break;
							case eFixed:
								Opcode = epOperationTestFixedNegative;
								break;
						}
					break;

				case eUnaryGetSign:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryGetSign]:  "
									"bad type"));
								break;
							case eInteger:
								Opcode = epOperationGetSignInteger;
								break;
							case eFloat:
								Opcode = epOperationGetSignFloat;
								break;
							case eDouble:
								Opcode = epOperationGetSignDouble;
								break;
							case eFixed:
								Opcode = epOperationGetSignFixed;
								break;
						}
					break;

				case eUnaryGetArrayLength:
					switch (GetExpressionsResultantType(UnaryOperator->Argument))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenUnaryOperator [eUnaryGetArrayLength]:  "
									"bad type"));
								break;
							case eArrayOfBoolean:
								Opcode = epGetBooleanArraySize;
								break;
							case eArrayOfInteger:
								Opcode = epGetIntegerArraySize;
								break;
							case eArrayOfFloat:
								Opcode = epGetFloatArraySize;
								break;
							case eArrayOfDouble:
								Opcode = epGetDoubleArraySize;
								break;
							case eArrayOfFixed:
								Opcode = epGetFixedArraySize;
								break;
						}
					break;
			}
		if (Opcode != epNop)
			{
				if (!AddPcodeInstruction(FuncCode,Opcode,NIL))
					{
						return False;
					}
			}

		*StackDepthParam = StackDepth;
		return True;
	}
