/* PcodeObject.c */
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

#define SHOW_ME_OPCODEREC  /* we want to see the definition for OpcodeRec */
#include "PcodeObject.h"
#include "Memory.h"
#include "DataMunging.h"


#if DEBUG
	#define MINPCODEARRAYLENGTH (4) /* ridiculously small for debugging */
#else
	#define MINPCODEARRAYLENGTH (128)
#endif


struct PcodeRec
	{
		OpcodeRec*			OpcodeArray;
		long						NumInstructions;
		long						MaxArrayLength;
	};


PcodeRec*					NewPcode(void)
	{
		PcodeRec*				Pcode;

		Pcode = (PcodeRec*)AllocPtrCanFail(sizeof(PcodeRec),"PcodeRec");
		if (Pcode == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Pcode->OpcodeArray = (OpcodeRec*)AllocPtrCanFail(MINPCODEARRAYLENGTH
			* sizeof(OpcodeRec),"OpcodeRec");
		if (Pcode->OpcodeArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Pcode);
				goto FailurePoint1;
			}
		Pcode->NumInstructions = 0;
		Pcode->MaxArrayLength = MINPCODEARRAYLENGTH;
		return Pcode;
	}


static MyBoolean	ExtendPcodeLength(PcodeRec* Pcode)
	{
		CheckPtrExistence(Pcode);
		if (Pcode->NumInstructions == Pcode->MaxArrayLength)
			{
				long						NewLength;
				OpcodeRec*			NewVector;

				NewLength = Pcode->MaxArrayLength * 2 + 1;
				NewVector = (OpcodeRec*)ResizePtr((char*)Pcode->OpcodeArray,
					NewLength * sizeof(OpcodeRec));
				if (NewVector == NIL)
					{
						/* failed */
						return False;
					}
				Pcode->OpcodeArray = NewVector;
				Pcode->MaxArrayLength = NewLength;
			}
		return True;
	}


void							DisposePcode(PcodeRec* Pcode)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(Pcode);
		Limit = Pcode->NumInstructions;
		Scan = 0;
		while (Scan < Limit)
			{
				switch (Pcode->OpcodeArray[Scan].Opcode)
					{
						case epFuncCallUnresolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
						case epFuncCallResolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
							if (Scan + 1 < Limit)
								{
									/* conditional needed since it's possible they could have had an */
									/* error before they got a chance to put the operands */
									ReleasePtr(Pcode->OpcodeArray[Scan + 1].ImmediateString);
								}
							if (Scan + 2 < Limit)
								{
									ReleasePtr((char*)(Pcode->OpcodeArray[Scan + 2].DataTypeArray));
								}
							Scan += 5;
							break;
						case epErrorTrap: /* <opcode> ^"<errorstring>" */
							if (Scan + 1 < Limit)
								{
									ReleasePtr(Pcode->OpcodeArray[Scan + 1].ImmediateString);
								}
							Scan += 2;
							break;
						case epOperationBooleanEqual: /* <opcode> */
						case epOperationBooleanNotEqual:
						case epOperationBooleanAnd:
						case epOperationBooleanOr:
						case epOperationBooleanNot:
						case epOperationBooleanToInteger:
						case epOperationBooleanToFloat:
						case epOperationBooleanToDouble:
						case epOperationBooleanToFixed:
						case epOperationIntegerAdd:
						case epOperationIntegerSubtract:
						case epOperationIntegerNegation:
						case epOperationIntegerMultiply:
						case epOperationIntegerDivide:
						case epOperationIntegerModulo:
						case epOperationIntegerShiftLeft:
						case epOperationIntegerShiftRight:
						case epOperationIntegerGreaterThan:
						case epOperationIntegerLessThan:
						case epOperationIntegerGreaterThanOrEqual:
						case epOperationIntegerLessThanOrEqual:
						case epOperationIntegerEqual:
						case epOperationIntegerNotEqual:
						case epOperationIntegerAbs:
						case epOperationIntegerToBoolean:
						case epOperationIntegerToFloat:
						case epOperationIntegerToDouble:
						case epOperationIntegerToFixed:
						case epOperationFloatAdd:
						case epOperationFloatSubtract:
						case epOperationFloatNegation:
						case epOperationFloatMultiply:
						case epOperationFloatDivide:
						case epOperationFloatGreaterThan:
						case epOperationFloatLessThan:
						case epOperationFloatGreaterThanOrEqual:
						case epOperationFloatLessThanOrEqual:
						case epOperationFloatEqual:
						case epOperationFloatNotEqual:
						case epOperationFloatAbs:
						case epOperationFloatToBoolean:
						case epOperationFloatToInteger:
						case epOperationFloatToDouble:
						case epOperationFloatToFixed:
						case epOperationDoubleAdd:
						case epOperationDoubleSubtract:
						case epOperationDoubleNegation:
						case epOperationDoubleMultiply:
						case epOperationDoubleDivide:
						case epOperationDoubleGreaterThan:
						case epOperationDoubleLessThan:
						case epOperationDoubleGreaterThanOrEqual:
						case epOperationDoubleLessThanOrEqual:
						case epOperationDoubleEqual:
						case epOperationDoubleNotEqual:
						case epOperationDoubleAbs:
						case epOperationDoubleToBoolean:
						case epOperationDoubleToInteger:
						case epOperationDoubleToFloat:
						case epOperationDoubleToFixed:
						case epOperationDoubleSin:
						case epOperationDoubleCos:
						case epOperationDoubleTan:
						case epOperationDoubleAtan:
						case epOperationDoubleLn:
						case epOperationDoubleExp:
						case epOperationDoubleSqrt:
						case epOperationDoublePower:
						case epOperationFixedAdd:
						case epOperationFixedSubtract:
						case epOperationFixedNegation:
						case epOperationFixedMultiply:
						case epOperationFixedDivide:
						case epOperationFixedShiftLeft:
						case epOperationFixedShiftRight:
						case epOperationFixedGreaterThan:
						case epOperationFixedLessThan:
						case epOperationFixedGreaterThanOrEqual:
						case epOperationFixedLessThanOrEqual:
						case epOperationFixedEqual:
						case epOperationFixedNotEqual:
						case epOperationFixedAbs:
						case epOperationFixedToBoolean:
						case epOperationFixedToInteger:
						case epOperationFixedToFloat:
						case epOperationFixedToDouble:
						case epGetBooleanArraySize: /* <opcode> */
						case epGetIntegerArraySize:
						case epGetFloatArraySize:
						case epGetDoubleArraySize:
						case epGetFixedArraySize:
						case epReturnFromSubroutine: /* <opcode> */
						case epLoadImmediateNILArray: /* <opcode> */
						case epMakeBooleanArray: /* <opcode> */
						case epMakeIntegerArray:
						case epMakeFloatArray:
						case epMakeDoubleArray:
						case epMakeFixedArray:
						case epStackPop: /* <opcode> */
						case epDuplicate: /* <opcode> */
						case epNop: /* <opcode> */
						case epStackAllocate: /* <opcode> */
						case epResizeBooleanArray2: /* <opcode> */
						case epResizeIntegerArray2:
						case epResizeFloatArray2:
						case epResizeDoubleArray2:
						case epResizeFixedArray2:
						case epStoreBooleanIntoArray2: /* <opcode> */
						case epStoreIntegerIntoArray2:
						case epStoreFloatIntoArray2:
						case epStoreDoubleIntoArray2:
						case epStoreFixedIntoArray2:
						case epLoadBooleanFromArray2: /* <opcode> */
						case epLoadIntegerFromArray2:
						case epLoadFloatFromArray2:
						case epLoadDoubleFromArray2:
						case epLoadFixedFromArray2:
						case epOperationBooleanXor:
						case epOperationIntegerAnd:
						case epOperationFixedAnd:
						case epOperationIntegerOr:
						case epOperationFixedOr:
						case epOperationIntegerXor:
						case epOperationFixedXor:
						case epOperationIntegerImpreciseDivide:
						case epOperationFloatShiftLeft:
						case epOperationDoubleShiftLeft:
						case epOperationFloatShiftRight:
						case epOperationDoubleShiftRight:
						case epOperationIntegerNot:
						case epOperationDoubleAsin:
						case epOperationDoubleAcos:
						case epOperationDoubleSqr:
						case epOperationTestIntegerNegative:
						case epOperationTestFloatNegative:
						case epOperationTestDoubleNegative:
						case epOperationTestFixedNegative:
						case epOperationGetSignInteger:
						case epOperationGetSignFloat:
						case epOperationGetSignDouble:
						case epOperationGetSignFixed:
							Scan += 1;
							break;
						case epStackPopMultiple: /* <opcode> <numwords> */
						case epStackDeallocateUnder: /* <opcode> <numwords> */
						case epOperationBooleanToIntegerBuried:  /* <opcode> <stackindex> */
						case epOperationBooleanToFloatBuried:
						case epOperationBooleanToDoubleBuried:
						case epOperationBooleanToFixedBuried:
						case epOperationIntegerToBooleanBuried:
						case epOperationIntegerToFloatBuried:
						case epOperationIntegerToDoubleBuried:
						case epOperationIntegerToFixedBuried:
						case epOperationFloatToBooleanBuried:
						case epOperationFloatToIntegerBuried:
						case epOperationFloatToDoubleBuried:
						case epOperationFloatToFixedBuried:
						case epOperationDoubleToBooleanBuried:
						case epOperationDoubleToIntegerBuried:
						case epOperationDoubleToFloatBuried:
						case epOperationDoubleToFixedBuried:
						case epOperationFixedToBooleanBuried:
						case epOperationFixedToIntegerBuried:
						case epOperationFixedToFloatBuried:
						case epOperationFixedToDoubleBuried:
						case epBranchUnconditional: /* <opcode> <branchoffset> */
						case epBranchIfZero:
						case epBranchIfNotZero:
						case epStoreIntegerOnStack: /* <opcode> <stackindex> */
						case epStoreFloatOnStack:
						case epStoreDoubleOnStack:
						case epStoreArrayOnStack:
						case epLoadIntegerFromStack:
						case epLoadFloatFromStack:
						case epLoadDoubleFromStack:
						case epLoadArrayFromStack:
						case epLoadImmediateInteger: /* <opcode> <integer>; also used for boolean & fixed */
							Scan += 2;
							break;
						case epLoadImmediateFloat: /* <opcode> ^<float> */
							if (Scan + 1 < Limit)
								{
									/* conditional needed since it's possible they could have had an */
									/* error before they got a chance to put the operands */
									ReleasePtr((char*)(Pcode->OpcodeArray[Scan + 1].ImmediateFloat));
								}
							Scan += 2;
							break;
						case epLoadImmediateDouble: /* <opcode> ^<double> */
							if (Scan + 1 < Limit)
								{
									ReleasePtr((char*)(Pcode->OpcodeArray[Scan + 1].ImmediateDouble));
								}
							Scan += 2;
							break;
						case epGetSampleLeftArray: /* <opcode> ^"<namestring>" */
						case epGetSampleRightArray:
						case epGetSampleMonoArray:
						case epGetWaveTableArray:
						case epGetWaveTableFrames:
						case epGetWaveTableTables:
							if (Scan + 1 < Limit)
								{
									ReleasePtr((char*)(Pcode->OpcodeArray[Scan + 1].ImmediateString));
								}
							Scan += 2;
							break;
						default:
							EXECUTE(PRERR(ForceAbort,"DisposePcode:  unknown opcode"));
							break;
					}
			}
		ReleasePtr((char*)(Pcode->OpcodeArray));
		ReleasePtr((char*)Pcode);
	}


long							PcodeGetNextAddress(PcodeRec* Pcode)
	{
		CheckPtrExistence(Pcode);
		return Pcode->NumInstructions;
	}


OpcodeRec*				GetOpcodeFromPcode(PcodeRec* Pcode)
	{
		CheckPtrExistence(Pcode);
		return Pcode->OpcodeArray;
	}


/* get the number of cells in the array */
long							GetNumberOfValidCellsInPcode(PcodeRec* Pcode)
	{
		CheckPtrExistence(Pcode);
		return Pcode->NumInstructions;
	}


/* put a new opcode array into the pcode.  this does not allocate or release any */
/* memory, but merely updates the pointer.  it should not be called by anyone */
/* except the optimizer.   after the optimizer is done, it resizes the array to */
/* be exactly the size we need. */
void							UpdateOpcodeInPcode(PcodeRec* Pcode, OpcodeRec* NewOpcodeArray,
										long NewNumInstructions)
	{
		CheckPtrExistence(Pcode);
		CheckPtrExistence(NewOpcodeArray);
		ERROR((NewNumInstructions < 0) || (NewNumInstructions
			> PtrSize((char*)NewOpcodeArray) / sizeof(OpcodeRec)),PRERR(ForceAbort,
			"UpdateOpcodeInPcode:  bad array size problem"));
		Pcode->OpcodeArray = NewOpcodeArray;
		Pcode->NumInstructions = NewNumInstructions;
	}


/* Add a pcode instruction.  *Index returns the index of the instruction so that */
/* branches can be patched up.  if Index is NIL, then it won't bother returning */
/* anything. */
MyBoolean					AddPcodeInstruction(PcodeRec* Pcode, Pcodes Opcode, long* Index)
	{
		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].Opcode = Opcode;
		if (Index != NIL)
			{
				*Index = Pcode->NumInstructions;
			}
		Pcode->NumInstructions += 1;
		return True;
	}


MyBoolean					AddPcodeOperandDouble(PcodeRec* Pcode, double ImmediateData)
	{
		double*					NewDouble;

		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		NewDouble = (double*)AllocPtrCanFail(sizeof(double),"PcodeDouble");
		if (NewDouble == NIL)
			{
				return False;
			}
		*NewDouble = ImmediateData;
		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].ImmediateDouble = NewDouble;
		Pcode->NumInstructions += 1;
		return True;
	}


MyBoolean					AddPcodeOperandFloat(PcodeRec* Pcode, float ImmediateData)
	{
		float*					NewFloat;

		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		NewFloat = (float*)AllocPtrCanFail(sizeof(float),"PcodeFloat");
		if (NewFloat == NIL)
			{
				return False;
			}
		*NewFloat = ImmediateData;
		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].ImmediateFloat = NewFloat;
		Pcode->NumInstructions += 1;
		return True;
	}


MyBoolean					AddPcodeOperandInteger(PcodeRec* Pcode, long ImmediateData)
	{
		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].ImmediateInteger = ImmediateData;
		Pcode->NumInstructions += 1;
		return True;
	}


/* the string must be null terminated */
MyBoolean					AddPcodeOperandString(PcodeRec* Pcode, char* String, long Length)
	{
		char*						NewString;

		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		NewString = AllocPtrCanFail(Length + 1,"PcodeString");
		if (NewString == NIL)
			{
				return False;
			}
		CopyData(String,NewString,Length);
		NewString[Length] = 0; /* null termination */

		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].ImmediateString = NewString;
		Pcode->NumInstructions += 1;
		return True;
	}


MyBoolean					AddPcodeOperandDataTypeArray(PcodeRec* Pcode, DataTypes* DataTypeArray)
	{
		DataTypes*			NewTypeArray;

		CheckPtrExistence(Pcode);
		if (!ExtendPcodeLength(Pcode))
			{
				return False;
			}
		NewTypeArray = (DataTypes*)CopyPtr((char*)DataTypeArray);
		if (NewTypeArray == NIL)
			{
				return False;
			}
		SetTag(NewTypeArray,"PcodeDataTypeArray");

		PRNGCHK(Pcode->OpcodeArray,&(Pcode->OpcodeArray[Pcode->NumInstructions]),
			sizeof(Pcode->OpcodeArray[Pcode->NumInstructions]));
		Pcode->OpcodeArray[Pcode->NumInstructions].DataTypeArray = NewTypeArray;
		Pcode->NumInstructions += 1;
		return True;
	}


void							ResolvePcodeBranch(PcodeRec* Pcode, long Where, long Destination)
	{
		CheckPtrExistence(Pcode);
		ERROR((Where < 0) || (Where + 1 >= Pcode->NumInstructions),PRERR(ForceAbort,
			"ResolvePcodeBranch:  address out of range"));
		ERROR((Pcode->OpcodeArray[Where].Opcode != epBranchUnconditional)
			&& (Pcode->OpcodeArray[Where].Opcode != epBranchIfZero)
			&& (Pcode->OpcodeArray[Where].Opcode != epBranchIfNotZero),PRERR(ForceAbort,
			"ResolvePcodeBranch:  patching a non-branch instruction"));
		Pcode->OpcodeArray[Where + 1].ImmediateInteger = Destination;
	}


static MyBoolean	LocalStrEqu(char* Left, char* Right, long LeftLen, long RightLen)
	{
		if (LeftLen == RightLen)
			{
				return MemEqu(Left,Right,LeftLen);
			}
		return False;
	}


void							PcodeUnlink(PcodeRec* Function, char* DeadFuncName,
										PcodeRec* DeadFuncCode)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(Function);
		CheckPtrExistence(DeadFuncCode);
		Limit = Function->NumInstructions;
		Scan = 0;
		while (Scan < Limit)
			{
				/* first, see if we have to delink it */
				if (Function->OpcodeArray[Scan].Opcode == epFuncCallResolved)
					{
						/* this is the one we have to delink. */
						/*    0              1              2            3            4    */
						/* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
						/* will be converted to */
						/* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
						ERROR(LocalStrEqu(Function->OpcodeArray[Scan + 1].ImmediateString,
							DeadFuncName,StrLen(Function->OpcodeArray[Scan + 1].ImmediateString),
							PtrSize(DeadFuncName)) && (DeadFuncCode->OpcodeArray
							!= Function->OpcodeArray[Scan + 4].FunctionOpcodeRecPtr),
							PRERR(ForceAbort,"PcodeUnlink:  found a function with the right name "
							"but it's not linked to the same code record"));
						ERROR(!LocalStrEqu(Function->OpcodeArray[Scan + 1].ImmediateString,
							DeadFuncName,StrLen(Function->OpcodeArray[Scan + 1].ImmediateString),
							PtrSize(DeadFuncName)) && (DeadFuncCode->OpcodeArray
							== Function->OpcodeArray[Scan + 4].FunctionOpcodeRecPtr),
							PRERR(ForceAbort,"PcodeUnlink:  found a function that's linked to this "
							"code record, but with a different function name"));
						if (LocalStrEqu(Function->OpcodeArray[Scan + 1].ImmediateString,
							DeadFuncName,StrLen(Function->OpcodeArray[Scan + 1].ImmediateString),
							PtrSize(DeadFuncName)))
							{
								/* if the name is the same, then delink it */
								Function->OpcodeArray[Scan].Opcode = epFuncCallUnresolved;
								Function->OpcodeArray[Scan + 4].FunctionOpcodeRecPtr = NIL;
							}
					}
				/* now, advance the program counter the right amount */
				Scan += GetInstructionLength(Function->OpcodeArray[Scan].Opcode);
			}
	}


long							GetInstructionLength(Pcodes OpcodeWord)
	{
		switch (OpcodeWord)
			{
				case epFuncCallUnresolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
				case epFuncCallResolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
					return 5;
				case epOperationBooleanEqual: /* <opcode> */
				case epOperationBooleanNotEqual:
				case epOperationBooleanAnd:
				case epOperationBooleanOr:
				case epOperationBooleanNot:
				case epOperationBooleanToInteger:
				case epOperationBooleanToFloat:
				case epOperationBooleanToDouble:
				case epOperationBooleanToFixed:
				case epOperationIntegerAdd:
				case epOperationIntegerSubtract:
				case epOperationIntegerNegation:
				case epOperationIntegerMultiply:
				case epOperationIntegerDivide:
				case epOperationIntegerModulo:
				case epOperationIntegerShiftLeft:
				case epOperationIntegerShiftRight:
				case epOperationIntegerGreaterThan:
				case epOperationIntegerLessThan:
				case epOperationIntegerGreaterThanOrEqual:
				case epOperationIntegerLessThanOrEqual:
				case epOperationIntegerEqual:
				case epOperationIntegerNotEqual:
				case epOperationIntegerAbs:
				case epOperationIntegerToBoolean:
				case epOperationIntegerToFloat:
				case epOperationIntegerToDouble:
				case epOperationIntegerToFixed:
				case epOperationFloatAdd:
				case epOperationFloatSubtract:
				case epOperationFloatNegation:
				case epOperationFloatMultiply:
				case epOperationFloatDivide:
				case epOperationFloatGreaterThan:
				case epOperationFloatLessThan:
				case epOperationFloatGreaterThanOrEqual:
				case epOperationFloatLessThanOrEqual:
				case epOperationFloatEqual:
				case epOperationFloatNotEqual:
				case epOperationFloatAbs:
				case epOperationFloatToBoolean:
				case epOperationFloatToInteger:
				case epOperationFloatToDouble:
				case epOperationFloatToFixed:
				case epOperationDoubleAdd:
				case epOperationDoubleSubtract:
				case epOperationDoubleNegation:
				case epOperationDoubleMultiply:
				case epOperationDoubleDivide:
				case epOperationDoubleGreaterThan:
				case epOperationDoubleLessThan:
				case epOperationDoubleGreaterThanOrEqual:
				case epOperationDoubleLessThanOrEqual:
				case epOperationDoubleEqual:
				case epOperationDoubleNotEqual:
				case epOperationDoubleAbs:
				case epOperationDoubleToBoolean:
				case epOperationDoubleToInteger:
				case epOperationDoubleToFloat:
				case epOperationDoubleToFixed:
				case epOperationDoubleSin:
				case epOperationDoubleCos:
				case epOperationDoubleTan:
				case epOperationDoubleAtan:
				case epOperationDoubleLn:
				case epOperationDoubleExp:
				case epOperationDoubleSqrt:
				case epOperationDoublePower:
				case epOperationFixedAdd:
				case epOperationFixedSubtract:
				case epOperationFixedNegation:
				case epOperationFixedMultiply:
				case epOperationFixedDivide:
				case epOperationFixedShiftLeft:
				case epOperationFixedShiftRight:
				case epOperationFixedGreaterThan:
				case epOperationFixedLessThan:
				case epOperationFixedGreaterThanOrEqual:
				case epOperationFixedLessThanOrEqual:
				case epOperationFixedEqual:
				case epOperationFixedNotEqual:
				case epOperationFixedAbs:
				case epOperationFixedToBoolean:
				case epOperationFixedToInteger:
				case epOperationFixedToFloat:
				case epOperationFixedToDouble:
				case epGetBooleanArraySize: /* <opcode> */
				case epGetIntegerArraySize:
				case epGetFloatArraySize:
				case epGetDoubleArraySize:
				case epGetFixedArraySize:
				case epReturnFromSubroutine: /* <opcode> */
				case epLoadImmediateNILArray: /* <opcode> */
				case epMakeBooleanArray: /* <opcode> */
				case epMakeIntegerArray:
				case epMakeFloatArray:
				case epMakeDoubleArray:
				case epMakeFixedArray:
				case epStackPop: /* <opcode> */
				case epDuplicate: /* <opcode> */
				case epNop: /* <opcode> */
				case epStackAllocate: /* <opcode> */
				case epResizeBooleanArray2: /* <opcode> */
				case epResizeIntegerArray2:
				case epResizeFloatArray2:
				case epResizeDoubleArray2:
				case epResizeFixedArray2:
				case epStoreBooleanIntoArray2: /* <opcode> */
				case epStoreIntegerIntoArray2:
				case epStoreFloatIntoArray2:
				case epStoreDoubleIntoArray2:
				case epStoreFixedIntoArray2:
				case epLoadBooleanFromArray2: /* <opcode> */
				case epLoadIntegerFromArray2:
				case epLoadFloatFromArray2:
				case epLoadDoubleFromArray2:
				case epLoadFixedFromArray2:
				case epOperationBooleanXor:
				case epOperationIntegerAnd:
				case epOperationFixedAnd:
				case epOperationIntegerOr:
				case epOperationFixedOr:
				case epOperationIntegerXor:
				case epOperationFixedXor:
				case epOperationIntegerImpreciseDivide:
				case epOperationFloatShiftLeft:
				case epOperationDoubleShiftLeft:
				case epOperationFloatShiftRight:
				case epOperationDoubleShiftRight:
				case epOperationIntegerNot:
				case epOperationDoubleAsin:
				case epOperationDoubleAcos:
				case epOperationDoubleSqr:
				case epOperationTestIntegerNegative:
				case epOperationTestFloatNegative:
				case epOperationTestDoubleNegative:
				case epOperationTestFixedNegative:
				case epOperationGetSignInteger:
				case epOperationGetSignFloat:
				case epOperationGetSignDouble:
				case epOperationGetSignFixed:
					return 1;
				case epStackPopMultiple: /* <opcode> <numwords> */
				case epStackDeallocateUnder: /* <opcode> <numwords> */
				case epOperationBooleanToIntegerBuried:  /* <opcode> <stackindex> */
				case epOperationBooleanToFloatBuried:
				case epOperationBooleanToDoubleBuried:
				case epOperationBooleanToFixedBuried:
				case epOperationIntegerToBooleanBuried:
				case epOperationIntegerToFloatBuried:
				case epOperationIntegerToDoubleBuried:
				case epOperationIntegerToFixedBuried:
				case epOperationFloatToBooleanBuried:
				case epOperationFloatToIntegerBuried:
				case epOperationFloatToDoubleBuried:
				case epOperationFloatToFixedBuried:
				case epOperationDoubleToBooleanBuried:
				case epOperationDoubleToIntegerBuried:
				case epOperationDoubleToFloatBuried:
				case epOperationDoubleToFixedBuried:
				case epOperationFixedToBooleanBuried:
				case epOperationFixedToIntegerBuried:
				case epOperationFixedToFloatBuried:
				case epOperationFixedToDoubleBuried:
				case epBranchUnconditional: /* <opcode> <branchoffset> */
				case epBranchIfZero:
				case epBranchIfNotZero:
				case epStoreIntegerOnStack: /* <opcode> <stackindex> */
				case epStoreFloatOnStack:
				case epStoreDoubleOnStack:
				case epStoreArrayOnStack:
				case epLoadIntegerFromStack:
				case epLoadFloatFromStack:
				case epLoadDoubleFromStack:
				case epLoadArrayFromStack:
				case epLoadImmediateInteger: /* <opcode> <integer>; also used for boolean & fixed */
				case epLoadImmediateFloat: /* <opcode> ^<float> */
				case epLoadImmediateDouble: /* <opcode> ^<double> */
				case epGetSampleLeftArray: /* <opcode> ^"<namestring>" */
				case epGetSampleRightArray:
				case epGetSampleMonoArray:
				case epGetWaveTableArray:
				case epGetWaveTableFrames:
				case epGetWaveTableTables:
				case epErrorTrap: /* <opcode> ^"<errorstring>" */
					return 2;
				default:
					EXECUTE(PRERR(ForceAbort,"GetInstructionLength:  unknown opcode"));
			}
		EXECUTE(PRERR(ForceAbort,"GetInstructionLength:  control reached end of function"));
	}
