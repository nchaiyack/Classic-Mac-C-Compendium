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
 * This file contains routines for management of symbol tables.  Symbol tables
 * are used very heavily in Harvest C.  They manage everything from
 * preprocessor symbols to string literals, and of course, the C identifiers.
 * 
 * 
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment SymbolLists


SYMVia_t
RawSymbol(char *name)
{
    register SYMVia_t               raw;
    raw = Ealloc(sizeof(SYM_t) + strlen(name));
    if (raw) {
		Via(raw)->next = 0;
		Via(raw)->TP = 0;
		Via(raw)->SymbolFlags = 0;
		Via(raw)->storage_class = 0;
		Via(raw)->numbers.CountParams = 0;
		Via(raw)->Definition.FuncBody = 0;
		Via(raw)->M68kDef.Loc = 0;
		strcpy(Via(raw)->name, name);
    }
    return raw;
}

LabSYMVia_t
RawLabSymbol(char *name)
{
    register LabSYMVia_t            raw;
    raw = Ealloc(sizeof(LabSYM_t) + strlen(name));
    if (raw) {
	Via(raw)->left = NULL;
	Via(raw)->right = NULL;
	Via(raw)->SymbolFlags = 0;
	Via(raw)->Definition.superclass = NULL;
	Via(raw)->M68kDef.where = NULL;
	strcpy(Via(raw)->name, name);
    }
    return raw;
}

LabSymListVia_t
RawLabTable(void)
/* Create an empty, unused label Symbol Table. */
{
    register LabSymListVia_t        raw;
    raw = Ealloc(sizeof(LabSymList_t));
    if (raw) {
	Via(raw)->_head = NULL;
	Via(raw)->count = 0;
    }
    return raw;
}

/*
 * A symbol table, is a data structure which describes the definitions of
 * symbols.  As identifiers are declared (as meanings are given to symbols),
 * these identifiers (symbols) are inserted into the appropriate symbol
 * table. Any identifier declared twice in the same scope is an error, but
 * one identifier may have many different meanings in different scopes.
 * Different scopes will, in general, be represented by different symbol
 * tables. Generally, scope can be global, or local to some compound
 * statement.  The following name spaces do not conflict with each other:
 * 1(objects,functions,typedef names, and enum constants), 2(labels), 3(tags
 * of structures, unions, and enumerations), and 4(members of each structure
 * or union individually).  Scope of a declared identifier begins at the end
 * of its declarator, and proceeds until the end of the translation unit.
 */

SymImageVia_t
ListAdd(SymImageVia_t table, SYMVia_t thesym)
{
    SymImageVia_t                   added = NULL;
    if (thesym) {
	added = Ealloc(sizeof(SymImage_t));
	Via(added)->Symbol = thesym;
	Via(added)->next = table;
	if (table) {
	    Via(added)->count = Via(table)->count + 1;
	} else {
	    Via(added)->count = 1;
	}
    }
    return added;
}

LabSYMVia_t
LabTableAdd(LabSymListVia_t table, char *name)
/*
 * General routine for adding a symbol to a symbol table.  The symbol table
 * data structures are used for many purposes.  Note that currently, a number
 * of routines depend on these tables being kept in order.
 */
{
    /*
     * All the table manipulation routines may be modified later to make the
     * tables vastly more efficient.
     */
    register LabSYMVia_t            tmp;
    register LabSYMVia_t            tmp2, prev;
    register unsigned long          hk;
    tmp = RawLabSymbol(name);
    Via(tmp)->HashKey = hk = TableHash(name);
    if (!table) {
	return tmp;
    }
    Via(tmp)->IndexInserted = ++Via(table)->count;
    if (Via(table)->_head) {
	tmp2 = Via(table)->_head;
	while (tmp2) {
	    prev = tmp2;
	    if (Via(tmp2)->HashKey < hk) {
		tmp2 = Via(tmp2)->right;
	    } else {
		tmp2 = Via(tmp2)->left;
	    }
	}
	if (Via(prev)->HashKey < hk) {
	    Via(prev)->right = tmp;
	} else {
	    Via(prev)->left = tmp;
	}
    } else {
	Via(table)->_head = tmp;
    }
    return tmp;
}

void
GenOneSeg(LabSYMVia_t cursym, InstListVia_t Codes)
{
    if (!(Via(cursym)->SymbolFlags & CODEOUTPUT)) {
    	Via(cursym)->SymbolFlags |= CODEOUTPUT;
	GenInst(M68op_DEFSEG, M68sz_none, BuildLabelLoc(MakeSysLabel(Via(cursym)->name)), NULL, Codes);
    }
}

void GenSegmentsRecurse(LabSYMVia_t cur, InstListVia_t Codes)
{
	if (cur) {
		GenSegmentsRecurse(Via(cur)->left,Codes);
		GenOneSeg(cur,Codes);
		GenSegmentsRecurse(Via(cur)->right,Codes);
	}
}

void
GenSegments(InstListVia_t Codes)
{
	GenSegmentsRecurse(Via(SegmentNames)->_head,Codes);
}

LabSYMVia_t
LabTableTailAdd(LabSymListVia_t table, char *name)
/*
 * General routine for adding a symbol to a symbol table.  The symbol table
 * data structures are used for many purposes.  Note that currently, a number
 * of routines depend on these tables being kept in order.
 */
{
    return LabTableAdd(table, name);
}

void
LabTableIndexSearch(LabSYMVia_t root, int ndx, LabSYMVia_t * out)
{
    if (root) {
	if (Via(root)->IndexInserted == ndx)
	    *out = root;
	else {
	    if (!(*out)) {
		LabTableIndexSearch(Via(root)->left, ndx, out);
	    }
	    if (!(*out)) {
		LabTableIndexSearch(Via(root)->right, ndx, out);
	    }
	}
    }
}

LabSYMVia_t
LabTableGetNum(LabSymListVia_t table, int num)
/*
 * This table returns symbol record # num from the given table.  If the table
 * does not contains num records, then NULL.  The first record in a table is
 * numbered 1.
 */
{
    LabSYMVia_t                     tmp;
    if (num > Via(table)->count)
	return NULL;
    tmp = NULL;
    LabTableIndexSearch(Via(table)->_head, num, &tmp);
    return tmp;
}

LabSYMVia_t
LabTableSearch(LabSymListVia_t table, char *name)
/*
 * This routine searches a table for the given name, returning the symbol
 * record for that name if it was found, otherwise NULL.
 */
{
    register LabSYMVia_t            tmp;
    unsigned long                   hashval;
    hashval = TableHash(name);
    tmp = Via(table)->_head;
    while (tmp) {
	if (Via(tmp)->HashKey == hashval) {
	    if (!strcmp(Via(tmp)->name, name)) {
		return tmp;
	    } else {
		if (Via(tmp)->HashKey < hashval)
		    tmp = Via(tmp)->right;
		else
		    tmp = Via(tmp)->left;
	    }
	} else {
	    if (Via(tmp)->HashKey < hashval)
		tmp = Via(tmp)->right;
	    else
		tmp = Via(tmp)->left;
	}
    }
    return NULL;
}
