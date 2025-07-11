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
 * This file implements operations regarding Scopes.
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment Scopes

#include "TypeRecord.h"
#include "ParseTree.h"
#include "SymTable.h"

SYMVia_t
LookUpTag(char *ident)
/*
 * Looks in the current block stack to find the tag with the given name.
 */
{
    SYMVia_t                        result;
    BlockStackVia_t                 tempstk;
    result = NULL;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Tags)
	    result = TableSearch(Via(Via(tempstk)->block)->Tags, ident);
	if (result) {
	    tempstk = NULL;
	} else {
	    tempstk = Via(tempstk)->next;
	}
    }
    return result;
}

SymListVia_t
CurrentSymbols(void)
{
    BlockStackVia_t                 tempstk;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Symbols)
	    return Via(Via(tempstk)->block)->Symbols;
	else
	    tempstk = Via(tempstk)->next;
    }
    return NULL;
}

SymListVia_t
CurrentTags(void)
{
    BlockStackVia_t                 tempstk;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Tags)
	    return Via(Via(tempstk)->block)->Tags;
	else
	    tempstk = Via(tempstk)->next;
    }
    return NULL;
}

SymListVia_t
CurrentLabels(void)
{
    BlockStackVia_t                 tempstk;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Labels)
	    return Via(Via(tempstk)->block)->Labels;
	else
	    tempstk = Via(tempstk)->next;
    }
    return NULL;
}

SymListVia_t
CurrentEnums(void)
{
    BlockStackVia_t                 tempstk;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Enums)
	    return Via(Via(tempstk)->block)->Enums;
	else
	    tempstk = Via(tempstk)->next;
    }
    return NULL;
}

SYMVia_t
LookUpSymbol(char *ident)
/*
 * Looks in the current block stack to find the symbol with the given name.
 */
{
    SYMVia_t                        result;
    BlockStackVia_t                 tempstk;
    result = NULL;
    tempstk = CurrentBlock;
    while (tempstk) {
	if (Via(Via(tempstk)->block)->Symbols)
	    result = TableSearch(
				    Via(Via(tempstk)->block)->Symbols,
				    (ident));
	if (result) {
	    tempstk = NULL;
	} else {
	    tempstk = Via(tempstk)->next;
	}
    }
    if (!result) {
	tempstk = CurrentBlock;
	while (tempstk) {
	    if (Via(Via(tempstk)->block)->Enums)
		result = TableSearch(Via(Via(tempstk)->block)->Enums,
				     (ident));
	    if (result) {
		tempstk = NULL;
	    } else {
		tempstk = Via(tempstk)->next;
	    }
	}
    }
    return result;
}

void
PushBlock(ScopesVia_t block)
/*
 * This routine pushes a block onto the block stack.  The block stack is used
 * for keeping track of scope.  The block argument is simply a pointer to the
 * parsetree node containing the compound statement currently being parsed.
 * CurrentBlock is the top of this stack,
 */
{
    BlockStackVia_t                 tempstk;
    if (!block) {
	VeryBadParseError("NULL table in PushBlock()");
    }
    tempstk = CurrentBlock;
    CurrentBlock = Ealloc(sizeof(BlockStack_t));
    Via(CurrentBlock)->block = block;
    Via(CurrentBlock)->next = tempstk;
}

void
PopBlock(void)
/* Pops the block stack. See PushBlock() above. */
{
    ScopesVia_t                     result;
    result = NULL;
    if (CurrentBlock) {
	BlockStackVia_t                 tempstk;
	tempstk = CurrentBlock;
	result = Via(CurrentBlock)->block;
	Efree(result);
	CurrentBlock = Via(CurrentBlock)->next;
	Efree(tempstk);
    } else {
	VeryBadParseError("Underflow on scope stack");
    }
}
