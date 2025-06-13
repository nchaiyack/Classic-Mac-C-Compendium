/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * Global variables
 * 
 */

#ifndef Globals_INTERFACE
#define Globals_INTERFACE

#include "SymTable.h"
#include "PPSymTable.h"
#include "SRC.h"

	/* The following contain the working data structures for this C source file */
extern 	ParamRecVia_t            ParamList;
extern LabSymListVia_t          SwitchCases;
#define MAXSWITCHES 64
extern LabSymListVia_t          Switches[MAXSWITCHES];
extern int                      CountSwitch;

extern LabSymListVia_t          Labels;	/* list of asm labels */
extern SymListVia_t             StringLits;	/* String literal pool */
extern SymListVia_t             TagsTable;	/* Struct/union/enum tags */
extern SymListVia_t             LabelsTable;	/* C labels */
extern SymListVia_t             StaticTable;	/* global list of static
						 * variables (static within
						 * functions) */
extern SymListVia_t             GlobalSymbolTable;	/* All external symbols
							 * are defined here. */
extern SymListVia_t             TP_defnames;	/* All TP_defnames are
						 * defined here. */
extern LabSymListVia_t             SegmentNames;	/* All segment names are
						 * stored here. */
extern PPSymListVia_t           Defines;	/* the list of all symbols
						 * defined my the
						 * preprocessor.  This
						 * includes both simple
						 * #defines and macros. */
/*
 * A SRC is the logical source of the C program text being lexed/parsed.  It
 * may be a region of memory or a file
 */

extern SRCListVia_t             SRCStack;	/* the SRC stack */
extern SRCKind_t                CurrentSRC;	/* the current SRC for C code */

extern FloatLitVia_t            FloatList;	/* floating point literal
						 * pool */
extern InstListVia_t            GlobalCodes;	/* very important - this is
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
extern MPWDictListVia_t         OBJNameList;
extern MPWListVia_t             GlobalRecords;

extern int                      ExtraBlocks;	/* used primarily in
						 * parsestmt.c, for keeping
						 * track of scopes */
extern char                     Errm[MAXERRORMSG];	/* used as a buffer for
							 * constructing error
							 * messages */

extern struct OptionsS          Opts, DefaultOpts;	/* A single structure
							 * containing all user
							 * options pertaining to
							 * compilation, parsing
							 * and code generation.  */

extern unsigned int             CountIncludes;

/*
 * The next variables are used while constructing struct declarations
 */
extern int                      CurFieldOffset;	/* offset of current bit
						 * field */
extern int                      NextStructOffset;	/* offset of next member */
extern int                      CurFieldBit;
extern int                      CurStructSize;
extern ParseTreeVia_t           LastSwitchExpression;	/* used by the parser to
							 * remember the
							 * expression in the
							 * last switch(expr)
							 * statement, so we can
							 * typecheck the case
							 * labels against that
							 * expression */
extern int                      InPreprocIf;	/* a flag which indicates to
						 * the parser whether or not
						 * we are parsing the
						 * expression following an
						 * #if or #elif directive */
extern KW_t                     KWTable[KEYWORDTABLESIZE];	/* The KWTable is a hash
								 * table for C reserved
								 * words.  Since
								 * virtually every token
								 * must be checked
								 * against the list of
								 * reserved word, it is
								 * worthwhile to make
								 * this search
								 * efficient. */
extern long                     CurrentEnumK;	/* The value of the current
						 * enum constant. Used during
						 * construction of an enum
						 * declaration. */
extern TypeRecordVia_t          EnumBeingDeclared;	/* During an enum
							 * declaration, this
							 * points to the type
							 * record of the enum
							 * being declared.  Each
							 * enum constant bears a
							 * pointer to its own
							 * type. */
extern SYMVia_t                 FunctionBeingDefined;	/* During a function
							 * definition, this
							 * points to the symbol
							 * being defined.  */
/*
 * All of the following variables are used in calculating various metrics
 */
extern unsigned int             T_PreprocSubsts;
extern unsigned int             T_PreprocBefore;
extern unsigned int             T_PreprocAfter;
extern unsigned int             T_CommentBytes;
extern unsigned int             T_StmtCount;
extern unsigned int             T_CountIdentifiers;
extern unsigned int             T_TotalIdentifierLength;
extern unsigned int             T_CountCasts;
extern unsigned int             T_CountPreprocCondits;
extern unsigned int             T_CountComments;
extern int                      FunctionReturnCount;
extern int                      FunctionComplexity;
extern int                      HalTotalOperands;
extern int                      HalTotalOperators;
extern int                      HalUniqOperators;
extern int                      HalUniqOperands;
extern int                      PreprocSymbolCount;
extern int                      DuplicateStringLits;
extern int                      TotalLines;
extern int                      CountFunctions;

extern int                      SumGlobals;
extern int                      SumGlobalSpace;
extern LocAMVia_t               StructDoubleReturnLoc;	/* During code
							 * generation, this
							 * points to the place
							 * to return a struct.
							 * doubles are now
							 * returned in d0-d1 */
/* These keep track of the efficiency of the keyword hash table. */
extern int                      HashCollisions;
extern int                      HashCount;

extern int                      OddGlobal;	/* true if an odd number of
						 * bytes have been generated
						 * for global data.
						 * depending on the next
						 * global variable, an
						 * alignment directive may be
						 * needed */
extern int                      DiscardedFunctionCall;	/* a flag indicating to
							 * the code generator
							 * that the result of
							 * the function call
							 * being generated is
							 * not needed. for
							 * example, extern int
							 * x(void); x(); */

extern char                     LastPreproc[64];	/* records the name of
							 * the last preproc
							 * directive used */

extern int                      TokenOnDeck;	/* flag indicating if a
						 * single token has been read
						 * and then putback. Such a
						 * token is on deck and is
						 * easily available to the
						 * parser in NextIs() */
extern char                     NextToken[MAXIDLENGTH];	/* tokens on deck are
							 * stored here */
extern Codigo_t                 NextTokenKind;	/* tokens on deck are stored
						 * here */

extern char                    *LastToken;	/* A very important variable.
						 * This contains the string
						 * of the last token read. */
extern int                      SizeOfFloat;	/* size of float type in
						 * bytes */
extern int                      SizeOfDouble;
extern int                      SizeOfLongDouble;
extern Codigo_t                 LastTokenKind;	/* code for the last token
						 * read */
extern long double              LastFloatingConstant;	/* value of the last
							 * floating point
							 * constant read -
							 * probably only valid
							 * if the last token
							 * really was a floating
							 * point constant */
extern long                     LastIntegerConstant;
extern char                     ThisToken[MAXIDLENGTH];	/* tokens are
							 * constructed here
							 * during lexing */
extern char                     LastChar;	/* The last character read
						 * which contributed to a
						 * token.  TODO - is this
						 * correct or is this the
						 * last character returned
						 * from RawCharacter() ? */

extern GenericFileVia_t         outfile;	/* output file for assembler
						 * source, etc */
extern GenericFileVia_t         errfile;	/* error file */

extern int                      NumErrors;	/* Cumulative count of
						 * errors. */
extern int                      AttemptWarnings;	/* Cumulative count of
							 * warnings. */
extern int                      NumWarnings;	/* Cumulative count of
						 * warnings. */
extern int                      gAllDone;	/* This is set true when the
						 * SRCStack is empty. */


/*
 * The Preprocessor uses the variable PPStatus to signal to the parser that
 * tokens are to be ignored, due to the a conditional compilation directive.
 * However, since these directives may be nested, there is a stack to keep
 * old values of PPStatus.  This stack is implemented as an array of int and
 * PPSP is the stack pointer.
 */
extern int                      PreprocStack[MAXNESTIFDEF];
extern int                      PPSP;
extern int                      PPStatus;
/*
 * The following are used for macro definition by the preprocessor.
 * MacroValue is used to temporarily store the value of the macro being
 * defined. The 'value' of a a macro, as I define it, is the string used in
 * expanding it.  In other words, it is the part just to the right of the id
 * and arguments : #define macro(a,b,c) VALUE
 */
extern EString_t                MacroValue;

extern BlockStackVia_t          CurrentBlock;	/* used in keeping track of
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
extern LabSYMVia_t              FunctionLastLabel;	/* during code
							 * generation, this is
							 * the number of the
							 * label which will
							 * occur at the end of
							 * the function. a
							 * return statement is
							 * compiled with a jump
							 * to this label */
extern int                      NextFloatLitLabel;	/* number of the next
							 * label for floating
							 * point literals */
extern int                      NextLitLabel;	/* number of the next label
						 * for string literals */
extern int                      NextEccLabel;	/* number of the next general
						 * Ecc generated label */
/*
 * used by the register allocator, these next three variables keep track of
 * which register is the next candidate for allocation. Register allocation
 * is done on the fly
 */
extern int                      NextTempFloatReg;
extern int                      NextTempDataReg;
extern int                      NextTempAddrReg;
/*
 * these arrays are part of the register allocator, and contain data
 * structures representing the machine registers.
 */
extern RegisterRecord_t         TempFloats[8];
extern RegisterRecord_t         TempDatas[8];
extern RegisterRecord_t         TempAddrs[8];
/*
 * used by the register allocator and code generator, this is a list of spill
 * slots
 */
extern SpillSlotVia_t           SpillList;
extern LabSYMVia_t              BreakLabel;	/* The current break label.
						 * A break statement is
						 * simply compiled as a jump
						 * to this label.  This label
						 * is set in the routine that
						 * generates code for a loop
						 * of some kind */
extern LabSYMVia_t              ContinueLabel;	/* similar to BreakLabel */
extern InstVia_t                LinkInst;	/* The pointer to the
						 * instruction record for the
						 * link instruction for the
						 * function being compiled.
						 * The constant contained
						 * herein may be modified
						 * during compilation to
						 * allocate more space on the
						 * stack */
extern EString_t                InitDataAcc;	/* used for accumulating data
						 * for an initializer */
extern int                      InitDataAccIndex;	/* index used for
							 * accumulating
							 * initializer data */
extern int                      InitDataAccSize;
extern void                    *RainyDayZone;

extern unsigned char           *BitsBuffer;
extern int                      BBIndex;
extern int                      thesz;	/* bitmap form of Ext                   */
extern int                      Pass;
extern int                      Pc;	/* Program Counter                      */
extern int                      Old_pc;	/* Program Counter at beginning         */
extern int                      Fwdsize;	/* default fwd ref size                 */
extern short                    CurVolrefnum;
extern long                     CurDirID;
extern char                     asmLine[];
extern LabSYMVia_t                      CurrentSegmentLabel;
extern LabSYMVia_t              LibLabelULMODT;
extern LabSYMVia_t              LibLabelLMODT;
extern LabSYMVia_t              LibLabelULMULT;
extern LabSYMVia_t              LibLabelULDIVT;
extern LabSYMVia_t              LibLabelLDIVT;
extern LabSYMVia_t              LibLabelXTOI;
extern int                      PartDone;
extern StringPoolVia_t          ParserStrings;
extern int                      gAbortCompile;

#endif
