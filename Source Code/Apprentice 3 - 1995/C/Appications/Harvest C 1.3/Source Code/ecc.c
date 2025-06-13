/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This is the principal file for Harvest C. It includes all output routines,
 * memory management, as well as management of compiler options.
 * 
 * 
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "structs.h"
#include "CHarvestDoc.h"
#include "CHarvestOptions.h"

extern CHarvestDoc *gProject;
FILE *fopenMAC(char *name,short vRefNum,long dirID,char *mode);

#pragma segment HarvestMain

/* Modules */
#include "TypeRecord.h"

int                             PartDone;

LabSYMVia_t                             CurrentSegmentLabel;
LabSYMVia_t                             NextFuncSegment;

short                           CurVolrefnum;
long                            CurDirID;

ParamRecVia_t                   ParamList;

int                             ExtraBlocks;	/* used primarily in
						 * parsestmt.c, for keeping
						 * track of scopes */
char                            Errm[MAXERRORMSG];	/* used as a buffer for
							 * constructing error
							 * messages */

unsigned int                    CountIncludes;

LabSymListVia_t                 SwitchCases;
#define MAXSWITCHES 64
LabSymListVia_t                 Switches[MAXSWITCHES];
int                             CountSwitch;

#ifdef OOPECC
SymListVia_t                    ClassList;	/* global list of classes */
SymListVia_t                    MessageList;	/* global list of messages */
#endif
LabSymListVia_t                 Labels;	/* list of asm labels */
SymListVia_t                    StringLits;	/* String literal pool */
SymListVia_t                    TagsTable;	/* Struct/union/enum tags */
SymListVia_t                    LabelsTable;	/* C labels */
/*
 * The next variables are used while constructing struct declarations
 */
int                             CurFieldOffset;	/* offset of current bit
						 * field */
int                             NextStructOffset;	/* offset of next member */
int                             CurFieldBit;
int                             CurStructSize;
ParseTreeVia_t                  LastSwitchExpression;	/* used by the parser to
							 * remember the
							 * expression in the
							 * last switch(expr)
							 * statement, so we can
							 * typecheck the case
							 * labels against that
							 * expression */
SymListVia_t                    StaticTable;	/* global list of static
						 * variables (static within
						 * functions) */
SymListVia_t                    GlobalSymbolTable;	/* All external symbols
							 * are defined here. */
SymListVia_t                    TP_defnames;	/* All TP_defnames are
						 * defined here. */
LabSymListVia_t                    SegmentNames;	/* All segment names are
						 * stored here. */
int                             InPreprocIf;	/* a flag which indicates to
						 * the parser whether or not
						 * we are parsing the
						 * expression following an
						 * #if or #elif directive */
/*
 * A SRC is the logical source of the C program text being lexed/parsed.  It
 * may be a region of memory or a file
 */

SRCListVia_t                    SRCStack;	/* the SRC stack */
SRCKind_t                       CurrentSRC;	/* the current SRC for C code */

KW_t                            KWTable[KEYWORDTABLESIZE];	/* The KWTable is a hash
								 * table for C reserved
								 * words.  Since
								 * virtually every token
								 * must be checked
								 * against the list of
								 * reserved word, it is
								 * worthwhile to make
								 * this search
								 * efficient. */
TypeRecordVia_t                 ClassBeingImplemented = 0;	/* during a class
								 * @implementation, this
								 * is the type record
								 * for that class. When
								 * referencing an
								 * instance variable,
								 * self is cast to this
								 * type before
								 * synthesizing the
								 * indirect member tree
								 * node. */
long                            CurrentEnumK;	/* The value of the current
						 * enum constant. Used during
						 * construction of an enum
						 * declaration. */
TypeRecordVia_t                 EnumBeingDeclared;	/* During an enum
							 * declaration, this
							 * points to the type
							 * record of the enum
							 * being declared.  Each
							 * enum constant bears a
							 * pointer to its own
							 * type. */
SYMVia_t                        FunctionBeingDefined;	/* During a function
							 * definition, this
							 * points to the symbol
							 * being defined.  */
SYMVia_t                        InliningFunction;	/* While inlining a
							 * function, this points
							 * to the function begin
							 * inlined */

struct uniqstring P__H         *HalUniqs = NULL;	/* a table of unique
							 * identifiers, used for
							 * calculating Halstead
							 * metrics */
/*
 * All of the following variables are used in calculating various metrics
 */
unsigned int                    T_PreprocSubsts = 0;
unsigned int                    T_PreprocBefore = 0;
unsigned int                    T_PreprocAfter = 0;
unsigned int                    T_CommentBytes = 0;
unsigned int                    T_StmtCount = 0;
unsigned int                    T_CountIdentifiers = 0;
unsigned int                    T_TotalIdentifierLength = 0;
unsigned int                    T_CountCasts = 0;
unsigned int                    T_CountPreprocCondits = 0;
unsigned int                    T_CountComments = 0;
int                             FunctionReturnCount = 0;
int                             FunctionComplexity = 0;
int                             HalTotalOperands = 0;
int                             HalTotalOperators = 0;
int                             HalUniqOperators = 0;
int                             HalUniqOperands = 0;
int                             PreprocSymbolCount = 0;
int                             DuplicateStringLits = 0;
int                             TotalLines = 0;
int                             CountFunctions = 0;

int                             SumGlobals;
int                             SumGlobalSpace;
LocAMVia_t                      StructDoubleReturnLoc;	/* During code
							 * generation, this
							 * points to the place
							 * to return a struct.
							 * doubles are now
							 * returned in d0-d1 */
/* These keep track of the efficiency of the keyword hash table. */
int                             HashCollisions;
int                             HashCount;

int                             OddGlobal;	/* true if an odd number of
						 * bytes have been generated
						 * for global data.
						 * depending on the next
						 * global variable, an
						 * alignment directive may be
						 * needed */
int                             DiscardedFunctionCall;	/* a flag indicating to
							 * the code generator
							 * that the result of
							 * the function call
							 * being generated is
							 * not needed. for
							 * example, extern int
							 * x(void); x(); */

StringPoolVia_t                 ParserStrings;

char                            LastPreproc[64];	/* records the name of
							 * the last preproc
							 * directive used */

int                             TokenOnDeck;	/* flag indicating if a
						 * single token has been read
						 * and then putback. Such a
						 * token is on deck and is
						 * easily available to the
						 * parser in NextIs() */
char                            NextToken[MAXIDLENGTH];	/* tokens on deck are
							 * stored here */
Codigo_t                        NextTokenKind;	/* tokens on deck are stored
						 * here */

char                           *LastToken;	/* A very important variable.
						 * This contains the string
						 * of the last token read. */
int                             SizeOfFloat;	/* size of float type in
						 * bytes */
int                             SizeOfDouble;
int                             SizeOfLongDouble;
Codigo_t                        LastTokenKind;	/* code for the last token
						 * read */
long double                     LastFloatingConstant;	/* value of the last
							 * floating point
							 * constant read -
							 * probably only valid
							 * if the last token
							 * really was a floating
							 * point constant */
long                            LastIntegerConstant;
char                            ThisToken[MAXIDLENGTH];	/* tokens are
							 * constructed here
							 * during lexing */
char                            LastChar;	/* The last character read
						 * which contributed to a
						 * token.  TODO - is this
						 * correct or is this the
						 * last character returned
						 * from RawCharacter() ? */

GenericFileVia_t                outfile;	/* output file for assembler
						 * source, etc */
GenericFileVia_t                errfile;	/* error file */

int                             NumErrors;	/* Cumulative count of
						 * errors. */
int                             AttemptWarnings;	/* Cumulative count of
							 * warnings. */
int                             NumWarnings;	/* Cumulative count of
						 * warnings. */


/*
 * The Preprocessor uses the variable PPStatus to signal to the parser that
 * tokens are to be ignored, due to the a conditional compilation directive.
 * However, since these directives may be nested, there is a stack to keep
 * old values of PPStatus.  This stack is implemented as an array of int and
 * PPSP is the stack pointer.
 */
int                             PreprocStack[MAXNESTIFDEF];
int                             PPSP;
int                             PPStatus;
/*
 * The following are used for macro definition by the preprocessor.
 * MacroValue is used to temporarily store the value of the macro being
 * defined. The 'value' of a a macro, as I define it, is the string used in
 * expanding it.  In other words, it is the part just to the right of the id
 * and arguments : #define macro(a,b,c) VALUE
 */
EString_t                       MacroValue;

BlockStackVia_t                 CurrentBlock;	/* used in keeping track of
						 * scopes. This points to the
						 * top of the scope stack.
						 * When looking up the
						 * meaning of some identifer,
						 * during parsing, the search
						 * starts in the symbol
						 * tables of the top scope,
						 * and proceeds downward
						 * until it is found or the
						 * end of the stack is
						 * reached. */
LabSYMVia_t                     FunctionLastLabel;	/* during code
							 * generation, this is
							 * the number of the
							 * label which will
							 * occur at the end of
							 * the function. a
							 * return statement is
							 * compiled with a jump
							 * to this label */
FloatLitVia_t                   FloatList;	/* floating point literal
						 * pool */
int                             NextFloatLitLabel;	/* number of the next
							 * label for floating
							 * point literals */
int                             NextLitLabel;	/* number of the next label
						 * for string literals */
int                             NextEccLabel;	/* number of the next general
						 * Ecc generated label */
/*
 * used by the register allocator, these next three variables keep track of
 * which register is the next candidate for allocation. Register allocation
 * is done on the fly
 */
int                             NextTempFloatReg;
int                             NextTempDataReg;
int                             NextTempAddrReg;
/*
 * these arrays are part of the register allocator, and contain data
 * structures representing the machine registers.
 */
RegisterRecord_t                TempFloats[8];
RegisterRecord_t                TempDatas[8];
RegisterRecord_t                TempAddrs[8];
/*
 * used by the register allocator and code generator, this is a list of spill
 * slots
 */
SpillSlotVia_t                  SpillList;
LabSYMVia_t                     BreakLabel;	/* The current break label.
						 * A break statement is
						 * simply compiled as a jump
						 * to this label.  This label
						 * is set in the routine that
						 * generates code for a loop
						 * of some kind */
LabSYMVia_t                     ContinueLabel;	/* similar to BreakLabel */
InstListVia_t                   GlobalCodes;	/* very important - this is
						 * the list of 68k
						 * instruction records into
						 * which code is generated.
						 * Harvest C currently uses
						 * no intermediate
						 * representation for
						 * generated code. The parser
						 * creates a tree
						 * representation of C
						 * declarations and
						 * expressions.  The code
						 * generator converts this
						 * tree into data structures
						 * representing 68000 machine
						 * instructions and assembler
						 * directives, storing them
						 * in this list.  This list
						 * is then either dumped as
						 * assembly source or as an
						 * MPW linker file. */
InstVia_t                       LinkInst;	/* The pointer to the
						 * instruction record for the
						 * link instruction for the
						 * function being compiled.
						 * The constant contained
						 * herein may be modified
						 * during compilation to
						 * allocate more space on the
						 * stack */
EString_t                       InitDataAcc;	/* used for accumulating data
						 * for an initializer */
int                             InitDataAccIndex;	/* index used for
							 * accumulating
							 * initializer data */
int                             InitDataAccSize;


EString_t
MakeEString(char *s)
{
#ifdef OLDMEM
    static char                    *holdit;
    holdit = s;
    return &holdit;
#else
	return s;
#endif
}

MemReq_t
Ealloc(unsigned long size)
{
#ifdef OLDMEM
    MemReq_t                        tmp;
    tmp = (MemReq_t) NewHandle(size);
    return tmp;
#else
	return (MemReq_t) icemalloc(size);
#endif
}

void
Efree(MemReq_t mem)
{
    if (!mem)
		return;
#ifdef OLDMEM
    DisposHandle(mem);
#else
    icefree(mem);
#endif
}

EString_t
AllocString(EString_t tmpstring)
{
    /*
     * Moves the string argument into dynamically allocated memory, and
     * returns a pointer to that string.  Basically, returns a copy of its
     * arg.
     */
    EString_t                       copy;
    copy = NULL;
    if (tmpstring) {
	/*
	 * Danger! Should we hlock tmpstring ?  If so, how do we know that it
	 * is a Memory Manager allocated handle ??
	 */
	copy = Ealloc(1 + strlen(Via(tmpstring)));
#ifdef OLDMEM
	HLock((Handle) copy);
#endif
	strcpy(Via(copy), Via(tmpstring));
#ifdef OLDMEM
	HUnlock((Handle) copy);
#endif
    }
    return copy;
}

static SysEnvRec                theWorld;

GenericFileVia_t
OpenGenericFile(char *name, short volrefnum, long dirID, char *mode, int iswind)
/* name must be a C string not a Pascal string */
{
    GenericFileVia_t                result = NULL;
    extern GenericFileVia_t         RawGenericFile(void);
    char                            pname[64];
    int                             bad;
    short                           refnum;
    strcpy(pname, name);
    c2pstr(pname);
    if (iswind) {
	result = RawGenericFile();
    } else {
	if (!strcmp(mode, "w")) {
	    bad = HDelete(volrefnum, dirID, pname);
	    bad = HCreate(volrefnum, dirID, pname, 'Jn15', '????');
	    UserFileError(bad);
	    bad = HOpen(volrefnum, dirID, pname, 2, &refnum);
	    UserFileError(bad);
	} else {
	    bad = HOpen(volrefnum, dirID, pname, 1, &refnum);
	    if (!bad) {
		result = RawGenericFile();
		strcpy(Via(result)->name, name);
		c2pstr(Via(result)->name);
		Via(result)->volrefnum = volrefnum;
		Via(result)->endfound = 0;
		Via(result)->dirID = dirID;
		Via(result)->refnum = refnum;
		Via(result)->buffersize = 10000;
		Via(result)->buffer = Ealloc(Via(result)->buffersize);
		Via(result)->bufferindex = 0;
		Via(result)->errval = 0;
		FillGFileBuffer(result);
		if (!strcmp(mode, "w"))
		    Via(result)->perm = 2;
		else
		    Via(result)->perm = 1;
	    } else
		return NULL;
	}
    }
    return result;
}

int
OpenInput(char *filename, int volrefnum, long dirID)
{
    FILE *                thefile;
    thefile = fopenMAC(filename, volrefnum, dirID, "r");
    if (thefile) {
	strcpy(CurrentSRC.fname, filename);
	CurrentSRC.where.io = thefile;
	CurrentSRC.isIO = 1;
	CurrentSRC.LineCount = 1;
	CurrentSRC.memindex = 0;
	CurrentSRC.PreprocLineDirty = 0;
	CurrentSRC.alreadyincluded = NULL;
	CurrentSRC.eol = 0;
	CurrentSRC.NeedtoKill = NULL;
	CurrentSRC.NumExtras = 0;
	return 1;
    } else {
	FatalError2("File not found: ", filename);
	return 0;
    }
}

int
OpenError(char *filename, int volrefnum, long dirID)
{
    char                            tmpname[128];
    int                             fndx;
    fndx = 0;
    strcpy(tmpname, filename);
    while ((tmpname[fndx] != '.') && (tmpname[fndx])) {
	fndx++;
    }
    tmpname[fndx] = 0;
    strcat(tmpname, ".err");
    errfile = OpenGenericFile(tmpname, volrefnum, dirID, "w", 1);
    if (!errfile) {
	FatalError("Couldn't open error file");
	return 0;
    }
    return 1;
}

int
OpenOutput(char *filename, int volrefnum, long dirID)
{
	outfile = NULL;
	return 1;
}

void
PushEnumTable(SymListVia_t syms)
{
    ScopesVia_t                     tempblock;
    tempblock = Ealloc(sizeof(Scopes_t));
    Via(tempblock)->Tags = NULL;
    Via(tempblock)->Symbols = NULL;
    Via(tempblock)->Labels = NULL;
    Via(tempblock)->Enums = syms;
    PushBlock(tempblock);
    ExtraBlocks++;
}

void
PushSpaces(SymListVia_t syms, SymListVia_t tags, SymListVia_t labs)
{
    /* A short hand for building a block and then pushing it */
    ScopesVia_t                     tempblock;
    tempblock = Ealloc(sizeof(Scopes_t));
    Via(tempblock)->Tags = tags;
    Via(tempblock)->Symbols = syms;
    Via(tempblock)->Labels = labs;
    Via(tempblock)->Enums = NULL;
    PushBlock(tempblock);
}

GenericFileVia_t
RawGenericFile(void)
{
    GenericFileVia_t                result;
    result = Ealloc(sizeof(GenericFile_t));
    Via(result)->theWind = NULL;
    Via(result)->nonmac = NULL;
    Via(result)->refnum = 0;
    Via(result)->Fullness = 0;
    Via(result)->buffersize = 0;
    Via(result)->bufferindex = 0;
    Via(result)->endfound = 0;
    Via(result)->volrefnum = 0;
    Via(result)->dirID = 0;
    Via(result)->fpos = 0;
    Via(result)->perm = 0;
    Via(result)->buffer = NULL;
    return result;
}

void
PutFileOnHold(FILE * f)
{
#ifdef Undefined
    long                            fpos;
    int                             bad;
    if (f) {
	if (Via(f)->refnum) {
	    bad = GetFPos(Via(f)->refnum, &fpos);
	    UserFileError(bad);
	    Via(f)->fpos = fpos;
	    bad = FSClose(Via(f)->refnum);
	    UserFileError(bad);
	}
    }
#endif
}

void
TakeFileOffHold(FILE * f)
{
#ifdef Undefined
    int                             bad;
    short                           refnum;
    if (f) {
	if (Via(f)->refnum) {
	    bad = HOpen(Via(f)->volrefnum, Via(f)->dirID, Via(f)->name, Via(f)->perm, &refnum);
	    UserFileError(bad);
	    Via(f)->refnum = refnum;
	    bad = SetFPos(refnum, 1, Via(f)->fpos);
	    UserFileError(bad);
	}
    }
#endif
}

void
CloseGenericFile(GenericFileVia_t f)
{
    OSErr                           bad;
    if (f) {
	if (Via(f)->theWind) {
	    /* Doesn't really close the window */
	} else {
	    if (Via(f)->refnum) {
		bad = FSClose(Via(f)->refnum);
		UserFileError(bad);
	    }
	}
	if (Via(f)->buffer)
	    Efree(Via(f)->buffer);
	Efree(f);
    }
}

long
FillGFileBuffer(GenericFileVia_t thef)
/* For input files, this fills the buffer by reading the file */
{
    long                            count;
#ifdef OLDMEM
    HLock((Handle) Via(thef)->buffer);
#endif
    count = Via(thef)->buffersize;
    Via(thef)->errval = FSRead(Via(thef)->refnum, &count, (char *) Via(Via(thef)->buffer));
    if (Via(thef)->errval == -39) {
	FSClose(Via(thef)->refnum);
	Via(thef)->refnum = 0;
	Via(thef)->errval = 0;
    }
    Via(thef)->Fullness = count;
    Via(thef)->bufferindex = 0;
#ifdef OLDMEM
    HUnlock((Handle) Via(thef)->buffer);
#endif    
    return count;
}

int
GetCGenericFile(GenericFileVia_t f)
{
    int                             c;
    if (Via(f)->endfound)
	return EOF;
    if (Via(f)->bufferindex >= Via(f)->Fullness)
	FillGFileBuffer(f);
    if (!Via(f)->Fullness)
	Via(f)->errval = -1;
    c = Via(Via(f)->buffer)[Via(f)->bufferindex++];
    if (Via(f)->errval) {
	c = ' ';
	Via(f)->endfound = 1;
    }
    return c;
}

void
GenericGetLine(GenericFileVia_t thef, char *buf)
{
    short                           c;
    int                             done = 0;
    do {
	c = GetCGenericFile(thef);
	switch (c) {
	case EOF:
	case '\n':
	    *buf = 0;
	    done = 1;
	    break;
	default:
	    *buf = c;
	    buf++;
	}
    } while (!done);
}

void
EachFileInit(void)
{
    InitTypes();
    InitPPTable();
    DiscardedFunctionCall = 0;
    ParserStrings = RawStringPool(1000);
    InitLocAM();
    GlobalCodes = RawInstructionList();
    CurrentBlock = NULL;
    TP_defnames = RawTable(113);
    SegmentNames = RawLabTable();
    CurrentSegmentLabel = NextFuncSegment = LabTableAdd(SegmentNames, "Main");
    gAllDone = 0;
    gAbortCompile = 0;
    PPSP = 0;
    PPStatus = 0;
    NumErrors = 0;
    NumWarnings = 0;
    AttemptWarnings = 0;
    InitDataAccIndex = 0;
    InitDataAccSize = 0;
    TokenOnDeck = 0;
    SumGlobals = 0;
    SumGlobalSpace = 0;
    CountFunctions = 0;
    CountSwitch = 0;
    CountIncludes = 0;

    ExtraBlocks = 0;
    HalUniqs = NULL;

    T_CountPreprocCondits = 0;
    T_PreprocSubsts = 0;
    T_PreprocBefore = 0;
    T_PreprocAfter = 0;
    T_CommentBytes = 0;
    T_StmtCount = 0;
    T_CountIdentifiers = 0;
    T_TotalIdentifierLength = 0;
    T_CountCasts = 0;
    T_CountComments = 0;

    HalTotalOperands = 0;
    HalTotalOperators = 0;
    HalUniqOperands = 0;
    HalUniqOperators = 0;
    PreprocSymbolCount = 0;
    InitDataAcc = (EString_t) Ealloc(MAXINITDATA);

    OddGlobal = 0;
    DuplicateStringLits = 0;
    SRCStack = NULL;
    FloatList = NULL;
    LinkInst = NULL;
    LastSwitchExpression = NULL;
    NextLitLabel = 1;
    NextEccLabel = 1;
    FunctionLastLabel = NULL;
    ParamList = NULL;
    SpillList = NULL;
    outfile = NULL;
    HashCollisions = 0;
    TotalLines = 0;
    InPreprocIf = 0;
    EnumBeingDeclared = 0;
    FunctionBeingDefined = NULL;
    FunctionReturnCount = 0;
    NextTempFloatReg = 0;
    NextTempDataReg = 0;
    NextTempAddrReg = 0;
    HashCount = 0;
    AddDefine("MC68000", MakeEString("1"));
    AddDefine("mc68000", MakeEString("1"));
    AddDefine("m68k", MakeEString("1"));
    AddDefine("macintosh", MakeEString("1"));
    AddDefine("applec", MakeEString("1"));	/* pretend like we're MPW */
    AddDefine("__HARVESTC__", MakeEString("1"));	/* pretend like we're MPW */
    AddDefine("__STDC__", MakeEString("0"));	/* because MPW does it this
						 * way */
	if (gProject) {
	    if (gProject->itsOptions->useMC68881)
			AddDefine("mc68881", MakeEString("1"));
	}
    Labels = RawLabTable();
    SetupLibLabels();
    StaticTable = RawTable(23);
    StringLits = RawTable(101);
    TagsTable = RawTable(113);
    LabelsTable = RawTable(23);
    GlobalSymbolTable = RawTable(809);
    PushSpaces(GlobalSymbolTable, TagsTable, LabelsTable);
}

InFileVia_t
Init(int argc, char **argv)
{
    InFileVia_t                     resultlist;
    char                            volname[64];

    EachFileInit();

    /* Floating point sizes */
    SizeOfDouble = 8;
    SizeOfFloat = 4;
    SizeOfLongDouble = 10;

    MacroValue = Ealloc(MAXMACROLENGTH);	/* used for macro expansion */

    BuildKWHash();		/* builds a hash table for key words */
}

void                            FreeSymbolList(SymListVia_t, int);

void
FreeLab(LabSYMVia_t cur)
{
    if (cur) {
	FreeLab(Via(cur)->left);
	FreeLab(Via(cur)->right);
	Efree(cur);
    }
}

void
FreeLabSymbolList(LabSymListVia_t table)
{
    if (table) {
	FreeLab(Via(table)->_head);
	Efree(table);
    }
}

#undef FREEMEM

void
RememberSwitch(LabSymListVia_t tab)
{
    Switches[CountSwitch++] = tab;
}

void
FreeSwitches(void)
{
    while (CountSwitch > 0) {
	FreeLabSymbolList(Switches[--CountSwitch]);
    }
}

void
FreeParamList(ParamRecVia_t head)
{
    if (head) {
	FreeParamList(Via(head)->next);
	Efree(head);
    }
}

void
KillCodesList(InstListVia_t Codes)
{
    InstVia_t                       cur;
    InstVia_t                       next;
    if (Codes) {
	cur = Via(Codes)->head;
	while (cur) {
	    next = Via(cur)->next;
	    /* All Loc records are separately freed */
	    Efree(cur);
	    cur = next;
	}
	Via(Codes)->head = Via(Codes)->tail = NULL;
	Via(Codes)->count = 0;
	Via(Codes)->PendingLabel = NULL;
	/*
	 * We don't free the list itself because we usually use this merely
	 * to "empty" the code list
	 */
    }
}

void
OneFileCleanUp(void)
{
    long                            x;
    long                            y;
    /*
     * Here will go all memory deallocation which needs to be done between
     * files.
     */
    CloseGenericFile(errfile);
    CloseGenericFile(outfile);
    while (CurrentBlock)
	PopBlock();
    FreeSymbolList(GlobalSymbolTable, 1);
    FreeSymbolList(TP_defnames, 1);
    FreeParamList(ParamList);
    FreeSymbolList(StringLits, 1);
    FreeSymbolList(StaticTable, 1);
    FreeSymbolList(TagsTable, 1);
    FreeSymbolList(LabelsTable, 1);
    FreeLabSymbolList(SegmentNames);
    FreeLabSymbolList(Labels);
    FreeSwitches();
    FreeTypeRecords();
    Efree(InitDataAcc);
    KillFloatList(FloatList);
    KillCodesList(GlobalCodes);
    KillAllLocations();
    KillSpillList();
    KillStringPool(ParserStrings);
    SpillList = NULL;
    Efree(GlobalCodes);
    FreePPSymbolList(Defines);
    y = MaxMem(&x);
}

int
UserFileError(int err)
{
    char                            mesg[80];
    if (err) {
	sprintf(mesg, "Error # %d\n", err);
	c2pstr(mesg);
	SysBeep(1);
    } else {
	return 0;
    }
}

void
CleanUp(void)
{
    FreeTypeRecords();
    Efree(MacroValue);
    FreePPSymbolList(Defines);
}

#ifdef OLDCODE
void
OneFileReport(void)
{
    /* This routine reports a number of software metrics */
    char                            msg[256];
    if (Opts.metrics) {
	T_CountCasts += CurrentSRC.CountCasts;
	T_TotalIdentifierLength += CurrentSRC.TotalIdentifierLength;
	T_CountIdentifiers += CurrentSRC.CountIdentifiers;
	T_PreprocSubsts += CurrentSRC.PreprocSubsts;
	T_PreprocBefore += CurrentSRC.PreprocBefore;
	T_PreprocAfter += CurrentSRC.PreprocAfter;
	T_CountPreprocCondits += CurrentSRC.CountPreprocCondits;
	T_StmtCount += CurrentSRC.StmtCount;
	T_CountComments += CurrentSRC.CountComments;
	T_CommentBytes += CurrentSRC.CommentBytes;
	sprintf(msg, "\nNumber of lines = %d\n", CurrentSRC.LineCount - 1);
	UserMesg(msg);
	sprintf(msg, "Number of chars = %d\n", CurrentSRC.CharCount - 1);
	UserMesg(msg);
	sprintf(msg, "Count casts = %d\n", CurrentSRC.CountCasts);
	UserMesg(msg);
	if (CurrentSRC.CountIdentifiers) {
	    sprintf(msg, "Avg id length = %g\n", (double) CurrentSRC.TotalIdentifierLength / CurrentSRC.CountIdentifiers);
	    UserMesg(msg);
	}
	sprintf(msg, "Preproc Substitutions = %d\n", CurrentSRC.PreprocSubsts);
	UserMesg(msg);
	if (CurrentSRC.PreprocSubsts && CurrentSRC.PreprocBefore) {
	    sprintf(msg, "Preproc Expansion factor = %g\n", (double) CurrentSRC.PreprocAfter / CurrentSRC.PreprocBefore);
	    UserMesg(msg);
	}
	sprintf(msg, "Preproc conditional compilations = %d\n", CurrentSRC.CountPreprocCondits);
	UserMesg(msg);
	sprintf(msg, "Count statements = %d\n", CurrentSRC.StmtCount);
	UserMesg(msg);
	sprintf(msg, "Count Comments = %d\n", CurrentSRC.CountComments);
	UserMesg(msg);
	sprintf(msg, "Bytes of comments = %d\n", CurrentSRC.CommentBytes);
	UserMesg(msg);
    }
}

void
Report(void)
{
    /* This routine reports a number of software metrics */
    char                            msg[256];
    UserMesg("\n----- Summary -----\n");
    sprintf(msg, "Time of compilation = %d seconds\n", (EndingTime - StartingTime) / 60);
    UserMesg(msg);
    if (Opts.metrics) {
	sprintf(msg, "Count casts = %d\n", T_CountCasts);
	UserMesg(msg);
	sprintf(msg, "Avg id length = %g\n", (double) T_TotalIdentifierLength / T_CountIdentifiers);
	UserMesg(msg);
	sprintf(msg, "Preproc Substitutions = %d\n", T_PreprocSubsts);
	UserMesg(msg);
	if (T_PreprocSubsts && T_PreprocBefore) {
	    sprintf(msg, "Preproc Expansion factor = %g\n", (double) T_PreprocAfter / T_PreprocBefore);
	    UserMesg(msg);
	}
	sprintf(msg, "Preproc conditional compilations = %d\n", T_CountPreprocCondits);
	UserMesg(msg);
	sprintf(msg, "Count statements = %d\n", T_StmtCount);
	UserMesg(msg);
	sprintf(msg, "Count Comments = %d\n", T_CountComments);
	UserMesg(msg);
	sprintf(msg, "Bytes of comments = %d\n", T_CommentBytes);
	UserMesgCR(msg);

	sprintf(msg, "Count Global Syms = %d\n", (int) Via(GlobalSymbolTable)->count);
	UserMesg(msg);
	sprintf(msg, "Count string lits = %d\n", (int) Via(StringLits)->count);
	UserMesg(msg);
	sprintf(msg, "Count Dup string lits = %d\n", DuplicateStringLits);
	UserMesg(msg);
	sprintf(msg, "Num of errors = %d\n", NumErrors);
	UserMesg(msg);
	sprintf(msg, "Num of warnings = %d\n", NumWarnings);
	UserMesg(msg);
	sprintf(msg, "Attempted warnings = %d\n", AttemptWarnings);
	UserMesg(msg);
	sprintf(msg, "Total # of lines = %d\n", TotalLines);
	UserMesg(msg);
	sprintf(msg, "Number of #include directives = %d\n", CountIncludes);
	UserMesg(msg);
	sprintf(msg, "Total globals = %d\n", SumGlobals);
	UserMesg(msg);
	sprintf(msg, "Total global space = %d\n", SumGlobalSpace);
	sprintf(msg, "Number of functions = %d\n", CountFunctions);
	UserMesg(msg);
	{
	    int                             ndx;
	    double                          HalV, HalL;
	    long                            HalN;
	    ndx = 0;
	    while (ndx < KEYWORDTABLESIZE) {
		if (KWTable[ndx].val) {
		    if (KWTable[ndx].uses) {
			HalUniqOperators++;
		    }
		}
		ndx++;
	    }
	    sprintf(msg, "Halstead metrics :\n");
	    UserMesg(msg);
	    sprintf(msg, "     Length : %d\n", HalN = HalTotalOperands + HalTotalOperators);
	    UserMesg(msg);
	    sprintf(msg, "     Volume : %g\n", (double) (HalV = HalN * (log2((double) (HalUniqOperands + HalUniqOperators)))));
	    UserMesg(msg);
	    sprintf(msg, "     Level  : %g\n", (double) (HalL = (2 / (double) HalUniqOperators) * (HalUniqOperands / (double) HalTotalOperands)));
	    UserMesg(msg);
	    sprintf(msg, "     Ment D : %g\n", (HalV / HalL));
	    UserMesg(msg);
	}
    }
}
#endif

int
ReInit(char *filename, int volrefnum, long dirID)
{
    EachFileInit();
    if (!OpenInput(filename, volrefnum, dirID))
	return 0;
    if (!OpenOutput(filename, volrefnum, dirID)) {
	PopSRC();
	return 0;
    }
    if (!OpenError(filename, volrefnum, dirID)) {
	PopSRC();
	CloseGenericFile(errfile);
	return 0;
    }
    CurrentSRC.ExtraChar[0] = GetCharacter();
    CurrentSRC.NumExtras = 1;
    return 1;
}

#ifdef SRC_COMM
void
PreprocOnly(char *name, short volrefnum, long dirID)
{
    Codigo_t                        result;
    char                            buf[256];
    OpenInput(name, volrefnum, dirID);
    OpenOutput(name, volrefnum, dirID);
    OpenError(name, volrefnum, dirID);
    sprintf(buf, "#line %d \"%s\"\n", CurrentSRC.LineCount, CurrentSRC.fname);
    Output(buf, outfile);
    while (!gAllDone) {
	result = GetToken();
	if (!gAllDone) {
	    switch (result) {
	    case CHARCONSTANT:
		/* Call to DumpCharK deleted here. */
		sprintf(buf, " %d", LastIntegerConstant);
		Output(buf, outfile);
		break;
	    case PASCSTRING_LITERAL:
	    case STRING_LITERAL:
		DumpStringLit((LastToken), outfile);
		break;
	    default:
		sprintf(buf, " %s", LastToken);
		Output(buf, outfile);
		break;
	    }
	}
    }
}

#endif

