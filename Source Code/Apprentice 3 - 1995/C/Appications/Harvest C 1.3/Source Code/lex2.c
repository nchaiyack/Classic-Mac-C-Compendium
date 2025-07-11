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
 * This file contains parts of the lexical scanner.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "structs.h"

#pragma segment Lex2

PPSYMVia_t
isDefined(char *name)
{
    PPSYMVia_t                      found;
    found = PPTableSearch(Defines, name);
    if (found) {
	if (GetPPSymFlags(found) & PREPROCUNDEF) {
	    return 0;
	} else {
	    return found;
	}
    } else {
	return 0;
    }
    return found;
}

PPSYMVia_t
AddDefine(char *name, EString_t value)
/*
 * This routine adds a #define symbol to the Preprocessor Defines list.  Note
 * that all #define symbols, including macros in the routine above, are
 * inserted at the front of the list.
 */
{
    PPSYMVia_t                      tmp;
#ifdef Undefined
    tmp = PPTableSearch(Defines, name);
    if (tmp) {
	UserWarning2(WARN_preprocredef, name);
	if (GetPPSymFlags(tmp) & PREPROCLOCKED) {
	    return tmp;
	}
	tmp = PPTableRemove(Defines, name);
    }
#endif
    PreprocSymbolCount++;
    tmp = PPTableAdd(Defines, name);
    SetPPSymValue(tmp, value);
    return tmp;
}

PPSYMVia_t
AddMacroFunc(char *name, int NumArgs, EString_t value, SymListVia_t ParmNames)
/*
 * This function adds a #define macro with parameters to the PreProcessor
 * macro list.  See the data structure definition for the macro structure for
 * explanations of the fields.
 */
{
    PPSYMVia_t                      tmp;
    tmp = AddDefine(name, value);
    if (tmp) {
	SetPPSymArgCount(tmp, NumArgs);
	SetPPSymArgs(tmp, ParmNames);
    }
    return tmp;
}

PPSYMVia_t
UnDefine(char *name)
{
    PPSYMVia_t                      found;
    found = PPTableSearch(Defines, name);
    if (found) {
	if (GetPPSymFlags(found) & PREPROCLOCKED) {
	    return found;
	} else {
	    return PPTableRemove(Defines, found);
	}
    } else {
	return 0;
    }
    return found;
}

/*
 * The PPStatus is used by the PreProcessor to determine when code should be
 * ignored or not.  When an #ifdef fails, PPStatus is set to true.  Whenever
 * PPStatus is true, the parser ignores the tokens it reads.  A stack is used
 * to keep track of nested #if type preprocessor commands.
 */

void
SetPPStatus(int stat)
/* Actually this is a stack push */
{
    PreprocStack[PPSP++] = PPStatus;
    PPStatus = stat;
}

void
GetPPStatus(void)
/* Actually this is a stack pop */
{
    PPStatus = PreprocStack[--PPSP];
}

int
hexvalue(char c)
/* Returns the integer value of the hex digit passed. */
{
    if ((c >= '0') && (c <= '9')) {
	return c - '0';
    } else if ((c >= 'a') && (c <= 'f')) {
	return c - 'a' + 10;
    } else if ((c >= 'A') && (c <= 'F')) {
	return c - 'A' + 10;
    } else {
	VeryBadParseError("hexvalue() was passed a non-hex character");
    }
    return 0;
}

int
ishexdigit(char c)
/* Returns true iff the arg is a valid hex digit. */
{
    return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) ||
	    ((c >= 'A') && (c <= 'F')));
}

int
isoctaldigit(char c)
/*
 * Returns true iff the arg is a valid octal digit.  Note that, in accordance
 * with ANSI, 8 and 9 are not valid octal digits.
 */
{
    return (((c >= '0') && (c <= '7')));
}

SYMVia_t
isTypedefName(char *name)
{
    SYMVia_t                        result;
    result = TableSearch(TP_defnames, (name));
    if (result) {
	if (LookUpSymbol(name)) {
	    result = NULL;
	}
    }
    return result;
}

int
isFirstIDChar(char c)
{
    /* Legal characters to begin an identifier are [a-zA-Z_] */
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c ==
	'_')
	return 1;
    else
	return 0;
}

int
isAnyIDChar(char c)
{
    /*
     * After the first character, an identifer may include digits.
     */
    return (isdigit(c) || isFirstIDChar(c));
}

int
isKeyword(char *toke)
/*
 * This routine uses a hash table for searching.  Note that the keyword
 * "defined" is special.  It is NOT a C keyword.  It is implemented as a
 * keyword, but is only valid during #if expressions. The only keyword
 * addition to the C language is "pascal" also "inline"
 */
{
    int                             ndx;
    int                             base;
    int                             done;
    ndx = KWHash(toke);
    base = ndx;
    done = 0;
    while (!done) {
	if (KWTable[ndx].val) {
	    if (!strcmp(KWTable[ndx].name, toke)) {
		if (KWTable[ndx].val == DEFINED) {
		    if (InPreprocIf) {
			return DEFINED;
		    } else {
			return 0;
		    }
		} else {
		    return KWTable[ndx].val;
		}
	    } else {
		ndx = (ndx + base) % KEYWORDTABLESIZE;
	    }
	} else
	    done = 1;
    }
    return 0;
}
