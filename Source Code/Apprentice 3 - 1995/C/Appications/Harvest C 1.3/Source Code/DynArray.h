/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file interfaces operations on dynamic arrays.
 * 
 */

#ifndef DynArray_INTERFACE
#define DynArray_INTERFACE

typedef struct DynArray         DynArray_t;
typedef DynArray_t P__H        *DynArrayVia_t;
typedef unsigned long           ObjID;
typedef void                   *ObjRef;

DynArrayVia_t
RawDynArray(unsigned long objsize, unsigned long startcapacity);


void
                                GrowDynArray(DynArrayVia_t da);


ObjID
AddObject(DynArrayVia_t da);



ObjRef
GetObject(DynArrayVia_t da, ObjID obj);


void
                                KillDynArray(DynArrayVia_t da);

#endif

