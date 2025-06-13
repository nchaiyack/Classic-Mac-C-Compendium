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
 * This file implements operations on the preprocessor symbol table.
 * 
 */

#include <Memory.h>
#include "structs.h"
#include "PPSymTable.h"
#include "AbsString.h"
#include "DynArray.h"

struct preprocsym {
    unsigned short                  SymbolFlags;
    PPSYMVia_t                      next;

    unsigned short                  ArgCount;
    SymListVia_t                    Args;

    EString_t                       PPValue;
    unsigned long                   nameID;
};

#define PPTABLESIZE 59

struct PPstablist {
    PPSYMVia_t                      table[PPTABLESIZE];
    short                           count;
};

/* Global variables */

PPSymListVia_t                  Defines;	/* the list of all symbols
						 * defined my the
						 * preprocessor.  This
						 * includes both simple
						 * #defines and macros. */

StringPoolVia_t                 PreprocStrings;
DynArrayVia_t                   PreprocArray;

/* Functions */

#ifdef OTHERHASH
unsigned long
PPTableHash(char *name)
/* This is the hash function for the symbol tables. */
{
    unsigned long                   h = 0;
    unsigned long                   g;

    for (; *name; ++name) {
	h = (h << 4) + *name;
	if (g = h & 0xf0000000)
	    h = (h ^ (g >> 24)) & 0x0fffffff;
    }
    return h % PPTABLESIZE;
}

#endif

unsigned long
PPTableHash(char *name)
/* This is the hash function for the symbol tables. */
{
    unsigned long                   h = 0;
    unsigned long                   g;

    for (; *name; ++name) {
	h = h * 65599 + *name;
    }
    return h % PPTABLESIZE;
}

PPSYM_t                        *
PP_ID2Ptr(PPSYMVia_t obj)
{
    return GetObject(PreprocArray, obj);
}

PPSYMVia_t
RawPPSymbol(char *name)
{
#ifdef Undefined
    register PPSYMVia_t             raw;
    raw = Ealloc(sizeof(PPSYM_t));
    Via(raw)->next = 0;
    Via(raw)->Args = 0;
    Via(raw)->SymbolFlags = 0;
    Via(raw)->PPValue = 0;
    Via(raw)->ArgCount = 0;
    Via(raw)->nameID = PutString(PreprocStrings, name);
    return raw;
#endif
    register PPSYMVia_t             raw;
    PPSYM_t                        *object;
    AbsStringID                     s;
    raw = AddObject(PreprocArray);
    s = PutString(PreprocStrings, name);
    object = PP_ID2Ptr(raw);
    object->nameID = s;
    return raw;
}

PPSymListVia_t
RawPPTable(void)
/* Create an empty, unused PP Symbol Table. */
{
    register PPSymListVia_t         raw;
    raw = Ealloc(sizeof(PPSymList_t));
    if (raw) {
    #ifdef OLDMEM
	HLock((Handle) raw);
	#endif
	memset((void *) Via(raw), 0, sizeof(PPSymList_t));
	#ifdef OLDMEM
	HUnlock((Handle) raw);
	#endif
    }
    Via(raw)->count = 0;
    return raw;
}

PPSYMVia_t
PPTableAdd(PPSymListVia_t table, char *name)
{
    /*
     * All the table manipulation routines may be modified later to make the
     * tables vastly more efficient.
     */
    register PPSYMVia_t             tmp;
    register unsigned long          hk;
    PPSYM_t                        *object;
    tmp = RawPPSymbol(name);
    hk = PPTableHash(name);
    assert(table);
    object = PP_ID2Ptr(tmp);
    object->next = Via(table)->table[hk];
    Via(table)->table[hk] = tmp;
    Via(table)->count++;
    return tmp;
}

PPSYMVia_t
PPTableSearch(PPSymListVia_t table, char *name)
/*
 * This routine searches a table for the given name, returning the symbol
 * record for that name if it was found, otherwise NULL.
 */
{
    register PPSYMVia_t             tmp;
    unsigned long                   hashval;
    hashval = PPTableHash(name);
    if (!Via(table)->count) return 0;
    tmp = Via(table)->table[hashval];
    while (tmp) {
	if (!AbsStringCmp(PreprocStrings, GetPPSymNameID(tmp), name)) {
	    return tmp;
	} else {
	    tmp = GetPPSymNext(tmp);
	}
    }
    return 0;
}

PPSYMVia_t
PPTableRemove(PPSymListVia_t table, PPSYMVia_t name)
/* Removes a given symbol from the table. */
{
    if (name) {
	KillString(PreprocStrings, GetPPSymNameID(name));
    }
    return 0;
}

#ifdef Undefined
void
FreePP(PPSYMVia_t cur)
{
    if (cur) {
	FreePP(Via(cur)->next);
	FreeSymbolList(Via(cur)->Args, 1);
	Efree(Via(cur)->PPValue);
	Efree(cur);
    }
}

#endif

void
FreePPSymbolList(PPSymListVia_t table)
{
    int                             ndx;
    if (table) {
#ifdef Undefined
	ndx = 0;
	while (ndx < PPTABLESIZE) {
	    FreePP(Via(table)->table[ndx++]);
	}
#endif
	Efree(table);
    }
    KillStringPool(PreprocStrings);
    KillDynArray(PreprocArray);
}

void
InitPPTable(void)
{
    Defines = RawPPTable();
    PreprocStrings = RawStringPool(1000);
    PreprocArray = RawDynArray(sizeof(PPSYM_t), 100);
}

void
SetPPSymFlags(PPSYMVia_t s, unsigned short f)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    obj->SymbolFlags = f;
}

unsigned short
GetPPSymFlags(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return obj->SymbolFlags;
}

void
SetPPSymValue(PPSYMVia_t s, EString_t v)
{
    PPSYM_t                        *obj;
    EString_t                       val;
    val = AllocString(v);
    obj = PP_ID2Ptr(s);
    assert(obj);
    if (v) {
	obj->PPValue = val;
    } else {
	obj->PPValue = 0;
    }
}

EString_t
GetPPSymValue(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return obj->PPValue;
}

void
SetPPSymArgCount(PPSYMVia_t s, int NumArgs)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    obj->ArgCount = NumArgs;
}

void
SetPPSymArgs(PPSYMVia_t s, SymListVia_t ParmNames)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    obj->Args = ParmNames;
}

int
GetPPSymArgCount(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return obj->ArgCount;
}

PPSYMVia_t
GetPPSymNext(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return obj->next;
}

unsigned long
GetPPSymNameID(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return obj->nameID;
}

void
GetPPSymName(PPSYMVia_t s, char *nm)
{
    GetAbsString(PreprocStrings, GetPPSymNameID(s), nm);
}

int
PPSymSearchArgNum(PPSYMVia_t s, char *ArgName)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    assert(ArgName);
    return TableSearchNum(obj->Args, ArgName);
}

int
GetPPSymValueLength(PPSYMVia_t s)
{
    PPSYM_t                        *obj;
    obj = PP_ID2Ptr(s);
    assert(obj);
    return strlen(Via(obj->PPValue));
}
