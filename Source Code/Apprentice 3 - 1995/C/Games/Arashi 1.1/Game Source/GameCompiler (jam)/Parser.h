/*/
     Project Arashi: Parser.h
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, March 2, 1993, 19:18
     Created: Saturday, February 13, 1993, 13:20

     Copyright � 1993, Juri Munkki
/*/

#pragma once

enum	{	kLexPlus, kLexMinus, kLexAbs,
			kLexMultiply, kLexDivide, kLexModulo, kLexPower,
			kLexLt, kLexGt, kLexEq,
			kLexMin, kLexMax,
			kLexOpenParen, kLexCloseParen,
			kLexRowOffset, kLexConstant, 
			kUnaryMinus, kLexVariable,
			kLexFun0, kLexFun1,
			kLexLevel,
			kAssignment,
			kParseError, kLexEof	};

enum	{	kMinKey, kMaxKey,
			kRandomKey, kSinKey, kCosKey,
			kIntKey, kRoundKey,
			kLevelKey, firstVariable };

typedef struct
{
	short	kind;			//	Determines what this lexical symbol is.

	union
	{	double	floating;		//	Used for numeric constants.
		long	offset;			//	Used as offset into datacube.
		long	token;			//	Token for variable
		long	level;			//	Level number from level statement
	}	value;
}	LexSymbol;

typedef	struct
{
	StringPtr	input;
	Handle		output;
	long		realSize;
	long		logicalSize;

	LexSymbol	lookahead;
	
	short		stackDepth;
	short		stackMax;
}	ParserVariables;

void		ParseCompare();
void		ParseExpr();
void		LexRead(LexSymbol *theSymbol);
void		EmitInstruction(LexSymbol *theOperation,short stackChange);
void		LoadLevel(short whichLevel);
double		EvalVariable(long token);
void		WriteVariable(long token, double value);
