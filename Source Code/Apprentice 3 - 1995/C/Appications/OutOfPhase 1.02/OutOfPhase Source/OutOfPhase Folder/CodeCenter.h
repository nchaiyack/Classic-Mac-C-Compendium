/* CodeCenter.h */

#ifndef Included_CodeCenter_h
#define Included_CodeCenter_h

/* CodeCenter module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Array */
/* Memory */
/* FunctionCode */
/* PcodeObject */
/* DataMunging */

struct CodeCenterRec;
typedef struct CodeCenterRec CodeCenterRec;

/* forwards */
struct FuncCodeRec;
union OpcodeRec;

/* create new object code storage database */
CodeCenterRec*			NewCodeCenter(void);

/* delete object code database and all objects in it */
void								DisposeCodeCenter(CodeCenterRec* CodeCenter);

/* if we have the pcode, but not the function, then find it */
/* it returns NIL if the function couldn't be found. */
struct FuncCodeRec*	GetFunctionFromOpcode(CodeCenterRec* CodeCenter,
											union OpcodeRec* Opcode);

/* obtain a handle for the named function */
struct FuncCodeRec*	ObtainFunctionHandle(CodeCenterRec* CodeCenter, char* FunctionName,
											long FuncNameLength);

/* find out how many functions are known by the code center */
long								CodeCenterGetNumFunctions(CodeCenterRec* CodeCenter);

/* delete all object code from a particular code module & delink references */
void								FlushModulesCompiledFunctions(CodeCenterRec* CodeCenter,
											void* Signature);

/* get a list of functions owned by a specified code module */
struct ArrayRec*		GetListOfFunctionsForModule(CodeCenterRec* CodeCenter,
											void* Signature);

/* find out if a function with the given name exists */
MyBoolean						CodeCenterHaveThisFunction(CodeCenterRec* CodeCenter,
											char* FunctionName, long FuncNameLength);

/* add this function to the code center.  it better not be in there already */
MyBoolean						AddFunctionToCodeCenter(CodeCenterRec* CodeCenter,
											struct FuncCodeRec* TheNewFunction, void* Signature);

#endif
