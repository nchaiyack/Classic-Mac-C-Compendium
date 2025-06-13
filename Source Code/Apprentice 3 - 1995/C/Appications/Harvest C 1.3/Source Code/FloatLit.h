/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for FloatLit records.
 * 
 */

#ifndef FloatLit_INTERFACE
#define FloatLit_INTERFACE

typedef struct FloatLit_S       FloatLit_t;
typedef FloatLit_t P__H        *FloatLitVia_t;

#include "CodeGen.h"

struct FloatLit_S {
    FloatLitVia_t                   next;

    LocAMVia_t                      Loc;
    unsigned short                  FloatFlags;
    union floatdata {
	long double                     num;
	char                            bytes[12];
    }                               val;
};

#endif
