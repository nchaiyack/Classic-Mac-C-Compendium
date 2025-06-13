/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains all the data structure definitions for Harvest C.
 * Generally each structure is declared, and then its type is assigned to a
 * typedef.
 * 
 * 
 */

#include "Common.h"

typedef long                    Codigo_t;

struct uniqstring {
    struct uniqstring P__H         *next;
    unsigned long                   HashKey;
    char                            name[1];
};

/* ------------------ Keyword Hash Table -------------------- */

/*
 * Below, the data structure for the key word records, found in the hash
 * table.
 */

struct Keywordrecord {
    char                           *name;
    Codigo_t                        val;
    long                            uses;
};
typedef struct Keywordrecord    KW_t;
/*
 * ------------- Parse Buffer (putting back tokens) -------------
 */

/*
 * Below, are the data structures for the Parse Buffer.  This data structure
 * facilitates the parser's ability to "back up" if it has read too many
 * tokens (ie if it discovers that the tokens read do not match the current
 * rule.)  The Parse Buffer is an array. In GetToken() , if the buffer
 * contains a token that token is returned, instead of fetching a new one,
 * and the token is removed from the buffer.
 */

struct PBnodeS {
    Codigo_t                        tokval;
    long double                     floating;
    long                            integer;
    char                            token[MAXIDLENGTH];
};
typedef struct PBnodeS          PB_t;

struct InFile_S {
    char                            fname[MAXFILENAME];
    short                           volrefnum;
    long                            dirID;
    struct InFile_S P__H           *next;
};
typedef struct InFile_S         InFile_t;
typedef InFile_t P__H          *InFileVia_t;


/* Actually this struct is never used */
struct MemBuf_S {
    unsigned char P__H             *buf;
    unsigned long                   ndx;
};
typedef struct MemBuf_S         MemBuf_t;
typedef MemBuf_t P__H          *MemBufVia_t;

#include <assert.h>


#include "Param.h"
#include "TypeRecord.h"
#include "Lexer.h"
#include "LinkStruct.h"
#include "ParseTree.h"
#include "SymTable.h"
#include "PPSymTable.h"
#include "CodeGen.h"
#include "Scopes.h"
#include "OMF.h"
#include "GenericFile.h"
#include "SRC.h"
#include "Assem.h"
#include "Errors.h"
#include "Optimize.h"
#include "Options.h"

#include "Protos.h"

#include "Globals.h"
