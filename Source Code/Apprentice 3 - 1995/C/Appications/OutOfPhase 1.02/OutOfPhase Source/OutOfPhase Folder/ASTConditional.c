/* ASTConditional.c */
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

#include "ASTConditional.h"
#include "ASTExpression.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "PromotableTypeCheck.h"


struct ASTCondRec
	{
		ASTExpressionRec*		Conditional;
		ASTExpressionRec*		Consequent;
		ASTExpressionRec*		Alternate; /* may be NIL */
		long								LineNumber;
	};


/* create a new if node.  the Alternate can be NIL. */
ASTCondRec*					NewConditional(struct ASTExpressionRec* Conditional,
											struct ASTExpressionRec* Consequent,
											struct ASTExpressionRec* Alternate,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTCondRec*				MyCond;

		CheckPtrExistence(Conditional);
		CheckPtrExistence(Consequent);
		if (Alternate != NIL)
			{
				CheckPtrExistence(Alternate);
			}
		CheckPtrExistence(TrashTracker);
		MyCond = (ASTCondRec*)AllocTrackedBlock(sizeof(ASTCondRec),TrashTracker);
		if (MyCond == NIL)
			{
				return MyCond;
			}
		SetTag(MyCond,"ASTCondRec");

		MyCond->LineNumber = LineNumber;
		MyCond->Conditional = Conditional;
		MyCond->Consequent = Consequent;
		MyCond->Alternate = Alternate;

		return MyCond;
	}


/* type check the conditional node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckConditional(DataTypes* ResultingDataType,
											ASTCondRec* Conditional, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		DataTypes					ConditionalReturnType;
		DataTypes					ConsequentReturnType;

		CheckPtrExistence(Conditional);
		CheckPtrExistence(TrashTracker);

		Error = TypeCheckExpression(&ConditionalReturnType,Conditional->Conditional,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}
		if (ConditionalReturnType != eBoolean)
			{
				*ErrorLineNumber = Conditional->LineNumber;
				return eCompileConditionalMustBeBoolean;
			}

		Error = TypeCheckExpression(&ConsequentReturnType,Conditional->Consequent,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		if (Conditional->Alternate == NIL)
			{
				/* no else clause */
				*ResultingDataType = ConsequentReturnType;
				return eCompileNoError;
			}
		 else
			{
				DataTypes					AlternateReturnType;

				/* there is an else clause */
				Error = TypeCheckExpression(&AlternateReturnType,Conditional->Alternate,
					ErrorLineNumber,TrashTracker);
				if (Error != eCompileNoError)
					{
						return Error;
					}
				/* make sure the types can be promoted to each other */
				if (CanRightBeMadeToMatchLeft(ConsequentReturnType,AlternateReturnType))
					{
						if (MustRightBePromotedToLeft(ConsequentReturnType,AlternateReturnType))
							{
								ASTExpressionRec*		PromotedThing;

								/* alternate must be promoted to be same as consequent */
								PromotedThing = PromoteTheExpression(AlternateReturnType/*orig*/,
									ConsequentReturnType/*desired*/,Conditional->Alternate,
									Conditional->LineNumber,TrashTracker);
								if (PromotedThing == NIL)
									{
										*ErrorLineNumber = Conditional->LineNumber;
										return eCompileOutOfMemory;
									}
								Conditional->Alternate = PromotedThing;
								/* sanity check */
								Error = TypeCheckExpression(&AlternateReturnType/*obtain new type*/,
									Conditional->Alternate,ErrorLineNumber,TrashTracker);
								ERROR(Error != eCompileNoError,PRERR(ForceAbort,
									"TypeCheckConditional:  type promotion caused failure"));
								ERROR(ConsequentReturnType != AlternateReturnType,PRERR(ForceAbort,
									"TypeCheckConditional:  after type promotion, types are no"
									" longer the same"));
							}
					}
				else if (CanRightBeMadeToMatchLeft(AlternateReturnType,ConsequentReturnType))
					{
						if (MustRightBePromotedToLeft(AlternateReturnType,ConsequentReturnType))
							{
								ASTExpressionRec*		PromotedThing;

								/* consequent must be promoted to be same as alternate */
								PromotedThing = PromoteTheExpression(ConsequentReturnType/*orig*/,
									AlternateReturnType/*desired*/,Conditional->Consequent,
									Conditional->LineNumber,TrashTracker);
								if (PromotedThing == NIL)
									{
										*ErrorLineNumber = Conditional->LineNumber;
										return eCompileOutOfMemory;
									}
								Conditional->Consequent = PromotedThing;
								/* sanity check */
								Error = TypeCheckExpression(&ConsequentReturnType/*obtain new type*/,
									Conditional->Consequent,ErrorLineNumber,TrashTracker);
								ERROR(Error != eCompileNoError,PRERR(ForceAbort,
									"TypeCheckConditional:  type promotion caused failure"));
								ERROR(ConsequentReturnType != AlternateReturnType,PRERR(ForceAbort,
									"TypeCheckConditional:  after type promotion, types are no"
									" longer the same"));
							}
					}
				else
					{
						/* can't promote */
						*ErrorLineNumber = Conditional->LineNumber;
						return eCompileTypeMismatchBetweenThenAndElse;
					}
				ERROR(ConsequentReturnType != AlternateReturnType,PRERR(ForceAbort,
					"TypeCheckConditional:  Consequent and Alternate return types differ"));
				*ResultingDataType = ConsequentReturnType;
				return eCompileNoError;
			}
		EXECUTE(PRERR(ForceAbort,"TypeCheckConditional:  control reached end of function"));
	}


/* generate code for a conditional.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenConditional(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTCondRec* Conditional)
	{
		long							StackDepth;
		long							PatchLocationForConditionalBranch;
		long							PatchForConditionalEnd;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(Conditional);
		StackDepth = *StackDepthParam;

		/* evaluate the condition */
		if (!CodeGenExpression(FuncCode,&StackDepth,Conditional->Conditional))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenConditional:  stack bad after evaluating conditional"));

		/* perform branch to bad guy */
		if (!AddPcodeInstruction(FuncCode,epBranchIfZero,&PatchLocationForConditionalBranch))
			{
				return False;
			}
		if (!AddPcodeOperandInteger(FuncCode,-1/*not known yet*/))
			{
				return False;
			}
		StackDepth -= 1;
		ERROR(StackDepth != *StackDepthParam,PRERR(ForceAbort,
			"CodeGenConditional:  stack bad after performing conditional branch"));

		/* evaluate the true branch */
		if (!CodeGenExpression(FuncCode,&StackDepth,Conditional->Consequent))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenConditional:  stack bad after evaluating true branch"));
		if (!AddPcodeInstruction(FuncCode,epBranchUnconditional,&PatchForConditionalEnd))
			{
				return False;
			}
		if (!AddPcodeOperandInteger(FuncCode,-1/*not known yet*/))
			{
				return False;
			}

		StackDepth -= 1;

		/* patch the conditional branch */
		ResolvePcodeBranch(FuncCode,PatchLocationForConditionalBranch,
			PcodeGetNextAddress(FuncCode));

		/* evaluate the false branch */
		if (Conditional->Alternate != NIL)
			{
				/* there is a real live alternate */
				if (!CodeGenExpression(FuncCode,&StackDepth,Conditional->Alternate))
					{
						return False;
					}
			}
		 else
			{
				/* there is no alternate, so push zero or nil */
				switch (GetExpressionsResultantType(Conditional->Consequent))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"CodeGenConditional:  bad type for 0"));
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
			"CodeGenConditional:  stack depth bad after alternate"));

		/* resolve the then-skipover-else branch */
		ResolvePcodeBranch(FuncCode,PatchForConditionalEnd,PcodeGetNextAddress(FuncCode));

		*StackDepthParam = StackDepth;
		return True;
	}
