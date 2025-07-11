/* ASTAssignment.c */
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

#include "ASTAssignment.h"
#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "PromotableTypeCheck.h"
#include "ASTOperand.h"
#include "SymbolTableEntry.h"
#include "ASTBinaryOperator.h"


struct ASTAssignRec
	{
		ASTExpressionRec*			LValueGenerator;
		ASTExpressionRec*			ObjectValue;
		long									LineNumber;
	};


/* create a new assignment node */
ASTAssignRec*				NewAssignment(struct ASTExpressionRec* LeftValue,
											struct ASTExpressionRec* RightValue,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTAssignRec*		Assignment;

		CheckPtrExistence(LeftValue);
		CheckPtrExistence(RightValue);
		CheckPtrExistence(TrashTracker);
		Assignment = (ASTAssignRec*)AllocTrackedBlock(sizeof(ASTAssignRec),TrashTracker);
		if (Assignment == NIL)
			{
				return NIL;
			}
		SetTag(Assignment,"ASTAssignRec");

		Assignment->LineNumber = LineNumber;
		Assignment->LValueGenerator = LeftValue;
		Assignment->ObjectValue = RightValue;

		return Assignment;
	}


/* type check the assignment node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckAssignment(DataTypes* ResultingDataType,
											ASTAssignRec* Assignment, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		DataTypes					RValueType;
		DataTypes					LValueType;

		CheckPtrExistence(Assignment);
		CheckPtrExistence(TrashTracker);

		Error = TypeCheckExpression(&RValueType,Assignment->ObjectValue,ErrorLineNumber,
			TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		Error = TypeCheckExpression(&LValueType,Assignment->LValueGenerator,ErrorLineNumber,
			TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		if (!CanRightBeMadeToMatchLeft(LValueType,RValueType))
			{
				*ErrorLineNumber = Assignment->LineNumber;
				return eCompileTypeMismatch;
			}

		if (MustRightBePromotedToLeft(LValueType,RValueType))
			{
				ASTExpressionRec*		ReplacementRValue;

				/* insert promotion operator above right hand side */
				ReplacementRValue = PromoteTheExpression(RValueType,LValueType,
					Assignment->ObjectValue,Assignment->LineNumber,TrashTracker);
				if (ReplacementRValue == NIL)
					{
						*ErrorLineNumber = Assignment->LineNumber;
						return eCompileOutOfMemory;
					}
				Assignment->ObjectValue = ReplacementRValue;
				/* sanity check */
				Error = TypeCheckExpression(&RValueType,Assignment->ObjectValue,ErrorLineNumber,
					TrashTracker);
				ERROR((Error != eCompileNoError),PRERR(ForceAbort,
					"TypeCheckAssignment:  type promotion caused failure"));
				ERROR(RValueType != LValueType,PRERR(ForceAbort,
					"TypeCheckAssignment:  after type promotion, types are no longer the same"));
			}

		/* make sure it's a valid lvalue */
		if (!IsExpressionValidLValue(Assignment->LValueGenerator))
			{
				*ErrorLineNumber = Assignment->LineNumber;
				return eCompileInvalidLValue;
			}

		*ResultingDataType = LValueType;
		return eCompileNoError;
	}


/* generate code for an assignment.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenAssignment(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTAssignRec* Assignment)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(Assignment);
		StackDepth = *StackDepthParam;

		/* find out what kind of assignment operation to perform */
		/*  - for single variables:  it stores the value from top of stack into the */
		/*    appropriate index, but does not pop the value. */
		/*  - for array variables:  the array index is computed and pushed on the */
		/*    stack.  then the array index is popped, and the element value is stored. */
		/*    the element value is NOT popped. */
		switch (WhatKindOfExpressionIsThis(Assignment->LValueGenerator))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenAssignment:  bad lvalue expression type"));
					break;

				case eExprOperand:
					{
						ASTOperandRec*		TheOperand;
						SymbolRec*				TheVariable;
						Pcodes						TheAssignmentOpcode;

						/* evaluate the value expression.  this leaves the result on the stack */
						if (!CodeGenExpression(FuncCode,&StackDepth,Assignment->ObjectValue))
							{
								return False;
							}
						ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
							"CodeGenAssignment:  CodeGenExpression made stack depth bad"));

						/* get the variable being assigned to */
						TheOperand = GetOperandOutOfExpression(Assignment->LValueGenerator);
						TheVariable = GetSymbolFromOperand(TheOperand);

						/* generate the assignment opcode word */
						switch (GetSymbolVariableDataType(TheVariable))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"CodeGenAssignment:  variable has unknown type"));
									break;
								case eBoolean:
								case eInteger:
								case eFixed:
									TheAssignmentOpcode = epStoreIntegerOnStack;
									break;
								case eFloat:
									TheAssignmentOpcode = epStoreFloatOnStack;
									break;
								case eDouble:
									TheAssignmentOpcode = epStoreDoubleOnStack;
									break;
								case eArrayOfBoolean:
								case eArrayOfInteger:
								case eArrayOfFloat:
								case eArrayOfDouble:
								case eArrayOfFixed:
									TheAssignmentOpcode = epStoreArrayOnStack;
									break;
							}
						if (!AddPcodeInstruction(FuncCode,TheAssignmentOpcode,NIL))
							{
								return False;
							}

						/* generate the assignment operand (destination variable index) */
						/* stack offsets are negative. */
						if (!AddPcodeOperandInteger(FuncCode,
							GetSymbolVariableStackLocation(TheVariable) - StackDepth))
							{
							}
					}
					break;

				case eExprBinaryOperator:
					{
						ASTBinaryOpRec*		TheBinaryOperator;
						ASTExpressionRec*	LeftOperand;
						ASTExpressionRec*	RightOperand;
						Pcodes						TheAssignmentOpcode;

						TheBinaryOperator = GetBinaryOperatorOutOfExpression(
							Assignment->LValueGenerator);
						LeftOperand = GetLeftOperandForBinaryOperator(TheBinaryOperator);
						RightOperand = GetRightOperandForBinaryOperator(TheBinaryOperator);
						/* the left operand is the array reference generator */
						/* the right operand is the array index generator. */

						/* 1. evaluate the expression that's going to be assigned, leave on stack */
						/* 2. evaluate array reference, leaving it on the stack */
						/* 3. evaluate array subscript, leaving it on the stack */
						/* 4. do assignment which */
						/*     - pops subscript */
						/*     - stores into array thing */
						/*     - value is left on stack */

						/* 1.  evaluate the value expression */
						if (!CodeGenExpression(FuncCode,&StackDepth,Assignment->ObjectValue))
							{
								return False;
							}
						ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
							"CodeGenAssignment:  eval array element new value messed up stack"));

						/* 2.  evaluate array reference */
						if (!CodeGenExpression(FuncCode,&StackDepth,LeftOperand))
							{
								return False;
							}
						ERROR(StackDepth != *StackDepthParam + 2,PRERR(ForceAbort,
							"CodeGenAssignment:  eval array reference messed up stack"));

						/* 3.  evaluate array subscript */
						if (!CodeGenExpression(FuncCode,&StackDepth,RightOperand))
							{
								return False;
							}
						ERROR(StackDepth != *StackDepthParam + 3,PRERR(ForceAbort,
							"CodeGenAssignment:  eval array subscript messed up stack"));

						/* 4.  generate the opcode for storing into an array */
						switch (GetExpressionsResultantType(LeftOperand))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"CodeGenAssignment:  array assignment bad result type"));
									break;
								case eArrayOfBoolean:
									TheAssignmentOpcode = epStoreBooleanIntoArray2;
									break;
								case eArrayOfInteger:
									TheAssignmentOpcode = epStoreIntegerIntoArray2;
									break;
								case eArrayOfFloat:
									TheAssignmentOpcode = epStoreFloatIntoArray2;
									break;
								case eArrayOfDouble:
									TheAssignmentOpcode = epStoreDoubleIntoArray2;
									break;
								case eArrayOfFixed:
									TheAssignmentOpcode = epStoreFixedIntoArray2;
									break;
							}
						if (!AddPcodeInstruction(FuncCode,TheAssignmentOpcode,NIL))
							{
								return False;
							}
						StackDepth -= 2; /* popping the array subscript */
						ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
							"CodeGenAssignment:  eval store into array internal stack messing"));
					}
					break;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenAssignment:  after assignment, stack state is bad"));

		*StackDepthParam = StackDepth;
		return True;
	}
