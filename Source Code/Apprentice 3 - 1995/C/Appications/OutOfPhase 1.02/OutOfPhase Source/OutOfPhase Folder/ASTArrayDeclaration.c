/* ASTArrayDeclaration.c */
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

#include "ASTArrayDeclaration.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "SymbolTableEntry.h"
#include "ASTExpression.h"
#include "PcodeObject.h"


struct ASTArrayDeclRec
	{
		SymbolRec*					SymbolTableEntry;
		ASTExpressionRec*		SizeExpression;
		long								LineNumber;
	};


/* create a new array variable constructor node.  this should ONLY be used for */
/* creating arrays.  variables that are initialized with an array that results from */
/* an expression should use ASTVariableDeclaration.  */
ASTArrayDeclRec*		NewArrayConstruction(struct SymbolRec* SymbolTableEntry,
											struct ASTExpressionRec* SizeExpression,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTArrayDeclRec*	ArrayThing;

		CheckPtrExistence(SymbolTableEntry);
		CheckPtrExistence(SizeExpression);
		CheckPtrExistence(TrashTracker);
		ERROR((GetSymbolVariableDataType(SymbolTableEntry) != eArrayOfBoolean)
			&& (GetSymbolVariableDataType(SymbolTableEntry) != eArrayOfInteger)
			&& (GetSymbolVariableDataType(SymbolTableEntry) != eArrayOfFloat)
			&& (GetSymbolVariableDataType(SymbolTableEntry) != eArrayOfDouble)
			&& (GetSymbolVariableDataType(SymbolTableEntry) != eArrayOfFixed),
			PRERR(ForceAbort,"NewArrayConstruction:  variable type is NOT an array"));
		ArrayThing = (ASTArrayDeclRec*)AllocTrackedBlock(sizeof(ASTArrayDeclRec),
			TrashTracker);
		if (ArrayThing == NIL)
			{
				return NIL;
			}
		SetTag(ArrayThing,"ASTArrayDeclRec");

		ArrayThing->LineNumber = LineNumber;
		ArrayThing->SymbolTableEntry = SymbolTableEntry;
		ArrayThing->SizeExpression = SizeExpression;

		return ArrayThing;
	}


/* type check the array variable constructor node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckArrayConstruction(DataTypes* ResultingDataType,
											ASTArrayDeclRec* ArrayConstructor, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		DataTypes					TheVariableType;
		DataTypes					SizeSpecifierType;
		CompileErrors			Error;

		CheckPtrExistence(ArrayConstructor);
		CheckPtrExistence(TrashTracker);

		if (WhatIsThisSymbol(ArrayConstructor->SymbolTableEntry) != eSymbolVariable)
			{
				*ErrorLineNumber = ArrayConstructor->LineNumber;
				return eCompileExpectedVariable;
			}

		TheVariableType = GetSymbolVariableDataType(ArrayConstructor->SymbolTableEntry);
		switch (TheVariableType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckArrayConstruction:  unknown data type"));
					break;
				case eBoolean:
				case eInteger:
				case eFloat:
				case eDouble:
				case eFixed:
					*ErrorLineNumber = ArrayConstructor->LineNumber;
					return eCompileExpectedArrayType;
				case eArrayOfBoolean:
				case eArrayOfInteger:
				case eArrayOfFloat:
				case eArrayOfDouble:
				case eArrayOfFixed:
					break;
			}

		Error = TypeCheckExpression(&SizeSpecifierType,ArrayConstructor->SizeExpression,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}
		if (SizeSpecifierType != eInteger)
			{
				*ErrorLineNumber = ArrayConstructor->LineNumber;
				return eCompileArraySizeSpecMustBeInteger;
			}

		*ResultingDataType = TheVariableType;
		return eCompileNoError;
	}


/* generate code for array declaration.  returns True if successful, or False if */
/* it fails. */
MyBoolean						CodeGenArrayConstruction(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTArrayDeclRec* ArrayConstructor)
	{
		long							StackDepth;
		Pcodes						OpcodeToGenerate;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(ArrayConstructor);
		StackDepth = *StackDepthParam;

		/* evaluate size expression, leaving result on stack */
		if (!CodeGenExpression(FuncCode,&StackDepth,ArrayConstructor->SizeExpression))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenArrayConstruction:  CodeGenExpression made stack depth error"));

		/* construct array operation.  this pops size, but pushes new array reference */
		switch (GetSymbolVariableDataType(ArrayConstructor->SymbolTableEntry))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenArrayConstruction:  bad variable type"));
					break;
				case eArrayOfBoolean:
					OpcodeToGenerate = epMakeBooleanArray;
					break;
				case eArrayOfInteger:
					OpcodeToGenerate = epMakeIntegerArray;
					break;
				case eArrayOfFloat:
					OpcodeToGenerate = epMakeFloatArray;
					break;
				case eArrayOfDouble:
					OpcodeToGenerate = epMakeDoubleArray;
					break;
				case eArrayOfFixed:
					OpcodeToGenerate = epMakeFixedArray;
					break;
			}
		if (!AddPcodeInstruction(FuncCode,OpcodeToGenerate,NIL))
			{
				return False;
			}

		/* now make the symbol table entry remember where on the stack it is. */
		SetSymbolVariableStackLocation(ArrayConstructor->SymbolTableEntry,StackDepth);

		/* duplicate the value for something to return */
		if (!AddPcodeInstruction(FuncCode,epDuplicate,NIL))
			{
				return False;
			}
		StackDepth += 1;
		ERROR(StackDepth != *StackDepthParam + 2,PRERR(ForceAbort,
			"CodeGenArrayConstruction:  stack depth error after duplicating value for return"));

		*StackDepthParam = StackDepth;
		return True;
	}
