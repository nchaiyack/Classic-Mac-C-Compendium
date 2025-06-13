/* PeepholeOptimizer.c */
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

#define SHOW_ME_OPCODEREC  /* we need to see the contents of opcode records */
#include "PeepholeOptimizer.h"
#include "PcodeObject.h"
#include "Memory.h"


/* this routine scans the entire code block to see if any branches are made */
/* to any but the first instruction specified in the run of instructions. */
/* Start points to the first instruction, Extent specifies the number of WORDS */
/* (not instructions).  Branches MAY point to Prog[Start], however. */
static MyBoolean		NoBranchToInterior(OpcodeRec* Prog, long Length,
											long Start, long Extent)
	{
		long							Scan;

		for (Scan = 0; Scan < Length; Scan += GetInstructionLength(Prog[Scan].Opcode))
			{
				PRNGCHK(Prog,&(Prog[Scan]),sizeof(Prog[Scan]));
				if ((Prog[Scan].Opcode == epBranchUnconditional)
					|| (Prog[Scan].Opcode == epBranchIfZero)
					|| (Prog[Scan].Opcode == epBranchIfNotZero))
					{
						/* branch operation detected, test index */
						PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
						if ((Prog[Scan + 1].ImmediateInteger > Start)
							&& (Prog[Scan + 1].ImmediateInteger < Start + Extent))
							{
								/* branch to interior found, so return false. */
								return False;
							}
					}
			}
		ERROR(Scan != Length,PRERR(ForceAbort,
			"NoBranchToInterior:  internal instruction alignment error"));
		return True;
	}


/* this routine eliminates the specified segment of code from the program */
/* and updates all branches pointing to areas beyond it.  The new length */
/* is returned.  it also disposes of any additional storage used by the */
/* instructions being deleted */
static long					DropCodeSegment(OpcodeRec* Prog, long Length,
											long Start, long Extent)
	{
		long							Scan;

		/* first, patch up branches */
		for (Scan = 0; Scan < Length; Scan += GetInstructionLength(Prog[Scan].Opcode))
			{
				/* looking for branch instructions */
				PRNGCHK(Prog,&(Prog[Scan]),sizeof(Prog[Scan]));
				if ((Prog[Scan].Opcode == epBranchUnconditional)
					|| (Prog[Scan].Opcode == epBranchIfZero)
					|| (Prog[Scan].Opcode == epBranchIfNotZero))
					{
						/* found a branch instruction.  does it need to be patched? */
						PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
						if (Prog[Scan + 1].ImmediateInteger > Start)
							{
								/* branch is beyond segment being dropped, so decrement it's address */
								/* by the length of the segment */
								Prog[Scan + 1].ImmediateInteger -= Extent;
							}
					}
			}
		ERROR(Scan != Length,PRERR(ForceAbort,
			"DropCodeSegment:  internal instruction alignment error:  branch resolve phase"));
		/* next, dispose of additional memory owned by the instructions that */
		/* are being deleted */
		for (Scan = Start; Scan < Start + Extent;
			Scan += GetInstructionLength(Prog[Scan].Opcode))
			{
				switch (Prog[Scan].Opcode)
					{
						case epFuncCallUnresolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */
						case epFuncCallResolved: /* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */
							PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
							ReleasePtr(Prog[Scan + 1].ImmediateString);
							PRNGCHK(Prog,&(Prog[Scan + 2]),sizeof(Prog[Scan + 2]));
							ReleasePtr((char*)(Prog[Scan + 2].DataTypeArray));
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
							break;
						case epLoadImmediateFloat: /* <opcode> ^<float> */
							PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
							ReleasePtr((char*)(Prog[Scan + 1].ImmediateFloat));
							break;
						case epLoadImmediateDouble: /* <opcode> ^<double> */
							PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
							ReleasePtr((char*)(Prog[Scan + 1].ImmediateDouble));
							break;
						case epGetSampleLeftArray: /* <opcode> ^"<namestring>" */
						case epGetSampleRightArray:
						case epGetSampleMonoArray:
							PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
							ReleasePtr((char*)(Prog[Scan + 1].ImmediateString));
							break;
						case epErrorTrap: /* <opcode> ^"<errorstring>" */
							PRNGCHK(Prog,&(Prog[Scan + 1]),sizeof(Prog[Scan + 1]));
							ReleasePtr(Prog[Scan + 1].ImmediateString);
							break;
						default:
							EXECUTE(PRERR(ForceAbort,"DropCodeSegment:  unknown opcode"));
					}
			}
		ERROR(Scan != Start + Extent,PRERR(ForceAbort,
			"DropCodeSegment:  internal instruction alignment error:  dispose phase"));
		/* now, delete the code segment */
		for (Scan = Start; Scan < Length - Extent; Scan += 1)
			{
				PRNGCHK(Prog,&(Prog[Scan]),sizeof(Prog[Scan]));
				PRNGCHK(Prog,&(Prog[Scan + Extent]),sizeof(Prog[Scan + Extent]));
				Prog[Scan] = Prog[Scan + Extent];
			}
		/* now, erase that little bit at the end */
		for (Scan = Length - Extent; Scan < Length; Scan += 1)
			{
				PRNGCHK(Prog,&(Prog[Scan]),sizeof(Prog[Scan]));
				Prog[Scan].Opcode = epNop;
			}
		/* now, return the new code length */
		return Length - Extent;
	}


/* this routine looks for indivisible dup/pop operations (with no interior */
/* branches) and eliminates them.  *Flag is set if some change was made, and the */
/* new length of Prog[] is returned. */
static long					EliminateDupPop(OpcodeRec* Prog, long Length, MyBoolean* Flag)
	{
		long							Scan;

		Scan = 0;
		while (Scan < Length)
			{
				/* look to see if this part can be dropped.  if it can, we don't increment */
				/* Scan so that we can look at the part that will be moved into where this */
				/* is as well.  otherwise, we do increment */
				if ((Prog[Scan].Opcode == epDuplicate)
					&& (Prog[Scan + 1].Opcode == epStackPop)
					&& NoBranchToInterior(Prog,Length,Scan,2))
					{
						/* found one! */
						Length = DropCodeSegment(Prog,Length,Scan,2);
						*Flag = True;
					}
				 else
					{
						/* increment only if not found */
						Scan += GetInstructionLength(Prog[Scan].Opcode);
					}
			}
		ERROR(Scan != Length,PRERR(ForceAbort,
			"EliminateDupPop:  internal instruction alignment error"));
		return Length;
	}


/* perform the optimizations */
void								OptimizePcode(PcodeRec* ThePcode)
	{
		OpcodeRec*				Prog;
		long							Length;
		MyBoolean					OptimizationFound;

		/* obtain the important information */
		Prog = GetOpcodeFromPcode(ThePcode);
		Length = GetNumberOfValidCellsInPcode(ThePcode);

		/* begin the optimization loop */
		do
			{
				/* we go until no more optimizations can be found, so we clear this */
				/* flag.  at the end, if the flag is set, we'll check again. */
				OptimizationFound = False;

				Length = EliminateDupPop(Prog,Length,&OptimizationFound);
			} while (OptimizationFound);

		/* resize the code block to eliminate any empty cells at the end */
		if (PtrSize((char*)Prog) != Length * sizeof(OpcodeRec))
			{
				OpcodeRec*				Temp;

				Temp = (OpcodeRec*)ResizePtr((char*)Prog,Length * sizeof(OpcodeRec));
				if (Temp != NIL)
					{
						UpdateOpcodeInPcode(ThePcode,Temp,Length);
					}
				/* if we couldn't resize it, then it's just too bad.  the evaluator */
				/* doesn't really depend on the length of the opcode for anything, so */
				/* it doesn't matter.  (DisposePcode does, but the end is erased with */
				/* nops, so it shouldn't delete anything important.) */
			}
	}
