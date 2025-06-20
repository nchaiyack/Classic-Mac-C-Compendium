/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains the lexical analyzer and preprocessor for Harvest C. The
 * preprocessor is not implemented as a separate stage, but as layers within
 * the lexer.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "structs.h"
#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CDataFile.h"
#include "CSourceFile.h"
#include "CHarvestOptions.h"
#include "CErrorLog.h"

extern CErrorLog *gErrs;

extern CHarvestDoc *gProject;
extern CHarvestApp *gApplication;
FILE *fopenMAC(char *name,short vRefNum,long dirID,char *mode);
extern	CSourceFile *gCurSourceFile;

#pragma segment Lexer

#define isEol(s) (((s) == '\n') || ((s) == '\r'))

void
ResetSRC(void)
{
    CurrentSRC.isIO = 0;
    CurrentSRC.where.io = NULL;
    CurrentSRC.PreprocLineDirty = 0;
    CurrentSRC.memindex = 0;
    CurrentSRC.fileKind = SRC_USRFILE;
    CurrentSRC.alreadyincluded = NULL;
    strcpy(CurrentSRC.fname, "");
    CurrentSRC.LineCount = 1;
    CurrentSRC.eol = 0;
    CurrentSRC.NeedtoKill = NULL;
    CurrentSRC.Macro = 0;
    CurrentSRC.ExtraChar[0] = 0;
    CurrentSRC.NumExtras = 0;
    CurrentSRC.CountCasts = 0;
    CurrentSRC.TotalIdentifierLength = 0;
    CurrentSRC.CountIdentifiers = 0;
    CurrentSRC.PreprocSubsts = 0;
    CurrentSRC.PreprocAfter = 0;
    CurrentSRC.PreprocBefore = 0;
    CurrentSRC.CountPreprocCondits = 0;
    CurrentSRC.StmtCount = 0;
    CurrentSRC.CountComments = 0;
    CurrentSRC.CommentBytes = 0;
}

void
PopSRC(void)
/*
 * Takes the top SRC off the stack and makes it current. When the stack is
 * empty, the file being parsed is done.
 */
{
    int                             wasio;
    SRCListVia_t                    tmpList;
    SRCKindVia_t                    tmpSRC;
    wasio = 0;
    tmpList = SRCStack;
    if (SRCStack) {
	if (CurrentSRC.isIO) {
	    tmpSRC = Via(tmpList)->data;
	}
    }
    if (CurrentSRC.isIO) {
	fclose(CurrentSRC.where.io);
	wasio = 1;
    }
    if (CurrentSRC.Macro) {
	CurrentSRC.Macro = 0;
    }
    if (CurrentSRC.NeedtoKill) {
	Efree(CurrentSRC.NeedtoKill);
    }
    if (tmpList) {
	tmpSRC = Via(tmpList)->data;
	SRCStack = Via(SRCStack)->next;
	CurrentSRC = Via(*tmpSRC);
	Efree(tmpList);
	Efree(tmpSRC);
	if (CurrentSRC.isIO) {
	    TakeFileOffHold(CurrentSRC.where.io);
	}
    } else {
	gAllDone = 1;
    }
}

void
PushSRC(void)
/*
 * Pushes the current SRC onto the stack.  This occurs when an include file
 * is invoked, or a preprocessor symbol is substituted. There may be other
 * cases where this routine is used in the future.
 */
{
    SRCKindVia_t                    tmpSRC;
    SRCListVia_t                    tmpList;
    tmpSRC = (SRCKindVia_t) Ealloc(sizeof(SRCKind_t));
    Via(*tmpSRC) = CurrentSRC;
    tmpList = (SRCListVia_t) Ealloc(sizeof(SRCList_t));
    Via(tmpList)->data = tmpSRC;
    Via(tmpList)->next = SRCStack;
    SRCStack = tmpList;
    if (CurrentSRC.isIO) {
	PutFileOnHold(CurrentSRC.where.io);
    }
    ResetSRC();
}

void
PutBackChar(char c)
/*
 * Puts a character back to the current SRC.  Used when the token parser
 * reads one two many characters.  The character buffer for each SRC has a
 * finite limit on its size.
 */
{
    if (!c)
	return;
    if (CurrentSRC.NumExtras >= MAXEXTRA) {
	VeryBadParseError("Too many characters PUTBACK");
    } else {
	CurrentSRC.ExtraChar[CurrentSRC.NumExtras++] = c;
	CurrentSRC.ExtraChar[CurrentSRC.NumExtras] = 0;
    }
}

unsigned char
RawCharacter(void)
{
    /*
     * This routine is lowest level routine for reading the SRC file. I am
     * breaking up the source file reading considerably, for portability and
     * a layered implementation of the preprocessor.
     */
    register int                    c;
  backtotop:
    c = 0;
    /* First, check to see if a character has been "put back" */
    if (CurrentSRC.NumExtras) {
	LastChar = CurrentSRC.ExtraChar[--CurrentSRC.NumExtras];
	return c = LastChar;
    } else {
	/* Now, handle the separate cases of file or string */
	if (CurrentSRC.isIO) {	/* FILE */
	    c = fgetc(CurrentSRC.where.io);
	    if (!gAllDone)
		CurrentSRC.CharCount++;
	    if (c == EOF) {
		PopSRC();
		if (!gAllDone)
		    goto backtotop;
	    }
	} else {
	    if (CurrentSRC.where.mem) {	/* String */
		c = Via(CurrentSRC.where.mem)[CurrentSRC.memindex];
		CurrentSRC.memindex++;
		if (c == 0) {
		    if (InPreprocIf) {
			PartDone = 1;
			return LastChar = 0;
		    }
		    PopSRC();
		    if (!gAllDone)
			goto backtotop;
		}
	    } else {		/* QQQQ Find out if this fragment is ever
				 * executed */
		if (InPreprocIf) {
		    PartDone = 1;
		    return LastChar = 0;
		}
		PopSRC();
		if (!gAllDone)
		    goto backtotop;
	    }
	}
    }
    /*
     * Source file lines are counted individually for each file. QQQQ:
     * Perhaps we should not count source lines for mem SRC's?
     */
#ifdef OLDLINEINC
    if (CurrentSRC.eol) {
	CurrentSRC.eol = 0;
	CurrentSRC.LineCount++;
	TotalLines++;
    }
    if (isEol(c)) {
		CurrentSRC.eol = 1;
    }
#else
    if (isEol(c)) {
		CurrentSRC.PreprocLineDirty = 0;
		TotalLines++;
		CurrentSRC.LineCount++;
    } else {
		if (!(isspace(c) || (c == '#')))
		    CurrentSRC.PreprocLineDirty = 1;
    }
#endif
    return (LastChar = c);
}

unsigned char
PreProcCharacter(void)
{
    /*
     * This routine handles layers 1 and 2 of the preprocessor, as described
     * in K & R, 2nd Ed., page 229. Trigraph sequences are converted, and all
     * occurences of the backslash followed by a newline are deleted, thus
     * splicing lines.
     */

    register unsigned char          c;
    unsigned char                   c2;
    c = RawCharacter();

    if (gProject->itsOptions->trigraphs) {
	int                             foundone;
	if (c == '?') {
	    c2 = RawCharacter();
	    if (c2 == '?') {
		/* trigraph */
		c2 = RawCharacter();
		foundone = 1;
		switch (c2) {
		case '=':
		    c = '#';
		    break;
		case '/':
		    c = '\\';
		    break;
		case '\'':
		    c = '^';
		    break;
		case '(':
		    c = '[';
		    break;
		case ')':
		    c = ']';
		    break;
		case '!':
		    c = '|';
		    break;
		case '<':
		    c = '{';
		    break;
		case '>':
		    c = '}';
		    break;
		case '-':
		    c = '~';
		    break;
		default:
		    foundone = 0;
		    PutBackChar('?');
		    PutBackChar('?');
		    break;
		}
		if (foundone) {
		    UserWarning(WARN_trigraphs);
		}
	    } else {
		PutBackChar(c2);
	    }
	}
    }
    /*
     * The fragment below splices lines separated by a backslash
     */
    if (c == '\\') {
	c2 = RawCharacter();
	if (isEol(c2))
	    c = RawCharacter();
	else
	    PutBackChar(c2);
    }
    return c;
}

unsigned char
TokenCharacter(void)
{
    /*
     * This character reading routine handles comments in the source.
     */

    register unsigned char          c;
    unsigned char                   c2;
    c = PreProcCharacter();

    if (c == '/') {
	c2 = PreProcCharacter();
	if (c2 == '*') {
	    int                             doneComment;
	    doneComment = 0;
	    CurrentSRC.CountComments++;
	    CurrentSRC.CommentBytes += 2;
	    while (!doneComment) {
		while (c != '*') {
		    c = PreProcCharacter();
		    CurrentSRC.CommentBytes++;
		    if (gAllDone) {
			FatalError("Unterminated comment");
		    }
		    if (c == '/') {
			c = PreProcCharacter();
			CurrentSRC.CommentBytes++;
			if (c == '*') {
			    UserWarning(WARN_nestedcomment);
			}
		    }
		}
		c = PreProcCharacter();
		CurrentSRC.CommentBytes++;
		if (c == '/') {
		    doneComment = 1;
		}
	    }
	    c = ' ';		/* ANSI says comments expand to whitespace */
	    /*
	     * To expand to nothing, this should read c = TokenCharacter()
	     */
	} else if ((c2 == '/')) {
	    while (!isEol(c)) {
		c = PreProcCharacter();
		CurrentSRC.CommentBytes++;
		if (gAllDone) {
		    FatalError("Unterminated comment");
		}
	    }
	} else {
	    /*
	     * In this case, '/' is the proper character to return, and we
	     * have read one character too many. We put it back.
	     */
	    PutBackChar(c2);
	}
    }
    return c;
}

unsigned char
NonSpaceCharacter(void)
/*
 * This routine simply returns a character that is assured not to be a space.
 * In addition, note that is gets its input from TokenCharacter, so comments
 * will never pass through here.
 */
{
    unsigned char                   c;
    c = TokenCharacter();
    while (isspace(c) && (!gAllDone))
	c = TokenCharacter();
    return c;
}

short
ParamVal(char *PStr)
{
    if (!strcmp(PStr, "__A0")) {
	return param__A0;
    } else if (!strcmp(PStr, "__A1")) {
	return param__A1;
    } else if (!strcmp(PStr, "__D0")) {
	return param__D0;
    } else if (!strcmp(PStr, "__D1")) {
	return param__D1;
    } else if (!strcmp(PStr, "__D2")) {
	return param__D2;
    }
    return 0;
}

void
StoreTillEOL(EString_t buf)
{
    register int                    ndx = 0;
    while (!isEol((Via(buf)[ndx++]) = PreProcCharacter()))	/* empty loop */
	;
    Via(buf)[ndx] = 0;
}

void
SkipTillEOL(void)
/*
 * Used occasionally by the PreProcessor for skipping the rest of a line -
 * use with care !
 */
{
	int c;
	c = PreProcCharacter();
	while (!isEol(c)) {
		c = PreProcCharacter();
	}
}

char
GetCharacter(void)
{				/* QQQQ routine too long ? */
    /*
     * A higher level character reading routine  - this one handles all the
     * commands for the PreProcessor.  It is important to keep in mind when
     * reading/writing this routine, that its purpose is to return a
     * character. Hence, even after handling a preproc directive, it is
     * necessary to fetch the next valid character to return.  In addition,
     * it is necessary to make this character fetch recursive, so further
     * preprocessor commands that may follow, will be executed.
     */
    char                            c;
    int                             ndx;
    int                             NumArgs;
    int                             donemacro;
    char                            PStr[128];
    int                             valndx;
    int                             doneargs;
    int                             ndx2;

    /* Warning, this routine has goto statements in it !!! */

  StartFromTheTop:

    ndx = 0;

    c = NonSpaceCharacter();	/* This routine skips all the whitespace
				 * before getting its first real character
				 * for processing.  For this reason, it is
				 * ONLY to be called between tokens.  Within
				 * a token, spaces are significant.  More
				 * specifically, spaces delimit tokens,
				 * except for string literals and character
				 * constants */
    if ((c == '#') && !CurrentSRC.PreprocLineDirty) {
	/* This indicates a preprocessor function */
	/*
	 * #ifdef #else #endif #elif #if #define #undef #include #ifndef
	 * #error #pragma #import
	 */
	ndx = 0;
	c = PreProcCharacter();
	/*
	 * spaces are allowed between the '#' and the preproc command itself,
	 * but they must be on the same line
	 */
	while ((!isEol(c)) && (!isFirstIDChar(c)))
	    c = PreProcCharacter();
	if (isEol(c)) {
	    /*
	     * This case means that the '#' was alone on the line. According
	     * to ANSI, this line is to be ignored, hence another character
	     * is fetched.
	     */
	    goto StartFromTheTop;
	}
	/*
	 * The routine reaches this point iff a non whitespace character was
	 * found on the same line as the '#'
	 */
	LastPreproc[ndx++] = c;
	while (isFirstIDChar(c = TokenCharacter())) {
	    LastPreproc[ndx++] = c;
	}
	LastPreproc[ndx] = 0;
	PutBackChar(c);
	/*
	 * LastPreproc is a string which now contains the command being
	 * executed.  We will test for each of these cases below and handle
	 * them individually.
	 */
	if ((!strcmp(LastPreproc, "include"))) {
	    char                            EndChar;
	    short                           incvol;
	    long                            incdir;
	    FILE *                newf;
	    int                             already;
	    FSSpec theHeader;
	    already = 0;
	    if (!PPStatus) {
		c = NonSpaceCharacter();
		/*
		 * We must skip whitespace between the word include and the
		 * filename, which may be in either <filename> or "filename"
		 * format.
		 */
		ndx = 0;
		if (c == '<') {
		    /* System Include file */
		    EndChar = '>';
#ifdef Undefined
		    strcpy(PStr,":Harvest C Headers:");
		    ndx = strlen(PStr);
#endif
		} else {
		    EndChar = '\"';
		}
		while ((c = PreProcCharacter()) != EndChar) {
		    PStr[ndx++] = c;
		    if (ndx >= 128) {
			FatalError("Include file name really long !!!!");
			break;
		    }
		}
		PStr[ndx] = 0;
		/*
		 * Now, PStr is a string containing the filename of the file
		 * to be included
		 */
		incvol = gCurSourceFile->theFile->volNum;
		incdir = gCurSourceFile->theFile->dirID;
		if (EndChar == '>') {
		    incvol = gApplication->StdIncludeVol;
		    incdir = gApplication->StdIncludeDir;
		}
		/*
		 * Now we have the filename, if this is an import, we need to
		 * check the current source to see if this has been included
		 * before.
		 */
		if (!already) {
		    newf = fopenMAC(PStr, incvol, incdir, "r");
		    if (!newf)
			newf = fopenMAC(PStr, gProject->StdAppVol, gProject->StdAppDir, "r");
		    if (!newf)
			newf = fopenMAC(PStr, gApplication->StdIncludeVol, gApplication->StdIncludeDir, "r");
		    if (!newf)
			newf = fopenMAC(PStr, gCurSourceFile->theFile->volNum, gCurSourceFile->theFile->dirID, "r");
		    if (newf) {
			CountIncludes++;
			PushSRC();
			CurrentSRC.isIO = 1;
			CurrentSRC.where.io = newf;
			if (EndChar == '>') {
				CurrentSRC.fileKind = SRC_SYSHEADER;
			}
			else {
				CurrentSRC.fileKind = SRC_USRHEADER;
			}
			strcpy(CurrentSRC.fname, PStr);
			goto StartFromTheTop;	/* still need to return a
						 * character */
		    } else {
			FatalError2("Include file not found ", PStr);
			SkipTillEOL();
			goto StartFromTheTop;	/* still need to return a
						 * character */
		    }
		} else {
		    SkipTillEOL();
		    goto StartFromTheTop;	/* still need to return a
						 * character */
		}
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "define")) {
	    SymListVia_t                    parmnames;
	    char                            TempName[128];
	    parmnames = NULL;
	    if (!PPStatus) {
		if (isspace(c))
		    c = NonSpaceCharacter();
		/*
		 * First, we read the identifier for the macro
		 */
		ndx = 0;
		PStr[ndx++] = c;
		while (isAnyIDChar(c = TokenCharacter())) {
		    PStr[ndx++] = c;
		}
		PStr[ndx] = 0;
		/*
		 * The identifier is now in PStr, and the character
		 * immediately following the identifier, is in c
		 */

		/*
		 * The character immediately following the identifier must be
		 * an open paren for this macro to be a macro function with
		 * parameters
		 */

		if (c == '(') {
		    /*
		     * This is a macro function.  It is now necessary to
		     * parse the parameters and eventually store the
		     * definition of the macro in Defines, using the
		     * AddMacroFunc routine
		     */
		    parmnames = RawTable(11);
		    NumArgs = 0;
		    ndx2 = 0;
		    doneargs = 0;
		    while (!doneargs) {
			/*
			 * Parse args and place into parmnames symbol list
			 */
			TempName[ndx2++] = c = NonSpaceCharacter();
			if (c != ')') {
			    while (isAnyIDChar(c = TokenCharacter())) {
				TempName[ndx2++] = c;
			    }
			    TempName[ndx2] = 0;
			    TableAdd(parmnames, TempName);
			    if (c == ')') {
				doneargs = 1;
			    }
			    if (c == ',') {
				NumArgs++;
				ndx2 = 0;
			    }
			} else {
			    doneargs = 1;
			}
		    }
		    NumArgs++;
		} else {
		    NumArgs = 0;
		    PutBackChar(c);
		}
		/*
		 * Here, macro functions and defined symbols are handled
		 * identically.  Both have a textual "value" which is
		 * terminated by a newline. Note that the characters being
		 * read are from TokenCharacter() , which means that lines
		 * ending in a backslashed have already been spliced by the
		 * time they arrive here.  Also, note that spaces are not
		 * ignored here, therefore a macro can expand to simply
		 * spaces.
		 */
		valndx = 0;
		donemacro = 0;
		while (!donemacro) {
		    c = TokenCharacter();
		    switch (c) {
		    case '\n':
		    case '\r':
			donemacro = 1;
			break;
		    default:
			if (valndx >= MAXMACROLENGTH)
			    FatalError("Macro too long!!!");
			Via(MacroValue)[valndx++] = c;
			break;
		    }
		}
		Via(MacroValue)[valndx] = 0;
		/*
		 * MacroValue now includes the value of the symbol/macro
		 * being defined.
		 */
		/*
		 * It is possible that valndx is 0, which means that there
		 * was no value on the line, and the symbol/macro will expand
		 * to nothing.
		 */
		/*
		 * NumArgs is true if the macro was a macro function.
		 */
		if (valndx) {
		    if (NumArgs) {
			if (!PPStatus)
			    AddMacroFunc(PStr, NumArgs, MacroValue, parmnames);
		    } else {
			if (!PPStatus)
			    AddDefine(PStr, MacroValue);
		    }
		} else {
		    if (NumArgs) {
			if (!PPStatus)
			    AddMacroFunc(PStr, NumArgs, NULL, parmnames);
		    } else {
			if (!PPStatus)
			    AddDefine(PStr, NULL);
		    }
		}
		/*
		 * No need for SkipTillEOL here because macros terminate at
		 * the end of the line.
		 */
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "ifdef")) {
	    CurrentSRC.CountPreprocCondits++;
	    if (!PPStatus) {
		c = NonSpaceCharacter();
		ndx = 0;
		PStr[ndx++] = c;
		while (isAnyIDChar(c = TokenCharacter())) {
		    PStr[ndx++] = c;
		}
		PStr[ndx] = 0;
		/*
		 * Now PStr contains the name of the identifer being tested.
		 */
		PutBackChar(c);
		/*
		 * A PPStatus which is true, indicates that the tokens to
		 * follow should be ignored. However, note that the level at
		 * which they are ignored is at the level of reading tokens.
		 * Therefore, all preprocessor functions are still executed.
		 * QQQQ: should this be changed ? At least, all preprocessor
		 * functions need to keep in mind that PPStatus may be true.
		 */
		if (isDefined(PStr) && !PPStatus) {
		    SetPPStatus(0);
		} else {
		    SetPPStatus(1);
		}
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		SetPPStatus(PPStatus);
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "ifndef")) {
	    /*
	     * #ifndef is the opposite of #ifdef, see above
	     */
	    CurrentSRC.CountPreprocCondits++;
	    if (!PPStatus) {
		c = NonSpaceCharacter();
		ndx = 0;
		PStr[ndx++] = c;
		while (isAnyIDChar(c = TokenCharacter())) {
		    PStr[ndx++] = c;
		}
		PStr[ndx] = 0;
		PutBackChar(c);
		if (!isDefined(PStr) && !PPStatus) {
		    SetPPStatus(0);
		} else {
		    SetPPStatus(1);
		}
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		SetPPStatus(1);
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "else")) {
	    /*
	     * This simply flips the PPStatus.  QQQQ is it necessary to
	     * verify that there has indeed been a conditional compilation
	     * test before the #else ?
	     */
	    int                             oldstat;
	    oldstat = PPStatus;
	    GetPPStatus();
	    if (PPStatus) {
		SetPPStatus(1);
	    } else {
		SetPPStatus(!oldstat);
	    }
	    SkipTillEOL();
	    goto StartFromTheTop;	/* still need to return a character */
	} else if (!strcmp(LastPreproc, "elif")) {
	    ParseTreeVia_t                  expr;
	    int                             flag;
	    EString_t                       IfLine;
	    SRCListVia_t                    tempstk;
	    int                             oldstat;
	    oldstat = PPStatus;
	    CurrentSRC.CountPreprocCondits++;
	    GetPPStatus();

	    if ((!PPStatus) && oldstat) {
		IfLine = Ealloc(MAXELIFEXPR);
		StoreTillEOL(IfLine);
		PushSRC();
		CurrentSRC.isIO = 0;
		CurrentSRC.where.mem = IfLine;

		tempstk = SRCStack;
		SRCStack = NULL;
		InPreprocIf = 1;
		PartDone = 0;
		expr = Do_constant_expr();
		PartDone = 0;
		SRCStack = tempstk;
		InPreprocIf = 0;
		Efree(IfLine);
		PopSRC();

		flag = GetIntValue(expr);
		FreeTree(expr);
		if (flag) {
		    SetPPStatus(0);
		}
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		SetPPStatus(1);
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "undef")) {
	    if (!PPStatus) {
		if (isspace(c))
		    c = NonSpaceCharacter();
		ndx = 0;
		PStr[ndx++] = c;
		while (isAnyIDChar(c = TokenCharacter())) {
		    PStr[ndx++] = c;
		}
		PStr[ndx] = 0;
		/*
		 * PStr now contains the identifer to be undefined
		 */
		if (!PPStatus) {
		    UnDefine(PStr);
		}
		PutBackChar(c);
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "endif")) {
	    GetPPStatus();
	    SkipTillEOL();
	    goto StartFromTheTop;	/* still need to return a character */
	} else if (!strcmp(LastPreproc, "if")) {
	    ParseTreeVia_t                  expr;
	    int                             flag;
	    EString_t                       IfLine;
	    SRCListVia_t                    tempstk;
	    CurrentSRC.CountPreprocCondits++;
	    if (!PPStatus) {
		IfLine = Ealloc(MAXELIFEXPR);
		StoreTillEOL(IfLine);
		PushSRC();
		CurrentSRC.isIO = 0;
		CurrentSRC.where.mem = IfLine;

		tempstk = SRCStack;
		SRCStack = NULL;
		InPreprocIf = 1;
		PartDone = 0;
		expr = Do_constant_expr();
		PartDone = 0;
		SRCStack = tempstk;
		Efree(IfLine);
		PopSRC();
		CurrentSRC.PreprocLineDirty = 0;
		InPreprocIf = 0;

		flag = GetIntValue(expr);
		FreeTree(expr);
		if (flag && !PPStatus) {
		    SetPPStatus(0);
		} else {
		    SetPPStatus(1);
		}
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		SetPPStatus(PPStatus);
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "line")) {
	    if (!PPStatus) {
		if (NextIs(INTCONSTANT)) {
		    CurrentSRC.LineCount = LastIntegerConstant;
		    if (NextIs(STRING_LITERAL)) {
			strcpy(CurrentSRC.fname, LastToken);
		    } else {
			PreprocError("Missing file name for #line");
		    }
		} else {
		    PreprocError("Missing line number for #line");
		}
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "pragma")) {
	    if (!PPStatus) {
		/* Read the next name - watch out for end of line */
		int                             cdone = 0;
		do {
		    c = TokenCharacter();
		    if (isEol(c))
			cdone = 2;
		    if (!isspace(c))
			cdone = 1;
		} while (!cdone);
		if (cdone != 2) {
		    ndx = 0;
		    PStr[ndx++] = c;
		    while (isAnyIDChar(c = TokenCharacter())) {
			PStr[ndx++] = c;
		    }
		    PStr[ndx] = 0;
		    PutBackChar(c);
		    if (!strcmp(PStr, "segment")) {
			/* Add a segment to the segment list */
			cdone = 0;
			do {
			    c = TokenCharacter();
			    if (isEol(c))
				cdone = 2;
			    if (!isspace(c))
				cdone = 1;
			} while (!cdone);
			if (cdone != 2) {
			    SYMVia_t                        seg;
			    ndx = 0;
			    PStr[ndx++] = c;
			    while (isAnyIDChar(c = TokenCharacter())) {
				PStr[ndx++] = c;
			    }
			    PStr[ndx] = 0;
			    PutBackChar(c);
			    CurrentSegmentLabel = LabTableAdd(SegmentNames, PStr);
			}
		    } else if (!strcmp(PStr, "parameter")) {
			int                             gotname = 0;
			int                             argindex = 0;
			int                             argdone = 0;
			ParamRecVia_t                   newparam;
			newparam = Ealloc(sizeof(ParamRec_t));
			Via(newparam)->returnreg = 0;
			Via(newparam)->args[0] = 0;
			Via(newparam)->args[1] = 0;
			Via(newparam)->args[2] = 0;
			Via(newparam)->args[3] = 0;
			Via(newparam)->name[0] = 0;
			Via(newparam)->next = NULL;

			cdone = 0;
			do {
			    c = TokenCharacter();
			    if (isEol(c)) {
				cdone = 2;
				PutBackChar(c);
			    }
			    if (!isspace(c))
				cdone = 1;
			} while (!cdone);
			if (cdone != 2) {
			    ndx = 0;
			    PStr[ndx++] = c;
			    while (isAnyIDChar(c = TokenCharacter())) {
				PStr[ndx++] = c;
			    }
			    PStr[ndx] = 0;
			    PutBackChar(c);
			    if (!strcmp(PStr, "__A0")) {
				Via(newparam)->returnreg = param__A0;
			    } else if (!strcmp(PStr, "__A1")) {
				Via(newparam)->returnreg = param__A1;
			    } else if (!strcmp(PStr, "__D0")) {
				Via(newparam)->returnreg = param__D0;
			    } else if (!strcmp(PStr, "__D1")) {
				Via(newparam)->returnreg = param__D1;
			    } else if (!strcmp(PStr, "__D2")) {
				Via(newparam)->returnreg = param__D2;
			    } else {
				strcpy(Via(newparam)->name, PStr);
				gotname = 1;
			    }
			    if (!gotname && (cdone != 2)) {
				cdone = 0;
				do {
				    c = TokenCharacter();
				    if (isEol(c)) {
					cdone = 2;
					PutBackChar(c);
				    }
				    if (!isspace(c))
					cdone = 1;
				} while (!cdone);
				if (cdone != 2) {
				    ndx = 0;
				    PStr[ndx++] = c;
				    while (isAnyIDChar(c = TokenCharacter())) {
					PStr[ndx++] = c;
				    }
				    PStr[ndx] = 0;
				    PutBackChar(c);
				}
				strcpy(Via(newparam)->name, PStr);
				gotname = 1;
			    }
			    if (cdone != 2) {
				cdone = 0;
				do {
				    c = TokenCharacter();
				    if (isEol(c)) {
					cdone = 2;
					PutBackChar(c);
				    }
				    if (!isspace(c))
					cdone = 1;
				} while (!cdone);
				if (cdone != 2) {
				    /* We have some regs to parse */
				    /* c must be a left paren */
				    assert(c == '(');
				    while (!argdone) {
					c = TokenCharacter();
					while (isspace(c))
					    c = TokenCharacter();
					ndx = 0;
					PStr[ndx++] = c;
					while (isAnyIDChar(c = TokenCharacter())) {
					    PStr[ndx++] = c;
					}
					PStr[ndx] = 0;
					Via(newparam)->args[argindex++] = ParamVal(PStr);
					while (isspace(c))
					    c = TokenCharacter();
					if (c == ')')
					    argdone = 1;
					else
					    assert(c == ',');
				    }
				}
			    }
			}
			Via(newparam)->argcount = argindex;
			Via(newparam)->next = ParamList;
			ParamList = newparam;
		    }
		}
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else if (!strcmp(LastPreproc, "error")) {
	    if (!PPStatus) {
		strcpy(PStr, "(#error) ");
		ndx = strlen(PStr);
		while (!isEol(c = TokenCharacter())) {
		    PStr[ndx++] = c;
		}
		PStr[ndx] = 0;
		UserError(PStr);
	    } else {
		SkipTillEOL();
		goto StartFromTheTop;	/* still need to return a character */
	    }
	} else {
	    PreprocError("Undefined preprocessor directive");
	    SkipTillEOL();
	    goto StartFromTheTop;	/* still need to return a character */
	}
    }
    return c;
}

void
LexerGetLine(char *buf)
{
    short                           c;
    c = GetCharacter();
    *buf++ = c;
    while (!isEol(c)) {
	c = TokenCharacter();
	if (isEol(c))
	    *buf = 0;
	else
	    *buf++ = c;
    }
}

char
EscapeConvert(char c)
{
    /*
     * This routine is used for converting escape characters within string
     * literals and character constants.
     */
    /*
     * QQQQ Do we need to define the effect of an undefined escape constant ?
     * ANSI does not.
     */
    int                             escapecode;
    switch (c) {
    case 'n':
	c = '\n';
	break;
    case 't':
	c = '\t';
	break;
    case 'v':
	c = '\v';
	break;
    case 'b':
	c = '\b';
	break;
    case 'r':
	c = '\r';
	break;
    case 'f':
	c = '\f';
	break;
    case 'a':
	c = '\a';
	break;
    case '\\':
	c = '\\';
	break;
    case '?':
	c = '\?';
	break;
    case '\'':
	c = '\'';
	break;
    case '\"':
	c = '\"';
	break;
    case 'x':
	escapecode = 0;
	while (ishexdigit(c = TokenCharacter()))
	    escapecode = escapecode * 16 + hexvalue(c);
	PutBackChar(c);
	c = escapecode;
	break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	/*
	 * octal character code
	 */
	escapecode = c - '0';
	while (isoctaldigit(c = TokenCharacter()))
	    escapecode = escapecode * 8 + (c - '0');
	PutBackChar(c);
	c = escapecode;
	break;
    }
    return c;
}

double
Eintpower(int mant, int expon)
{
    double                          result;
    int                             counter;
    result = 1;
    counter = 0;
    if (expon) {
	if (expon > 0) {
	    while (counter++ < expon) {
		result = result * mant;
	    }
	} else {
	    expon = -expon;
	    while (counter++ < expon) {
		result = result / mant;
	    }
	}
    } else {
	if (mant) {
	    result = 1;
	} else {
	    VeryBadParseError("0^0 is undefined (float constant)");
	    result = 0;
	}
    }
    return result;
}

/*
 * Reserved words: auto break case char const continue default do double else
 * enum extern float for goto if INLINE int long PASCAL register return short
 * signed sizeof static struct switch typedef union unsigned volatile while
 */

/*
 * Operators >>= <<= += -= = /= %= &= ^= |= >> << ++ -- -> && || <= >= == !=
 * ; { } , : = ( ) [ ] . & ! ~ - +
 * 
 * / < > ^ | ?
 */

#pragma segment AppInit

int
KWHash(char *name)
/* This is the hash function for the keyword hash table. */
{
    register int                    result;
    result = 0;
    while (*name) {
	result += *name;
	name++;
    }
    HashCount++;
    result = result % KEYWORDTABLESIZE;
    if (!result)
	result = 1;
    return result;
}

void
AddKW(char *name, Codigo_t val)
/*
 * Adds a new keyword to the keyword hash table.  Used to build the table
 * initially, and not used afterwards.
 */
{
    int                             TableNdx;
    int                             Base;
    TableNdx = Base = KWHash(name);
    while (KWTable[TableNdx].val) {
	HashCollisions++;
	TableNdx = (TableNdx + Base) % KEYWORDTABLESIZE;
    }
    KWTable[TableNdx].val = val;
    KWTable[TableNdx].name = name;
    KWTable[TableNdx].uses = 0;
}

void
BuildKWHash(void)
/* This routine builds the hash table for keywords. */
{
    AddKW("auto", AUTO);
    AddKW("break", BREAK);
    AddKW("case", CASE);
    AddKW("char", CHAR);
    AddKW("const", CONST);
    AddKW("continue", CONTINUE);
    AddKW("default", DEFAULT);
    AddKW("do", DO);
    AddKW("double", DOUBLE);
    AddKW("else", ELSE);
    AddKW("enum", ENUM);
    AddKW("extern", EXTERN);
    AddKW("float", FLOAT);
    AddKW("for", FOR);
    AddKW("goto", GOTO);
    AddKW("if", IF);
    AddKW("int", INT);
    AddKW("long", LONG);
    AddKW("pascal", PASCAL);
    AddKW("asm", ASM);
    AddKW("register", REGISTER);
    AddKW("return", RETURN);
    AddKW("short", SHORT);
    AddKW("signed", SIGNED);
    AddKW("sizeof", SIZEOF);
    AddKW("struct", STRUCT);
    AddKW("switch", SWITCH);
    AddKW("typedef", TYPEDEF);
    AddKW("union", UNION);
    AddKW("unsigned", UNSIGNED);
    AddKW("static", STATIC);
    AddKW("void", VOID);
    AddKW("volatile", VOLATILE);
    AddKW("while", WHILE);
    AddKW("...", ELLIPSIS);
    AddKW(">>=", RIGHT_ASSIGN);
    AddKW("<<=", LEFT_ASSIGN);
    AddKW("+=", ADD_ASSIGN);
    AddKW("-=", SUB_ASSIGN);
    AddKW("*=", MUL_ASSIGN);
    AddKW("/=", DIV_ASSIGN);
    AddKW("%=", MOD_ASSIGN);
    AddKW("&=", AND_ASSIGN);
    AddKW("^=", XOR_ASSIGN);
    AddKW("|=", OR_ASSIGN);
    AddKW(">>", RIGHT_OP);
    AddKW("<<", LEFT_OP);
    AddKW("++", INC_OP);
    AddKW("--", DEC_OP);
    AddKW("->", PTR_OP);
    AddKW("&&", AND_OP);
    AddKW("||", OR_OP);
    AddKW("<=", LE_OP);
    AddKW(">=", GE_OP);
    AddKW("==", EQ_OP);
    AddKW("!=", NE_OP);
    AddKW(";", ';');
    AddKW("{", '{');
    AddKW("}", '}');
    AddKW(",", ',');
    AddKW(":", ':');
    AddKW("=", '=');
    AddKW("(", '(');
    AddKW(")", ')');
    AddKW("[", '[');
    AddKW("]", ']');
    AddKW(".", '.');
    AddKW("&", '&');
    AddKW("!", '!');
    AddKW("~", '~');
    AddKW("-", '-');
    AddKW("+", '+');
    AddKW("*", '*');
    AddKW("/", '/');
    AddKW("%", '%');
    AddKW("<", '<');
    AddKW(">", '>');
    AddKW("^", '^');
    AddKW("|", '|');
    AddKW("?", '?');
    /*
     * The defined keyword is a very special keyword.  In reality it is not a
     * C keyword.  It is implemented as such for simplicity.  It is a
     * preprocessor feature.  Only during parsing of #if and #elif
     * expressions is this valid as a keyword.
     */
    AddKW("defined", DEFINED);
    AddKW("#", '#');
    /*
     * The crosshatch keyword is defined only for the purpose of the inline
     * assembler.  Usually, the character reading routines will catch it as a
     * preproc directive...
     */
    AddKW("@interface", atINTERFACE);
    AddKW("@implementation", atIMPLEMENTATION);
    AddKW("@end", atEND);
    AddKW("@selector", atSELECTOR);
    AddKW("@defs", atDEFS);
    AddKW("@encode", atENCODE);
    AddKW("@public", atPUBLIC);

    AddKW("extended", EXTENDED);
    AddKW("comp", COMP);
    AddKW("single", FLOAT);
}
