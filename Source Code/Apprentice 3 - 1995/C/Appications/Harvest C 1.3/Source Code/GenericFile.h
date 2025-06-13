/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for GenericFile records.
 * 
 */

#ifndef GenericFile_INTERFACE
#define GenericFile_INTERFACE

typedef struct GenericFile_S    GenericFile_t;
typedef GenericFile_t P__H     *GenericFileVia_t;

#include "Common.h"
#include <stdio.h>

struct GenericFile_S {
    char                            name[MAXFILENAME];
    short                           volrefnum;
    short                           errval;
    short                           endfound;
    long                            dirID;
    short                           refnum;
    void                           *theWind;	/* TODO Should be WindowPtr */
    FILE                           *nonmac;
    long                            fpos;
    int                             perm;

    EString_t						buffer;
    unsigned long                   buffersize;
    unsigned long                   bufferindex;
    unsigned long                   Fullness;
};

#endif
