/* ASTVariableDeclaration.h */

#ifndef Included_ASTVariableDeclaration_h
#define Included_ASTVariableDeclaration_h

/* ASTVariableDeclaration module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* ASTExpression */
/* SymbolTableEntry */
/* TrashTracker */
/* Memory */
/* PcodeObject */
/* CompilerRoot */
/* PromotableTypeCheck */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTVarDeclRec;
typedef struct ASTVarDeclRec ASTVarDeclRec;

/* all memory allocated in this module is through TrashTracker */

/* forwards */
struct TrashTrackRec;
struct SymbolRec;
struct ASTExpressionRec;

/* allocate a new variable declaration.  if the Initializer expression is NIL, then */
/* the object is initialized to NIL or zero when it enters scope. */
ASTVarDeclRec*			NewVariableDeclaration(struct SymbolRec* SymbolTableEntry,
											struct ASTExpressionRec* Initializer,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the variable declaration node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckVariableDeclaration(DataTypes* ResultingDataType,
											ASTVarDeclRec* VariableDeclaration, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for a variable declaration. returns True if successful, or */
/* False if it fails. */
MyBoolean						CodeGenVarDecl(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTVarDeclRec* VariableDeclaration);

#endif
