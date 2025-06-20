/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file provides interface for operations on the preprocessor symbol table.
 * 
 */

#ifndef PPSymTable_INTERFACE
#define PPSymTable_INTERFACE

typedef struct preprocsym       PPSYM_t;
typedef struct PPstablist       PPSymList_t;
typedef unsigned long           PPSYMVia_t;
typedef PPSymList_t P__H       *PPSymListVia_t;

/* Functions */

PPSYMVia_t
RawPPSymbol(char *name);

PPSymListVia_t
RawPPTable(void);

PPSYMVia_t
PPTableAdd(PPSymListVia_t table, char *name);

int
                                PPTableSearchNum(PPSymListVia_t table, char *name);

PPSYMVia_t
PPTableSearch(PPSymListVia_t table, char *name);

PPSYMVia_t
PPTableRemove(PPSymListVia_t table, PPSYMVia_t name);

void
                                FreePP(PPSYMVia_t cur);

void
                                FreePPSymbolList(PPSymListVia_t table);

void
                                SetPPSymFlags(PPSYMVia_t s, unsigned short f);

unsigned short
                                GetPPSymFlags(PPSYMVia_t s);

void
                                SetPPSymValue(PPSYMVia_t s, EString_t v);

EString_t
GetPPSymValue(PPSYMVia_t s);

void
                                SetPPSymArgCount(PPSYMVia_t s, int NumArgs);

void
                                SetPPSymArgs(PPSYMVia_t s, SymListVia_t ParmNames);

int
                                GetPPSymArgCount(PPSYMVia_t s);

void
                                PPLock(PPSYMVia_t s);

void
                                PPUnlock(PPSYMVia_t s);

void
                                GetPPSymName(PPSYMVia_t s, char *);

char                            GetPPSymValChar(PPSYMVia_t s, int ndx);

int
                                PPSymSearchArgNum(PPSYMVia_t s, char *ArgName);

int
                                GetPPSymValueLength(PPSYMVia_t s);

PPSYMVia_t
GetPPSymNext(PPSYMVia_t s);

unsigned long
                                GetPPSymNameID(PPSYMVia_t s);

#endif
