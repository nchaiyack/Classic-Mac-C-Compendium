/*
 * rdebug.r - tracebk, get_name, xdisp, ctrace, rtrace, failtrace, strace,
 *   atrace, cotrace
 */

/*
 * Prototypes.
 */
hidden int     glbcmp    Params((char *pi, char *pj));
hidden int     keyref    Params((union block *bp, dptr dp));
hidden novalue showline  Params((char *f, int l));
hidden novalue showlevel Params((register int n));
hidden novalue ttrace	Params((noargs));
hidden novalue xtrace
   Params((struct b_proc *bp, word nargs, dptr arg, int pline, char *pfile));

/*
 * tracebk - print a trace of procedure calls.
 */
#ifdef PresentationManager
/* have to add COMPILER support too */
novalue tracebk(void *foo, dptr argp, HWND hwndMLE)
#else					/* PresentationManager */
novalue tracebk(lcl_pfp, argp)

#if COMPILER
struct p_frame *lcl_pfp;
#endif					/* COMPILER */

dptr argp;
#endif					/* PresentationManager */
   {
   struct b_proc *cproc;

#if COMPILER

   struct debug *debug;
   word nparam;

   if (lcl_pfp == NULL)
      return;
   debug = PFDebug(*lcl_pfp);
   tracebk(lcl_pfp->old_pfp, lcl_pfp->old_argp);
   cproc = debug->proc;
   xtrace(cproc, (word)abs((int)cproc->nparam), argp, debug->old_line,
      debug->old_fname);

#else					/* COMPILER */

   struct pf_marker *origpfp = pfp;
   dptr arg;
   inst cipc;

   /*
    * Chain back through the procedure frame markers, looking for the
    *  first one, while building a foward chain of pointers through
    *  the expression frame pointers.
    */

   for (pfp->pf_efp = NULL; pfp->pf_pfp != NULL; pfp = pfp->pf_pfp) {
      (pfp->pf_pfp)->pf_efp = (struct ef_marker *)pfp;
      }

   /* Now start from the base procedure frame marker, producing a listing
    *  of the procedure calls up through the last one.
    */

   while (pfp) {
#ifdef PresentationManager
      /* point at the beginning of the string */
      PMStringBufPtr = PMStringBuf;
#endif					/* PresentationManager */
      arg = &((dptr)pfp)[-(pfp->pf_nargs) - 1];
      cproc = (struct b_proc *)BlkLoc(arg[0]);    
      /*
       * The ipc in the procedure frame points after the "invoke n".
       */
      cipc = pfp->pf_ipc;
      --cipc.opnd;
      --cipc.op;

      xtrace(cproc, pfp->pf_nargs, &arg[0], findline(cipc.opnd),
         findfile(cipc.opnd));
#ifdef PresentationManager
      /* insert the text in the MLE */
      WinSendMsg(hwndMLE, MLM_INSERT, MPFROMP(PMStringBuf), (MPARAM)0);
#endif					/* PresentationManager */
      /*
       * On the last call, show both the call and the offending expression.
       */
      if (pfp == origpfp) {
#ifdef PresentationManager
         /* make sure we are at the beginning of the buffer */
         PMStringBufPtr = PMStringBuf;
         ttrace();
         /* add it to the MLE */
         WinSendMsg(hwndMLE, MLM_INSERT, MPFROMP(PMStringBuf), (MPARAM)0);
#else					/* PresentationManager */
         ttrace();
#endif					/* PresentationManager */
         break;
         }
 
      pfp = (struct pf_marker *)(pfp->pf_efp);
      }
#endif					/* COMPILER */
   }

/*
 * xtrace - procedure *bp is being called with nargs arguments, the first
 *  of which is at arg; produce a trace message.
 */
static novalue xtrace(bp, nargs, arg, pline, pfile)
struct b_proc *bp;
word nargs;
dptr arg;
int pline;
char *pfile;
   {

#ifndef PresentationManager 
   fprintf(stderr, "   ");
#endif					/* PresentationManager */
   if (bp == NULL)
      fprintf(stderr, "????");
   else {

#if COMPILER
       putstr(stderr, &(bp->pname));
#else					/* COMPILER */
       if (arg[0].dword == D_Proc)
          putstr(stderr, &(bp->pname));
       else
          outimage(stderr, arg, 0);
       arg++;
#endif					/* COMPILER */

       putc('(', stderr);
       while (nargs--) {
          outimage(stderr, arg++, 0);
          if (nargs)
             putc(',', stderr);
          }
       putc(')', stderr);
       }
	 
   if (pline != 0)
      fprintf(stderr, " from line %d in %s", pline, pfile);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * get_name -- function to get print name of variable.
 */
int get_name(dp1,dp0)
   dptr dp1, dp0;
   {
   dptr dp, varptr;
   tended union block *blkptr;
   dptr arg1;                           /* 1st parameter */
   dptr loc1;                           /* 1st local */
   struct b_proc *proc;                 /* address of procedure block */
   char sbuf[100];			/* buffer; might be too small */
   char *s, *s2;
   word i, j, k;
   int t;

#if COMPILER
   arg1 = argp;
   loc1 = pfp->tend.d;
   proc = PFDebug(*pfp)->proc;
#else					/* COMPILER */
   arg1 = &argp[1];
   loc1 = pfp->pf_locals;
   proc = &BlkLoc(*argp)->proc;
#endif					/* COMPILER */

   type_case *dp1 of {
      tvsubs: {
         blkptr = BlkLoc(*dp1);
         get_name(&(blkptr->tvsubs.ssvar),dp0);
         sprintf(sbuf,"[%ld:%ld]",blkptr->tvsubs.sspos,
            blkptr->tvsubs.sspos+blkptr->tvsubs.sslen);
         k = StrLen(*dp0);
         j = strlen(sbuf);

	 /*
	  * allocate space for both the name and the subscript image,
	  *  and then copy both parts into the allocated space
	  */
	 Protect(s = alcstr(NULL, k + j), return Error);
	 s2 = StrLoc(*dp0);
	 StrLoc(*dp0) = s;
         StrLen(*dp0) = j + k;
	 for (i = 0; i < k; i++)
	    *s++ = *s2++;
	 s2 = sbuf;
	 for (i = 0; i < j; i++)
	    *s++ = *s2++;
         }

      tvtbl: {
         t = keyref(BlkLoc(*dp1) ,dp0);
         if (t == Error)
            return Error;
          }

      kywdint:
         if (VarLoc(*dp1) == &kywd_ran) {
            StrLen(*dp0) = 7;
            StrLoc(*dp0) = "&random";
            }
         else if (VarLoc(*dp1) == &kywd_trc) {
            StrLen(*dp0) = 6;
            StrLoc(*dp0) = "&trace";
            }

#ifdef FncTrace
         else if (VarLoc(*dp1) == &kywd_ftrc) {
            StrLen(*dp0) = 7;
            StrLoc(*dp0) = "&ftrace";
            }
#endif					/* FncTrace */

         else if (VarLoc(*dp1) == &kywd_dmp) {
            StrLen(*dp0) = 5;
            StrLoc(*dp0) = "&dump";
            }
         else if (VarLoc(*dp1) == &kywd_err) {
            StrLen(*dp0) = 6;
            StrLoc(*dp0) = "&error";
            }
         else
            syserr("name: unknown integer keyword variable");
            
      kywdevent:
#ifdef MultiThread
         if (VarLoc(*dp1) == &curpstate->eventsource) {
            StrLen(*dp0) = 12;
            StrLoc(*dp0) = "&eventsource";
            }
         else if (VarLoc(*dp1) == &curpstate->eventval) {
            StrLen(*dp0) = 11;
            StrLoc(*dp0) = "&eventvalue";
            }
         else if (VarLoc(*dp1) == &curpstate->eventcode) {
            StrLen(*dp0) = 10;
            StrLoc(*dp0) = "&eventcode";
            }
         else
#endif					/* MultiThread */
            syserr("name: unknown event keyword variable");
            
      kywdwin: {
         StrLen(*dp0) = 7;
         StrLoc(*dp0) = "&window";
         }

      kywdstr: {
         StrLen(*dp0) = 9;
         StrLoc(*dp0) = "&progname";
         }

      kywdpos: {
         StrLen(*dp0) = 4;
         StrLoc(*dp0) = "&pos";
         }

      kywdsubj: {
         StrLen(*dp0) = 8;
         StrLoc(*dp0) = "&subject";
         }

      default:
         if (Offset(*dp1) == 0) {
            /*
             * Must be a named variable.
             */
            dp = VarLoc(*dp1);		 /* get address of variable */
            if (InRange(globals,dp,eglobals)) {
               *dp0 = gnames[dp - globals]; 		/* global */
	       return GlobalName;
	       }
            else if (InRange(statics,dp,estatics)) {
               i = dp - statics - proc->fstatic;	/* static */
               if (i < 0 || i >= proc->nstatic)
                  syserr("name: unreferencable static variable");
               i += abs((int)proc->nparam) + abs((int)proc->ndynam);
               *dp0 = proc->lnames[i];
	       return StaticName;
               }
            else if (InRange(arg1, dp, &arg1[abs((int)proc->nparam)])) {
               *dp0 = proc->lnames[dp - arg1];          /* argument */
	       return ParamName;
	       }
            else if (InRange(loc1, dp, &loc1[proc->ndynam])) {
               *dp0 = proc->lnames[dp - loc1 + abs((int)proc->nparam)];
	       return LocalName;
               }
            else
               syserr("name: cannot determine variable name");
            }
         else {
            /*
             * Must be an element of a structure.
             */
            blkptr = (union block *)VarLoc(*dp1);
            varptr = (dptr)((word *)VarLoc(*dp1) + Offset(*dp1));
            switch ((int)BlkType(blkptr)) {
               case T_Lelem: 		/* list */
                  i = varptr - &blkptr->lelem.lslots[blkptr->lelem.first] + 1;
                  if (i < 1)
                     i += blkptr->lelem.nslots;
                  while (blkptr->lelem.listprev != NULL) {
                     blkptr = blkptr->lelem.listprev;
                     i += blkptr->lelem.nused;
                     }
                  sprintf(sbuf,"L[%ld]",i);
                  i = strlen(sbuf);
                  Protect(StrLoc(*dp0) = alcstr(sbuf,i), return Error);
                  StrLen(*dp0) = i;
                  break;
               case T_Record: 		/* record */
                  i = varptr - blkptr->record.fields;
                  proc = &blkptr->record.recdesc->proc;

                  sprintf(sbuf,"%s.%s",StrLoc(proc->recname),
                      StrLoc(proc->lnames[i]));

                  i = strlen(sbuf);
                  Protect(StrLoc(*dp0) = alcstr(sbuf,i), return Error);
                  StrLen(*dp0) = i;
                  break;
               case T_Telem: 		/* table */
                  t = keyref(blkptr,dp0);
                  if (t == Error)
                      return Error;
                  break;
               default:		/* none of the above */
#ifdef EventMon
                  *dp0 = emptystr;
#else                                   /* EventMon */
                  syserr("name: invalid structure reference");
#endif                                  /* EventMon */

               }
           }
      }
   return Succeeded;
   }

#if COMPILER
#begdef PTraceSetup()
   struct b_proc *proc;

   --k_trace;
   showline(file_name, line_num);
   showlevel(k_level);
   proc = PFDebug(*pfp)->proc; /* get address of procedure block */
   putstr(stderr, &proc->pname);
#enddef
 
/*
 * ctrace - a procedure is being called; produce a trace message.
 */
novalue ctrace()
   {
   dptr arg;
   int n;

   PTraceSetup();

   putc('(', stderr);
   arg = argp;
   n = abs((int)proc->nparam);
   while (n--) {
      outimage(stderr, arg++, 0);
      if (n)
         putc(',', stderr);
      }
   putc(')', stderr);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * rtrace - a procedure is returning; produce a trace message.
 */

novalue rtrace()
   {
   PTraceSetup();

   fprintf(stderr, " returned ");
   outimage(stderr, pfp->rslt, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * failtrace - procedure named s is failing; produce a trace message.
 */

novalue failtrace()
   {
   PTraceSetup();

   fprintf(stderr, " failed\n");
   fflush(stderr);
   }

/*
 * strace - a procedure is suspending; produce a trace message.
 */

novalue strace()
   {
   PTraceSetup();

   fprintf(stderr, " suspended ");
   outimage(stderr, pfp->rslt, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * atrace - a procedure is being resumed; produce a trace message.
 */
novalue atrace()
   {
   PTraceSetup();

   fprintf(stderr, " resumed\n");
   fflush(stderr);
   }
#endif					/* COMPILER */

/*
 * keyref(bp,dp) -- print name of subscripted table
 */
static int keyref(bp, dp)
   union block *bp;
   dptr dp;
   {
   char *s, *s2;
   int i, len;

   if (getimage(&(bp->telem.tref),dp) == Error)
      return Error;	

   /*
    * Allocate space, and copy the image surrounded by "T[" and "]"
    */
   Protect(s = alcstr(NULL, StrLen(*dp) + 3), return Error);
   s2 = StrLoc(*dp);
   len = StrLen(*dp);
   StrLoc(*dp) = s;
   StrLen(*dp) = StrLen(*dp) + 3;
   *s++ = 'T'; *s++ = '[';
   for (i = 0; i < len; i++)
      *s++ = *s2++;
   *s++ = ']';
   return Succeeded;
   }

#ifdef Coexpr
/*
 * cotrace -- a co-expression context switch; produce a trace message.
 */
novalue cotrace(ccp, ncp, swtch_typ, valloc)
struct b_coexpr *ccp;
struct b_coexpr *ncp;
int swtch_typ;
dptr valloc;
   {
   struct b_proc *proc;

#if !COMPILER
   inst t_ipc;
#endif					/* !COMPILER */

   --k_trace;

#if COMPILER
   showline(ccp->file_name, ccp->line_num);
   proc = PFDebug(*ccp->es_pfp)->proc;     /* get address of procedure block */
#else					/* COMPILER */

   /*
    * Compute the ipc of the instruction causing the context switch.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   proc = (struct b_proc *)BlkLoc(*argp);
#endif					/* COMPILER */

   showlevel(k_level);
   putstr(stderr, &proc->pname);
   fprintf(stderr,"; co-expression_%ld ", (long)ccp->id);
   switch (swtch_typ) {
      case A_Coact:
         fprintf(stderr,": ");
         outimage(stderr, valloc, 0);
         fprintf(stderr," @ ");
         break;
      case A_Coret:
         fprintf(stderr,"returned ");
         outimage(stderr, valloc, 0);
         fprintf(stderr," to ");
         break;
      case A_Cofail:
         fprintf(stderr,"failed to ");
         break;
      }
   fprintf(stderr,"co-expression_%ld\n", (long)ncp->id);
   fflush(stderr);
   }
#endif					/* Coexpr */

/*
 * showline - print file and line number information.
 */
static novalue showline(f, l)
char *f;
int l;
   {
   int i;

   i = (int)strlen(f);

#if MVS
   while (i > 22) {
#else					/* MVS */
   while (i > 13) {
#endif					/* MVS */
      f++;
      i--;
      }
   if (l > 0)

#if MVS
      fprintf(stderr, "%-22s: %4d  ",f, l);
   else
      fprintf(stderr, "                      :      ");
#else					/* MVS */
      fprintf(stderr, "%-13s: %4d  ",f, l);
   else
      fprintf(stderr, "              :      ");
#endif					/* MVS */

   }

/*
 * showlevel - print "| " n times.
 */
static novalue showlevel(n)
register int n;
   {
   while (n-- > 0) {
      putc('|', stderr);
      putc(' ', stderr);
      }
   }

#if !COMPILER

#include "::h:opdefs.h"


extern struct b_list value_tmp;		/* argument of Op_Apply */
extern struct b_proc *opblks[];


/*
 * ttrace - show offending expression.
 */
hidden novalue ttrace()
   {
   struct b_proc *bp;
   word nargs;

#ifndef PresentationManager
   fprintf(stderr, "   ");
#endif					/* PresentationManager */

   switch ((int)lastop) {

      case Op_Invoke:
         bp = (struct b_proc *)BlkLoc(*xargp);
         nargs = xnargs;
         if (xargp[0].dword == D_Proc)
            putstr(stderr, &(bp->pname));
         else
            outimage(stderr, xargp, 0);
         putc('(', stderr);
         while (nargs--) {
            outimage(stderr, ++xargp, 0);
            if (nargs)
               putc(',', stderr);
            }
         putc(')', stderr);
         break;

      case Op_Toby:
         putc('{', stderr);
         outimage(stderr, ++xargp, 0);
         fprintf(stderr, " to ");
         outimage(stderr, ++xargp, 0);
         fprintf(stderr, " by ");
         outimage(stderr, ++xargp, 0);
         putc('}', stderr);
         break;

      case Op_Subsc:
         putc('{', stderr);
         outimage(stderr, ++xargp, 0);

#if EBCDIC != 1
         putc('[', stderr);
#else					/* EBCDIC != 1 */
         putc('$', stderr);
         putc('<', stderr);
#endif					/* EBCDIC != 1 */

         outimage(stderr, ++xargp, 0);

#if EBCDIC != 1
         putc(']', stderr);
#else					/* EBCDIC != 1 */
         putc('$', stderr);
         putc('>', stderr);
#endif					/* EBCDIC != 1 */

         putc('}', stderr);
         break;

      case Op_Sect:
         putc('{', stderr);
         outimage(stderr, ++xargp, 0);

#if EBCDIC != 1
         putc('[', stderr);
#else					/* EBCDIC != 1 */
         putc('$', stderr);
         putc('<', stderr);
#endif					/* EBCDIC != 1 */

         outimage(stderr, ++xargp, 0);
         putc(':', stderr);
         outimage(stderr, ++xargp, 0);

#if EBCDIC != 1
         putc(']', stderr);
#else					/* EBCDIC != 1 */
         putc('$', stderr);
         putc('>', stderr);
#endif					/* EBCDIC != 1 */

         putc('}', stderr);
         break;

      case Op_Bscan:
         putc('{', stderr);
         outimage(stderr, xargp, 0);
         fputs(" ? ..}", stderr);
         break;

      case Op_Coact:
         putc('{', stderr);
         outimage(stderr, ++xargp, 0);
         fprintf(stderr, " @ ");
         outimage(stderr, ++xargp, 0);
         putc('}', stderr);
         break;

      case Op_Apply:
         outimage(stderr, xargp++, 0);
         fprintf(stderr," ! ");
         outimage(stderr, (dptr)&value_tmp, 0);
         break;

      case Op_Create:
         fprintf(stderr,"{create ..}");
         break;

      case Op_Field:
         putc('{', stderr);
         outimage(stderr, ++xargp, 0);
         fprintf(stderr, " . ");
	 ++xargp;
	 if (IntVal(*xargp) == -1)
            fprintf(stderr, "field");
	 else
            fprintf(stderr, "%s", StrLoc(fnames[IntVal(*xargp)]));
         putc('}', stderr);
         break;

      case Op_Limit:
         fprintf(stderr, "limit counter: ");
         outimage(stderr, xargp, 0);
         break;

      case Op_Llist:

#if EBCDIC != 1
         fprintf(stderr,"[ ... ]");
#else					/* EBCDIC != 1 */
         fputs("$< ... $>", stderr);
#endif					/* EBCDIC != 1 */
         break;

   
      default:
         bp = opblks[lastop];
         nargs = abs((int)bp->nparam);
         putc('{', stderr);
         if (lastop == Op_Bang || lastop == Op_Random)
            goto oneop;
         if (abs((int)bp->nparam) >= 2) {
            outimage(stderr, ++xargp, 0);
            putc(' ', stderr);
            putstr(stderr, &(bp->pname));
            putc(' ', stderr);
   	    }
         else
oneop:
         putstr(stderr, &(bp->pname));
         outimage(stderr, ++xargp, 0);
         putc('}', stderr);
      }
	 
   if (ipc.opnd != NULL)
      fprintf(stderr, " from line %d in %s", findline(ipc.opnd),
         findfile(ipc.opnd));
#ifndef PresentationManager
   putc('\n', stderr);
#endif					/* PresentationManager */
   fflush(stderr);
   }





/*
 * ctrace - procedure named s is being called with nargs arguments, the first
 *  of which is at arg; produce a trace message.
 */
novalue ctrace(dp, nargs, arg)
dptr dp;
int nargs;
dptr arg;
   {

   showline(findfile(ipc.opnd), findline(ipc.opnd));
   showlevel(k_level);
   putstr(stderr, dp);
   putc('(', stderr);
   while (nargs--) {
      outimage(stderr, arg++, 0);
      if (nargs)
         putc(',', stderr);
      }
   putc(')', stderr);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * rtrace - procedure named s is returning *rval; produce a trace message.
 */

novalue rtrace(dp, rval)
dptr dp;
dptr rval;
   {
   inst t_ipc;

   /*
    * Compute the ipc of the return instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, dp);
   fprintf(stderr, " returned ");
   outimage(stderr, rval, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * failtrace - procedure named s is failing; produce a trace message.
 */

novalue failtrace(dp)
dptr dp;
   {
   inst t_ipc;

   /*
    * Compute the ipc of the fail instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, dp);
   fprintf(stderr, " failed");
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * strace - procedure named s is suspending *rval; produce a trace message.
 */

novalue strace(dp, rval)
dptr dp;
dptr rval;
   {
   inst t_ipc;

   /*
    * Compute the ipc of the suspend instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, dp);
   fprintf(stderr, " suspended ");
   outimage(stderr, rval, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * atrace - procedure named s is being resumed; produce a trace message.
 */

novalue atrace(dp)
dptr dp;
   {
   inst t_ipc;

   /*
    * Compute the ipc of the instruction causing resumption.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, dp);
   fprintf(stderr, " resumed");
   putc('\n', stderr);
   fflush(stderr);
   }

#ifdef Coexpr
/*
 * coacttrace -- co-expression is being activated; produce a trace message.
 */
novalue coacttrace(ccp, ncp)
struct b_coexpr *ccp;
struct b_coexpr *ncp;
   {
   struct b_proc *bp;
   inst t_ipc;

   bp = (struct b_proc *)BlkLoc(*argp);
   /*
    * Compute the ipc of the activation instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, &(bp->pname));
   fprintf(stderr,"; co-expression_%ld : ", (long)ccp->id);
   outimage(stderr, (dptr)(sp - 3), 0);
   fprintf(stderr," @ co-expression_%ld\n", (long)ncp->id);
   fflush(stderr);
   }

/*
 * corettrace -- return from co-expression; produce a trace message.
 */
novalue corettrace(ccp, ncp)
struct b_coexpr *ccp;
struct b_coexpr *ncp;
   {
   struct b_proc *bp;
   inst t_ipc;

   bp = (struct b_proc *)BlkLoc(*argp);
   /*
    * Compute the ipc of the coret instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, &(bp->pname));
   fprintf(stderr,"; co-expression_%ld returned ", (long)ccp->id);
   outimage(stderr, (dptr)(&ncp->es_sp[-3]), 0);
   fprintf(stderr," to co-expression_%ld\n", (long)ncp->id);
   fflush(stderr);
   }

/*
 * cofailtrace -- failure return from co-expression; produce a trace message.
 */
novalue cofailtrace(ccp, ncp)
struct b_coexpr *ccp;
struct b_coexpr *ncp;
   {
   struct b_proc *bp;
   inst t_ipc;

   bp = (struct b_proc *)BlkLoc(*argp);
   /*
    * Compute the ipc of the cofail instruction.
    */
   t_ipc.op = ipc.op - 1;
   showline(findfile(t_ipc.opnd), findline(t_ipc.opnd));
   showlevel(k_level);
   putstr(stderr, &(bp->pname));
   fprintf(stderr,"; co-expression_%ld failed to co-expression_%ld\n",
      (long)ccp->id, (long)ncp->id);
   fflush(stderr);
   }
#endif					/* Coexpr */
#endif					/* !COMPILER */

/*
 * Service routine to display variables in given number of
 *  procedure calls to file f.
 */

int xdisp(fp,dp,count,f)
#if COMPILER
   struct p_frame *fp;
#else					/* COMPILER */
   struct pf_marker *fp;
#endif					/* COMPILER */
   register dptr dp;
   int count;
   FILE *f;
   {
   register dptr np;
   register int n;
   struct b_proc *bp;
   word nglobals, *indices;

   while (count--) {		/* go back through 'count' frames */
      if (fp == NULL)
         break;       /* needed because &level is wrong in co-expressions */

#if COMPILER
      bp = PFDebug(*fp)->proc;	/* get address of procedure block */
#else					/* COMPILER */
      bp = (struct b_proc *)BlkLoc(*dp++); /* get addr of procedure block */
      /* #%#% was: no post-increment there, but *pre*increment dp below */
#endif					/* COMPILER */

      /*
       * Print procedure name.
       */
      putstr(f, &(bp->pname));
      fprintf(f, " local identifiers:\n");

      /*
       * Print arguments.
       */
      np = bp->lnames;
      for (n = abs((int)bp->nparam); n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, np);
         fprintf(f, " = ");
         outimage(f, dp++, 0);
         putc('\n', f);
         np++;
         }

      /*
       * Print locals.
       */
#if COMPILER
      dp = fp->tend.d;
#else					/* COMPILER */
      dp = &fp->pf_locals[0];
#endif					/* COMPILER */
      for (n = bp->ndynam; n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, np);
         fprintf(f, " = ");
         outimage(f, dp++, 0);
         putc('\n', f);
         np++;
         }

      /*
       * Print statics.
       */
      dp = &statics[bp->fstatic];
      for (n = bp->nstatic; n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, np);
         fprintf(f, " = ");
         outimage(f, dp++, 0);
         putc('\n', f);
         np++;
         }

#if COMPILER
      dp = fp->old_argp;
      fp = fp->old_pfp;
#else					/* COMPILER */
      dp = fp->pf_argp;
      fp = fp->pf_pfp;
#endif					/* COMPILER */
      }

   /*
    * Print globals.  Sort names in lexical order using temporary index array.
    */

#if COMPILER
   nglobals = n_globals;
#else					/* COMPILER */
   nglobals = eglobals - globals;
#endif					/* COMPILER */

   indices = (word *)malloc((msize)nglobals * sizeof(word));
   if (indices == NULL)
      return Failed;
   else {
      for (n = 0; n < nglobals; n++)
         indices[n] = n;
      qsort ((char*)indices, (int)nglobals, sizeof(word), (int (*)())glbcmp);
      fprintf(f, "\nglobal identifiers:\n");
      for (n = 0; n < nglobals; n++) {
         fprintf(f, "   ");
         putstr(f, &gnames[indices[n]]);
         fprintf(f, " = ");
         outimage(f, &globals[indices[n]], 0);
         putc('\n', f);
         }
      fflush(f);
      free((pointer)indices);
      }
   return Succeeded;
   }

/*
 * glbcmp - compare the names of two globals using their temporary indices.
 */
static int glbcmp (pi, pj)
char *pi, *pj;
   {
   register word i = *(word *)pi;
   register word j = *(word *)pj;
   return lexcmp(&gnames[i], &gnames[j]);
   }

