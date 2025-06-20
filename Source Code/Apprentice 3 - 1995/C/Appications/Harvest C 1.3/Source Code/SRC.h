/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for SRC records.
 * 
 */

#ifndef SRC_INTERFACE
#define SRC_INTERFACE

typedef struct SRCListS         SRCList_t;
typedef SRCList_t P__H         *SRCListVia_t;
typedef struct SRCS             SRCKind_t;
typedef SRCKind_t P__H         *SRCKindVia_t;

#include "SymTable.h"
#include "PPSymTable.h"

/* ------------------ SRC -------------------- */
/*
 * Below, are the data structure definitions for SRC.  A SRC is the location
 * of the C code that we are processing. In other words, this tells the IO
 * routines where to get the C source code.  The alternatives are a file or a
 * string. The RawCharacter routine knows enough to make the current SRC
 * transparent to the rest of the compiler.  This allows other routines the
 * ability to change the SRC of the program code easily. For example,
 * #include changes the SRC to another file, pushing the current file onto
 * the SRC stack.  When some SRC ends, the stack is popped.  When the stack
 * is empty, parsing terminates.
 */

union SRC {
    FILE *                io;
    EString_t                       mem;
};

#define SRC_SYSHEADER 1
#define SRC_USRHEADER 2
#define SRC_USRFILE   3

struct SRCS {
    int                             isIO;
    int								fileKind;
    union SRC                       where;
    char                            ExtraChar[MAXEXTRA];
    int                             NumExtras;
    int                             LineCount;
    long                            CharCount;
    short                           PreprocLineDirty;
    int                             eol;
    PPSYMVia_t                      Macro;
    EString_t                       NeedtoKill;
    int                             memindex;
    struct uniqstring P__H         *alreadyincluded;
    char                            fname[64];
    unsigned int                    CountCasts;
    unsigned int                    TotalIdentifierLength;
    unsigned int                    CountIdentifiers;
    unsigned int                    PreprocSubsts;
    unsigned int                    PreprocAfter;
    unsigned int                    PreprocBefore;
    unsigned int                    CountPreprocCondits;
    unsigned int                    StmtCount;
    unsigned int                    CountComments;
    unsigned int                    CommentBytes;
};

/*
 * the ExtraChar field above, gives the parser the ability to put back
 * characters if it decides it has read too many.  The RawCharacter routine
 * checks this buffer before returning a new one read from the current SRC.
 * Handy...
 */

struct SRCListS {
    SRCKindVia_t                    data;
    struct SRCListS P__H           *next;
};

#endif
