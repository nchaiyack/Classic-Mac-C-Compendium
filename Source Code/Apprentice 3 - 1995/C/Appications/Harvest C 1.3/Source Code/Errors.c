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
 * This file contains routines for error handling
 * 
 * 
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <console.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "tokens.h"
#include "limitations.h"
#include "structs.h"

#include "CHarvestDoc.h"
#include "CHarvestOptions.h"
#include "CErrorLog.h"

extern CHarvestDoc *gProject;
extern CErrorLog *gErrs;


/*
 * The following routines are used for communication of errors and warnings
 * to the user.  I have broken them down greatly for later portability.  The
 * lowest level routine is UserMesg().
 */

void
Output(char *mesg, GenericFileVia_t thefile)
/*
 * This is a general routine for sending output.  It is separated as an
 * entity because it is implementation dependent.
 */
{
#ifdef Undefined
    int                             bad;
    if (Via(thefile)->theWind) {
	/* Display in the window */
	long                            count;
	count = strlen(mesg);
    } else {
	/* Write directly to the file */
	long                            count;
	count = strlen(mesg);
	bad = FSWrite(Via(thefile)->refnum, &count, mesg);
    }
#endif
	gErrs->Hprintf(mesg);
}

void
OutputCR(char *mesg, GenericFileVia_t thefile)
/*
 * This is a general routine for sending output.  It is separated as an
 * entity because it is implementation dependent. This routine appends a
 * carriage return.
 */
{
#ifdef Undefined
    int                             bad;
    char                            m[256];
    sprintf(m, "%s\n", mesg);
    if (Via(thefile)->theWind) {
	/* Display in the window */
	long                            count;
	count = strlen(m);
    } else {
	/* Write directly to the file */
	long                            count;
	count = strlen(m);
	bad = FSWrite(Via(thefile)->refnum, &count, m);
}
#endif
	gErrs->Hprintf(mesg);
}

void
UserMesg(char *mesg)
/*
 * This is a general routine for issuing a message to the user.  It is
 * separated as an entity because the fprintf to the err channel is
 * implementation dependent, and the Macintosh version will not do it this
 * way.
 */
{
    char                            buf[256];
    sprintf(buf, "%s", mesg);
    Output(buf, errfile);
}

void
UserMesgCR(char *mesg)
/*
 * This is a general routine for issuing a message to the user.  It is
 * separated as an entity because the fprintf to the err channel is
 * implementation dependent, and the Macintosh version will not do it this
 * way.  This routine appends a carriage return.
 */
{
    char                            buf[256];
    sprintf(buf, "%s", mesg);
    Output(buf, errfile);
}

void
File_Line(char *buf)
/*
 * Outputs the current line number and file to the err channel. This is used
 * when issuing an error to the user.  Unfortunately, because of token
 * lookahead, this does not always give the actual line on which the error
 * occurred.
 */
{
	if (CurrentSRC.fileKind == SRC_SYSHEADER) {
   		 sprintf(buf, "<%s>:%d ", CurrentSRC.fname, CurrentSRC.LineCount);
	}
	else {
   		 sprintf(buf, "\"%s\":%d ", CurrentSRC.fname, CurrentSRC.LineCount);
	}
}

void
EccErrorNOFILE(char *mesg)
/*
 * General error routine.
 */
{
    NumErrors++;
    UserMesg(" ERRR ");
    UserMesgCR(mesg);
}

void
CodegenError(char *mesg)
{
    EccErrorNOFILE(mesg);
}

void
EccError(char *mesg)
/*
 * General error routine.  ALL errors should pass through here, so they may
 * be counted properly.
 */
{
    char buf[256];
    NumErrors++;
    File_Line(buf);
    strcat(buf,mesg);
    UserMesgCR(buf);
}

int
AssertError(char *file, int line, char *expr)
{
    char                            tmp[128];
    sprintf(tmp, "Assertion failed: %s line %d: %s\n", file, line, expr);
    c2pstr(tmp);
    DebugStr(tmp);
    return 1;
}

void
TypeError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Type, : %s", mesg);
    EccError(tmp);
}

void
SemanticError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Semantic, : %s", mesg);
    EccError(tmp);
}

void
DeclError2(char *mesg, char *info)
{
    char                            tmp[128];
    sprintf(tmp, "Declaration, : %s %s", mesg, info);
    EccError(tmp);
}

void
DeclErrorSYM(char *mesg, SYMVia_t thesym)
{
    char                            tmp[128];
#ifdef OLDMEM
    HLock((Handle) thesym);
#endif
    sprintf(tmp, "Declaration, : %s %s", mesg, (Via(thesym)->name));
#ifdef OLDMEM
    HUnlock((Handle) thesym);
#endif
    EccError(tmp);
    /* TODO Generate info here about previous declaration */
}

void
DeclError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Declaration, : %s", mesg);
    EccError(tmp);
}

void
FatalError2(char *mesg, char *info)
{
    char                            tmp[196];
    sprintf(tmp, "FATAL, : %s %s", mesg, info);
    gAbortCompile = 1;
    EccError(tmp);
}

void
FatalError(char *mesg)
{
    char                            tmp[196];
    sprintf(tmp, "FATAL, : %s", mesg);
    EccError(tmp);
}

void
PreprocError2(char *mesg, char *info)
{
    char                            tmp[128];
    sprintf(tmp, "Preproc, : %s %s", mesg, info);
    EccError(tmp);
}

void
UserError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "User, : %s", mesg);
    EccError(tmp);
}

void
PreprocError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Preproc, : %s", mesg);
    EccError(tmp);
}

void
LexError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Lexical, : %s", mesg);
    EccError(tmp);
}

void
SemanticError2(char *mesg, char *info)
{
    char                            tmp[128];
    sprintf(tmp, "Semantic, : %s %s", mesg, info);
    EccError(tmp);
}

void
InlineAsmError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Asm Syntax : %s", mesg);
    EccError(tmp);
}

void
SyntaxError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "Syntax, before %s: %s", LastToken, mesg);
    EccError(tmp);
}

void
Gen68Error(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "VERYBAD: %s", mesg);
    c2pstr(tmp);
    DebugStr(tmp);
}

void
VeryBadParseError(char *mesg)
{
    char                            tmp[128];
    sprintf(tmp, "File %s Line %d VERYBAD: %s",
	    CurrentSRC.fname, CurrentSRC.LineCount, mesg);
    c2pstr(tmp);
    DebugStr(tmp);
}

void
UserWarning(int num)
{
    char buf[256];
    AttemptWarnings++;
    if (!gProject->itsOptions->noWarnings) {
	if (gProject->itsOptions->allWarnings || gProject->itsOptions->warnings[num]) {
	    NumWarnings++;
	    File_Line(buf);
	    strcat(buf," WARN ");
	    GetWarningText(buf,num);
	    UserMesgCR(buf);
	}
    }
}

void
UserWarning2(int num, char *mesg)
{
    char buf[256];
    AttemptWarnings++;
    if (!gProject->itsOptions->noWarnings) {
	if (gProject->itsOptions->allWarnings || gProject->itsOptions->warnings[num]) {
	    NumWarnings++;
	    File_Line(buf);
	    strcat(buf," WARN ");
	    GetWarningText(buf,num);
	    strcat(buf,mesg);
	    UserMesgCR(buf);
	}
    }
}

void
EccWarning(char *mesg)
/*
 * General warning routine.  ALL warnings should pass through here, so that
 * they may be counted properly and not issued if warnings are disabled.
 */
{
	char buf[256];
    NumWarnings++;
    if (!gProject->itsOptions->noWarnings) {
	File_Line(buf);
	strcat(buf,mesg);
	UserMesgCR(buf);
    }
}
