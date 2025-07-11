/* FunctionCode.h */

#ifndef Included_FunctionCode_h
#define Included_FunctionCode_h

/* FunctionCode module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* PcodeObject */
/* Memory */
/* SymbolList */
/* SymbolTableEntry */

#include "PcodeObject.h"

struct FuncCodeRec;
typedef struct FuncCodeRec FuncCodeRec;

/* forwards */
struct PcodeRec;
struct SymbolListRec;

/* dispose of a function object */
void							DisposeFunction(FuncCodeRec* Function);

/* get the actual code block for a function. */
struct PcodeRec*	GetFunctionPcode(FuncCodeRec* FuncCode);

/* get the name of a function (actual, not copy).  it's a pointer into the heap */
char*							GetFunctionName(FuncCodeRec* FuncCode);

/* get the list of parameters for a function.  it returns a heap block array */
/* of data types with fields from left parameter to right parameter. */
DataTypes*				GetFunctionParameterTypeList(FuncCodeRec* FuncCode);

/* get the data type a function returns.  could be undefined. */
DataTypes					GetFunctionReturnType(FuncCodeRec* FuncCode);

/* create a new function thing */
FuncCodeRec*			NewFunction(char* FuncName, long NameLength,
										struct SymbolListRec* Parameters, struct PcodeRec* PcodeThing,
										DataTypes ReturnType);
#endif
