/*/
     Project Arashi: Parser.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, March 14, 1993, 21:21
     Created: Thursday, February 11, 1993, 7:18

     Copyright � 1993, Juri Munkki
/*/

#include "Numbers.h"
#include "Parser.h"
#include "RAMFiles.h"
#include "CStringDictionary.h"
#include "CTagBase.h"
#include <math.h>
#include "InternalVars.h"


#ifdef	DEBUGPARSER
#include "stdio.h"
#define	DEBUGPAR(a)	{	a	}
#else
#define	DEBUGPAR(a)	{		}
#endif

ParserVariables	parserVar;
long			lastKeyword;
long			lastVariable;

typedef	struct
{
	double	value;
	short	calcLevel;
} variableValue;

Handle				theScript;
double				*stackP;
CStringDictionary	*symTable;
CTagBase			*variableBase;
CTagBase			*programBase;
short				currentLevel = 0;
short				fromLevel = 32000;
short				upToLevel = -1;

void	InitSymbols()
{
	symTable = new CStringDictionary;
	symTable->IStringDictionary();
	
	symTable->AddDictEntry("\pmin",-1);
	symTable->AddDictEntry("\pmax",-1);
	symTable->AddDictEntry("\prandom",-1);
	symTable->AddDictEntry("\psin",-1);
	symTable->AddDictEntry("\pcos",-1);
	symTable->AddDictEntry("\pint",-1);
	symTable->AddDictEntry("\pround",-1);
	lastKeyword = symTable->AddDictEntry("\plevel",-1);
	lastVariable = lastKeyword;
	
	variableBase = new CTagBase;
	variableBase->ITagBase();
	
	programBase = new CTagBase;
	programBase->ITagBase();
}

void	ResetVariables()
{
	variableBase->Dispose();
	programBase->Dispose();
	InitSymbols();
}
void	CreateVariable(
	long	token)
{
	variableValue	zeroValue;
	
	zeroValue.value = 0;
	zeroValue.calcLevel = -1;
	variableBase->WriteEntry(token, sizeof(variableValue), &zeroValue);
}

void	VariableToKeyword(
	LexSymbol	*theSymbol)
{
	switch(theSymbol->value.token)
	{
		case kMinKey:
			theSymbol->kind = kLexMin;
			break;
		case kMaxKey:
			theSymbol->kind = kLexMax;
			break;
		case kRandomKey:
			theSymbol->kind = kLexFun0;
			break;
		case kSinKey:
		case kCosKey:
		case kIntKey:
		case kRoundKey:
			theSymbol->kind = kLexFun1;
			break;
		case kLevelKey:
			theSymbol->kind = kLexLevel;
			break;
	}
}

short	MatchVariable(
	StringPtr	theString)
{
	unsigned	char theChar;
	short	matchCount = 0;
	short	state = TRUE;
	
	theChar = theString[0];
	
	if(	(theChar >= 'a' && theChar <= 'z') ||
		(theChar >= 'A' && theChar <= 'Z'))
	{	
		matchCount++;
		
		do
		{
			theChar = theString[matchCount];
			
			if(	(theChar >= 'a' && theChar <= '}') || theChar == '_' ||
				(theChar >= 'A' && theChar <= ']') || theChar == '.' ||
				(theChar >= '0' && theChar <= '9'))
			{
				matchCount++;
			}
			else
			{
				state = FALSE;
			}
		} while(state);
	}
	
	return matchCount;
}
/*
**	Match the exponent part of a floating point number.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to the exponent
**	of a floating point number.
*/
short	MatchExponent(
	StringPtr	theString)
{
	enum	{	initialState,
				integerState,
				endState	};

	short	matchCount = 0;
	char	theChar;
	short	state = initialState;

	while(state != endState)
	{	theChar = theString[matchCount];
		
		if(theChar == 0)
			state = endState;	//	String ends with null

		switch(state)
		{	case initialState:
				if(	theChar == '+'	||	theChar == '-'	||
					(theChar >= '0' && theChar <= '9'))
				{		matchCount++;
						state = integerState;
				}
				else	state = endState;
				break;
			case integerState:
				if(theChar >= '0' && theChar <= '9')
				{	matchCount++;
				}
				else	state = endState;
				break;				
		}
	}

	return matchCount;
}

/*
**	Match the number part of a row index.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to the row
**	index of a row specifier. Row specifiers are of the
**	format @n, where n is an integer. (@1, @2, @3, ...)
*/
short	MatchRowIndex(
	StringPtr	theString)
{
	short	matchCount = 0;
	char	theChar;

	theChar = *theString++;

	while(theChar >= '0' && theChar <= '9')
	{	matchCount++;
		theChar = *theString++;
	}

	return matchCount;
}

/*
**	Match a floating point number.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to a floating
**	point number.
*/
short	MatchFloat(
	StringPtr	theString)
{
	enum	{	initialState,
				integerPartState,
				decimalPartState,
				exponentMatchState,
				endState	};

	short	matchCount = 0;
	char	theChar;
	short	state = initialState;

	while(state != endState)
	{	theChar = theString[matchCount];
		
		if(theChar == 0)
			state = endState;	//	String ends with null

		switch(state)
		{	case initialState:
				if(	theChar == '+'	||	theChar == '-'	||
					(theChar >= '0' && theChar <= '9'))
				{		matchCount++;
						state = integerPartState; 
				}
#ifdef ALLOWSHORTHAND		//	Are numbers like . and .0 and .e1 and .0e1 valid?
				else if(theChar == '.')
				{	matchCount++;
					state = decimalPartState;
				}
#endif
				else	state = endState;
				break;

			case integerPartState:
				if(theChar >= '0' && theChar <= '9')
				{	matchCount++;
				}
				else if(theChar == 'e' || theChar == 'E')
				{	state = exponentMatchState;
				}
				else if(theChar == '.')
				{	matchCount++;
					state = decimalPartState;
				}
				else	state = endState;
				break;

			case decimalPartState:
				if(theChar >= '0' && theChar <= '9')
				{	matchCount++;
				}
				else if(theChar == 'e' || theChar == 'E')
				{	state = exponentMatchState;
				}
				else	state = endState;
				break;				

			case exponentMatchState:
				{
					short	expLength = MatchExponent(theString+matchCount+1);
				
					if(expLength > 0)
					{	matchCount += expLength+1;
					}
					state = endState;
				}
				break;
		}
	}
	
	return matchCount;
}


void	EmitInstruction(
	LexSymbol	*theOperation,
	short		stackChange)
{
	long	writeOffset = parserVar.logicalSize;

	parserVar.stackDepth += stackChange;
	if(parserVar.stackDepth > parserVar.stackMax)
		parserVar.stackMax = parserVar.stackDepth;

	if(	IncreaseByClump(	parserVar.output,
							&parserVar.realSize,
							&parserVar.logicalSize,
							sizeof(LexSymbol),
							sizeof(LexSymbol)*10) == noErr)
	{	*(LexSymbol *)(*parserVar.output+writeOffset) = *theOperation;
	}
}

void	SkipComment()
{
	parserVar.input+= 2;
	if(parserVar.input[0] != 0)
	{	while(	(parserVar.input[0] != 0) &&
				!(parserVar.input[0] == '*' && parserVar.input[1] == '/'))
		{	parserVar.input++;
		}
		
		if(parserVar.input[0] != 0)
		{	parserVar.input += 2;
		}
	}
}
void	SkipOneLineComment()
{
	parserVar.input+= 2;
	if(parserVar.input[0] != 0)
	{	while(	(parserVar.input[0] != 0) &&
				!(parserVar.input[0] == 13))
		{	parserVar.input++;
		}
		
		if(parserVar.input[0] != 0)
		{	parserVar.input += 1;
		}
	}
}

void	LexRead(
	LexSymbol	*theSymbol)
{
	unsigned char	theChar;
	short			matchCount;

	//	First, skip any whitespace like returns, tabs, spaces and control characters.
	
	theChar = *parserVar.input;
	
	while(theChar <= 32 && theChar > 0)
	{	theChar = *(++parserVar.input);
	}
	
	switch(theChar)
	{
		case '+':
			theSymbol->kind = kLexPlus;
			parserVar.input++;
			break;
		case '-':
			theSymbol->kind = kLexMinus;
			parserVar.input++;
			break;
		case '|':
			theSymbol->kind = kLexAbs;
			parserVar.input++;
			break;
		case '*':
			theSymbol->kind = kLexMultiply;
			parserVar.input++;
			break;
		case '/':
			if(parserVar.input[1] == '*')
			{	SkipComment();
				LexRead(theSymbol);
			}
			else if(parserVar.input[1] == '/')
			{	SkipOneLineComment();
				LexRead(theSymbol);
			}
			else
			{	theSymbol->kind = kLexDivide;
				parserVar.input++;
			}
			break;
		case '%':
			theSymbol->kind = kLexModulo;
			parserVar.input++;
			break;
		case '^':
			theSymbol->kind = kLexPower;
			parserVar.input++;
			break;
		case '>':
			theSymbol->kind = kLexGt;
			parserVar.input++;
			break;
		case '<':
			theSymbol->kind = kLexLt;
			parserVar.input++;
			break;
		case '=':
			theSymbol->kind = kLexEq;
			parserVar.input++;
			break;
#ifdef TESTMINMAX
		case ']':
			theSymbol->kind = kLexMin;
			parserVar.input++;
			break;
		case '[':
			theSymbol->kind = kLexMax;
			parserVar.input++;
			break;
#endif
		case '(':
			theSymbol->kind = kLexOpenParen;
			parserVar.input++;
			break;
		case ')':
			theSymbol->kind = kLexCloseParen;
			parserVar.input++;
			break;
		case 0:
			theSymbol->kind = kLexEof;
			break;
		default:
			matchCount = MatchVariable(parserVar.input);
			if(matchCount > 0)
			{	theSymbol->value.token = symTable->FindEntry(parserVar.input - 1, matchCount);
				if(theSymbol->value.token > lastVariable)
				{	lastVariable = theSymbol->value.token;
					CreateVariable(lastVariable);
				}
				theSymbol->kind = kLexVariable;
				if(theSymbol->value.token <= lastKeyword)
				{	VariableToKeyword(theSymbol);
				}
				parserVar.input += matchCount;
			}
			else
			{	matchCount = MatchFloat(parserVar.input);
				if(matchCount > 0)
				{	char	temp;
				
					temp = parserVar.input[-1];
					parserVar.input[-1] = matchCount;
					theSymbol->kind = kLexConstant;
					theSymbol->value.floating = StringToLongDouble(parserVar.input-1);
					parserVar.input[-1] = temp;
					parserVar.input += matchCount;
				}
			}
			
			if(matchCount == 0)
			{	theSymbol->kind = kParseError;
			}
			break;
	}
}

void	LexMatch(
	short kind)
{
	if(parserVar.lookahead.kind == kind)
		LexRead(&parserVar.lookahead);
	else
		parserVar.lookahead.kind = kParseError;
}


void	ParseFactor()
{
	LexSymbol	savedSymbol;
	
	switch(parserVar.lookahead.kind)
	{	
		case kLexFun0:
			EmitInstruction(&parserVar.lookahead,1);
			DEBUGPAR(printf("function0 ");)
			LexMatch(parserVar.lookahead.kind);
			break;
			
		case kLexFun1:
			savedSymbol = parserVar.lookahead;
			LexMatch(parserVar.lookahead.kind);
			ParseFactor();
			EmitInstruction(&savedSymbol, 0);
			DEBUGPAR(printf("function1 ");)
			break;

		case kLexConstant:
			EmitInstruction(&parserVar.lookahead,1);
			DEBUGPAR(printf("%3.2f ", parserVar.lookahead.value.floating);)
			LexMatch(parserVar.lookahead.kind);
			break;
			
		case kLexVariable:
			EmitInstruction(&parserVar.lookahead,1);
			DEBUGPAR(printf("Variable #%ld ", parserVar.lookahead.value.token);)
			LexMatch(parserVar.lookahead.kind);
			break;
			
		case kLexRowOffset:
			EmitInstruction(&parserVar.lookahead,1);
			DEBUGPAR(printf("@(%ld) ", parserVar.lookahead.value.offset);)
			LexMatch(parserVar.lookahead.kind);
			break;
		
		case kLexMinus:
			LexMatch(parserVar.lookahead.kind);
			ParseFactor();
			
			savedSymbol.kind = kUnaryMinus;
			EmitInstruction(&savedSymbol,0);
			DEBUGPAR(printf("neg ");)
			break;

		case kLexAbs:
			LexMatch(parserVar.lookahead.kind);
			ParseFactor();

			savedSymbol.kind = kLexAbs;
			EmitInstruction(&savedSymbol,0);
			DEBUGPAR(printf("abs ");)
			break;

		case kLexPlus:	//	This is a no-op.
			LexMatch(parserVar.lookahead.kind);
			ParseFactor();
			break;
		
		case kLexOpenParen:
			LexMatch(parserVar.lookahead.kind);
			ParseCompare();
			if(parserVar.lookahead.kind == kLexCloseParen)
			{	LexMatch(parserVar.lookahead.kind);
			}
			else
			{	parserVar.lookahead.kind = kParseError;
			}
			break;

		default:
			parserVar.lookahead.kind = kParseError;
			break;
	}
}
void	ParsePower()
{
	LexSymbol	mySymbol;
	
	ParseFactor();
	
	while(1)
	{	if(	parserVar.lookahead.kind == kLexPower)
		{	mySymbol.kind = parserVar.lookahead.kind;

			LexMatch(parserVar.lookahead.kind);
			ParseFactor();
		
			EmitInstruction(&mySymbol,-1);

			DEBUGPAR(printf("^ ");)
		}
		else
		{	if(parserVar.lookahead.kind != kLexEof)
				parserVar.lookahead.kind == kParseError;
				
			break;
		}
	}
}
void	ParseTerm()
{
	LexSymbol	mySymbol;
	
	ParsePower();
	
	while(1)
	{	if(	parserVar.lookahead.kind == kLexMultiply ||
			parserVar.lookahead.kind == kLexDivide ||
			parserVar.lookahead.kind == kLexModulo)
		{	mySymbol.kind = parserVar.lookahead.kind;

			LexMatch(parserVar.lookahead.kind);
			ParsePower();
		
			EmitInstruction(&mySymbol,-1);

#ifdef DEBUGPARSER
			if(mySymbol.kind == kLexMultiply)	printf("* ");
			else
			if(mySymbol.kind == kLexModulo) 	printf("% ");
			else								printf("/ ");
#endif
		}
		else
		{	if(parserVar.lookahead.kind != kLexEof)
				parserVar.lookahead.kind == kParseError;
				
			break;
		}
	}
}

void	ParseExpr()
{
	LexSymbol	mySymbol;
	
	ParseTerm();
	
	while(1)
	{	if(	parserVar.lookahead.kind == kLexPlus ||
			parserVar.lookahead.kind == kLexMinus)
		{	mySymbol.kind = parserVar.lookahead.kind;

			LexMatch(parserVar.lookahead.kind);
			ParseTerm();
			EmitInstruction(&mySymbol,-1);

#ifdef DEBUGPARSER
			if(mySymbol.kind == kLexPlus)	printf("+ ");
			else							printf("- ");
#endif
		}
		else
		{	if(parserVar.lookahead.kind != kLexEof)
				parserVar.lookahead.kind == kParseError;
			
			break;
		}
	}
}
void	ParseMinMax()
{
	LexSymbol	mySymbol;
	
	ParseExpr();
	
	while(1)
	{	if(	parserVar.lookahead.kind == kLexMin ||
			parserVar.lookahead.kind == kLexMax)
		{	mySymbol.kind = parserVar.lookahead.kind;

			LexMatch(parserVar.lookahead.kind);
			ParseExpr();
			EmitInstruction(&mySymbol,-1);

#ifdef DEBUGPARSER
			if(mySymbol.kind == kLexMin)	printf("min ");
			else							printf("max ");
#endif
		}
		else
		{	if(parserVar.lookahead.kind != kLexEof)
				parserVar.lookahead.kind == kParseError;
			
			break;
		}
	}
}


void	ParseCompare()
{
	LexSymbol	mySymbol;
	
	ParseMinMax();
	
	while(1)
	{	if(	parserVar.lookahead.kind == kLexGt ||
			parserVar.lookahead.kind == kLexEq ||
			parserVar.lookahead.kind == kLexLt)
		{	mySymbol.kind = parserVar.lookahead.kind;

			LexMatch(parserVar.lookahead.kind);
			ParseMinMax();
			EmitInstruction(&mySymbol,-1);

#ifdef DEBUGOUTPUT
			if(mySymbol.kind == kLexGt	)		printf("> ");
			else if(mySymbol.kind == kLexLt)	printf("< ");
			else								printf("= ");
#endif
		}
		else
		{	if(parserVar.lookahead.kind != kLexEof)
				parserVar.lookahead.kind == kParseError;
			
			break;
		}
	}
}

void	ParseStatement(
	LexSymbol	*statement)
{
	if(parserVar.lookahead.kind == kLexVariable)
	{	*statement = parserVar.lookahead;
		
		LexMatch(parserVar.lookahead.kind);
		if(parserVar.lookahead.kind == kLexEq)
		{
			LexMatch(parserVar.lookahead.kind);

			if(statement->value.token > lastVariable)
			{	lastVariable = statement->value.token;
				CreateVariable(lastVariable);
			}
			
			ParseCompare();
			statement->kind = kAssignment;
			EmitInstruction(statement, -1);
			DEBUGPAR(printf("Store to var #%ld ", statement->value.token);)
		}
		else
		{	parserVar.lookahead.kind = kParseError;
		}
	}
	else
	if(parserVar.lookahead.kind == kLexLevel)
	{	*statement = parserVar.lookahead;
		
		LexMatch(parserVar.lookahead.kind);
		if(parserVar.lookahead.kind == kLexConstant)
		{	statement->value.level = parserVar.lookahead.value.floating;
			DEBUGPAR(printf("Start of level info for level #%3.0lf ", parserVar.lookahead.value.floating);)
			LexMatch(parserVar.lookahead.kind);
		}
		else
		{	parserVar.lookahead.kind = kParseError;
		}
	}
	else	parserVar.lookahead.kind = kParseError;
}

void	SetupCompiler(
	StringPtr theInput)
{
	parserVar.input = theInput;
	parserVar.output = NewHandle(0);
	parserVar.realSize = 0;
	parserVar.logicalSize = 0;
	parserVar.stackDepth = 0;
	parserVar.stackMax = 0;

}

void	WriteVariable(
	long	token,
	double	value)
{
	variableValue	*theVar;
	double			temp;
	
	theVar = variableBase->GetEntryPointer(token);
	if(theVar)
	{	theVar->calcLevel = currentLevel;
		theVar->value = value;
	}

}
double	EvalVariable(
	long	token)
{
	variableValue	*theVar;
	double			temp;
	
	theVar = variableBase->GetEntryPointer(token);
	if(theVar->calcLevel != currentLevel)
	{	LexSymbol	*theProgram;
	
		theVar->calcLevel = currentLevel;
		theProgram = programBase->GetEntryPointer(token);
		if(theProgram)
		{	while(theProgram->kind != kAssignment)
			{	switch(theProgram->kind)
				{	case kLexConstant:
						*(++stackP) = theProgram->value.floating;
						break;
					case kLexVariable:
						temp = EvalVariable(theProgram->value.token);
						*(++stackP) = temp;
						break;
					case kLexPlus:		stackP[-1] += stackP[0];	stackP--;	break;
					case kLexMinus:		stackP[-1] -= stackP[0];	stackP--;	break;
					case kLexMultiply:	stackP[-1] *= stackP[0];	stackP--;	break;
					case kLexDivide:	stackP[-1] /= stackP[0];	stackP--;	break;

					case kLexGt:		stackP[-1] = stackP[-1] > stackP[0];	stackP--;	break;
					case kLexLt:		stackP[-1] = stackP[-1] < stackP[0];	stackP--;	break;
					case kLexEq:		stackP[-1] = stackP[-1] == stackP[0];	stackP--;	break;
					
					case kLexPower:		stackP[-1] = pow(stackP[-1], stackP[0]);stackP--;	break;
					
					case kLexMin:
							if(stackP[-1] < stackP[0])	stackP[-1] = stackP[0];
							stackP--;
							break;
					case kLexMax:
							if(stackP[-1] > stackP[0])	stackP[-1] = stackP[0];
							stackP--;
							break;

					case kLexModulo:
						stackP[-1] = fmod(stackP[-1], stackP[0]);
						stackP--;
						break;
					case kUnaryMinus:	stackP[0] = -stackP[0];		break;
					case kLexAbs:
						if(stackP[0] < 0)
							stackP[0] = -stackP[0];
						break;
					case kLexFun0:
						*(++stackP) = ((unsigned int)Random()) /65536.0;
						break;
					case kLexFun1:
						switch(theProgram->value.token)
						{	
							case kSinKey:	*stackP = sin(*stackP);				break;
							case kCosKey:	*stackP = cos(*stackP);				break;
							case kRoundKey:	*stackP = floor(*stackP+0.5);		break;
							case kIntKey:	*stackP = floor(*stackP);			break;
						}
						break;

				}
				theProgram++;
			}
			theVar->value = *(stackP--);
		}
	}
	
	return	theVar->value;
}

void	LoadProgram()
{
	OSErr	err;
	short	ref;
	long	preSize, postSize, wholeSize;
	
	theScript = GetResource('TEXT',2128);
	DetachResource(theScript);
	
	preSize = GetHandleSize(theScript);
	
	err = FSOpen("\pArashi Script", 0, &ref);

	if(err != noErr)
	{	Handle	postScript;
	
		postScript = GetResource('TEXT',2129);
		postSize = GetHandleSize(postScript);
		wholeSize = postSize + preSize;
		HandAndHand(postScript, theScript);
		ReleaseResource(postScript);
	}
	else
	{	
	
		GetEOF(ref, &postSize);
		wholeSize = postSize + GetHandleSize(theScript);
		SetHandleSize(theScript,wholeSize);
		HLock(theScript);
		if(GetHandleSize(theScript) == wholeSize)
		{	FSRead(ref, &postSize, *theScript+wholeSize-postSize);
		}
		FSClose(ref);
	}
	HUnlock(theScript);
	SetHandleSize(theScript, wholeSize+1);
	HLock(theScript);
	(*theScript)[wholeSize] = 0;	//	Append a NULL to terminate the script.
}

void	LoadLevel(
	short	whichLevel)
{
	LexSymbol	statement;

	currentLevel = whichLevel;
	
	if(fromLevel > currentLevel)
	{	ResetVariables();
		SetupCompiler((StringPtr)*theScript);
		LexRead(&parserVar.lookahead);
		fromLevel = -1;
		upToLevel = -1;
	}
	
	while(whichLevel >= upToLevel)
	{	ParseStatement(&statement);
		SetHandleSize(parserVar.output, parserVar.logicalSize);

		if(parserVar.lookahead.kind == kParseError)
		{
			DEBUGPAR(printf(" ** Parse Error **\n");)
			upToLevel = 32000;
		}
		else
		{	DEBUGPAR(printf("\n");)
			if(statement.kind == kAssignment)
			{	programBase->WriteHandle(statement.value.token, parserVar.output);
			}
			else
			if(statement.kind == kLexLevel)
			{	fromLevel = upToLevel;
				upToLevel = statement.value.level;
			}
		}
		DisposHandle(parserVar.output);
		SetupCompiler(parserVar.input);
		
		if(parserVar.lookahead.kind == kLexEof)
		{	upToLevel = 32000;
		}
	}
	
	WriteVariable(	kVarLevelNumber, (double) currentLevel);
}
void	RunThis(
	StringPtr	script)
{
	LexSymbol	statement;

	parserVar.input = script;
	
	SetupCompiler(parserVar.input);
	LexRead(&parserVar.lookahead);

	do
	{	ParseStatement(&statement);
		SetHandleSize(parserVar.output, parserVar.logicalSize);

		if(parserVar.lookahead.kind == kParseError)
		{	DEBUGPAR(printf(" ** Parse Error **\n");)
		}
		else
		{	if(statement.kind == kAssignment)
			{	programBase->WriteHandle(statement.value.token, parserVar.output);
				programBase->Lock();
				currentLevel++;
				DEBUGPAR(printf(" ==> %lf\n", EvalVariable(statement.value.token));)
				programBase->Unlock();
			}
			else
			if(statement.kind == kLexLevel)
			{	fromLevel = upToLevel;
				upToLevel = statement.value.level;
				DEBUGPAR(printf("\n");)
			}
		}
		DisposHandle(parserVar.output);
		SetupCompiler(parserVar.input);
		
	} while(!(	parserVar.lookahead.kind == kLexEof ||
				parserVar.lookahead.kind == kParseError));

}

void	AllocParser()
{
	InitSymbols();
	stackP = (double *)NewPtr(sizeof(double) * 256);

	LoadProgram();

}

#ifdef DEBUGPARSER
void	main()
{
	unsigned	char	dummystring[128];
	unsigned	char	*ditto;
	int					numChars;
	short				i;
	short				j;

	printf("Enter expressions and see if I can parse them.\n");

	InitSymbols();
	stackP = (double *)NewPtr(sizeof(double) * 256);
	
	LoadProgram();
	
	for(i=1;i<15;i++)
	{	LoadLevel(i);
		programBase->Lock();
		
		printf("Level %4d   Flippercount: %d\n", currentLevel, (int)EvalVariable(kVarFlipperCount));
		programBase->Unlock();
	}


	do
	{	ditto=(void *)fgets((char *)dummystring,127,stdin);
		RunThis(dummystring);

	} while(ditto);
}
#endif