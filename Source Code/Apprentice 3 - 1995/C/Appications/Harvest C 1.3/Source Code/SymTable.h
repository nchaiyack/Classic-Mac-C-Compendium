/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for the symbol tables.
 * 
 */

#ifndef SymTable_INTERFACE
#define SymTable_INTERFACE

#include "Common.h"

typedef struct stabS            SYM_t;
typedef struct labelsym         LabSYM_t;
typedef struct stablist         SymList_t;
typedef struct labstablist      LabSymList_t;
typedef SYM_t P__H             *SYMVia_t;
typedef SymList_t P__H         *SymListVia_t;
typedef LabSYM_t P__H          *LabSYMVia_t;
typedef LabSymList_t P__H      *LabSymListVia_t;
typedef struct SymTableStack_S  SymTableStack_t;
typedef SymTableStack_t P__H   *SymTableStackVia_t;
typedef struct SymImage_S       SymImage_t;
typedef SymImage_t P__H        *SymImageVia_t;

#include "TypeRecord.h"
#include "ParseTree.h"
#include "CodeGen.h"

/* ------------------ Symbol Tables ------------------ */

/*
 * Below, the structure declaration for symbol records.  TODO This structure
 * needs a full explanation here, when one is available. Note : this
 * structure is the storage record for SYMBOLS.  All kinds of symbols may go
 * here, but it is not a storage record for the parse tree.  The parse tree
 * format follows...
 */

union intnumbers {
    int                             CountParams;	/* Symbols : macro funcs */
    int                             EnumVal;	/* Symbols : enum const value */
    int                             structoffset;	/* Symbols : struct
							 * members */
    int                             CountUses;	/* Symbols : number of uses. */
};

/* Mask constants for types. */
#define ISINLINEMASK 1
#define YADUMPEDMASK 2
#define INCOMPLETEMASK 4
#define ISPASCALMASK 8
#define ISACLASS 16
#define SIZEDONE 32
#define SIGNKNOWN 64
#define SIGNEDTYPE 128

/* Mask constants for symbols. */
#define BEINGEXPANDED 1
#define CODEOUTPUT 2
#define PREPROCLOCKED 4
#define PREPROCUNDEF 8
#define REFOUTPUT 16
#define ISTRAPMASK 32
#define ISPASCALSTRING 64
#define ISINSTANCEVAR 128
#define ISPUBLIC 256
#define HASPARAM 512
#define DEADPPSYM 1024

/*
 * This structure is used for symbol tables of all kinds.  This obnoxious
 * size of this structure is one of my big motivations for implementing OO
 * stuff in Ecc, so I can create a class for symbol lists, and subclass it
 * for each kind of symbol list that I need
 */

struct stablist {		/* This is a standard symbol table. */
    SYMVia_t P__H                  *hashtable;
    short                           count;
    short                           tablesize;
    short                           isShadow;
};

struct stabS {
    unsigned short                  IndexInserted;
    unsigned short                  SymbolFlags;
    SYMVia_t                        next;
    TypeRecordVia_t                 TP;
    enum StorageClassCode           storage_class;

    union intnumbers                numbers;

    union Trees {
	SymListVia_t                    superclass;
	ParseTreeVia_t                  FuncBody;	/* For functions */
	ParseTreeVia_t                  Initializer;	/* For data objects */
	ParseTreeVia_t                  Stmt;	/* For C labels */
	ParseTreeVia_t                  CaseConstant;
	EString_t                       PreProcValue;	/* Macro expansion */
	struct TwoShorts {
	    short                           StartBit;
	    short                           EndBit;
	}                               StartEndBits;	/* For bit fields */
    }                               Definition;
    union M68kDef_U {
	InstVia_t                       where;	/* For labels */
	LocAMVia_t                      Loc;	/* For data & functions */
    }                               M68kDef;
    char                            name[1];	/* The NewHandle will ensure
						 * that enough space is
						 * allocated for the name to
						 * be stored here */
};

struct labelsym {
    short                           IndexInserted;
    unsigned long                   HashKey;
    unsigned short                  SymbolFlags;
    LabSYMVia_t                     left;
    LabSYMVia_t                     right;

    union Trees                     Definition;
    union M68kDef_U                 M68kDef;
    char                            name[1];
};

struct labstablist {
    LabSYMVia_t                     _head;
    short                           count;
};

struct SymTableStack_S {
    SymList_t P__H                 *table;
    struct SymTableStack_S P__H    *next;
};

struct SymImage_S {
    SYMVia_t                        Symbol;
    struct SymImage_S P__H         *next;
    short                           count;
};

TypeRecordVia_t
GetSymTP(SYMVia_t s);

void SetSymTP(SYMVia_t s, TypeRecordVia_t TP);
void GetSymName(SYMVia_t s, char *nm);
SYMVia_t TableSearch(SymListVia_t,char *);

#endif
