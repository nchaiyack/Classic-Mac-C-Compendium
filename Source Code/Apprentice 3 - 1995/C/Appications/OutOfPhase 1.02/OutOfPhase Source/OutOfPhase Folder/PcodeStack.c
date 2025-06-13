/* PcodeStack.c */
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

#define SHOW_ME_STACKELEMENT  /* we need to be able to see contents of StackElement */
#include "PcodeStack.h"
#include "Memory.h"


#define INITIALNUMSTACKCELLS (32)
#define STACKEXTENDSIZE (16)


/* this structure remembers the parameters until evaluation is called for */
struct ParamStackRec
	{
		StackElement*			InitialStack;
		long							NumElements;
		long							InitialStackSize;
	};


/* create a list of parameters that will lead to a function call */
ParamStackRec*		NewParamStack(void)
	{
		ParamStackRec*	Stack;

		Stack = (ParamStackRec*)AllocPtrCanFail(sizeof(ParamStackRec),
			"ParamStackRec");
		if (Stack == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Stack->InitialStack = (StackElement*)AllocPtrCanFail(
			INITIALNUMSTACKCELLS * sizeof(StackElement),"StackElement");
		if (Stack->InitialStack == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Stack);
				goto FailurePoint1;
			}
		Stack->NumElements = 0;
		Stack->InitialStackSize = INITIALNUMSTACKCELLS;
		return Stack;
	}


void							DisposeParamStack(ParamStackRec* Stack)
	{
		long						Scan;

		CheckPtrExistence(Stack);
		for (Scan = Stack->NumElements - 1; Scan >= 0; Scan -= 1)
			{
				PRNGCHK(Stack->InitialStack,&(Stack->InitialStack[Scan]),
					sizeof(Stack->InitialStack[Scan]));
				if ((Stack->InitialStack[Scan].ElementType == esArray)
					&& (Stack->InitialStack[Scan].Data.ArrayHandle != NIL))
					{
						DisposeIfNotOnStack(Stack->InitialStack,Scan);
					}
			}
		ReleasePtr((char*)(Stack->InitialStack));
		ReleasePtr((char*)Stack);
	}


void							DisposeIfNotOnStack(StackElement* Stack, long StackPtr)
	{
		CheckPtrExistence(Stack);
		ERROR(Stack[StackPtr].ElementType != esArray,PRERR(ForceAbort,
			"DisposeIfNotOnStack:  top of stack isn't an array"));
		ERROR(Stack[StackPtr].Data.ArrayHandle == NIL,PRERR(ForceAbort,
			"DisposeIfNotOnStack:  array is NIL"));
		CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle);
		CheckPtrExistence(Stack[StackPtr].Data.ArrayHandle->Array);
		Stack[StackPtr].Data.ArrayHandle->RefCount -= 1; /* decrement reference count */
		if (Stack[StackPtr].Data.ArrayHandle->RefCount == 0)
			{
				/* reference count = no existing references */
				ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle->Array));
				ReleasePtr((char*)(Stack[StackPtr].Data.ArrayHandle));
				Stack[StackPtr].Data.ArrayHandle = NIL;
			}
	}


void							DisposeIfOnStackOnlyOnce(StackElement* Stack, long StackPtr, long Where)
	{
		CheckPtrExistence(Stack);
		ERROR(Stack[Where].ElementType != esArray,PRERR(ForceAbort,
			"DisposeIfOnStackOnlyOnce:  top of stack isn't an array"));
		ERROR(Stack[Where].Data.ArrayHandle == NIL,PRERR(ForceAbort,
			"DisposeIfOnStackOnlyOnce:  array is NIL"));
		CheckPtrExistence(Stack[Where].Data.ArrayHandle);
		CheckPtrExistence(Stack[Where].Data.ArrayHandle->Array);
		Stack[Where].Data.ArrayHandle->RefCount -= 1; /* decrement reference count */
		if (Stack[Where].Data.ArrayHandle->RefCount == 0)
			{
				/* reference count = no existing references */
				ReleasePtr((char*)(Stack[Where].Data.ArrayHandle->Array));
				ReleasePtr((char*)(Stack[Where].Data.ArrayHandle));
				Stack[Where].Data.ArrayHandle = NIL;
			}
	}


MyBoolean					AddIntegerToStack(ParamStackRec* Stack, long IntegerValue)
	{
		CheckPtrExistence(Stack);
		if (Stack->NumElements == Stack->InitialStackSize)
			{
				StackElement*		NewStack;

				/* enlarge stack */
				NewStack = (StackElement*)ResizePtr((char*)(Stack->InitialStack),
					(Stack->InitialStackSize + STACKEXTENDSIZE) * sizeof(StackElement));
				if (NewStack == NIL)
					{
						return False;
					}
				Stack->InitialStack = NewStack;
				Stack->InitialStackSize += STACKEXTENDSIZE;
			}
		Stack->InitialStack[Stack->NumElements].Data.Integer = IntegerValue;
		Stack->InitialStack[Stack->NumElements].ElementType = esScalar;
		Stack->NumElements += 1;
		return True;
	}


MyBoolean					AddFloatToStack(ParamStackRec* Stack, float FloatValue)
	{
		CheckPtrExistence(Stack);
		if (Stack->NumElements == Stack->InitialStackSize)
			{
				StackElement*		NewStack;

				/* enlarge stack */
				NewStack = (StackElement*)ResizePtr((char*)(Stack->InitialStack),
					(Stack->InitialStackSize + STACKEXTENDSIZE) * sizeof(StackElement));
				if (NewStack == NIL)
					{
						return False;
					}
				Stack->InitialStack = NewStack;
				Stack->InitialStackSize += STACKEXTENDSIZE;
			}
		Stack->InitialStack[Stack->NumElements].Data.Float = FloatValue;
		Stack->InitialStack[Stack->NumElements].ElementType = esScalar;
		Stack->NumElements += 1;
		return True;
	}


MyBoolean					AddDoubleToStack(ParamStackRec* Stack, double DoubleValue)
	{
		CheckPtrExistence(Stack);
		if (Stack->NumElements == Stack->InitialStackSize)
			{
				StackElement*		NewStack;

				/* enlarge stack */
				NewStack = (StackElement*)ResizePtr((char*)(Stack->InitialStack),
					(Stack->InitialStackSize + STACKEXTENDSIZE) * sizeof(StackElement));
				if (NewStack == NIL)
					{
						return False;
					}
				Stack->InitialStack = NewStack;
				Stack->InitialStackSize += STACKEXTENDSIZE;
			}
		Stack->InitialStack[Stack->NumElements].Data.Double = DoubleValue;
		Stack->InitialStack[Stack->NumElements].ElementType = esScalar;
		Stack->NumElements += 1;
		return True;
	}


/* the ACTUAL array is added, so you no longer own it after this! */
MyBoolean					AddArrayToStack(ParamStackRec* Stack, void* Array)
	{
		CheckPtrExistence(Stack);
		if (Stack->NumElements == Stack->InitialStackSize)
			{
				StackElement*		NewStack;

				/* enlarge stack */
				NewStack = (StackElement*)ResizePtr((char*)(Stack->InitialStack),
					(Stack->InitialStackSize + STACKEXTENDSIZE) * sizeof(StackElement));
				if (NewStack == NIL)
					{
						return False;
					}
				Stack->InitialStack = NewStack;
				Stack->InitialStackSize += STACKEXTENDSIZE;
			}
		if (Array != NIL)
			{
				Stack->InitialStack[Stack->NumElements].Data.ArrayHandle =
					(ArrayHandleType*)AllocPtrCanFail(sizeof(ArrayHandleType),"ArrayHandleType");
				if (Stack->InitialStack[Stack->NumElements].Data.ArrayHandle == NIL)
					{
						return False;
					}
				Stack->InitialStack[Stack->NumElements].Data.ArrayHandle->Array = Array;
				Stack->InitialStack[Stack->NumElements].Data.ArrayHandle->RefCount = 1;
			}
		 else
			{
				Stack->InitialStack[Stack->NumElements].Data.ArrayHandle = NIL;
			}
		Stack->InitialStack[Stack->NumElements].ElementType = esArray;
		Stack->NumElements += 1;
		return True;
	}


long							GetStackInteger(ParamStackRec* Stack, long Index)
	{
		CheckPtrExistence(Stack);
		ERROR((Index < 0) || (Index >= Stack->NumElements),PRERR(ForceAbort,
			"GetStackInteger:  index out of range"));
		ERROR(Stack->InitialStack[Index].ElementType != esScalar,PRERR(ForceAbort,
			"GetStackInteger:  not a scalar value"));
		return Stack->InitialStack[Index].Data.Integer;
	}


float							GetStackFloat(ParamStackRec* Stack, long Index)
	{
		CheckPtrExistence(Stack);
		ERROR((Index < 0) || (Index >= Stack->NumElements),PRERR(ForceAbort,
			"GetStackFloat:  index out of range"));
		ERROR(Stack->InitialStack[Index].ElementType != esScalar,PRERR(ForceAbort,
			"GetStackFloat:  not a scalar value"));
		return Stack->InitialStack[Index].Data.Float;
	}


double						GetStackLongDouble(ParamStackRec* Stack, long Index)
	{
		CheckPtrExistence(Stack);
		ERROR((Index < 0) || (Index >= Stack->NumElements),PRERR(ForceAbort,
			"GetStackLongDouble:  index out of range"));
		ERROR(Stack->InitialStack[Index].ElementType != esScalar,PRERR(ForceAbort,
			"GetStackLongDouble:  not a scalar value"));
		return Stack->InitialStack[Index].Data.Double;
	}


/* It returns the actual array, so you only get to use it on a loan basis */
void*							GetStackArray(ParamStackRec* Stack, long Index)
	{
		CheckPtrExistence(Stack);
		ERROR((Index < 0) || (Index >= Stack->NumElements),PRERR(ForceAbort,
			"GetStackArrayCopy:  index out of range"));
		ERROR(Stack->InitialStack[Index].ElementType != esArray,PRERR(ForceAbort,
			"GetStackArrayCopy:  not an array value"));
		if (Stack->InitialStack[Index].Data.ArrayHandle == NIL)
			{
				return NIL;
			}
		 else
			{
				CheckPtrExistence(Stack->InitialStack[Index].Data.ArrayHandle);
				CheckPtrExistence(Stack->InitialStack[Index].Data.ArrayHandle->Array);
				return Stack->InitialStack[Index].Data.ArrayHandle->Array;
			}
	}


StackElement*			GetStackBase(ParamStackRec* Stack)
	{
		CheckPtrExistence(Stack);
		return Stack->InitialStack;
	}


long							GetStackInitialSize(ParamStackRec* Stack)
	{
		CheckPtrExistence(Stack);
		return Stack->InitialStackSize;
	}


long							GetStackNumElements(ParamStackRec* Stack)
	{
		CheckPtrExistence(Stack);
		return Stack->NumElements;
	}


void							SetStackInformation(ParamStackRec* Stack, long NewTotalSize,
										long NewNumElements, StackElement* NewStackAddress)
	{
		CheckPtrExistence(Stack);
		PRNGCHK(NewStackAddress,NewStackAddress,sizeof(StackElement) * NewTotalSize);
		ERROR((NewNumElements < 0) || (NewNumElements > NewTotalSize),PRERR(ForceAbort,
			"SetStackInformation:  parametric weirdness"));
		ERROR(NewNumElements != Stack->NumElements,PRERR(ForceAbort,
			"SetStackInformation:  initial stack pointer is different"));
		Stack->InitialStack = NewStackAddress;
		Stack->InitialStackSize = NewTotalSize;
	}
