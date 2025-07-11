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
 * This file implements operations on abstract strings.
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"

#pragma segment AbsStrings

#include "AbsString.h"

/*
 * An abstract string is a reference to a string in a pool.  The pool is
 * simply a large relocatable region of memory which can be grown. A
 * reference to an individual string in the pool is simply the offset of that
 * string from the start of the pool.
 */

struct StringPool {
    EString_t						Pool;
    unsigned long                   PoolSize;
    unsigned long                   CountStrings;
    unsigned long                   TotalLength;
};

StringPoolVia_t
RawStringPool(unsigned long startsize)
{
    StringPoolVia_t                 raw;
    raw = Ealloc(sizeof(StringPool_t));
    if (raw) {
	Via(raw)->PoolSize = startsize;
	Via(raw)->CountStrings = 0;
	Via(raw)->TotalLength = 0;
	Via(raw)->Pool = Ealloc(startsize);
    }
    return raw;
}

void
GrowPool(StringPoolVia_t pool)
{
#ifdef OLDMEM
    SetHandleSize((Handle) Via(pool)->Pool, Via(pool)->PoolSize * 1.5);
#else
	char *newPool;
	newPool = (char *) icemalloc((long) (Via(pool)->PoolSize * 1.5));
	memcpy(newPool,Via(pool)->Pool,Via(pool)->PoolSize);
	icefree(Via(pool)->Pool);
	Via(pool)->Pool = (EString_t) newPool;
#endif
    Via(pool)->PoolSize *= 1.5;
}

AbsStringID
PutString(StringPoolVia_t pool, char *s)
{
    unsigned long                   slen;
    AbsStringID                     result;
    slen = strlen(s);
    if ((Via(pool)->TotalLength + slen) >= Via(pool)->PoolSize) {
	GrowPool(pool);
    }
    strcpy((char *) &(Via(Via(pool)->Pool)[result = Via(pool)->TotalLength]), s);
    Via(pool)->CountStrings++;
    Via(pool)->TotalLength += (slen + 1);
    return result;
}

void
GetAbsString(StringPoolVia_t pool, AbsStringID sid, char *s)
{
    strcpy(s, (char *) &(Via(Via(pool)->Pool)[sid]));
}

void
KillString(StringPoolVia_t pool, AbsStringID sid)
{
    Via(Via(pool)->Pool)[sid] = 0;
}

int
AbsStringCmp(StringPoolVia_t pool, AbsStringID sid, char *nm)
{
    return strcmp((char *) &(Via(Via(pool)->Pool)[sid]), nm);
}

int
AbsStringLen(StringPoolVia_t pool, AbsStringID sid)
{
    return strlen((char *) &(Via(Via(pool)->Pool)[sid]));
}

void
KillStringPool(StringPoolVia_t pool)
{
    Efree(Via(pool)->Pool);
    Efree(pool);
}
