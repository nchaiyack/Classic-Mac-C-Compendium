/* PcodeObject.h */

#ifndef Included_PcodeObject_h
#define Included_PcodeObject_h

/* PcodeObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* DataMunging */

/* Stack indices are non-positive:  0 is the word on the */
/* top of the stack and descending values (-1, -2, -3, ...) are words further */
/* from the top of stack. */
typedef enum
	{
		/* Unlinked function call.  The function's name is known from compile time, but */
		/* the address has not been linked.  The first parameter is the name of the */
		/* function (a string).  The second parameter is the list of types of the */
		/* parameters for the function in question.  The third parameter */
		/* is the return type of the function.  The return address is pushed */
		/* onto the stack */
		epFuncCallUnresolved EXECUTE(= 16000),
			/* <opcode> ^"<functionname>" ^[paramlist] <returntype> <reserved> */

		/* Linked function call.  The opcode epFuncCallUnresolved is converted to this */
		/* when first encountered and successfully linked.  The first parameter is not */
		/* changed.  The second parameter is released and converted to a pointer to */
		/* the appropriate function's PcodeRec.  The third parameter */
		/* is the return type of the function.  The return address is pushed */
		/* onto the stack */
		epFuncCallResolved,
			/* <opcode> ^"<functionname>" ^[paramlist] <returntype> ^<OpcodeRec> */

		/* Debugging function.  If executed, it traps and presents the error message */
		/* to the user.  It expects there to be a boolean on the stack which indicates */
		/* whether the user can resume or not.  The boolean is popped if user resumes. */
		epErrorTrap,  /* <opcode> ^"<errorstring>" */

		/* Branch instructions.  The parameter is the absolute address within the */
		/* current block of code.  Conditional branch instructions test and pop */
		/* the top of stack */
		epBranchUnconditional,  /* <opcode> <branchoffset> */
		epBranchIfZero,
		epBranchIfNotZero,

		/* calling conventions:  first, push a word on to reserve a place for the */
		/* return value.  next, push the parameters on.  then call the function (which */
		/* pushes the return address on).  the function returns and pops the return */
		/* address off.  then the function pops the parameters off from under return addr. */
		/* Return from subroutine.  This instruction marks the end of code.  The */
		/* address to jump to is contained in the top of the stack.  This address is */
		/* popped off the stack and execution resumes at the new address. */
		epReturnFromSubroutine,  /* <opcode> */

		/* Intrinsic operators.  for binary operators, the right hand operand is on */
		/* top of the stack, and one word is popped.  for unary operators, no words */
		/* are popped */
		epOperationBooleanEqual,
		epOperationBooleanNotEqual,
		epOperationBooleanAnd,
		epOperationBooleanOr,
		epOperationBooleanXor,
		epOperationBooleanNot,
		epOperationBooleanToInteger,
		epOperationBooleanToFloat,
		epOperationBooleanToDouble,
		epOperationBooleanToFixed,
		epOperationIntegerAdd,
		epOperationIntegerSubtract,
		epOperationIntegerNegation,
		epOperationIntegerMultiply,
		epOperationIntegerDivide,
		epOperationIntegerImpreciseDivide,
		epOperationIntegerModulo,
		epOperationIntegerShiftLeft,
		epOperationIntegerShiftRight,
		epOperationIntegerGreaterThan,
		epOperationIntegerLessThan,
		epOperationIntegerGreaterThanOrEqual,
		epOperationIntegerLessThanOrEqual,
		epOperationIntegerEqual,
		epOperationIntegerNotEqual,
		epOperationIntegerAbs,
		epOperationIntegerToBoolean,
		epOperationIntegerToFloat,
		epOperationIntegerToDouble,
		epOperationIntegerToFixed,
		epOperationIntegerAnd,
		epOperationIntegerOr,
		epOperationIntegerXor,
		epOperationIntegerNot,
		epOperationTestIntegerNegative,
		epOperationGetSignInteger,
		epOperationFloatAdd,
		epOperationFloatSubtract,
		epOperationFloatNegation,
		epOperationFloatMultiply,
		epOperationFloatDivide,
		epOperationFloatShiftLeft,
		epOperationFloatShiftRight,
		epOperationFloatGreaterThan,
		epOperationFloatLessThan,
		epOperationFloatGreaterThanOrEqual,
		epOperationFloatLessThanOrEqual,
		epOperationFloatEqual,
		epOperationFloatNotEqual,
		epOperationFloatAbs,
		epOperationFloatToBoolean,
		epOperationFloatToInteger,
		epOperationFloatToDouble,
		epOperationFloatToFixed,
		epOperationTestFloatNegative,
		epOperationGetSignFloat,
		epOperationDoubleAdd,
		epOperationDoubleSubtract,
		epOperationDoubleNegation,
		epOperationDoubleMultiply,
		epOperationDoubleDivide,
		epOperationDoubleShiftLeft,
		epOperationDoubleShiftRight,
		epOperationDoubleGreaterThan,
		epOperationDoubleLessThan,
		epOperationDoubleGreaterThanOrEqual,
		epOperationDoubleLessThanOrEqual,
		epOperationDoubleEqual,
		epOperationDoubleNotEqual,
		epOperationDoubleAbs,
		epOperationDoubleToBoolean,
		epOperationDoubleToInteger,
		epOperationDoubleToFloat,
		epOperationDoubleToFixed,
		epOperationDoubleSin,
		epOperationDoubleCos,
		epOperationDoubleTan,
		epOperationDoubleAsin,
		epOperationDoubleAcos,
		epOperationDoubleAtan,
		epOperationDoubleLn,
		epOperationDoubleExp,
		epOperationDoubleSqrt,
		epOperationDoubleSqr,
		epOperationDoublePower,
		epOperationTestDoubleNegative,
		epOperationGetSignDouble,
		epOperationFixedAdd,
		epOperationFixedSubtract,
		epOperationFixedNegation,
		epOperationFixedMultiply,
		epOperationFixedDivide,
		epOperationFixedShiftLeft,
		epOperationFixedShiftRight,
		epOperationFixedGreaterThan,
		epOperationFixedLessThan,
		epOperationFixedGreaterThanOrEqual,
		epOperationFixedLessThanOrEqual,
		epOperationFixedEqual,
		epOperationFixedNotEqual,
		epOperationFixedAbs,
		epOperationFixedToBoolean,
		epOperationFixedToInteger,
		epOperationFixedToFloat,
		epOperationFixedToDouble,
		epOperationFixedAnd,
		epOperationFixedOr,
		epOperationFixedXor,
		epOperationTestFixedNegative,
		epOperationGetSignFixed,
		/* additional operators for converting the data type that isn't on top of stack */
		epOperationBooleanToIntegerBuried,  /* <opcode> <stackindex> */
		epOperationBooleanToFloatBuried,
		epOperationBooleanToDoubleBuried,
		epOperationBooleanToFixedBuried,
		epOperationIntegerToBooleanBuried,
		epOperationIntegerToFloatBuried,
		epOperationIntegerToDoubleBuried,
		epOperationIntegerToFixedBuried,
		epOperationFloatToBooleanBuried,
		epOperationFloatToIntegerBuried,
		epOperationFloatToDoubleBuried,
		epOperationFloatToFixedBuried,
		epOperationDoubleToBooleanBuried,
		epOperationDoubleToIntegerBuried,
		epOperationDoubleToFloatBuried,
		epOperationDoubleToFixedBuried,
		epOperationFixedToBooleanBuried,
		epOperationFixedToIntegerBuried,
		epOperationFixedToFloatBuried,
		epOperationFixedToDoubleBuried,

		/* these obtain the size of the array on top of stack, replacing the array */
		/* with the size value. */
		epGetBooleanArraySize,  /* <opcode> */
		epGetIntegerArraySize,
		epGetFloatArraySize,
		epGetDoubleArraySize,
		epGetFixedArraySize,

		/* array is pushed on stack, then size is pushed on stack.  this resizes */
		/* the array, and pops the new size word. */
		epResizeBooleanArray2,  /* <opcode> */
		epResizeIntegerArray2,
		epResizeFloatArray2,
		epResizeDoubleArray2,
		epResizeFixedArray2,

		/* store values on stack.  the value on top is stored BUT NOT POPPED */
		epStoreIntegerOnStack,  /* <opcode> <stackindex> */
		epStoreFloatOnStack,
		epStoreDoubleOnStack,
		epStoreArrayOnStack,
		/* load values.  the value is loaded and pushed onto the stack */
		epLoadIntegerFromStack,
		epLoadFloatFromStack,
		epLoadDoubleFromStack,
		epLoadArrayFromStack,

		/* allocate one empty cell on the stack */
		epStackAllocate, /* <opcode> */

		/* deallocate one cell from the stack */
		epStackPop,  /* <opcode> */

		/* deallocate several cells from the stack */
		epStackPopMultiple,  /* <opcode> <numstackwords> */

		/* deallocate some cells from underneath the top cell */
		epStackDeallocateUnder,  /* <opcode> <numwords> */

		/* duplicate word on top of stack */
		epDuplicate,  /* <opcode> */

		/* no op */
		epNop,  /* <opcode> */

		/* allocate array on stack; size is on stack to start out with */
		epMakeBooleanArray,  /* <opcode> */
		epMakeIntegerArray,
		epMakeFloatArray,
		epMakeDoubleArray,
		epMakeFixedArray,

		/* store value into array.  value is pushed on stack, then array ref, then */
		/* array subscript.  ref and subscript are popped; value remains */
		epStoreBooleanIntoArray2,  /* <opcode> */
		epStoreIntegerIntoArray2,
		epStoreFloatIntoArray2,
		epStoreDoubleIntoArray2,
		epStoreFixedIntoArray2,

		/* load from array.  array is on stack, then subscript is on top of that.  both */
		/* are popped and the value is pushed on the stack. */
		epLoadBooleanFromArray2,  /* <opcode> */
		epLoadIntegerFromArray2,
		epLoadFloatFromArray2,
		epLoadDoubleFromArray2,
		epLoadFixedFromArray2,

		/* load an immediate value on to the stack */
		epLoadImmediateInteger, /* <opcode> <integer>; also used for boolean & fixed */
		epLoadImmediateFloat, /* <opcode> ^<float> */
		epLoadImmediateDouble, /* <opcode> ^<double> */
		epLoadImmediateNILArray, /* <opcode> */

		/* get a fixed array corresponding to the named sample */
		epGetSampleLeftArray, /* <opcode> ^"<namestring>" */
		epGetSampleRightArray, /* <opcode> ^"<namestring>" */
		epGetSampleMonoArray, /* <opcode> ^"<namestring>" */
		epGetWaveTableArray, /* <opcode> ^"<namestring>" */
		/* get attributes for wave tables */
		epGetWaveTableFrames, /* <opcode> ^"<namestring>" */
		epGetWaveTableTables /* <opcode> ^"<namestring>" */
	} Pcodes;

/* these are the data types recognized by the system */
typedef enum
	{
		eBoolean EXECUTE(= -5152),
		eInteger,
		eFloat,
		eDouble,
		eFixed,
		eArrayOfBoolean,
		eArrayOfInteger,
		eArrayOfFloat,
		eArrayOfDouble,
		eArrayOfFixed
	} DataTypes;

union OpcodeRec;
typedef union OpcodeRec OpcodeRec;

#ifdef SHOW_ME_OPCODEREC
/* publicly declared so we can break up the pcode stuff */
union OpcodeRec
	{
		Pcodes							Opcode;
		double*							ImmediateDouble;
		float*							ImmediateFloat;
		long								ImmediateInteger;
		char*								ImmediateString;
		union OpcodeRec*		FunctionOpcodeRecPtr;
		DataTypes*					DataTypeArray;
	};
#endif

struct PcodeRec;
typedef struct PcodeRec PcodeRec;

/* allocate a new pseudocode block */
PcodeRec*					NewPcode(void);

/* release the memory associated with the pseudocode block */
void							DisposePcode(PcodeRec* Pcode);

/* find out what the address of the next instruction will be */
long							PcodeGetNextAddress(PcodeRec* Pcode);

/* get the opcode array for a pcode (so we don't have to declare Pcode publicly.) */
OpcodeRec*				GetOpcodeFromPcode(PcodeRec* Pcode);

/* get the number of cells in the array */
long							GetNumberOfValidCellsInPcode(PcodeRec* Pcode);

/* put a new opcode array into the pcode.  this does not allocate or release any */
/* memory, but merely updates the pointer.  it should not be called by anyone */
/* except the optimizer.   after the optimizer is done, it resizes the array to */
/* be exactly the size we need. */
void							UpdateOpcodeInPcode(PcodeRec* Pcode, OpcodeRec* NewOpcodeArray,
										long NewNumInstructions);

/* add a pcode instruction or an operand value */
MyBoolean					AddPcodeInstruction(PcodeRec* Pcode, Pcodes Opcode, long* Index);
MyBoolean					AddPcodeOperandDouble(PcodeRec* Pcode, double ImmediateData);
MyBoolean					AddPcodeOperandFloat(PcodeRec* Pcode, float ImmediateData);
MyBoolean					AddPcodeOperandInteger(PcodeRec* Pcode, long ImmediateData);
MyBoolean					AddPcodeOperandString(PcodeRec* Pcode, char* String, long Length);
MyBoolean					AddPcodeOperandDataTypeArray(PcodeRec* Pcode, DataTypes* DataTypeArray);

/* resolve a pcode branch whose destination was not known earlier but now is */
void							ResolvePcodeBranch(PcodeRec* Pcode, long Where, long Destination);

/* unlink references to a function which is soon going to disappear. */
void							PcodeUnlink(PcodeRec* Function, char* DeadFuncName,
										PcodeRec* DeadFuncCode);

/* get the number of words for the specified instruction opcode */
long							GetInstructionLength(Pcodes OpcodeWord);

#endif
