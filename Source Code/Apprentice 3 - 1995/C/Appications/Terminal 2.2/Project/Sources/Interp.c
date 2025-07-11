/*
	Terminal 2.2
	"Interp.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "interp.h"

#define	FALSE	0
#define TRUE	1
#define EOF		0xFF
#define LINE	256		/* Maximum line size */

extern Byte EmptyStr[];	/* Empty string */

/* ----- Error codes -------------------------------------------------- */

enum errs {
	EARLYEOF = 1,		/* Unexpected end of file */
	UNRECOGNIZED,		/* ... unrecognized */
	DUPL_DECLARE,		/* ... duplicate identifier */
	TABLEOVERFLOW,		/* Symbol table full */
	MEMERR,				/* Out of heap memory */
	UNDECLARED,			/* ... undeclared identifier */
	SYNTAX,				/* Syntax error */
	MATCHERR,			/* ... unmatched */
	MISSING,			/* ... missing */
	NOTFUNC,			/* Not a function */
	OUTOFPLACE,			/* ... out of place */
	BUFFULL,			/* Token buffer overflow */
	DIVIDEERR,			/* Divide by zero */
	POINTERERR,			/* Pointer error */
	PARAMERR			/* Parameter error */
};

/* ----- Symbol table structure ---------------------------------------- */

typedef struct {
	Byte *name;			/* Points to symbol name (in token buffer) */
	INTEGER value;		/* Value (integer or pointer) */
	Byte size;			/* 0: function, 1: char, 4: int */
	Byte ind;			/* Indirection level */
} SYMBOL;

/* ----- Environment for expression evaluation ------------------------- */

typedef struct {
	SYMBOL *sp;			/* Local symbol table pointer */
	INTEGER value;		/* Value or address of variable */
	Byte size;			/* 0: function, 1: char, 4: int */
	Byte ind;			/* Indirection level */
	Byte adr;			/* 0: value, 1: address */
} ENV;

/* ----- Function macros ----------------------------------------------- */

#define bypass()			tptr += strlen((char *)tptr) + 1
#define iswhite(c)			(c == ' ' || c == '\t')
#define iscsymf(c)			(isalpha(c) || c == '_')
#define iscsym(c)			(isalnum(c) || c == '_')

/* ----- Function prototypes ------------------------------------------- */

Byte *allocate(long);
void x2str(long, Byte *);
long a2x(Byte *);
Byte *token2str(short);
Byte gettoken(void);
Byte getok(void);
Byte iskeyword(void);
Byte isident(void);
Byte istoken(void);
Byte getword(void);
Byte getcx(void);
SYMBOL *addsymbol(SYMBOL *, Byte *, INTEGER, Byte, Byte);
SYMBOL *findsymbol(SYMBOL *, Byte *, SYMBOL *);
SYMBOL *ifsymbol(SYMBOL *, Byte *, SYMBOL *);
void error(enum errs, Byte *);
Boolean iftoken(Byte);
void skippair(Byte, Byte);
void needtoken(Byte);
Byte nexttoken(void);
Byte escseq(void);
Byte h2(void);

void compound_statement(SYMBOL *);
void statement(SYMBOL *);
void statements(SYMBOL *);
void skip_statements(SYMBOL *);
INTEGER pfunction(Byte *, SYMBOL *);

INTEGER expression(SYMBOL *);
void assign(ENV *);
void or(ENV *);
void and(ENV *);
void eq(ENV *);
void le(ENV *);
void plus(ENV *);
void mult(ENV *);
void unary(ENV *);
void variable(ENV *);
void primary(ENV *);
void rvalue(ENV *);
void store(ENV *, INTEGER);

/* ----- Characters in source, not copied to token buffer -------------- */

#define COMMENT1	'/'
#define COMMENT2	'*'
#define QUOTES		'"'
#define QUOTE		'\''

/* ----- Tokens (found in token buffer) -------------------------------- */

#define LINENO		127		/* '\015', must be unique */
#define BREAK		'b'		/* break */
#define CHAR		'c'		/* char */
#define ELSE		'e'		/* else */
#define FOR			'f'		/* for */
#define IF			'i'		/* if */
#define INT			'l'		/* int */
#define RETURN		'r'		/* return */
#define WHILE		'w'		/* while */

#define IDENT		'I'		/* <identifier> */
#define CONSTANT	'C'		/* <constant> */
#define STRING		'S'		/* <string> */

#define AUTOINC		'P'		/* ++ */
#define AUTODEC		'D'		/* -- */
#define EQUALTO		'E'		/* == */
#define NOTEQUAL	'N'		/* != */
#define GE			'G'		/* >= */
#define LE			'L'		/* <= */
#define AUTOADD		'A'		/* += */
#define AUTOSUB		'B'		/* -= */
#define AUTOMUL		'M'		/* *= */
#define AUTODIV		'V'		/* /= */
#define AUTOMOD		'M'		/* %= */
#define ADDRESS		'@'		/* &  */

#define AND			'&'		/* && */
#define OR			'|'		/* || */
#define POINTER		'*'		/* pointer */
#define PLUS		'+'
#define	MINUS		'-'
#define MULTIPLY	'*'
#define DIVIDE		'/'
#define MODULO		'%'
#define EQUAL		'='
#define LESS		'<'
#define GREATER		'>'
#define NOT			'!'
#define LPAREN		'('
#define RPAREN		')'
#define LBRACE		'{'
#define RBRACE		'}'
#define LBRACKET	'['
#define RBRACKET	']'
#define COMMA		','
#define SEMICOLON	';'

/* ----- Table of keywords and their tokens ---------------------------- */

static struct keywords {
	Byte *kw;
	Byte kwtoken;
} kwds[] = {
	(Byte *)"\015",		LINENO,
	(Byte *)"break",	BREAK,
	(Byte *)"char",		CHAR,
	(Byte *)"else",		ELSE,
	(Byte *)"for",		FOR,
	(Byte *)"if",		IF,
	(Byte *)"int",		INT,
	(Byte *)"return",	RETURN,
	(Byte *)"while",	WHILE,
	NULL,				0
};

/* ----- Table of direct translate tokens ------------------------------ */

static Byte tokens[] = {
	COMMA, LBRACE, RBRACE, LPAREN, RPAREN, EQUAL, NOT, POINTER,
	LESS, GREATER, AND, OR, SEMICOLON, LBRACKET, RBRACKET,
	MULTIPLY, DIVIDE, MODULO, PLUS, MINUS, EOF, 0
};

/* ----- Local data ---------------------------------------------------- */

/*
	Memory layout:                 		<- Globals
	High addr	+---------------------+
				| global symbols      |
				|.....................| <- EndGlobals
				|                     |
				| local symbol        |
				| (function params)   |	<- SymTop (grows down)
				+---------------------+
				|                     |
				| free memory         |
				|                     | <- StackPtr (grows up)
				+---------------------+
				|                     | 
				| arrays and function |
				| parameters          | <- LoMem
				+---------------------+
				|                     |
				| token buffer        |
				|                     | <- TokenBuffer
	Low addr	+---------------------+

*/

static SYMBOL *Globals;		/* Function/variable symbol table */
static SYMBOL *EndGlobals;	/* Last global symbol */
static SYMBOL *SymTop;		/* Last symbol in table */
static Byte *StackPtr;		/* Arrays and function parameters */
static Byte *LoMem;			/* Array allocation starts here */
static Byte *tptr;			/* Running token pointer */
static Byte *TokenBuffer;	/* Compiled token buffer */
static short skipping;		/* Semaphore used for skipping statements */
static short breaking;		/* TRUE if "break" statement executed */
static short returning;		/* TRUE if "return" statement executed */
static INTEGER frtn;		/* Return value from a function */
static long linenumber;		/* Line number in source file */

/* ----- Return remaining stack space ---------------------------------- */

INTEGER SI_stack(params)	/* Used by shell as intrinsic function */
INTEGER *params;
{
#pragma unused(params)
	return (Byte *)SymTop - StackPtr;
}

/* ----- Allocate memory on the stack ---------------------------------- */

static Byte *allocate(size)
register long size;
{
	register Byte *sp = StackPtr;

	if (size & 1)	/* Make sure stack pointer remains even */
		size++;
	if ((StackPtr += size) >= (Byte *)SymTop)
		error (MEMERR, EmptyStr);
	return sp;
}

/* ----- Lexical scan and call linker ---------------------------------- */

void SI_Load(intrinsics, memory, size)
register INTRINSIC *intrinsics;	/* Intrinsic functions provided by shell */
Byte *memory;					/* Start of memory provided by shell */
long size;						/* Size of memory provided by shell */
{
	register short tok;
	register short n;

	/* Set up memory pointers */

	if (size & 1)		/* Make sure address is even */
		size--;
	LoMem = (Byte *)(SymTop = Globals =
		(SYMBOL *)((tptr = TokenBuffer = memory) + size)) - LINE;

	/* Load token buffer */

	linenumber = 1;
	do {
		if (tptr >= LoMem)
			error(BUFFULL, EmptyStr);
		n = linenumber;

		/* *tptr++ = tok = gettoken(); 	Ok in THINK C but not in MPW! */
		tptr++; tok = gettoken(); *(tptr - 1) = tok;

		n = linenumber - n;
		switch (tok) {
			case CONSTANT:
			case IDENT:
			case STRING:
				bypass();
				break;
			case LINENO:
				++linenumber;
				break;
		}
		while (n--) {
			if (tptr >= LoMem)
				error(BUFFULL, EmptyStr);
			*tptr++ = LINENO;
		}
	} while (tok != EOF);
	if ((long)tptr & 1)	/* Make sure address is even */
		tptr++;
	linenumber = 0;	/* From now on error() must count LINENO tokens */

	/* Add intrinsic functions to symbol table */

	StackPtr = LoMem = tptr;
	for ( ; intrinsics->fn; intrinsics++)
		addsymbol(Globals,intrinsics->fname,(INTEGER)intrinsics->fn,0,0);

	/* Link the global variables and functions */

	tptr = TokenBuffer;
	while ((tok = nexttoken()) != EOF) {
		if (tok == CHAR || tok == INT) {		/* Variable declaration */
			do {
				register SYMBOL *symbole;
				short ind = 0;
				while (iftoken(POINTER))
					ind++;						/* char *xyz */
				needtoken(IDENT);
				symbole = addsymbol(Globals, tptr, 0,
					(tok == CHAR) ? 1 : sizeof(INTEGER), ind);
				bypass();
				if (iftoken(LBRACKET)) {
					if (iftoken(RBRACKET))		/* xyz[] */
						(symbole->ind)++;
					else {						/* xyz[...] */
						short size;
						size = (symbole->size == 1 && symbole->ind == 0) ?
							1 : sizeof(INTEGER);
						symbole->value =
							(INTEGER)allocate(size * expression(Globals));
						(symbole->ind)++;
						needtoken(RBRACKET);
					}
				}
				if (iftoken(EQUAL)) {
					if (iftoken(LBRACE)) {		/* x = { xxx, ... } */
						INTEGER *p;
						symbole->value = (INTEGER)StackPtr;
						do {
							p = (INTEGER *)allocate(sizeof(INTEGER));
							*p = expression(Globals);
						} while (iftoken(COMMA));
						needtoken(RBRACE);
					} else {					/* x = xxx */
						symbole->value = expression(Globals);
					}
				}
			} while (iftoken(COMMA));
			needtoken(SEMICOLON);
		} else if (tok == IDENT) {		/* Function definition */
			Byte *name = tptr;
			bypass();
			addsymbol(Globals, name, (INTEGER)tptr, 0, 0);
			skippair(LPAREN, RPAREN);
			skippair(LBRACE, RBRACE);			/* xyz(...) {...} */
		} else
			error(EARLYEOF, EmptyStr);
	}
	EndGlobals = SymTop;
}

/* ----- Start the interpreter ----------------------------------------- */

INTEGER SI_Interpret()
{
	skipping = 0;
	breaking = returning = FALSE;
	tptr = (Byte *)"Imain\0();";
	return expression(SymTop);
}

/* ----- Return the next token ----------------------------------------- */

static Byte gettoken()
{
	register Byte tok;

	tok = getword();
	if (!tok)						/* Not a char/string constant */
		if (!(tok = iskeyword()))	/* No keyword */
			if (!(tok = istoken()))	/* No one character token */
				tok = isident();	/* Then should be ident. or constant */
	if (!tok)
		error(UNRECOGNIZED, tptr);
	return tok;
}

/* ----- Test to see if current word is a one character token ---------- */

static Byte istoken()
{
	register Byte *t = tokens;	/* Single character tokens */
	register Byte t2;

	if (strlen((char *)tptr) != 1)
		return 0;
	while (*t)
		if (*tptr == *t++) {
			switch (*tptr) {
				case EOF:
					break;
				case AND:		/* Distinction between & and && */
					if ((t2 = getcx()) != AND) {
						*tptr = ADDRESS;
						SI_UngetSource(t2);
					}
					break;
				case OR:		/* Must be || */
					if (getcx() != OR)
						error(MISSING, tptr);
					break;
				case PLUS:		/* Distinction between +, ++ and += */
				case MINUS:		/* Distinction between -, -- and -= */
					if ((t2 = getcx()) == *tptr)
						*tptr = (*tptr == PLUS) ? AUTOINC : AUTODEC;
					else if (t2 == EQUAL)
						*tptr = (*tptr == PLUS) ? AUTOADD : AUTOSUB;
					else
						SI_UngetSource(t2);
					break;
				case RBRACE:	/* May be last token */
				case SEMICOLON:
					break;
				default:
					if ((t2 = getcx()) == EQUAL) {
						switch (*tptr) {
							case EQUAL:				/* == */
								return EQUALTO;
							case NOT:				/* != */
								return NOTEQUAL;
							case LESS:				/* <= */
								return LE;
							case GREATER:			/* >= */
								return GE;
							case MULTIPLY:			/* *= */
								return AUTOMUL;
							case DIVIDE:			/* /= */
								return AUTODIV;
							case MODULO:			/* %= */
								return AUTOMOD;
						}
					}
					SI_UngetSource(t2);
					break;
			}
			return *tptr;
		}
	return 0;
}

/* ----- Test word for a keyword --------------------------------------- */

static Byte iskeyword()
{
	register struct keywords *k = kwds;

	while (k->kw)
		if (!strcmp((char *)k->kw, (char *)tptr))
			return k->kwtoken;
		else
			k++;
	return 0;
}

/* ----- Test for an ident (or constant) ------------------------------- */

static Byte isident()
{
	register Byte *wd = tptr;
	register long n = 0;

	if (iscsymf(*wd))			/* Letter or underscore */
		return IDENT;
	if (!strncmp((char *)wd, "0x", 2) || !strncmp((char *)wd, "0X", 2)) {
		wd += 2;				/* 0x... hex constant */
		while (*wd) {
			if (!isxdigit(*wd))
				return 0;		/* Not a hex digit */
			n = (n << 4) + (isdigit(*wd) ? *wd - '0':
				tolower(*wd) - 'a' + 10);
			wd++;
		}
	} else
		while (*wd) {
			if (!isdigit(*wd))
				return 0;		/* Not a digit */
			n = (n * 10) + (*wd -'0');
			wd++;
		}
	x2str(n, (Byte *)tptr);		/* Converted constant */
	return CONSTANT;
}

/* ----- Get the next word from the input stream ----------------------- */

static Byte getword()
{
	register Byte *wd = tptr;
	register Byte c;
	register Byte tok;

	do
		c = getok();				/* Bypass white space */
	while (iswhite(c));
	if (c == QUOTE) {
		register unsigned long n = 0;
		register short max = 4;		/* Maximum 4 characters */
		while ((c = getcx()) != QUOTE) {
			if (!max)
				error(MISSING, (Byte *)"'");/* Needs the other quote */
			max--;
			if (c  == '\\')			/* Escape sequence (\015) */
				c = escseq();
			n = (n << 8) | (c & 0xFF);
		}
		x2str(n, (Byte *)tptr);		/* Build the constant value */
		return CONSTANT;
	}
	if (c == QUOTES) {
		tok = STRING;				/* Quoted string "abc" */
		while ((c = getcx()) != QUOTES)
			*wd++ = (c == '\\') ? escseq() : c;
	} else {
		tok = 0;
		*wd++ = c;					/* 1st char of word */
		while (iscsym(c)) {			/* Build an ident */
			c = getok();
			if (iscsym(c))
				*wd++ = c;
			else
				SI_UngetSource(c);
		}
	}
	*wd = '\0';		/* Null terminate the string or word */
	return tok;
}

/* ----- Escape sequence in litteral constant or string ---------------- */

static Byte h2()
{
	register Byte v = 0;
	register short n = 2;
	register Byte c;

	while (n--) {
		c = getcx();
		if (!isxdigit(c)) {
			Byte s[2];
			s[0] = c;
			s[1] = 0;
			error(OUTOFPLACE, s);	/* Not a hex digit */
		}
		v = (v << 4) + (isdigit(c) ? c - '0': tolower(c) - 'a' + 10);
	}
	return v;
}

static Byte escseq()
{
	register Byte c = getcx();

	return (c == 'n' ? '\012' :				/* 0x0A (LF)	*/
		c == 't' ? '\011' :					/* 0x09 (TAB)	*/
		c == 'f' ? '\014' :					/* 0x0C (FF)	*/
		c == 'a' ? '\007' :					/* 0x07 (BEL)	*/
		c == 'b' ? '\010' :					/* 0x08 (BS)	*/
		c == 'r' ? '\015' :					/* 0x0D (CR)	*/
		c == 'v' ? '\013' :					/* 0x0B	(VT)	*/
		c == '0' ? '\0' :					/* 0x00 (NUL)	*/
		(c == 'x') || (c == 'X') ? h2() :	/* 2 hex digits */
		c);
}

/* ----- Get a character from the input stream ------------------------- */

static Byte getok()
{
	register short c;
	register short c1;

	while ((c = SI_GetSource()) == COMMENT1) {
		if ((c1 = SI_GetSource()) != COMMENT2) {
			SI_UngetSource(c1);
			break;
		}
		do {
			while ((c1 = getcx()) != COMMENT2)
				if (c1 == '\015')
					++linenumber;
			c1 = getcx();
			if (c1 == '\015')
				++linenumber;
		} while (c1 != COMMENT1);
	}
	return c;
}

/* ----- Read a character from input, error if EOF --------------------- */

static Byte getcx()
{
	register short c;

	if ((c = SI_GetSource()) == -1)
		error(EARLYEOF, EmptyStr);
	return c;
}

/* ----- A function is called thru a pointer --------------------------- */

static INTEGER pfunction(fp, sp)
register Byte *fp;					/* Points to function definition */
SYMBOL *sp;
{
	register short i;
	register short p = 0;			/* Number of parameters */
	Byte *savetptr;					/* Will be saved and restored */
	Byte *ap = StackPtr;			/* Start of local arrays */
	register INTEGER *pp;

	needtoken(LPAREN);
	if (!iftoken(RPAREN)) {			/* Scan for actual parameters */
		do {
			pp = (INTEGER *)allocate(sizeof(INTEGER));
			*pp = expression(sp);	/* Evaluate parameter */
			p++;
		} while (iftoken(COMMA));
		needtoken(RPAREN);
	}
	savetptr = tptr;
	if (*fp == LPAREN) {			/* Call token function */
		tptr = fp;
		needtoken(LPAREN);
		sp = SymTop;				/* Local symbols start here */
		pp = (INTEGER *)ap;
		for (i = 0; i < p; i++) {	/* Params into local symbol table */
			short size;
			short ind = 0;
			if (iftoken(CHAR))
				size = 1;
			else if (iftoken(INT))
				size = sizeof(INTEGER);
			else
				error(PARAMERR, EmptyStr);
			while (iftoken(POINTER))
				ind++;
			needtoken(IDENT);
			addsymbol(sp, tptr, *pp++, size, ind);
			bypass();
			if (i < p-1)
				needtoken(COMMA);
		}
		StackPtr = ap;				/* Remove parameters from stack */
		needtoken(RPAREN);
		compound_statement(sp);		/* Execute the function */
		SymTop = sp;				/* Release the local symbols */
		breaking = returning = FALSE;
	} else {						/* Call intrinisic function */
		if (*fp != 0x4E || (long)fp & 1)/* Check for LINK instruction */
			error(NOTFUNC, EmptyStr);	/* ...on an even address */
		frtn = (*(IFUNC)fp)(ap);
		StackPtr = ap;				/* Remove parameters from stack */
	}
	tptr = savetptr;
	return frtn;					/* The function's return value */
}

/* ----- Execute one statement or a {} block --------------------------- */

static void statements(sp)
register SYMBOL *sp;
{
	if (iftoken(LBRACE)) {
		--tptr;
		compound_statement(sp);
	} else
		statement(sp);
}

/* ----- Execute a {} statement block ---------------------------------- */

static void compound_statement(sp)
register SYMBOL *sp;
{
	register short tok;

	if (!skipping) {
		register Byte *svtptr = tptr;
		register SYMBOL *spp = SymTop;	/* Local symbol table */
		Byte *app = StackPtr;

		needtoken(LBRACE);
		do {							/* Local variables in block */
			register SYMBOL *symbole;
			short size = 1;
			switch (tok = nexttoken()) {
				case INT:
					size = sizeof(INTEGER);
				case CHAR:
					do {
						short ind = 0;
						while (iftoken(POINTER))
							ind++;
						needtoken(IDENT);
						symbole = addsymbol(spp, tptr, 0, size, ind);
						bypass();
						if (iftoken(EQUAL))		/* Handle assignments */
							symbole->value = expression(sp);
						else if (iftoken(LBRACKET)) {	/* Array */
							short n =
								(symbole->size == 1 && symbole->ind == 0) ?
								1 : sizeof(INTEGER);
							symbole->value =
								(INTEGER)allocate(n * expression(sp));
							(symbole->ind)++;
							needtoken(RBRACKET);
						}
					} while (iftoken(COMMA));
					needtoken(SEMICOLON);
					break;
				default:
					tptr--;
					tok = 0;
			}
		} while (tok);
		while (!iftoken(RBRACE) && !breaking && !returning)
			statements(sp);
		SymTop = spp;				/* Free the local symbols */
		StackPtr = app;				/* Free the local arrays */
		tptr = svtptr;				/* Point to the opening brace */
	}
	skippair(LBRACE, RBRACE);		/* Skip to end of block */
}

/* ----- Execute a single statement ------------------------------------ */

static void statement(sp)
register SYMBOL *sp;
{
	register INTEGER rtn;
	register short tok;

	switch (tok = nexttoken()) {
		case IF:
			/* if ( expression ) statements                 */
			/* if ( expression ) statements else statements */
			if (skipping) {
				skippair(LPAREN, RPAREN);
				skip_statements(sp);
				while (iftoken(ELSE))
					skip_statements(sp);
				break;
			}
			needtoken(LPAREN);
			rtn = expression(sp);		/* Condidtion beeing tested */
			needtoken(RPAREN);
			if (rtn)
				statements(sp);			/* Condition is TRUE */
			else
				skip_statements(sp);	/* Condition is FALSE */
			while (iftoken(ELSE))
				if (rtn)				/* Do the reverse for else */
					skip_statements(sp);
				else
					statements(sp);
			break;
		case WHILE:
			/* while ( expression) statements */
			if (skipping) {
				skippair(LPAREN, RPAREN);
				skip_statements(sp);
				break;
			}
			{
				Byte *svtptr = tptr;
				breaking = returning = FALSE;
				do {
					tptr = svtptr;
					needtoken(LPAREN);
					rtn = expression(sp);		/* The condition tested */
					needtoken(RPAREN);
					if (rtn)					/* Condition is TRUE */
						statements(sp);
					else						/* Condition is FALSE */
						skip_statements(sp);
				} while (rtn && !breaking && !returning);
				breaking = FALSE;
			}
			break;
		case FOR:
			/* for (expression ; expression ; expression) statements */
			if (skipping) {
				skippair(LPAREN, RPAREN);
				skip_statements(sp);
				break;
			}
			{
				Byte *fortest, *forloop, *forblock;
				Byte *svtptr = tptr;		/* svtptr -> 1st ( after for */

				needtoken(LPAREN);
				if (!iftoken(SEMICOLON)) {
					expression(sp);			/* Initial expression */
					needtoken(SEMICOLON);
				}
				fortest = tptr;				/* fortest:terminating test */
				tptr = svtptr;
				skippair(LPAREN, RPAREN);
				forblock = tptr;			/* forblock: block to run */
				tptr = fortest;
				breaking = returning = FALSE;
				while (TRUE) {
					if (!iftoken(SEMICOLON)) {
						if (!expression(sp))	/* Terminating test */
							break;
						needtoken(SEMICOLON);
					}
					forloop = tptr;
					tptr = forblock;
					statements(sp);			/* The loop statement(s) */
					if (breaking || returning)
						break;
					tptr = forloop;
					if (!iftoken(RPAREN)) {
						expression(sp);		/* End of loop expression */
						needtoken(RPAREN);
					}
					tptr = fortest;
				}
				tptr = forblock;
				skip_statements(sp);		/* Skip past the block */
				breaking = FALSE;
			}
			break;
		case RETURN:
			/* return ;            */
			/* return expression ; */
			if (!iftoken(SEMICOLON)) {
				frtn = expression(sp);		/* Function return value */
				needtoken(SEMICOLON);
			}
			returning = !skipping;
			break;
		case BREAK:
			/* break ; */
			needtoken(SEMICOLON);
			breaking = !skipping;
			break;
		case IDENT:
		case POINTER:
		case AUTOINC:
		case AUTODEC:
		case LPAREN:
			/* expression ; */
			--tptr;
			expression(sp);
			needtoken(SEMICOLON);
			break;
		case SEMICOLON:
			/* ; */
			break;
		default:
			error(OUTOFPLACE, token2str(tok));
	}
}

/* ----- Bypass statement(s) ------------------------------------------- */

static void skip_statements(sp)
register SYMBOL *sp;
{
	skipping++;			/* Semaphore that suppresses assignments, */
	statements(sp);		/* ...breaks, returns, ++, --, function calls */
	--skipping;			/* Turn off semaphore */
}

/* ----- Recursive descent expression analyzer ------------------------- */

static void rvalue(env)			/* Read value */
register ENV *env;
{
	register short character;

	if (skipping) {
		env->value = 1;
		env->adr = FALSE;
		return;
	}
	if (env->adr) {
		switch (env->size) {
			case 1:
				character = (env->ind) ? FALSE: TRUE;
				break;
			case 0:
			case sizeof(INTEGER):
				character = FALSE;
				break;
			default:
				error(SYNTAX, EmptyStr);
		}
		if (character) {
			register Byte *v = (Byte *)env->value;
			env->value = *v;
		} else {
			register INTEGER *v = (INTEGER *)env->value;
			env->value = *v;
		}
		env->adr = FALSE;
	}
}

static void store(env, val)		/* Store value */
register ENV *env;
register INTEGER val;
{
	register short character;

	if (skipping)
		return;
	if (env->adr) {
		switch (env->size) {
			case 1:
				character = (env->ind) ? FALSE: TRUE;
				break;
			case sizeof(INTEGER):
				character = FALSE;
				break;
			default:
				error(SYNTAX, EmptyStr);
		}
		if (character) {
			register Byte *v = (Byte *)env->value;
			*v = val;
		} else {
			register INTEGER *v = (INTEGER *)env->value;
			*v = val;
		}
	} else
		error(SYNTAX, EmptyStr);
}

static INTEGER expression(sp)	/* Evaluate expression */
register SYMBOL *sp;
{
	ENV env;

	env.sp = sp;
	assign(&env);
	rvalue(&env);
	return env.value;		/* Return expression result */
}

static void assign(env)		/* Handle assignments (=) */
register ENV *env;
{
	ENV env2;

	or(env);
	while (iftoken(EQUAL)) {
		env2.sp = env->sp;
		assign(&env2);
		rvalue(&env2);
		store(env, env2.value);
	}
}

static void or(env)		/* Handle logical or (||) */
register ENV *env;
{
	ENV env2;

	and(env);
	while (iftoken(OR)) {
		rvalue(env);
		env2.sp = env->sp;
		or(&env2);
		rvalue(&env2);
		env->value = env->value || env2.value;
	}
}

static void and(env)	/* Handle logical and (&&) */
register ENV *env;
{
	ENV env2;

	eq(env);
	while (iftoken(AND)) {
		rvalue(env);
		env2.sp = env->sp;
		and(&env2);
		rvalue(&env2);
		env->value = env->value && env2.value;
	}
}

static void eq(env)		/* Handle equal (==) and not equal (!=) */
register ENV *env;
{
	register short tok;
	ENV env2;

	le(env);
	while (TRUE)
		switch (tok = nexttoken()) {
			case EQUALTO:
				rvalue(env);
				env2.sp = env->sp;
				eq(&env2);
				rvalue(&env2);
				env->value = env->value == env2.value;
				break;
			case NOTEQUAL:
				rvalue(env);
				env2.sp = env->sp;
				eq(&env2);
				rvalue(&env2);
				env->value = env->value != env2.value;
				break;
			default:
				tptr--;
				return;
		}
}

static void le(env)		/* Handle relational operators: <= >= < > */
register ENV *env;
{
	register short tok;
	ENV env2;

	plus(env);
	while (TRUE)
		switch (tok = nexttoken()) {
			case LE:
				rvalue(env);
				env2.sp = env->sp;
				le(&env2);
				rvalue(&env2);
				env->value = env->value <= env2.value;
				break;
			case GE:
				rvalue(env);
				env2.sp = env->sp;
				le(&env2);
				rvalue(&env2);
				env->value = env->value >= env2.value;
				break;
			case LESS:
				rvalue(env);
				env2.sp = env->sp;
				le(&env2);
				rvalue(&env2);
				env->value = env->value < env2.value;
				break;
			case GREATER:
				rvalue(env);
				env2.sp = env->sp;
				le(&env2);
				rvalue(&env2);
				env->value = env->value > env2.value;
				break;
			default:
				tptr--;
				return;
		}
}

static void plus(env)			/* Handle addition and substraction */
register ENV *env;
{
	register short tok;
	register short scale;
	ENV env2;

	mult(env);
	while (TRUE)
		switch (tok = nexttoken()) {
			case PLUS:
				rvalue(env);
				env2.sp = env->sp;
				plus(&env2);
				rvalue(&env2);
				scale = ((env->ind == 1 && env->size == sizeof(INTEGER)) ||
					env->ind > 1) ? sizeof(INTEGER) : 1;
				env->value += scale * env2.value;
				break;
			case MINUS:
				rvalue(env);
				env2.sp = env->sp;
				plus(&env2);
				rvalue(&env2);
				if (env->ind && env2.ind) {		/* Pointer difference */
					if (env->ind != env2.ind)
						error(POINTERERR, EmptyStr);
					scale = ((env->ind == 1 &&
						env->size == sizeof(INTEGER)) ||
						env->ind > 1) ? sizeof(INTEGER) : 1;
					env->value = (env->value - env2.value) / scale;
					env->size = sizeof(INTEGER);
					env->ind = 0;
				} else {
					scale = ((env->ind == 1 &&
						env->size == sizeof(INTEGER)) ||
						env->ind > 1) ? sizeof(INTEGER) : 1;
					env->value -= scale * env2.value;
				}
				break;
			default:
				tptr--;
				return;
		}
}

static void mult(env)		/* Handle multiplication, division, modulo */
register ENV *env;
{
	register short tok;
	ENV env2;

	unary(env);
	while (TRUE)
		switch (tok = nexttoken()) {
			case MULTIPLY:
				rvalue(env);
				env2.sp = env->sp;
				mult(&env2);
				rvalue(&env2);
				env->value *= env2.value;
				break;
			case DIVIDE:
				rvalue(env);
				env2.sp = env->sp;
				mult(&env2);
				rvalue(&env2);
	         	if (!env2.value)
					error(DIVIDEERR, EmptyStr);
				env->value /= env2.value;
				break;
			case MODULO:
				rvalue(env);
				env2.sp = env->sp;
				mult(&env2);
				rvalue(&env2);
	         	if (!env2.value)
					error(DIVIDEERR, EmptyStr);
				env->value %= env2.value;
				break;
			default:
				tptr--;
				return;
		}
}

/*
	Check for:
	leading ++
	leading --
	unary -
	pointer indicator (*)
	address operator (&)
	trailing ++
	trailing --
*/

static void unary(env)
register ENV *env;
{
	ENV env2;

	if (iftoken(AUTOINC)) {
		unary(env);
		env2 = *env;
		rvalue(&env2);
		env2.value += ((env->ind == 1 && env->size == sizeof(INTEGER)) ||
			env->ind > 1) ? sizeof(INTEGER) : 1;
		store(env, env2.value);
		return;
	}

	if (iftoken(AUTODEC)) {
		unary(env);
		env2 = *env;
		rvalue(&env2);
		env2.value -= ((env->ind == 1 && env->size == sizeof(INTEGER)) ||
			env->ind > 1) ? sizeof(INTEGER) : 1;
		store(env, env2.value);
		return;
	}

	if (iftoken(NOT)) {
		unary(env);
		rvalue(env);
		env->value = !env->value;
		env->size = sizeof(INTEGER);
		env->ind = 0;
		env->adr = FALSE;
		return;
	}

	if (iftoken(MINUS)) {
		unary(env);
		rvalue(env);
		env->value = -env->value;
		env->size = sizeof(INTEGER);
		env->ind = 0;
		env->adr = FALSE;
		return;
	}

	if (iftoken(POINTER)) {
		unary(env);
		rvalue(env);
		if (!env->ind)
			error(POINTERERR, EmptyStr);
		--(env->ind);
		switch (env->size) {
			case 1:
				env->size = (env->ind) ? sizeof(INTEGER) : 1;
				break;
			case sizeof(INTEGER):
				env->size = sizeof(INTEGER);
				break;
			default:
				error(SYNTAX, EmptyStr);
		}
		env->adr = TRUE;
		return;
	}

	if (iftoken(ADDRESS)) {
		unary(env);
		if (!env->adr)
			error(SYNTAX, EmptyStr);
		env->size = sizeof(INTEGER);
		env->ind = 0;
		env->adr = FALSE;
		return;
	}

	variable(env);

	if (iftoken(AUTOINC)) {
		register INTEGER value;
		env2 = *env;
		rvalue(&env2);
		value = env2.value +
			(((env->ind == 1 && env->size == sizeof(INTEGER)) ||
			env->ind > 1) ? sizeof(INTEGER) : 1);
		store(env, value);
		*env = env2;
		return;
	}

	if (iftoken(AUTODEC)) {
		register INTEGER value;
		env2 = *env;
		rvalue(&env2);
		value = env2.value -
			(((env->ind == 1 && env->size == sizeof(INTEGER)) ||
			env->ind > 1) ? sizeof(INTEGER) : 1);
		store(env, value);
		*env = env2;
		return;
	}
}

static void variable(env)	/* Variables, arrays and functions */
register ENV *env;
{
	register short tok;
	register INTEGER index;
	register short size;

	primary(env);
	switch (tok = nexttoken()) {
		case LPAREN:
			tptr--;
			rvalue(env);
			if (skipping) {
				skippair(LPAREN, RPAREN);
				env->value = 1;
			} else
				env->value = pfunction((Byte *)env->value, env->sp);
			env->ind = 0;
			env->size = sizeof(INTEGER);
			env->adr = FALSE;
			break;
		case LBRACKET:
			index = expression(env->sp);
			needtoken(RBRACKET);
			rvalue(env);
			if (!env->ind)
				error(SYNTAX, EmptyStr);
			--(env->ind);
			switch (env->size) {
				case 1:
					size = (env->ind) ? sizeof(INTEGER) : 1;
					break;
				case sizeof(INTEGER):
					size = sizeof(INTEGER);
					break;
				default:
					error(SYNTAX, EmptyStr);
			}
			env->value += index * size;
			env->adr = TRUE;
			break;
		default:
			tptr--;
	}
}

static void primary(env)	/* Constants, strings and identifiers */
register ENV *env;
{
	short tok;
	register SYMBOL *sym;

	switch (tok = nexttoken()) {
		case LPAREN:
			assign(env);
			needtoken(RPAREN);
			break;
		case CONSTANT:
			env->value = a2x((Byte *)tptr);
			bypass();
			env->ind = 0;
			env->size = sizeof(INTEGER);
			env->adr = FALSE;
			break;
		case STRING:
			env->value = (INTEGER)tptr;
			bypass();
			env->ind = 0;
			env->size = sizeof(INTEGER);
			env->adr = FALSE;
			break;
		case IDENT:
			/* First check locals, then globals */
			if (!(sym = ifsymbol(env->sp, tptr, SymTop)))
				sym = findsymbol(Globals, tptr, EndGlobals);
			bypass();
			env->value = (INTEGER)&sym->value;
			/* Adjust address of char variables */
			if (sym->size == 1 && sym->ind == 0)
				env->value += sizeof(INTEGER) - 1;
			env->ind = sym->ind;
			env->size = sym->size;
			env->adr = TRUE;
			break;
		default:
			error(OUTOFPLACE, token2str(tok));
	}
}

/* ----- Skip the tokens between a matched pair ------------------------ */

static void skippair(ltok, rtok)
register Byte ltok;
register Byte rtok;
{
	register short pairct = 0;
	register Byte tok;

	needtoken(tok = ltok);
	while (TRUE) {
		if (tok == ltok)
			pairct++;
		if (tok == rtok)
			if (--pairct == 0)
				break;
		if ((tok = nexttoken()) == EOF)
			error(MATCHERR, token2str(ltok));
	}
}

/* ----- A specified token is required next ---------------------------- */

static void needtoken(tk)
register Byte tk;
{
	if (nexttoken() != tk)
		error(MISSING, token2str(tk));
}

/* ----- Test for a specified token next in line ----------------------- */

static Boolean iftoken(tk)
register Byte tk;
{
	if (nexttoken() == tk)
		return TRUE;
	--tptr;
	return FALSE;
}

/* ----- Get the next token from the buffer ---------------------------- */

static Byte nexttoken()
{
	while (*tptr == LINENO)
		tptr++;
	return *tptr++;
}

/* ----- Add a symbol to the symbol table ------------------------------ */

static SYMBOL *addsymbol(s, name, value, size, ind)
register SYMBOL *s;				/* Start of local symbol table */
register Byte *name;			/* Pointer to symbol name */
register INTEGER value;			/* Value of symbol */
register Byte size;				/* Size of value */
register Byte ind;				/* Indirection level */
{
	if (ifsymbol(s, name, SymTop))
		error(DUPL_DECLARE, name);		/* Already declared */
	s = --SymTop;
	if ((Byte *)s < StackPtr)
		error(TABLEOVERFLOW, name);		/* Symbol table full */
	s->name = name;
	s->value = value;
	s->size = size;
	s->ind = ind;
	return s;
}

/* ----- Find a symbol on the symbol table (error if not found) -------- */

static SYMBOL *findsymbol(s, sym, ends)
register SYMBOL *s;				/* Start of local symbol table */
register Byte *sym;				/* Symbol name */
register SYMBOL *ends;			/* End of local symbol table */
{
	if (!(s = ifsymbol(s, sym, ends)))
		error(UNDECLARED, sym);
	return s;
}

/* ----- Test for a symbol on the symbol table ------------------------- */

static SYMBOL *ifsymbol(s, sym, sp)
register SYMBOL *s;				/* Start of local symbol table */
register Byte *sym;				/* Symbol name */
register SYMBOL *sp;			/* End of local symbol table */
{
	while (sp < s) {
		if (!strcmp((char *)sym, (char *)sp->name))
			return sp;
		sp++;
	}
	return NULL;
}

/* ----- Post an error to the shell ------------------------------------ */

static void error(erno, s)
register enum errs erno;
register Byte *s;
{
	register Byte *p;
	register n;

	if (linenumber)
		n = linenumber;
	else {
		if (tptr < TokenBuffer || tptr >= LoMem)
			n = 0;	/* Happens if main() is not found */
		else {
			for (n = 1, p = TokenBuffer; p <= tptr; p++)
				if (*p == LINENO)
					n++;
		}
	}
	SI_Error(erno, s, n);
}

/* ----- Convert token to string (for error messages) ------------------ */

static Byte *token2str(token)
register short token;
{
	static Byte s[2];
	register Byte *p = s;

	switch (token) {
		case AUTOINC:
			*p++ = '+';
			*p++ = '+';
			break;
		case AUTODEC:
			*p++ = '-';
			*p++ = '-';
			break;
		case EQUALTO:
			*p++ = '=';
			*p++ = '=';
			break;
		case NOTEQUAL:
			*p++ = '!';
			*p++ = '=';
			break;
		case GE:
			*p++ = '>';
			*p++ = '=';
			break;
		case LE:
			*p++ = '<';
			*p++ = '=';
			break;
		case AUTOADD:
			*p++ = '+';
			*p++ = '=';
			break;
		case AUTOSUB:
			*p++ = '-';
			*p++ = '=';
			break;
		case AUTOMUL:
			*p++ = '*';
			*p++ = '=';
			break;
		case AUTODIV:
			*p++ = '/';
			*p++ = '=';
			break;
		case AND:
			*p++ = '&';
		case ADDRESS:
			*p++ = '&';
			break;
		case OR:
			*p++ = '|';
		default:
			*p++ = token;
	}
	*p = '\0';
	return s;
}

/* ----- Convert long to string ---------------------------------------- */

static void x2str(num, str)
register long num;				/* Number to convert */
register Byte *str;				/* String for result */
{
	register short n;
	register Byte nibble;
	register short flg = FALSE;

	for (n = 28; n >=0 ; n -= 4) {
		if (nibble = (num >> n) & 0x0F)
			flg = TRUE;
		if (flg)
			*str++ = nibble | 0x30;
		}
	*str = 0;
}

/* ----- Convert string to long ---------------------------------------- */

long a2x(s)
register Byte *s;
{
	register unsigned long v = 0;

	while (isspace(*s))
		s++;
	while (*s >= 0x30 && *s <= 0x3F)	/* '0' .. '?' */
		v = (v << 4) + (*s++ & 0x0F);
	return (long)v;
}
