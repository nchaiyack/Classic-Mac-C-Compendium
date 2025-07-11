/* CompilerParser.c */
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

#include "CompilerParser.h"
#include "CompilerScanner.h"
#include "SymbolTableEntry.h"
#include "ASTExpression.h"
#include "SymbolTable.h"
#include "TrashTracker.h"
#include "SymbolList.h"
#include "PcodeObject.h"
#include "Memory.h"
#include "ASTAssignment.h"
#include "ASTBinaryOperator.h"
#include "ASTErrorForm.h"
#include "ASTLoop.h"
#include "ASTArrayDeclaration.h"
#include "ASTVariableDeclaration.h"
#include "ASTConditional.h"
#include "ASTUnaryOperator.h"
#include "ASTOperand.h"
#include "ASTFuncCall.h"
#include "ASTExpressionList.h"
#include "ASTWaveGetter.h"
#include "FloatingPoint.h"


/* protoplops */
static CompileErrors	ParseFunction(SymbolRec** FunctionSymbolTableEntryOut,
												ASTExpressionRec** FunctionBodyOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParsePrototype(SymbolRec** PrototypeSymbolTableEntryOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseFormalParamStart(SymbolListRec** FormalArgListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseType(DataTypes* TypeOut, ScannerRec* Scanner,
												long* LineNumberOut);

static CompileErrors	ParseFormalParamList(SymbolListRec** FormalArgListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseVarTail(ASTExpressionRec** ExpressionOut,
												TokenRec* VariableName, long VariableDeclLine,
												DataTypes VariableType, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseIfRest(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseWhileLoop(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseLoopWhileUntil(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LoopBodyExpression, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut, long LineNumberOfLoop);

static CompileErrors	ParseUntilLoop(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseExpr2(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseFormalArg(SymbolRec** FormalArgOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseFormalArgTail(SymbolListRec** ArgListTailOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseIfTail(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* Predicate, ASTExpressionRec* Consequent,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseExpr3(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr2Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr4(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr3Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseConjOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut);

static CompileErrors	ParseExpr5(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr4Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseRelOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut);

static CompileErrors	ParseExpr6(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr5Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseAddOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut);

static CompileErrors	ParseExpr7(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseMultOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut);

static CompileErrors	ParseExpr8(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExpr7Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* TheExpr8Thing, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseFuncCall(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* FunctionGenerator, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseArraySubscript(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* ArrayGenerator, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut);

static CompileErrors	ParseExponentiation(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseActualStart(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseActualList(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseActualTail(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseExprListTail(ASTExprListRec** ListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);

static CompileErrors	ParseExprListElem(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut);




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
												struct TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		CompileErrors				Error;

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* do lookahead on "func" */
		if ((GetTokenType(Token) == eTokenKeyword)
			&& (GetTokenKeywordTag(Token) == eKeywordFunc))
			{
				/* push token back */
				UngetToken(Scanner,Token);

				/* parse function definition */
				Error = ParseFunction(FunctionSymbolTableEntryOut,FunctionBodyOut,
					Scanner,SymbolTable,TrashTracker,LineNumberOut);
				if (Error != eCompileNoError)
					{
						return Error;
					}
			}

		/* do lookahead on "proto" */
		else if ((GetTokenType(Token) == eTokenKeyword)
			&& (GetTokenKeywordTag(Token) == eKeywordProto))
			{
				/* push token back */
				UngetToken(Scanner,Token);

				/* parse prototype */
				Error = ParsePrototype(FunctionSymbolTableEntryOut,Scanner,SymbolTable,
					TrashTracker,LineNumberOut);
				*FunctionBodyOut = NIL; /* no code body for a prototype */
				if (Error != eCompileNoError)
					{
						return Error;
					}
			}

		/* otherwise, it's an error */
		else
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedFuncOrProto;
			}


		/* swallow the semicolon */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenSemicolon)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedSemicolon;
			}

		return eCompileNoError;
	}




/* this parses a function declaration, returning the symbol table entry for the */
/* function in *FunctionSymbolTableEntryOut and the expression for the function */
/* in *FunctionBodyOut. */
/*  14:   <function>         ::= func <identifier> ( <formalparamstart> ) : */
/*      <type> <expr> */
/* FIRST SET: */
/* <function>         : {func} */
/* FOLLOW SET: */
/* <function>         : {;} */
static CompileErrors	ParseFunction(SymbolRec** FunctionSymbolTableEntryOut,
												ASTExpressionRec** FunctionBodyOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		TokenRec*						FunctionName;
		SymbolListRec*			FormalArgumentList;
		CompileErrors				Error;
		DataTypes						ReturnType;
		long								LineNumberOfIdentifier;

		/* swallow "func" */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordFunc))
			{
				/* this is impossible -- we should be able to do some error checking here, */
				/* but it seems uncertain since we don't (formally) know whose calling us. */
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedFunc;
			}

		/* get the identifier */
		FunctionName = GetNextToken(Scanner);
		if (FunctionName == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(FunctionName) != eTokenIdentifier)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedIdentifier;
			}
		LineNumberOfIdentifier = GetCurrentLineNumber(Scanner);

		/* add the identifier to the symbol table */
		*FunctionSymbolTableEntryOut = NewSymbol(TrashTracker,
			GetTokenIdentifierString(FunctionName),
			PtrSize(GetTokenIdentifierString(FunctionName)));
		if (*FunctionSymbolTableEntryOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		switch (AddSymbolToTable(SymbolTable,*FunctionSymbolTableEntryOut))
			{
				case eAddSymbolNoErr:
					break;
				case eAddSymbolAlreadyExists:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileMultiplyDefinedIdentifier;
				case eAddSymbolNoMemory:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileOutOfMemory;
				default:
					EXECUTE(PRERR(ForceAbort,"ParseFunction:  bad value from AddSymbolToTable"));
					break;
			}

		/* create a new lexical level */
		if (!IncrementSymbolTableLevel(SymbolTable))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* swallow the open parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedOpenParen;
			}

		/* parse <formalparamstart> */
		Error = ParseFormalParamStart(&FormalArgumentList,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* swallow the close parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedCloseParen;
			}

		/* swallow the colon */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenColon)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedColon;
			}

		/* parse the return type of the function */
		Error = ParseType(&ReturnType,Scanner,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* store the interesting information into the symbol table entry */
		SymbolBecomeFunction(*FunctionSymbolTableEntryOut,FormalArgumentList,ReturnType);

		/* parse the body of the function */
		Error = ParseExpr(FunctionBodyOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* pop lexical level */
		DecrementSymbolTableLevel(SymbolTable);

		return eCompileNoError;
	}




/* this parses a prototype of a function and returns a symbol table entry in */
/* the *PrototypeSymbolTableEntryOut place. */
/*  21:   <prototype>        ::= proto <identifier> ( <formalparamstart> ) : */
/*      <type> */
/* FIRST SET: */
/* <prototype>        : {proto} */
/* FOLLOW SET: */
/*  <prototype>        : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParsePrototype(SymbolRec** PrototypeSymbolTableEntryOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		TokenRec*						FunctionName;
		long								LineNumberOfIdentifier;
		CompileErrors				Error;
		SymbolListRec*			FormalArgumentList;
		DataTypes						ReturnType;

		/* swallow "proto" */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordProto))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedFunc;
			}

		/* get the identifier */
		FunctionName = GetNextToken(Scanner);
		if (FunctionName == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(FunctionName) != eTokenIdentifier)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedIdentifier;
			}
		LineNumberOfIdentifier = GetCurrentLineNumber(Scanner);

		/* add the identifier to the symbol table */
		*PrototypeSymbolTableEntryOut = NewSymbol(TrashTracker,
			GetTokenIdentifierString(FunctionName),
			PtrSize(GetTokenIdentifierString(FunctionName)));
		if (*PrototypeSymbolTableEntryOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		switch (AddSymbolToTable(SymbolTable,*PrototypeSymbolTableEntryOut))
			{
				case eAddSymbolNoErr:
					break;
				case eAddSymbolAlreadyExists:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileMultiplyDefinedIdentifier;
				case eAddSymbolNoMemory:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileOutOfMemory;
				default:
					EXECUTE(PRERR(ForceAbort,"ParsePrototype:  bad value from AddSymbolToTable"));
					break;
			}

		/* create a new lexical level */
		if (!IncrementSymbolTableLevel(SymbolTable))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* swallow the open parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedOpenParen;
			}

		/* parse <formalparamstart> */
		Error = ParseFormalParamStart(&FormalArgumentList,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* swallow the close parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedCloseParen;
			}

		/* swallow the colon */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenColon)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedColon;
			}

		/* parse the return type of the function */
		Error = ParseType(&ReturnType,Scanner,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* store the interesting information into the symbol table entry */
		SymbolBecomeFunction(*PrototypeSymbolTableEntryOut,FormalArgumentList,ReturnType);

		/* pop lexical level */
		DecrementSymbolTableLevel(SymbolTable);

		return eCompileNoError;
	}




/* this parses an argument list.  the argument list may be empty, in which case */
/* the empty list (NIL) is returned in *FormalArgListOut. */
/*  15:   <formalparamstart> ::= <formalparamlist> */
/*  16:                      ::=  */
/* FIRST SET: */
/* <formalparamstart> : {<identifier>, <formalparamlist>, <formalarg>} */
/* FOLLOW SET: */
/* <formalparamstart> : {)} */
static CompileErrors	ParseFormalParamStart(SymbolListRec** FormalArgListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		/* get a token so we can lookahead */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* if it's a paren, then we abort */
		if (GetTokenType(Token) == eTokenCloseParen)
			{
				/* stuff it back */
				UngetToken(Scanner,Token);

				/* we want to return the empty list since argument list is empty */
				*FormalArgListOut = NIL;

				return eCompileNoError;
			}

		/* stuff it back */
		UngetToken(Scanner,Token);

		/* it really is something, so parse it */
		return ParseFormalParamList(FormalArgListOut,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
	}




/* this function parses a type and returns the corresponding enumeration value */
/* in *TypeOut. */
/*   3:   <type>             ::= void */
/*   4:                      ::= bool */
/*   5:                      ::= int */
/*   6:                      ::= single */
/*   7:                      ::= double */
/*   8:                      ::= fixed */
/*   9:                      ::= boolarray */
/*  10:                      ::= intarray */
/*  11:                      ::= singlearray */
/*  12:                      ::= doublearray */
/*  13:                      ::= fixedarray */
/* FIRST SET: */
/* <type>             : {void, bool, int, single, double, fixed, boolarray, */
/*      intarray, singlearray, doublearray, fixedarray} */
/* FOLLOW SET: */
/*  <type>             : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, cos, */
/*       tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, if, */
/*       then, else, elseif, while, until, do, resize, to, error, true, */
/*       false, set, (, ), CLOSEBRACKET, ,, :=, ;, -, EQ, <prototype>, <expr>, */
/*       <formalargtail>, <vartail>, <expr2>, <expr3>, <expr4>, <expr5>, <expr6>, */
/*       <unary_oper>, <expr7>, <expr8>, <actualtail>, <iftail>, <whileloop>, */
/*       <loopwhileuntil>, <untilloop>, <exprlisttail>} */
static CompileErrors	ParseType(DataTypes* TypeOut, ScannerRec* Scanner,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		/* get the word */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* make sure it's a keyword */
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedTypeSpecifier;
			}

		/* do the decoding */
		switch (GetTokenKeywordTag(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedTypeSpecifier;
				case eKeywordVoid:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileVoidExpressionIsNotAllowed;
				case eKeywordBool:
					*TypeOut = eBoolean;
					break;
				case eKeywordInt:
					*TypeOut = eInteger;
					break;
				case eKeywordSingle:
					*TypeOut = eFloat;
					break;
				case eKeywordDouble:
					*TypeOut = eDouble;
					break;
				case eKeywordFixed:
					*TypeOut = eFixed;
					break;
				case eKeywordBoolarray:
					*TypeOut = eArrayOfBoolean;
					break;
				case eKeywordIntarray:
					*TypeOut = eArrayOfInteger;
					break;
				case eKeywordSinglearray:
					*TypeOut = eArrayOfFloat;
					break;
				case eKeywordDoublearray:
					*TypeOut = eArrayOfDouble;
					break;
				case eKeywordFixedarray:
					*TypeOut = eArrayOfFixed;
					break;
			}

		return eCompileNoError;
	}




/*   26:   <expr>             ::= <expr2> */
/*  109:   <expr>             ::= if <ifrest> */
/*  114:   <expr>             ::= <whileloop> */
/*  115:                      ::= do <expr> <loopwhileuntil> */
/*  116:                      ::= <untilloop> */
/*  121:   <expr>             ::= set <expr> := <expr> */
/*  125:   <expr>             ::= resize <expr> to <expr> */
/*  126:                      ::= error <string> resumable <expr> */
/*  XXX:                      ::= getsampleleft <string> */
/*  XXX:                      ::= getsampleright <string> */
/*  XXX:                      ::= getsample <string> */
/*  XXX:                      ::= getwavenumframes <string> */
/*  XXX:                      ::= getwavenumtables <string> */
/*  XXX:                      ::= getwavedata <string> */
/* FIRST SET: */
/*  <expr>             : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, cos, */
/*       tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, if, */
/*       while, until, do, resize, error, true, false, set, (, -, */
/*       <prototype>, <expr2>, <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, */
/*       <expr7>, <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <expr>             : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
CompileErrors					ParseExpr(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		long								LineNumberForFirstToken;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		/* get the first token to be gotten */
		Token = GetNextToken(Scanner);
		LineNumberForFirstToken = GetCurrentLineNumber(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = LineNumberForFirstToken;
				return eCompileOutOfMemory;
			}

		/* see what action should be taken */
		switch (GetTokenType(Token))
			{
				default:
					UngetToken(Scanner,Token);
					return ParseExpr2(ExpressionOut,Scanner,SymbolTable,TrashTracker,
						LineNumberOut);

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								UngetToken(Scanner,Token);
								return ParseExpr2(ExpressionOut,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);

							/*  109:   <expr>             ::= if <ifrest> */
							case eKeywordIf:
								return ParseIfRest(ExpressionOut,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);

							/*  114:   <expr>             ::= <whileloop> */
							/* FIRST SET: */
							/*  <whileloop>        : {while} */
							case eKeywordWhile:
								UngetToken(Scanner,Token);
								return ParseWhileLoop(ExpressionOut,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);

							/*  115:                      ::= do <expr> <loopwhileuntil> */
							case eKeywordDo:
								{
									ASTExpressionRec*		BodyExpression;
									CompileErrors				Error;

									Error = ParseExpr(&BodyExpression,Scanner,SymbolTable,
										TrashTracker,LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									/* parse the rest of it */
									return ParseLoopWhileUntil(ExpressionOut,BodyExpression,Scanner,
										SymbolTable,TrashTracker,LineNumberOut,LineNumberForFirstToken);
								}

							/*  116:                      ::= <untilloop> */
							/* FIRST SET */
							/*  <untilloop>        : {until} */
							case eKeywordUntil:
								UngetToken(Scanner,Token);
								return ParseUntilLoop(ExpressionOut,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);

							/*  121:   <expr>             ::= set <expr> := <expr> */
							case eKeywordSet:
								{
									ASTExpressionRec*		LValue;
									ASTExpressionRec*		RValue;
									CompileErrors				Error;
									ASTAssignRec*				TotalAssignment;

									Error = ParseExpr(&LValue,Scanner,SymbolTable,TrashTracker,
										LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									/* swallow the colon-equals */
									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenColonEqual)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedColonEqual;
										}

									Error = ParseExpr(&RValue,Scanner,SymbolTable,TrashTracker,
										LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									TotalAssignment = NewAssignment(LValue,RValue,TrashTracker,
										LineNumberForFirstToken);
									if (TotalAssignment == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprAssignment(TotalAssignment,TrashTracker,
										LineNumberForFirstToken);
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  125:   <expr>             ::= resize <expr> to <expr> */
							case eKeywordResize:
								{
									ASTExpressionRec*		ArrayGenerator;
									ASTExpressionRec*		NewSizeExpression;
									CompileErrors				Error;
									ASTBinaryOpRec*			BinaryOperator;

									Error = ParseExpr(&ArrayGenerator,Scanner,SymbolTable,TrashTracker,
										LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									/* swallow the to */
									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if ((GetTokenType(Token) != eTokenKeyword)
										|| (GetTokenKeywordTag(Token) != eKeywordTo))
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedTo;
										}

									Error = ParseExpr(&NewSizeExpression,Scanner,SymbolTable,TrashTracker,
										LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									BinaryOperator = NewBinaryOperator(eBinaryResizeArray,ArrayGenerator,
										NewSizeExpression,TrashTracker,LineNumberForFirstToken);
									if (BinaryOperator == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprBinaryOperator(BinaryOperator,TrashTracker,
										LineNumberForFirstToken);
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  126:                      ::= error <string> resumable <expr> */
							case eKeywordError:
								{
									TokenRec*						MessageString;
									ASTExpressionRec*		ResumableCondition;
									CompileErrors				Error;
									ASTErrorFormRec*		ErrorForm;

									MessageString = GetNextToken(Scanner);
									if (MessageString == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(MessageString) != eTokenString)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileExpectedStringLiteral;
										}

									/* swallow the resumable */
									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if ((GetTokenType(Token) != eTokenKeyword)
										|| (GetTokenKeywordTag(Token) != eKeywordResumable))
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedResumable;
										}

									Error = ParseExpr(&ResumableCondition,Scanner,SymbolTable,
										TrashTracker,LineNumberOut);
									if (Error != eCompileNoError)
										{
											return Error;
										}

									ErrorForm = NewErrorForm(ResumableCondition,
										GetTokenStringValue(MessageString),TrashTracker,
										LineNumberForFirstToken);
									if (ErrorForm == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprErrorForm(ErrorForm,TrashTracker,
										LineNumberForFirstToken);
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = LineNumberForFirstToken;
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getsampleleft <string> */
							case eKeywordGetsampleleft:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterSampleLeft,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getsampleright <string> */
							case eKeywordGetsampleright:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterSampleRight,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getsample <string> */
							case eKeywordGetsample:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterSampleMono,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getwavenumframes <string> */
							case eKeywordGetwavenumframes:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterWaveFrames,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getwavenumtables <string> */
							case eKeywordGetwavenumtables:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterWaveTables,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}

							/*  XXX:                      ::= getwavedata <string> */
							case eKeywordGetwavedata:
								{
									ASTWaveGetterRec*		WaveGetterThang;

									Token = GetNextToken(Scanner);
									if (Token == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}
									if (GetTokenType(Token) != eTokenString)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileExpectedStringLiteral;
										}

									WaveGetterThang = NewWaveGetter(GetTokenStringValue(Token),
										eWaveGetterWaveArray,TrashTracker,GetCurrentLineNumber(Scanner));
									if (WaveGetterThang == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									*ExpressionOut = NewExprWaveGetter(WaveGetterThang,TrashTracker,
										GetCurrentLineNumber(Scanner));
									if (*ExpressionOut == NIL)
										{
											*LineNumberOut = GetCurrentLineNumber(Scanner);
											return eCompileOutOfMemory;
										}

									return eCompileNoError;
								}
						}
			}
		EXECUTE(PRERR(ForceAbort,"ParseExpr:  control reached end of function"));
	}




/*   17:   <formalparamlist>  ::= <formalarg> <formalargtail>  */
/* FIRST SET: */
/*  <formalparamlist>  : {<identifier>, <formalarg>} */
/* FOLLOW SET: */
/*  <formalparamlist>  : {)} */
static CompileErrors	ParseFormalParamList(SymbolListRec** FormalArgListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		CompileErrors				Error;
		SymbolRec*					FormalArgOut;
		SymbolListRec*			ListTail;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseFormalArg(&FormalArgOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		Error = ParseFormalArgTail(&ListTail,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		*FormalArgListOut = SymbolListCons(FormalArgOut,ListTail,TrashTracker);
		if (*FormalArgListOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*  117:   <whileloop>        ::= while <expr> do <expr> */
/* FIRST SET: */
/*  <whileloop>        : {while} */
/* FOLLOW SET: */
/*  <whileloop>        : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseWhileLoop(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTExpressionRec*		ConditionalExpr;
		ASTExpressionRec*		BodyExpr;
		CompileErrors				Error;
		ASTLoopRec*					WhileLoopThing;
		long								LineNumberOfWholeForm;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		LineNumberOfWholeForm = GetCurrentLineNumber(Scanner);

		/* munch while */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordWhile))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedWhile;
			}

		Error = ParseExpr(&ConditionalExpr,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* munch do */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordDo))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedDo;
			}

		Error = ParseExpr(&BodyExpr,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		WhileLoopThing = NewLoop(eLoopWhileDo,ConditionalExpr,BodyExpr,TrashTracker,
			LineNumberOfWholeForm);
		if (WhileLoopThing == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprLoop(WhileLoopThing,TrashTracker,LineNumberOfWholeForm);
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*  118:   <untilloop>        ::= until <expr> do <expr> */
/* FIRST SET: */
/*  <untilloop>        : {until} */
/* FOLLOW SET: */
/*  <untilloop>        : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseUntilLoop(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTExpressionRec*		ConditionalExpr;
		ASTExpressionRec*		BodyExpr;
		CompileErrors				Error;
		ASTLoopRec*					UntilLoopThing;
		long								LineNumberOfWholeForm;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		LineNumberOfWholeForm = GetCurrentLineNumber(Scanner);

		/* munch until */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordUntil))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedUntil;
			}

		Error = ParseExpr(&ConditionalExpr,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* munch do */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordDo))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedDo;
			}

		Error = ParseExpr(&BodyExpr,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		UntilLoopThing = NewLoop(eLoopUntilDo,ConditionalExpr,BodyExpr,TrashTracker,
			LineNumberOfWholeForm);
		if (UntilLoopThing == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprLoop(UntilLoopThing,TrashTracker,LineNumberOfWholeForm);
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   24:   <vartail>          ::= EQ <expr> */
/*   25:                      ::= ( <expr> ) */
/* FIRST SET: */
/*  <vartail>          : {(, EQ} */
/* FOLLOW SET: */
/*  <vartail>          : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseVarTail(ASTExpressionRec** ExpressionOut,
												TokenRec* VariableName, long VariableDeclLine,
												DataTypes VariableType, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		SymbolRec*					SymbolTableEntry;

		CheckPtrExistence(VariableName);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* create symbol table entry */
		SymbolTableEntry = NewSymbol(TrashTracker,GetTokenIdentifierString(
			VariableName),PtrSize(GetTokenIdentifierString(VariableName)));
		if (SymbolTableEntry == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		SymbolBecomeVariable(SymbolTableEntry,VariableType);

		/* see what to do */
		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOpenParenOrEqual;

				/* array declaration */
				case eTokenOpenParen:
					{
						ASTExpressionRec*			ArraySizeExpression;
						CompileErrors					Error;
						ASTArrayDeclRec*			ArrayConstructor;

						if ((VariableType != eArrayOfBoolean) && (VariableType != eArrayOfInteger)
							&& (VariableType != eArrayOfFloat) && (VariableType != eArrayOfDouble)
							&& (VariableType != eArrayOfFixed))
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileArrayConstructionOnScalarType;
							}

						Error = ParseExpr(&ArraySizeExpression,Scanner,SymbolTable,TrashTracker,
							LineNumberOut);
						if (Error != eCompileNoError)
							{
								return Error;
							}

						/* swallow the close paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenCloseParen)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileExpectedCloseParen;
							}

						/* build the array constructor node */
						ArrayConstructor = NewArrayConstruction(SymbolTableEntry,
							ArraySizeExpression,TrashTracker,VariableDeclLine);
						if (ArrayConstructor == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}

						/* build AST node */
						*ExpressionOut = NewExprArrayDecl(ArrayConstructor,TrashTracker,
							VariableDeclLine);
						if (*ExpressionOut == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
					}
					break;

				/* variable construction */
				case eTokenEqual:
					{
						ASTExpressionRec*			Initializer;
						CompileErrors					Error;
						ASTVarDeclRec*				VariableConstructor;

						Error = ParseExpr(&Initializer,Scanner,SymbolTable,TrashTracker,
							LineNumberOut);
						if (Error != eCompileNoError)
							{
								return Error;
							}

						/* build variable thing */
						VariableConstructor = NewVariableDeclaration(SymbolTableEntry,
							Initializer,TrashTracker,VariableDeclLine);
						if (VariableConstructor == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}

						/* encapsulate */
						*ExpressionOut = NewExprVariableDeclaration(VariableConstructor,TrashTracker,
							VariableDeclLine);
						if (*ExpressionOut == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
					}
					break;
			}

		/* add the identifier to the symbol table */
		switch (AddSymbolToTable(SymbolTable,SymbolTableEntry))
			{
				case eAddSymbolNoErr:
					break;
				case eAddSymbolAlreadyExists:
					*LineNumberOut = VariableDeclLine;
					return eCompileMultiplyDefinedIdentifier;
				case eAddSymbolNoMemory:
					*LineNumberOut = VariableDeclLine;
					return eCompileOutOfMemory;
				default:
					EXECUTE(PRERR(ForceAbort,"ParseVarTail:  bad value from AddSymbolToTable"));
					break;
			}

		return eCompileNoError;
	}




/*  110:   <ifrest>           ::= <expr> then <expr> <iftail> */
/* FIRST SET: */
/*  <ifrest>           : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, */
/*       cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, */
/*       if, while, until, do, resize, error, true, false, set, (, -, */
/*       <prototype>, <expr>, <expr2>, <expr3>, <expr4>, <expr5>, <expr6>, */
/*       <unary_oper>, <expr7>, <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <ifrest>           : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseIfRest(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		CompileErrors				Error;
		ASTExpressionRec*		Predicate;
		ASTExpressionRec*		Consequent;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr(&Predicate,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* eat the "then" */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordThen))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedThen;
			}

		Error = ParseExpr(&Consequent,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseIfTail(ExpressionOut,Predicate,Consequent,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
	}




/*  119:   <loopwhileuntil>   ::= while <expr> */
/*  120:                      ::= until <expr> */
/* FIRST SET: */
/*  <loopwhileuntil>   : {while, until} */
/* FOLLOW SET: */
/*  <loopwhileuntil>   : {then, else, elseif, while, until, do, to, */
/*       ), CLOSEBRACKET, ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, */
/*       <exprlisttail>} */
static CompileErrors	ParseLoopWhileUntil(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LoopBodyExpression, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut, long LineNumberOfLoop)
	{
		TokenRec*						Token;
		LoopTypes						LoopKind;
		ASTExpressionRec*		ConditionalExpression;
		CompileErrors				Error;
		ASTLoopRec*					LoopThang;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(LoopBodyExpression);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		/* see what there is to do */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedWhileOrUntil;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedWhileOrUntil;

				case eKeywordWhile:
					LoopKind = eLoopDoWhile;
					break;

				case eKeywordUntil:
					LoopKind = eLoopDoUntil;
					break;
			}

		Error = ParseExpr(&ConditionalExpression,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		LoopThang = NewLoop(LoopKind,ConditionalExpression,LoopBodyExpression,
			TrashTracker,LineNumberOfLoop);
		if (LoopThang == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprLoop(LoopThang,TrashTracker,LineNumberOfLoop);
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   27:   <expr2>            ::= <expr3> <expr2prime> */
/* FIRST SET: */
/*  <expr2>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, asin, */
/*       acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, true, */
/*       false, (, -, <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, */
/*       <expr8>} */
/* FOLLOW SET: */
/*  <expr2>            : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr2(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		CompileErrors				Error;
		ASTExpressionRec*		LeftHandSide;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr3(&LeftHandSide,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseExpr2Prime(ExpressionOut,LeftHandSide,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
	}




/*   18:   <formalarg>        ::= <identifier> : <type> */
/* FIRST SET: */
/*  <formalarg>        : {<identifier>} */
/* FOLLOW SET: */
/*  <formalarg>        : {), ,, <formalargtail>} */
static CompileErrors	ParseFormalArg(SymbolRec** FormalArgOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						IdentifierName;
		TokenRec*						Token;
		DataTypes						Type;
		CompileErrors				Error;
		long								LineNumberOfIdentifier;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		LineNumberOfIdentifier = GetCurrentLineNumber(Scanner);

		IdentifierName = GetNextToken(Scanner);
		if (IdentifierName == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(IdentifierName) != eTokenIdentifier)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedIdentifier;
			}

		/* swallow the colon */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenColon)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedColon;
			}

		Error = ParseType(&Type,Scanner,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		*FormalArgOut = NewSymbol(TrashTracker,GetTokenIdentifierString(IdentifierName),
			PtrSize(GetTokenIdentifierString(IdentifierName)));
		if (*FormalArgOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		SymbolBecomeVariable(*FormalArgOut,Type);

		switch (AddSymbolToTable(SymbolTable,*FormalArgOut))
			{
				case eAddSymbolNoErr:
					break;
				case eAddSymbolAlreadyExists:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileMultiplyDefinedIdentifier;
				case eAddSymbolNoMemory:
					*LineNumberOut = LineNumberOfIdentifier;
					return eCompileOutOfMemory;
				default:
					EXECUTE(PRERR(ForceAbort,"ParseFormalArg:  bad value from AddSymbolToTable"));
					break;
			}

		return eCompileNoError;
	}




/*   19:   <formalargtail>    ::= , <formalparamlist> */
/*   20:                      ::=  */
/* FIRST SET: */
/*  <formalargtail>    : {,} */
/* FOLLOW SET: */
/*  <formalargtail>    : {)} */
static CompileErrors	ParseFormalArgTail(SymbolListRec** ArgListTailOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedCommaOrCloseParen;

				case eTokenComma:
					return ParseFormalParamList(ArgListTailOut,Scanner,SymbolTable,TrashTracker,
						LineNumberOut);

				case eTokenCloseParen:
					UngetToken(Scanner,Token);
					*ArgListTailOut = NIL; /* end of list */
					return eCompileNoError;
			}
		EXECUTE(PRERR(ForceAbort,"ParseFormalArgTail:  control reaches end of function"));
	}




/*  111:   <iftail>           ::= else <expr> */
/*  112:                      ::= elseif <ifrest> */
/*  113:                      ::=  */
/* FIRST SET: */
/*  <iftail>           : {else, elseif} */
/* FOLLOW SET: */
/*  <iftail>           : {then, else, elseif, while, until, do, to, ), CLOSEBRACKET, */
/*       ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
/* note that 'else' and 'elseif' are in both the first and follow set.  this is */
/* because if-then-else isn't LL(1).  we handle this by binding else to the deepest */
/* if statement. */
static CompileErrors	ParseIfTail(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* Predicate, ASTExpressionRec* Consequent,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTCondRec*					Conditional;
		ASTExpressionRec*		Alternative;
		CompileErrors				Error;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		/* see what the token is */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* do the operation */
		switch (GetTokenType(Token))
			{
					/*  113:                      ::=  */
				default:
				 NullificationPoint:
					UngetToken(Scanner,Token);
					Conditional = NewConditional(Predicate,Consequent,NIL,TrashTracker,
						GetCurrentLineNumber(Scanner));
					if (Conditional == NIL)
						{
							*LineNumberOut = GetCurrentLineNumber(Scanner);
							return eCompileOutOfMemory;
						}
					break;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
								/*  113:                      ::=  */
							default:
								goto NullificationPoint;

								/*  111:   <iftail>           ::= else <expr> */
							case eKeywordElse:
								Error = ParseExpr(&Alternative,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								Conditional = NewConditional(Predicate,Consequent,Alternative,
									TrashTracker,GetCurrentLineNumber(Scanner));
								if (Conditional == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								break;

								/*  112:                      ::= elseif <ifrest> */
							case eKeywordElseif:
								Error = ParseIfRest(&Alternative,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								Conditional = NewConditional(Predicate,Consequent,Alternative,
									TrashTracker,GetCurrentLineNumber(Scanner));
								if (Conditional == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								break;
						}
					break;
			}

		/* finish building expression node */
		*ExpressionOut = NewExprConditional(Conditional,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   33:   <expr3>            ::= <expr4> <expr3prime> */
/* FIRST SET: */
/*  <expr3>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, asin, */
/*       acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, true, */
/*       false, (, -, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, <expr8>} */
/* FOLLOW SET: */
/*  <expr3>            : {and, or, xor, then, else, elseif, while, until, do, */
/*       to, ), CLOSEBRACKET, ,, :=, ;, <expr2prime>, <conj_oper>, <actualtail>, */
/*       <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr3(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		CompileErrors				Error;
		ASTExpressionRec*		LeftHandSide;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr4(&LeftHandSide,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseExpr3Prime(ExpressionOut,LeftHandSide,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
	}




/*   28:   <expr2prime>       ::= <conj_oper> <expr3> <expr2prime> */
/*   29:                      ::=  */
/* FIRST SET: */
/*  <expr2prime>       : {and, or, xor, <conj_oper>} */
/* FOLLOW SET: */
/*  <expr2prime>       : {then, else, elseif, while, until, do, to, ), */
/*       CLOSEBRACKET, ,, :=, ;, <actualtail>, <iftail>, <loopwhileuntil>, */
/*       <exprlisttail>} */
static CompileErrors	ParseExpr2Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		BinaryOpType				OperatorType;
		CompileErrors				Error;
		ASTExpressionRec*		RightHandSide;
		ASTBinaryOpRec*			WholeOperator;
		ASTExpressionRec*		ThisWholeNode;

		CheckPtrExistence(LeftHandSide);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
				 NullifyPoint:
					UngetToken(Scanner,Token);
					*ExpressionOut = LeftHandSide;
					return eCompileNoError;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								goto NullifyPoint;

							case eKeywordAnd:
							case eKeywordOr:
							case eKeywordXor:
								/* actually do the thing */
								UngetToken(Scanner,Token);
								Error = ParseConjOper(&OperatorType,Scanner,LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								Error = ParseExpr3(&RightHandSide,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								/* build the operator node */
								WholeOperator = NewBinaryOperator(OperatorType,LeftHandSide,
									RightHandSide,TrashTracker,GetCurrentLineNumber(Scanner));
								if (WholeOperator == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								ThisWholeNode = NewExprBinaryOperator(WholeOperator,TrashTracker,
									GetCurrentLineNumber(Scanner));
								if (ThisWholeNode == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								return ParseExpr2Prime(ExpressionOut,ThisWholeNode,Scanner,SymbolTable,
									TrashTracker,LineNumberOut);
						}
					break;
			}
		EXECUTE(PRERR(ForceAbort,"ParseExpr2Prime:  control reached end of function"));
	}




/*   42:   <expr4>            ::= <expr5> <expr4prime> */
/* FIRST SET: */
/*  <expr4>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, asin, */
/*       acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, true, */
/*       false, (, -, <expr5>, <expr6>, <unary_oper>, <expr7>, <expr8>} */
/* FOLLOW SET: */
/*  <expr4>            : {and, or, xor, then, else, elseif, while, until, do, */
/*       to, ), CLOSEBRACKET, ,, :=, ;, EQ, NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, */
/*       <conj_oper>, <expr3prime>, <rel_oper>, <actualtail>, <iftail>, */
/*       <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr4(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		CompileErrors				Error;
		ASTExpressionRec*		LeftHandSide;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr5(&LeftHandSide,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseExpr4Prime(ExpressionOut,LeftHandSide,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
	}




/*   34:   <expr3prime>       ::= <rel_oper> <expr4> <expr3prime> */
/*   35:                      ::=  */
/* FIRST SET: */
/*  <expr3prime>       : {EQ, NEQ, LT, LTEQ, GR, GREQ, <rel_oper>} */
/* FOLLOW SET: */
/*  <expr3prime>       : {and, or, xor, then, else, elseif, while, until, */
/*       do, to, ), CLOSEBRACKET, ,, :=, ;, <expr2prime>, <conj_oper>, */
/*       <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr3Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		BinaryOpType				OperatorType;
		CompileErrors				Error;
		ASTExpressionRec*		RightHandSide;
		ASTBinaryOpRec*			WholeOperator;
		ASTExpressionRec*		ThisWholeNode;

		CheckPtrExistence(LeftHandSide);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					UngetToken(Scanner,Token);
					*ExpressionOut = LeftHandSide;
					return eCompileNoError;

				case eTokenLessThan:
				case eTokenLessThanOrEqual:
				case eTokenGreaterThan:
				case eTokenGreaterThanOrEqual:
				case eTokenEqual:
				case eTokenNotEqual:
					/* actually do the thing */
					UngetToken(Scanner,Token);
					Error = ParseRelOper(&OperatorType,Scanner,LineNumberOut);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					Error = ParseExpr4(&RightHandSide,Scanner,SymbolTable,TrashTracker,
						LineNumberOut);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					/* build the operator node */
					WholeOperator = NewBinaryOperator(OperatorType,LeftHandSide,
						RightHandSide,TrashTracker,GetCurrentLineNumber(Scanner));
					if (WholeOperator == NIL)
						{
							*LineNumberOut = GetCurrentLineNumber(Scanner);
							return eCompileOutOfMemory;
						}
					ThisWholeNode = NewExprBinaryOperator(WholeOperator,TrashTracker,
						GetCurrentLineNumber(Scanner));
					if (ThisWholeNode == NIL)
						{
							*LineNumberOut = GetCurrentLineNumber(Scanner);
							return eCompileOutOfMemory;
						}
					return ParseExpr3Prime(ExpressionOut,ThisWholeNode,Scanner,SymbolTable,
						TrashTracker,LineNumberOut);
			}

		EXECUTE(PRERR(ForceAbort,"ParseExpr3Prime:  control reached end of function"));
	}




/*   30:   <conj_oper>        ::= and */
/*   31:                      ::= or */
/*   32:                      ::= xor */
/* FIRST SET: */
/*  <conj_oper>        : {and, or, xor} */
/* FOLLOW SET: */
/*  <conj_oper>        : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, */
/*       asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, */
/*       true, false, (, -, <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, */
/*       <expr7>, <expr8>} */
static CompileErrors	ParseConjOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOperatorOrStatement;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileExpectedOperatorOrStatement;

							case eKeywordAnd:
								*OperatorOut = eBinaryAnd;
								return eCompileNoError;

							case eKeywordOr:
								*OperatorOut = eBinaryOr;
								return eCompileNoError;

							case eKeywordXor:
								*OperatorOut = eBinaryXor;
								return eCompileNoError;
						}
					break;
			}
		EXECUTE(PRERR(ForceAbort,"ParseConjOper:  control reached end of function"));
	}




/*   47:   <expr5>            ::= <expr6> <expr5prime> */
/* FIRST SET: */
/*  <expr5>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, asin, */
/*       acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, true, */
/*       false, (, -, <expr6>, <unary_oper>, <expr7>, <expr8>} */
/* FOLLOW SET: */
/*  <expr5>            : {and, or, xor, then, else, elseif, while, until, do, */
/*       to, ), CLOSEBRACKET, ,, :=, ;, +, -, EQ, NEQ, LT, LTEQ, GR, GREQ, */
/*       <expr2prime>, <conj_oper>, <expr3prime>, <rel_oper>, <expr4prime>, */
/*       <add_oper>, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr5(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		CompileErrors				Error;
		ASTExpressionRec*		LeftHandSide;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr6(&LeftHandSide,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseExpr5Prime(ExpressionOut,LeftHandSide,Scanner,SymbolTable,
			TrashTracker,LineNumberOut);
	}




/*   43:   <expr4prime>       ::= <add_oper> <expr5> <expr4prime> */
/*   44:                      ::=  */
/* FIRST SET: */
/*  <expr4prime>       : {+, -, <add_oper>} */
/* FOLLOW SET: */
/*  <expr4prime>       : {and, or, xor, then, else, elseif, while, until, */
/*       do, to, ), CLOSEBRACKET, ,, :=, ;, EQ, NEQ, LT, LTEQ, GR, GREQ, */
/*       <expr2prime>, <conj_oper>, <expr3prime>, <rel_oper>, <actualtail>, */
/*       <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr4Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		BinaryOpType				OperatorType;
		CompileErrors				Error;
		ASTExpressionRec*		RightHandSide;
		ASTBinaryOpRec*			WholeOperator;
		ASTExpressionRec*		ThisWholeNode;

		CheckPtrExistence(LeftHandSide);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					UngetToken(Scanner,Token);
					*ExpressionOut = LeftHandSide;
					return eCompileNoError;

				case eTokenPlus:
				case eTokenMinus:
					UngetToken(Scanner,Token);
					Error = ParseAddOper(&OperatorType,Scanner,LineNumberOut);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					Error = ParseExpr5(&RightHandSide,Scanner,SymbolTable,TrashTracker,
						LineNumberOut);
					if (Error != eCompileNoError)
						{
							return Error;
						}
					/* create the node */
					WholeOperator = NewBinaryOperator(OperatorType,LeftHandSide,RightHandSide,
						TrashTracker,GetCurrentLineNumber(Scanner));
					if (WholeOperator == NIL)
						{
							*LineNumberOut = GetCurrentLineNumber(Scanner);
							return eCompileOutOfMemory;
						}
					ThisWholeNode = NewExprBinaryOperator(WholeOperator,TrashTracker,
						GetCurrentLineNumber(Scanner));
					if (ThisWholeNode == NIL)
						{
							*LineNumberOut = GetCurrentLineNumber(Scanner);
							return eCompileOutOfMemory;
						}
					return ParseExpr4Prime(ExpressionOut,ThisWholeNode,Scanner,SymbolTable,
						TrashTracker,LineNumberOut);
			}
		EXECUTE(PRERR(ForceAbort,"ParseConjOper:  control reached end of function"));
	}




/*   36:   <rel_oper>         ::= LT */
/*   37:                      ::= LTEQ */
/*   38:                      ::= GR */
/*   39:                      ::= GREQ */
/*   40:                      ::= EQ */
/*   41:                      ::= NEQ */
/* FIRST SET: */
/*  <rel_oper>         : {EQ, NEQ, LT, LTEQ, GR, GREQ} */
/* FOLLOW SET: */
/*  <rel_oper>         : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, */
/*       asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, */
/*       true, false, (, -, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, */
/*       <expr8>} */
static CompileErrors	ParseRelOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOperatorOrStatement;

				case eTokenEqual:
					*OperatorOut = eBinaryEqual;
					return eCompileNoError;

				case eTokenNotEqual:
					*OperatorOut = eBinaryNotEqual;
					return eCompileNoError;

				case eTokenLessThan:
					*OperatorOut = eBinaryLessThan;
					return eCompileNoError;

				case eTokenLessThanOrEqual:
					*OperatorOut = eBinaryLessThanOrEqual;
					return eCompileNoError;

				case eTokenGreaterThan:
					*OperatorOut = eBinaryGreaterThan;
					return eCompileNoError;

				case eTokenGreaterThanOrEqual:
					*OperatorOut = eBinaryGreaterThanOrEqual;
					return eCompileNoError;
			}
		EXECUTE(PRERR(ForceAbort,"ParseRelOper:  control reached end of function"));
	}




/*   56:   <expr6>            ::= <unary_oper> <expr6> */
/*   57:                      ::= <expr7> */
/* FIRST SET: */
/*  <expr6>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, asin, */
/*       acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, true, */
/*       false, (, -, <unary_oper>, <expr7>, <expr8>} */
/* FOLLOW SET: */
/*  <expr6>            : {and, or, xor, div, mod, SHR, SHL, then, else, elseif, */
/*       while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, /, +, -, EQ, NEQ, */
/*       LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, <expr3prime>, <rel_oper>, */
/*       <expr4prime>, <add_oper>, <expr5prime>, <mult_oper>, <actualtail>, */
/*       <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr6(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		UnaryOpType					UnaryOperatorThing;
		ASTExpressionRec*		UnaryArgument;
		CompileErrors				Error;
		ASTUnaryOpRec*			UnaryOpNode;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
				 OtherPoint:
					UngetToken(Scanner,Token);
					return ParseExpr7(ExpressionOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);

				case eTokenMinus:
					UnaryOperatorThing = eUnaryNegation;
					break;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								goto OtherPoint;

							case eKeywordNot:
								UnaryOperatorThing = eUnaryNot;
								break;

							case eKeywordSin:
								UnaryOperatorThing = eUnarySine;
								break;

							case eKeywordCos:
								UnaryOperatorThing = eUnaryCosine;
								break;

							case eKeywordTan:
								UnaryOperatorThing = eUnaryTangent;
								break;

							case eKeywordAsin:
								UnaryOperatorThing = eUnaryArcSine;
								break;

							case eKeywordAcos:
								UnaryOperatorThing = eUnaryArcCosine;
								break;

							case eKeywordAtan:
								UnaryOperatorThing = eUnaryArcTangent;
								break;

							case eKeywordLn:
								UnaryOperatorThing = eUnaryLogarithm;
								break;

							case eKeywordExp:
								UnaryOperatorThing = eUnaryExponentiation;
								break;

							case eKeywordBool:
								UnaryOperatorThing = eUnaryCastToBoolean;
								break;

							case eKeywordInt:
								UnaryOperatorThing = eUnaryCastToInteger;
								break;

							case eKeywordSingle:
								UnaryOperatorThing = eUnaryCastToSingle;
								break;

							case eKeywordDouble:
								UnaryOperatorThing = eUnaryCastToDouble;
								break;

							case eKeywordFixed:
								UnaryOperatorThing = eUnaryCastToFixed;
								break;

							case eKeywordSqr:
								UnaryOperatorThing = eUnarySquare;
								break;

							case eKeywordSqrt:
								UnaryOperatorThing = eUnarySquareRoot;
								break;

							case eKeywordAbs:
								UnaryOperatorThing = eUnaryAbsoluteValue;
								break;

							case eKeywordNeg:
								UnaryOperatorThing = eUnaryTestNegative;
								break;

							case eKeywordSign:
								UnaryOperatorThing = eUnaryGetSign;
								break;

							case eKeywordLength:
								UnaryOperatorThing = eUnaryGetArrayLength;
								break;
						}
					break;
			}

		/* build argument */
		Error = ParseExpr6(&UnaryArgument,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* build node */
		UnaryOpNode = NewUnaryOperator(UnaryOperatorThing,UnaryArgument,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (UnaryOpNode == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprUnaryOperator(UnaryOpNode,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   48:   <expr5prime>       ::= <mult_oper> <expr6> <expr5prime> */
/*   49:                      ::=  */
/* FIRST SET: */
/*  <expr5prime>       : {div, mod, SHR, SHL, *, /, <mult_oper>} */
/* FOLLOW SET: */
/*  <expr5prime>       : {and, or, xor, then, else, elseif, while, until, */
/*       do, to, ), CLOSEBRACKET, ,, :=, ;, +, -, EQ, NEQ, LT, LTEQ, GR, */
/*       GREQ, <expr2prime>, <conj_oper>, <expr3prime>, <rel_oper>, <expr4prime>, */
/*       <add_oper>, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr5Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* LeftHandSide, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTExpressionRec*		RightHandSide;
		BinaryOpType				OperatorThing;
		CompileErrors				Error;
		ASTBinaryOpRec*			BinaryOperator;
		ASTExpressionRec*		WholeThingThing;

		CheckPtrExistence(LeftHandSide);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
				 NullifyPoint:
					*ExpressionOut = LeftHandSide;
					UngetToken(Scanner,Token);
					return eCompileNoError;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								goto NullifyPoint;

							case eKeywordDiv:
							case eKeywordMod:
							 DoTheStuffPoint:
								UngetToken(Scanner,Token);
								Error = ParseMultOper(&OperatorThing,Scanner,LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								Error = ParseExpr6(&RightHandSide,Scanner,SymbolTable,TrashTracker,
									LineNumberOut);
								if (Error != eCompileNoError)
									{
										return Error;
									}
								/* create the node */
								BinaryOperator = NewBinaryOperator(OperatorThing,LeftHandSide,
									RightHandSide,TrashTracker,GetCurrentLineNumber(Scanner));
								if (BinaryOperator == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								WholeThingThing = NewExprBinaryOperator(BinaryOperator,TrashTracker,
									GetCurrentLineNumber(Scanner));
								if (WholeThingThing == NIL)
									{
										*LineNumberOut = GetCurrentLineNumber(Scanner);
										return eCompileOutOfMemory;
									}
								return ParseExpr5Prime(ExpressionOut,WholeThingThing,Scanner,
									SymbolTable,TrashTracker,LineNumberOut);
						}
					break;

				case eTokenShiftLeft:
				case eTokenShiftRight:
				case eTokenStar:
				case eTokenSlash:
					goto DoTheStuffPoint;
			}
		EXECUTE(PRERR(ForceAbort,"ParseExpr5Prime:  control reached end of function"));
	}




/*   45:   <add_oper>         ::= + */
/*   46:                      ::= - */
/*  <add_oper>         : {+, -} */
/*  <add_oper>         : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, */
/*       asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, */
/*       true, false, (, -, <expr5>, <expr6>, <unary_oper>, <expr7>, <expr8>} */
static CompileErrors	ParseAddOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOperatorOrStatement;

				case eTokenPlus:
					*OperatorOut = eBinaryPlus;
					return eCompileNoError;

				case eTokenMinus:
					*OperatorOut = eBinaryMinus;
					return eCompileNoError;
			}
		EXECUTE(PRERR(ForceAbort,"ParseAddOper:  control reached end of function"));
	}




/*   79:   <expr7>            ::= <expr8> <expr7prime> */
/* FIRST SET: */
/*  <expr7>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, true, false, (, <expr8>} */
/* FOLLOW SET: */
/*  <expr7>            : {and, or, xor, div, mod, SHR, SHL, then, else, elseif, */
/*       while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, /, +, -, EQ, NEQ, */
/*       LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, <expr3prime>, <rel_oper>, */
/*       <expr4prime>, <add_oper>, <expr5prime>, <mult_oper>, <actualtail>, */
/*       <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr7(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		ASTExpressionRec*		ResultOfExpr8;
		CompileErrors				Error;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr8(&ResultOfExpr8,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		return ParseExpr7Prime(ExpressionOut,ResultOfExpr8,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
	}




/*   50:   <mult_oper>        ::= * */
/*   51:                      ::= / */
/*   52:                      ::= div */
/*   53:                      ::= mod */
/*   54:                      ::= SHL */
/*   55:                      ::= SHR */
/*  <mult_oper>        : {div, mod, SHR, SHL, *, /} */
/*  <mult_oper>        : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, not, sin, cos, tan, */
/*       asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, length, */
/*       true, false, (, -, <expr6>, <unary_oper>, <expr7>, <expr8>} */
static CompileErrors	ParseMultOper(BinaryOpType* OperatorOut, ScannerRec* Scanner,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOperatorOrStatement;

				case eTokenStar:
					*OperatorOut = eBinaryMultiplication;
					return eCompileNoError;

				case eTokenSlash:
					*OperatorOut = eBinaryImpreciseDivision;
					return eCompileNoError;

				case eTokenShiftLeft:
					*OperatorOut = eBinaryShiftLeft;
					return eCompileNoError;

				case eTokenShiftRight:
					*OperatorOut = eBinaryShiftRight;
					return eCompileNoError;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileExpectedOperatorOrStatement;

							case eKeywordDiv:
								*OperatorOut = eBinaryIntegerDivision;
								return eCompileNoError;

							case eKeywordMod:
								*OperatorOut = eBinaryIntegerRemainder;
								return eCompileNoError;
						}
					break;
			}
		EXECUTE(PRERR(ForceAbort,"ParseMultOper:  control reached end of function"));
	}




/*   92:   <expr8>            ::= <identifier> */
/*   93:                      ::= <integer> */
/*   94:                      ::= <single> */
/*   95:                      ::= <double> */
/*   96:                      ::= <fixed> */
/*   97:                      ::= <string> */
/*   98:                      ::= true */
/*   99:                      ::= false */
/*  108:                      ::= ( <exprlist> ) */
/*  <expr8>            : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, true, false, (} */
/*  <expr8>            : {and, or, xor, div, mod, SHR, SHL, then, else, elseif, */
/*       while, until, do, to, (, ), OPENBRACKET, CLOSEBRACKET, ,, :=, ;, *, */
/*       /, +, -, ^, EQ, NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, */
/*       <expr3prime>, <rel_oper>, <expr4prime>, <add_oper>, <expr5prime>, */
/*       <mult_oper>, <expr7prime>, <arraysubscript>, <funccall>, <exponentiation>, */
/*       <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr8(ASTExpressionRec** ExpressionOut, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		ASTOperandRec*			TheOperand;
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileExpectedOperand;

					/*   92:   <expr8>            ::= <identifier> */
				case eTokenIdentifier:
					{
						SymbolRec*				TheSymbolTableEntry;

						TheSymbolTableEntry = GetSymbolFromTable(SymbolTable,
							GetTokenIdentifierString(Token),PtrSize(GetTokenIdentifierString(Token)));
						if (TheSymbolTableEntry == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileIdentifierNotDeclared;
							}
						TheOperand = NewSymbolReference(TrashTracker,TheSymbolTableEntry,
							GetCurrentLineNumber(Scanner));
					}
					break;

					/*   93:                      ::= <integer> */
				case eTokenInteger:
					TheOperand = NewIntegerLiteral(TrashTracker,GetTokenIntegerValue(Token),
						GetCurrentLineNumber(Scanner));
					break;

					/*   94:                      ::= <single> */
				case eTokenSingle:
					TheOperand = NewSingleLiteral(TrashTracker,GetTokenSingleValue(Token),
						GetCurrentLineNumber(Scanner));
					break;

					/*   95:                      ::= <double> */
				case eTokenDouble:
					TheOperand = NewDoubleLiteral(TrashTracker,GetTokenDoubleValue(Token),
						GetCurrentLineNumber(Scanner));
					break;

					/*   96:                      ::= <fixed> */
				case eTokenFixed:
					TheOperand = NewFixedLiteral(TrashTracker,GetTokenFixedValue(Token),
						GetCurrentLineNumber(Scanner));
					break;

					/*   97:                      ::= <string> */
				case eTokenString:
					/* this was here, but I took it out */
					*LineNumberOut = GetCurrentLineNumber(Scanner);
					return eCompileCantHaveStringLiteralThere;
#if 0
					TheOperand = NewStringLiteral(TrashTracker,GetTokenStringValue(Token),
						GetCurrentLineNumber(Scanner));
#endif

					/*  108:                      ::= ( <exprlist> ) */
				case eTokenOpenParen:
					{
						CompileErrors			Error;
						ASTExprListRec*		ListOfExpressions;

						/* open a new scope */
						if (!IncrementSymbolTableLevel(SymbolTable))
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}

						/* parse the expression sequence */
						Error = ParseExprList(&ListOfExpressions,Scanner,SymbolTable,TrashTracker,
							LineNumberOut);
						if (Error != eCompileNoError)
							{
								return Error;
							}

						/* dispose of the current scope */
						DecrementSymbolTableLevel(SymbolTable);

						/* build the thing */
						*ExpressionOut = NewExprSequence(ListOfExpressions,TrashTracker,
							GetCurrentLineNumber(Scanner));
						if (*ExpressionOut == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}

						/* clean up by getting rid of the close paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenCloseParen)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileExpectedCloseParen;
							}
					}
					return eCompileNoError;

				case eTokenKeyword:
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileExpectedOperand;

								/*   98:                      ::= true */
							case eKeywordTrue:
								TheOperand = NewBooleanLiteral(TrashTracker,True,
									GetCurrentLineNumber(Scanner));
								break;

								/*   99:                      ::= false */
							case eKeywordFalse:
								TheOperand = NewBooleanLiteral(TrashTracker,False,
									GetCurrentLineNumber(Scanner));
								break;

								/* this was added later. */
							case eKeywordPi:
								TheOperand = NewDoubleLiteral(TrashTracker,DGETPI(),
									GetCurrentLineNumber(Scanner));
								break;
						}
					break;
			}

		if (TheOperand == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprOperand(TheOperand,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   80:   <expr7prime>       ::= <arraysubscript> */
/*   81:                      ::= <funccall> */
/*   82:                      ::= <exponentiation> */
/*   83:                      ::=  */
/*  <expr7prime>       : {(, OPENBRACKET, ^, <arraysubscript>, <funccall>, */
/*       <exponentiation>} */
/*  <expr7prime>       : {and, or, xor, div, mod, SHR, SHL, then, else, */
/*       elseif, while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, /, */
/*       +, -, EQ, NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, */
/*       <expr3prime>, <rel_oper>, <expr4prime>, <add_oper>, <expr5prime>, */
/*       <mult_oper>, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExpr7Prime(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* TheExpr8Thing, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(TheExpr8Thing);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					UngetToken(Scanner,Token);
					*ExpressionOut = TheExpr8Thing;
					return eCompileNoError;

					/*   81:                      ::= <funccall> */
				case eTokenOpenParen:
					UngetToken(Scanner,Token);
					return ParseFuncCall(ExpressionOut,TheExpr8Thing,Scanner,SymbolTable,
						TrashTracker,LineNumberOut);

					/*   80:   <expr7prime>       ::= <arraysubscript> */
				case eTokenOpenBracket:
					UngetToken(Scanner,Token);
					return ParseArraySubscript(ExpressionOut,TheExpr8Thing,Scanner,SymbolTable,
						TrashTracker,LineNumberOut);

					/*   82:                      ::= <exponentiation> */
				case eTokenCircumflex:
					{
						ASTExpressionRec*			RightHandSide;
						ASTBinaryOpRec*				TheOperator;
						CompileErrors					Error;

						UngetToken(Scanner,Token);
						Error = ParseExponentiation(&RightHandSide,Scanner,SymbolTable,
							TrashTracker,LineNumberOut);
						if (Error != eCompileNoError)
							{
								return Error;
							}
						TheOperator = NewBinaryOperator(eBinaryExponentiation,TheExpr8Thing,
							RightHandSide,TrashTracker,GetCurrentLineNumber(Scanner));
						if (TheOperator == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
						*ExpressionOut = NewExprBinaryOperator(TheOperator,TrashTracker,
							GetCurrentLineNumber(Scanner));
						if (*ExpressionOut == NIL)
							{
								*LineNumberOut = GetCurrentLineNumber(Scanner);
								return eCompileOutOfMemory;
							}
						return eCompileNoError;
					}
			}
		EXECUTE(PRERR(ForceAbort,"ParseExpr7Prime:  control reached end of function"));
	}




/*   85:   <funccall>         ::= ( <actualstart> ) */
/* FIRST SET: */
/*  <funccall>         : {(} */
/* FOLLOW SET: */
/*  <funccall>         : {and, or, xor, div, mod, SHR, SHL, then, else, elseif, */
/*       while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, /, +, -, EQ, */
/*       NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, <expr3prime>, */
/*       <rel_oper>, <expr4prime>, <add_oper>, <expr5prime>, <mult_oper>, */
/*       <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseFuncCall(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* FunctionGenerator, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTExprListRec*			ListOfParameters;
		CompileErrors				Error;
		ASTFuncCallRec*			TheFunctionCall;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		/* swallow open parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedOpenParen;
			}

		/* parse the argument list */
		Error = ParseActualStart(&ListOfParameters,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* swallow close parenthesis */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedCommaOrCloseParen;
			}

		TheFunctionCall = NewFunctionCall(ListOfParameters,FunctionGenerator,
			TrashTracker,GetCurrentLineNumber(Scanner));
		if (TheFunctionCall == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		*ExpressionOut = NewExprFunctionCall(TheFunctionCall,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   84:   <arraysubscript>   ::= OPENBRACKET <exprlist> CLOSEBRACKET */
/* FIRST SET: */
/*  <arraysubscript>   : {OPENBRACKET} */
/* FOLLOW SET: */
/*  <arraysubscript>   : {and, or, xor, div, mod, SHR, SHL, then, else, */
/*       elseif, while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, */
/*       /, +, -, EQ, NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, */
/*       <expr3prime>, <rel_oper>, <expr4prime>, <add_oper>, <expr5prime>, */
/*       <mult_oper>, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseArraySubscript(ASTExpressionRec** ExpressionOut,
												ASTExpressionRec* ArrayGenerator, ScannerRec* Scanner,
												SymbolTableRec* SymbolTable, TrashTrackRec* TrashTracker,
												long* LineNumberOut)
	{
		TokenRec*						Token;
		ASTExpressionRec*		Subscript;
		CompileErrors				Error;
		ASTBinaryOpRec*			ArraySubsOperation;
		ASTExprListRec*			SubscriptRaw;

		CheckPtrExistence(ArrayGenerator);
		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenBracket)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedOpenBracket;
			}

		Error = ParseExprList(&SubscriptRaw,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}
		Subscript = NewExprSequence(SubscriptRaw,TrashTracker,GetCurrentLineNumber(Scanner));
		if (Subscript == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseBracket)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedCloseBracket;
			}

		ArraySubsOperation = NewBinaryOperator(eBinaryArraySubscripting,ArrayGenerator,
			Subscript,TrashTracker,GetCurrentLineNumber(Scanner));
		if (ArraySubsOperation == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		*ExpressionOut = NewExprBinaryOperator(ArraySubsOperation,TrashTracker,
			GetCurrentLineNumber(Scanner));
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*  124:   <exprlist>         ::= <exprlistelem> <exprlisttail> */
/* FIRST SET: */
/*  <exprlist>         : {<identifier>, <integer>, <single>, <double>, <fixed>, */
/*       <string>, bool, int, single, double, fixed, proto, var, not, sin, */
/*       cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, neg, sign, */
/*       length, if, while, until, do, resize, error, true, false, */
/*       set, (, -, <prototype>, <expr>, <expr2>, <expr3>, <expr4>, <expr5>, */
/*       <expr6>, <unary_oper>, <expr7>, <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <exprlist>         : {), CLOSEBRACKET, EOF} */
CompileErrors					ParseExprList(ASTExprListRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		CompileErrors				Error;
		ASTExpressionRec*		FirstExpression;
		TokenRec*						Token;
		ASTExprListRec*			RestOfList;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		if ((GetTokenType(Token) == eTokenCloseParen)
			|| (GetTokenType(Token) == eTokenCloseBracket)
			|| (GetTokenType(Token) == eTokenEndOfInput))
			{
				UngetToken(Scanner,Token);
				*ExpressionOut = NIL; /* empty list */
				return eCompileNoError;
			}

		/* see if we should parse a non-existent element (prototype) which */
		/* generates no code */
		if ((GetTokenType(Token) == eTokenKeyword)
			&& (GetTokenKeywordTag(Token) == eKeywordProto))
			{
				SymbolRec*					ProtoSymbolOut;
				CompileErrors				Error;

				UngetToken(Scanner,Token);
				Error = ParsePrototype(&ProtoSymbolOut,Scanner,SymbolTable,
					TrashTracker,LineNumberOut);
				if (Error != eCompileNoError)
					{
						return Error;
					}
				/* this is declarative and generates no code */
				/* as a hack to get this to work, we'll expect a semicolon and */
				/* another expression list */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompileOutOfMemory;
					}
				/* eat up the semicolon */
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompilePrototypeCantBeLastThingInExprList;
					}
				/* now just parse the rest of the expression list */
				return ParseExprList(ExpressionOut,Scanner,SymbolTable,TrashTracker,
					LineNumberOut);
			}

		/* get first part of list */
		UngetToken(Scanner,Token);
		Error = ParseExprListElem(&FirstExpression,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		/* get the rest of the list */
		Error = ParseExprListTail(&RestOfList,Scanner,SymbolTable,TrashTracker,
			LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		*ExpressionOut = ASTExprListCons(FirstExpression,RestOfList,TrashTracker);
		if (*ExpressionOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   91:   <exponentiation>   ::= ^ <expr7> */
/* FIRST SET: */
/*  <exponentiation>   : {^} */
/* FOLLOW SET: */
/*  <exponentiation>   : {and, or, xor, div, mod, SHR, SHL, then, else, */
/*       elseif, while, until, do, to, ), CLOSEBRACKET, ,, :=, ;, *, */
/*       /, +, -, EQ, NEQ, LT, LTEQ, GR, GREQ, <expr2prime>, <conj_oper>, */
/*       <expr3prime>, <rel_oper>, <expr4prime>, <add_oper>, <expr5prime>, */
/*       <mult_oper>, <actualtail>, <iftail>, <loopwhileuntil>, <exprlisttail>} */
static CompileErrors	ParseExponentiation(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCircumflex)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedRightAssociativeOperator;
			}

		return ParseExpr7(ExpressionOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
	}




/*   86:   <actualstart>      ::= <actuallist> */
/*   87:                      ::=  */
/* FIRST SET: */
/*  <actualstart>      : {<identifier>, <integer>, <single>, <double>, */
/*       <fixed>, <string>, bool, int, single, double, fixed, proto, var, */
/*       not, sin, cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, */
/*       neg, sign, length, if, while, until, do, resize, error, */
/*       true, false, set, (, -, <prototype>, <expr>, <expr2>, */
/*       <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, <expr8>, */
/*       <actuallist>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <actualstart>      : {)} */
static CompileErrors	ParseActualStart(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* handle the nullification */
		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				*ParamListOut = NIL; /* empty list is NIL */
				return eCompileNoError;
			}

		UngetToken(Scanner,Token);
		return ParseActualList(ParamListOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
	}




/*   88:   <actuallist>       ::= <expr> <actualtail> */
/* FIRST SET: */
/*  <actuallist>       : {<identifier>, <integer>, <single>, <double>, */
/*       <fixed>, <string>, bool, int, single, double, fixed, proto, var, */
/*       not, sin, cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, abs, */
/*       neg, sign, length, if, while, until, do, resize, error, */
/*       true, false, set, (, -, <prototype>, <expr>, <expr2>, */
/*       <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, <expr8>, */
/*       <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <actuallist>       : {)} */
static CompileErrors	ParseActualList(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		ASTExpressionRec*		FirstExpression;
		CompileErrors				Error;
		ASTExprListRec*			RestOfList;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Error = ParseExpr(&FirstExpression,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		Error = ParseActualTail(&RestOfList,Scanner,SymbolTable,TrashTracker,LineNumberOut);
		if (Error != eCompileNoError)
			{
				return Error;
			}

		*ParamListOut = ASTExprListCons(FirstExpression,RestOfList,TrashTracker);
		if (*ParamListOut == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		return eCompileNoError;
	}




/*   89:   <actualtail>       ::= , <actuallist> */
/*   90:                      ::=  */
/* FIRST SET: */
/*  <actualtail>       : {,} */
/* FOLLOW SET: */
/*  <actualtail>       : {)} */
static CompileErrors	ParseActualTail(ASTExprListRec** ParamListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenComma)
			&& (GetTokenType(Token) != eTokenCloseParen))
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedCommaOrCloseParen;
			}

		/* handle nullification */
		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				*ParamListOut = NIL; /* empty list */
				return eCompileNoError;
			}

		return ParseActualList(ParamListOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
	}




/*  125:   <exprlisttail>     ::= ; <exprlist> */
/*  126:                      ::=  */
/* FIRST SET: */
/*  <exprlisttail>     : {;} */
/* FOLLOW SET: */
/*  <exprlisttail>     : {), CLOSEBRACKET, EOF} */
static CompileErrors	ParseExprListTail(ASTExprListRec** ListOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		/* find out if we should continue or not */
		if ((GetTokenType(Token) == eTokenCloseParen)
			|| (GetTokenType(Token) == eTokenCloseBracket)
			|| (GetTokenType(Token) == eTokenEndOfInput))
			{
				UngetToken(Scanner,Token);
				*ListOut = NIL; /* empty list */
				return eCompileNoError;
			}

		if (GetTokenType(Token) != eTokenSemicolon)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileExpectedSemicolon;
			}

		return ParseExprList(ListOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
	}




/*  124:   <exprlistelem>     ::= <expr> */
/*  125:                      ::= var <identifier> : <type> <vartail> */
/*   22:   <exprlistelem>     ::= <prototype> */
/* FIRST SET: */
/*  <exprlistelem>     : {<identifier>, <integer>, <single>, <double>, */
/*       <fixed>, <string>, bool, int, single, double, fixed, proto, */
/*       var, not, sin, cos, tan, asin, acos, atan, ln, exp, sqr, sqrt, */
/*       abs, neg, sign, length, if, while, until, do, resize, error, */
/*       true, false, set, (, -, <prototype>, <expr>, */
/*       <expr2>, <expr3>, <expr4>, <expr5>, <expr6>, <unary_oper>, <expr7>, */
/*       <expr8>, <whileloop>, <untilloop>} */
/* FOLLOW SET: */
/*  <exprlistelem>     : {), CLOSEBRACKET, ;, <exprlisttail>} */
static CompileErrors	ParseExprListElem(ASTExpressionRec** ExpressionOut,
												ScannerRec* Scanner, SymbolTableRec* SymbolTable,
												TrashTrackRec* TrashTracker, long* LineNumberOut)
	{
		TokenRec*						Token;

		CheckPtrExistence(Scanner);
		CheckPtrExistence(SymbolTable);
		CheckPtrExistence(TrashTracker);

		/* lookahead to see what to do */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*LineNumberOut = GetCurrentLineNumber(Scanner);
				return eCompileOutOfMemory;
			}

		if ((GetTokenType(Token) == eTokenKeyword)
			&& (GetTokenKeywordTag(Token) == eKeywordVar))
			{
				TokenRec*						VariableName;
				DataTypes						ReturnType;
				CompileErrors				Error;
				long								LineNumberForFirstToken;

				/* get the identifier */
				VariableName = GetNextToken(Scanner);
				if (VariableName == NIL)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompileOutOfMemory;
					}
				if (GetTokenType(VariableName) != eTokenIdentifier)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompileExpectedIdentifier;
					}
				LineNumberForFirstToken = GetCurrentLineNumber(Scanner);

				/* get the colon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompileOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenColon)
					{
						*LineNumberOut = GetCurrentLineNumber(Scanner);
						return eCompileExpectedColon;
					}

				/* get the variable's type */
				Error = ParseType(&ReturnType,Scanner,LineNumberOut);
				if (Error != eCompileNoError)
					{
						return Error;
					}

				/* finish up the declaration of the variable */
				return ParseVarTail(ExpressionOut,VariableName,LineNumberForFirstToken,
					ReturnType,Scanner,SymbolTable,TrashTracker,LineNumberOut);
			}
		else
			{
				/* do the other thing */
				UngetToken(Scanner,Token);
				return ParseExpr(ExpressionOut,Scanner,SymbolTable,TrashTracker,LineNumberOut);
			}
	}
