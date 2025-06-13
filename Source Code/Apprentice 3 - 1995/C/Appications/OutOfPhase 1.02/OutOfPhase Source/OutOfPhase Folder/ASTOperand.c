/* ASTOperand.c */
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

#include "ASTOperand.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "SymbolTableEntry.h"


struct ASTOperandRec
	{
		ASTOperandType		Type;
		long							LineNumberOfSource;
		union
			{
				long							IntegerValue;
				MyBoolean					BooleanValue;
				float							SingleValue;
				double						DoubleValue;
				largefixedsigned	FixedValue;
#if 0
				char*							StringValue;
#endif
				SymbolRec*				SymbolTableEntry;
			} u;
	};


/* create a new integer literal */
ASTOperandRec*		NewIntegerLiteral(struct TrashTrackRec* TrashTracker,
										long IntegerLiteralValue, long LineNumber)
	{
		ASTOperandRec*	IntegerLit;

		CheckPtrExistence(TrashTracker);
		IntegerLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (IntegerLit == NIL)
			{
				return NIL;
			}
		SetTag(IntegerLit,"ASTOperandRec: NewIntegerLiteral");

		IntegerLit->Type = eASTOperandIntegerLiteral;
		IntegerLit->LineNumberOfSource = LineNumber;
		IntegerLit->u.IntegerValue = IntegerLiteralValue;

		return IntegerLit;
	}


/* create a new boolean literal */
ASTOperandRec*		NewBooleanLiteral(struct TrashTrackRec* TrashTracker,
										MyBoolean BooleanLiteralValue, long LineNumber)
	{
		ASTOperandRec*	BooleanLit;

		CheckPtrExistence(TrashTracker);
		BooleanLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (BooleanLit == NIL)
			{
				return NIL;
			}
		SetTag(BooleanLit,"ASTOperandRec: NewBooleanLiteral");

		BooleanLit->Type = eASTOperandBooleanLiteral;
		BooleanLit->LineNumberOfSource = LineNumber;
		BooleanLit->u.BooleanValue = BooleanLiteralValue;

		return BooleanLit;
	}


/* create a new single precision literal */
ASTOperandRec*		NewSingleLiteral(struct TrashTrackRec* TrashTracker,
										float SingleLiteralValue, long LineNumber)
	{
		ASTOperandRec*	SingleLit;

		CheckPtrExistence(TrashTracker);
		SingleLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (SingleLit == NIL)
			{
				return NIL;
			}
		SetTag(SingleLit,"ASTOperandRec: NewSingleLiteral");

		SingleLit->Type = eASTOperandSingleLiteral;
		SingleLit->LineNumberOfSource = LineNumber;
		SingleLit->u.SingleValue = SingleLiteralValue;

		return SingleLit;
	}


/* create a new double precision literal */
ASTOperandRec*		NewDoubleLiteral(struct TrashTrackRec* TrashTracker,
										double DoubleLiteralValue, long LineNumber)
	{
		ASTOperandRec*	DoubleLit;

		CheckPtrExistence(TrashTracker);
		DoubleLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (DoubleLit == NIL)
			{
				return NIL;
			}
		SetTag(DoubleLit,"ASTOperandRec: NewDoubleLiteral");

		DoubleLit->Type = eASTOperandDoubleLiteral;
		DoubleLit->LineNumberOfSource = LineNumber;
		DoubleLit->u.DoubleValue = DoubleLiteralValue;

		return DoubleLit;
	}


/* create a new fixed-point literal */
ASTOperandRec*		NewFixedLiteral(struct TrashTrackRec* TrashTracker,
										largefixedsigned FixedLiteralValue, long LineNumber)
	{
		ASTOperandRec*	FixedLit;

		CheckPtrExistence(TrashTracker);
		FixedLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (FixedLit == NIL)
			{
				return NIL;
			}
		SetTag(FixedLit,"ASTOperandRec: NewFixedLiteral");

		FixedLit->Type = eASTOperandFixedLiteral;
		FixedLit->LineNumberOfSource = LineNumber;
		FixedLit->u.FixedValue = FixedLiteralValue;

		return FixedLit;
	}


#if 0
/* create a new string literal.  the string should be a valid heap block and is */
/* stored in the structure (i.e. a copy is NOT made) */
ASTOperandRec*		NewStringLiteral(struct TrashTrackRec* TrashTracker,
										char* StringLiteralValue, long LineNumber)
	{
		ASTOperandRec*	StringLit;

		CheckPtrExistence(TrashTracker);
		CheckPtrExistence(StringLiteralValue);
		StringLit = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (StringLit == NIL)
			{
				return NIL;
			}
		SetTag(StringLit,"ASTOperandRec:  NewStringLiteral");

		StringLit->Type = eASTOperandStringLiteral;
		StringLit->LineNumberOfSource = LineNumber;
		StringLit->u.StringValue = StringLiteralValue;

		return StringLit;
	}
#endif


/* create a new symbol reference. */
ASTOperandRec*		NewSymbolReference(struct TrashTrackRec* TrashTracker,
										struct SymbolRec* SymbolTableEntry, long LineNumber)
	{
		ASTOperandRec*	VarRef;

		CheckPtrExistence(TrashTracker);
		CheckPtrExistence(SymbolTableEntry);
		VarRef = (ASTOperandRec*)AllocTrackedBlock(sizeof(ASTOperandRec),TrashTracker);
		if (VarRef == NIL)
			{
				return NIL;
			}
		SetTag(VarRef,"ASTOperandRec: NewSymbolReference");

		VarRef->Type = eASTOperandSymbol;
		VarRef->LineNumberOfSource = LineNumber;
		VarRef->u.SymbolTableEntry = SymbolTableEntry;

		return VarRef;
	}


/* find out if it is a symbol */
MyBoolean					IsOperandASymbol(ASTOperandRec* Operand)
	{
		CheckPtrExistence(Operand);
		return (Operand->Type == eASTOperandSymbol);
	}


/* get a symbol from the operand */
struct SymbolRec*	GetSymbolFromOperand(ASTOperandRec* Operand)
	{
		CheckPtrExistence(Operand);
		ERROR(!IsOperandASymbol(Operand),PRERR(ForceAbort,
			"GetSymbolFromOperand:  not a symbolic operand"));
		return Operand->u.SymbolTableEntry;
	}


/* type check the operand node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors			TypeCheckOperand(DataTypes* ResultingDataType,
										ASTOperandRec* Operand, long* ErrorLineNumber,
										struct TrashTrackRec* TrashTracker)
	{
		CheckPtrExistence(Operand);
		CheckPtrExistence(TrashTracker);

		switch (Operand->Type)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckOperand:  unknown operand kind"));
					break;
				case eASTOperandIntegerLiteral:
					*ResultingDataType = eInteger;
					break;
				case eASTOperandBooleanLiteral:
					*ResultingDataType = eBoolean;
					break;
				case eASTOperandSingleLiteral:
					*ResultingDataType = eFloat;
					break;
				case eASTOperandDoubleLiteral:
					*ResultingDataType = eDouble;
					break;
				case eASTOperandFixedLiteral:
					*ResultingDataType = eFixed;
					break;
				case eASTOperandSymbol:
					if (eSymbolVariable != WhatIsThisSymbol(Operand->u.SymbolTableEntry))
						{
							*ErrorLineNumber = Operand->LineNumberOfSource;
							return eCompileMustBeAVariableIdentifier;
						}
					*ResultingDataType = GetSymbolVariableDataType(Operand->u.SymbolTableEntry);
					break;
			}
		return eCompileNoError;
	}


/* find out what kind of operand this is */
ASTOperandType		OperandWhatIsIt(ASTOperandRec* TheOperand)
	{
		CheckPtrExistence(TheOperand);
		return TheOperand->Type;
	}


/* generate code for an operand. returns True if successful, or False if it fails. */
MyBoolean					CodeGenOperand(struct PcodeRec* FuncCode,
										long* StackDepthParam, ASTOperandRec* Operand)
	{
		long						StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(Operand);
		StackDepth = *StackDepthParam;

		switch (Operand->Type)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenOperand:  unknown operand type"));
					break;

				case eASTOperandIntegerLiteral:
					if (!AddPcodeInstruction(FuncCode,epLoadImmediateInteger,NIL))
						{
							return False;
						}
					if (!AddPcodeOperandInteger(FuncCode,Operand->u.IntegerValue))
						{
							return False;
						}
					break;
				case eASTOperandBooleanLiteral:
					if (!AddPcodeInstruction(FuncCode,epLoadImmediateInteger,NIL))
						{
							return False;
						}
					if (!AddPcodeOperandInteger(FuncCode,Operand->u.BooleanValue))
						{
							return False;
						}
					break;
				case eASTOperandSingleLiteral:
					if (!AddPcodeInstruction(FuncCode,epLoadImmediateFloat,NIL))
						{
							return False;
						}
					if (!AddPcodeOperandFloat(FuncCode,Operand->u.SingleValue))
						{
							return False;
						}
					break;
				case eASTOperandDoubleLiteral:
					if (!AddPcodeInstruction(FuncCode,epLoadImmediateDouble,NIL))
						{
							return False;
						}
					if (!AddPcodeOperandDouble(FuncCode,Operand->u.DoubleValue))
						{
							return False;
						}
					break;
				case eASTOperandFixedLiteral:
					if (!AddPcodeInstruction(FuncCode,epLoadImmediateInteger,NIL))
						{
							return False;
						}
					if (!AddPcodeOperandInteger(FuncCode,Operand->u.FixedValue))
						{
							return False;
						}
					break;

				/* this had better be a variable */
				case eASTOperandSymbol:
					switch (GetSymbolVariableDataType(Operand->u.SymbolTableEntry))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"CodeGenOperand:  bad variable type"));
								break;
							case eBoolean:
							case eInteger:
							case eFixed:
								if (!AddPcodeInstruction(FuncCode,epLoadIntegerFromStack,NIL))
									{
										return False;
									}
								break;
							case eFloat:
								if (!AddPcodeInstruction(FuncCode,epLoadFloatFromStack,NIL))
									{
										return False;
									}
								break;
							case eDouble:
								if (!AddPcodeInstruction(FuncCode,epLoadDoubleFromStack,NIL))
									{
										return False;
									}
								break;
							case eArrayOfBoolean:
							case eArrayOfInteger:
							case eArrayOfFloat:
							case eArrayOfDouble:
							case eArrayOfFixed:
								if (!AddPcodeInstruction(FuncCode,epLoadArrayFromStack,NIL))
									{
										return False;
									}
								break;
						}
					/* stack offsets are negative */
					if (!AddPcodeOperandInteger(FuncCode,GetSymbolVariableStackLocation(
						Operand->u.SymbolTableEntry) - StackDepth))
						{
							return False;
						}
					break;
			}
		StackDepth += 1;
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenOperand:  stack depth error after pushing operand"));

		*StackDepthParam = StackDepth;
		return True;
	}
