/************************\
*                        *
*         Parser         *    
*                        *
\************************/

/* One pass optimizing compiler for typeless expressions
written by Gabriele Speranza on a Sinclair QL (summer 1990),
adapted to C language expressions and modified gen.c (november 1990)
quickly ported to the Macintosh (jan-feb 1991)
added comments & dynamic choice between the two calling protocols (june 92)
Algorithms used:
lexical analyzer: table lookup
expression parser: left corner parsing
optimizer: lazy evaluation & strength reduction
*/


/* the QL screen is small, and the QL keyboard is rather awkward
to use, hence a rather strange graphical appearence of the listing
and almost no original comments. Now that I'm writing on a Mac I tried
to add more comments, but more than one year has passed since I wrote
the program and these comments are necessarily imprecise */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "gc.h"

/* #define hidden static*/
#define hidden
/* i prossimi dovrebbero stare in ctype.h ma evidentemente qui non ci sono... */
#define iscsym(c) (isalnum(c)||(c)=='_')
#define iscsymf(c) (isalpha(c)||(c)=='_')


struct t token;
FILE *out;
localmem currentExpr;

static char buffer[160], *textPointer;
static jmp_buf mainProgram;

#define sTsize 64
#define predefined 6

/* table of predefined symbols:the code generator
supports constant values and dynamic variables, but
all new symbols are considered absolute (the C static
variables), hence to try the capabilities of the program
I have 3 predefined constants and 3 predefined dynamic
variables */


tabel *lastEntry,symbolTable[sTsize]={
{dyn,"x",0},{dyn,"y",0},{dyn,"z",0},
{const,"u",1},{const,"v",2},
{const,"w",0}};


/* the lexical analyzer is table driven, and here are the tables: */


static char tabBichar[][2]={
   '-','>','<','<','>','>','+','+',
   '-','-','=','=','!','=','&','&',
   '|','|','<','=','>','='};
static byte valBichar[][2]={
   'P',255,0,4,1,4,0,255,1,255,EQ,1,
   NE,1,0,2,1,2,LE,1,GE,1};
static word prioBichar[]={
   0,0100135,0100135,060161,060161,
   0115,0115,055,045,0125,0125};
static char tabMono[]={
   '+','-','&','|','*','/','%','!',
   '~','<','>','^','?',',','=' };
static byte valMono[]={
   3,3,3,3,4,4,4,0,0,1,1,3,6,0,10};
static word prioMono[]={
   0100145,0140145,0140105,0100065,
   0140155,0100155,0100155,0162,0162,
   0125,0125,0100075,034,017,020027};

/* coding of the priority:
least significant 3 bits (l.s. octal digit) the associativity: see the #define's
for (unary) prefix, (unary) postfix, leftass, rightass...;
next 9 bits (three more octal digits) are the priority: 0 is reserved, hence
up to 511 levels of precedence are possible. Only about 20 are really used for
C expressions.
The remaining 4 bits will be shifted to the "features" bytes, containing some
attributes of the operator (lval1op,lval2op,unaryToo,asgnOpToo)
The input parameter of E_parser has this same coding, but the upper four bit
have a different meaning (bits 12 to 14 are always cleared, bit 15 means "this
sub-expression is not nested within a parenthesis" and is not used currently,
but it was used in the summer 90 version which used a Pascal-like syntax rather
than a C-like one)

Since it's the lexical analyzer which determines priority and associativity of
operators, it would be extremely easy to dynamically insert and delete 
user-defined operators (e.g. using identifiers or sequences of chars which, by 
some simple rule, are guaranteed not to create ambiguities to the lexical analyzer).
Question: why do Ada and C++ allow to redefine existing operators, but not to define
	new ones ?
Answer: Ichbiah and Stroustrup never heard about left-corner parsing (the one
	used here). Probably they knew that C-Prolog allows to redefine operators,
	but Prolog means AI and AI algorithms are horribly slow and memory hungry
	(the left corner expression parser should be slightly faster and should
	use less memory than any other expression parsing algorithm)
*/



#define UNARYPRIO 0162
#define ASSIGNPRIO 027
#define ASSIGNOPFEA 022


void  lexanal(),E_parser(),
   gestioneSel(),Statement();
void Emit(char*, ...);

void main()
{
/*static word w2[4]={206,200,284,8};
static word w1[4]={260,200,22,8};*/
int width,heigth;
FILE*fopen_win();
lastEntry= &symbolTable[predefined];

#if 0
/* This program was written on the QL were windows were
supported by the OS and were compatible with a "console-like"
view as a stream of bytes. So, I defined two windows, one
for input and one for output.
On the Mac, I had to modify the source code of the ANSI
library in order to get two windows which support fprintf,
but now that solution is quite obsolete. I might have adapted
the printing module of suntar to handle two windows, but
I had more important things to do. So, you should be
satisfied by having the two windows merged into one
*/

InitConsole();
screen_size(&width,&heigth);
if(width>=640){
	config_console(44,4,"\pinput",-1,25,40,0,0,0);
	printf("\f");
	out=fopen_win(44,320,"\poutput",0,25,40,0,0);
	}
else{
	config_console(44,4,"\pinput",-1,25,40,0,0,0);
	printf("\f");
	out=fopen_win(44,256,"\poutput",0,25,40,0,0);
	}
#else
out=stdout;
#endif

/*printf("Programma di esempio\n");
printf("Parsing espressioni e\n");
printf("generazione di codice\n\n");
*/
printf("\nDemo code generator\nby Gabriele Speranza November 1990\n");
printf("Type expressions in C, for example\"2+2\",\n\"t=(a[x]-b.u)^f(y,3+s); b= y*8+1\"\n");
printf("or \"a=b?(!!c||d):(foo+=j),g(h|r)\"\n\n");

if(setjmp(mainProgram)<0)
   {
   /*printf("Riprova !!\n");*/
   printf("Try again !\n");
   fflush(out); }
for(;;){
   printf("Expr: ");
   /*getline(&buffer,sizeof(buffer));*/
   fgets(buffer,sizeof(buffer),stdin);
   if(buffer[1]=='\0'){
      fclose(out);
      return;
   }
   if(out!=stdout) fprintf(out,"\f");
   textPointer= &buffer[0];
   Statement();
   fflush(out);
   printf("\n");
}}
/*********************/
void Statement()
{
/* it does NOT parse C-like statements, only sequences of expressions separated
by ';', but above the "expression" level it's typical to find something
named "statement" */



static char *classes[]={
"immediate","*immediate","simplevar",
"*simplevar","relocatable","based",
"generated","flags"};
do{
   lexanal();
   if(token.asc!='Z'){
      E_parser(Normal|externalCall);
/*      printf("class=%s\nlval=%d CExpr=%d\n",
         classes[currentExpr.class],
         currentExpr.lval,currentExpr.isCExpr);
*/
      if(currentExpr.class==flags){
         ForceJump(&currentExpr,0);
         Emit("...");
         }
      else
         ForceA(&currentExpr);
      }
   }
while(token.asc==';');
Emit("END");
if(token.asc!='Z')errore("extra chars");
}
/**********************/

/* parameter parsing must be done in a different way according
to how parameters are passed (on the Mac, the two methods are 
called C and Pascal), since the "parse tree" must be built in
two different ways (this programs builds no parse tree, but 
conceptually it exists, simply it's destroyed during the same pass
which should construct it). This parser has both methods, with a global
variable choosing

/* hidden */void FunParameters()
{localmem first;
lexanal();
first.op='(';first.opfea=0;
InitSecOp(&first,8);
while(token.asc!=')'){
   E_parser(commaIsNotOp);
   BinaryOp(&first,7);
   if(token.asc==',')lexanal();
   else if(token.asc!=')')errore(") expected");
}
first.op=')';first.opfea=32;
BinaryOp(&first,8); lexanal();
}
/**********************/
hidden void restOfPars()
{localmem first;
first.op=')';first.opfea=0;
E_parser(commaIsNotOp);
InitSecOp(&first,11);
if(token.asc==','){
   lexanal();
   restOfPars();
   }
else if(token.asc!=')')
   errore(") expected");
first.opfea=32;
BinaryOp(&first,11);
}
/**********************/
hidden void FunParsInv()
{localmem first;
first.op='(';first.opfea=0;
lexanal();InitSecOp(&first,12);
if(token.asc!=')')
   restOfPars();
first.opfea=32;
BinaryOp(&first,12);lexanal();
}
/**********************/
hidden void RecordOffset()
{localmem first;
first.op='.';first.opfea=lval1op;
InitSecOp(&first,9);
lexanal();
if(token.asc!='I'||
   token.symTable->name[0]=='\0'||
   token.symTable->typ!=const)
      errore("field name expected");
primary(token.symTable);
BinaryOp(&first,9);
lexanal();
}
/*****************/
hidden void ArrayIndex()
{localmem first;
static localmem aux={
   '*',0,0,{0,immediate /* e il resto 0 */}};
first.op='[';first.opfea=32;
/* for(;;){ solo per indici multipli
uncomment for Pascal, which allows a[x,y]
*/
   InitSecOp(&first,9);
   lexanal(); E_parser(Normal);
/* solo per limite inf != 0
  uncomment for Pascal, where the lower limit is not 0

   aux.eval= - limite inferiore ;
   aux.op='+';
   BinaryOp(&aux,3);
   aux.op='*';
*/
   aux.eval=4; /* size dell'entit
 puntata */
   BinaryOp(&aux,4);
/* solo per indici multipli
uncomment for Pascal, which allows a[x,y]
   if(token.asc==',')
      BinaryOp(&first,9);
   else
      break;
   }
*/
match(']');
BinaryOp(&first,9);

}
/**********************/
hidden void gestioneSel()
{
extern word parsInverted;
for(;;){
   switch(token.asc){
   case 'P' /*'->'*/:
      UnaryPre('*',0);
      /* RecordOffset();
      break; */
   case '.':
      RecordOffset();
      break;
   case '[':
      ArrayIndex();
      break;
   case '(':
      if(!parsInverted)
         FunParameters();
      else
         FunParsInv();
      break;
   default:
      return;
   }}
}
/*********************/
hidden void E_parser(input)
word input;
{

/* left-corner expression parser */


if(token.priority!=0 &&
   (token.features&unaryToo)){
   token.priority=UNARYPRIO;
   if(token.asc=='&')token.features=lval1op;
   }
if(token.asc=='I'){
   primary(token.symTable);

   if(token.symTable->name[0]!='\0'){
 /* well, I must use some arbitrary criteria to tell which functions
 have a C interface and which ones have a Pascal interface... */
      parsInverted=((token.symTable->name[0])|0x20)<'m';
      lexanal();
      gestioneSel();
      }
   else
      lexanal();
   }
else if(token.asc=='('){
   lexanal(); E_parser(Normal);
   parsInverted=0;
   match(')');gestioneSel();
   /* converting it to Pascal syntax, remember that somewhere there must be a
   this.lval=false */
   }
else if((token.priority&7)==prefix){
   char c=token.asc,f=token.features;
   lexanal();
   E_parser(UNARYPRIO-prefix);
   /* that's for C, where all unary prefix operators have the same precedence,
   the highest, and right-associate with other operators at the same priority
   (*p++ means *(p++) ).
   Converting to Pascal syntax, where the precedence is not the highest, one
   should remember that a<-b is OK but a*-b is illegal; and since they
   left-associate with other operators at the same priority (-a-b means (-a)-b...)
   you should do this:
   int i=token.priority; lexanal(); E_parser(i+8-prefix);
   */
   UnaryPre(c,f);
   }
else
   errore("missing token");

/* MAIN LOOP */

{word p,t,minprio=input&0xFFF;
localmem first;

/*OK, in theory I should mask out the lowest three bits (associativity)
in both token.priority and minprio. However, not masking them and
cleverly coding the associativity field allowed me to have some
control about what happens when operators of different associativity are
placed at the same priority, and what I obtained seems better than what
happens by the "theoric" law obtained by masking those bits */

while(token.priority>=minprio){

   t=token.priority&7;
   if(t==prefix)
      errore("missing operator");
   if(t==postfix){
      p=token.asc;lexanal();
      UnaryPost(p,input);
      }
   else {
      int i=token.index;
      p=token.priority;
      first.op=token.asc;
      first.opfea=token.features;
      InitSecOp(&first,i);
      lexanal();
      switch(t){
      case leftass:
         E_parser(p+(8-leftass));break;
      case rightass:
         E_parser(p);break;
      case noass:
         /* OK, I know that C has no no-ass operators, but
         they are supported anyway. And <,>,<= and so on should have been no-ass,
         the C meaning of a<b<c is absolutely silly */
         E_parser(p+(8-noass));
         if(token.priority==p)
            errore("noass !");
         break;
      case ternary:
         E_parser(p);
         if(token.asc!=':')
            errore("misused ternary");
         BinaryOp(&first,5);
         lexanal(); E_parser(p);
         break;
      default:
         errore("???");
      }
      BinaryOp(&first,i);
      }
   }
}
}
/*********************/
hidden void lexanal()
{
/* a very very simple lexical analyzer, containing a very simple
symbol table handling
*/


char *p=textPointer;
static tabel lit={const,{'\0'},0};

token.priority=0;
while(*p>0&&isspace(*p)) p++;
if(*p=='\0')
   {token.asc='Z';token.index=255;}
else if(*p<0)
   {token.asc= *p++;token.index=255;}
else if(*p>0&&isdigit(*p)){
   lit.val=0;
   do
       lit.val=lit.val*10+ *p++ -'0';
   while(*p>0&&isdigit(*p));
   token.asc='I';token.index=255;
   token.symTable=&lit;
   }
else if(*p>0&&iscsymf(*p)){ /*car iniziale id */
   char *p1=&(lastEntry->name[0]);
   while(*p>0&&iscsym(*p)) /*car non iniziale id*/
      *p1++ = *p++;
   *p1='\0';
   token.symTable= &symbolTable[0];
   while(strcmp(token.symTable->name,lastEntry->name))
      token.symTable++;
   if(token.symTable==lastEntry)
      {lastEntry->typ=ass;
      if(lastEntry==&symbolTable[sTsize-1])
         errore("sym table full");
      lastEntry++;
      }
   token.asc='I';token.index=255;
   }
else{
   char ricerca[2]; int i=0;

   ricerca[0]=*p++;ricerca[1]=*p;
   do
      if(*((word*)&ricerca[0])==*((word*)&tabBichar[i]))
         break;
   while(++i<sizeof(tabBichar)/2);
   if(i<sizeof(tabBichar)/2){
      token.asc=valBichar[i][0];
      token.index=valBichar[i][1];
      token.priority=prioBichar[i];
      p++;
      }
   else{
      token.asc=ricerca[0];
      i=0;
      do
         if(token.asc==tabMono[i])
            break;
      while(++i<sizeof(tabMono));
      if(i<sizeof(tabMono)){
         token.index=valMono[i];
         token.priority=prioMono[i];
         if(token.index==1)
            token.asc=token.asc=='<'?LT:GT;
         }
      else
         token.index=255;
   }
   token.features=0;
   if(token.priority!=0){ /* operatore... */
      token.features=(token.priority>>12)&0xF;
      token.priority&=0xFFF;
      if(*p=='='&&(token.features&asgnOpToo)){
         token.features=ASSIGNOPFEA;
         token.priority=ASSIGNPRIO;
         p++;
         }
      }

}
textPointer=p;
}
/*******************/
match(c)
char c;
{
static char p[]="missing  ";
if(token.asc==c)
   lexanal();
else{
   p[sizeof(p)-2]=c;
   errore(p);
   }
}
/*******************/
errore(mesg)
char *mesg;
{
printf("\n%s\n",mesg);
printf("%s\n",textPointer);
longjmp(mainProgram,-1);
exit(0);  /*inutile...*/
}
