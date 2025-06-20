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
 * This file implements operations on dynamic arrays.
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment DynArrays

#include "DynArray.h"


struct DynArray {
    EString_t						Objects;
    unsigned long                   ObjectSize;
    unsigned long                   ArrayObjCapacity;
    unsigned long                   CountObjects;
};

DynArrayVia_t
RawDynArray(unsigned long objsize, unsigned long startcapacity)
{
    DynArrayVia_t                   raw;
    raw = Ealloc(sizeof(DynArray_t));
    if (raw) {
	Via(raw)->Objects = Ealloc(objsize * startcapacity);
	Via(raw)->ObjectSize = objsize;
	Via(raw)->ArrayObjCapacity = startcapacity;
	Via(raw)->CountObjects = 1;	/* We start with 1 so 0 is invalid */
    }
    return raw;
}

void
GrowDynArray(DynArrayVia_t da)
{
#ifdef OLDMEM
    SetHandleSize((Handle) Via(da)->Objects, Via(da)->ObjectSize * Via(da)->ArrayObjCapacity * 1.5);
#else
	char *newMem;
	newMem = (char *) icemalloc((long) (Via(da)->ObjectSize * Via(da)->ArrayObjCapacity * 1.5));
	memcpy(newMem,Via(da)->Objects,Via(da)->ObjectSize * Via(da)->ArrayObjCapacity);
	icefree(Via(da)->Objects);
	Via(da)->Objects = newMem;
#endif
    Via(da)->ArrayObjCapacity *= 1.5;
}

ObjID
AddObject(DynArrayVia_t da)
{
    unsigned long                   result;
    if ((Via(da)->CountObjects + 5) >= Via(da)->ArrayObjCapacity) {
		GrowDynArray(da);
    }
    memset(&(Via(Via(da)->Objects)[result = (Via(da)->CountObjects) * Via(da)->ObjectSize]), 0L, Via(da)->ObjectSize);
    Via(da)->CountObjects++;
    return result;
}

ObjRef
GetObject(DynArrayVia_t da, ObjID obj)
{
    return &(Via(Via(da)->Objects)[obj]);
}

void
KillDynArray(DynArrayVia_t da)
{
    if (da) {
	Efree(Via(da)->Objects);
	Efree(da);
    }
}
