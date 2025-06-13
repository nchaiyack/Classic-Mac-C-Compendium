/*
 * lcode.c -- linker routines to parse .u1 files and produce icode.
 */

#include "link.h"
#include "tproto.h"
#include "tglobals.h"
#include "opcode.h"
#include "keyword.h"
#include "::h:version.h"
#include "::h:header.h"

/*
 *  This needs fixing ...
 */
#undef CsetPtr
#define CsetPtr(b,c)	((c) + (((b)&0377) >> LogIntBits)) 

/*
 * Prototypes.
 */

hidden novalue	backpatch	Params((int lab));
hidden novalue	clearlab	Params((noargs));
hidden novalue	flushcode	Params((noargs));
hidden novalue	intout		Params((int oint));
hidden novalue	lemit		Params((int op,char *name));
hidden novalue	lemitcon	Params((int k));
hidden novalue	lemiteven	Params((noargs));
hidden novalue	lemitin		Params((int op,word offset,int n,char *name));
hidden novalue	lemitint	Params((int op,long i,char *name));
hidden novalue	lemitl		Params((int op,int lab,char *name));
hidden novalue	lemitn		Params((int op,word n,char *name));
hidden novalue	lemitproc
   Params((word name,int nargs,int ndyn,int nstat, int fstat));
hidden novalue	lemitr		Params((int op,word loc,char *name));
hidden novalue	outblock	Params((char *addr,int count));
hidden novalue	setfile		Params((noargs));
hidden novalue	wordout		Params((word oword));

#ifdef DeBugLinker
hidden novalue	dumpblock	Params((char *addr,int count));
#endif					/* DeBugLinker */

#if AMIGA
#include <fcntl.h>
#endif					/* AMIGA */

#if MSDOS
extern long fileOffsetOfStuffThatGoesInICX;
  /* defined in Globals.h, set in link.c, used below */
#endif                                  /* MSDOS */

#if MVS
extern char *routname;
#endif					/* MVS */

word pc = 0;		/* simulated program counter */

#define outword(n)	wordout((word)(n))
#define outop(n)	intout((int)(n))
#define CodeCheck(n) if ((long)codep + (n) > (long)((long)codeb + maxcode))\
                     codeb = (char *) trealloc(codeb, &codep, &maxcode, 1,\
                       (n), "code buffer");

/*
 * gencode - read .u1 file, resolve variable references, and generate icode.
 *  Basic process is to read each line in the file and take some action
 *  as dictated by the opcode.	This action sometimes involves parsing
 *  of arguments and usually culminates in the call of the appropriate
 *  lemit* routine.
 */
novalue gencode()
   {
   register int op, k, lab;
   int j, nargs, flags, implicit;
   char *name;
   word id, procname;
   struct centry *cp;
   struct gentry *gp;
   struct fentry *fp;
   union xval gg;

   while ((op = getopc(&name)) != EOF) {
      switch (op) {

         /* Ternary operators. */

         case Op_Toby:
         case Op_Sect:

         /* Binary operators. */

         case Op_Asgn:
         case Op_Cat:
         case Op_Diff:
         case Op_Div:
         case Op_Eqv:
         case Op_Inter:
         case Op_Lconcat:
         case Op_Lexeq:
         case Op_Lexge:
         case Op_Lexgt:
         case Op_Lexle:
         case Op_Lexlt:
         case Op_Lexne:
         case Op_Minus:
         case Op_Mod:
         case Op_Mult:
         case Op_Neqv:
         case Op_Numeq:
         case Op_Numge:
         case Op_Numgt:
         case Op_Numle:
         case Op_Numlt:
         case Op_Numne:
         case Op_Plus:
         case Op_Power:
         case Op_Rasgn:
         case Op_Rswap:
         case Op_Subsc:
         case Op_Swap:
         case Op_Unions:

         /* Unary operators. */

         case Op_Bang:
         case Op_Compl:
         case Op_Neg:
         case Op_Nonnull:
         case Op_Null:
         case Op_Number:
         case Op_Random:
         case Op_Refresh:
         case Op_Size:
         case Op_Tabmat:
         case Op_Value:

         /* Instructions. */

         case Op_Bscan:
         case Op_Ccase:
         case Op_Coact:
         case Op_Cofail:
         case Op_Coret:
         case Op_Dup:
         case Op_Efail:
         case Op_Eret:
         case Op_Escan:
         case Op_Esusp:
         case Op_Limit:
         case Op_Lsusp:
         case Op_Pfail:
         case Op_Pnull:
         case Op_Pop:
         case Op_Pret:
         case Op_Psusp:
         case Op_Push1:
         case Op_Pushn1:
         case Op_Sdup:
            newline();
            lemit(op, name);
            break;

         case Op_Chfail:
         case Op_Create:
         case Op_Goto:
         case Op_Init:
            lab = getlab();
            newline();
            lemitl(op, lab, name);
            break;

         case Op_Cset:
         case Op_Real:
            k = getdec();
            newline();
            lemitr(op, lctable[k].c_pc, name);
            break;

         case Op_Field:
            id = getid();
            newline();
            fp = flocate(id);
            if (fp != NULL)
               lemitn(op, (word)(fp->f_fid-1), name);
	    else {
               lwarn(&lsspace[id],
                  "no record having this field is ever created", "");
               lemitn(op, (word)-1, name);
               }
            break;


         case Op_Int: {
            long i;
            k = getdec();
            newline();
            cp = &lctable[k];
            /*
             * Check to see if a large integers has been converted to a string.
             *  If so, generate the code for +s.
             */
            if (cp->c_flag & F_StrLit) {
               lemit(Op_Pnull,"pnull");
               lemitin(Op_Str, cp->c_val.sval, cp->c_length, "str");
               lemit(Op_Number,"number");
               break;
               }
            i = (long)cp->c_val.ival;
            lemitint(op, i, name);
            break;
            }


         case Op_Invoke:
            k = getdec();
            newline();
            if (k == -1)
               lemit(Op_Apply,"apply");
            else
               lemitn(op, (word)k, name);
            break;

         case Op_Keywd:
            id = getstr();
            newline();
            k = klookup(&lsspace[id]);
            switch (k) {
               case 0:
                  lfatal(&lsspace[id],"invalid keyword");	
                  break;
               case K_FAIL:
                  lemit(Op_Efail,"efail");
                  break;
               case K_NULL:
                  lemit(Op_Pnull,"pnull");
                  break;
               default:
               lemitn(op, (word)k, name);
            }
            break;

         case Op_Llist:
            k = getdec();
            newline();
            lemitn(op, (word)k, name);
            break;

         case Op_Lab:
            lab = getlab();
            newline();

#ifdef DeBugLinker
            if (Dflag)
               fprintf(dbgfile, "L%d:\n", lab);
#endif					/* DeBugLinker */
            backpatch(lab);
            break;

         case Op_Line:
            /*
             * Line number change.
             *  All the interesting stuff happens in Op_Colm now.
             */
            lineno = getdec();

#ifndef SrcColumnInfo
            /*
             * Enter the value in the line number table
             *  that is stored in the icode file and used during error
             *  handling and execution monitoring.  One can generate a VM
             *  instruction for these changes, but since the numbers are not
             *  saved and restored during backtracking, it is more accurate
             *  to check for line number changes in-line in the interpreter.
             *  Fortunately, the in-line check is about as fast as executing
             *  Op_Line instructions.  All of this is complicated by the use
             *  of Op_Line to generate Noop instructions when enabled by the
             *  LineCodes #define.
             *
             * If SrcColumnInfo is required, this code is duplicated,
             *  with changes, in the Op_Colm case below.
             */
            if (lnfree >= &lntable[nsize])
               lntable  = (struct ipc_line *)trealloc(lntable, &lnfree, &nsize,
                  sizeof(struct ipc_line), 1, "line number table");
            lnfree->ipc = pc;
            lnfree->line = lineno;
            lnfree++;
#endif					/* SrcColumnInfo */

            /*
             * Could generate an Op_Line for monitoring, but don't anymore:
             *
             * lemitn(op, (word)lineno, name);
             */
            
            newline();


#ifdef LineCodes
#ifndef EventMon
            lemit(Op_Noop,"noop");
#endif					/* EventMon */
#endif					/* LineCodes */

            break;

         case Op_Colm:			/* always recognize, maybe ignore */

            colmno = getdec();
#ifdef SrcColumnInfo
            if (lnfree >= &lntable[nsize])
               lntable  = (struct ipc_line *)trealloc(lntable, &lnfree, &nsize,
                  sizeof(struct ipc_line), 1, "line number table");
            lnfree->ipc = pc;
            lnfree->line = lineno + (colmno << 16);
            lnfree++;
#endif					/* SrcColumnInfo */
            break;

         case Op_Mark:
            lab = getlab();
            newline();
            lemitl(op, lab, name);
            break;

         case Op_Mark0:
            lemit(op, name);
            break;

         case Op_Str:
            k = getdec();
            newline();
            cp = &lctable[k];
            lemitin(op, cp->c_val.sval, cp->c_length, name);
            break;
        
         case Op_Tally:
            k = getdec();
            newline();
            lemitn(op, (word)k, name);
            break;

         case Op_Unmark:
            lemit(Op_Unmark, name);
            break;

         case Op_Var:
            k = getdec();
            newline();
            flags = lltable[k].l_flag;
            if (flags & F_Global)
               lemitn(Op_Global, (word)(lltable[k].l_val.global->g_index),
                  "global");
            else if (flags & F_Static)
               lemitn(Op_Static, (word)(lltable[k].l_val.staticid-1), "static");
            else if (flags & F_Argument)
               lemitn(Op_Arg, (word)(lltable[k].l_val.offset-1), "arg");
            else
               lemitn(Op_Local, (word)(lltable[k].l_val.offset-1), "local");
            break;

         /* Declarations. */

         case Op_Proc:
            getstr();
            newline();
            procname = putident(strlen(&lsspace[lsfree]) + 1, 0);
            if (procname >= 0 && (gp = glocate(procname)) != NULL) {
               /*
                * Initialize for wanted procedure.
                */
               locinit();
               clearlab();
               lineno = 0;
               implicit = gp->g_flag & F_ImpError;
               nargs = gp->g_nargs;
               lemiteven();
               }
            else {
               /*
                * Skip unreferenced procedure.
                */
               while ((op = getopc(&name)) != EOF && op != Op_End) 
                  if (op == Op_Filen)
                     setfile();		/* handle filename op while skipping */
                  else
                     newline();		/* ignore everything else */
               }
            break;

         case Op_Local:
            k = getdec();
            flags = getoct();
            id = getid();
            putlocal(k, id, flags, implicit, procname);
            break;

         case Op_Con:
            k = getdec();
            flags = getoct();
            if (flags & F_IntLit) {
               {
               long m;
               word s_indx;

               j = getdec();		/* number of characters in integer */
               m = getint(j,&s_indx);	/* convert if possible */
               if (m < 0) { 		/* negative indicates integer too big */
                  gg.sval = s_indx;	/* convert to a string */
                  putconst(k, F_StrLit, j, pc, &gg);
                  }
               else {			/* integers is small enough */
                  gg.ival = m;
                  putconst(k, flags, 0, pc, &gg);
                  }
               }
               }
            else if (flags & F_RealLit) {
               gg.rval = getreal();
               putconst(k, flags, 0, pc, &gg);
               }
            else if (flags & F_StrLit) {
               j = getdec();
               gg.sval = getstrlit(j);
               putconst(k, flags, j, pc, &gg);
               }
            else if (flags & F_CsetLit) {
               j = getdec();
               gg.sval = getstrlit(j);
               putconst(k, flags, j, pc, &gg);
               }
            else
               fprintf(stderr, "gencode: illegal constant\n");
            newline();
            lemitcon(k);
            break;

         case Op_Filen:
            setfile();
            break;

         case Op_Declend:
            newline();
            gp->g_pc = pc;
            lemitproc(procname, nargs, dynoff, lstatics-static1, static1);
            break;

         case Op_End:
            newline();
            flushcode();
            break;

         default:
            fprintf(stderr, "gencode: illegal opcode(%d): %s\n", op, name);
            newline();
         }
      }
   }

/*
 * setfile - handle Op_Filen.
 */
hidden novalue setfile()
   {
   if (fnmfree >= &fnmtbl[fnmsize])
      fnmtbl = (struct ipc_fname *) trealloc(fnmtbl, &fnmfree,
         &fnmsize, sizeof(struct ipc_fname), 1, "file name table");

#ifdef CRAY
   fnmfree->ipc = pc/8;
#else					/* CRAY */
   fnmfree->ipc = pc;
#endif					/* CRAY */

   fnmfree->fname = getrest();
   fnmfree++;
   newline();
   }

/*
 *  lemit - emit opcode.
 *  lemitl - emit opcode with reference to program label.
 *	for a description of the chaining and backpatching for labels.
 *  lemitn - emit opcode with integer argument.
 *  lemitr - emit opcode with pc-relative reference.
 *  lemitin - emit opcode with reference to identifier table & integer argument.
 *  lemitint - emit word opcode with integer argument.
 *  lemiteven - emit null bytes to bring pc to word boundary.
 *  lemitcon - emit constant table entry.
 *  lemitproc - emit procedure block.
 *
 * The lemit* routines call out* routines to effect the "outputting" of icode.
 *  Note that the majority of the code for the lemit* routines is for debugging
 *  purposes.
 */
static novalue lemit(op, name)
int op;
char *name;
   {

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t\t\t\t# %s\n", (long)pc, op, name);
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name)	*/
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   outop(op);
   }

static novalue lemitl(op, lab, name)
int op, lab;
char *name;
   {

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\tL%d\t\t\t# %s\n", (long)pc, op, lab, name);
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name)	*/
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   if (lab >= maxlabels)
      labels  = (word *) trealloc(labels, NULL, &maxlabels, sizeof(word), 
         lab - maxlabels + 1, "labels");
   outop(op);
   if (labels[lab] <= 0) {		/* forward reference */
      outword(labels[lab]);
      labels[lab] = WordSize - pc;	/* add to front of reference chain */
      }
   else					/* output relative offset */

#ifdef CRAY
      outword((labels[lab] - (pc + WordSize))/8);
#else					/* CRAY */
      outword(labels[lab] - (pc + WordSize));
#endif					/* CRAY */
   }

static novalue lemitn(op, n, name)
int op;
word n;
char *name;
   {

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t%ld\t\t\t# %s\n", (long)pc, op, (long)n,
         name);
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name) */
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   outop(op);
   outword(n);
   }


static novalue lemitr(op, loc, name)
int op;
word loc;
char *name;
   {

#ifdef CRAY
   loc = (loc - pc - 16)/8;
#else					/* CRAY */
   loc -= pc + ((IntBits/ByteBits) + WordSize);
#endif					/* CRAY */

#ifdef DeBugLinker
   if (Dflag) {
      if (loc >= 0)
         fprintf(dbgfile, "%ld:\t%d\t*+%ld\t\t\t# %s\n",(long) pc, op,
            (long)loc, name);
      else
         fprintf(dbgfile, "%ld:\t%d\t*-%ld\t\t\t# %s\n",(long) pc, op,
            (long)-loc, name);
      }
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name) */
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   outop(op);
   outword(loc);
   }

static novalue lemitin(op, offset, n, name)
int op, n;
word offset;
char *name;
   {

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t%d,S+%ld\t\t\t# %s\n", (long)pc, op, n,
         (long)offset, name);
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name) */
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   outop(op);
   outword(n);
   outword(offset);
   }

/*
 * lemitint can have some pitfalls.  outword is used to output the
 *  integer and this is picked up in the interpreter as the second
 *  word of a short integer.  The integer value output must be
 *  the same size as what the interpreter expects.  See op_int and op_intx
 *  in interp.s
 */
static novalue lemitint(op, i, name)
int op;
long i;
char *name;
   {

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile,"%ld:\t%d\t%ld\t\t\t# %s\n",(long)pc,op,(long)i,name);
#else					/* DeBugLinker */
#if MACINTOSH && MPW
/* #pragma unused(name) */
#endif					/* MACINTOSH && MPW */
#endif					/* DeBugLinker */

   outop(op);
   outword(i);
   }

static novalue lemiteven()
   {
   word x = 0;
   register int len;

   if (len = pc % (IntBits/ByteBits))
      outblock((char *)x, (IntBits/ByteBits) - len);
   }

static novalue lemitcon(k)
register int k;
   {
   register int i, j;
   register char *s;
   int csbuf[CsetSize];
   union {
      char ovly[1];  /* Array used to overlay l and f on a bytewise basis. */
      long l;
      double f;
      } x;

   if (lctable[k].c_flag & F_RealLit) {

#ifdef Double
/* access real values one word at a time */
      {  int *rp, *rq;
         rp = (int *) &(x.f);
         rq = (int *) &(lctable[k].c_val.rval);
         *rp++ = *rq++;
         *rp	= *rq;
      }
#else					/* Double */
      x.f = lctable[k].c_val.rval;
#endif					/* Double */

#ifdef DeBugLinker
      if (Dflag) {
         fprintf(dbgfile, "%ld:\t%d\n", (long)pc, T_Real);
         dumpblock(x.ovly,sizeof(double));
         fprintf(dbgfile, "\t\t\t( %g )\n",x.f);
         }
#endif					/* DeBugLinker */

      outword(T_Real);

#ifdef Double
#if WordBits != 64
/* fill out real block with an empty word */
      outword(0);
#ifdef DeBugLinker
      if (Dflag)
	 fprintf(dbgfile,"\t0\t\t\t\t\t# padding\n")
#endif					/* DeBugLinker */
#endif					/* WordBits != 64 */
#endif					/* Double */

      outblock(x.ovly,sizeof(double));
      }
   else if (lctable[k].c_flag & F_CsetLit) {
      for (i = 0; i < CsetSize; i++)
         csbuf[i] = 0;
      s = &lsspace[lctable[k].c_val.sval];
      i = lctable[k].c_length;
      while (i--) {
         Setb(ToAscii(*s), csbuf);
         s++;
         }
      j = 0;
      for (i = 0; i < 256; i++) {
         if (Testb(i, csbuf))
           j++;
         }

#ifdef DeBugLinker
      if (Dflag) {
         fprintf(dbgfile, "%ld:\t%d\n",(long) pc, T_Cset);
         fprintf(dbgfile, "\t%d\n",j);
         }
#endif					/* DeBugLinker */

      outword(T_Cset);
      outword(j);		   /* cset size */
      outblock((char *)csbuf,sizeof(csbuf));

#ifdef DeBugLinker
      if (Dflag)
         dumpblock((char *)csbuf,CsetSize);
#endif					/* DeBugLinker */

      }
   }

static novalue lemitproc(name, nargs, ndyn, nstat, fstat)
word name;
int nargs, ndyn, nstat, fstat;
   {
   register int i;
   register char *p;
   word s_indx;
   int size;
   /*
    * FncBlockSize = sizeof(BasicFncBlock) +
    *  sizeof(descrip)*(# of args + # of dynamics + # of statics).
    */
#ifdef MultiThread
   size = (10*WordSize) + (2*WordSize) * (abs(nargs)+ndyn+nstat);
#else					/* MultiThread */
   size = (9*WordSize) + (2*WordSize) * (abs(nargs)+ndyn+nstat);
#endif					/* MultiThread */

   p = &lsspace[name];
#ifdef DeBugLinker
   if (Dflag) {
      fprintf(dbgfile, "%ld:\t%d\n", (long)pc, T_Proc); /* type code */
      fprintf(dbgfile, "\t%d\n", size);			/* size of block */
      fprintf(dbgfile, "\tZ+%ld\n",(long)(pc+size));	/* entry point */
      fprintf(dbgfile, "\t%d\n", nargs);		/* # arguments */
      fprintf(dbgfile, "\t%d\n", ndyn);			/* # dynamic locals */
      fprintf(dbgfile, "\t%d\n", nstat);		/* # static locals */
      fprintf(dbgfile, "\t%d\n", fstat);		/* first static */
      fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n",	/* name of procedure */
         (int)strlen(p), (long)(name), p);
      }
#endif					/* DeBugLinker */

   outword(T_Proc);
   outword(size);
   outword(pc + size - 2*WordSize); /* Have to allow for the two words
                			that we've already output. */
   outword(nargs);
   outword(ndyn);
   outword(nstat);
   outword(fstat);
#ifdef MultiThread
   outword(0);			/* program (filled in by interp) */
#endif				/* MultiThread */
   outword(strlen(p));          /* procedure name: length & offset */
   outword(name);

   /*
    * Output string descriptors for argument names by looping through
    *  all locals, and picking out those with F_Argument set.
    */
   for (i = 0; i <= nlocal; i++) {
      if (lltable[i].l_flag & F_Argument) {
         s_indx = lltable[i].l_name;
         p = &lsspace[s_indx];

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n", (int)strlen(p),
               (long)s_indx, p);
#endif					/* DeBugLinker */

         outword(strlen(p));
         outword(s_indx);
         }
      }

   /*
    * Output string descriptors for local variable names.
    */
   for (i = 0; i <= nlocal; i++) {
      if (lltable[i].l_flag & F_Dynamic) {
         s_indx = lltable[i].l_name;
         p = &lsspace[s_indx];

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n", (int)strlen(p),
               (long)s_indx, p);
#endif					/* DeBugLinker */

         outword(strlen(p));
         outword(s_indx);
         }
      }

   /*
    * Output string descriptors for static variable names.
    */
   for (i = 0; i <= nlocal; i++) {
      if (lltable[i].l_flag & F_Static) {
         s_indx = lltable[i].l_name;
         p = &lsspace[s_indx];

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n", (int)strlen(p),
               (long)s_indx, p);
#endif					/* DeBugLinker */

         outword(strlen(p));
         outword(s_indx);
         }
      }
   }

/*
 * gentables - generate interpreter code for global, static,
 *  identifier, and record tables, and built-in procedure blocks.
 */

novalue gentables()
   {
   register int i;
   register char *s;
   register struct gentry *gp;
   struct fentry *fp;
   struct rentry *rp;
   struct header hdr;

#if MVS
   FILE *toutfile;		/* temporary file for icode output */
#endif					/* MVS */

   lemiteven();

   /*
    * Output record constructor procedure blocks.
    */
   hdr.Records = pc;

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile,"\n%ld:\t%d\t\t\t\t# record blocks\n",(long)pc,nrecords);
#endif					/* DeBugLinker */

   outword(nrecords);
   for (gp = lgfirst; gp != NULL; gp = gp->g_next) {
      if ((gp->g_flag & F_Record) && gp->g_procid > 0) {
         s = &lsspace[gp->g_name];
         gp->g_pc = pc;

#ifdef DeBugLinker
         if (Dflag) {
            fprintf(dbgfile, "%ld:\n", pc);
            fprintf(dbgfile, "\t%d\n", T_Proc);
            fprintf(dbgfile, "\t%d\n", RkBlkSize(gp));
            fprintf(dbgfile, "\t_mkrec\n");
            fprintf(dbgfile, "\t%d\n", gp->g_nargs);
            fprintf(dbgfile, "\t-2\n");
            fprintf(dbgfile, "\t%d\n", gp->g_procid);
            fprintf(dbgfile, "\t1\n");
            fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n", (int)strlen(s),
               (long)gp->g_name, s);
            }

#endif					/* DeBugLinker */

         outword(T_Proc);		/* type code */
         outword(RkBlkSize(gp));
         outword(0);			/* entry point (filled in by interp)*/
         outword(gp->g_nargs);		/* number of fields */
         outword(-2);			/* record constructor indicator */
         outword(gp->g_procid);		/* record id */
         outword(1);			/* serial number */
#ifdef MultiThread
         outword(0);			/* program (filled in by interp) */
#endif					/* MultiThread */
         outword(strlen(s));		/* name of record: size and offset */
         outword(gp->g_name);

         for (i=0;i<gp->g_nargs;i++) {	/* field names (filled in by interp) */
            int foundit = 0;
            /*
             * Find the field list entry corresponding to field i in
             * record gp, then write out a descriptor for it.
             */
            for (fp = lffirst; fp != NULL; fp = fp->f_nextentry) {
               for (rp = fp->f_rlist; rp!= NULL; rp=rp->r_link) {
                  if (rp->r_gp == gp && rp->r_fnum == i) {
                     if (foundit) {
                        /*
                         * This internal error should never occur
                         */
                        fprintf(stderr,"found rec %d field %d already!!\n",
                        	gp->g_procid, i);
                        fflush(stderr);
                        exit(1);
                        }
#ifdef DeBugLinker
                     if (Dflag)
                        fprintf(dbgfile, "\t%d\tS+%ld\t\t\t# %s\n",
                           (int)strlen(&lsspace[fp->f_name]),
                           fp->f_name, &lsspace[fp->f_name]);
#endif					/* DeBugLinker */
                     outword(strlen(&lsspace[fp->f_name]));
                     outword(fp->f_name);
                     foundit++;
                     }
                  }
               }
            if (!foundit) {
               /*
                * This internal error should never occur
                */
               fprintf(stderr,"never found rec %d field %d!!\n",
                       gp->g_procid,i);
               fflush(stderr);
               exit(1);
               }
            }
         }
      }

   /*
    * Output record/field table.
    */
   hdr.Ftab = pc;

#ifdef DeBugLinker
   if (Dflag)
      fprintf(dbgfile, "\n%ld:\t\t\t\t\t# record/field table\n", (long)pc);
#endif					/* DeBugLinker */

   for (fp = lffirst; fp != NULL; fp = fp->f_nextentry) {

#ifdef DeBugLinker
      if (Dflag)
         fprintf(dbgfile, "%ld:\t\t\t\t\t# %s\n", (long)pc,
            &lsspace[fp->f_name]);
#endif					/* DeBugLinker */

      rp = fp->f_rlist;
      for (i = 1; i <= nrecords; i++) {
         while (rp != NULL && rp->r_gp->g_procid < 0)
            rp = rp->r_link;		/* skip unreferenced constructor */
         if (rp != NULL && rp->r_gp->g_procid == i) {

#ifdef DeBugLinker
            if (Dflag)
                fprintf(dbgfile, "\t%d\n", rp->r_fnum);
#endif					/* DeBugLinker */

            outop(rp->r_fnum);

            rp = rp->r_link;
            }
         else {

#ifdef DeBugLinker
            if (Dflag)
                fprintf(dbgfile, "\t-1\n");
#endif					/* DeBugLinker */

            outop(-1);
            }

#ifdef DeBugLinker
         if (Dflag && (i == nrecords || (i & 03) == 0))
            putc('\n', dbgfile);
#endif					/* DeBugLinker */

         }
      }

   /*
    * Output descriptors for field names.
    */

   hdr.Fnames = pc;
   for (fp = lffirst; fp != NULL; fp = fp->f_nextentry) {
      s = &lsspace[fp->f_name];

#ifdef DeBugLinker
      if (Dflag)
         fprintf(dbgfile, "%ld:\t%d\tS+%ld\t\t\t# %s\n",
            (long)pc, (int)strlen(s), (long)fp->f_name, s);
#endif					/* DeBugLinker */

      outword(strlen(s));      /* name of field: length & offset */
      outword(fp->f_name);
   }


   /*
    * Output global variable descriptors.
    */
   hdr.Globals = pc;
   for (gp = lgfirst; gp != NULL; gp = gp->g_next) {
      if (gp->g_flag & F_Builtin) {		/* function */

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "%ld:\t%06lo\t%d\t\t\t# %s\n",
                (long)pc, (long)D_Proc, -gp->g_procid, &lsspace[gp->g_name]);
#endif					/* DeBugLinker */

         outword(D_Proc);
         outword(-gp->g_procid);
         }
      else if (gp->g_flag & F_Proc) {		/* Icon procedure */

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "%ld:\t%06lo\tZ+%ld\t\t\t# %s\n",
                (long)pc,(long)D_Proc, (long)gp->g_pc, &lsspace[gp->g_name]);
#endif					/* DeBugLinker */

         outword(D_Proc);
         outword(gp->g_pc);
         }
      else if (gp->g_flag & F_Record) {		/* record constructor */

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "%ld:\t%06lo\tZ+%ld\t\t\t# %s\n",
                (long)pc, (long)D_Proc, (long)gp->g_pc, &lsspace[gp->g_name]);
#endif					/* DeBugLinker */

         outword(D_Proc);
         outword(gp->g_pc);
         }
      else {					/* simple global variable */

#ifdef DeBugLinker
         if (Dflag)
            fprintf(dbgfile, "%ld:\t%06lo\t0\t\t\t# %s\n",(long)pc,
               (long)D_Null, &lsspace[gp->g_name]);
#endif					/* DeBugLinker */

         outword(D_Null);
         outword(0);
         }
      }

   /*
    * Output descriptors for global variable names.
    */
   hdr.Gnames = pc;
   for (gp = lgfirst; gp != NULL; gp = gp->g_next) {

#ifdef DeBugLinker
      if (Dflag)
         fprintf(dbgfile, "%ld:\t%d\tS+%ld\t\t\t# %s\n",
            (long)pc, (int)strlen(&lsspace[gp->g_name]), (long)(gp->g_name),
               &lsspace[gp->g_name]);
#endif					/* DeBugLinker */

      outword(strlen(&lsspace[gp->g_name]));
      outword(gp->g_name);
      }

   /*
    * Output a null descriptor for each static variable.
    */
   hdr.Statics = pc;
   for (i = lstatics; i > 0; i--) {

#ifdef DeBugLinker
      if (Dflag)
         fprintf(dbgfile, "%ld:\t0\t0\n", (long)pc);
#endif					/* DeBugLinker */

      outword(D_Null);
      outword(0);
      }
   flushcode();

   /*
    * Output the string constant table and the two tables associating icode
    *  locations with source program locations.  Note that the calls to write
    *  really do all the work.
    */

   hdr.Filenms = pc;
   if (longwrite((char *)fnmtbl, (long)((char *)fnmfree - (char *)fnmtbl),
      outfile) < 0)
         quit("cannot write icode file");

#ifdef DeBugLinker
   if (Dflag) {
      int k = 0;
      struct ipc_fname *ptr;
      for (ptr = fnmtbl; ptr < fnmfree; ptr++) {
         fprintf(dbgfile, "%ld:\t%03d\tS+%03d\t\t\t# %s\n",
            (long)(pc + k), ptr->ipc, ptr->fname, &lsspace[ptr->fname]);
         k = k + 8;
         }
      putc('\n', dbgfile);
      }

#endif					/* DeBugLinker */

   pc += (char *)fnmfree - (char *)fnmtbl;

   hdr.linenums = pc;
   if (longwrite((char *)lntable, (long)((char *)lnfree - (char *)lntable),
      outfile) < 0)
         quit("cannot write icode file");

#ifdef DeBugLinker
   if (Dflag) {
      int k = 0;
      struct ipc_line *ptr;
      for (ptr = lntable; ptr < lnfree; ptr++) {
         fprintf(dbgfile, "%ld:\t%03d\t%03d\n", (long)(pc + k),
            ptr->ipc, ptr->line);
         k = k + 8;
         }
      putc('\n', dbgfile);
      }

#endif					/* DeBugLinker */

   pc += (char *)lnfree - (char *)lntable;

   hdr.Strcons = pc;
#ifdef DeBugLinker
   if (Dflag) {
      int c, j, k;
      j = k = 0;
      for (s = lsspace; s < &lsspace[lsfree]; ) {
         fprintf(dbgfile, "%ld:\t%03o", (long)(pc + k), *s++ & 0377);
         k = k + 8;
         for (i = 7; i > 0; i--) {
            if (s >= &lsspace[lsfree])
               fprintf(dbgfile,"    ");
            else
               fprintf(dbgfile, " %03o", *s++ & 0377);
            }
         fprintf(dbgfile, "   ");
         for (i = 0; i < 8; i++)
            if (j < lsfree)
               putc(isprint(c = lsspace[j++] & 0377) ? c : ' ', dbgfile);
         putc('\n', dbgfile);
         }
      }

#endif					/* DeBugLinker */


   if (longwrite(lsspace, (long)lsfree, outfile) < 0)
         quit("cannot write icode file");

   pc += lsfree;

   /*
    * Output icode file header.
    */
   hdr.hsize = pc;
   strcpy((char *)hdr.config,IVersion);
   hdr.trace = trace;


#ifdef DeBugLinker
   if (Dflag) {
      fprintf(dbgfile, "\n");
      fprintf(dbgfile, "size:	 %ld\n", (long)hdr.hsize);
      fprintf(dbgfile, "trace:	 %ld\n", (long)hdr.trace);
      fprintf(dbgfile, "records: %ld\n", (long)hdr.Records);
      fprintf(dbgfile, "ftab:	 %ld\n", (long)hdr.Ftab);
      fprintf(dbgfile, "fnames:  %ld\n", (long)hdr.Fnames);
      fprintf(dbgfile, "globals: %ld\n", (long)hdr.Globals);
      fprintf(dbgfile, "gnames:  %ld\n", (long)hdr.Gnames);
      fprintf(dbgfile, "statics: %ld\n", (long)hdr.Statics);
      fprintf(dbgfile, "strcons:   %ld\n", (long)hdr.Strcons);
      fprintf(dbgfile, "filenms:   %ld\n", (long)hdr.Filenms);
      fprintf(dbgfile, "linenums:   %ld\n", (long)hdr.linenums);
      fprintf(dbgfile, "config:   %s\n", hdr.config);
      }
#endif					/* DeBugLinker */

#ifdef Header
   fseek(outfile, hdrsize, 0);
#else                                   /* Header */

#if MVS
/*
 * This kind of backpatching cannot work on a PDS member, and that's
 *  probably where the code is going.  So the code goes out first to
 *  a temporary file, and then copied to the real icode file after
 *  the header is written.
 */
   fseek(outfile, sizeof(hdr), SEEK_SET);
   toutfile = outfile;
   outfile = fopen(routname, WriteBinary);
   if (outfile == NULL)
      quitf("cannot create %s",routname);
#else
#if MSDOS
   fseek(outfile, fileOffsetOfStuffThatGoesInICX, 0);
#else					/* MSDOS */
   fseek(outfile, 0L, 0);
#endif					/* MSDOS */
#endif                                  /* MVS */
#endif                                  /* Header */

   if (longwrite((char *)&hdr, (long)sizeof(hdr), outfile) < 0)
      quit("cannot write icode file");

#if MVS
   {
      char *allelse = malloc(hdr.hsize);
      if (hdr.hsize != fread(allelse, 1, hdr.hsize, toutfile) ||
          longwrite(allelse, hdr.hsize, outfile) < 0)
            quit("cannot write icode file");
      free(allelse);
      fclose(toutfile);
   }
#endif					/* MVS */

   if (verbose >= 2) {
      word tsize = sizeof(hdr) + hdr.hsize;
#ifdef Header
      fprintf(stderr, "  bootstrap  %7ld\n", hdrsize);
      tsize += hdrsize;
#endif					/* Header */
      fprintf(stderr, "  header     %7ld\n", (long)sizeof(hdr));
      fprintf(stderr, "  procedures %7ld\n", (long)hdr.Records);
      fprintf(stderr, "  records    %7ld\n", (long)(hdr.Ftab - hdr.Records));
      fprintf(stderr, "  fields     %7ld\n", (long)(hdr.Globals - hdr.Ftab));
      fprintf(stderr, "  globals    %7ld\n", (long)(hdr.Statics - hdr.Globals));
      fprintf(stderr, "  statics    %7ld\n", (long)(hdr.Filenms - hdr.Statics));
      fprintf(stderr, "  linenums   %7ld\n", (long)(hdr.Strcons - hdr.Filenms));
      fprintf(stderr, "  strings    %7ld\n", (long)(hdr.hsize - hdr.Strcons));
      fprintf(stderr, "  total      %7ld\n", (long)tsize);
      }
   }

/*
 * intout(i) outputs i as an int that is used by the runtime system
 *  IntBits/ByteBits bytes must be moved from &word[0] to &codep[0].
 */
static novalue intout(oint)
int oint;
   {
   int i;
   union {
      int i;
      char c[IntBits/ByteBits]; 
      } u;
 
   CodeCheck(IntBits/ByteBits);
   u.i = oint;

   for (i = 0; i < IntBits/ByteBits; i++)
      codep[i] = u.c[i];

   codep += IntBits/ByteBits;
   pc += IntBits/ByteBits;
   }

/*
 * wordout(i) outputs i as a word that is used by the runtime system
 *  WordSize bytes must be moved from &oword[0] to &codep[0].
 */
static novalue wordout(oword)
word oword;
   {
   int i;
   union {
        word i;
        char c[WordSize];
        } u;

   CodeCheck(WordSize);
   u.i = oword;

   for (i = 0; i < WordSize; i++)
      codep[i] = u.c[i];

   codep += WordSize;
   pc += WordSize;
   }

/*
 * outblock(a,i) output i bytes starting at address a.
 */
static novalue outblock(addr,count)
char *addr;
int count;
   {
   CodeCheck(count);
   pc += count;
   while (count--)
      *codep++ = *addr++;
   }

#ifdef DeBugLinker
/*
 * dumpblock(a,i) dump contents of i bytes at address a, used only
 *  in conjunction with -L.
 */
static novalue dumpblock(addr, count)
char *addr;
int count;
   {
   int i;
   for (i = 0; i < count; i++) {
      if ((i & 7) == 0)
         fprintf(dbgfile,"\n\t");
      fprintf(dbgfile," %03o",(0377 & (unsigned)addr[i]));
      }
   putc('\n',dbgfile);
   }
#endif					/* DeBugLinker */

/*
 * flushcode - write buffered code to the output file.
 */
static novalue flushcode()
   {
   if (codep > codeb)
      if (longwrite(codeb, DiffPtrs(codep,codeb), outfile) < 0)
         quit("cannot write icode file");
   codep = codeb;
   }

/*
 * clearlab - clear label table to all zeroes.
 */
static novalue clearlab()
   {
   register int i;

   for (i = 0; i < maxlabels; i++)
      labels[i] = 0;
   }

/*
 * backpatch - fill in all forward references to lab.
 */
static novalue backpatch(lab)
int lab;
   {
   word p, r;
   char *q;
   char *cp, *cr;
   register int j;

   if (lab >= maxlabels)
      labels  = (word *) trealloc(labels, NULL, &maxlabels, sizeof(word),
         lab - maxlabels + 1, "labels");

   p = labels[lab];
   if (p > 0)
      quit("multiply defined label in ucode");
   while (p < 0) {		/* follow reference chain */

#ifdef CRAY
      r = (pc - (WordSize - p))/8;	/* compute relative offset */
#else					/* CRAY */
      r = pc - (WordSize - p);	/* compute relative offset */
#endif					/* CRAY */
      q = codep - (pc + p);	/* point to word with address */
      cp = (char *) &p;		/* address of integer p       */
      cr = (char *) &r;		/* address of integer r       */
      for (j = 0; j < WordSize; j++) {	  /* move bytes from word pointed to */
         *cp++ = *q;			  /* by q to p, and move bytes from */
         *q++ = *cr++;			  /* r to word pointed to by q */
         }			/* moves integers at arbitrary addresses */
      }
   labels[lab] = pc;
   }

#ifdef DeBugLinker
novalue idump(s)		/* dump code region */
   char *s;
   {
   int *c;

   fprintf(stderr,"\ndump of code region %s:\n",s);
   for (c = (int *)codeb; c < (int *)codep; c++)
       fprintf(stderr,"%ld: %d\n",(long)c, (int)*c);
   fflush(stderr);
   }
#endif					/* DeBugLinker */
