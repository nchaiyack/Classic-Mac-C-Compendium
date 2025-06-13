/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains interfaces for error handling
 * 
 * 
 * 
 */

#ifndef Optimize_INTERFACE
#define Optimize_INTERFACE

typedef struct FoldValue_S      FoldValue_t;
typedef FoldValue_t P__H       *FoldValueVia_t;

#include "SymTable.h"
#include "ParseTree.h"

struct FoldValue_S {
    long                            intval;
    unsigned long                   uintval;
    long double                     realval;
    short                           isunsigned;
    short                           isint;
    short                           isstring;
    SYMVia_t                        thesymbol;
    short                           issymb;
    short                           isoffset;
    ParseTreeVia_t                  init;
    short                           isK;
};

#endif
