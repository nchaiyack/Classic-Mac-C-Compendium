/*
 * dcl-ANSI.c
 *
 * By Jamie McCarthy, April 92.  This is public domain.
 * If you have any questions or comments, you can reach me at
 * �k044477@kzoo.edu� on the Internet, or at �j.mccarthy�
 * on AppleLink.
 *
 * This code is based on a small program in Kernighan & Ritchie's
 * �The C Programming Language,� 2nd ed., �5.12.  Yes, that's
 * the famous �K&R,� the book that no C programmer should be
 * without.  I changed a lot of stuff, enough that I feel
 * comfortable about distributing this source (although I'm not
 * a lawyer...sigh...)
 *
 * Yeah, I know, I know, the code's a mess.  It could probably
 * be a helluva lot more elegant.  But it works.  Give me money,
 * and I'll clean it up for you.  :-)
 *
 */



/******************************/

#include <ctype.h>
#include <string.h>

#include "dcl-ANSI.h"

/******************************/

	/*
	 * Miscellaneous dcl-related variables and junk.
	 */
enum {
	kCStrTheVarNamed = kCStrLastErr + 1,
	kCStrIsOfType,
	kCStrEndOfSentence,
	kCStrSpaceSeparator,
	kCStrPointerTo,
	kCStrFunction,
	kCStrWithUndefdParams,
	kCStrAcceptingParamsOfType,
	kCStrCommaSeparator,
	kCStrAnd,
	kCStrReturningType,
	kCStrArray,
	kCStrOf,
	kCStrAnErrorOccurred
} ;
OSErr gError;
Handle gCDeclHndl;
char *gCDeclPtr, *gCDeclEndPtr;
#define getChar() (gCDeclPtr >= gCDeclEndPtr ? (++gCDeclPtr,'\0') : *gCDeclPtr++)
#define ungetChar() (--gCDeclPtr)
#define is_alpha(c) (isalpha(c) || (c) == '_')
#define is_alnum(c) (isalnum(c) || (c) == '_')
char gEnglish[1024];
char gToken[100];
char gName[100];
char gDataType[7][100];
short gDataTypeNo;
short gTokenType; enum { kTTName=1, kTTSpecifier, kTTBrackets };
enum { kLeftParen = '(', kRightParen = ')', kLeftBracket = '[', kRightBracket = ']' } ;
Boolean gRequireIdentifier;
Boolean gNextTypeIsBeingReturned;
Boolean gNextTypeIsArrayElement;

/******************************/

void startupDcl(void);
void scarfWhitespace(void);
Boolean isSpecifier(char *s);
char *englishCat(const char *suffix);
void declarator(void);
void dclANSI(void);
void dirDcl(void);
void funcProto(void);
short getToken(void);
void buildFinalString(void);
void shutdownDcl(void);

/******************************/



void startupDcl(void)
{
	startupCStrs();
	if (gError != noErr) return;
	
	HLock(gCDeclHndl);
	gCDeclPtr = *gCDeclHndl;
	gCDeclEndPtr = gCDeclPtr + GetHandleSize(gCDeclHndl);
	gEnglish[0] = '\0';
	gToken[0] = '\0';
	gName[0] = '\0';
	gDataType[0][0] = gDataType[1][0] = gDataType[2][0] =
		gDataType[3][0] = gDataType[4][0] = gDataType[5][0] = gDataType[6][0] =
		'\0';
	gDataTypeNo = 0;
	gRequireIdentifier = TRUE;
	gNextTypeIsBeingReturned = FALSE;
	gNextTypeIsArrayElement = FALSE;
	getToken();
}



char *englishCat(const char *suffix)
{
	if (gError != noErr
		|| strlen(gEnglish) + strlen(suffix) > sizeof(gEnglish) - 4) {
		
		if (gError == noErr) {
			gError = kCStrEnglishTooLong;
		}
		return NULL;
		
	} else {
		
		return strcat(gEnglish, suffix);
		
	}
}



void scarfWhitespace(void)
{
	char c;
	do {
		c = getChar();
	} while (c != '\0' && isspace(c));
	ungetChar();
}



Boolean isSpecifier(char *s)
{
	short i, nSpecifiers;
	nSpecifiers = getNSpecifiers();
	for (i = 1; i <= nSpecifiers; ++i) {
		if (!strcmp(s, getSpecifier(i))) {
			return TRUE;
		}
	}
	return FALSE;
}



void declarator(void)
{
	if (gDataTypeNo >= 6) {
		
		gError = kCStrFuncProtosTooDeep;
		
	} else {
		
		gDataType[gDataTypeNo][0] = '\0';
		while (gTokenType == kTTSpecifier) {
			if (gDataType[gDataTypeNo][0] != '\0') {
				strcat(gDataType[gDataTypeNo], getMiscCStr(kCStrSpaceSeparator));
			}
			strcat(gDataType[gDataTypeNo], gToken);
			getToken();
		}
		if (gDataType[gDataTypeNo][0] == '\0') {
			gError = kCStrNoTypeSpecifier;
		} else {
			
			++gDataTypeNo;
			dclANSI();
			--gDataTypeNo;
			
			englishCat(gDataType[gDataTypeNo]);
			
		}
		
	}
}



void dclANSI(void)
{
	short nStars = 0;
	while (gError == noErr && gTokenType == '*') {
		++nStars;
		getToken();
	}
	if (gError == noErr && gTokenType != '\0') {
		dirDcl();
	}
	while (gError == noErr && nStars-- > 0) {
		englishCat(getMiscCStr(kCStrPointerTo));
		englishCat(getMiscCStr(kCStrSpaceSeparator));
		gNextTypeIsBeingReturned = FALSE;
		gNextTypeIsArrayElement = FALSE;
	}
}



void dirDcl(void)
{
	short type;
	
	if (gTokenType == kLeftParen) {
		getToken();
		if (gTokenType == kRightParen) {
			if (gRequireIdentifier) {
				gError = kCStrExpectedIdentOrDCL;
			} else {
				if (gNextTypeIsBeingReturned) {
					gError = kCStrFuncCantReturnFunc;
				} else if (gNextTypeIsArrayElement) {
					gError = kCStrArrayTypeCantBeFunc;
				} else {
					englishCat(getMiscCStr(kCStrFunction));
					englishCat(getMiscCStr(kCStrSpaceSeparator));
					englishCat(getMiscCStr(kCStrWithUndefdParams));
					englishCat(getMiscCStr(kCStrSpaceSeparator));
					englishCat(getMiscCStr(kCStrReturningType));
					englishCat(getMiscCStr(kCStrSpaceSeparator));
					gNextTypeIsBeingReturned = TRUE;
				}
			}
		} else {
			if (gTokenType == kTTSpecifier && !gRequireIdentifier) {
				englishCat(getMiscCStr(kCStrFunction));
				englishCat(getMiscCStr(kCStrSpaceSeparator));
				englishCat(getMiscCStr(kCStrAcceptingParamsOfType));
				if (gError == noErr) {
					funcProto();
					if (gError == noErr) {
						englishCat(getMiscCStr(kCStrAnd));
						englishCat(getMiscCStr(kCStrSpaceSeparator));
						englishCat(getMiscCStr(kCStrReturningType));
						englishCat(getMiscCStr(kCStrSpaceSeparator));
						gNextTypeIsBeingReturned = TRUE;
					}
				}
			} else {
				dclANSI();
			}
			if (gError == noErr && gTokenType != kRightParen) {
				gError = kCStrMissingRightParen;
			}
		}
	} else if (gTokenType == kTTName) {
		if (gName[0] == '\0') {
				// The only name we care about is that of the main declarator,
				// the first one found.
			strcpy(gName, gToken);
		}
	} else if (!gRequireIdentifier && gTokenType == kTTBrackets) {
		englishCat(getMiscCStr(kCStrArray));
		englishCat(gToken);
		englishCat(getMiscCStr(kCStrSpaceSeparator));
		englishCat(getMiscCStr(kCStrOf));
		englishCat(getMiscCStr(kCStrSpaceSeparator));
		gNextTypeIsArrayElement = TRUE;
	} else {
		gError = kCStrExpectedIdentOrDCL;
	}
	while (gError == noErr && 
		((type = getToken()) == kTTBrackets
		|| type == kLeftParen)) {
		
		if (type == kLeftParen) {
			if (gNextTypeIsBeingReturned) {
				gError = kCStrFuncCantReturnFunc;
			} else if (gNextTypeIsArrayElement) {
				gError = kCStrArrayTypeCantBeFunc;
			} else {
				englishCat(getMiscCStr(kCStrFunction));
				englishCat(getMiscCStr(kCStrSpaceSeparator));
				if (gError == noErr && (type=getToken()) == kRightParen) {
					englishCat(getMiscCStr(kCStrWithUndefdParams));
					englishCat(getMiscCStr(kCStrSpaceSeparator));
					englishCat(getMiscCStr(kCStrReturningType));
					englishCat(getMiscCStr(kCStrSpaceSeparator));
					gNextTypeIsBeingReturned = TRUE;
				} else {
					englishCat(getMiscCStr(kCStrAcceptingParamsOfType));
					if (gError == noErr) {
						funcProto();
						if (gError == noErr) {
							englishCat(getMiscCStr(kCStrAnd));
							englishCat(getMiscCStr(kCStrSpaceSeparator));
							englishCat(getMiscCStr(kCStrReturningType));
							englishCat(getMiscCStr(kCStrSpaceSeparator));
							gNextTypeIsBeingReturned = TRUE;
						}
					}
				}
			}
		} else {
			if (gNextTypeIsBeingReturned) {
				gError = kCStrFuncCantReturnArray;
			} else {
				englishCat(getMiscCStr(kCStrArray));
				englishCat(gToken);
				englishCat(getMiscCStr(kCStrSpaceSeparator));
				englishCat(getMiscCStr(kCStrOf));
				englishCat(getMiscCStr(kCStrSpaceSeparator));
				gNextTypeIsArrayElement = TRUE;
			}
		}
		
	}
	
	if (!gRequireIdentifier && gError == kCStrExpectedIdentOrDCL) {
		gError = noErr;
	}
}



void funcProto(void)
{
	Boolean continueGettingArguments;
	do {
		gRequireIdentifier = FALSE;
		declarator();
		continueGettingArguments = (gError == noErr && gTokenType == ',');
		if (continueGettingArguments) {
			englishCat(getMiscCStr(kCStrCommaSeparator));
			getToken();
		}
	} while (gError == noErr && continueGettingArguments);
	gRequireIdentifier = TRUE;
	if (gError == noErr && gTokenType != kRightParen) {
		gError = kCStrMissingRightParen;
	}
}



short getToken(void)
{
	short c;
	char *p = gToken;
	
	scarfWhitespace();
	c = getChar();
	if (c == ';') c = '\0';
	if (c == kLeftParen) {
		gTokenType = kLeftParen;
	} else if (c == kLeftBracket) {
		*p++ = c;
		scarfWhitespace();
		c = getChar();
		if (is_alpha(c)) {
			do {
				*p++ = c;
			} while ( (c = getChar()) != '\0' && is_alnum(c) );
		} else if (isdigit(c)) {
			do {
				*p++ = c;
			} while ( (c = getChar()) != '\0' && isdigit(c) );
		}
		ungetChar();
		scarfWhitespace();
		c = getChar();
		if (c == kRightBracket) {
			*p++ = c;
			*p = '\0';
			gTokenType = kTTBrackets;
		} else {
			gError = kCStrArraySyntaxError;
		}
	} else if (is_alpha(c)) {
		*p++ = c;
		while ((c=getChar()), (is_alnum(c) || c == ':')) {
			*p++ = c;
		}
		*p = '\0';
		ungetChar();
		if (isSpecifier(gToken)) {
			gTokenType = kTTSpecifier;
		} else {
			gTokenType = kTTName;
		}
	} else {
		gTokenType = c;
	}
	return gTokenType;
}



void buildFinalString(void)
{
		/*
		 * Put the whole string into gEnglish, which can be up to 1K long.
		 * This can involve swapping a lot of stuff around.
		 */
	
	if (gError == noErr && gTokenType != '\0') {
		gError = kCStrSyntaxError;
	}
	if (gError < 0 || gError >= getNMiscCStrs()) {
		unsigned char errNumStr[8];
		strcpy(gEnglish, getMiscCStr(kCStrMacOSErrorOccurred));
		NumToString(gError, errNumStr);
		gError = 0;
		englishCat(PtoCstr(errNumStr));
		englishCat(getMiscCStr(kCStrPeriod));
	} else if (gError == 0) {
		short preEnglishLength, temp1, temp2;
		preEnglishLength = strlen(getMiscCStr(kCStrTheVarNamed))
			+ strlen(gName)
			+ strlen(getMiscCStr(kCStrIsOfType));
		if (gName[0] == '\0') {
			gError = kCStrExpectedVarName;
		} else if (preEnglishLength + strlen(gEnglish)
			+ strlen(getMiscCStr(kCStrEndOfSentence))
			> sizeof(gEnglish)) {
			gError = kCStrEnglishTooLong;
		} else {
			BlockMove(&gEnglish[0], &gEnglish[preEnglishLength], strlen(gEnglish)+1);
			temp1 = strlen(getMiscCStr(kCStrTheVarNamed));
			BlockMove(getMiscCStr(kCStrTheVarNamed), &gEnglish[0], temp1);
			temp2 = strlen(gName);
			BlockMove(&gName[0], &gEnglish[temp1], temp2);
			temp1 += temp2;
			temp2 = strlen(getMiscCStr(kCStrIsOfType));
			BlockMove(getMiscCStr(kCStrIsOfType), &gEnglish[temp1], temp2);
			englishCat(getMiscCStr(kCStrEndOfSentence));
		}
	}
	if (gError > 0) {
		strcpy(gEnglish, getMiscCStr(kCStrAnErrorOccurred));
		strcat(gEnglish, getMiscCStr(gError));
	}
	
	if (gError == 0) {
		long secs;
		DateTimeRec myDTR;
		GetDateTime(&secs);
		Secs2Date(secs, &myDTR);
		if (myDTR.month == 4 && myDTR.day == 1) {
			englishCat(" I think.");
		}
	}
}



void shutdownDcl(void)
{
	DisposHandle(gCDeclHndl);
	shutdownCStrs();
}

