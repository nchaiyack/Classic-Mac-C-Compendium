
# line 3 "yfor.y"
#include "stdio.h"

struct nodet {
	int token;

	struct nodet
		*left,
		*right;

	float constv;
	char *var;			/* pointer to name of variable involved */
};

typedef struct nodet node;

float f;
node *mkone(),*nlist;


# line 25 "yfor.y"
typedef union  {
	node *n;
	float lexc;
	char *lexv;
} YYSTYPE;
# define TPLUS 257
# define TMINUS 258
# define TSTAR 259
# define TSLASH 260
# define TEQUAL 261
# define TLP 262
# define TRP 263
# define TFN 264
# define TFN2 265
# define TCOMMA 266
# define TIDENT 267
# define TFLOAT 268
# define UMINUS 269
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 103 "yfor.y"


#include "fcomp.c"

/*
*  include lexical analyzer
*/
#include "fparse.l.c"
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 14
# define YYLAST 53
short yyact[]={

   7,   2,  17,   6,   8,  11,  12,  14,  13,   9,
  10,  24,  14,  13,  25,  11,  12,  14,  13,   5,
   3,  27,  11,  12,  14,  13,   4,   1,  22,  11,
  12,  14,  13,   0,  15,  16,   0,   0,  18,  19,
  20,  21,   0,   0,  23,   0,   0,   0,   0,   0,
   0,   0,  26 };
short yypact[]={

-266,-1000,-241,-258,-228,-1000,-1000,-258,-258,-260,
-1000,-258,-258,-258,-258,-1000,-235,-258,-247,-247,
-1000,-1000,-1000,-252,-1000,-258,-242,-1000 };
short yypgo[]={

   0,  27,  26,  19,   3 };
short yyr1[]={

   0,   1,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   3,   4 };
short yyr2[]={

   0,   3,   1,   1,   3,   3,   3,   3,   2,   3,
   4,   6,   1,   1 };
short yychk[]={

-1000,  -1, 267, 261,  -2,  -3,  -4, 258, 262, 267,
 268, 257, 258, 260, 259,  -2,  -2, 262,  -2,  -2,
  -2,  -2, 263,  -2, 263, 266,  -2, 263 };
short yydef[]={

   0,  -2,   0,   0,   1,   2,   3,   0,   0,  13,
  12,   0,   0,   0,   0,   8,   0,   0,   4,   5,
   6,   7,   9,   0,  10,   0,   0,  11 };
#ifndef lint
static	char yaccpar_sccsid[] = "@(#)yaccpar 1.2 86/07/18 SMI"; /* from UCB 4.1 83/02/11 */
#endif

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps>= &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:
# line 47 "yfor.y"
{ 
			nlist = mkone(TEQUAL,yypvt[-0].n,NULL);
			nlist->var = yypvt[-2].lexv;				/* save name of the variable */
			} break;
case 4:
# line 56 "yfor.y"
{
				yyval.n = mkone(TPLUS,yypvt[-2].n,yypvt[-0].n);
				} break;
case 5:
# line 60 "yfor.y"
{
				yyval.n = mkone(TMINUS,yypvt[-2].n,yypvt[-0].n);
				} break;
case 6:
# line 64 "yfor.y"
{
				yyval.n = mkone(TSLASH,yypvt[-2].n,yypvt[-0].n);
				} break;
case 7:
# line 68 "yfor.y"
{
				yyval.n = mkone(TSTAR,yypvt[-2].n,yypvt[-0].n);
				} break;
case 8:
# line 72 "yfor.y"
{
				yyval.n = mkone(UMINUS,yypvt[-0].n,NULL);
				} break;
case 9:
# line 76 "yfor.y"
{
				yyval.n = yypvt[-1].n;
				} break;
case 10:
# line 80 "yfor.y"
{
				yyval.n = mkone(TFN,yypvt[-1].n,NULL);
				yyval.n->var = yypvt[-3].lexv;
				} break;
case 11:
# line 85 "yfor.y"
{
				yyval.n = mkone(TFN2,yypvt[-3].n,yypvt[-1].n);
				yyval.n->var = yypvt[-5].lexv;
				} break;
case 12:
# line 92 "yfor.y"
{ 
				yyval.n = mkone(TFLOAT,NULL,NULL);
				yyval.n->constv = yypvt[-0].lexc;
				} break;
case 13:
# line 97 "yfor.y"
{ 
				yyval.n = mkone(TIDENT,NULL,NULL);
				yyval.n->var = yypvt[-0].lexv;
				} break;
		}
		goto yystack;  /* stack new state and value */

	}
