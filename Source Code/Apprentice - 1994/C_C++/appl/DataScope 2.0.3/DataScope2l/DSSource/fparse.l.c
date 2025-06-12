# include "stdio.h"
# define replaceget(a)  fcompget(a)
# define replaceput(a,b) fcompout(a)
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) replaceput(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):replaceget(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
		{ return(TSTAR); }
break;
case 2:
		{ return(TSLASH); }
break;
case 3:
		{ return(TPLUS); }
break;
case 4:
		{ return(TMINUS); }
break;
case 5:
		{ return(TLP); }
break;
case 6:
		{ return(TRP); }
break;
case 7:
		{ return(TEQUAL); }
break;
case 8:
         { return(TCOMMA); }
break;
case 9:
{
			if (NULL == (yylval.lexv = NewPtr(strlen(yytext)+1)))
				return(-1);
			strcpy(yylval.lexv,yytext);
			return(TIDENT);
			}
break;
case 10:
	case 11:
case 12:
case 13:
	{
			sscanf(yytext,"%f",&yylval.lexc);
			return(TFLOAT);
			}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

yywrap()
	{
	return(1);

}
int yyvstop[] = {
0,

5,
0,

6,
0,

1,
0,

3,
0,

8,
0,

4,
0,

2,
0,

10,
0,

7,
0,

9,
0,

12,
0,

11,
0,

11,
12,
0,

13,
0,

12,
0,

11,
0,

11,
12,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	1,3,	1,4,	1,5,	
1,6,	1,7,	1,8,	1,9,	
1,10,	1,11,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,0,	
1,0,	1,0,	1,12,	1,0,	
1,0,	1,0,	1,13,	0,0,	
0,0,	1,13,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
1,0,	1,0,	1,0,	1,0,	
0,0,	1,0,	9,14,	9,14,	
9,14,	9,14,	9,14,	9,14,	
9,14,	9,14,	9,14,	9,14,	
16,20,	0,0,	16,20,	0,0,	
0,0,	16,21,	16,21,	16,21,	
16,21,	16,21,	16,21,	16,21,	
16,21,	16,21,	16,21,	0,0,	
1,0,	1,0,	1,0,	1,0,	
1,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
2,3,	2,4,	2,5,	0,0,	
2,7,	2,8,	2,9,	2,10,	
14,17,	14,17,	18,24,	18,24,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	2,0,	2,0,	
2,0,	2,12,	2,0,	2,0,	
2,0,	11,15,	0,0,	11,11,	
11,11,	11,11,	11,11,	11,11,	
11,11,	11,11,	11,11,	11,11,	
11,11,	0,0,	0,0,	0,0,	
14,17,	14,17,	18,24,	18,24,	
0,0,	0,0,	0,0,	11,16,	
11,16,	0,0,	0,0,	2,0,	
2,0,	2,0,	2,0,	17,22,	
2,0,	17,22,	0,0,	0,0,	
17,23,	17,23,	17,23,	17,23,	
17,23,	17,23,	17,23,	17,23,	
17,23,	17,23,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	11,16,	
11,16,	0,0,	0,0,	2,0,	
2,0,	2,0,	2,0,	2,0,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	0,0,	
0,0,	0,0,	0,0,	13,13,	
0,0,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	13,13,	
13,13,	13,13,	13,13,	15,18,	
15,18,	15,18,	15,18,	15,18,	
15,18,	15,18,	15,18,	15,18,	
15,18,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
19,25,	0,0,	19,25,	15,19,	
15,19,	19,26,	19,26,	19,26,	
19,26,	19,26,	19,26,	19,26,	
19,26,	19,26,	19,26,	20,21,	
20,21,	20,21,	20,21,	20,21,	
20,21,	20,21,	20,21,	20,21,	
20,21,	22,23,	22,23,	22,23,	
22,23,	22,23,	22,23,	22,23,	
22,23,	22,23,	22,23,	15,19,	
15,19,	24,27,	0,0,	24,27,	
0,0,	0,0,	24,28,	24,28,	
24,28,	24,28,	24,28,	24,28,	
24,28,	24,28,	24,28,	24,28,	
25,26,	25,26,	25,26,	25,26,	
25,26,	25,26,	25,26,	25,26,	
25,26,	25,26,	27,28,	27,28,	
27,28,	27,28,	27,28,	27,28,	
27,28,	27,28,	27,28,	27,28,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-128,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+0,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+9,
yycrank+0,	0,		yyvstop+11,
yycrank+50,	0,		0,	
yycrank+0,	0,		yyvstop+13,
yycrank+147,	0,		yyvstop+15,
yycrank+0,	0,		yyvstop+17,
yycrank+208,	0,		yyvstop+19,
yycrank+108,	yysvec+9,	yyvstop+21,
yycrank+283,	0,		yyvstop+23,
yycrank+65,	0,		0,	
yycrank+180,	0,		0,	
yycrank+110,	yysvec+15,	yyvstop+25,
yycrank+305,	0,		0,	
yycrank+315,	0,		0,	
yycrank+0,	yysvec+20,	yyvstop+28,
yycrank+325,	0,		0,	
yycrank+0,	yysvec+22,	yyvstop+30,
yycrank+342,	0,		0,	
yycrank+352,	0,		0,	
yycrank+0,	yysvec+25,	yyvstop+32,
yycrank+362,	0,		0,	
yycrank+0,	yysvec+27,	yyvstop+34,
0,	0,	0};
struct yywork *yytop = yycrank+419;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,01  ,'+' ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'D' ,'D' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'D' ,'D' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.1 86/07/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
