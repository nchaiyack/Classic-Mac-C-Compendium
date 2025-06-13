/* ASTVariableDeclaration.c */
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

#include "ASTVariableDeclaration.h"
#include "ASTExpression.h"
#include "SymbolTableEntry.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "PromotableTypeCheck.h"


struct ASTVarDeclRec
	{
		SymbolRec*					SymbolTableEntry;
		ASTExpressionRec*		InitializationExpression;
		long								LineNumber;
	};


/* allocate a new variable declaration.  if the Initializer expression is NIL, then */
/* the object is initialized to NIL or zero when it enters scope. */
ASTVarDeclRec*			NewVariableDeclaration(struct SymbolRec* SymbolTableEntry,
											struct ASTExpressionRec* Initializer,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTVarDeclRec*		VarDecl;

		CheckPtrExistence(SymbolTableEntry);
		CheckPtrExistence(Initializer);
		VarDecl = (ASTVarDeclRec*)AllocTrackedBlock(sizeof(ASTVarDeclRec),TrashTracker);
		if (VarDecl == NIL)
			{
				return NIL;
			}
		SetTag(VarDecl,"ASTVarDeclRec");

		VarDecl->LineNumber = LineNumber;
		VarDecl->SymbolTableEntry = SymbolTableEntry;
		VarDecl->InitializationExpression = Initializer;

		return VarDecl;
	}


/* type check the variable declaration node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckVariableDeclaration(DataTypes* ResultingDataType,
											ASTVarDeclRec* VariableDeclaration, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;

		CheckPtrExistence(VariableDeclaration);
		CheckPtrExistence(TrashTracker);

		if (VariableDeclaration->InitializationExpression == NIL)
			{
				/* no initializer -- default */
				*ResultingDataType = GetSymbolVariableDataType(
					VariableDeclaration->SymbolTableEntry);
				return eCompileNoError;
			}
		 else
			{
				DataTypes					InitializerType;
				DataTypes					VariableType;

				/* initializer checking */
				VariableType = GetSymbolVariableDataType(VariableDeclaration->SymbolTableEntry);
				Error = TypeCheckExpression(&InitializerType,
					VariableDeclaration->InitializationExpression,ErrorLineNumber,TrashTracker);
				if (Error != eCompileNoError)
					{
						return Error;
					}
				if (!CanRightBeMadeToMatchLeft(VariableType,InitializerType))
					{
						*ErrorLineNumber = VariableDeclaration->LineNumber;
						return eCompileTypeMismatchInAssignment;
					}
				if (MustRightBePromotedToLeft(VariableType,InitializerType))
					{
						ASTExpressionRec*		PromotedInitializer;

						/* promote the initializer */
						PromotedInitializer = PromoteTheExpression(InitializerType/*orig*/,
							VariableType/*desired*/,VariableDeclaration->InitializationExpression,
							VariableDeclaration->LineNumber,TrashTracker);
						if (PromotedInitializer == NIL)
							{
								*ErrorLineNumber = VariableDeclaration->LineNumber;
								return eCompileOutOfMemory;
							}
						VariableDeclaration->InitializationExpression = PromotedInitializer;
						/* sanity check */
						Error = TypeCheckExpression(&InitializerType/*obtain new right type*/,
							VariableDeclaration->InitializationExpression,ErrorLineNumber,
							TrashTracker);
						ERROR((Error != eCompileNoError),PRERR(ForceAbort,
							"TypeCheckVariableDeclaration:  type promotion caused failure"));
					}
				ERROR(VariableType != InitializerType,PRERR(ForceAbort,
					"TypeCheckVariableDeclaration:  after type promotion, types are no"
					" longer the same"));
				*ResultingDataType = InitializerType;
				return eCompileNoError;
			}
		EXECUTE(PRERR(ForceAbort,"TypeCheckVariableDeclaration:  control reached end"));
	}


/* generate code for a variable declaration. returns True if successful, or */
/* False if it fails. */
MyBoolean						CodeGenVarDecl(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTVarDeclRec* VariableDeclaration)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(VariableDeclaration);
		StackDepth = *StackDepthParam;

		/* evaluate the value initializer expression if there is one */
		if (VariableDeclaration->InitializationExpression != NIL)
			{
				/* there's a true initializer */
				if (!CodeGenExpression(FuncCode,&StackDepth,
					VariableDeclaration->InitializationExpression))
					{
						return False;
					}
			}
		 else
			{
				/* generate the default zero value for this type */
				switch (GetSymbolVariableDataType(VariableDeclaration->SymbolTableEntry))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"CodeGenVarDecl:  bad variable type"));
							break;
						case eBoolean:
						case eInteger:
						case eFixed:
							if (!AddPcodeInstruction(FuncCode,epLoadImmediateInteger,NIL))
								{
									return False;
								}
							if (!AddPcodeOperandInteger(FuncCode,0))
								{
									return False;
								}
							break;
						case eFloat:
							if (!AddPcodeInstruction(FuncCode,epLoadImmediateFloat,NIL))
								{
									return False;
								}
							if (!AddPcodeOperandFloat(FuncCode,0))
								{
									return False;
								}
							break;
						case eDouble:
							if (!AddPcodeInstruction(FuncCode,epLoadImmediateDouble,NIL))
								{
									return False;
								}
							if (!AddPcodeOperandDouble(FuncCode,0))
								{
									return False;
								}
							break;
						case eArrayOfBoolean:
						case eArrayOfInteger:
						case eArrayOfFloat:
						case eArrayOfDouble:
						case eArrayOfFixed:
							if (!AddPcodeInstruction(FuncCode,epLoadImmediateNILArray,NIL))
								{
									return False;
								}
							break;
					}
				StackDepth += 1;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenVarDecl:  stack depth error after evaluting initializer"));

		/* save the index of the stack into the variable thing yah */
		SetSymbolVariableStackLocation(VariableDeclaration->SymbolTableEntry,StackDepth);

		/* duplicate the value so there's something to return */
		if (!AddPcodeInstruction(FuncCode,epDuplicate,NIL))
			{
				return False;
			}
		StackDepth += 1;
		ERROR(StackDepth != *StackDepthParam + 2,PRERR(ForceAbort,
			"CodeGenVarDecl:  stack depth error after duplicating value for return"));

		*StackDepthParam = StackDepth;
		return True;
	}
