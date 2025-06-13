/* PcodeDisassembly.c */
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

#define SHOW_ME_OPCODEREC  /* we need to be able to see contents of OpcodeRec */
#include "PcodeDisassembly.h"
#include "Memory.h"
#include "PcodeObject.h"
#include "Numbers.h"


/* this defines the maximum length of one line of disassembly.  it should */
/* be more than enough. */
#define DISBUFSIZE (1024)


/* utility routine for incrementally constructing each disassmbly line */
static void				CopyStrOver(char Buffer[DISBUFSIZE], char* String, long* Index)
	{
		while (*String != 0)
			{
				ERROR(*Index >= DISBUFSIZE,PRERR(ForceAbort,"CopyStrOver:  buffer overrun"));
				Buffer[*Index] = *String;
				String += 1;
				(*Index) += 1;
			}
	}


/* disassemble pcode and return a string block containing all the data */
char*							DisassemblePcode(PcodeRec* Pcode, char CarriageReturn)
	{
		char*						Thang;
		char						Buffer[DISBUFSIZE];
		long						Scan;
		long						Limit;
		long						BuffIndex;
		OpcodeRec*			OpcodeArray;

		CheckPtrExistence(Pcode);
		Thang = AllocPtrCanFail(0,"DisassemblePcodeThang");
		if (Thang == NIL)
			{
				return NIL;
			}
		OpcodeArray = GetOpcodeFromPcode(Pcode);
		Limit = PtrSize((char*)OpcodeArray) / sizeof(OpcodeRec);
		Scan = 0;
		while (Scan < Limit)
			{
				char*							StringTemp;

				BuffIndex = 0;
				StringTemp = IntegerToString(Scan);
				if (StringTemp == NIL)
					{
					 FailurePoint:
						ReleasePtr(Thang);
						return NIL;
					}
				BuffIndex = PtrSize(StringTemp);
				ERROR(BuffIndex > DISBUFSIZE,PRERR(ForceAbort,"DisassemblePcode:  buffer overrun"));
				CopyData(StringTemp,Buffer,BuffIndex);
				while (BuffIndex < 8)
					{
						Buffer[BuffIndex] = 32;
						BuffIndex += 1;
					}
				ReleasePtr(StringTemp);
				switch (OpcodeArray[Scan].Opcode)
					{
						case epFuncCallUnresolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
							CopyStrOver(Buffer,"call_unlinked ",&BuffIndex);
							CopyStrOver(Buffer,OpcodeArray[Scan + 1].ImmediateString,&BuffIndex);
							Scan += 5;
							break;
						case epFuncCallResolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
							CopyStrOver(Buffer,"call_linked ",&BuffIndex);
							CopyStrOver(Buffer,OpcodeArray[Scan + 1].ImmediateString,&BuffIndex);
							Scan += 5;
							break;

						case epErrorTrap: /* <opcode> ^"<errorstring>" */
							CopyStrOver(Buffer,"error ",&BuffIndex);
						 AppendStringPoint:
							CopyStrOver(Buffer,OpcodeArray[Scan + 1].ImmediateString,&BuffIndex);
						 ScanPlusTwoPoint:
							Scan += 2;
							break;

						case epOperationBooleanEqual: /* <opcode> */
							CopyStrOver(Buffer,"eq.b",&BuffIndex);
						 ScanPlusOnePoint:
							Scan += 1;
							break;
						case epOperationBooleanNotEqual:
							CopyStrOver(Buffer,"neq.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanAnd:
							CopyStrOver(Buffer,"and.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanOr:
							CopyStrOver(Buffer,"or.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanNot:
							CopyStrOver(Buffer,"not.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanToInteger:
							CopyStrOver(Buffer,"booltoint",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanToFloat:
							CopyStrOver(Buffer,"booltofloat",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanToDouble:
							CopyStrOver(Buffer,"booltodouble",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanToFixed:
							CopyStrOver(Buffer,"booltofixed",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerAdd:
							CopyStrOver(Buffer,"add.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerSubtract:
							CopyStrOver(Buffer,"sub.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerNegation:
							CopyStrOver(Buffer,"neg.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerMultiply:
							CopyStrOver(Buffer,"mult.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerDivide:
							CopyStrOver(Buffer,"div.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerModulo:
							CopyStrOver(Buffer,"mod.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerShiftLeft:
							CopyStrOver(Buffer,"asl.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerShiftRight:
							CopyStrOver(Buffer,"asr.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerGreaterThan:
							CopyStrOver(Buffer,"gr.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerLessThan:
							CopyStrOver(Buffer,"ls.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerGreaterThanOrEqual:
							CopyStrOver(Buffer,"greq.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerLessThanOrEqual:
							CopyStrOver(Buffer,"lseq.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerEqual:
							CopyStrOver(Buffer,"eq.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerNotEqual:
							CopyStrOver(Buffer,"neq.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerAbs:
							CopyStrOver(Buffer,"abs.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerToBoolean:
							CopyStrOver(Buffer,"inttobool",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerToFloat:
							CopyStrOver(Buffer,"inttofloat",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerToDouble:
							CopyStrOver(Buffer,"inttodouble",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerToFixed:
							CopyStrOver(Buffer,"inttofixed",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatAdd:
							CopyStrOver(Buffer,"add.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatSubtract:
							CopyStrOver(Buffer,"sub.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatNegation:
							CopyStrOver(Buffer,"neg.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatMultiply:
							CopyStrOver(Buffer,"mult.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatDivide:
							CopyStrOver(Buffer,"div.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatGreaterThan:
							CopyStrOver(Buffer,"gr.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatLessThan:
							CopyStrOver(Buffer,"ls.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatGreaterThanOrEqual:
							CopyStrOver(Buffer,"greq.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatLessThanOrEqual:
							CopyStrOver(Buffer,"lseq.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatEqual:
							CopyStrOver(Buffer,"eq.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatNotEqual:
							CopyStrOver(Buffer,"neq.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatAbs:
							CopyStrOver(Buffer,"abs.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatToBoolean:
							CopyStrOver(Buffer,"floattobool",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatToInteger:
							CopyStrOver(Buffer,"floattoint",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatToDouble:
							CopyStrOver(Buffer,"floattodouble",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatToFixed:
							CopyStrOver(Buffer,"floattofixed",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleAdd:
							CopyStrOver(Buffer,"add.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleSubtract:
							CopyStrOver(Buffer,"sub.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleNegation:
							CopyStrOver(Buffer,"neg.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleMultiply:
							CopyStrOver(Buffer,"mult.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleDivide:
							CopyStrOver(Buffer,"div.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleGreaterThan:
							CopyStrOver(Buffer,"gr.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleLessThan:
							CopyStrOver(Buffer,"ls.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleGreaterThanOrEqual:
							CopyStrOver(Buffer,"greq.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleLessThanOrEqual:
							CopyStrOver(Buffer,"lseq.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleEqual:
							CopyStrOver(Buffer,"eq.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleNotEqual:
							CopyStrOver(Buffer,"neq.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleAbs:
							CopyStrOver(Buffer,"abs.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleToBoolean:
							CopyStrOver(Buffer,"doubletobool",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleToInteger:
							CopyStrOver(Buffer,"doubletoint",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleToFloat:
							CopyStrOver(Buffer,"doubletofloat",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleToFixed:
							CopyStrOver(Buffer,"doubletofixed",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleSin:
							CopyStrOver(Buffer,"sin.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleCos:
							CopyStrOver(Buffer,"cos.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleTan:
							CopyStrOver(Buffer,"tan.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleAtan:
							CopyStrOver(Buffer,"atan.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleLn:
							CopyStrOver(Buffer,"ln.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleExp:
							CopyStrOver(Buffer,"exp.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleSqrt:
							CopyStrOver(Buffer,"sqrt.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoublePower:
							CopyStrOver(Buffer,"pow.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedAdd:
							CopyStrOver(Buffer,"add.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedSubtract:
							CopyStrOver(Buffer,"sub.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedNegation:
							CopyStrOver(Buffer,"neg.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedMultiply:
							CopyStrOver(Buffer,"mult.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedDivide:
							CopyStrOver(Buffer,"div.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedShiftLeft:
							CopyStrOver(Buffer,"asl.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedShiftRight:
							CopyStrOver(Buffer,"asr.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedGreaterThan:
							CopyStrOver(Buffer,"gr.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedLessThan:
							CopyStrOver(Buffer,"ls.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedGreaterThanOrEqual:
							CopyStrOver(Buffer,"greq.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedLessThanOrEqual:
							CopyStrOver(Buffer,"lseq.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedEqual:
							CopyStrOver(Buffer,"eq.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedNotEqual:
							CopyStrOver(Buffer,"neq.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedAbs:
							CopyStrOver(Buffer,"abs.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedToBoolean:
							CopyStrOver(Buffer,"fixedtobool",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedToInteger:
							CopyStrOver(Buffer,"fixedtoint",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedToFloat:
							CopyStrOver(Buffer,"fixedtofloat",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedToDouble:
							CopyStrOver(Buffer,"fixedtodouble",&BuffIndex);
							goto ScanPlusOnePoint;
						case epGetBooleanArraySize: /* <opcode> */
							CopyStrOver(Buffer,"arraysize.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epGetIntegerArraySize:
							CopyStrOver(Buffer,"arraysize.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epGetFloatArraySize:
							CopyStrOver(Buffer,"arraysize.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epGetDoubleArraySize:
							CopyStrOver(Buffer,"arraysize.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epGetFixedArraySize:
							CopyStrOver(Buffer,"arraysize.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epReturnFromSubroutine: /* <opcode> */
							CopyStrOver(Buffer,"return",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadImmediateNILArray: /* <opcode> */
							CopyStrOver(Buffer,"loadnull",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationBooleanXor:
							CopyStrOver(Buffer,"xor.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerAnd:
							CopyStrOver(Buffer,"and.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedAnd:
							CopyStrOver(Buffer,"and.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerOr:
							CopyStrOver(Buffer,"or.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedOr:
							CopyStrOver(Buffer,"or.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerXor:
							CopyStrOver(Buffer,"xor.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFixedXor:
							CopyStrOver(Buffer,"xor.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerImpreciseDivide:
							CopyStrOver(Buffer,"divimpr.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatShiftLeft:
							CopyStrOver(Buffer,"shl.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleShiftLeft:
							CopyStrOver(Buffer,"shl.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationFloatShiftRight:
							CopyStrOver(Buffer,"shr.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleShiftRight:
							CopyStrOver(Buffer,"shr.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationIntegerNot:
							CopyStrOver(Buffer,"not.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleAsin:
							CopyStrOver(Buffer,"asin.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleAcos:
							CopyStrOver(Buffer,"acos.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationDoubleSqr:
							CopyStrOver(Buffer,"sqr.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationTestIntegerNegative:
							CopyStrOver(Buffer,"isneg.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationTestFloatNegative:
							CopyStrOver(Buffer,"isneg.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationTestDoubleNegative:
							CopyStrOver(Buffer,"isneg.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationTestFixedNegative:
							CopyStrOver(Buffer,"isneg.f",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationGetSignInteger:
							CopyStrOver(Buffer,"sign.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationGetSignFloat:
							CopyStrOver(Buffer,"sign.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationGetSignDouble:
							CopyStrOver(Buffer,"sign.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epOperationGetSignFixed:
							CopyStrOver(Buffer,"sign.f",&BuffIndex);
							goto ScanPlusOnePoint;

						case epStackPop: /* <opcode> */
							CopyStrOver(Buffer,"pop",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStackDeallocateUnder: /* <opcode> <numwords> */
							CopyStrOver(Buffer,"popmultipleunder ",&BuffIndex);
							goto AppendInteger;
						case epDuplicate: /* <opcode> */
							CopyStrOver(Buffer,"dup",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStackPopMultiple: /* <opcode> <numwords> */
							CopyStrOver(Buffer,"popmultiple ",&BuffIndex);
							goto AppendInteger;
						case epStackAllocate: /* <opcode> */
							CopyStrOver(Buffer,"alloc",&BuffIndex);
							goto ScanPlusOnePoint;

						case epNop: /* <opcode> */
							CopyStrOver(Buffer,"nop",&BuffIndex);
							goto ScanPlusOnePoint;

						case epOperationBooleanToIntegerBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"booltoint Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationBooleanToFloatBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"booltofloat Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationBooleanToDoubleBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"booltodouble Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationBooleanToFixedBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"booltofixed Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationIntegerToBooleanBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"inttobool Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationIntegerToFloatBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"inttofloat Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationIntegerToDoubleBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"inttodouble Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationIntegerToFixedBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"inttofixed Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFloatToBooleanBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"floattobool Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFloatToIntegerBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"floattoint Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFloatToDoubleBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"floattodouble Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFloatToFixedBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"floattofixed Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationDoubleToBooleanBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"doubletobool Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationDoubleToIntegerBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"doubletoint Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationDoubleToFloatBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"doubletofloat Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationDoubleToFixedBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"doubletofixed Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFixedToBooleanBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"fixedtobool Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFixedToIntegerBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"fixedtoint Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFixedToFloatBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"fixedtofloat Stack[",&BuffIndex);
							goto AppendStack;
						case epOperationFixedToDoubleBuried: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"fixedtodouble Stack[",&BuffIndex);
							goto AppendStack;

						case epBranchUnconditional: /* <opcode> <branchoffset> */
							CopyStrOver(Buffer,"bra ",&BuffIndex);
						 AppendInteger:
							StringTemp = IntegerToString(OpcodeArray[Scan + 1].ImmediateInteger);
							if (StringTemp == NIL)
								{
									goto FailurePoint;
								}
							CopyData(StringTemp,&(Buffer[BuffIndex]),PtrSize(StringTemp));
							BuffIndex += PtrSize(StringTemp);
							ReleasePtr(StringTemp);
							goto ScanPlusTwoPoint;
						case epBranchIfZero:
							CopyStrOver(Buffer,"brz ",&BuffIndex);
							goto AppendInteger;
						case epBranchIfNotZero:
							CopyStrOver(Buffer,"brnz ",&BuffIndex);
							goto AppendInteger;

						case epResizeBooleanArray2: /* <opcode> */
							CopyStrOver(Buffer,"resize.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epResizeIntegerArray2:
							CopyStrOver(Buffer,"resize.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epResizeFloatArray2:
							CopyStrOver(Buffer,"resize.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epResizeDoubleArray2:
							CopyStrOver(Buffer,"resize.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epResizeFixedArray2:
							CopyStrOver(Buffer,"resize.f",&BuffIndex);
							goto ScanPlusOnePoint;

						case epStoreIntegerOnStack: /* <opcode> <stackindex> */
							CopyStrOver(Buffer,"store.i Stack[",&BuffIndex);
						 AppendStack:
							StringTemp = IntegerToString(OpcodeArray[Scan + 1].ImmediateInteger);
							if (StringTemp == NIL)
								{
									goto FailurePoint;
								}
							CopyData(StringTemp,&(Buffer[BuffIndex]),PtrSize(StringTemp));
							BuffIndex += PtrSize(StringTemp);
							ReleasePtr(StringTemp);
							Buffer[BuffIndex++] = ']';
							goto ScanPlusTwoPoint;
						case epStoreFloatOnStack:
							CopyStrOver(Buffer,"store.s Stack[",&BuffIndex);
							goto AppendStack;
						case epStoreDoubleOnStack:
							CopyStrOver(Buffer,"store.d Stack[",&BuffIndex);
							goto AppendStack;
						case epStoreArrayOnStack:
							CopyStrOver(Buffer,"store.a Stack[",&BuffIndex);
							goto AppendStack;
						case epLoadIntegerFromStack:
							CopyStrOver(Buffer,"load.i Stack[",&BuffIndex);
							goto AppendStack;
						case epLoadFloatFromStack:
							CopyStrOver(Buffer,"load.s Stack[",&BuffIndex);
							goto AppendStack;
						case epLoadDoubleFromStack:
							CopyStrOver(Buffer,"load.d Stack[",&BuffIndex);
							goto AppendStack;
						case epLoadArrayFromStack:
							CopyStrOver(Buffer,"load.a Stack[",&BuffIndex);
							goto AppendStack;

						case epMakeBooleanArray: /* <opcode> */
							CopyStrOver(Buffer,"newarray.b",&BuffIndex);
							goto ScanPlusOnePoint;
						case epMakeIntegerArray:
							CopyStrOver(Buffer,"newarray.i",&BuffIndex);
							goto ScanPlusOnePoint;
						case epMakeFloatArray:
							CopyStrOver(Buffer,"newarray.s",&BuffIndex);
							goto ScanPlusOnePoint;
						case epMakeDoubleArray:
							CopyStrOver(Buffer,"newarray.d",&BuffIndex);
							goto ScanPlusOnePoint;
						case epMakeFixedArray:
							CopyStrOver(Buffer,"newarray.f",&BuffIndex);
							goto ScanPlusOnePoint;

						case epStoreBooleanIntoArray2: /* <opcode> */
							CopyStrOver(Buffer,"store.b Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStoreIntegerIntoArray2:
							CopyStrOver(Buffer,"store.i Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStoreFloatIntoArray2:
							CopyStrOver(Buffer,"store.s Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStoreDoubleIntoArray2:
							CopyStrOver(Buffer,"store.d Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epStoreFixedIntoArray2:
							CopyStrOver(Buffer,"store.f Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadBooleanFromArray2: /* <opcode> */
							CopyStrOver(Buffer,"load.b Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadIntegerFromArray2:
							CopyStrOver(Buffer,"load.i Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadFloatFromArray2:
							CopyStrOver(Buffer,"load.s Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadDoubleFromArray2:
							CopyStrOver(Buffer,"load.d Array[]",&BuffIndex);
							goto ScanPlusOnePoint;
						case epLoadFixedFromArray2:
							CopyStrOver(Buffer,"load.f Array[]",&BuffIndex);
							goto ScanPlusOnePoint;

						case epLoadImmediateInteger: /* <opcode> <integer>; also used for boolean & fixed */
							CopyStrOver(Buffer,"load.i #",&BuffIndex);
							goto AppendInteger;
						case epLoadImmediateFloat: /* <opcode> ^<float> */
							CopyStrOver(Buffer,"load.s #",&BuffIndex);
						 AppendFloat:
							StringTemp = LongDoubleToString(*(OpcodeArray[Scan + 1]
								.ImmediateFloat),8,1e-4,1e6);
							if (StringTemp == NIL)
								{
									goto FailurePoint;
								}
							CopyData(StringTemp,&(Buffer[BuffIndex]),PtrSize(StringTemp));
							BuffIndex += PtrSize(StringTemp);
							ReleasePtr(StringTemp);
							goto ScanPlusTwoPoint;

						case epLoadImmediateDouble: /* <opcode> ^<double> */
							CopyStrOver(Buffer,"load.d #",&BuffIndex);
						 AppendDouble:
							StringTemp = LongDoubleToString(*(OpcodeArray[Scan + 1]
								.ImmediateDouble),17,1e-4,1e6);
							if (StringTemp == NIL)
								{
									goto FailurePoint;
								}
							CopyData(StringTemp,&(Buffer[BuffIndex]),PtrSize(StringTemp));
							BuffIndex += PtrSize(StringTemp);
							ReleasePtr(StringTemp);
							goto ScanPlusTwoPoint;

						case epGetSampleLeftArray: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getsampleft.f ",&BuffIndex);
							goto AppendStringPoint;
						case epGetSampleRightArray: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getsampright.f ",&BuffIndex);
							goto AppendStringPoint;
						case epGetSampleMonoArray: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getsampmono.f ",&BuffIndex);
							goto AppendStringPoint;
						case epGetWaveTableArray: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getwaveframecount.i ",&BuffIndex);
							goto AppendStringPoint;
						case epGetWaveTableFrames: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getwavetablecount.i ",&BuffIndex);
							goto AppendStringPoint;
						case epGetWaveTableTables: /* <opcode> ^"<namestring>" */
							CopyStrOver(Buffer,"getwavetablearray.f ",&BuffIndex);
							goto AppendStringPoint;

						default:
							EXECUTE(PRERR(ForceAbort,"DisassemblePcode:  unknown opcode"));
							break;
					}
				Buffer[BuffIndex++] = CarriageReturn;
				ERROR(BuffIndex > DISBUFSIZE,PRERR(ForceAbort,"DisassemblePcode:  buffer overrun"));
				StringTemp = ResizePtr(Thang,PtrSize(Thang) + BuffIndex);
				if (StringTemp == NIL)
					{
						goto FailurePoint;
					}
				Thang = StringTemp;
				PRNGCHK(Thang,&(Thang[PtrSize(Thang) - BuffIndex]),BuffIndex);
				CopyData(Buffer,&(Thang[PtrSize(Thang) - BuffIndex]),BuffIndex);
			}
		return Thang;
	}
