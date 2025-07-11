/* PcodeSystem.h */

#ifndef Included_PcodeSystem_h
#define Included_PcodeSystem_h

/* PcodeSystem module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* EventLoop */
/* DataMunging */
/* Alert */
/* FixedPoint */
/* FloatingPoint */
/* PcodeStack */
/* PcodeObject */
/* CodeCenter */
/* SampleConsts */
/* FunctionCode */

#include "FixedPoint.h"
#include "SampleConsts.h"

/* forward declarations */
struct ParamStackRec;
struct PcodeRec;
struct CodeCenterRec;
union OpcodeRec;

/* the errors that can occur when evaluating a pcode program */
typedef enum
	{
		eEvalNoError EXECUTE(= -7865),
		eEvalUndefinedFunction,
		eEvalWrongNumParametersForFunction,
		eEvalWrongParameterType,
		eEvalWrongReturnType,
		eEvalErrorTrapEncountered,
		eEvalUserCancelled,
		eEvalDivideByZero,
		eEvalOutOfMemory,
		eEvalArrayDoesntExist,
		eEvalArraySubscriptOutOfRange,
		eEvalGetSampleNotDefined,
		eEvalGetSampleWrongChannelType
	} EvalErrors;

/* evaluate the provided function using the initial stack provided in Prep, */
/* the program provided in Pcode, the object pool provided in CodeCenter, */
/* and the callback routines for obtaining samples.  the SampleName parameter */
/* of the callback routine is a NULL termimated string. */
/* If an error occurs, the OpcodeRec and program counter of the instruction */
/* that generated the exception are returned.  the Prep is used in place, which */
/* means that any changes made to the stack during evaluation will be available */
/* in the Prep when the function returns.  Stack depth is NOT changed, though. */
EvalErrors				EvaluatePcode(struct ParamStackRec* Prep, struct PcodeRec* Pcode,
										struct CodeCenterRec* CodeCenter, union OpcodeRec** OffendingPcode,
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
											largefixedsigned** WaveTableData));

/* return a static null terminated string describing an error code. */
char*							GetPcodeErrorMessage(EvalErrors Error);

#endif
