/* PcodeSystem.c */
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

#define SHOW_ME_OPCODEREC  /* we need to see the internals of OpcodeRec */
#define SHOW_ME_STACKELEMENT  /* we also need to be able to see into stack elements */
#include "PcodeSystem.h"
#include "Memory.h"
#include "EventLoop.h"
#include "DataMunging.h"
#include "Alert.h"
#include "FloatingPoint.h"
#include "PcodeStack.h"
#include "PcodeObject.h"
#include "CodeCenter.h"
#include "FunctionCode.h"


/* number of pcode execution cycles between relinquish CPU calls */
#if DEBUG
	#define WIZZBANGCOUNTINITIALIZER (1024L) /* give us some responsiveness when debugging */
#else
	#define WIZZBANGCOUNTINITIALIZER (65536L)
#endif


EvalErrors				EvaluatePcode(ParamStackRec* Prep, PcodeRec* Pcode,
										CodeCenterRec* CodeCenter, OpcodeRec** OffendingPcode,
										long* OffendingInstruction, void* Refcon,
										SampleErrors (*GetSampleLeftCopy)(void* Refcon, char* SampleName,
											largefixedsigned** SampleData),
										SampleErrors (*GetSampleRightCopy)(void* Refcon, char* SampleName,
											largefixedsigned** SampleData),
										SampleErrors (*GetSampleMiddleCopy)(void* Refcon, char* SampleName,
											largefixedsigned** SampleData),
										SampleErrors (*GetWaveTableFrameCount)(void* Refcon,
											char* WaveTableName, long* FrameCountOut),
										SampleErrors (*GetWaveTableTableCount)(void* Refcon,
											char* WaveTableName, long* TableCountOut),
										SampleErrors (*GetWaveTableCopy)(void* Refcon, char* WaveTableName,
											largefixedsigned** WaveTableData))
	{
		/* machine variables */
		StackElement*		Stack;
		long						StackSize;
		long						StackPtr;
		long						ProgramCounter;
		OpcodeRec*			CurrentProcedure;
		long						Index;
		long						Scan;
		EvalErrors			ErrorCode;
		long						WizzBangCount;
		long						FuncCallDepth;


		/* check everything out */
		ERROR(OffendingPcode == NIL,PRERR(ForceAbort,
			"EvaluatePcode:  OffendingPcode is NIL"));
		ERROR(OffendingInstruction == NIL,PRERR(ForceAbort,
			"EvaluatePcode:  OffendingInstruction is NIL"));
		CheckPtrExistence(Prep);
		CheckPtrExistence(Pcode);

		/* initialize variables */
		Stack = GetStackBase(Prep);
		StackSize = GetStackInitialSize(Prep);
		StackPtr = GetStackNumElements(Prep) - 1;
		ProgramCounter = 0;
		CurrentProcedure = GetOpcodeFromPcode(Pcode);
		FuncCallDepth = 0;

		/* main execution loop.  this ends when there is nothing on the stack and */
		/* something tries to execute a return from subroutine, which means the outermost */
		/* procedure is returning.  the final return value of the program will be in P. */
		WizzBangCount = WIZZBANGCOUNTINITIALIZER;
		while (1)
			{
				WizzBangCount -= 1;
				if (WizzBangCount == 0)
					{
						WizzBangCount = WIZZBANGCOUNTINITIALIZER;
						if (RelinquishCPUJudiciouslyCheckCancel())
							{
								ErrorCode = eEvalUserCancelled;
								goto ExceptionPoint;
							}
					}
				PRNGCHK(CurrentProcedure,&(CurrentProcedure[ProgramCounter].Opcode),
					sizeof(CurrentProcedure[ProgramCounter].Opcode));
				switch (CurrentProcedure[ProgramCounter++].Opcode)
					{

						default:
							EXECUTE(PRERR(ForceAbort,"EvaluatePcode:  unknown opcode evaluated"));
							break;

						case epFuncCallUnresolved:
							/* an unresolved function call.  try to resolve it */
							/*     -1           0               1           2             3   */
							/* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
							{
								FuncCodeRec*		Function;
								DataTypes*			ParameterList;
								long						Limit;

								/* special note: we can't dispose of what we get from CodeCenter */
								/* because it gives us the real thing, not a copy */
								PRNGCHK(CurrentProcedure,
									&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
									sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
								Function = ObtainFunctionHandle(CodeCenter,
									&(*(CurrentProcedure[ProgramCounter + 0].ImmediateString)),
									StrLen(CurrentProcedure[ProgramCounter + 0].ImmediateString));
								if (Function == NIL)
									{
										ErrorCode = eEvalUndefinedFunction;
										goto ExceptionPoint;
									}
								ParameterList = GetFunctionParameterTypeList(Function);
								PRNGCHK(CurrentProcedure,
									&(CurrentProcedure[ProgramCounter + 1].DataTypeArray),
									sizeof(CurrentProcedure[ProgramCounter + 1].DataTypeArray));
								Limit = PtrSize((char*)CurrentProcedure[ProgramCounter
									+ 1].DataTypeArray);
								if (PtrSize((char*)ParameterList) != Limit)
									{
										/* different number of parameters */
										ErrorCode = eEvalWrongNumParametersForFunction;
										goto ExceptionPoint;
									}
								Limit = Limit / sizeof(DataTypes);
								for (Index = 0; Index < Limit; Index += 1)
									{
										if (CurrentProcedure[ProgramCounter + 1].DataTypeArray[Index]
											!= ParameterList[Index])
											{
												/* parameters of different types */
												ErrorCode = eEvalWrongParameterType;
												goto ExceptionPoint;
											}
									}
								PRNGCHK(CurrentProcedure,
									&(CurrentProcedure[ProgramCounter + 2].ImmediateInteger),
									sizeof(CurrentProcedure[ProgramCounter + 2].ImmediateInteger));
								if ((DataTypes)(CurrentProcedure[ProgramCounter + 2].ImmediateInteger)
									!= GetFunctionReturnType(Function))
									{
										/* different return types */
										ErrorCode = eEvalWrongReturnType;
										goto ExceptionPoint;
									}
								/* finally, the function appears to be the one we want. */
								PRNGCHK(CurrentProcedure,
									&(CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr),
									sizeof(CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr));
								/* first, install the PcodeRec in the instruction */
								CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr
									= GetOpcodeFromPcode(GetFunctionPcode(Function));
								/* next, change the instruction to epFuncCallResolved */
								CurrentProcedure[ProgramCounter - 1].Opcode = epFuncCallResolved;
							}
							goto epFunctionCallResolvedPoint;

						case epFuncCallResolved:
						 epFunctionCallResolvedPoint:
							/* a function call whose destination is known. */
							/*     -1           0               1           2             3    */
							/* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr),
								sizeof(CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 3]
								.FunctionOpcodeRecPtr);
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epFuncCallResolved]: stack size inconsistency"));
								}
							/* store return address on stack.  the return address is the address */
							/* of the NEXT instruction. */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].ElementType = esReturnAddress;
							Stack[StackPtr].Data.ReturnAddress.Procedure = CurrentProcedure;
							Stack[StackPtr].Data.ReturnAddress.Index
								= ProgramCounter + 4; /* 4 words to skip */
							/* perform the jump */
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 3].FunctionOpcodeRecPtr);
							CurrentProcedure = CurrentProcedure[ProgramCounter + 3]
								.FunctionOpcodeRecPtr;
							ProgramCounter = 0;
							/* increment function call depth counter */
							FuncCallDepth += 1;
							break;

						case epErrorTrap:
							/*    -1            0    */
							/* <opcode> ^"<errorstring>" */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epErrorTrap]:  expected scalar on stack"));
							if (Stack[StackPtr].Data.Integer == 0)
								{
									/* non-recoverable error -- abort */
									(void)AskYesNoCancel("Resumable error trap encountered:  _",
										CurrentProcedure[ProgramCounter + 0].ImmediateString,
										"Abort","Abort",NIL);
									ErrorCode = eEvalErrorTrapEncountered;
									goto ExceptionPoint;
								}
							 else
								{
									if (AskYesNoCancel("Resumable error trap encountered:  _",
										CurrentProcedure[ProgramCounter + 0].ImmediateString,
										"Resume","Abort",NIL) != eYes)
										{
											ErrorCode = eEvalErrorTrapEncountered;
											goto ExceptionPoint;
										}
								}
							/* leave argument on stack */
							ProgramCounter += 1;
							break;

						case epBranchUnconditional:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							ProgramCounter = CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							break;

						case epBranchIfZero:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epBranchIfZero]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer == 0)
								{
									PRNGCHK(CurrentProcedure,
										&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
										sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
									ProgramCounter = CurrentProcedure[ProgramCounter + 0]
										.ImmediateInteger;
								}
							 else
								{
									ProgramCounter += 1;
								}
							StackPtr -= 1;
							break;

						case epBranchIfNotZero:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epBranchIfNotZero]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer != 0)
								{
									PRNGCHK(CurrentProcedure,
										&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
										sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
									ProgramCounter = CurrentProcedure[ProgramCounter + 0]
										.ImmediateInteger;
								}
							 else
								{
									ProgramCounter += 1;
								}
							StackPtr -= 1;
							break;

						case epReturnFromSubroutine:
							/* - pop return address from top of stack */
							if (FuncCallDepth == 0)
								{
									/* if we are returning from root, then it's the end */
									goto TotallyDonePoint;
								}
							FuncCallDepth -= 1;
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esReturnAddress),PRERR(ForceAbort,
								"EvaluatePcode [epReturnFromSubroutine]: Stack[0] is not a return address"));
							ProgramCounter = Stack[StackPtr].Data.ReturnAddress.Index;
							CheckPtrExistence(Stack[StackPtr].Data.ReturnAddress.Procedure);
							CurrentProcedure = Stack[StackPtr].Data.ReturnAddress.Procedure;
							EXECUTE(Stack[StackPtr].ElementType = esScalar;)
							StackPtr -= 1;
							break;

						/* arithmetic operations */
						/* the right hand argument for binary operators is on top of stack */
						case epOperationFixedEqual:
						case epOperationIntegerEqual:
						case epOperationBooleanEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer == Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedNotEqual:
						case epOperationIntegerNotEqual:
						case epOperationBooleanNotEqual:
						case epOperationBooleanXor:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerNotEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer != Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationIntegerXor:
						case epOperationFixedXor:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerXor]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer ^ Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationBooleanAnd:
						case epOperationIntegerAnd:
						case epOperationFixedAnd:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerAnd]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer & Stack[StackPtr].Data.Integer;
								/* note:  bitwise and! */
							StackPtr -= 1;
							break;
						case epOperationBooleanOr:
						case epOperationIntegerOr:
						case epOperationFixedOr:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerOr]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer | Stack[StackPtr].Data.Integer;
								/* note:  bitwise or! */
							StackPtr -= 1;
							break;
						case epOperationBooleanNot:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationBooleanNot]: bad stack element type"));
							Stack[StackPtr].Data.Integer = !Stack[StackPtr].Data.Integer;
							break;
						case epOperationIntegerNot:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerNot]: bad stack element type"));
							Stack[StackPtr].Data.Integer = ~Stack[StackPtr].Data.Integer;
							break;
						case epOperationBooleanToInteger:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationBooleanToInteger]: bad stack element type"));
							break;
						case epOperationTestFixedNegative:
						case epOperationTestIntegerNegative:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationTestIntegerNegative]: bad stack element type"));
							Stack[StackPtr].Data.Integer = (Stack[StackPtr].Data.Integer != 0);
							break;
						case epOperationTestFloatNegative:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationTestFloatNegative]: bad stack element type"));
							Stack[StackPtr].Data.Integer = (Stack[StackPtr].Data.Float != 0);
							break;
						case epOperationTestDoubleNegative:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationTestDoubleNegative]: bad stack element type"));
							Stack[StackPtr].Data.Integer = (Stack[StackPtr].Data.Double != 0);
							break;
						case epOperationGetSignFixed:
						case epOperationGetSignInteger:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationGetSignInteger]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									Stack[StackPtr].Data.Integer = -1;
								}
							else if (Stack[StackPtr].Data.Integer > 0)
								{
									Stack[StackPtr].Data.Integer = 1;
								}
							else
								{
									Stack[StackPtr].Data.Integer = 0;
								}
							break;
						case epOperationGetSignFloat:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationGetSignFloat]: bad stack element type"));
							if (Stack[StackPtr].Data.Float < 0)
								{
									Stack[StackPtr].Data.Integer = -1;
								}
							else if (Stack[StackPtr].Data.Float > 0)
								{
									Stack[StackPtr].Data.Integer = 1;
								}
							else
								{
									Stack[StackPtr].Data.Integer = 0;
								}
							break;
						case epOperationGetSignDouble:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationGetSignFloat]: bad stack element type"));
							if (Stack[StackPtr].Data.Double < 0)
								{
									Stack[StackPtr].Data.Integer = -1;
								}
							else if (Stack[StackPtr].Data.Double > 0)
								{
									Stack[StackPtr].Data.Integer = 1;
								}
							else
								{
									Stack[StackPtr].Data.Integer = 0;
								}
							break;
						case epOperationIntegerToFloat:
						case epOperationBooleanToFloat:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToFloat]: bad stack element type"));
							Stack[StackPtr].Data.Float = Stack[StackPtr].Data.Integer;
							break;
						case epOperationIntegerToDouble:
						case epOperationBooleanToDouble:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToDouble]: bad stack element type"));
							Stack[StackPtr].Data.Double = Stack[StackPtr].Data.Integer;
							break;
						case epOperationIntegerToFixed:
						case epOperationBooleanToFixed:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToFixed]: bad stack element type"));
							Stack[StackPtr].Data.Integer = int2largefixed(Stack[StackPtr].Data.Integer);
							break;
						case epOperationFixedNegation:
						case epOperationIntegerNegation:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerNegation]: bad stack element type"));
							Stack[StackPtr].Data.Integer = - Stack[StackPtr].Data.Integer;
							break;
						case epOperationFixedAdd:
						case epOperationIntegerAdd:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerAdd]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer + Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedSubtract:
						case epOperationIntegerSubtract:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerSubtract]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer - Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationIntegerMultiply:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerMultiply]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer * Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationIntegerDivide:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerDivide]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer == 0)
								{
									ErrorCode = eEvalDivideByZero;
									goto ExceptionPoint;
								}
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer / Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationIntegerImpreciseDivide:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerImpreciseDivide]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer == 0)
								{
									ErrorCode = eEvalDivideByZero;
									goto ExceptionPoint;
								}
							Stack[StackPtr - 1].Data.Double = (double)Stack[StackPtr - 1].Data.Integer
								/ (double)Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationIntegerModulo:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerModulo]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer == 0)
								{
									ErrorCode = eEvalDivideByZero;
									goto ExceptionPoint;
								}
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer % Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedAbs:
						case epOperationIntegerAbs:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerAbs]: bad stack element type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									Stack[StackPtr].Data.Integer = - Stack[StackPtr].Data.Integer;
								}
							break;
						case epOperationFixedShiftLeft:
						case epOperationIntegerShiftLeft:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerShiftLeft]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer << Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedShiftRight:
						case epOperationIntegerShiftRight:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerShiftRight]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer / (1L << Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationFixedGreaterThan:
						case epOperationIntegerGreaterThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerGreaterThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer > Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedLessThan:
						case epOperationIntegerLessThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerLessThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer < Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedGreaterThanOrEqual:
						case epOperationIntegerGreaterThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerGreaterThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer >= Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedLessThanOrEqual:
						case epOperationIntegerLessThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerLessThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Integer <= Stack[StackPtr].Data.Integer;
							StackPtr -= 1;
							break;
						case epOperationFixedToBoolean:
						case epOperationIntegerToBoolean:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToBoolean]: bad stack element type"));
							Stack[StackPtr].Data.Integer = Stack[StackPtr].Data.Integer != 0;
							break;
						case epOperationFloatAdd:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatAdd]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float + Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatSubtract:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatSubtract]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float - Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatNegation:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatNegation]: bad stack element type"));
							Stack[StackPtr].Data.Float = - Stack[StackPtr].Data.Float;
							break;
						case epOperationFloatMultiply:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatMultiply]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float * Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatDivide:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatDivide]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float / Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatShiftLeft:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatShiftLeft]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float * FPOWER(2,Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationFloatShiftRight:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatShiftRight]: bad stack element type"));
							Stack[StackPtr - 1].Data.Float =
								Stack[StackPtr - 1].Data.Float * FPOWER(2,-Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationFloatGreaterThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatGreaterThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float > Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatLessThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatLessThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float < Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatGreaterThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatGreaterThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float >= Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatLessThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatLessThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float <= Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float == Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatNotEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatNotEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Float != Stack[StackPtr].Data.Float;
							StackPtr -= 1;
							break;
						case epOperationFloatAbs:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatAbs]: bad stack element type"));
							if (Stack[StackPtr].Data.Float < 0)
								{
									Stack[StackPtr].Data.Float = - Stack[StackPtr].Data.Float;
								}
							break;
						case epOperationFloatToBoolean:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToBoolean]: bad stack element type"));
							Stack[StackPtr].Data.Integer = Stack[StackPtr].Data.Float != 0;
							break;
						case epOperationFloatToInteger:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToInteger]: bad stack element type"));
							Stack[StackPtr].Data.Integer = Stack[StackPtr].Data.Float;
							break;
						case epOperationFloatToDouble:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToDouble]: bad stack element type"));
							Stack[StackPtr].Data.Double = Stack[StackPtr].Data.Float;
							break;
						case epOperationFloatToFixed:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToFixed]: bad stack element type"));
							Stack[StackPtr].Data.Integer = double2largefixed(Stack[StackPtr].Data.Float);
							break;
						case epOperationDoubleAdd:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleAdd]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double + Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleSubtract:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleSubtract]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double - Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleNegation:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleNegation]: bad stack element type"));
							Stack[StackPtr].Data.Double = - Stack[StackPtr].Data.Double;
							break;
						case epOperationDoubleMultiply:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleMultiply]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double * Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleDivide:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleDivide]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double / Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleShiftLeft:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleShiftLeft]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double * DPOWER(2,Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationDoubleShiftRight:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleShiftLeft]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								Stack[StackPtr - 1].Data.Double * DPOWER(2,-Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationDoubleGreaterThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleGreaterThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double > Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleLessThan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleLessThan]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double < Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleGreaterThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleGreaterThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double >= Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleLessThanOrEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleLessThanOrEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double <= Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double == Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleNotEqual:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleNotEqual]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer =
								Stack[StackPtr - 1].Data.Double != Stack[StackPtr].Data.Double;
							StackPtr -= 1;
							break;
						case epOperationDoubleAbs:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleAbs]: bad stack element type"));
							if (Stack[StackPtr].Data.Double < 0)
								{
									Stack[StackPtr].Data.Double = - Stack[StackPtr].Data.Double;
								}
							break;
						case epOperationDoubleToBoolean:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToBoolean]: bad stack element type"));
							Stack[StackPtr].Data.Integer = Stack[StackPtr].Data.Double != 0;
							break;
						case epOperationDoubleToInteger:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToInteger]: bad stack element type"));
							Stack[StackPtr].Data.Integer = Stack[StackPtr].Data.Double;
							break;
						case epOperationDoubleToFloat:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToFloat]: bad stack element type"));
							Stack[StackPtr].Data.Float = Stack[StackPtr].Data.Double;
							break;
						case epOperationDoubleToFixed:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToFixed]: bad stack element type"));
							Stack[StackPtr].Data.Integer = double2largefixed(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleSin:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleSin]: bad stack element type"));
							Stack[StackPtr].Data.Double = DSIN(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleCos:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleCos]: bad stack element type"));
							Stack[StackPtr].Data.Double = DCOS(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleTan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleTan]: bad stack element type"));
							Stack[StackPtr].Data.Double = DTAN(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleAsin:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleAsin]: bad stack element type"));
							Stack[StackPtr].Data.Double = DASIN(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleAcos:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleAcos]: bad stack element type"));
							Stack[StackPtr].Data.Double = DACOS(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleAtan:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleAtan]: bad stack element type"));
							Stack[StackPtr].Data.Double = DATAN(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleLn:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleLn]: bad stack element type"));
							Stack[StackPtr].Data.Double = DLN(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleExp:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleExp]: bad stack element type"));
							Stack[StackPtr].Data.Double = DEXP(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleSqrt:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleSqrt]: bad stack element type"));
							Stack[StackPtr].Data.Double = DSQRT(Stack[StackPtr].Data.Double);
							break;
						case epOperationDoubleSqr:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleSqr]: bad stack element type"));
							Stack[StackPtr].Data.Double = Stack[StackPtr].Data.Double
								* Stack[StackPtr].Data.Double;
							break;
						case epOperationDoublePower:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoublePower]: bad stack element type"));
							Stack[StackPtr - 1].Data.Double =
								DPOWER(Stack[StackPtr - 1].Data.Double,Stack[StackPtr].Data.Double);
							StackPtr -= 1;
							break;
						case epOperationFixedMultiply:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedMultiply]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer = largefixedmult(
								Stack[StackPtr - 1].Data.Integer,Stack[StackPtr].Data.Integer);
							StackPtr -= 1;
							break;
						case epOperationFixedDivide:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 1].ElementType != esScalar)
								,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedDivide]: bad stack element type"));
							Stack[StackPtr - 1].Data.Integer = double2largefixed(
								largefixed2double(Stack[StackPtr - 1].Data.Integer)
								/ largefixed2double(Stack[StackPtr].Data.Integer));
							StackPtr -= 1;
							break;
						case epOperationFixedToInteger:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToInteger]: bad stack element type"));
							Stack[StackPtr].Data.Integer = largefixed2int(Stack[StackPtr].Data.Integer);
							break;
						case epOperationFixedToFloat:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToFloat]: bad stack element type"));
							Stack[StackPtr].Data.Float = largefixed2double(Stack[StackPtr].Data.Integer);
							break;
						case epOperationFixedToDouble:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToDouble]: bad stack element type"));
							Stack[StackPtr].Data.Double = largefixed2double(Stack[StackPtr].Data.Integer);
							break;

						/* other conversion operations */
						case epOperationBooleanToIntegerBuried:  /* <opcode> <stackindex> */
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationBooleanToIntegerBuried]:  not a scalar"));
							ProgramCounter += 1;
							break;
						case epOperationIntegerToFloatBuried:
						case epOperationBooleanToFloatBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToFloatBuried]:  not a scalar"));
							Stack[Index].Data.Float = Stack[Index].Data.Integer;
							ProgramCounter += 1;
							break;
						case epOperationIntegerToDoubleBuried:
						case epOperationBooleanToDoubleBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToDoubleBuried]:  not a scalar"));
							Stack[Index].Data.Double = Stack[Index].Data.Integer;
							ProgramCounter += 1;
							break;
						case epOperationIntegerToFixedBuried:
						case epOperationBooleanToFixedBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToFixedBuried]:  not a scalar"));
							Stack[Index].Data.Integer = int2largefixed(Stack[Index].Data.Integer);
							ProgramCounter += 1;
							break;
						case epOperationFixedToBooleanBuried:
						case epOperationIntegerToBooleanBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationIntegerToBooleanBuried]:  not a scalar"));
							Stack[Index].Data.Integer = Stack[Index].Data.Integer != 0;
							ProgramCounter += 1;
							break;
						case epOperationFloatToBooleanBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToBooleanBuried]:  not a scalar"));
							Stack[Index].Data.Integer = Stack[Index].Data.Float != 0;
							ProgramCounter += 1;
							break;
						case epOperationFloatToIntegerBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToIntegerBuried]:  not a scalar"));
							Stack[Index].Data.Integer = Stack[Index].Data.Float;
							ProgramCounter += 1;
							break;
						case epOperationFloatToDoubleBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToDoubleBuried]:  not a scalar"));
							Stack[Index].Data.Double = Stack[Index].Data.Float;
							ProgramCounter += 1;
							break;
						case epOperationFloatToFixedBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFloatToFixedBuried]:  not a scalar"));
							Stack[Index].Data.Integer = double2largefixed(Stack[Index].Data.Float);
							ProgramCounter += 1;
							break;
						case epOperationDoubleToBooleanBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToBooleanBuried]:  not a scalar"));
							Stack[Index].Data.Integer = Stack[Index].Data.Double != 0;
							ProgramCounter += 1;
							break;
						case epOperationDoubleToIntegerBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToIntegerBuried]:  not a scalar"));
							Stack[Index].Data.Integer = Stack[Index].Data.Double;
							ProgramCounter += 1;
							break;
						case epOperationDoubleToFloatBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToFloatBuried]:  not a scalar"));
							Stack[Index].Data.Float = Stack[Index].Data.Double;
							ProgramCounter += 1;
							break;
						case epOperationDoubleToFixedBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationDoubleToFixedBuried]:  not a scalar"));
							Stack[Index].Data.Integer = double2largefixed(Stack[Index].Data.Double);
							ProgramCounter += 1;
							break;
						case epOperationFixedToIntegerBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToIntegerBuried]:  not a scalar"));
							Stack[Index].Data.Integer = largefixed2int(Stack[Index].Data.Integer);
							ProgramCounter += 1;
							break;
						case epOperationFixedToFloatBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToFloatBuried]:  not a scalar"));
							Stack[Index].Data.Float = largefixed2double(Stack[Index].Data.Integer);
							ProgramCounter += 1;
							break;
						case epOperationFixedToDoubleBuried:
							/*    -1            0    */
							/* <opcode> <branchoffset> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epOperationFixedToDoubleBuried]:  not a scalar"));
							Stack[Index].Data.Double = largefixed2double(Stack[Index].Data.Integer);
							ProgramCounter += 1;
							break;

						/* array sizing stuff. */
						case epGetBooleanArraySize:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epGetBooleanArraySize]: bad stack element type"));
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
							Index = PtrSize((char*)(Stack[StackPtr].Data.ArrayHandle->Array));
							DisposeIfNotOnStack(Stack,StackPtr);
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Index;
							break;
						case epGetIntegerArraySize:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epGetIntegerArraySize]: bad stack element type"));
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
							Index = PtrSize((char*)(Stack[StackPtr].Data.ArrayHandle->Array))
								/ sizeof(long);
							DisposeIfNotOnStack(Stack,StackPtr);
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Index;
							break;
						case epGetFloatArraySize:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epGetFloatArraySize]: bad stack element type"));
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
							Index = PtrSize((char*)(Stack[StackPtr].Data.ArrayHandle->Array))
								/ sizeof(float);
							DisposeIfNotOnStack(Stack,StackPtr);
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Index;
							break;
						case epGetDoubleArraySize:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epGetDoubleArraySize]: bad stack element type"));
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
							Index = PtrSize((char*)(Stack[StackPtr].Data.ArrayHandle->Array))
								/ sizeof(double);
							DisposeIfNotOnStack(Stack,StackPtr);
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Index;
							break;
						case epGetFixedArraySize:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epGetFixedArraySize]: bad stack element type"));
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
							Index = PtrSize((char*)(Stack[StackPtr].Data.ArrayHandle->Array))
								/ sizeof(largefixedsigned);
							DisposeIfNotOnStack(Stack,StackPtr);
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Index;
							break;

						case epResizeBooleanArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epResizeBooleanArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epResizeBooleanArray2]:  array slot of wrong type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							if (Stack[StackPtr - 1].Data.ArrayHandle != NIL)
								{
									void*					OldReference;
									void*					NewReference;
									long					SizeDifference;

									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
									OldReference = Stack[StackPtr - 1].Data.ArrayHandle->Array;
									SizeDifference = Stack[StackPtr].Data.Integer
										- (PtrSize((char*)OldReference) / sizeof(char));
									NewReference = ResizePtr((char*)OldReference,
										Stack[StackPtr].Data.Integer);
									if (NewReference == NIL)
										{
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									/* now patch up the references */
									Stack[StackPtr - 1].Data.ArrayHandle->Array = NewReference;
									/* if we added to the array, then we have to zero those elements */
									if (SizeDifference > 0)
										{
											long					Limit;

											Limit = Stack[StackPtr].Data.Integer;
											for (Index = Limit - SizeDifference; Index < Limit; Index += 1)
												{
													PRNGCHK(NewReference,&(((char*)NewReference)[Index]),
														sizeof(char));
													((char*)NewReference)[Index] = 0;
												}
										}
								}
							 else
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							StackPtr -= 1;
							break;
						case epResizeIntegerArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epResizeIntegerArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epResizeIntegerArray]:  array slot of wrong type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							if (Stack[StackPtr - 1].Data.ArrayHandle != NIL)
								{
									void*					OldReference;
									void*					NewReference;
									long					SizeDifference;

									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
									OldReference = Stack[StackPtr - 1].Data.ArrayHandle->Array;
									SizeDifference = Stack[StackPtr].Data.Integer
										- (PtrSize((char*)OldReference) / sizeof(long));
									NewReference = ResizePtr((char*)OldReference,
										Stack[StackPtr].Data.Integer * sizeof(long));
									if (NewReference == NIL)
										{
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									/* now patch up the references */
									Stack[StackPtr - 1].Data.ArrayHandle->Array = NewReference;
									/* if we added to the array, then we have to zero those elements */
									if (SizeDifference > 0)
										{
											long					Limit;

											Limit = Stack[StackPtr].Data.Integer;
											for (Index = Limit - SizeDifference; Index < Limit; Index += 1)
												{
													PRNGCHK(NewReference,&(((long*)NewReference)[Index]),
														sizeof(long));
													((long*)NewReference)[Index] = 0;
												}
										}
								}
							 else
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							StackPtr -= 1;
							break;
						case epResizeFloatArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epResizeFloatArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epResizeFloatArray]:  array slot of wrong type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							if (Stack[StackPtr - 1].Data.ArrayHandle != NIL)
								{
									void*					OldReference;
									void*					NewReference;
									long					SizeDifference;

									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
									OldReference = Stack[StackPtr - 1].Data.ArrayHandle->Array;
									SizeDifference = Stack[StackPtr].Data.Integer
										- (PtrSize((char*)OldReference) / sizeof(float));
									NewReference = ResizePtr((char*)OldReference,
										Stack[StackPtr].Data.Integer * sizeof(float));
									if (NewReference == NIL)
										{
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									/* now patch up the references */
									Stack[StackPtr - 1].Data.ArrayHandle->Array = NewReference;
									/* if we added to the array, then we have to zero those elements */
									if (SizeDifference > 0)
										{
											long					Limit;

											Limit = Stack[StackPtr].Data.Integer;
											for (Index = Limit - SizeDifference; Index < Limit; Index += 1)
												{
													PRNGCHK(NewReference,&(((float*)NewReference)[Index]),
														sizeof(float));
													((float*)NewReference)[Index] = 0;
												}
										}
								}
							 else
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							StackPtr -= 1;
							break;
						case epResizeDoubleArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epResizeDoubleArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epResizeDoubleArray]:  array slot of wrong type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							if (Stack[StackPtr - 1].Data.ArrayHandle != NIL)
								{
									void*					OldReference;
									void*					NewReference;
									long					SizeDifference;

									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
									OldReference = Stack[StackPtr - 1].Data.ArrayHandle->Array;
									SizeDifference = Stack[StackPtr].Data.Integer
										- (PtrSize((char*)OldReference) / sizeof(double));
									NewReference = ResizePtr((char*)OldReference,
										Stack[StackPtr].Data.Integer * sizeof(double));
									if (NewReference == NIL)
										{
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									/* now patch up the references */
									Stack[StackPtr - 1].Data.ArrayHandle->Array = NewReference;
									/* if we added to the array, then we have to zero those elements */
									if (SizeDifference > 0)
										{
											long					Limit;

											Limit = Stack[StackPtr].Data.Integer;
											for (Index = Limit - SizeDifference; Index < Limit; Index += 1)
												{
													PRNGCHK(NewReference,&(((double*)NewReference)[Index]),
														sizeof(double));
													((double*)NewReference)[Index] = 0;
												}
										}
								}
							 else
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							StackPtr -= 1;
							break;
						case epResizeFixedArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epResizeFixedArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epResizeFixedArray]:  array slot of wrong type"));
							if (Stack[StackPtr].Data.Integer < 0)
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							if (Stack[StackPtr - 1].Data.ArrayHandle != NIL)
								{
									void*					OldReference;
									void*					NewReference;
									long					SizeDifference;

									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
									CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
									OldReference = Stack[StackPtr - 1].Data.ArrayHandle->Array;
									SizeDifference = Stack[StackPtr].Data.Integer
										- (PtrSize((char*)OldReference) / sizeof(largefixedsigned));
									NewReference = ResizePtr((char*)OldReference,
										Stack[StackPtr].Data.Integer * sizeof(largefixedsigned));
									if (NewReference == NIL)
										{
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									/* now patch up the references */
									Stack[StackPtr - 1].Data.ArrayHandle->Array = NewReference;
									/* if we added to the array, then we have to zero those elements */
									if (SizeDifference > 0)
										{
											long					Limit;

											Limit = Stack[StackPtr].Data.Integer;
											for (Index = Limit - SizeDifference; Index < Limit; Index += 1)
												{
													PRNGCHK(NewReference,
														&(((largefixedsigned*)NewReference)[Index]),
														sizeof(largefixedsigned));
													((largefixedsigned*)NewReference)[Index] = 0;
												}
										}
								}
							 else
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							StackPtr -= 1;
							break;

						case epStoreIntegerOnStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreIntegerOnStack]: bad stack element type"));
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreIntegerOnStack]:  array slot of wrong type"));
							Stack[Index].Data.Integer = Stack[StackPtr].Data.Integer;
							ProgramCounter += 1;
							/* don't pop the value from the stack though */
							break;
						case epStoreFloatOnStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreFloatOnStack]: bad stack element type"));
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreFloatOnStack]:  array slot of wrong type"));
							Stack[Index].Data.Float = Stack[StackPtr].Data.Float;
							ProgramCounter += 1;
							/* don't pop the value from the stack though */
							break;
						case epStoreDoubleOnStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreDoubleOnStack]: bad stack element type"));
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epStoreDoubleOnStack]:  array slot of wrong type"));
							Stack[Index].Data.Double = Stack[StackPtr].Data.Double;
							ProgramCounter += 1;
							/* don't pop the value from the stack though */
							break;
						case epStoreArrayOnStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreArrayOnStack]: bad stack element type"));
							/* this one has to make sure it deallocates an array that it overwrites. */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR((Stack[Index].ElementType != esScalar)
								&& (Stack[Index].ElementType != esArray),
								PRERR(ForceAbort,
								"EvaluatePcode [epStoreArrayOnStack]:  array slot of wrong type"));
							if ((Stack[Index].ElementType == esArray)
								&& (Stack[Index].Data.ArrayHandle != NIL))
								{
									DisposeIfOnStackOnlyOnce(Stack,StackPtr,Index);
								}
							Stack[Index].Data.ArrayHandle = Stack[StackPtr].Data.ArrayHandle;
							if (Stack[Index].Data.ArrayHandle != NIL)
								{
									/* increment the reference count */
									Stack[Index].Data.ArrayHandle->RefCount += 1;
								}
							Stack[Index].ElementType = esArray;
							ProgramCounter += 1;
							/* don't pop the value from the stack though */
							break;
						case epLoadIntegerFromStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadIntegerFromStack]:  array slot of wrong type"));
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epLoadIntegerFromStack]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Integer = Stack[Index].Data.Integer;
							ProgramCounter += 1;
							break;
						case epLoadFloatFromStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadFloatFromStack]:  array slot of wrong type"));
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epLoadFloatFromStack]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Float = Stack[Index].Data.Float;
							ProgramCounter += 1;
							break;
						case epLoadDoubleFromStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadDoubleFromStack]:  array slot of wrong type"));
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epLoadDoubleFromStack]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].ElementType = esScalar;
							Stack[StackPtr].Data.Double = Stack[Index].Data.Double;
							ProgramCounter += 1;
							break;
						case epLoadArrayFromStack:
							/*    -1         0    */
							/* <opcode> <stackindex> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = StackPtr + CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							PRNGCHK(Stack,&(Stack[Index]),sizeof(Stack[Index]));
							ERROR(Stack[Index].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadArrayFromStack]:  array slot of wrong type"));
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epLoadArrayFromStack]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = Stack[Index].Data.ArrayHandle;
							if (Stack[StackPtr].Data.ArrayHandle != NIL)
								{
									/* increment the reference count */
									Stack[StackPtr].Data.ArrayHandle->RefCount += 1;
								}
							Stack[StackPtr].ElementType = esArray;
							ProgramCounter += 1;
							break;

						case epStackPop:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							if ((Stack[StackPtr].ElementType == esArray)
								&& (Stack[StackPtr].Data.ArrayHandle != NIL))
								{
									DisposeIfNotOnStack(Stack,StackPtr);
								}
							StackPtr -= 1;
							break;

						case epStackAllocate:
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epStackAllocate]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].ElementType = esScalar;
							break;

						case epStackPopMultiple:
							/*     -1       0   */
							/* <opcode> <numwords> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							Index = CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							while (Index > 0)
								{
									PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
									if ((Stack[StackPtr].ElementType == esArray)
										&& (Stack[StackPtr].Data.ArrayHandle != NIL))
										{
											DisposeIfNotOnStack(Stack,StackPtr);
										}
									StackPtr -= 1;
									Index -= 1;
								}
							ProgramCounter += 1;
							break;

						/* deallocate multiple cells under the current top */
						case epStackDeallocateUnder:
							/*    -1        0   */
							/* <opcode> <numwords> */
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							/* get the number of cells to deallocate */
							Index = CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							Scan = StackPtr;
							while (Index > 0)
								{
									PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
									if ((Stack[StackPtr - 1].ElementType == esArray)
										&& (Stack[StackPtr - 1].Data.ArrayHandle != NIL))
										{
											DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
										}
									PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
									Stack[StackPtr - 1] = Stack[StackPtr];
									StackPtr -= 1;
									Index -= 1;
								}
							Stack[StackPtr] = Stack[Scan];
							ProgramCounter += 1;
							break;

						/* duplicate the top word of the stack */
						case epDuplicate:
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epLoadDoubleFromStack]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							Stack[StackPtr] = Stack[StackPtr - 1];
							if ((Stack[StackPtr].ElementType == esArray)
								&& (Stack[StackPtr].Data.ArrayHandle != NIL))
								{
									/* increment the reference count */
									Stack[StackPtr].Data.ArrayHandle->RefCount += 1;
								}
							break;

						/* no operation */
						case epNop:
							break;

						/* new array allocation procedures */
						case epMakeBooleanArray:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epMakeBooleanArray]: bad stack element type"));
							Index = Stack[StackPtr].Data.Integer;
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->Array = AllocPtrCanFail(Index,
								"EvalPcodeBoolArray");
							if (Stack[StackPtr].Data.ArrayHandle->Array == NIL)
								{
									ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
									Stack[StackPtr].Data.ArrayHandle = NIL;
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
							Stack[StackPtr].ElementType = esArray;
							for (Scan = 0; Scan < Index; Scan += 1)
								{
									PRNGCHK(Stack[StackPtr].Data.ArrayHandle->Array,
										&(((char*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan]),
										sizeof(char));
									((char*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan] = 0;
								}
							break;
						case epMakeIntegerArray:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epMakeIntegerArray]: bad stack element type"));
							Index = Stack[StackPtr].Data.Integer;
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->Array = AllocPtrCanFail(
								Index * sizeof(long),"EvalPcodeIntArray");
							if (Stack[StackPtr].Data.ArrayHandle->Array == NIL)
								{
									ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
									Stack[StackPtr].Data.ArrayHandle = NIL;
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
							Stack[StackPtr].ElementType = esArray;
							for (Scan = 0; Scan < Index; Scan += 1)
								{
									PRNGCHK(Stack[StackPtr].Data.ArrayHandle->Array,
										&(((long*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan]),
										sizeof(long));
									((long*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan] = 0;
								}
							break;
						case epMakeFloatArray:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epMakeFloatArray]: bad stack element type"));
							Index = Stack[StackPtr].Data.Integer;
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->Array = AllocPtrCanFail(
								Index * sizeof(float),"EvalPcodeFloatArray");
							if (Stack[StackPtr].Data.ArrayHandle->Array == NIL)
								{
									ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
									Stack[StackPtr].Data.ArrayHandle = NIL;
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
							Stack[StackPtr].ElementType = esArray;
							for (Scan = 0; Scan < Index; Scan += 1)
								{
									PRNGCHK(Stack[StackPtr].Data.ArrayHandle->Array,
										&(((float*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan]),
										sizeof(float));
									((float*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan] = 0;
								}
							break;
						case epMakeDoubleArray:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epMakeDoubleArray]: bad stack element type"));
							Index = Stack[StackPtr].Data.Integer;
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->Array = AllocPtrCanFail(
								Index * sizeof(double),"EvalPcodeDoubleArray");
							if (Stack[StackPtr].Data.ArrayHandle->Array == NIL)
								{
									ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
									Stack[StackPtr].Data.ArrayHandle = NIL;
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
							Stack[StackPtr].ElementType = esArray;
							for (Scan = 0; Scan < Index; Scan += 1)
								{
									PRNGCHK(Stack[StackPtr].Data.ArrayHandle->Array,
										&(((double*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan]),
										sizeof(double));
									((double*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan] = 0;
								}
							break;
						case epMakeFixedArray:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epMakeFixedArray]: bad stack element type"));
							Index = Stack[StackPtr].Data.Integer;
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->Array = AllocPtrCanFail(
								Index * sizeof(long),"EvalPcodeFixedArray");
							if (Stack[StackPtr].Data.ArrayHandle->Array == NIL)
								{
									ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
									Stack[StackPtr].Data.ArrayHandle = NIL;
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
							Stack[StackPtr].ElementType = esArray;
							for (Scan = 0; Scan < Index; Scan += 1)
								{
									PRNGCHK(Stack[StackPtr].Data.ArrayHandle->Array,
										&(((largefixedsigned*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan]),
										sizeof(largefixedsigned));
									((largefixedsigned*)(Stack[StackPtr].Data.ArrayHandle->Array))[Scan] = 0;
								}
							break;

						case epStoreBooleanIntoArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							PRNGCHK(Stack,&(Stack[StackPtr - 2]),sizeof(Stack[StackPtr - 2]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 2].ElementType != esScalar),PRERR(ForceAbort,
								"EvaluatePcode [epStoreBooleanIntoArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreBooleanIntoArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((char*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(char));
							((char*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer] = Stack[StackPtr - 2].Data.Integer;
							DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
							StackPtr -= 2; /* pop subscript and reference */
							break;
						case epStoreIntegerIntoArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							PRNGCHK(Stack,&(Stack[StackPtr - 2]),sizeof(Stack[StackPtr - 2]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 2].ElementType != esScalar),PRERR(ForceAbort,
								"EvaluatePcode [epStoreIntegerIntoArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreIntegerIntoArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(long)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((long*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(long));
							((long*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer] = Stack[StackPtr - 2].Data.Integer;
							DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
							StackPtr -= 2; /* pop subscript and reference */
							break;
						case epStoreFloatIntoArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							PRNGCHK(Stack,&(Stack[StackPtr - 2]),sizeof(Stack[StackPtr - 2]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 2].ElementType != esScalar),PRERR(ForceAbort,
								"EvaluatePcode [epStoreFloatIntoArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreFloatIntoArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(float)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((float*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(float));
							((float*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer] = Stack[StackPtr - 2].Data.Float;
							DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
							StackPtr -= 2; /* pop subscript and reference */
							break;
						case epStoreDoubleIntoArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							PRNGCHK(Stack,&(Stack[StackPtr - 2]),sizeof(Stack[StackPtr - 2]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 2].ElementType != esScalar),PRERR(ForceAbort,
								"EvaluatePcode [epStoreDoubleIntoArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreDoubleIntoArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(double)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((double*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(double));
							((double*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer] = Stack[StackPtr - 2].Data.Double;
							DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
							StackPtr -= 2; /* pop subscript and reference */
							break;
						case epStoreFixedIntoArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							PRNGCHK(Stack,&(Stack[StackPtr - 2]),sizeof(Stack[StackPtr - 2]));
							ERROR((Stack[StackPtr].ElementType != esScalar)
								|| (Stack[StackPtr - 2].ElementType != esScalar),PRERR(ForceAbort,
								"EvaluatePcode [epStoreFixedIntoArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epStoreFixedIntoArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(largefixedsigned)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((largefixedsigned*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(largefixedsigned));
							((largefixedsigned*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer] = Stack[StackPtr - 2].Data.Integer;
							DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
							StackPtr -= 2; /* pop subscript and reference */
							break;

						case epLoadBooleanFromArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadBooleanFromArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadBooleanFromArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((char*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(char));
							{
								long						BoolTemp;

								BoolTemp = ((char*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
									Stack[StackPtr].Data.Integer];
								DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
								StackPtr -= 1;
								Stack[StackPtr].ElementType = esScalar;
								Stack[StackPtr].Data.Integer = BoolTemp;
							}
							break;
						case epLoadIntegerFromArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadIntegerFromArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadIntegerFromArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(long)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((long*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(long));
							{
								long						IntegerTemp;

								IntegerTemp = ((long*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
									Stack[StackPtr].Data.Integer];
								DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
								StackPtr -= 1;
								Stack[StackPtr].ElementType = esScalar;
								Stack[StackPtr].Data.Integer = IntegerTemp;
							}
							break;
						case epLoadFloatFromArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadFloatFromArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadFloatFromArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(float)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((float*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(float));
							{
								float						FloatTemp;

								FloatTemp = ((float*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
									Stack[StackPtr].Data.Integer];
								DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
								StackPtr -= 1;
								Stack[StackPtr].ElementType = esScalar;
								Stack[StackPtr].Data.Float = FloatTemp;
							}
							break;
						case epLoadDoubleFromArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadDoubleFromArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadDoubleFromArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(double)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((double*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
								Stack[StackPtr].Data.Integer]),sizeof(double));
							{
								double						DoubleTemp;

								DoubleTemp = ((double*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))[
									Stack[StackPtr].Data.Integer];
								DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
								StackPtr -= 1;
								Stack[StackPtr].ElementType = esScalar;
								Stack[StackPtr].Data.Double = DoubleTemp;
							}
							break;
						case epLoadFixedFromArray2:
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							ERROR(Stack[StackPtr].ElementType != esScalar,PRERR(ForceAbort,
								"EvaluatePcode [epLoadFixedFromArray]: bad stack element type"));
							PRNGCHK(Stack,&(Stack[StackPtr - 1]),sizeof(Stack[StackPtr - 1]));
							ERROR(Stack[StackPtr - 1].ElementType != esArray,PRERR(ForceAbort,
								"EvaluatePcode [epLoadFixedFromArray]:  not an array"));
							if (Stack[StackPtr - 1].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalArrayDoesntExist;
									goto ExceptionPoint;
								}
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle);
							CheckPtrExistence(Stack[StackPtr - 1].Data.ArrayHandle->Array);
							if ((Stack[StackPtr].Data.Integer < 0) || (Stack[StackPtr].Data.Integer
								>= PtrSize((char*)Stack[StackPtr - 1].Data.ArrayHandle->Array)
								/ sizeof(largefixedsigned)))
								{
									ErrorCode = eEvalArraySubscriptOutOfRange;
									goto ExceptionPoint;
								}
							PRNGCHK(Stack[StackPtr - 1].Data.ArrayHandle->Array,
								&(((largefixedsigned*)(Stack[StackPtr - 1].Data.ArrayHandle->Array))
								[Stack[StackPtr].Data.Integer]),sizeof(largefixedsigned));
							{
								largefixedsigned		FixedTemp;

								FixedTemp = ((largefixedsigned*)(Stack[StackPtr - 1].Data
									.ArrayHandle->Array))[Stack[StackPtr].Data.Integer];
								DisposeIfOnStackOnlyOnce(Stack,StackPtr,StackPtr - 1);
								StackPtr -= 1;
								Stack[StackPtr].ElementType = esScalar;
								Stack[StackPtr].Data.Integer = FixedTemp;
							}
							break;

						/* load immediate values */
						case epLoadImmediateInteger:
							/*    -1        0   */
							/* <opcode> <integer>; also used for boolean & fixed */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epFuncCallResolved]: stack size inconsistency"));
								}
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateInteger),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateInteger));
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.Integer
								= CurrentProcedure[ProgramCounter + 0].ImmediateInteger;
							Stack[StackPtr].ElementType = esScalar;
							ProgramCounter += 1;
							break;
						case epLoadImmediateFloat:
							/*    -1        0   */
							/* <opcode> <integer>; also used for boolean & fixed */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epFuncCallResolved]: stack size inconsistency"));
								}
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateFloat),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateFloat));
							PRNGCHK(CurrentProcedure[ProgramCounter + 0].ImmediateFloat,
								&(*(CurrentProcedure[ProgramCounter + 0].ImmediateFloat)),sizeof(float));
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.Float
								= *(CurrentProcedure[ProgramCounter + 0].ImmediateFloat);
							Stack[StackPtr].ElementType = esScalar;
							ProgramCounter += 1;
							break;
						case epLoadImmediateDouble:
							/*    -1        0   */
							/* <opcode> <integer>; also used for boolean & fixed */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epFuncCallResolved]: stack size inconsistency"));
								}
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateDouble),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateDouble));
							PRNGCHK(CurrentProcedure[ProgramCounter + 0].ImmediateDouble,
								&(*(CurrentProcedure[ProgramCounter + 0].ImmediateDouble)),
								sizeof(double));
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.Double
								= *(CurrentProcedure[ProgramCounter + 0].ImmediateDouble);
							Stack[StackPtr].ElementType = esScalar;
							ProgramCounter += 1;
							break;
						case epLoadImmediateNILArray:
							/* <opcode> */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epFuncCallResolved]: stack size inconsistency"));
								}
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = NIL;
							Stack[StackPtr].ElementType = esArray;
							break;

						case epGetSampleLeftArray:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetSampleLeftArray]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							switch ((*GetSampleLeftCopy)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,
								(largefixedsigned**)&(Stack[StackPtr].Data.ArrayHandle->Array)))
								{
									case eEvalSampleNoError:
										CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
										Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
										Stack[StackPtr].ElementType = esArray;
										break;
									case eEvalSampleUndefined:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetSampleLeftArray]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;
						case epGetSampleRightArray:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetSampleRightArray]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							switch ((*GetSampleRightCopy)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,
								(largefixedsigned**)&(Stack[StackPtr].Data.ArrayHandle->Array)))
								{
									case eEvalSampleNoError:
										CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
										Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
										Stack[StackPtr].ElementType = esArray;
										break;
									case eEvalSampleUndefined:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetSampleRightArray]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;
						case epGetSampleMonoArray:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetSampleMonoArray]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							switch ((*GetSampleMiddleCopy)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,
								(largefixedsigned**)&(Stack[StackPtr].Data.ArrayHandle->Array)))
								{
									case eEvalSampleNoError:
										CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
										Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
										Stack[StackPtr].ElementType = esArray;
										break;
									case eEvalSampleUndefined:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetSampleMonoArray]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;
						case epGetWaveTableArray:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetWaveTableArray]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							Stack[StackPtr].Data.ArrayHandle = (ArrayHandleType*)AllocPtrCanFail(
								sizeof(ArrayHandleType),"ArrayHandleType");
							if (Stack[StackPtr].Data.ArrayHandle == NIL)
								{
									ErrorCode = eEvalOutOfMemory;
									goto ExceptionPoint;
								}
							switch ((*GetWaveTableCopy)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,
								(largefixedsigned**)&(Stack[StackPtr].Data.ArrayHandle->Array)))
								{
									case eEvalSampleNoError:
										CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
										Stack[StackPtr].Data.ArrayHandle->RefCount = 1;
										Stack[StackPtr].ElementType = esArray;
										break;
									case eEvalSampleUndefined:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetWaveTableArray]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;
						case epGetWaveTableFrames:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetWaveTableFrames]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							switch ((*GetWaveTableFrameCount)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,&(Stack[StackPtr].Data.Integer)))
								{
									case eEvalSampleNoError:
										break;
									case eEvalSampleUndefined:
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetWaveTableFrames]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;
						case epGetWaveTableTables:
							/*    -1           0    */
							/* <opcode> ^"<namestring>" */
							StackPtr += 1;
							if (StackPtr >= StackSize)
								{
									StackElement*			NewStack;

									NewStack = (StackElement*)ResizePtr((char*)Stack,(StackSize + 8)
										* sizeof(StackElement));
									if (NewStack == NIL)
										{
											StackPtr -= 1;
											ErrorCode = eEvalOutOfMemory;
											goto ExceptionPoint;
										}
									Stack = NewStack;
									StackSize += 8;
									ERROR(StackSize != PtrSize((char*)Stack) / sizeof(StackElement),
										PRERR(ForceAbort,"EvaluatePcode [epGetWaveTableTables]: stack size inconsistency"));
								}
							Stack[StackPtr].ElementType = esScalar;
							PRNGCHK(CurrentProcedure,
								&(CurrentProcedure[ProgramCounter + 0].ImmediateString),
								sizeof(CurrentProcedure[ProgramCounter + 0].ImmediateString));
							CheckPtrExistence(CurrentProcedure[ProgramCounter + 0].ImmediateString);
							PRNGCHK(Stack,&(Stack[StackPtr]),sizeof(Stack[StackPtr]));
							switch ((*GetWaveTableTableCount)(Refcon,CurrentProcedure[
								ProgramCounter + 0].ImmediateString,&(Stack[StackPtr].Data.Integer)))
								{
									case eEvalSampleNoError:
										break;
									case eEvalSampleUndefined:
										ErrorCode = eEvalGetSampleNotDefined;
										goto ExceptionPoint;
									case eEvalSampleWrongChannel:
										ErrorCode = eEvalGetSampleWrongChannelType;
										goto ExceptionPoint;
									case eEvalSampleNotEnoughMemoryToCopy:
										ErrorCode = eEvalOutOfMemory;
										goto ExceptionPoint;
									default:
										EXECUTE(PRERR(ForceAbort,
											"EvaluatePcode [epGetWaveTableTables]: unknown return code"));
										break;
								}
							ProgramCounter += 1;
							break;

					} /* end switch */
			} /* end while */


		/* when something bad happens, set the ErrorCode with the error code and */
		/* jump here.  this will release all allocated arrays on the stack and in */
		/* the registers, set the offending function pcode, and return */
	 ExceptionPoint:
		/* first, release all objects */
		while (StackPtr != GetStackNumElements(Prep) - 1)
			{
				/* pop all elements except those that were there originally */
				if ((Stack[StackPtr].ElementType == esArray)
					&& (Stack[StackPtr].Data.ArrayHandle != NIL))
					{
						DisposeIfNotOnStack(Stack,StackPtr);
					}
				StackPtr -= 1;
			}
		/* then set up the error values */
		*OffendingPcode = CurrentProcedure;
		*OffendingInstruction = ProgramCounter - 1;
		/* write back values that might have changed */
		SetStackInformation(Prep,StackSize,StackPtr + 1,Stack);
		return ErrorCode;


		/* when execution finishes, jump here.  The lowest element [0] in the stack will */
		/* have the return value, placed there according to calling conventions */
	 TotallyDonePoint:
		/* check stack pointer */
		ERROR(StackPtr != GetStackNumElements(Prep) - 1,PRERR(ForceAbort,
			"EvaluatePcode:  normal exit, but final stack pointer != initial stack pointer"));
		/* write back values that might have changed */
		SetStackInformation(Prep,StackSize,StackPtr + 1,Stack);
		/* return message that indicates everything went fine */
		return eEvalNoError;
	}


char*							GetPcodeErrorMessage(EvalErrors Error)
	{
		char*						ErrorString;

		switch (Error)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetPcodeErrorMessage:  unknown error code"));
					break;
				case eEvalUndefinedFunction:
					ErrorString = "Called an undefined function.";
					break;
				case eEvalErrorTrapEncountered:
					ErrorString = "Error trap encountered; user cancelled execution.";
					break;
				case eEvalUserCancelled:
					ErrorString = "User cancelled.";
					break;
				case eEvalDivideByZero:
					ErrorString = "Divide by zero.";
					break;
				case eEvalOutOfMemory:
					ErrorString = "Out of memory.";
					break;
				case eEvalArrayDoesntExist:
					ErrorString = "Use of unallocated (NIL) array.";
					break;
				case eEvalArraySubscriptOutOfRange:
					ErrorString = "Array subscript out of range.";
					break;
				case eEvalGetSampleNotDefined:
					ErrorString = "Attempt to access undefined sample or wave table.";
					break;
				case eEvalGetSampleWrongChannelType:
					ErrorString = "Attempt to load sample array with wrong channel type.";
					break;
				case eEvalWrongNumParametersForFunction:
					ErrorString = "Wrong number of parameters for function call.";
					break;
				case eEvalWrongParameterType:
					ErrorString = "Parameter for function call does not match function's type.";
					break;
				case eEvalWrongReturnType:
					ErrorString = "Return type for function call does not match function's type.";
					break;
			}
		return ErrorString;
	}
