/* ASTArrayDeclaration.h */

#ifndef Included_ASTArrayDeclaration_h
#define Included_ASTArrayDeclaration_h

/* ASTArrayDeclaration module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* TrashTracker */
/* Memory */
/* SymbolTableEntry */
/* ASTExpression */
/* PcodeObject */
/* CompilerRoot */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTArrayDeclRec;
typedef struct ASTArrayDeclRec ASTArrayDeclRec;

/* this module allocates all memory through TrashTracker */

/* forwards */
struct TrashTrackRec;
struct SymbolRec;
struct ASTExpressionRec;
struct PcodeRec;

/* create a new array variable constructor node.  this should ONLY be used for */
/* creating arrays.  variables that are initialized with an array that results from */
/* an expression should use ASTVariableDeclaration. */
ASTArrayDeclRec*		NewArrayConstruction(struct SymbolRec* SymbolTableEntry,
											struct ASTExpressionRec* SizeExpression,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the array variable constructor node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckArrayConstruction(DataTypes* ResultingDataType,
											ASTArrayDeclRec* ArrayConstructor, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for array declaration.  returns True if successful, or False if */
/* it fails. */
MyBoolean						CodeGenArrayConstruction(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTArrayDeclRec* ArrayConstructor);

#endif
