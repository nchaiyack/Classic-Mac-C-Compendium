/* ASTLoop.c */
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

#include "ASTLoop.h"
#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"


struct ASTLoopRec
	{
		LoopTypes						KindOfLoop;
		ASTExpressionRec*		ControlExpression;
		ASTExpressionRec*		BodyExpression;
		long								LineNumber;
	};


/* create a new loop node */
ASTLoopRec*					NewLoop(LoopTypes LoopType, struct ASTExpressionRec* ControlExpr,
											struct ASTExpressionRec* BodyExpr,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTLoopRec*				Loop;

		CheckPtrExistence(ControlExpr);
		CheckPtrExistence(BodyExpr);
		CheckPtrExistence(TrashTracker);
		Loop = (ASTLoopRec*)AllocTrackedBlock(sizeof(ASTLoopRec),TrashTracker);
		if (Loop == NIL)
			{
				return NIL;
			}
		SetTag(Loop,"ASTLoopRec");

		Loop->LineNumber = LineNumber;
		Loop->KindOfLoop = LoopType;
		Loop->ControlExpression = ControlExpr;
		Loop->BodyExpression = BodyExpr;

		return Loop;
	}


/* type check the loop node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckLoop(DataTypes* ResultingDataType,
											ASTLoopRec* Loop, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		DataTypes					ConditionalType;
		DataTypes					BodyType;

		CheckPtrExistence(Loop);
		CheckPtrExistence(TrashTracker);

		Error = TypeCheckExpression(&ConditionalType,Loop->ControlExpression,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}
		if (ConditionalType != eBoolean)
			{
				*ErrorLineNumber = Loop->LineNumber;
				return eCompileConditionalMustBeBoolean;
			}

		Error = TypeCheckExpression(&BodyType,Loop->BodyExpression,ErrorLineNumber,
			TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		*ResultingDataType = BodyType;
		return eCompileNoError;
	}


/* generate code for a loop. returns True if successful, or False if it fails. */
MyBoolean						CodeGenLoop(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTLoopRec* Loop)
	{
		long							StackDepth;
		long							WhileBranchPatchupLocation EXECUTE(= -1);
		long							LoopBackAgainLocation;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(Loop);
		StackDepth = *StackDepthParam;

		/* this is the loopie thing.  the only real difference between a while and */
		/* a repeat loop is that the while has an extra branch to the test. */
		/* repeat loop: */
		/*  - push default return value */
		/*  - pop previous value */
		/*  - evaluate body */
		/*  - perform test */
		/*  - branch if we keep looping to the pop previous value point */
		/* while loop: */
		/*  - push default return value */
		/*  - jump to the test */
		/*  - pop previous value */
		/*  - evaluate body */
		/*  - perform test */
		/*  - branch if we keep looping to the pop previous value point */

		/* push the default value -- same type as the body expression has */
		switch (GetExpressionsResultantType(Loop->BodyExpression))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenLoop:  bad type of body expression"));
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
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenLoop:  stack depth error after pushing default value"));

		/* if loop is a head-tested loop (while loop) then insert extra branch */
		if ((Loop->KindOfLoop == eLoopWhileDo) || (Loop->KindOfLoop == eLoopUntilDo))
			{
				if (!AddPcodeInstruction(FuncCode,epBranchUnconditional,
					&WhileBranchPatchupLocation))
					{
						return False;
					}
				if (!AddPcodeOperandInteger(FuncCode,-1/*target unknown*/))
					{
						return False;
					}
			}

		/* remember this address! */
		LoopBackAgainLocation = PcodeGetNextAddress(FuncCode);

		/* pop previous result */
		if (!AddPcodeInstruction(FuncCode,epStackPop,NIL))
			{
				return False;
			}
		StackDepth -= 1;
		ERROR(StackDepth != *StackDepthParam,PRERR(ForceAbort,
			"CodeGenLoop:  stack depth error after popping previous"));

		/* evaluate the body */
		if (!CodeGenExpression(FuncCode,&StackDepth,Loop->BodyExpression))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenLoop:  stack depth error after evaluating body"));

		/* patch up the while branch */
		if ((Loop->KindOfLoop == eLoopWhileDo) || (Loop->KindOfLoop == eLoopUntilDo))
			{
				ResolvePcodeBranch(FuncCode,WhileBranchPatchupLocation,
					PcodeGetNextAddress(FuncCode));
			}

		/* evaluate the test */
		if (!CodeGenExpression(FuncCode,&StackDepth,Loop->ControlExpression))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 2,PRERR(ForceAbort,
			"CodeGenLoop:  stack depth error after evaluating control"));

		/* do the appropriate branch */
		if ((Loop->KindOfLoop == eLoopWhileDo) || (Loop->KindOfLoop == eLoopDoWhile))
			{
				/* "while do" and "do while" need a branch if true conditional */
				if (!AddPcodeInstruction(FuncCode,epBranchIfNotZero,NIL))
					{
						return False;
					}
				if (!AddPcodeOperandInteger(FuncCode,LoopBackAgainLocation))
					{
						return False;
					}
			}
		 else
			{
				/* "until do" and "do until" need a branch if false conditional */
				if (!AddPcodeInstruction(FuncCode,epBranchIfZero,NIL))
					{
						return False;
					}
				if (!AddPcodeOperandInteger(FuncCode,LoopBackAgainLocation))
					{
						return False;
					}
			}
		StackDepth -= 1;
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenLoop:  stack depth error after control branch"));

		*StackDepthParam = StackDepth;
		return True;
	}
