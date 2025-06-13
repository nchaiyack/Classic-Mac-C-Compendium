/*****************************\
*                             *
*      Code Generator         *
*                             *
\*****************************/




/*
Question: why in most compilers it's so difficult to see the generated code ?
Answer: because the generated code is often so bad that if you should examine it
	carefully, you would think that it's better to try another compiler
This code generator shows its code on screen, hence its code had to be
tolerably good, but I wanted to write a simple one-pass code generator,
and the only well-known one-pass optimizing technique is peephole optimization.
Choosing an accumulator-based architecture helped, since there is no
need for any register allocation strategy, but that was not enough.
This program uses a few techniques, including a very restricted form
of peephole optimization, but the big work is done in another way.
Most compilers generate bad code knowing that later optimizing passes will
modify it, but this code generator had to be one-pass, hence it had to
identify bad code BEFORE generating it.
So, I chose a rarely used method, the "lazy evaluation" or "approching
deadline" technique.
Really, that philosophy sounds familiar to me, and I prefer to call it
the "lazy Italian" technique. You see, Italians are of the
same race as Mexicans, and maybe you remember those movies with a peone
who must do a work: he takes out the working tools, then he thinks:
today I've done enough, I'll do the rest tomorrow. And tomorrow he
does a siesta. But Italians are slightly different, when the deadline
approaches they remember that they belong to the most civil people in the
world, and they know that they must do the job, so they do it and they do
it wonderfully well. Obviously, since so much work has accumulated they may
use some technique which would not be possible otherwise, exploiting the
mass of things to do: this code generator realizes that the delayed work
could benefit from the new information it gathered in the meantime and
exploits it.

More technically: an expression is NOT left in a fixed state: there is a
standard struct which may say "I've generated all the code to compute
the expression into register A (Accumulator) or X (Index), but it might
also say "no code was generated, all the informations are stored in this
struct" or a mixed situation (some code was generated, but it did not
compute the final expression, only a part of it, the rest is stored in the
struct).
There is only one struct, but for a two-operand operator there are,
temporarily, two. The two operands and the operation on them must always be
represented by a single resulting struct when one arrives to their end (the
deadline), and that could oblige to generate code, but (lazy attitude) the
program never generates more code than is strictly necessary to bring the
expression to a valid state of the struct, and tries all ways (i.e.
optimizations !) to avoid generating that code, or to avoid generating many
instructions.
Then there are some functions as ForceA, ForceGenerated etc. that oblige
the code generator to bring the struct to a state or to one of a group
of states (obviously the one that requires less work...)

The possible states are:
immediate: the value of the expression is stored in the struct (rvalue only)
*immediate or immediate+1: the absolute, nonrelocatable address of the
	variable containing the value of the expression is stored in the
	struct (l/rvalue)
simplevar: the struct stores a pointer to a symbol table entry which
	describes the variable, either absolute or dynamic (l/rvalue), plus
	an offset (usually 0)
*simplevar or simplevar+1: as above, but with an added indirection,
	the variable contains the address of the variable  (good for
	VAR-parameter passing) (l/rvalue)
relocatable: the value is a constant value plus the address of an absolute
	variable (the pointer to symbol table is stored as a 16-bit
	offset) minus another absolute address (rvalue only)
based: the program generated code to store a value into X, to which it
	must add the fixed offset stored in the struct in order to
	get the address of the variable containing the value (l/rvalue)
generated: the program generated code to store a value into A or X, the
	struct contains only a boolean to tell which register (rvalue only)
flags: the boolean expression is true if the generated code during execution
	will jump to labelTrue or if a conditional branch placed AFTER all
	the previously generated code will find true the "condition", it's
	false in case of a jump to labelFalse or if the condition is false
	(rvalue only). That seems complex, but it's the key to the optimization
	of && and ||.
Finally, there are two temporary states that are NOT coded in the struct
(the function which generated them must remember that):
Stack: as generated, followed by the generation of a PUSH
Jump: as flags, followed by the generation of a branch testing "condition"


Written by Gabriele Speranza, summer 1990
modified by rearranging and enlarging the set of possible
expression states, november 1990
*/


#include <stdio.h>
#include <string.h>

#include "gc.h"




/* #define hidden static */
#define hidden /**/
/* i soliti problemi di librerie non del tutto compatibili: */

void movmem(char*, char*, int);
void movmem(src, dest, len)
register char*dest,*src;
register int len;
{
	 while (--len>=0) *dest++ = *src++;
}

word newlabel();
void InitSecOp(),BinaryOp(),Bflush(),BEmit();

static word lastWasLabel=0;
static char *AX[2]={"A","X"};

/*********************/
hidden void AddressingMode(expr)
localmem*expr;
{
switch(expr->class){
case immediate:
    fprintf(out,"#%ld",expr->eval);break;
case immediate+1:
    fprintf(out,"%ld",expr->eval);break;
case simplevar:
    if(expr->displacement!=0)
       fprintf(out,"%d+",expr->displacement);
    fprintf(out,"%s",expr->sTable->name);
    if(expr->sTable->typ==dyn)
       fprintf(out,"(SP)");
    break;
case relocatable:
    fprintf(out,"#");
    if(expr->symplus==0)
       fprintf(out,"%ld",expr->eval);
    else{
       if(expr->eval!=0)
          fprintf(out,"%ld+",expr->eval);
       {char*p=TOLONG(expr->symplus);
       fprintf(out,"%s",((tabel*)p)->name);}
       }
    if(expr->symminus!=0){
       char*p=TOLONG(expr->symminus);
       fprintf(out,"-%s",((tabel*)p)->name);
       }
    break;
case based:
    if(expr->displacement==0)
       fprintf(out,"(X)");
    else
       fprintf(out,"%d(X)",expr->displacement);
    break;
case generated:
    fprintf(out,"%s",AX[expr->place]);
    break;
default:
/**/
printf("%d %lx",expr->class,expr);
    errore("???");
}}
/****************************/


#include <stdarg.h>


void Emit(char *control, ...)

{
va_list ap;

va_start(ap,control);


Bflush();
if(!lastWasLabel)
   fprintf(out,"    ");
else
   lastWasLabel=0;
while(*control!='\0'){
   if(*control=='%'){
      control++;
      if(*control=='n')
         fprintf(out,"\n    ");
      else if(*control=='s')
         fprintf(out,"%s",va_arg(ap,char*));
      else if(*control=='e')
         AddressingMode(va_arg(ap,localmem*));
/*      else if(*control=='l')
         fprintf(out,"L%d",va_arg(ap,int));
*/
      else if(*control=='d')
         fprintf(out,"%ld",va_arg(ap,long));
      else
      /**/{
      printf("control=%c",*control);
         errore("???");	}
      }
   else
      fprintf(out,"%c",*control);
   control++;
   } /* end while */
va_end(ap);
fprintf(out,"\n");
fflush(out);
}
/*******************/

/* an assembler usually accepts a forward reference, but it does not
accept a forward reference to a forward reference, hence I can't simply
emit EQU directives, I must store them until the label is emitted */


static word EQUtab[10][2];
static int lastEQU=0;

hidden void partition(lab)
word lab;
{int i;
for(i=lastEQU-1;i>=0;i--)
   if(EQUtab[i][1]==lab)
      if(--lastEQU!=i){
         word t;
         EQUtab[i][1]=EQUtab[lastEQU][1];
         t=EQUtab[i][0];
         EQUtab[i][0]=EQUtab[lastEQU][0];
         EQUtab[lastEQU][0]=t;
         }
}

hidden void LEmit(lab)
word lab;
{int i,f;
if(lab==0)return;
Bflush();
if(lastWasLabel)fprintf(out,"\n");
else lastWasLabel=1;
fprintf(out,"L%d:",lab);
if(lab<=9)fprintf(out," ");
f=lastEQU; partition(lab);
for(i=lastEQU;i<f;i++)
   LEmit(EQUtab[1][0]);
}

hidden void EQUforward(l1,l2)
word l1,l2;
{
if(lastEQU==10)errore("too complex logic expr");
EQUtab[lastEQU][0]=l1;
EQUtab[lastEQU++][1]=l2;
}

hidden void EQUbackwards(l1,l2)
word l1,l2;
{int i,f;
if(lastWasLabel){
   fprintf(out,"\n");
   lastWasLabel=0;}
fprintf(out,"L%d %sEQU L%d",l1,l1<=9?" ":"",l2);
f=lastEQU; partition(l1);
for(i=lastEQU;i<f;i++)
   EQUbackwards(EQUtab[1][0],l2);
}
/********************/
static byte bufferFull=0;
static byte bufferVal;
static localmem* bufferPt;

/* PUSHs and BRanches are not emitted immediately, they are stored
in a buffer from which is flushed at the next Emit, but they may also
be annihilated before being flushed.
That's not only an optimization, it's essential to follow the rule that
a costant expression may be used in every place a literal may be, including
places (e.g. declarations) where code must NOT be generated, since the
expression will only be entered as an attribute in the symbol table. And
I could not avoid to call a LazyPUSH or LazyJMP before knowing whether
the current expression is constant or not.
*/

hidden void LazyPUSH(plac)
{
Bflush();
bufferFull=1;
bufferVal=plac;
}
LazyPOP(plac)
{
if(!bufferFull)
   Emit("POP %s",(long)AX[plac]);
else
   {if(plac!=bufferVal)
      Emit("MOV %s,%s",(long)AX[plac],(long)AX[bufferVal]);
   bufferFull=0;
   }
}
hidden void LazyJMP(expr)
localmem*expr;
{if(bufferFull&&bufferVal==16+T)
   expr->auxlabel=0xFFFF;
else{
   expr->auxlabel=0;
   Bflush();
   bufferFull=1;
   bufferVal=16+T;
   bufferPt=expr;
   }
}
hidden void LazyLabel(expr)
localmem*expr;
{
if(expr->auxlabel==0xFFFF)return;
if(bufferFull&&bufferPt==expr)
   bufferFull=0;
else
   LEmit(expr->auxlabel);
}
void Bflush()
{
if(!bufferFull)return;
bufferFull=0;
if(bufferVal<16)
   Emit("PUSH %s",(long)AX[bufferVal]);
else
   BEmit(bufferVal&0xF,
      bufferPt->auxlabel=newlabel());

}

hidden void BEmit(cond,lab)
{
static char *brCode=
   "NOP\0BRA\0BEQ\0BNE\0BLE\0BGT\0BLT\0BGE";
if(cond==F)return; /* niente NOP...*/
Bflush();
if(!lastWasLabel)
   fprintf(out,"    ");
else
   lastWasLabel=0;
fprintf(out,"%s L%d\n",&brCode[cond<<2],lab);
}
/*******************************/
#define this currentExpr
#define right currentExpr
#define left (*first)
#define son currentExpr
#define issimple(x) (x.class<4)
#define islazy(x) (x.class<5)
#define INVERSE(x) ((x)^1)
#define Flabel gci.aa.Label[0]
#define Tlabel gci.aa.Label[1]
static byte bitarray[]={5,0,4,2,1,2,2,0};
#define isimr(x) (bitarray[x.class]&1)
#define usesX(x) (bitarray[x.class]&2)
#define isimsim(x) (bitarray[x.class]&4)

/********************/
void ForceNative(expr)
localmem*expr;
{if(expr->class==simplevar+1){
   expr->class=simplevar;
   Emit("LOAD X,%e",expr);
   expr->class=based;expr->displacement=0;
   }
}
/********************/
void ForceRvalue(expr)
localmem*expr;
{if(expr->lval)
   expr->lval=expr->isCExpr=0;
}
/**********************************/
void ForceA(expr)  /* solo per rvalue... */
localmem*expr;
{
if(expr->class==generated){
   if(expr->place!=0)
      {Emit("MOV A,X");expr->place=0;}
   return;
   }
else if(expr->class!=flags){
   if(expr->class==immediate&&expr->eval==0)
      Emit("CLR A");
   else{
      ForceNative(expr);
      Emit("LOAD A,%e",expr);
      }
   expr->isCExpr=0;
   }
else{  /* flags */
   if(expr->condition<=T&&expr->Flabel==0&&expr->Tlabel==0)
      Emit(expr->condition==T?"LOAD A,#1":"CLR A");
   else{
      word t=newlabel();
      if(expr->Flabel==0)
         if(expr->Tlabel==0){
            Emit("CLR A");
            BEmit(INVERSE(expr->condition),t);
            Emit("INC A");
            }
         else{
            Emit("CLR A");
            BEmit(INVERSE(expr->condition),t);
            LEmit(expr->Tlabel);
            Emit("LOAD A,#1");
            }
      else  /* Flabel!=0 */
         if(expr->Tlabel==0){
            Emit("LOAD A,#1");
            BEmit(expr->condition,t);
            LEmit(expr->Flabel);
            Emit("CLR A");
            }
         else{
            BEmit(INVERSE(expr->condition),expr->Tlabel);
            LEmit(expr->Flabel);
            Emit("CLR A");
            BEmit(T,t);
            LEmit(expr->Tlabel);
            Emit("LOAD A,#1");
            }
      LEmit(t);
      }
   }
expr->class=generated;
expr->place=0;
}
/********************/
void ForceX(expr) /* solo Rvalue !!! */
localmem*expr;
{
if(expr->class==flags)
   ForceA(expr);
if(expr->class==generated){
   if(!expr->place)
      {Emit("MOV X,A");expr->place=1;}
   return;
   }
ForceNative(expr);
Emit("LOAD X,%e",expr);
expr->class=generated;
expr->place=1;expr->isCExpr=0;
}
/********************/
void ForceFlags(expr)
localmem*expr;
{
if(expr->class==flags)return;
if(expr->class==immediate)
   expr->condition=expr->eval?T:F;
else{
   ForceNative(expr);
   if(expr->class==generated&&expr->place==1)
      ForceA(expr); /* TEST X non esistente */
   Emit("TEST %e",expr);
   expr->condition=NE;
   }
expr->class=flags;
expr->Flabel=expr->Tlabel=expr->isCExpr=0;
}
/********************/
void ForceBased(expr)  /* solo per lvalue ! */
localmem *expr;
{ForceNative(expr);
if(expr->class==based)return;
Emit("LEA %e",expr);
expr->class=based;
expr->displacement=expr->isCExpr=0;
}
/********************/
void ForceStack(expr)
localmem*expr;
{
if(expr->class==generated&&expr->place!=0)
   LazyPUSH(1);
else{
   ForceA(expr); LazyPUSH(0);
   }
/* expr->place=-1; */
}
/********************/
void ForceJump(expr,direction)
localmem*expr;
{
ForceFlags(expr);
if(!expr->label[direction])
   expr->label[direction]=newlabel();
BEmit(expr->condition^(1-direction),
   expr->label[direction]);
LEmit(expr->label[1-direction]);
/* expr->label[1-direction]=0;
   expr->condition=-1; */
}
/********************/
hidden word newlabel()
{
static word lastLab=0;
return ++lastLab;
}
hidden void SuppressLabels(expr)
localmem*expr;
{if(expr->class!=flags)return;
LEmit(expr->Flabel);LEmit(expr->Tlabel);
/* expr->Flabel=expr->Tlabel=0; */
}
/*********************************/
void primary(symbolTable)
tabel *symbolTable;
{
if(symbolTable->typ==const){
   this.class=immediate;
   this.eval=symbolTable->val;
   this.lval=0;this.isCExpr=1;
   }
else{
   this.class=simplevar;
   this.sTable=symbolTable;
   this.displacement=0;
   this.lval=1;
   this.isCExpr=symbolTable->typ==ass;
   }
}
/****************/
void UnaryPre(op,features)
char op;
{
if(features&lval1op)
   {if(!son.lval)errore("lvalue expected");}
else
   ForceRvalue(&son);
/* if(op=='+') in C non c'
   ForceRvalue(&son);
else
*/
if(op=='-')
   if(isimr(son)){
      word t=this.symplus;
      this.symplus=this.symminus;
      this.symminus=t;
      son.eval=-son.eval;
      }
   else{
      ForceA(&son);Emit("NEG");}
else if(op=='~')
   if(son.class==immediate)
      son.eval=~son.eval;
   else{
      ForceA(&son);Emit("NOT");}
else if(op=='&'){
   if(issimple(son))
      if(son.class&1)
         son.class--;
      else if(son.sTable->typ==ass){
         this.class=relocatable;
         this.symplus=(word)(TOWORD(son.sTable));
         this.eval=son.displacement;
         this.symminus=0;
         }
      else{   /* dyn */
         ForceBased(&son);
         this.class=generated;this.place=1;
         }
   else{  /* based */
      if(this.displacement!=0)
         Emit("LEA %e",&son);
      this.class=generated;this.place=1;
      }
   this.lval=0;
   }
else if(op=='*'){
   if(isimsim(son)){  /*immediate,simplevar*/
      if(son.class!=immediate)son.isCExpr=0;
      son.class++;}
   else if(son.class==relocatable&&son.symminus==0){
      this.class=simplevar;this.displacement=son.eval;
      {char*p=TOLONG(son.symplus);
      this.sTable=((tabel*)p);}
      }
   else{
      ForceX(&son);
      this.class=based;this.displacement=0;
      }
   this.lval=1;
   }
else if(op=='!')
   if(son.class==immediate)
      this.eval=!son.eval;
   else{
      word t;
      ForceFlags(&son);
      son.condition ^=1; /* cio =INVERSE(son.condition) */
      t=son.Flabel;
      son.Flabel=son.Tlabel;
      son.Tlabel=t;
      }
else{ /* ++ e -- */
   ForceNative(&son);
   Emit(op==0?"INC %e":"DEC %e",&son);
   ForceRvalue(&son);
   }
}
/****************/
void UnaryPost(op,input)
char op;
{
ForceNative(&this);
if(!(input&externalCall)||token.asc!=';')
   Emit("LOAD A,%e",&son);
Emit(op==0?"INC %e":"DEC %e",&son);
this.class=generated;
this.lval=this.isCExpr=this.place=0;
}
/****************/



/* for binary operators, the parser calls two code generator routines in
two places:
ISO_... is called after the first operand, when the operator is known but
	nothing is known about the second operand. "first" points to a
	struct describing the left operand, which has also a few fields which
	the code generator may use for its local variables
BIN_... is called after the second operand, "first" points to the same
	struct passed to ISO_..., the global variable "right" describes
	the right operand, and the result of the operation must be stored
	int "this" (physically overlayed with "right")
For unary operators there is only one call (in theory there could be
two calls for prefix operators, but no operator had anything which needed
to be done before the operand)
Also the ternary operator and the parameter passing are transformed into 
a binary parse tree, but BIN_... may be called many times after a single ISO_...

*/

#if NOT_OPTIMIZING

/* not optimazed versions: don't try to read any of the
optimazing routines without understanding the general
rules followed by the non-optimazing routines */

/*  non-optimizing version NON OPERATIVO */
hidden char* opCode(op)
char op;
{
static char
tab[]={'+','-','|','&','^'};
static char tab1[]=
   "ADD\0SUB\0OR\0\0AND\0XOR\0ERROR";
 i=0;
while(tab[i]!=op&&i<5) i++;
return &tab1[i<<2];
}

hidden void ISO_plusminus(first)
localmem*first;
{
/* semplificato, senza ottimizzazioni... */
ForceStack(&left);
}
hidden void BIN_plusminus(first)
localmem*first;
{
ForceA(&right);
if(left.op=='-')
   {Emit("NEG");left.op='+';}
/* else if(op non commutativo)
   Emit("EXC A,(SP)"); */
Emit("%s (SP)+",opCode(left.op));
}

/*******************/
/* non-optimizing version NON OPERATIVO */
hidden void ISO_andOr(first)
localmem*first;
{
ForceJump(&left,left.op);
}
hidden void BIN_andOr(first)
localmem*first;
{
ForceFlags(&right);
if(right.label[left.op]==0)
   right.label[left.op]=left.label[left.op];
else
   EQUforward(left.label[left.op],right.label[left.op]);
}
/*******************/
/* non-optimizing version NON OPERATIVO */
hidden void ISO_ternary(first)
localmem*first;
{ForceJump(&left,0);
}
hidden void BIN_1ternary(first)
localmem*first;
{
ForceA(&right);
BEmit(T,left.auxlabel=newlabel());
LEmit(left.Flabel);
}
hidden void BIN_2ternary(first)
localmem*first;
{ForceA(&right);
LEmit(left.auxlabel);
}
#endif /* NOT_OPTIMAZING */



hidden void ISO_assign(first)
localmem*first;
{if(left.class==based)
   LazyPUSH(1);
}
hidden void BIN_assign(first)
localmem*first;
{char *p;

if(usesX(right)) /* *simplevar,based o generated */
   ForceA(&right);
if(left.class==based)
   LazyPOP(1);
if(right.class==immediate&&right.eval==0)
   p="CLR";
else
   {ForceA(&right); p="STORE"; }
ForceNative(&left);
Emit("%s %e",p,&left);
this.isCExpr=0;
}
/*******************/
hidden GestioneReloc(first)
localmem*first;
{
if(left.class==immediate){
   left.symplus=left.symminus=0;
   left.class=relocatable;}
if(right.class==immediate){
   right.symplus=right.symminus=0;
   right.class=relocatable;}
if(left.op=='-'){
   word t=right.symplus;
   right.symplus=right.symminus;
   right.symminus=t;
   right.eval=-right.eval;
   left.op='+';
   }
if(left.symplus==right.symminus){
   left.symplus=right.symminus=0;}
if(right.symplus==left.symminus){
   right.symplus=left.symminus=0;}
if(right.symplus==0){
   right.symplus=left.symplus;
   left.symplus=0;}
if(right.symminus==0){
   right.symminus=left.symminus;
   left.symminus=0;}

if(left.symplus==0&&left.symminus==0){
   right.eval+=left.eval;
   if(right.symplus==0&&right.symminus==0)
      right.class=immediate;
   return 1; /* ce l'ho fatta */
   }
return 0; /* la somma resta a run time */
}
/******************/
hidden void GenOp(op,expr)
char op;
localmem*expr;
{
static char
tab[]={'+','-','|','&','^'};
static char tab1[]=
   "ADD\0SUB\0OR\0\0AND\0XOR\0ERROR";
char*p;
int i=0;
long labs();

while(tab[i]!=op&&i<5)i++;
p=&tab1[i<<2];
if(!expr)
   Emit("%s (SP)+",p);
else if(expr->class!=immediate){
   ForceNative(expr);
   Emit("%s %e",p,expr);}
else if(i!=3&&expr->eval==0||
        i==3&&expr->eval==-1)
   ;  /* elemento neutro: +0,&-1 etc. */
else if(i<=1&&labs(expr->eval)<=1)
   if(i==(expr->eval<0))
      Emit("INC A");
   else
      Emit("DEC A");
else
   Emit("%s #%d",p,expr->eval);
}
/***************************/
hidden void ISO_generic(first)
localmem*first;
{
if(!islazy(left))ForceStack(&left);
}
hidden void BIN_plusminus(first)
localmem*first;
{
if(isimr(left)&&isimr(right)){
   if(left.class==immediate&&right.class==immediate)
      switch(left.op){
      case'+':this.eval+=left.eval;return;
      case'-':this.eval=left.eval-right.eval;return;
      case'|':this.eval|=left.eval;return;
      case'&':this.eval&=left.eval;return;
      case'^':this.eval^=left.eval;return;
      }
   if((left.op=='-'||left.op=='+')&&GestioneReloc(first)!=0)
      return;
   }
if(right.class>=generated||
   right.class==based&&left.class==simplevar+1)
   ForceA(&right);
if(left.class==generated)
   if(right.class==generated){
      if(left.op=='-')
         Emit("NEG%nADD (SP)+");
      else
         GenOp(left.op,NULL);
      }
   else{   /* generated-lazy */
      LazyPOP(0);
      GenOp(left.op,&right);}
else{  /* left==lazy */
   ForceNative(&left);
   if(right.class==generated){
      if(left.op=='-')
         {Emit("NEG");left.op='+';}
      GenOp(left.op,&left);
      }
   else{  /* lazy-lazy */
      if(left.op=='-'||left.class!=immediate){
         ForceA(&left); GenOp(left.op,&right);}
      else{
         ForceA(&right);GenOp(left.op,&left);}
      }
   }
this.class=generated;
this.place=this.lval=this.isCExpr=0;
}
/******************/
#define SWAPCOND(op) op<=3?op:11-op
hidden void BIN_compare(first)
localmem*first;
{
if(left.class==immediate&&right.class==immediate){
   switch(left.op){
   case NE:case EQ:
      this.eval=left.eval==right.eval;break;
   case GE:case LT:
      this.eval=left.eval <right.eval;break;
   case GT:case LE:
      this.eval=left.eval<=right.eval;break;
   }
   if(left.op&1)this.eval^=1;
   return;
   }
if(left.class==generated)
   if(right.class>=generated){
      ForceA(&right);
      Emit("SUB (SP)+");
      this.condition=SWAPCOND(left.op);
      }
   else{
      LazyPOP(0);
      if(right.class==immediate&&right.eval==0)
         Emit("TEST A");
      else
         GenOp('-',&right);
      this.condition=left.op;
      }
else  /* left==lazy */
   if(right.class==immediate){
      if(right.eval==0){
         ForceNative(&left);
         Emit("TEST %e",&left);}
      else{
         ForceA(&left);
         GenOp('-',&right);
         }
      this.condition=left.op;
      }
   else{
      if(left.class==immediate&&left.eval==0){
         ForceNative(&right);
         Emit("TEST %e",&right);}
      else{
         ForceA(&right);
         GenOp('-',&left);
         }
      this.condition=SWAPCOND(left.op);
      }
this.class=flags;
this.Tlabel=this.Flabel=0;
this.isCExpr=0;
}
/*******************/
hidden void ISO_offset(first)
localmem*first;
{
if(left.lval==0){
   if(!islazy(left))
      ForceStack(&left);
   else if(left.class==immediate){
      left.lval=1;left.class=immediate+1;}
   }
else
   if(left.class==based)LazyPUSH(1);
}
hidden void BIN_offset(first)
localmem*first;
{
if(left.lval){
   if(usesX(right))ForceA(&right);
   if(left.class==based)LazyPOP(1);
   if(right.class==immediate){
      if(left.class==immediate+1){
         this.eval+=left.eval;
         this.class=immediate+1;}
      else{
         ForceNative(&left);
         this.displacement=left.displacement+right.eval;
         this.class=left.class;
         this.sTable=left.sTable; /* serve solo se simplevar */
         }
      }
   else{
      ForceBased(&left);
      if(!islazy(right))ForceA(&right);
      Emit("ADDX %e",&right);
      this.class=based;
      this.displacement=left.displacement;
      this.isCExpr=0;
      }
   }
else{  /* rvalue */
   if(left.class==relocatable&&right.class==immediate){
      left.eval+=right.eval;
      ForceX(&left);
      this.class=based;this.displacement=0;
      }
   else{
      if(usesX(right))ForceA(&right);
      if(!islazy(left))
         LazyPOP(1);
      else
         ForceX(&left);
      if(!islazy(right))ForceA(&right);
      if(right.class!=immediate){
         Emit("ADDX %e",&right);
         this.displacement=0;
         }
      else
         this.displacement=right.eval;
      this.class=based;
      }
   this.isCExpr=0;
   }
this.lval=1;
}
/*******************************/


/* there are TWO ways to pass parameters on the stack:
Pushing first the first parameter
Pushing first the LAST parameter
The second way is necessary to implement the functions with a variable
number of parameters of C (e.g. printf), the first one is simpler, 
hence it's used when there is no printf to implement
Note: this routines were written on a QL, not on the Mac. The C-like
parameter passing method implemented is compatble with C compilers
on the Mac, but the Pascal-like method does not account another
feature of Pascal compilers for the Mac (and the Toolbox in ROM),
which pass the return value on the stack rather than in a register
(really, on paper I wrote the MC68000 equivalents of the
abstract-accumulator-machine instructions which are generated by
this program, but I never found the time to type them)
*/


/* Di questa routine ne esistono due
versioni, che pongono i parametri
sullo stack in ordine diretto o
inverso: c' bisogno anche di un
parser leggermente diverso e la
word parsInverted lo seleziona */

/* PRIMO PARAMETRO PRIMO SULLO STACK 
   (come nel C della GST) (as in Macintosh Pascal, but the 
   			return value is in accumulator)
*/
word parsInverted=0;

hidden void ISO_funCall(first)
localmem*first;
{
if(islazy(left))
   left.counter=0;
else{
   ForceStack(&left);
   left.counter=2;
   }
}
#define ISO_funpar null
hidden void BIN_funpar(first)
localmem*first;
{
ForceStack(&right);
left.counter+=2;
}
hidden void BIN_funCall(first)
localmem*first;
{

if(left.class==simplevar&&left.sTable->typ==ass||
   left.class==immediate){
   if(left.class==immediate)
      left.class=immediate+1;
   Emit("CALL %e",&left);
   }
else{
   if(!islazy(left)){
      if(left.counter==2)
         { LazyPOP(1);left.counter=0;}
      else
         Emit("LOAD X,%d(SP)",(long)left.counter-2);
      }
   else
      ForceX(&left);
   Emit("CALL (X)");
   }
if(left.counter!=0)Emit("ADD SP,#%d",(long)left.counter);
this.class=generated;
this.lval=this.place=this.isCExpr=0;
}
/* */
/***********************/
/* ULTIMO PARAMETRO PRIMO SULLO STACK (as in Macintosh Think C)
   (come nel C MetaComco) */

static word parOffset,parLname;

hidden word newlname()
{static word lastName=0;
return ++lastName;
}
void QEmit(lab,val)
word lab,val;
{
if(lastWasLabel){
   fprintf(out,"\n");lastWasLabel=0;}
fprintf(out,"C%d%sEQU %d\n",lab,lab<=9?"  ":" ",val);
}
hidden void ISO_funCall_inv(first)
localmem*first;
{
if(!islazy(left))ForceStack(&left);
left.counter=parOffset;parOffset=0;
left.auxLname=parLname;parLname=0;
}
hidden void ISO_funpar_inv(first)
localmem*first;
{
if(parLname==0)
   if(islazy(left))
      ;
   else if(token.asc==')'){    /* Wow,per un pelo ! */
      ForceStack(&left);
      parOffset=2;
      }
   else{ /* devo usare il modo costoso */
      Emit("SUB SP,#C%d",(long)(parLname=newlname()));
      ForceStack(&left);
      parOffset=2;
      }
else{
   ForceA(&left);
   Emit("STORE %d(SP)",(long)parOffset);
   parOffset+=2;
   }
}
hidden void BIN_funpar_inv(first)
localmem*first;
{
if(!islazy(left))return;
if(parLname!=0){
   QEmit(parLname,parOffset-2);
   parLname=0;
   }
ForceStack(&left);
parOffset+=2;
}
hidden void BIN_funCall_inv(first)
localmem*first;
{
if(parLname!=0)
   QEmit(parLname,parOffset-2);
if(!islazy(left))parOffset+=2;
if(left.class==simplevar&&left.sTable->typ==ass||
   left.class==immediate){
   if(left.class==immediate)
      left.class=immediate+1;
   Emit("CALL %e",&left);
   }
else{
   if(!islazy(left)){
      if(parOffset==2)
         { LazyPOP(1);parOffset=0;}
      else
         Emit("LOAD X,%d(SP)",(long)parOffset-2);
      }
   else
      ForceX(&left);
   Emit("CALL (X)");
   }
if(parOffset!=0)Emit("ADD SP,#%d",(long)parOffset);
this.class=generated;
this.lval=this.place=this.isCExpr=0;
parOffset=left.counter;parLname=left.auxLname;
}

/***********************************/
hidden int power2(val)
long val;
{int i;
if(val<=0)return -1;
for(i=0;!(val&1);val>>=1)i++;
if(val==1)return i;
else return -1;
}

hidden void GenMDS(op,expr)
char op;
localmem*expr;
{
static char tab[]={'*','/','%',0,1},
   *tab1[]={"MULT","DIV","MOD","LSHIFT","RSHIFT"},
   tab2[]="ASL\0ASR";
int i,j;
long val;
for(i=0;tab[i]!=op&&i<5;i++) ;
if(expr==NULL)
   {Emit("CALL %s",tab1[i]);
   return;}
val=expr->eval;
if(i<=1){  /* * e / */
   if(val==1)
      return;
   else if((j=power2(val))>=0){
      i+=3;val=j;}
   else if(i==0){
      if(val<0&&(j=power2(-val))>=0){
         Emit("NEG");
         i=3;val=j;}
      else if(val==0)
         {Emit("CLR A");return;}
      else if(val==3){
         Emit("PUSH A%nADD a%nADD (SP)+");
         return;}
      }
   }
if(i==2){  /* MOD */
   if((j=power2(val))>=0)
      {Emit("AND #%d",~(~(long)0<<j));
      return;}
   }
else if(i>=3){
   if(val<0){
      val=-val;i=7-i;}
   if(val<=3){
      while(--val>=0)
         Emit(&tab2[-12+(i<<2)]);
      return;}
   }
Emit("PUSH A%nLOAD A,#%d%nCALL %s",val,tab1[i]);
}
/*******************/
hidden void ISO_multdiv(first)
localmem*first;
{if(left.class!=immediate)ForceStack(&left);
}
hidden void BIN_multdiv(first)
localmem*first;
{
if(left.class==immediate)
   if(right.class==immediate)
      switch(left.op){
      case'*':this.eval*=left.eval;return;
      case'/':this.eval=left.eval/right.eval;return;
      case'%':this.eval=left.eval%right.eval;return;
      case 0 :this.eval=left.eval<<right.eval;return;
      case 1 :this.eval=left.eval>>right.eval;return;
      }
   else if(left.op=='*'){
      ForceA(&right);
      GenMDS('*',&left);}
   else{
      ForceA(&right);
      Emit("LOAD X,#%d%nPUSH X",left.eval);
      GenMDS(left.op,NULL);
      }
else
   if(right.class==immediate){
      LazyPOP(0);
      GenMDS(left.op,&right);
      }
   else{
      ForceA(&right);
      GenMDS(left.op,NULL);
      }
this.class=generated;
this.place=this.isCExpr=0;
}

/******************************/
hidden void ISO_andOr(first)
localmem*first;
{
if(left.class!=immediate)
   ForceJump(&left,left.op);
else{
   if(left.eval!=0)left.eval=1;
   if(left.eval==left.op)
      LazyJMP(&left);
   }
}
hidden void BIN_andOr(first)
localmem*first;
{
if(left.class==immediate)
   if(left.eval==left.op){ /* 0&& oppure 1|| */
      LazyLabel(&left);SuppressLabels(&right);
      this.eval=left.eval;this.class=immediate;
      }
   else{
      left.eval=0;left.op=NE;
      BIN_compare(&left);
      }
else{
   ForceFlags(&right);
   if(right.label[left.op]==0)
      right.label[left.op]=left.label[left.op];
   else
      EQUforward(left.label[left.op],right.label[left.op]);
   }
}
/*******************/
hidden void ISO_ternary(first)
localmem*first;
{
if(left.class!=immediate)
   ForceJump(&left,0);
else if(left.eval==0)
   LazyJMP(&left);
else
   left.eval=1;
}
hidden void BIN_1ternary(first)
localmem*first;
{
if(left.class!=immediate){
   ForceA(&right);
   BEmit(T,left.auxlabel=newlabel());
   LEmit(left.Flabel);
   left.auxternary=0xFF;
   }
else{
   byte b=left.eval;
   right.isCExpr&=left.isCExpr;
   movmem(&right.gci,&left.gci,sizeof(struct GCI));
   if((left.auxternary=b)==0){
      LazyLabel(&left);SuppressLabels(&right);
      }
   else
      LazyJMP(&left);
   }
}
hidden void BIN_2ternary(first)
localmem*first;
{
if(left.auxternary==0xFF){
   ForceA(&right);
   LEmit(left.auxlabel);
   }
else if(left.auxternary!=0){
   LazyLabel(&left);SuppressLabels(&right);
   left.isCExpr&=right.isCExpr;
   movmem(&left.gci,&right.gci,sizeof(struct GCI));
   }
}
/*******************/
#define ISO_comma SuppressLabels
#define BIN_comma null

/*******************/
void null(){}
static void (*ISOjmpTab[])()={
ISO_comma,ISO_generic,ISO_andOr,
ISO_generic,ISO_multdiv,
null,ISO_ternary,ISO_funpar,
ISO_funCall,ISO_offset,ISO_assign,
ISO_funpar_inv,ISO_funCall_inv};
static void (*BINjmpTab[])()={
BIN_comma,BIN_compare,BIN_andOr,
BIN_plusminus,BIN_multdiv,
BIN_1ternary,BIN_2ternary,BIN_funpar,
BIN_funCall,BIN_offset,BIN_assign,
BIN_funpar_inv,BIN_funCall_inv};

/**********************/
void InitSecOp(first,i)
localmem *first;
{
movmem(&(currentExpr.gci),&(first->gci),sizeof(struct GCI));
if(!(first->opfea&32))
   if(first->opfea&lval1op)
      {if(!first->lval)errore("lvalue expected");}
   else
      ForceRvalue(first);
if(first->opfea&assignOp){
   ForceNative(first);
   ISO_assign(first);
   first->counter=first->class; /* dirty !! */
   }
(*ISOjmpTab[i])(first);
}
/********************/
void BinaryOp(first,i)
localmem*first;
{
if(!(first->opfea&32))ForceRvalue(&this);
(*BINjmpTab[i])(first);
if(first->opfea&assignOp){
   first->class=first->counter;
   BIN_assign(first);
   }
this.isCExpr&=left.isCExpr;
}
