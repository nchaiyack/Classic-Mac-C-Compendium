/* CompilerScanner.c */
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

#include "CompilerScanner.h"
#include "TrashTracker.h"
#include "Memory.h"
#include "DataMunging.h"
#include "StringThing.h"
#include "Numbers.h"


#define MAXNUMKEYWORDS (128)


#define OPAREN '('
#define CPAREN ')'
#define OBRACKET '['
#define CBRACKET ']'


struct TokenRec
	{
		TokenTypes				Type;
		union
			{
				long							IntegerValue;
				double						DoubleValue;
				float							SingleValue;
				MyBoolean					BooleanValue;
				char*							StringValue;
				largefixedsigned	FixedValue;
			} u;
	};


typedef struct
	{
		char*							KeywordName;
		long							KeywordLength;
		long							TagValue;
	} KeywordRec;


struct ScannerRec
	{
		char*							Block;
		long							BlockLength;
		long							Index;
		long							LineNumber;
		long							NumKeywords;
		TrashTrackRec*		Allocator;
		TokenRec*					PushedBackToken;
		KeywordRec				KeywordList[MAXNUMKEYWORDS];
	};


/* this type is used when parsing floating point numbers to remember what */
/* part we're on */
typedef enum
	{
		eIntegerPart EXECUTE(= -6751),
		eFractionalPart,
		eExponentialPart,
		eExponNumberPart,
		eNumberFinished
	} NumStateType;


/* this type is used for explicitly specifying the type of a number */
typedef enum
	{
		eTypeNotSpecified EXECUTE(= -5612),
		eTypeSingle,
		eTypeDouble,
		eTypeFixed,
		eTypeInteger
	} NumFormType;


/* create a scanner information record */
ScannerRec*					NewScanner(struct TrashTrackRec* TrashTracker, char* RawData)
	{
		ScannerRec*				Scanner;

		CheckPtrExistence(RawData);

		Scanner = (ScannerRec*)AllocTrackedBlock(sizeof(ScannerRec),TrashTracker);
		if (Scanner == NIL)
			{
				return NIL;
			}
		SetTag(Scanner,"ScannerRec");

		Scanner->Block = AllocTrackedBlock(PtrSize(RawData),TrashTracker);
		if (Scanner->Block == NIL)
			{
				return NIL;
			}
		SetTag(Scanner->Block,"ScannerRec:  data block");

		CopyData(RawData,Scanner->Block,PtrSize(RawData));

		Scanner->BlockLength = PtrSize(Scanner->Block);
		Scanner->Index = 0;
		Scanner->LineNumber = 1;
		Scanner->NumKeywords = 0;
		Scanner->Allocator = TrashTracker;
		Scanner->PushedBackToken = NIL;

		return Scanner;
	}


/* negative = L < R; zero = equal, positive = L > R */
static int					CompareStrings(char* LRef, long LLen, char* RRef, long RLen)
	{
		long							Scan;

		Scan = 0;
		while ((Scan < LLen) && (Scan < RLen))
			{
				int								Result;

				Result = LRef[Scan] - RRef[Scan];
				if (Result != 0)
					{
						/* negative means LRef < RRef */
						/* positive means LRef > RRef */
						return Result;
					}
				Scan += 1;
			}
		/* if strings are equal up to smallest lengthed one, then */
		/* if left is shorter, then left is less */
		/* if right is shorter, then right is less */
		/* if lengths are equal, then strings are equal */
		return LLen - RLen;
	}


/* add a token to the scanner.  Keyword is a null terminated statically */
/* allocated string */
void								AddKeywordToScanner(ScannerRec* Scanner, char* Keyword, long Tag)
	{
		long							Scan;
		long							KeywordLength;

		CheckPtrExistence(Scanner);

		ERROR(Scanner->NumKeywords == MAXNUMKEYWORDS,PRERR(ForceAbort,
			"AddKeywordToScanner:  too many keywords"));

		KeywordLength = StrLen(Keyword);
		Scan = 0;
		while (Scan < Scanner->NumKeywords)
			{
				int								Compare;

				Compare = CompareStrings(Keyword,KeywordLength,Scanner->KeywordList[Scan]
					.KeywordName,Scanner->KeywordList[Scan].KeywordLength);
				ERROR(Compare == 0,PRERR(ForceAbort,
					"AddKeywordToScanner:  keyword already known"));
				if (Compare < 0)
					{
						long							Index;

						/* our string is less, so insert at Scan */
						for (Index = Scanner->NumKeywords; Index > Scan; Index -= 1)
							{
								Scanner->KeywordList[Index] = Scanner->KeywordList[Index - 1];
							}
						Scanner->KeywordList[Scan].KeywordName = Keyword;
						Scanner->KeywordList[Scan].KeywordLength = KeywordLength;
						Scanner->KeywordList[Scan].TagValue = Tag;
						Scanner->NumKeywords += 1;
						return;
					}
				/* else go to the next one */
				Scan += 1;
			}
		/* if there was nowhere to insert, then just put it in there hey */
		Scanner->KeywordList[Scanner->NumKeywords].KeywordName = Keyword;
		Scanner->KeywordList[Scanner->NumKeywords].KeywordLength = KeywordLength;
		Scanner->KeywordList[Scanner->NumKeywords].TagValue = Tag;
		Scanner->NumKeywords += 1;
	}


/* get the line number (starting at 1) that the scanner is on right now */
long								GetCurrentLineNumber(ScannerRec* Scanner)
	{
		CheckPtrExistence(Scanner);
		return Scanner->LineNumber;
	}


#define ENDOFTEXT (-1)

static int					GetCharacter(ScannerRec* Scanner)
	{
		int								Value;

		CheckPtrExistence(Scanner);
		if (Scanner->Index == Scanner->BlockLength)
			{
				return ENDOFTEXT;
			}
		Value = Scanner->Block[Scanner->Index];
		Scanner->Index += 1;
		return Value;
	}


static void					UngetCharacter(ScannerRec* Scanner)
	{
		CheckPtrExistence(Scanner);
		ERROR(Scanner->Index == 0,PRERR(ForceAbort,"UngetCharacter:  can't do it"));
		Scanner->Index -= 1;
	}


/* get a token */
TokenRec*						GetNextToken(ScannerRec* Scanner)
	{
		int								C;
		TokenRec*					Token;

		CheckPtrExistence(Scanner);

		/* check for pushback */
		if (Scanner->PushedBackToken != NIL)
			{
				TokenRec*					Temp;

				Temp = Scanner->PushedBackToken;
				Scanner->PushedBackToken = NIL;
				return Temp;
			}

		/* get a character */
		C = GetCharacter(Scanner);

		/* strip while space */
		while (((C >= 0) && (C <= 32)) || (C == '#'))
			{
				if ((C == 13) || (C == 10))
					{
						Scanner->LineNumber += 1;
					}
				if (C == '#')
					{
						/* comment */
						while ((C != 13) && (C != 10) && (C != ENDOFTEXT))
							{
								C = GetCharacter(Scanner);
							}
					}
				 else
					{
						C = GetCharacter(Scanner);
					}
			}

		/* handle the end of text character */
		if (C == ENDOFTEXT)
			{
				Token = (TokenRec*)AllocTrackedBlock(sizeof(TokenRec),Scanner->Allocator);
				if (Token == NIL)
					{
						return NIL;
					}
				Token->Type = eTokenEndOfInput;
			}

		/* handle a string literal */
		else if (C == '\x22')
			{
				StringThingRec*		String;
				char*							StringCopy;
				long							StringLength;

				String = NewStringThing();
				if (String == NIL)
					{
						return NIL;
					}

				C = GetCharacter(Scanner);
				while (C != '\x22')
					{
						char							Buffer[1];

						Buffer[0] = C;
						if (!AppendStringThing(String,&(Buffer[0]),1))
							{
								DisposeStringThing(String);
								return NIL;
							}
						if ((C == 10) || (C == 13))
							{
								Scanner->LineNumber += 1;
							}
						C = GetCharacter(Scanner);
					}

				StringCopy = StringThingGetSubrange(String,0,GetStringThingLength(String));
				DisposeStringThing(String);
				if (StringCopy == NIL)
					{
						return NIL;
					}
				StringLength = PtrSize(StringCopy);

				Token = (TokenRec*)AllocTrackedBlock(sizeof(TokenRec),Scanner->Allocator);
				if (Token == NIL)
					{
						ReleasePtr(StringCopy);
						return NIL;
					}
				Token->Type = eTokenString;

				Token->u.StringValue = AllocTrackedBlock(StringLength,Scanner->Allocator);
				if (Token->u.StringValue == NIL)
					{
						ReleasePtr(StringCopy);
						return NIL;
					}
				CopyData(StringCopy,Token->u.StringValue,StringLength);
				ReleasePtr(StringCopy);
			}

		/* handle an identifier:  [a-zA-Z_][a-zA-Z0-9_]*  */
		else if (((C >= 'a') && (C <= 'z')) || ((C >= 'A') && (C <= 'Z')) || (C == '_'))
			{
				StringThingRec*		String;
				long							LowBound;
				long							HighBoundPlusOne;
				MyBoolean					ContinueLoopingFlag;
				long							KeywordIndex; /* -1 == not a keyword */
				char*							StringCopy;
				long							StringLength;

				String = NewStringThing();
				if (String == NIL)
					{
						return NIL;
					}
				/* read the entire token */
				while (((C >= 'a') && (C <= 'z')) || ((C >= 'A') && (C <= 'Z')) || (C == '_')
					|| ((C >= '0') && (C <= '9')))
					{
						char							Buffer[1];

						Buffer[0] = C;
						if (!AppendStringThing(String,&(Buffer[0]),1))
							{
								DisposeStringThing(String);
								return NIL;
							}
						C = GetCharacter(Scanner);
					}
				/* unget the character that made us stop */
				UngetCharacter(Scanner);
				/* get the string out of the line buffer */
				StringCopy = StringThingGetSubrange(String,0,GetStringThingLength(String));
				DisposeStringThing(String);
				if (StringCopy == NIL)
					{
						return NIL;
					}
				StringLength = PtrSize(StringCopy);

				/* figure out if it is a keyword */
				LowBound = 0;
				HighBoundPlusOne = Scanner->NumKeywords;
				ContinueLoopingFlag = True;
				while (ContinueLoopingFlag)
					{
						long							MidPoint;
						int								CompareResult;

						ERROR(LowBound > HighBoundPlusOne,PRERR(ForceAbort,
							"GetNextToken:  we seem to have a problem with low and high bounds"));

						MidPoint = (LowBound + HighBoundPlusOne) / 2;

						CompareResult = CompareStrings(StringCopy,PtrSize(StringCopy),
							Scanner->KeywordList[MidPoint].KeywordName,
							Scanner->KeywordList[MidPoint].KeywordLength);
						/* CompareResult == 0  -->  found the target */
						/* CompareResult < 0  --> in the first half of the list */
						/* CompareResult > 0  --> in the second half of the list */

						if (CompareResult == 0)
							{
								/* found the one */
								KeywordIndex = MidPoint;
								ContinueLoopingFlag = False;
							}
						 else
							{
								if (CompareResult < 0)
									{
										/* select first half of list */
										HighBoundPlusOne = MidPoint;
									}
								else /* if (CompareResult > 0) */
									{
										/* select second half of list */
										LowBound = MidPoint + 1;
									}
								/* termination condition:  if range in array collapses to an */
								/* empty array, then there is no entry in the array */
								if (LowBound == HighBoundPlusOne)
									{
										KeywordIndex = -1; /* indicate there is no keyword */
										ContinueLoopingFlag = False;
									}
							}
					}

				/* create the token */
				Token = (TokenRec*)AllocTrackedBlock(sizeof(TokenRec),Scanner->Allocator);
				if (Token == NIL)
					{
						ReleasePtr(StringCopy);
						return NIL;
					}

				if (KeywordIndex == -1)
					{
						/* no keyword; make a string containing token */
						Token->u.StringValue = AllocTrackedBlock(StringLength,Scanner->Allocator);
						if (Token->u.StringValue == NIL)
							{
								ReleasePtr(StringCopy);
								return NIL;
							}
						CopyData(StringCopy,Token->u.StringValue,StringLength);
						Token->Type = eTokenIdentifier;
					}
				 else
					{
						Token->Type = eTokenKeyword;
						Token->u.IntegerValue = Scanner->KeywordList[KeywordIndex].TagValue;
					}
				ReleasePtr(StringCopy);
			}

		/* integer or floating?  [0-9]+  [0-9]+"."[0-9]+([Ee][+-]?[0-9]+)?[sdf]?  */
		else if (((C >= '0') && (C <= '9')) || (C == '.'))
			{
				NumFormType				SpecifiedNumType;
				NumStateType			NumberState;
				StringThingRec*		String;
				char*							StringData;
				long							StringLength;

				/* initialize the state value */
				NumberState = eIntegerPart;
				SpecifiedNumType = eTypeNotSpecified;

				Token = (TokenRec*)AllocTrackedBlock(sizeof(TokenRec),Scanner->Allocator);
				if (Token == NIL)
					{
						return NIL;
					}

				String = NewStringThing();
				if (String == NIL)
					{
						return NIL;
					}

				while (((C >= '0') && (C <= '9')) || (C == '.') || (C == '+') || (C == '-')
					|| (C == 's') || (C == 'd') || (C == 'f') || (C == 'e') || (C == 'E'))
					{
						char							Buffer[1];

						/* do some state changes */
						if (C == '.')
							{
								if (NumberState != eIntegerPart)
									{
										Token->Type = eTokenError;
										Token->u.IntegerValue = eScannerMalformedFloat;
										DisposeStringThing(String);
										goto AbortNumberErrorPoint;
									}
								 else
									{
										NumberState = eFractionalPart;
									}
							}
						else if ((C == 'e') || (C == 'E'))
							{
								if ((NumberState != eIntegerPart) && (NumberState != eFractionalPart))
									{
										Token->Type = eTokenError;
										Token->u.IntegerValue = eScannerMalformedFloat;
										DisposeStringThing(String);
										goto AbortNumberErrorPoint;
									}
								 else
									{
										NumberState = eExponentialPart;
									}
							}
						else if ((C == '+') || (C == '-'))
							{
								if (NumberState != eExponentialPart)
									{
										/* this is not an error, since it could be a unary operator */
										/* coming later, so we stop, but don't abort */
										goto FinishNumberPoint; /* character ungot at target */
									}
								 else
									{
										NumberState = eExponNumberPart;
									}
							}
						else if (C == 's')
							{
								if (NumberState == eNumberFinished)
									{
										Token->Type = eTokenError;
										Token->u.IntegerValue = eScannerMalformedFloat;
										DisposeStringThing(String);
										goto AbortNumberErrorPoint;
									}
								 else
									{
										NumberState = eNumberFinished;
										SpecifiedNumType = eTypeSingle;
										C = 32; /* so adding it to the string doesn't do damage */
									}
							}
						else if (C == 'd')
							{
								if (NumberState == eNumberFinished)
									{
										Token->Type = eTokenError;
										Token->u.IntegerValue = eScannerMalformedFloat;
										DisposeStringThing(String);
										goto AbortNumberErrorPoint;
									}
								 else
									{
										NumberState = eNumberFinished;
										SpecifiedNumType = eTypeDouble;
										C = 32;
									}
							}
						else if (C == 'f')
							{
								if (NumberState == eNumberFinished)
									{
										Token->Type = eTokenError;
										Token->u.IntegerValue = eScannerMalformedFloat;
										DisposeStringThing(String);
										goto AbortNumberErrorPoint;
									}
								 else
									{
										NumberState = eNumberFinished;
										SpecifiedNumType = eTypeFixed;
										C = 32;
									}
							}

						/* actually save the character */
						Buffer[0] = C;
						if (!AppendStringThing(String,&(Buffer[0]),1))
							{
								DisposeStringThing(String);
								return NIL;
							}

						C = GetCharacter(Scanner);
					}
			 FinishNumberPoint:
				UngetCharacter(Scanner);

				StringData = StringThingGetSubrange(String,0,GetStringThingLength(String));
				DisposeStringThing(String);
				if (StringData == NIL)
					{
						return NIL;
					}
				StringLength = PtrSize(StringData);

				/* if the token type is not specified, then see what we can guess */
				if (eTypeNotSpecified == SpecifiedNumType)
					{
						if (NumberState == eIntegerPart)
							{
								/* if we only got as far as the integer part, then it's an int */
								SpecifiedNumType = eTypeInteger;
							}
						 else
							{
								/* otherwise, assume the highest precision type */
								SpecifiedNumType = eTypeDouble;
							}
					}

				/* create the token */
				switch (SpecifiedNumType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"GetNextToken:  bad number type specifier"));
							break;
						case eTypeSingle:
							Token->Type = eTokenSingle;
							Token->u.SingleValue = StringToLongDouble(StringData,StringLength);
							break;
						case eTypeDouble:
							Token->Type = eTokenDouble;
							Token->u.DoubleValue = StringToLongDouble(StringData,StringLength);
							break;
						case eTypeFixed:
							Token->Type = eTokenFixed;
							Token->u.FixedValue = double2largefixed(StringToLongDouble(StringData,
								StringLength));
							break;
						case eTypeInteger:
							Token->Type = eTokenInteger;
							Token->u.IntegerValue = StringToInteger(StringData,StringLength);
							break;
					}
				ReleasePtr(StringData);

				/* this is the escape point for when a bad character is encountered. */
			 AbortNumberErrorPoint:
				;
			}

		/* handle a symbol */
		else
			{
				Token = (TokenRec*)AllocTrackedBlock(sizeof(TokenRec),Scanner->Allocator);
				if (Token == NIL)
					{
						return NIL;
					}

				switch (C)
					{
						default:
							Token->Type = eTokenError;
							Token->u.IntegerValue = eScannerUnknownCharacter;
							break;
						case OPAREN:
							Token->Type = eTokenOpenParen;
							break;
						case CPAREN:
							Token->Type = eTokenCloseParen;
							break;
						case OBRACKET:
							Token->Type = eTokenOpenBracket;
							break;
						case CBRACKET:
							Token->Type = eTokenCloseBracket;
							break;
						case ':':
							C = GetCharacter(Scanner);
							if (C == '=')
								{
									Token->Type = eTokenColonEqual;
								}
							 else
								{
									/* push the character back */
									UngetCharacter(Scanner);
									Token->Type = eTokenColon;
								}
							break;
						case ';':
							Token->Type = eTokenSemicolon;
							break;
						case ',':
							Token->Type = eTokenComma;
							break;
						case '+':
							Token->Type = eTokenPlus;
							break;
						case '-':
							Token->Type = eTokenMinus;
							break;
						case '*':
							Token->Type = eTokenStar;
							break;
						case '/':
							Token->Type = eTokenSlash;
							break;
						case '=':
							Token->Type = eTokenEqual;
							break;
						case '<':
							C = GetCharacter(Scanner);
							if (C == '>')
								{
									Token->Type = eTokenNotEqual;
								}
							else if (C == '=')
								{
									Token->Type = eTokenLessThanOrEqual;
								}
							else if (C == '<')
								{
									Token->Type = eTokenShiftLeft;
								}
							else
								{
									Token->Type = eTokenLessThan;
									UngetCharacter(Scanner);
								}
							break;
						case '>':
							C = GetCharacter(Scanner);
							if (C == '=')
								{
									Token->Type = eTokenGreaterThanOrEqual;
								}
							else if (C == '>')
								{
									Token->Type = eTokenShiftRight;
								}
							else
								{
									Token->Type = eTokenGreaterThan;
									UngetCharacter(Scanner);
								}
							break;
						case '^':
							Token->Type = eTokenCircumflex;
							break;
					}
			}

		return Token;
	}


/* push a token back onto the token stream */
void								UngetToken(ScannerRec* Scanner, TokenRec* Token)
	{
		CheckPtrExistence(Scanner);
		CheckPtrExistence(Token);
		ERROR(Scanner->PushedBackToken != NIL,PRERR(ForceAbort,
			"UngetToken:  there is already a token pushed back"));
		Scanner->PushedBackToken = Token;
	}


/* get the type of a token */
TokenTypes					GetTokenType(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		return Token->Type;
	}


/* get the string associated with an identifier */
char*								GetTokenIdentifierString(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenIdentifier,PRERR(ForceAbort,
			"GetTokenIdentifierString:  token isn't an identifier"));
		return Token->u.StringValue;
	}


/* get the string associated with a string token */
char*								GetTokenStringValue(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenString,PRERR(ForceAbort,
			"GetTokenStringValue:  token isn't a string"));
		return Token->u.StringValue;
	}


/* get the integer value associated with an integer token */
long								GetTokenIntegerValue(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenInteger,PRERR(ForceAbort,
			"GetTokenIntegerValue:  token isn't an integer"));
		return Token->u.IntegerValue;
	}


/* get the single precision value associated with a single float token */
float								GetTokenSingleValue(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenSingle,PRERR(ForceAbort,
			"GetTokenSingleValue:  token isn't a single"));
		return Token->u.SingleValue;
	}


/* get the double precision value associated with a double float token */
double							GetTokenDoubleValue(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenDouble,PRERR(ForceAbort,
			"GetTokenDoubleValue:  token isn't a double"));
		return Token->u.DoubleValue;
	}


/* get the fixed precision value associated with a fixed token */
largefixedsigned		GetTokenFixedValue(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenFixed,PRERR(ForceAbort,
			"GetTokenFixedValue:  token isn't a fixed point"));
		return Token->u.FixedValue;
	}


/* get the tag associated with a keyword token */
long								GetTokenKeywordTag(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenKeyword,PRERR(ForceAbort,
			"GetTokenKeywordTag:  token isn't a keyword"));
		return Token->u.IntegerValue;
	}


/* get the error code associated with an error token */
ScannerErrors				GetTokenErrorCode(TokenRec* Token)
	{
		CheckPtrExistence(Token);
		ERROR(Token->Type != eTokenError,PRERR(ForceAbort,
			"GetTokenErrorCode:  token isn't a keyword"));
		return (ScannerErrors)Token->u.IntegerValue;
	}
