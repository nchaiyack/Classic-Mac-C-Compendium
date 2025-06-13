/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for type records.
 * 
 */

#ifndef Options_INTERFACE
#define Options_INTERFACE

/* ------------------ Compilation Options ------------------ */

/*
 * Below, the structure containing compilation options for ECC. This
 * structure will be modified as new options are added later.
 */

#include "Errors.h"

#define FRAME 6

struct OptionsS {
    int                             preproconly;
    int                             genC;
    int                             gen68k;
    /*
     * The above are really the four modes of operation.  Only 68k code
     * generation is useful for compilation.
     */
    int                             useTrigraphs;
    int                             int2byte;
    int                             Annotate;
    int                             progress;
    int                             _mc68020;
    int                             _mc68881;
    int                             metrics;
    int                             nocodegen;
    int                             SignedChars;
    int                             oopecc;
    short                           StdIncludeVol;
    long                            StdIncludeDir;
    short                           StdLibVol;
    long                            StdLibDir;
    short                           StdAppVol;
    long                            StdAppDir;
    short                           StdBuildVol;
    long                            StdBuildDir;
    char                            StdAppName[128];
    int                             GiveTime;
    long                            StdAppSig;
    int                             dolink;
    int                             PCrelSlits;
    int                             MPWoutput;
    int                             MBGSymbols;
    int                             allwarnings;
    int                             AregGlobals;
    int                             BigGlobals;
    int                             nowarnings;
    short                           Warnings[MAXWARNINGS];
};


#endif
