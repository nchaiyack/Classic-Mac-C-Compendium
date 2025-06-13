/* CodeCenter.c */
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

#include "CodeCenter.h"
#include "Array.h"
#include "Memory.h"
#include "FunctionCode.h"
#include "PcodeObject.h"
#include "DataMunging.h"


typedef struct
	{
		void*							Signature;
		FuncCodeRec*			Function;
	} CodeEntryRec;


struct CodeCenterRec
	{
		ArrayRec*					CodeList; /* array of CodeEntryRec's */
	};


/* create new object code storage database */
CodeCenterRec*			NewCodeCenter(void)
	{
		CodeCenterRec*		CodeCenter;

		CodeCenter = (CodeCenterRec*)AllocPtrCanFail(sizeof(CodeCenterRec),"CodeCenterRec");
		if (CodeCenter == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		CodeCenter->CodeList = NewArray();
		if (CodeCenter->CodeList == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)CodeCenter);
				goto FailurePoint1;
			}
		return CodeCenter;
	}


/* delete object code database and all objects in it */
void								DisposeCodeCenter(CodeCenterRec* CodeCenter)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(CodeCenter);
		Limit = ArrayGetLength(CodeCenter->CodeList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				CodeEntryRec*			CodeRecord;

				CodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				DisposeFunction(CodeRecord->Function);
				ReleasePtr((char*)CodeRecord);
			}
		DisposeArray(CodeCenter->CodeList);
		ReleasePtr((char*)CodeCenter);
	}


/* if we have the pcode, but not the function, then find it */
/* it returns NIL if the function couldn't be found. */
struct FuncCodeRec*	GetFunctionFromOpcode(CodeCenterRec* CodeCenter,
											union OpcodeRec* Opcode)
	{
		long							Limit;
		long							Scan;

		CheckPtrExistence(CodeCenter);
		Limit = ArrayGetLength(CodeCenter->CodeList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				CodeEntryRec*			CodeRecord;

				CodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				if (GetOpcodeFromPcode(GetFunctionPcode(CodeRecord->Function)) == Opcode)
					{
						return CodeRecord->Function;
					}
			}
		return NIL;
	}


/* obtain a handle for the named function */
struct FuncCodeRec*	ObtainFunctionHandle(CodeCenterRec* CodeCenter, char* FunctionName,
											long FuncNameLength)
	{
		long							Limit;
		long							Scan;

		CheckPtrExistence(CodeCenter);
		Limit = ArrayGetLength(CodeCenter->CodeList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				CodeEntryRec*			CodeRecord;
				char*							LocalFunctionName;

				CodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				LocalFunctionName = GetFunctionName(CodeRecord->Function);
				if (PtrSize(LocalFunctionName) == FuncNameLength)
					{
						if (MemEqu(LocalFunctionName,FunctionName,FuncNameLength))
							{
								return CodeRecord->Function;
							}
					}
			}
		return NIL;
	}


/* find out how many functions are known by the code center */
long								CodeCenterGetNumFunctions(CodeCenterRec* CodeCenter)
	{
		CheckPtrExistence(CodeCenter);
		return ArrayGetLength(CodeCenter->CodeList);
	}


/* delete all object code from a particular code module & delink references */
void								FlushModulesCompiledFunctions(CodeCenterRec* CodeCenter,
											void* Signature)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(CodeCenter);
		Limit = ArrayGetLength(CodeCenter->CodeList);
		Scan = 0;
		while (Scan < Limit)
			{
				CodeEntryRec*			CodeRecord;

				CodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				if (CodeRecord->Signature == Signature)
					{
						long							Index;

						/* unlink references to this function */
						for (Index = 0; Index < Limit; Index += 1)
							{
								CodeEntryRec*			OtherCodeRecord;

								OtherCodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,
									Index);
								PcodeUnlink(GetFunctionPcode(OtherCodeRecord->Function),
									GetFunctionName(CodeRecord->Function),
									GetFunctionPcode(CodeRecord->Function));
							}
						/* delete storage occupied by this function */
						DisposeFunction(CodeRecord->Function);
						ReleasePtr((char*)CodeRecord);
						/* delete the function from the array & adjust Limit (local array size) */
						ArrayDeleteElement(CodeCenter->CodeList,Scan);
						Limit -= 1;
					}
				 else
					{
						Scan += 1;
					}
			}
	}


/* get a list of functions owned by a specified code module */
struct ArrayRec*		GetListOfFunctionsForModule(CodeCenterRec* CodeCenter,
											void* Signature)
	{
		long							Scan;
		long							Limit;
		ArrayRec*					List;

		CheckPtrExistence(CodeCenter);
		List = NewArray();
		if (List == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Limit = ArrayGetLength(CodeCenter->CodeList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				CodeEntryRec*			CodeRecord;

				CodeRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				if (CodeRecord->Signature == Signature)
					{
						if (!ArrayAppendElement(List,CodeRecord->Function))
							{
								DisposeArray(List);
								goto FailurePoint1;
							}
					}
			}
		return List;
	}


/* find out if a function with the given name exists */
MyBoolean						CodeCenterHaveThisFunction(CodeCenterRec* CodeCenter,
											char* FunctionName, long FuncNameLength)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(CodeCenter);
		Limit = ArrayGetLength(CodeCenter->CodeList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				CodeEntryRec*			FuncRecord;
				char*							TestFuncName;

				FuncRecord = (CodeEntryRec*)ArrayGetElement(CodeCenter->CodeList,Scan);
				TestFuncName = GetFunctionName(FuncRecord->Function);
				if (PtrSize(TestFuncName) == FuncNameLength)
					{
						if (StrEqu(FunctionName,TestFuncName))
							{
								/* yup, function's been added before */
								return True;
							}
					}
			}
		return False;
	}


/* add this function to the code center.  it better not be in there already */
MyBoolean						AddFunctionToCodeCenter(CodeCenterRec* CodeCenter,
											struct FuncCodeRec* TheNewFunction, void* Signature)
	{
		CodeEntryRec*			FuncRecord;

		CheckPtrExistence(CodeCenter);
		CheckPtrExistence(TheNewFunction);
		ERROR(CodeCenterHaveThisFunction(CodeCenter,GetFunctionName(TheNewFunction),
			PtrSize(GetFunctionName(TheNewFunction))),PRERR(ForceAbort,
			"AddFunctionToCodeCenter:  function is already in the database"));
		FuncRecord = (CodeEntryRec*)AllocPtrCanFail(sizeof(CodeEntryRec),"CodeEntryRec");
		if (FuncRecord == NIL)
			{
			 FailurePoint1:
				return False;
			}
		FuncRecord->Signature = Signature;
		FuncRecord->Function = TheNewFunction;
		if (!ArrayAppendElement(CodeCenter->CodeList,FuncRecord))
			{
			 FailurePoint2:
				ReleasePtr((char*)FuncRecord);
				goto FailurePoint1;
			}
		return True;
	}
