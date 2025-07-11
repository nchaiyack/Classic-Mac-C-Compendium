/*
 * dcl-ANSI FKEY.c
 *
 * By Jamie McCarthy, April 92.  This is public domain.
 *
 * This source file provides a bridge between dcl-ANSI.c,
 * which just does the translation, and the clipboard
 * scrap and FKEY mechanism.
 *
 */



/******************************/

#include <SetupA4.h>
#include <pascal.h>
#include <ExternalInterface.h>
#include <DialogUtilities.h>

#include "dcl-ANSI.h"

/******************************/

enum {
	kNMiscCStrs = 28,
	kNSpecifiers = 23
} ;

char *gMiscCStr[kNMiscCStrs];
char *gSpecifier[kNSpecifiers];

/******************************/

pascal void main(void);

void getCDeclaration(void);
void putFinalString(void);

/******************************/



pascal void main(void)
{
	RememberA0();
	SetUpA4();
	
	gError = noErr;
	
	getCDeclaration();
	
	startupDcl();
	if (gError == noErr) {
		declarator();
	}
	buildFinalString();
	shutdownDcl();
	
	putFinalString();
	
	RestoreA4();
}



/******************************/



void getCDeclaration(void)
{
	long theLength, theOffset;
	gCDeclHndl = NewHandle(0);
	
	theLength = GetScrap(gCDeclHndl, 'TEXT', &theOffset);
	if (theLength < 0) {
		gError = theLength;
		gCDeclHndl = NULL;
	}
}



void putFinalString(void)
{
	long theLength;
	gError = ZeroScrap();
	if (gError == noErr) {
		theLength = strlen(gEnglish);
		gError = PutScrap(theLength, 'TEXT', &gEnglish[0]);
	}
	
		/*
		 * We need to tell whatever app we're in that it has a new clipboard,
		 * and that it needs to convert that clipboard to its own private
		 * format.  Here's how we do that.  Thanks to Bob Boonstra
		 * (jrb@mitre.org) for telling me how to do this.
		 */
	
	PostEvent(osEvt,
		(suspendResumeMessage<<24) + resumeFlag + convertClipboardFlag);
}



/******************************/



void startupCStrs(void)
{
		// If you add or remove strings, remember to change kNMiscCStrs.
	gMiscCStr[0] = "missing left-parenthesis.";
	gMiscCStr[1] = "missing right-parenthesis.";
	gMiscCStr[2] = "expected an identifier or a dcl but didn�t find one.";
	gMiscCStr[3] = "didn�t find a variable name.";
	gMiscCStr[4] = "couldn�t find a type specifier (ANSI, unlike K&R 1st ed., requires one).";
	gMiscCStr[5] = "syntax error.";
	gMiscCStr[6] = "array syntax error.";
	gMiscCStr[7] = "the English output string is too long. (!)";
	gMiscCStr[8] = "function prototypes are nested too deep (my fault�sorry).";
	gMiscCStr[9] = "a function cannot return an actual function�try a function pointer instead, maybe.";
	gMiscCStr[10] = "a function cannot return an actual array�try a pointer instead, maybe.";
	gMiscCStr[11] = "an array�s element type cannot be an actual function�try a pointer to function instead, maybe.";
	gMiscCStr[12] = "A Mac OS error occurred. Your lucky number for today is ";
	gMiscCStr[13] = ".";
	gMiscCStr[14] = "The variable named �";
	gMiscCStr[15] = "� is of type �";
	gMiscCStr[16] = ".�";
	gMiscCStr[17] = " ";
	gMiscCStr[18] = "pointer to";
	gMiscCStr[19] = "function";
	gMiscCStr[20] = "with undefined parameters";
	gMiscCStr[21] = "accepting a parameter list of type (";
	gMiscCStr[22] = ", ";
	gMiscCStr[23] = ") and";
	gMiscCStr[24] = "returning type";
	gMiscCStr[25] = "array";
	gMiscCStr[26] = "of";
	gMiscCStr[27] = "An error occurred: ";
		// If you add or remove strings, remember to change kNSpecifiers.
	gSpecifier[0] = "void";
	gSpecifier[1] = "unsigned";
	gSpecifier[2] = "signed";
	gSpecifier[3] = "const";
	gSpecifier[4] = "volatile";
	gSpecifier[5] = "char";
	gSpecifier[6] = "short";
	gSpecifier[7] = "long";
	gSpecifier[8] = "int";
	gSpecifier[9] = "register";
	gSpecifier[10] = "static";
	gSpecifier[11] = "extern";
	gSpecifier[12] = "auto";
	gSpecifier[13] = "float";
	gSpecifier[14] = "double";
	gSpecifier[15] = "pascal";
	gSpecifier[16] = "Handle";
	gSpecifier[17] = "Ptr";
	gSpecifier[18] = "Rect";
	gSpecifier[19] = "Byte";
	gSpecifier[20] = "Boolean";
	gSpecifier[21] = "GrafPtr";
	gSpecifier[22] = "WindowPtr";
}



short getNMiscCStrs(void)
{
	return kNMiscCStrs;
}



char *getMiscCStr(short index)
{
	return gMiscCStr[index-1];
}



short getNSpecifiers(void)
{
	return kNSpecifiers;
}



char *getSpecifier(short index)
{
	return gSpecifier[index-1];
}



void shutdownCStrs(void)
{
	
}

