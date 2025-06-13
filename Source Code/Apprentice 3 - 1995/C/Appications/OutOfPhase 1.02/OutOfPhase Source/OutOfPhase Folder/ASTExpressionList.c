/* ASTExpressionList.c */
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

#include "ASTExpressionList.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "ASTExpression.h"


struct ASTExprListRec
	{
		struct ASTExpressionRec*		First;
		ASTExprListRec*							Rest;
	};


/* cons an AST expression onto a list */
ASTExprListRec*			ASTExprListCons(ASTExpressionRec* First, ASTExprListRec* Rest,
											struct TrashTrackRec* TrashTracker)
	{
		ASTExprListRec*		NewNode;

		if (First != NIL)
			{
				CheckPtrExistence(First);
			}
		if (Rest != NIL)
			{
				CheckPtrExistence(Rest);
			}

		NewNode = (ASTExprListRec*)AllocTrackedBlock(sizeof(ASTExprListRec),TrashTracker);
		if (NewNode == NIL)
			{
				return NIL;
			}
		SetTag(NewNode,"ASTExprListRec");

		NewNode->First = First;
		NewNode->Rest = Rest;

		return NewNode;
	}


/* type check a list of expressions.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckExprList(DataTypes* ResultingDataType,
											ASTExprListRec* ExpressionList, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;

		/* ExpressionList should NOT be NIL, since that's handled by the expression */
		/* container */
		CheckPtrExistence(ExpressionList);
		CheckPtrExistence(TrashTracker);

		/* the result type is filled in now */
		Error = TypeCheckExpression(ResultingDataType,ExpressionList->First,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* if there is another one, then do it */
		if (ExpressionList->Rest != NIL)
			{
				return TypeCheckExprList(ResultingDataType,ExpressionList->Rest,ErrorLineNumber,
					TrashTracker);
			}
		 else
			{
				return eCompileNoError;
			}
	}


/* get the first expression */
struct ASTExpressionRec*	ExprListGetFirstExpr(ASTExprListRec* ExpressionList)
	{
		CheckPtrExistence(ExpressionList);
		return ExpressionList->First;
	}


/* get the tail expression list */
ASTExprListRec*			ExprListGetRestList(ASTExprListRec* ExpressionList)
	{
		CheckPtrExistence(ExpressionList);
		return ExpressionList->Rest;
	}


/* install a new first in the list */
void								ExprListPutNewFirst(ASTExprListRec* ExpressionList,
											struct ASTExpressionRec* NewFirst)
	{
		CheckPtrExistence(ExpressionList);
		CheckPtrExistence(NewFirst);
		ExpressionList->First = NewFirst;
	}


/* this is a helper function for generating code */
static MyBoolean		CodeGenSequenceHelper(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExprListRec* ExpressionList)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(ExpressionList);
		StackDepth = *StackDepthParam;

		/* generate code for the first expression */
		if (!CodeGenExpression(FuncCode,&StackDepth,ExpressionList->First))
			{
				return False;
			}

		if (ExpressionList->Rest != NIL)
			{
				/* if there is another expression, then pop the value we just */
				/* calcuated & evaluate the next one */
				if (!AddPcodeInstruction(FuncCode,epStackPop,NIL))
					{
						return False;
					}
				StackDepth -= 1;
				if (!CodeGenSequenceHelper(FuncCode,&StackDepth,ExpressionList->Rest))
					{
						return False;
					}
			}
		/* else no next one, so just keep it */

		*StackDepthParam = StackDepth;
		return True;
	}


/* generate code for an expression list that is a series of sequential expressions. */
/* returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpressionListSequence(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExprListRec* ExpressionList)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(ExpressionList);
		StackDepth = *StackDepthParam;

		/* generate code for all of the expressions */
		if (!CodeGenSequenceHelper(FuncCode,&StackDepth,ExpressionList))
			{
				return False;
			}

		/* if there are any more than 1 additional value on the stack, then we */
		/* must pop all the other values off, since they are local variables */
		if (StackDepth - *StackDepthParam > 1)
			{
				if (!AddPcodeInstruction(FuncCode,epStackDeallocateUnder,NIL))
					{
						return False;
					}
				if (!AddPcodeOperandInteger(FuncCode,StackDepth - *StackDepthParam - 1))
					{
						return False;
					}
				StackDepth = *StackDepthParam + 1;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenExpressionListSequence:  stack messed up after recurrance call"));

		*StackDepthParam = StackDepth;
		return True;
	}


/* generate code for an argument list -- all args stay on the stack. */
/* returns True if successful, or False if it fails. */
MyBoolean						CodeGenExpressionListArguments(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTExprListRec* ExpressionList)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);

		/* see if there is even any code to be generated */
		if (ExpressionList == NIL)
			{
				/* nope */
				return True;
			}

		CheckPtrExistence(ExpressionList);
		StackDepth = *StackDepthParam;

		/* generate code for the first expression */
		if (!CodeGenExpression(FuncCode,&StackDepth,ExpressionList->First))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenExpressionListArguments:  stack messed up"));

		/* if there's another argument, then do it too */
		if (ExpressionList->Rest != NIL)
			{
				if (!CodeGenExpressionListArguments(FuncCode,&StackDepth,ExpressionList->Rest))
					{
						return False;
					}
			}

		*StackDepthParam = StackDepth;
		return True;
	}
