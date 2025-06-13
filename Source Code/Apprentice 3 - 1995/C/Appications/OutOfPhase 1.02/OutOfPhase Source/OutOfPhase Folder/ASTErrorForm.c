/* ASTErrorForm.c */
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

#include "ASTErrorForm.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "ASTExpression.h"


struct ASTErrorFormRec
	{
		ASTExpressionRec*		ResumeCondition;
		char*								MessageString;
		long								LineNumber;
	};


/* create a new AST error form */
ASTErrorFormRec*		NewErrorForm(struct ASTExpressionRec* Expression, char* String,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTErrorFormRec*	ErrorForm;

		CheckPtrExistence(Expression);
		ErrorForm = (ASTErrorFormRec*)AllocTrackedBlock(sizeof(ASTErrorFormRec),TrashTracker);
		if (ErrorForm == NIL)
			{
				return NIL;
			}
		SetTag(ErrorForm,"ASTErrorFormRec");

		ErrorForm->ResumeCondition = Expression;
		ErrorForm->MessageString = String;
		ErrorForm->LineNumber = LineNumber;

		return ErrorForm;
	}


/* type check the error message node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckErrorForm(DataTypes* ResultingDataType,
											ASTErrorFormRec* ErrorMessage, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CompileErrors			Error;
		DataTypes					ResumeConditionType;

		CheckPtrExistence(ErrorMessage);
		CheckPtrExistence(TrashTracker);

		Error = TypeCheckExpression(&ResumeConditionType,ErrorMessage->ResumeCondition,
			ErrorLineNumber,TrashTracker);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		if (ResumeConditionType != eBoolean)
			{
				*ErrorLineNumber = ErrorMessage->LineNumber;
				return eCompileErrorNeedsBooleanArg;
			}

		*ResultingDataType = eBoolean;
		return eCompileNoError;
	}


/* generate code for an error thing.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenErrorForm(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTErrorFormRec* ErrorForm)
	{
		long							StackDepth;

		CheckPtrExistence(FuncCode);
		CheckPtrExistence(ErrorForm);
		StackDepth = *StackDepthParam;

		/* evaluate the resume condition */
		if (!CodeGenExpression(FuncCode,&StackDepth,ErrorForm->ResumeCondition))
			{
				return False;
			}
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenErrorForm:  stack depth error evaluating resume condition"));

		/* do the thing */
		if (!AddPcodeInstruction(FuncCode,epErrorTrap,NIL))
			{
				return False;
			}
		if (!AddPcodeOperandString(FuncCode,ErrorForm->MessageString,
			PtrSize(ErrorForm->MessageString)))
			{
				return False;
			}
		StackDepth -= 1; /* consume operand */
		ERROR(StackDepth != *StackDepthParam,PRERR(ForceAbort,
			"CodeGenErrorForm:  stack depth error after trap"));

		/* error instruction leaves a return code, after consuming its operand. */
		StackDepth += 1;
		ERROR(StackDepth != *StackDepthParam + 1,PRERR(ForceAbort,
			"CodeGenErrorForm:  stack depth error after pushing return value"));

		*StackDepthParam = StackDepth;
		return True;
	}
