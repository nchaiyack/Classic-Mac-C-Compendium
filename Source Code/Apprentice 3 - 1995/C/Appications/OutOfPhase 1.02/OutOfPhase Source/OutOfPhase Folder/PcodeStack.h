/* PcodeStack.h */

#ifndef Included_PcodeStack_h
#define Included_PcodeStack_h

/* PcodeStack module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

/* these are the various things you can have on the stack */
typedef enum
	{
		esScalar EXECUTE(= -18351),
		esArray,
		esReturnAddress
	} StackTypes;

struct StackElement;
typedef struct StackElement StackElement;

/* this record is one entry in the stack */
/* this is public for those who need to quickly access stack elements */
#ifdef SHOW_ME_STACKELEMENT
typedef struct
	{
		/* reference counting suggested by Abe Megahed */
		void*							Array;
		long							RefCount;
	} ArrayHandleType;
struct StackElement
	{
		union StackThangPlace
			{
				long							Integer;
				float							Float;
				double						Double;
				ArrayHandleType*	ArrayHandle;
				struct
					{
						union OpcodeRec*	Procedure;
						long							Index;
					}								ReturnAddress;
			}								Data;
		StackTypes				ElementType;
		/* the padding assumes that it won't add up to 16 bytes.  On the Macintosh, */
		/* double is either 8 or 12 bytes and enums are 2 bytes --> 14 bytes, which */
		/* makes padding 2 bytes.  On most systems where enums are 4 bytes, doubles */
		/* are 8 bytes --> 12 bytes, which makes padding 4 bytes.  We may run into */
		/* trouble on systems with 8-byte pointers, which will make ReturnAddress */
		/* 12 bytes and ElementType (probably) 4 bytes; if this happens, then just */
		/* delete the padding.  The whole reason for padding the structure out to 16 */
		/* bytes is to encourage compilers to do strength reduction on array accesses */
		/* to the stack. */
		char							Padding[16 - (sizeof(union StackThangPlace) + sizeof(StackTypes))];
	};
#endif

struct ParamStackRec;
typedef struct ParamStackRec ParamStackRec;

/* create a list of parameters that will lead to a function call */
ParamStackRec*		NewParamStack(void);

/* this should be called when the parameter list is completely finished to */
/* dispose of all arrays that might still be stored in it */
void							DisposeParamStack(ParamStackRec* Stack);

/* this is a utility routine that disposes of an array on top of stack if it */
/* isn't anywhere else on the stack.  It's used if you are about to pop the top */
/* element so that you don't leave orphaned arrays floating around in memory. */
/* StackPtr is the index of the top element of the stack. */
void							DisposeIfNotOnStack(StackElement* Stack, long StackPtr);

/* this is a utility routine to dispose of an array in the stack somewhere if it */
/* doesn't occur in the stack anywhere else.  It's used if you are about to make */
/* an assignment to the array slot to make sure arrays get disposed of properly. */
/* StackPtr is the index of the top element of the stack.  Where is the index */
/* of the element that is being checked. */
void							DisposeIfOnStackOnlyOnce(StackElement* Stack, long StackPtr, long Where);

/* add a parameter to the parameter stack.  If the data has to */
/* be allocated dynamically, a copy is made.  True is returned if successful. */
MyBoolean					AddIntegerToStack(ParamStackRec* Stack, long IntegerValue);
MyBoolean					AddFloatToStack(ParamStackRec* Stack, float FloatValue);
MyBoolean					AddDoubleToStack(ParamStackRec* Stack, double DoubleValue);
/* the ACTUAL array is added, so you no longer own it after this! */
MyBoolean					AddArrayToStack(ParamStackRec* Stack, void* Array);

/* routines for reading data from a parameter stack */
long							GetStackInteger(ParamStackRec* Stack, long Index);
float							GetStackFloat(ParamStackRec* Stack, long Index);
double						GetStackLongDouble(ParamStackRec* Stack, long Index);
void*							GetStackArray(ParamStackRec* Stack, long Index);

/* routines for obtaining internal stuff for the stack */
StackElement*			GetStackBase(ParamStackRec* Stack);
long							GetStackInitialSize(ParamStackRec* Stack);
long							GetStackNumElements(ParamStackRec* Stack);

/* routine for restoring changed information to stack */
void							SetStackInformation(ParamStackRec* Stack, long NewTotalSize,
										long NewNumElements, StackElement* NewStackAddress);

#endif
