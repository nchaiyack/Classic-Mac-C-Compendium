/* CompilerRoot.h */

#ifndef Included_CompilerRoot_h
#define Included_CompilerRoot_h

/* CompilerRoot module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* PcodeObject */
/* Memory */
/* CompilerScanner */
/* TrashTracker */
/* CompilerParser */
/* SymbolTable */
/* SymbolTableEntry */
/* ASTExpression */
/* PromotableTypeCheck */
/* SymbolList */
/* CodeCenter */
/* FunctionCode */
/* PeepholeOptimizer */
/* DataMunging */
/* ASTExpressionList */

#include "PcodeObject.h"

/* forwards */
struct CodeCenterRec;
struct PcodeRec;

/* syntactical errors that can occur while compiling */
typedef enum
	{
		eCompileNoError EXECUTE(= -32412),
		eCompileOutOfMemory,
		eCompileExpectedFuncOrProto,
		eCompileExpectedFunc,
		eCompileExpectedIdentifier,
		eCompileExpectedOpenParen,
		eCompileExpectedCloseParen,
		eCompileExpectedColon,
		eCompileExpectedSemicolon,
		eCompileMultiplyDefinedIdentifier,
		eCompileExpectedTypeSpecifier,
		eCompileExpectedExpressionForm,
		eCompileExpectedColonEqual,
		eCompileExpectedTo,
		eCompileExpectedStringLiteral,
		eCompileExpectedResumable,
		eCompileExpectedWhile,
		eCompileExpectedDo,
		eCompileExpectedUntil,
		eCompileExpectedOpenParenOrEqual,
		eCompileExpectedThen,
		eCompileExpectedWhileOrUntil,
		eCompileExpectedCommaOrCloseParen,
		eCompileExpectedElseOrElseIf,
		eCompileExpectedOperatorOrStatement,
		eCompileExpectedOperand,
		eCompileIdentifierNotDeclared,
		eCompileExpectedRightAssociativeOperator,
		eCompileExpectedOpenBracket,
		eCompileExpectedCloseBracket,
		eCompileExpectedVariable,
		eCompileExpectedArrayType,
		eCompileArraySizeSpecMustBeInteger,
		eCompileTypeMismatch,
		eCompileInvalidLValue,
		eCompileOperandsMustBeScalar,
		eCompileOperandsMustBeSequencedScalar,
		eCompileOperandsMustBeIntegers,
		eCompileRightOperandMustBeInteger,
		eCompileArraySubscriptMustBeInteger,
		eCompileArrayRequiredForSubscription,
		eCompileDoubleRequiredForExponentiation,
		eCompileArrayRequiredForResize,
		eCompileIntegerRequiredForResize,
		eCompileConditionalMustBeBoolean,
		eCompileTypeMismatchBetweenThenAndElse,
		eCompileErrorNeedsBooleanArg,
		eCompileFunctionIdentifierRequired,
		eCompileArgumentTypeConflict,
		eCompileWrongNumberOfArgsToFunction,
		eCompileCantHaveStringLiteralThere,
		eCompileMustBeAVariableIdentifier,
		eCompileOperandMustBeBooleanOrInteger,
		eCompileOperandMustBeDouble,
		eCompileArrayRequiredForGetLength,
		eCompileTypeMismatchInAssignment,
		eCompileVoidExpressionIsNotAllowed,
		eCompileMultiplyDeclaredFunction,
		eCompilePrototypeCantBeLastThingInExprList,
		eCompileInputBeyondEndOfFunction,
		eCompileArrayConstructionOnScalarType
	} CompileErrors;

/* keywords */
typedef enum
	{
		eKeywordFunc EXECUTE(= -5112),
		eKeywordProto,
		eKeywordVoid,
		eKeywordBool,
		eKeywordInt,
		eKeywordSingle,
		eKeywordDouble,
		eKeywordFixed,
		eKeywordBoolarray,
		eKeywordIntarray,
		eKeywordSinglearray,
		eKeywordDoublearray,
		eKeywordFixedarray,
		eKeywordVar,
		eKeywordIf,
		eKeywordWhile,
		eKeywordDo,
		eKeywordUntil,
		eKeywordSet,
		eKeywordResize,
		eKeywordTo,
		eKeywordError,
		eKeywordResumable,
		eKeywordNot,
		eKeywordSin,
		eKeywordCos,
		eKeywordTan,
		eKeywordAsin,
		eKeywordAcos,
		eKeywordAtan,
		eKeywordLn,
		eKeywordExp,
		eKeywordSqr,
		eKeywordSqrt,
		eKeywordAbs,
		eKeywordNeg,
		eKeywordSign,
		eKeywordLength,
		eKeywordPi,
		eKeywordTrue,
		eKeywordFalse,
		eKeywordThen,
		eKeywordElse,
		eKeywordElseif,
		eKeywordAnd,
		eKeywordOr,
		eKeywordXor,
		eKeywordDiv,
		eKeywordMod,
		eKeywordGetsampleleft,
		eKeywordGetsampleright,
		eKeywordGetsample,
		eKeywordGetwavenumframes,
		eKeywordGetwavenumtables,
		eKeywordGetwavedata
	} KeywordsType;

/* this function is used for specifying information about a parameter */
typedef struct
	{
		char*						ParameterName;
		DataTypes				ParameterType;
	} FunctionParamRec;

/* compile a module.  a module is a text block with a series of function definitions. */
/* if compilation succeeds, the functions are added to the CodeCenter object. */
/* the text data is NOT altered. */
CompileErrors			CompileModule(long* ErrorLineNumber, char* TextData, void* Signature,
										struct CodeCenterRec* CodeCenter);

/* return a null terminated static string describing the error. */
char*							GetCompileErrorString(CompileErrors Error);

/* compile a special function.  a special function has no function header, but is */
/* simply some code to be executed.  the parameters the code is expecting are provided */
/* in the FuncArray[] and NumParams.  the first parameter is deepest beneath the */
/* top of stack.  the TextData is NOT altered.  if an error occurrs, *FunctionOut */
/* will NOT contain a valid object */
CompileErrors			CompileSpecialFunction(FunctionParamRec FuncArray[], long NumParams,
										long* ErrorLineNumber, DataTypes* ReturnType, char* TextData,
										struct PcodeRec** FunctionOut);

#endif
