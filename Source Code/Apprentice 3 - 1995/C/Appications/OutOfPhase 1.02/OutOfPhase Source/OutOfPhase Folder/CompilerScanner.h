/* CompilerScanner.h */

#ifndef Included_CompilerScanner_h
#define Included_CompilerScanner_h

/* CompilerScanner module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* TrashTracker */
/* DataMunging */
/* StringThing */
/* Numbers */
/* FixedPoint */

#include "FixedPoint.h"

/* workaround for strange CodeWarrior bug -- it doesn't like identifier TokenRec */
#define TokenRec MyTRec

struct ScannerRec;
typedef struct ScannerRec ScannerRec;

struct TokenRec;
typedef struct TokenRec TokenRec;

/* forward */
struct TrashTrackRec;

/* all blocks are allocated with TrashTracker */

/* token type */
typedef enum
	{
		eTokenIdentifier EXECUTE(= -8438),
		eTokenString,
		eTokenInteger,
		eTokenSingle,
		eTokenDouble,
		eTokenFixed,

		eTokenKeyword,

		eTokenOpenParen, /* ( */
		eTokenCloseParen, /* ) */
		eTokenOpenBracket, /* [ */
		eTokenCloseBracket, /* ] */
		eTokenColon, /* : */
		eTokenSemicolon, /* ; */
		eTokenComma, /* , */
		eTokenColonEqual, /* := */
		eTokenPlus, /* + */
		eTokenMinus, /* - */
		eTokenStar, /* * */
		eTokenSlash, /* / */
		eTokenEqual, /* = */
		eTokenNotEqual, /* <> */
		eTokenLessThan, /* < */
		eTokenLessThanOrEqual, /* <= */
		eTokenGreaterThan, /* > */
		eTokenGreaterThanOrEqual, /* >= */
		eTokenShiftLeft, /* << */
		eTokenShiftRight, /* >> */
		eTokenCircumflex, /* ^ */

		eTokenEndOfInput,

		eTokenError
	} TokenTypes;

typedef enum
	{
		eScannerMalformedFloat EXECUTE(= -27517),
		eScannerUnknownCharacter
	} ScannerErrors;

/* create a scanner information record */
ScannerRec*					NewScanner(struct TrashTrackRec* TrashTracker, char* RawData);

/* add a token to the scanner.  Keyword is a null terminated statically */
/* allocated string */
void								AddKeywordToScanner(ScannerRec* Scanner, char* Keyword, long Tag);

/* get the line number (starting at 1) that the scanner is on right now */
long								GetCurrentLineNumber(ScannerRec* Scanner);

/* get a token */
TokenRec*						GetNextToken(ScannerRec* Scanner);

/* push a token back onto the token stream */
void								UngetToken(ScannerRec* Scanner, TokenRec* Token);

/* get the type of a token */
TokenTypes					GetTokenType(TokenRec* Token);

/* get the string associated with an identifier */
char*								GetTokenIdentifierString(TokenRec* Token);

/* get the string associated with a string token */
char*								GetTokenStringValue(TokenRec* Token);

/* get the integer value associated with an integer token */
long								GetTokenIntegerValue(TokenRec* Token);

/* get the single precision value associated with a single float token */
float								GetTokenSingleValue(TokenRec* Token);

/* get the double precision value associated with a double float token */
double							GetTokenDoubleValue(TokenRec* Token);

/* get the fixed precision value associated with a fixed token */
largefixedsigned		GetTokenFixedValue(TokenRec* Token);

/* get the tag associated with a keyword token */
long								GetTokenKeywordTag(TokenRec* Token);

/* get the error code associated with an error token */
ScannerErrors				GetTokenErrorCode(TokenRec* Token);

#endif
