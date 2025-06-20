/* ASTOperand.h */

#ifndef Included_ASTOperand_h
#define Included_ASTOperand_h

/* ASTOperand module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* FixedPoint */
/* TrashTracker */
/* Memory */
/* SymbolTableEntry */
/* PcodeObject */
/* CompilerRoot */

#include "PcodeObject.h"
#include "CompilerRoot.h"
#include "FixedPoint.h"

struct ASTOperandRec;
typedef struct ASTOperandRec ASTOperandRec;

/* all memory allocated from this module is through TrashTracker */

/* operand types */
typedef enum
	{
		eASTOperandIntegerLiteral EXECUTE(= -22512),
		eASTOperandBooleanLiteral,
		eASTOperandSingleLiteral,
		eASTOperandDoubleLiteral,
		eASTOperandFixedLiteral,
#if 0
		eASTOperandStringLiteral,
#endif
		eASTOperandSymbol
	} ASTOperandType;

/* forwards */
struct TrashTrackRec;
struct SymbolRec;

/* create a new integer literal */
ASTOperandRec*		NewIntegerLiteral(struct TrashTrackRec* TrashTracker,
										long IntegerLiteralValue, long LineNumber);

/* create a new boolean literal */
ASTOperandRec*		NewBooleanLiteral(struct TrashTrackRec* TrashTracker,
										MyBoolean BooleanLiteralValue, long LineNumber);

/* create a new single precision literal */
ASTOperandRec*		NewSingleLiteral(struct TrashTrackRec* TrashTracker,
										float SingleLiteralValue, long LineNumber);

/* create a new double precision literal */
ASTOperandRec*		NewDoubleLiteral(struct TrashTrackRec* TrashTracker,
										double DoubleLiteralValue, long LineNumber);

/* create a new fixed-point literal */
ASTOperandRec*		NewFixedLiteral(struct TrashTrackRec* TrashTracker,
										largefixedsigned FixedLiteralValue, long LineNumber);

#if 0
/* create a new string literal.  the string should be a valid heap block and is */
/* stored in the structure (i.e. a copy is NOT made) */
ASTOperandRec*		NewStringLiteral(struct TrashTrackRec* TrashTracker,
										char* StringLiteralValue, long LineNumber);
#endif

/* create a new symbol reference. */
ASTOperandRec*		NewSymbolReference(struct TrashTrackRec* TrashTracker,
										struct SymbolRec* SymbolTableEntry, long LineNumber);

/* find out if it is a symbol */
MyBoolean					IsOperandASymbol(ASTOperandRec* Operand);

/* get a symbol from the operand */
struct SymbolRec*	GetSymbolFromOperand(ASTOperandRec* Operand);

/* type check the operand node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors			TypeCheckOperand(DataTypes* ResultingDataType,
										ASTOperandRec* Operand, long* ErrorLineNumber,
										struct TrashTrackRec* TrashTracker);

/* find out what kind of operand this is */
ASTOperandType		OperandWhatIsIt(ASTOperandRec* TheOperand);

/* generate code for an operand. returns True if successful, or False if it fails. */
MyBoolean					CodeGenOperand(struct PcodeRec* FuncCode,
										long* StackDepthParam, ASTOperandRec* Operand);

#endif
