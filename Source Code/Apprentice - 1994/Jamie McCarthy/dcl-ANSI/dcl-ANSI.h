/*
 * dcl-ANSI.h
 *
 * By Jamie McCarthy, April 92.  This is public domain.
 *
 */



/******************************/

#include <ctype.h>
#include <string.h>

/******************************/



extern Handle gCDeclHndl;
extern char *gCDeclPtr, *gCDeclEndPtr;
extern char gEnglish[1024];
extern OSErr gError;
extern short gTokenType;

extern void startupDcl(void);
extern void declarator(void);
extern void buildFinalString(void);
extern void shutdownDcl(void);

enum {
	kNCStrings,
	kCStrMissingLeftParen,
	kCStrMissingRightParen,
	kCStrExpectedIdentOrDCL,
	kCStrExpectedVarName,
	kCStrNoTypeSpecifier,
	kCStrSyntaxError,
	kCStrArraySyntaxError,
	kCStrEnglishTooLong,
	kCStrFuncProtosTooDeep,
	kCStrFuncCantReturnFunc,
	kCStrFuncCantReturnArray,
	kCStrArrayTypeCantBeFunc,
	kCStrMacOSErrorOccurred,
	kCStrPeriod,
	
	kCStrLastErr = kCStrPeriod
} ;


	/*
	 * These aren't defined in dcl-ANSI.c;  the two implementations each
	 * define them differently.
	 *
	 * The string-getting routines are one-based;  getMiscCStr(getNMiscCStrs())
	 * is valid but getMiscCStr(getNMiscCStrs()+1) is not.
	 */
extern void startupCStrs(void);
extern short getNMiscCStrs(void);
extern char *getMiscCStr(short index);
extern short getNSpecifiers(void);
extern char *getSpecifier(short index);
extern void shutdownCStrs(void);
