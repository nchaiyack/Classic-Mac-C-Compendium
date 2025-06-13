/* CompilerParser.h */

#ifndef Included_CompilerParser_h
#define Included_CompilerParser_h

/* CompilerParser module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* CompilerRoot */
/* SymbolTableEntry */
/* ASTExpression */
/* SymbolTable */
/* TrashTracker */
/* SymbolList */
/* PcodeObject */
/* Memory */
/* ASTAssignment */
/* ASTBinaryOperator */
/* ASTErrorForm */
/* ASTLoop */
/* ASTArrayDeclaration */
/* ASTVariableDeclaration */
/* ASTConditional */
/* ASTUnaryOperator */
/* ASTOperand */
/* ASTFuncCall */
/* ASTExpressionList */
/* ASTWaveGetter */
/* FloatingPoint */

#include "CompilerRoot.h" /* error type */

/* all objects created from this routine are allocated with TrashTracker. */

/* forward declarations */
struct SymbolTableRec;
struct TrashTrackRec;
struct SymbolRec;
struct ASTExpressionRec;
struct ScannerRec;
struct SymbolTableRec;
struct ASTExprListRec;

/* parse a top-level form, which is either a prototype or a function.  prototypes */
/* are entered into the symbol table and return NIL in *FunctionBodyOut but returns */
/* eCompileNoError. */
/*   1:   <form>             ::= <function> ; */
/*   2:                      ::= <prototype> ; */
/* FIRST SET: */
/* <form>             : {func, proto, <function>, <prototype>} */
/* FOLLOW SET: */
/* <form>             : {$$$} */
CompileErrors					ParseForm(struct SymbolRec** FunctionSymbolTableEntryOut,
												struct ASTExpressionRec** FunctionBodyOut,
												struct ScannerRec* Scanner, struct SymbolTableRec* SymbolTable,
												struct TrashTrackRec* TrashTracker, long* LineNumberOut);


/*   22:   <expr>             ::= <prototype> */
/*   26:   <expr>             ::= <expr2> */
/*  109:   <expr>             ::= if <ifrest> */
/*  114:   <expr>             ::= <whileloop> */
/*  115:                      ::= do <expr> <loopwhileuntil> */
/*  116:                      ::= <untilloop> */
/*  121:   <expr>             ::= set <expr> := <expr> */
/*  125:   <expr>             ::= resize <expr> to <expr> */
/*  126:                      ::= error <string> resumable <expr> */
/* FIRST SET: */
/*  <expr>             : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, cos, */
/*       tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, if, */
/*       while, until, do, resize, error, maxint, minint, maxsingle, minsingle, */
/*       maxdouble, mindouble, maxfixed, minfixed, true, false, set, (, -, */
/*       <prototype>, <expr2>, <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, */
/*       <expr7>, <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <expr>             : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
CompileErrors					ParseExpr(struct ASTExpressionRec** ExpressionOut,
												struct ScannerRec* Scanner, struct SymbolTableRec* SymbolTable,
												struct TrashTrackRec* TrashTracker, long* LineNumberOut);

/*  124:   <exprlist>         ::= <exprlistelem> <exprlisttail> */
/* FIRST SET: */
/*  <exprlist>         : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, */
/*       cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, */
/*       length, if, while, until, do, resize, error, maxint, minint, maxsingle, */
/*       minsingle, maxdouble, mindouble, maxfixed, minfixed, true, false, */
/*       set, (, -, <prototype>, <expr>, <expr2>, <expr3>, <expr4>, <expr5>, */
/*       <expr6>, <unary_oper>, <expr7>, <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <exprlist>         : {), CLOSEBRACKET} */
CompileErrors					ParseExprList(struct ASTExprListRec** ExpressionOut,
												struct ScannerRec* Scanner, struct SymbolTableRec* SymbolTable,
												struct TrashTrackRec* TrashTracker, long* LineNumberOut);

#endif
