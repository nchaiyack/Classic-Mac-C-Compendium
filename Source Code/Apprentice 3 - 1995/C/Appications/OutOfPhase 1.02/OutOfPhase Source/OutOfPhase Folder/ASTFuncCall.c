/* ASTFuncCall.c */
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

#include "ASTFuncCall.h"
#include "ASTExpressionList.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "ASTExpression.h"
#include "SymbolTableEntry.h"
#include "SymbolList.h"
#include "PromotableTypeCheck.h"
#include "ASTOperand.h"


struct ASTFuncCallRec
	{
		ASTExprListRec*			ArgumentList;
		ASTExpressionRec*		FunctionGenerator;
		long								LineNumber;
	};


/* create a new function call node.  the argument list can be NIL if there are */
/* no arguments. */
ASTFuncCallRec*			NewFunctionCall(struct ASTExprListRec* ArgumentList,
											struct ASTExpressionRec* FunctionGeneratorExpression,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTFuncCallRec*		FuncCall;

		CheckPtrExistence(FunctionGeneratorExpression);
		if (ArgumentList != NIL)
			{
				CheckPtrExistence(ArgumentList);
			}
		CheckPtrExistence(TrashTracker);
		FuncCall = (ASTFuncCallRec*)AllocTrackedBlock(sizeof(ASTFuncCallRec),TrashTracker);
		if (FuncCall == NIL)
			{
				return NIL;
			}
		SetTag(FuncCall,"ASTFuncCallRec");

		FuncCall->LineNumber = LineNumber;
		FuncCall->ArgumentList = ArgumentList;
		FuncCall->FunctionGenerator = FunctionGeneratorExpression;

		return FuncCall;
	}


/* type check the function call node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckFunctionCall(DataTypes* ResultingDataType,
											ASTFuncCallRec* FunctionCall, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		SymbolRec*				FunctionSymbol;
		SymbolListRec*		FunctionArgumentStepper;
		ASTExprListRec*		ExpressionListStepper;

		CheckPtrExistence(FunctionCall);
		CheckPtrExistence(TrashTracker);

		/* get the symbol for the function */
		Error = ExpressionGetFunctionCallSymbol(&FunctionSymbol,
			FunctionCall->FunctionGenerator);
		if (Error != eCompileNoError)
			{
				*ErrorLineNumber = FunctionCall->LineNumber;
				return Error;
			}
		CheckPtrExistence(FunctionSymbol);

		FunctionArgumentStepper = GetSymbolFunctionArgList(FunctionSymbol);
		ExpressionListStepper = FunctionCall->ArgumentList;
		while ((FunctionArgumentStepper != NIL) && (ExpressionListStepper != NIL))
			{
				DataTypes					FormalType;
				DataTypes					ActualType;

				FormalType = GetSymbolVariableDataType(
					GetFirstFromSymbolList(FunctionArgumentStepper));
				Error = TypeCheckExpression(&ActualType,ExprListGetFirstExpr(
					ExpressionListStepper),ErrorLineNumber,TrashTracker);
				if (Error != eCompileNoError)
					{
						return Error;
					}
				if (!CanRightBeMadeToMatchLeft(FormalType,ActualType))
					{
						*ErrorLineNumber = FunctionCall->LineNumber;
						return eCompileArgumentTypeConflict;
					}
				if (MustRightBePromotedToLeft(FormalType,ActualType))
					{
						ASTExpressionRec*		PromotedThing;

						/* consequent must be promoted to be same as alternate */
						PromotedThing = PromoteTheExpression(ActualType/*orig*/,
							FormalType/*desired*/,ExprListGetFirstExpr(ExpressionListStepper),
							FunctionCall->LineNumber,TrashTracker);
						if (PromotedThing == NIL)
							{
								*ErrorLineNumber = FunctionCall->LineNumber;
								return eCompileOutOfMemory;
							}
						ExprListPutNewFirst(ExpressionListStepper,PromotedThing);
						/* sanity check */
						Error = TypeCheckExpression(&ActualType/*obtain new type*/,
							ExprListGetFirstExpr(ExpressionListStepper),ErrorLineNumber,TrashTracker);
						ERROR(Error != eCompileNoError,PRERR(ForceAbort,
							"TypeCheckFunctionCall:  type promotion caused failure"));
						ERROR(ActualType != FormalType,PRERR(ForceAbort,
							"TypeCheckFunctionCall:  after type promotion, types are no"
							" longer the same"));
					}

				/* advance to the next list thing */
				ExpressionListStepper = ExprListGetRestList(ExpressionListStepper);
				FunctionArgumentStepper = GetRestListFromSymbolList(FunctionArgumentStepper);
			}
		if ((ExpressionListStepper != NIL) || (FunctionArgumentStepper != NIL))
			{
				*ErrorLineNumber = FunctionCall->LineNumber;
				return eCompileWrongNumberOfArgsToFunction;
			}

		*ResultingDataType = GetSymbolFunctionReturnType(FunctionSymbol);
		return eCompileNoError;
	}


/* generate code for a function call. returns True if successful, or False if it fails. */
MyBoolean						CodeGenFunctionCall(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTFuncCallRec* FunctionCall)
	{
		long							StackDepth;
		SymbolRec*				FunctionSymbol;
		ASTOperandRec*		FunctionContainer;
		DataTypes*				DataTypeArray;
		SymbolListRec*		FormalArgumentList;
		long							Index;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(FunctionCall);
		StackDepth = *StackDepthParam;

		/* push word for return value */
		if (!AddPcodeInstruction(FuncCode,epStackAllocate,NIL))
			{
				return False;
			}
		StackDepth += 1;

		/* push each argument on stack; from left to right */
		if (!CodeGenExpressionListArguments(FuncCode,&StackDepth,FunctionCall->ArgumentList))
			{
				return False;
			}

		/* figure out how to call the function */
		FunctionContainer = GetOperandOutOfExpression(FunctionCall->FunctionGenerator);
		FunctionSymbol = GetSymbolFromOperand(FunctionContainer);
		if (!AddPcodeInstruction(FuncCode,epFuncCallUnresolved,NIL))
			{
				return False;
			}
		/* push function name */
		if (!AddPcodeOperandString(FuncCode,GetSymbolName(FunctionSymbol),
			PtrSize(GetSymbolName(FunctionSymbol))))
			{
				return False;
			}
		/* push a parameter list record for runtime checking */
		FormalArgumentList = GetSymbolFunctionArgList(FunctionSymbol);
		/* now that we have the list, let's just check the stack for consistency */
		ERROR(StackDepth != *StackDepthParam + 1 + GetSymbolListLength(FormalArgumentList),
			PRERR(ForceAbort,"CodeGenFunctionCall:  stack depth error after pushing args"));
		DataTypeArray = (DataTypes*)AllocPtrCanFail(sizeof(DataTypes)
			* GetSymbolListLength(FormalArgumentList),"Function Parameter Type List");
		if (DataTypeArray == NIL)
			{
				return False;
			}
		Index = 0;
		while (FormalArgumentList != NIL)
			{
				DataTypeArray[Index] = GetSymbolVariableDataType(
					GetFirstFromSymbolList(FormalArgumentList));
				Index += 1;
				FormalArgumentList = GetRestListFromSymbolList(FormalArgumentList);
			}
		if (!AddPcodeOperandDataTypeArray(FuncCode,DataTypeArray))
			{
				ReleasePtr((char*)DataTypeArray);
				return False;
			}
		ReleasePtr((char*)DataTypeArray);
		/* save the return type */
		if (!AddPcodeOperandInteger(FuncCode,GetSymbolFunctionReturnType(FunctionSymbol)))
			{
				return False;
			}
		/* make an instruction operand for the "reserved" thing */
		if (!AddPcodeOperandInteger(FuncCode,0))
			{
				return False;
			}

		/* increment stack pointer since there will be 1 returned value */
		(*StackDepthParam) += 1;

		return True;
	}
