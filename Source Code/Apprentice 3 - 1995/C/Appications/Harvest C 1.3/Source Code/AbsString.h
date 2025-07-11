/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file interfaces operations on abstract strings.
 * 
 */

#ifndef AbsString_INTERFACE
#define AbsString_INTERFACE

#include "conditcomp.h"

typedef unsigned long           AbsStringID;
typedef struct StringPool       StringPool_t;
typedef StringPool_t P__H      *StringPoolVia_t;

StringPoolVia_t                 RawStringPool(unsigned long startsize);
void                            GrowPool(StringPoolVia_t pool);
AbsStringID                     PutString(StringPoolVia_t pool, char *s);
void                            GetAbsString(StringPoolVia_t pool, AbsStringID sid, char *s);
int                             AbsStringLen(StringPoolVia_t pool, AbsStringID sid);
void                            KillStringPool(StringPoolVia_t pool);
void
                                KillString(StringPoolVia_t pool, AbsStringID sid);

int
                                AbsStringCmp(StringPoolVia_t pool, AbsStringID sid, char *nm);

#endif
