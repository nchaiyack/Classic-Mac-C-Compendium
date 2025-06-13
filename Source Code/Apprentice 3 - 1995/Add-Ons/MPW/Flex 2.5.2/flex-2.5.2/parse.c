#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 CEH (Berkeley) 27/04/95";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 10 "parse.y"
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 * 
 * The United States Government has rights in this work pursuant
 * to contract no. DE-AC03-76SF00098 between the United States
 * Department of Energy and the University of California.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* $Header: /home/daffy/u0/vern/flex/RCS/parse.y,v 2.26 95/03/05 20:37:32 vern Exp $ */


/* Some versions of bison are broken in that they use alloca() but don't
 * declare it properly.  The following is the patented (just kidding!)
 * #ifdef chud to fix the problem, courtesy of Francois Pinard.
 */
#ifdef YYBISON
/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not __GNUC__ */
#if HAVE_ALLOCA_H
#include <alloca.h>
#else /* not HAVE_ALLOCA_H */
#ifdef _AIX
 #pragma alloca
#else /* not _AIX */
#ifdef __hpux
void *alloca ();
#else /* not __hpux */
#ifdef __TURBOC__
#include <malloc.h>
#else
char *alloca ();
#endif /* not __TURBOC__ */
#endif /* not __hpux */
#endif /* not _AIX */
#endif /* not HAVE_ALLOCA_H */
#endif /* not __GNUC__ */
#endif /* YYBISON */

/* Bletch, ^^^^ that was ugly! */


#include "flexdef.h"
#include <stdarg.h>

int pat, scnum, eps, headcnt, trailcnt, anyccl, lastchar, i, rulelen;
int trlcontxt, xcluflg, currccl, cclsorted, varlength, variable_trail_rule;

int *scon_stk;
int scon_stk_ptr;

static int madeany = false;  /* whether we've made the '.' character class */
int previous_continued_action;	/* whether the previous rule's action was '|' */

/* Expand a POSIX character class expression. */
#define CCL_EXPR(func) \
	{ \
	int c; \
	for ( c = 0; c < csize; ++c ) \
		if ( isascii(c) && func(c) ) \
			ccladd( currccl, c ); \
	}

/* While POSIX defines isblank(), it's not ANSI C. */
#define IS_BLANK(c) ((c) == ' ' || (c) == '\t')

/* On some over-ambitious machines, such as DEC Alpha's, the default
 * token type is "long" instead of "int"; this leads to problems with
 * declaring yylval in flexdef.h.  But so far, all the yacc's I've seen
 * wrap their definitions of YYSTYPE with "#ifndef YYSTYPE"'s, so the
 * following should ensure that the default token type is "int".
 */
#define YYSTYPE int

#line 105 "y.tab.c"
#define CHAR 257
#define NUMBER 258
#define SECTEND 259
#define SCDECL 260
#define XSCDECL 261
#define NAME 262
#define PREVCCL 263
#define EOF_OP 264
#define OPTION_OP 265
#define OPT_OUTFILE 266
#define OPT_PREFIX 267
#define OPT_YYCLASS 268
#define CCE_ALNUM 269
#define CCE_ALPHA 270
#define CCE_BLANK 271
#define CCE_CNTRL 272
#define CCE_DIGIT 273
#define CCE_GRAPH 274
#define CCE_LOWER 275
#define CCE_PRINT 276
#define CCE_PUNCT 277
#define CCE_SPACE 278
#define CCE_UPPER 279
#define CCE_XDIGIT 280
#define YYERRCODE 256


#include "Res.h"

static const short* yylhs;
static const short* yylen;
static const short* yydefred;
static const short* yydgoto;
static const short* yysindex;
static const short* yyrindex;
static const short* yygindex;
static const short* yytable;
static const short* yycheck;


void
InitParseArrays (void)
{
	SLoad(yylhs);
	SLoad(yylen);
	SLoad(yydefred);
	SLoad(yydgoto);
	SLoad(yysindex);
	SLoad(yyrindex);
	SLoad(yygindex);
	SLoad(yytable);
	SLoad(yycheck);
}

#define YYTABLESIZE 302


#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 280
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"'\"'",0,"'$'",0,0,0,"'('","')'","'*'","'+'","','","'-'","'.'","'/'",0,0,
0,0,0,0,0,0,0,0,0,0,"'<'","'='","'>'","'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"'{'","'|'","'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR","NUMBER","SECTEND",
"SCDECL","XSCDECL","NAME","PREVCCL","EOF_OP","OPTION_OP","OPT_OUTFILE",
"OPT_PREFIX","OPT_YYCLASS","CCE_ALNUM","CCE_ALPHA","CCE_BLANK","CCE_CNTRL",
"CCE_DIGIT","CCE_GRAPH","CCE_LOWER","CCE_PRINT","CCE_PUNCT","CCE_SPACE",
"CCE_UPPER","CCE_XDIGIT",
};
char *yyrule[] = {
"$accept : goal",
"goal : initlex sect1 sect1end sect2 initforrule",
"initlex :",
"sect1 : sect1 startconddecl namelist1",
"sect1 : sect1 options",
"sect1 :",
"sect1 : error",
"sect1end : SECTEND",
"startconddecl : SCDECL",
"startconddecl : XSCDECL",
"namelist1 : namelist1 NAME",
"namelist1 : NAME",
"namelist1 : error",
"options : OPTION_OP optionlist",
"optionlist : optionlist option",
"optionlist :",
"option : OPT_OUTFILE '=' NAME",
"option : OPT_PREFIX '=' NAME",
"option : OPT_YYCLASS '=' NAME",
"sect2 : sect2 scon initforrule flexrule '\\n'",
"sect2 : sect2 scon '{' sect2 '}'",
"sect2 :",
"initforrule :",
"flexrule : '^' rule",
"flexrule : rule",
"flexrule : EOF_OP",
"flexrule : error",
"scon_stk_ptr :",
"scon : '<' scon_stk_ptr namelist2 '>'",
"scon : '<' '*' '>'",
"scon :",
"namelist2 : namelist2 ',' sconname",
"namelist2 : sconname",
"namelist2 : error",
"sconname : NAME",
"rule : re2 re",
"rule : re2 re '$'",
"rule : re '$'",
"rule : re",
"re : re '|' series",
"re : series",
"re2 : re '/'",
"series : series singleton",
"series : singleton",
"singleton : singleton '*'",
"singleton : singleton '+'",
"singleton : singleton '?'",
"singleton : singleton '{' NUMBER ',' NUMBER '}'",
"singleton : singleton '{' NUMBER ',' '}'",
"singleton : singleton '{' NUMBER '}'",
"singleton : '.'",
"singleton : fullccl",
"singleton : PREVCCL",
"singleton : '\"' string '\"'",
"singleton : '(' re ')'",
"singleton : CHAR",
"fullccl : '[' ccl ']'",
"fullccl : '[' '^' ccl ']'",
"ccl : ccl CHAR '-' CHAR",
"ccl : ccl CHAR",
"ccl : ccl ccl_expr",
"ccl :",
"ccl_expr : CCE_ALNUM",
"ccl_expr : CCE_ALPHA",
"ccl_expr : CCE_BLANK",
"ccl_expr : CCE_CNTRL",
"ccl_expr : CCE_DIGIT",
"ccl_expr : CCE_GRAPH",
"ccl_expr : CCE_LOWER",
"ccl_expr : CCE_PRINT",
"ccl_expr : CCE_PUNCT",
"ccl_expr : CCE_SPACE",
"ccl_expr : CCE_UPPER",
"ccl_expr : CCE_XDIGIT",
"string : string CHAR",
"string :",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH	YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE	YYMAXDEPTH
#else
#define YYSTACKSIZE	500
#define YYMAXDEPTH	500
#endif
#endif
int		yydebug, yynerrs, yyerrflag, yychar;
short	*yyssp;
YYSTYPE	*yyvsp;
YYSTYPE	yyval, yylval;
short	yyss[YYSTACKSIZE];
YYSTYPE	yyvs[YYSTACKSIZE];
#define yystacksize	YYSTACKSIZE
#line 769 "parse.y"


/* build_eof_action - build the "<<EOF>>" action for the active start
 *                    conditions
 */

void build_eof_action()
	{
	register int i;
	char action_text[MAXLINE];

	for ( i = 1; i <= scon_stk_ptr; ++i )
		{
		if ( sceof[scon_stk[i]] )
			format_pinpoint_message(
				"multiple <<EOF>> rules for start condition %s",
				scname[scon_stk[i]] );

		else
			{
			sceof[scon_stk[i]] = true;
			sprintf( action_text, "case YY_STATE_EOF(%s):\n",
				scname[scon_stk[i]] );
			add_action( action_text );
			}
		}

	line_directive_out( (FILE *) 0, 1 );

	/* This isn't a normal rule after all - don't count it as
	 * such, so we don't have any holes in the rule numbering
	 * (which make generating "rule can never match" warnings
	 * more difficult.
	 */
	--num_rules;
	++num_eof_rules;
	}


/* format_synerr - write out formatted syntax error */

void format_synerr( msg, arg )
char msg[], arg[];
	{
	char errmsg[MAXLINE];

	(void) sprintf( errmsg, msg, arg );
	synerr( errmsg );
	}


/* synerr - report a syntax error */

void synerr( str )
char str[];
	{
	syntaxerror = true;
	pinpoint_message( str );
	}


/* format_warn - write out formatted warning */

void format_warn( msg, arg )
char msg[], arg[];
	{
	char warn_msg[MAXLINE];

	(void) sprintf( warn_msg, msg, arg );
	warn( warn_msg );
	}


/* warn - report a warning, unless -w was given */

void warn( str )
char str[];
	{
	line_warning( str, linenum );
	}

/* format_pinpoint_message - write out a message formatted with one string,
 *			     pinpointing its location
 */

void format_pinpoint_message( msg, arg )
char msg[], arg[];
	{
	char errmsg[MAXLINE];

	(void) sprintf( errmsg, msg, arg );
	pinpoint_message( errmsg );
	}


/* pinpoint_message - write out a message, pinpointing its location */

void pinpoint_message( str )
char str[];
	{
	line_pinpoint( str, linenum );
	}


/* line_warning - report a warning at a given line, unless -w was given */

void line_warning( str, line )
char str[];
int line;
	{
	char warning[MAXLINE];

	if ( ! nowarn )
		{
		sprintf( warning, "warning, %s", str );
		line_pinpoint( warning, line );
		}
	}


/* line_pinpoint - write out a message, pinpointing it at the given line */

void line_pinpoint( str, line )
char str[];
int line;
	{
#ifdef macintosh
	fflush(stdout);
	fprintf( stderr, "File \"%s\"; Line %d\t# %s\n", infilename, line, str );
#else
	fprintf( stderr, "\"%s\", line %d: %s\n", infilename, line, str );
#endif
	}


/* yyerror - eat up an error message from the parser;
 *	     currently, messages are ignore
 */

void yyerror( msg )
char msg[];
	{
	}
#line 531 "y.tab.c"
#define YYABORT		goto yyabort
#define YYREJECT	goto yyabort
#define YYACCEPT	goto yyaccept
#define YYERROR		goto yyerrlab


static void
yyprintf (const char* format, ...)
{
	if (yydebug) {
		va_list ap;
		printf("yydebug: ");
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
	}
}


int
yyparse (void)
{
	register int yym, yyn, yystate;
#if YYDEBUG
	register char* yys;
	extern char* getenv(const char*);

	if (yys = getenv("YYDEBUG")) {
		yyn = *yys;
		if (yyn >= '0' && yyn <= '9')
			yydebug = yyn - '0';
	}
#endif

	InitParseArrays();		// CEH

	yynerrs = 0;
	yyerrflag = 0;
	yychar = (-1);

	yyssp = yyss;
	yyvsp = yyvs;
	*yyssp = yystate = 0;

yyloop:
	if (yyn = yydefred[yystate]) goto yyreduce;
	if (yychar < 0) {
		if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
		if (yydebug) {
			yys = 0;
			if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
			if (!yys) yys = "illegal-symbol";
			yyprintf("state %d, reading %d (%s)\n", yystate, yychar, yys);
		}
#endif
	}
	if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
			yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
	{
#if YYDEBUG
		yyprintf("state %d, shifting to state %d\n", yystate, yytable[yyn]);
#endif
		if (yyssp >= yyss + yystacksize - 1)
			goto yyoverflow;

		*++yyssp = yystate = yytable[yyn];
		*++yyvsp = yylval;
		yychar = (-1);
		if (yyerrflag > 0)  --yyerrflag;
		goto yyloop;
	}
	if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
			yyn <= YYTABLESIZE && yycheck[yyn] == yychar) {
		yyn = yytable[yyn];
		goto yyreduce;
	}
	if (yyerrflag) goto yyinrecovery;
#ifdef lint
	goto yynewerror;
#endif
yynewerror:
	yyerror("syntax error");
#ifdef lint
	goto yyerrlab;
#endif
yyerrlab:
	++yynerrs;
yyinrecovery:
	if (yyerrflag < 3) {
		yyerrflag = 3;
		for (;;) {
			if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
					yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE) {
#if YYDEBUG
				yyprintf("state %d, error recovery shifting to state %d\n",
						 *yyssp, yytable[yyn]);
#endif
				if (yyssp >= yyss + yystacksize - 1)
					goto yyoverflow;

				*++yyssp = yystate = yytable[yyn];
				*++yyvsp = yylval;
				goto yyloop;
			} else {
#if YYDEBUG
				yyprintf("error recovery discarding state %d\n", *yyssp);
#endif
				if (yyssp <= yyss) goto yyabort;
				--yyssp;
				--yyvsp;
			}
		}
	} else {
		if (yychar == 0) goto yyabort;
#if YYDEBUG
		if (yydebug) {
			yys = 0;
			if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
			if (!yys) yys = "illegal-symbol";
			yyprintf("state %d, error recovery discards token %d (%s)\n",
					 yystate, yychar, yys);
		}
#endif
		yychar = (-1);
		goto yyloop;
	}
yyreduce:
#if YYDEBUG
	yyprintf("state %d, reducing by rule %d (%s)\n", yystate, yyn, yyrule[yyn]);
#endif
	yym = yylen[yyn];
	yyval = yyvsp[1-yym];
	switch (yyn) {
case 1:
#line 106 "parse.y"
{ /* add default rule */
			int def_rule;

			pat = cclinit();
			cclnegate( pat );

			def_rule = mkstate( -pat );

			/* Remember the number of the default rule so we
			 * don't generate "can't match" warnings for it.
			 */
			default_rule = num_rules;

			finish_rule( def_rule, false, 0, 0 );

			for ( i = 1; i <= lastsc; ++i )
				scset[i] = mkbranch( scset[i], def_rule );

			if ( spprdflt )
				add_action(
				"YY_FATAL_ERROR( \"flex scanner jammed\" )" );
			else
				add_action( "ECHO" );

			add_action( ";\n\tYY_BREAK\n" );
			}
break;
case 2:
#line 135 "parse.y"
{ /* initialize for processing rules */

			/* Create default DFA start condition. */
			scinstal( "INITIAL", false );
			}
break;
case 6:
#line 146 "parse.y"
{ synerr( "unknown error processing section 1" ); }
break;
case 7:
#line 150 "parse.y"
{
			check_options();
			scon_stk = allocate_integer_array( lastsc + 1 );
			scon_stk_ptr = 0;
			}
break;
case 8:
#line 158 "parse.y"
{ xcluflg = false; }
break;
case 9:
#line 161 "parse.y"
{ xcluflg = true; }
break;
case 10:
#line 165 "parse.y"
{ scinstal( nmstr, xcluflg ); }
break;
case 11:
#line 168 "parse.y"
{ scinstal( nmstr, xcluflg ); }
break;
case 12:
#line 171 "parse.y"
{ synerr( "bad start condition list" ); }
break;
case 16:
#line 182 "parse.y"
{
			outfilename = copy_string( nmstr );
			did_outfilename = 1;
			}
break;
case 17:
#line 187 "parse.y"
{ prefix = copy_string( nmstr ); }
break;
case 18:
#line 189 "parse.y"
{ yyclass = copy_string( nmstr ); }
break;
case 19:
#line 193 "parse.y"
{ scon_stk_ptr = yyvsp[-3]; }
break;
case 20:
#line 195 "parse.y"
{ scon_stk_ptr = yyvsp[-3]; }
break;
case 22:
#line 200 "parse.y"
{
			/* Initialize for a parse of one rule. */
			trlcontxt = variable_trail_rule = varlength = false;
			trailcnt = headcnt = rulelen = 0;
			current_state_type = STATE_NORMAL;
			previous_continued_action = continued_action;
			in_rule = true;

			new_rule();
			}
break;
case 23:
#line 213 "parse.y"
{
			pat = yyvsp[0];
			finish_rule( pat, variable_trail_rule,
				headcnt, trailcnt );

			if ( scon_stk_ptr > 0 )
				{
				for ( i = 1; i <= scon_stk_ptr; ++i )
					scbol[scon_stk[i]] =
						mkbranch( scbol[scon_stk[i]],
								pat );
				}

			else
				{
				/* Add to all non-exclusive start conditions,
				 * including the default (0) start condition.
				 */

				for ( i = 1; i <= lastsc; ++i )
					if ( ! scxclu[i] )
						scbol[i] = mkbranch( scbol[i],
									pat );
				}

			if ( ! bol_needed )
				{
				bol_needed = true;

				if ( performance_report > 1 )
					pinpoint_message(
			"'^' operator results in sub-optimal performance" );
				}
			}
break;
case 24:
#line 249 "parse.y"
{
			pat = yyvsp[0];
			finish_rule( pat, variable_trail_rule,
				headcnt, trailcnt );

			if ( scon_stk_ptr > 0 )
				{
				for ( i = 1; i <= scon_stk_ptr; ++i )
					scset[scon_stk[i]] =
						mkbranch( scset[scon_stk[i]],
								pat );
				}

			else
				{
				for ( i = 1; i <= lastsc; ++i )
					if ( ! scxclu[i] )
						scset[i] =
							mkbranch( scset[i],
								pat );
				}
			}
break;
case 25:
#line 273 "parse.y"
{
			if ( scon_stk_ptr > 0 )
				build_eof_action();
	
			else
				{
				/* This EOF applies to all start conditions
				 * which don't already have EOF actions.
				 */
				for ( i = 1; i <= lastsc; ++i )
					if ( ! sceof[i] )
						scon_stk[++scon_stk_ptr] = i;

				if ( scon_stk_ptr == 0 )
					warn(
			"all start conditions already have <<EOF>> rules" );

				else
					build_eof_action();
				}
			}
break;
case 26:
#line 296 "parse.y"
{ synerr( "unrecognized rule" ); }
break;
case 27:
#line 300 "parse.y"
{ yyval = scon_stk_ptr; }
break;
case 28:
#line 304 "parse.y"
{ yyval = yyvsp[-2]; }
break;
case 29:
#line 307 "parse.y"
{
			yyval = scon_stk_ptr;

			for ( i = 1; i <= lastsc; ++i )
				{
				int j;

				for ( j = 1; j <= scon_stk_ptr; ++j )
					if ( scon_stk[j] == i )
						break;

				if ( j > scon_stk_ptr )
					scon_stk[++scon_stk_ptr] = i;
				}
			}
break;
case 30:
#line 324 "parse.y"
{ yyval = scon_stk_ptr; }
break;
case 33:
#line 332 "parse.y"
{ synerr( "bad start condition list" ); }
break;
case 34:
#line 336 "parse.y"
{
			if ( (scnum = sclookup( nmstr )) == 0 )
				format_pinpoint_message(
					"undeclared start condition %s",
					nmstr );
			else
				{
				for ( i = 1; i <= scon_stk_ptr; ++i )
					if ( scon_stk[i] == scnum )
						{
						format_warn(
							"<%s> specified twice",
							scname[scnum] );
						break;
						}

				if ( i > scon_stk_ptr )
					scon_stk[++scon_stk_ptr] = scnum;
				}
			}
break;
case 35:
#line 359 "parse.y"
{
			if ( transchar[lastst[yyvsp[0]]] != SYM_EPSILON )
				/* Provide final transition \now/ so it
				 * will be marked as a trailing context
				 * state.
				 */
				yyvsp[0] = link_machines( yyvsp[0],
						mkstate( SYM_EPSILON ) );

			mark_beginning_as_normal( yyvsp[0] );
			current_state_type = STATE_NORMAL;

			if ( previous_continued_action )
				{
				/* We need to treat this as variable trailing
				 * context so that the backup does not happen
				 * in the action but before the action switch
				 * statement.  If the backup happens in the
				 * action, then the rules "falling into" this
				 * one's action will *also* do the backup,
				 * erroneously.
				 */
				if ( ! varlength || headcnt != 0 )
					warn(
		"trailing context made variable due to preceding '|' action" );

				/* Mark as variable. */
				varlength = true;
				headcnt = 0;
				}

			if ( lex_compat || (varlength && headcnt == 0) )
				{ /* variable trailing context rule */
				/* Mark the first part of the rule as the
				 * accepting "head" part of a trailing
				 * context rule.
				 *
				 * By the way, we didn't do this at the
				 * beginning of this production because back
				 * then current_state_type was set up for a
				 * trail rule, and add_accept() can create
				 * a new state ...
				 */
				add_accept( yyvsp[-1],
					num_rules | YY_TRAILING_HEAD_MASK );
				variable_trail_rule = true;
				}
			
			else
				trailcnt = rulelen;

			yyval = link_machines( yyvsp[-1], yyvsp[0] );
			}
break;
case 36:
#line 414 "parse.y"
{ synerr( "trailing context used twice" ); }
break;
case 37:
#line 417 "parse.y"
{
			headcnt = 0;
			trailcnt = 1;
			rulelen = 1;
			varlength = false;

			current_state_type = STATE_TRAILING_CONTEXT;

			if ( trlcontxt )
				{
				synerr( "trailing context used twice" );
				yyval = mkstate( SYM_EPSILON );
				}

			else if ( previous_continued_action )
				{
				/* See the comment in the rule for "re2 re"
				 * above.
				 */
				warn(
		"trailing context made variable due to preceding '|' action" );

				varlength = true;
				}

			if ( lex_compat || varlength )
				{
				/* Again, see the comment in the rule for
				 * "re2 re" above.
				 */
				add_accept( yyvsp[-1],
					num_rules | YY_TRAILING_HEAD_MASK );
				variable_trail_rule = true;
				}

			trlcontxt = true;

			eps = mkstate( SYM_EPSILON );
			yyval = link_machines( yyvsp[-1],
				link_machines( eps, mkstate( '\n' ) ) );
			}
break;
case 38:
#line 460 "parse.y"
{
			yyval = yyvsp[0];

			if ( trlcontxt )
				{
				if ( lex_compat || (varlength && headcnt == 0) )
					/* Both head and trail are
					 * variable-length.
					 */
					variable_trail_rule = true;
				else
					trailcnt = rulelen;
				}
			}
break;
case 39:
#line 478 "parse.y"
{
			varlength = true;
			yyval = mkor( yyvsp[-2], yyvsp[0] );
			}
break;
case 40:
#line 484 "parse.y"
{ yyval = yyvsp[0]; }
break;
case 41:
#line 489 "parse.y"
{
			/* This rule is written separately so the
			 * reduction will occur before the trailing
			 * series is parsed.
			 */

			if ( trlcontxt )
				synerr( "trailing context used twice" );
			else
				trlcontxt = true;

			if ( varlength )
				/* We hope the trailing context is
				 * fixed-length.
				 */
				varlength = false;
			else
				headcnt = rulelen;

			rulelen = 0;

			current_state_type = STATE_TRAILING_CONTEXT;
			yyval = yyvsp[-1];
			}
break;
case 42:
#line 516 "parse.y"
{
			/* This is where concatenation of adjacent patterns
			 * gets done.
			 */
			yyval = link_machines( yyvsp[-1], yyvsp[0] );
			}
break;
case 43:
#line 524 "parse.y"
{ yyval = yyvsp[0]; }
break;
case 44:
#line 528 "parse.y"
{
			varlength = true;

			yyval = mkclos( yyvsp[-1] );
			}
break;
case 45:
#line 535 "parse.y"
{
			varlength = true;
			yyval = mkposcl( yyvsp[-1] );
			}
break;
case 46:
#line 541 "parse.y"
{
			varlength = true;
			yyval = mkopt( yyvsp[-1] );
			}
break;
case 47:
#line 547 "parse.y"
{
			varlength = true;

			if ( yyvsp[-3] > yyvsp[-1] || yyvsp[-3] < 0 )
				{
				synerr( "bad iteration values" );
				yyval = yyvsp[-5];
				}
			else
				{
				if ( yyvsp[-3] == 0 )
					{
					if ( yyvsp[-1] <= 0 )
						{
						synerr(
						"bad iteration values" );
						yyval = yyvsp[-5];
						}
					else
						yyval = mkopt(
							mkrep( yyvsp[-5], 1, yyvsp[-1] ) );
					}
				else
					yyval = mkrep( yyvsp[-5], yyvsp[-3], yyvsp[-1] );
				}
			}
break;
case 48:
#line 575 "parse.y"
{
			varlength = true;

			if ( yyvsp[-2] <= 0 )
				{
				synerr( "iteration value must be positive" );
				yyval = yyvsp[-4];
				}

			else
				yyval = mkrep( yyvsp[-4], yyvsp[-2], INFINITY );
			}
break;
case 49:
#line 589 "parse.y"
{
			/* The singleton could be something like "(foo)",
			 * in which case we have no idea what its length
			 * is, so we punt here.
			 */
			varlength = true;

			if ( yyvsp[-1] <= 0 )
				{
				synerr( "iteration value must be positive" );
				yyval = yyvsp[-3];
				}

			else
				yyval = link_machines( yyvsp[-3],
						copysingl( yyvsp[-3], yyvsp[-1] - 1 ) );
			}
break;
case 50:
#line 608 "parse.y"
{
			if ( ! madeany )
				{
				/* Create the '.' character class. */
				anyccl = cclinit();
				ccladd( anyccl, '\n' );
				cclnegate( anyccl );

				if ( useecs )
					mkeccl( ccltbl + cclmap[anyccl],
						ccllen[anyccl], nextecm,
						ecgroup, csize, csize );

				madeany = true;
				}

			++rulelen;

			yyval = mkstate( -anyccl );
			}
break;
case 51:
#line 630 "parse.y"
{
			if ( ! cclsorted )
				/* Sort characters for fast searching.  We
				 * use a shell sort since this list could
				 * be large.
				 */
				cshell( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]], true );

			if ( useecs )
				mkeccl( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]],
					nextecm, ecgroup, csize, csize );

			++rulelen;

			yyval = mkstate( -yyvsp[0] );
			}
break;
case 52:
#line 648 "parse.y"
{
			++rulelen;

			yyval = mkstate( -yyvsp[0] );
			}
break;
case 53:
#line 655 "parse.y"
{ yyval = yyvsp[-1]; }
break;
case 54:
#line 658 "parse.y"
{ yyval = yyvsp[-1]; }
break;
case 55:
#line 661 "parse.y"
{
			++rulelen;

			if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				yyvsp[0] = clower( yyvsp[0] );

			yyval = mkstate( yyvsp[0] );
			}
break;
case 56:
#line 672 "parse.y"
{ yyval = yyvsp[-1]; }
break;
case 57:
#line 675 "parse.y"
{
			cclnegate( yyvsp[-1] );
			yyval = yyvsp[-1];
			}
break;
case 58:
#line 682 "parse.y"
{
			if ( caseins )
				{
				if ( yyvsp[-2] >= 'A' && yyvsp[-2] <= 'Z' )
					yyvsp[-2] = clower( yyvsp[-2] );
				if ( yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
					yyvsp[0] = clower( yyvsp[0] );
				}

			if ( yyvsp[-2] > yyvsp[0] )
				synerr( "negative range in character class" );

			else
				{
				for ( i = yyvsp[-2]; i <= yyvsp[0]; ++i )
					ccladd( yyvsp[-3], i );

				/* Keep track if this ccl is staying in
				 * alphabetical order.
				 */
				cclsorted = cclsorted && (yyvsp[-2] > lastchar);
				lastchar = yyvsp[0];
				}

			yyval = yyvsp[-3];
			}
break;
case 59:
#line 710 "parse.y"
{
			if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				yyvsp[0] = clower( yyvsp[0] );

			ccladd( yyvsp[-1], yyvsp[0] );
			cclsorted = cclsorted && (yyvsp[0] > lastchar);
			lastchar = yyvsp[0];
			yyval = yyvsp[-1];
			}
break;
case 60:
#line 721 "parse.y"
{
			/* Too hard to properly maintain cclsorted. */
			cclsorted = false;
			yyval = yyvsp[-1];
			}
break;
case 61:
#line 728 "parse.y"
{
			cclsorted = true;
			lastchar = 0;
			currccl = yyval = cclinit();
			}
break;
case 62:
#line 735 "parse.y"
{ CCL_EXPR(isalnum) }
break;
case 63:
#line 736 "parse.y"
{ CCL_EXPR(isalpha) }
break;
case 64:
#line 737 "parse.y"
{ CCL_EXPR(IS_BLANK) }
break;
case 65:
#line 738 "parse.y"
{ CCL_EXPR(iscntrl) }
break;
case 66:
#line 739 "parse.y"
{ CCL_EXPR(isdigit) }
break;
case 67:
#line 740 "parse.y"
{ CCL_EXPR(isgraph) }
break;
case 68:
#line 741 "parse.y"
{ CCL_EXPR(islower) }
break;
case 69:
#line 742 "parse.y"
{ CCL_EXPR(isprint) }
break;
case 70:
#line 743 "parse.y"
{ CCL_EXPR(ispunct) }
break;
case 71:
#line 744 "parse.y"
{ CCL_EXPR(isspace) }
break;
case 72:
#line 745 "parse.y"
{
				if ( caseins )
					CCL_EXPR(islower)
				else
					CCL_EXPR(isupper)
				}
break;
case 73:
#line 751 "parse.y"
{ CCL_EXPR(isxdigit) }
break;
case 74:
#line 755 "parse.y"
{
			if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				yyvsp[0] = clower( yyvsp[0] );

			++rulelen;

			yyval = link_machines( yyvsp[-1], mkstate( yyvsp[0] ) );
			}
break;
case 75:
#line 765 "parse.y"
{ yyval = mkstate( SYM_EPSILON ); }
break;
#line 1379 "y.tab.c"
	}
	yyssp -= yym;
	yystate = *yyssp;
	yyvsp -= yym;
	yym = yylhs[yyn];
	if (yystate == 0 && yym == 0) {
#if YYDEBUG
		yyprintf("after reduction, shifting from state 0 to state %d\n", YYFINAL);
#endif
		yystate = YYFINAL;
		*++yyssp = YYFINAL;
		*++yyvsp = yyval;
		if (yychar < 0) {
			if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
			if (yydebug) {
				yys = 0;
				if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
				if (!yys) yys = "illegal-symbol";
				yyprintf("state %d, reading %d (%s)\n", YYFINAL, yychar, yys);
			}
#endif
		}
		if (yychar == 0) goto yyaccept;
		goto yyloop;
	}
	if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
			yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
		yystate = yytable[yyn];
	else
		yystate = yydgoto[yym];
#if YYDEBUG
	yyprintf("after reduction, shifting from state %d to state %d\n", *yyssp, yystate);
#endif
	if (yyssp >= yyss + yystacksize - 1)
		goto yyoverflow;

	*++yyssp = yystate;
	*++yyvsp = yyval;
	goto yyloop;
yyoverflow:
	yyerror("yacc stack overflow");
yyabort:
	return 1;
yyaccept:
	return 0;
}
