/* FunctionCode.c */
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

#include "FunctionCode.h"
#include "Memory.h"
#include "SymbolList.h"
#include "SymbolTableEntry.h"


struct FuncCodeRec
	{
		char*							FunctionName;
		long							NumParameters;
		DataTypes*				ParameterTypeList;
		PcodeRec*					Code;
		DataTypes					ReturnType;
	};


/* dispose of a function object */
void							DisposeFunction(FuncCodeRec* Function)
	{
		CheckPtrExistence(Function);
		ReleasePtr(Function->FunctionName);
		ReleasePtr((char*)Function->ParameterTypeList);
		DisposePcode(Function->Code);
		ReleasePtr((char*)Function);
	}


/* get the actual code block for a function. */
struct PcodeRec*	GetFunctionPcode(FuncCodeRec* FuncCode)
	{
		CheckPtrExistence(FuncCode);
		return FuncCode->Code;
	}


/* get the name of a function (actual, not copy).  it's a pointer into the heap */
char*							GetFunctionName(FuncCodeRec* FuncCode)
	{
		CheckPtrExistence(FuncCode);
		return FuncCode->FunctionName;
	}


/* get the list of parameters for a function.  it returns a heap block array */
/* of data types with fields from left parameter to right parameter. */
DataTypes*				GetFunctionParameterTypeList(FuncCodeRec* FuncCode)
	{
		CheckPtrExistence(FuncCode);
		return FuncCode->ParameterTypeList;
	}


/* get the data type a function returns.  could be undefined. */
DataTypes					GetFunctionReturnType(FuncCodeRec* FuncCode)
	{
		CheckPtrExistence(FuncCode);
		return FuncCode->ReturnType;
	}


/* create a new function thing */
FuncCodeRec*			NewFunction(char* FuncName, long NameLength,
										struct SymbolListRec* Parameters, struct PcodeRec* PcodeThing,
										DataTypes ReturnType)
	{
		FuncCodeRec*		FuncCode;
		SymbolListRec*	FormalParameterScanner;
		long						ParamIndex;

		FuncCode = (FuncCodeRec*)AllocPtrCanFail(sizeof(FuncCodeRec),"FuncCodeRec");
		if (FuncCode == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		FuncCode->FunctionName = AllocPtrCanFail(NameLength,"FuncCodeRec:name");
		if (FuncCode->FunctionName == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)FuncCode);
				goto FailurePoint1;
			}
		CopyData(FuncName,FuncCode->FunctionName,NameLength);

		FuncCode->NumParameters = GetSymbolListLength(Parameters);
		FuncCode->ParameterTypeList = (DataTypes*)AllocPtrCanFail(sizeof(DataTypes)
			* FuncCode->NumParameters,"FuncCodeRec:ParameterTypeList");
		if (FuncCode->ParameterTypeList == NIL)
			{
			 FailurePoint3:
				ReleasePtr((char*)FuncCode->FunctionName);
				goto FailurePoint2;
			}
		FormalParameterScanner = Parameters;
		ParamIndex = 0;
		while (FormalParameterScanner != NIL)
			{
				FuncCode->ParameterTypeList[ParamIndex] = GetSymbolVariableDataType(
					GetFirstFromSymbolList(FormalParameterScanner));
				FormalParameterScanner = GetRestListFromSymbolList(FormalParameterScanner);
				ParamIndex += 1;
			}

		FuncCode->Code = PcodeThing;

		FuncCode->ReturnType = ReturnType;

		return FuncCode;
	}
