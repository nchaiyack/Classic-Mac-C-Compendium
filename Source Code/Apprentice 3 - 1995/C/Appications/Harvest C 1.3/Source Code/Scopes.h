/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for scoping.
 * 
 */

#ifndef Scopes_INTERFACE
#define Scopes_INTERFACE

typedef struct scopeS           Scopes_t;
typedef Scopes_t P__H          *ScopesVia_t;
typedef struct BlockStack_S     BlockStack_t;
typedef BlockStack_t P__H      *BlockStackVia_t;

#include "SymTable.h"

struct scopeS {
    SymListVia_t                    Tags;
    SymListVia_t                    Labels;
    SymListVia_t                    Symbols;
    SymListVia_t                    Enums;
};

struct BlockStack_S {
    ScopesVia_t                     block;
    BlockStackVia_t                 next;
};

#endif
