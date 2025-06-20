#if !COMPILER
/*
 * File: imain.r
 * Interpreter main program, argument handling, and such.
 * Contents: main, icon_call, icon_setup, resolve, xmfree
 */

#include "::h:version.h"
#include "::h:header.h"
#include "::h:opdefs.h"

/*
 * Prototypes.
 */

hidden	novalue	env_err	Params((char *msg,char *name,char *val));
novalue	icon_setup	Params((int argc, char **argv, int *ip));

/*
 * The following code is operating-system dependent [@imain.01].  Declarations
 *   that are system-dependent.
 */

#if PORT
   /* probably needs something more */
Deliberate Syntax Error
#endif					/* PORT */

#if MACINTOSH
#if MPW
int NoOptions = 0;
#endif					/* MPW */
#endif					/* MACINTOSH */

#if AMIGA || ARM || ATARI_ST || MSDOS || MVS || VM || OS2 || UNIX\
   || VMS
   /* nothing needed */
#endif					/* AMIGA || ARM || ATARI_ST ... */

/*
 * End of operating-system specific code.
 */

#ifdef MemMon

#ifndef MultiThread
extern FILE *monfile;
#endif					/* MultiThread */

char *monfname;
#endif					/* MemMon */

extern set_up;

/*
 * A number of important variables follow.
 */

#ifndef MultiThread
int n_globals = 0;			/* number of globals */
int n_statics = 0;			/* number of statics */
#endif					/* MultiThread */

extern int_setup;


/*
 * Initial icode sequence. This is used to invoke the main procedure with one
 *  argument.  If main returns, the Op_Quit is executed.
 */
word istart[3];
int mterm = Op_Quit;



#ifdef MSWindows
int CmdParamToArgv(char *s, char ***avp)
   {
   char *t = salloc(s), *t2;
   int rv=0;
   t2 = t;
   while (*t2) {
      while (isspace(*t2)) t2++;
      if (*t2 == '\0') break;
      if (!isspace(*t2)) {
         rv++;
         while (!isspace(*t2)) t2++;
         }
      }
   rv++; /* make room for "iconx" at front */
   *avp = (char **)alloc(rv * sizeof(char *));
   rv = 0;
   (*avp)[rv++] = salloc("iconx.exe");
   t2 = t;
   while (*t2) {
      while (isspace(*t2)) t2++;
      if (*t2 == '\0') break;
      if (!isspace(*t2)) {
         (*avp)[rv++] = t2;
         while ((*t2 != '\0') && !isspace(*t2)) t2++;
         if (*t2) *t2++ = '\0';
         }
      }
   return rv;  
   }

novalue MSStartup(int argc, char **argv, HINSTANCE hInstance, HINSTANCE hPrevInstance)
   {
   WNDCLASS wc;
#ifdef MSNT
   (void) AllocConsole();
#endif					/* MSNT */
   if (!hPrevInstance) {
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = WndProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance  = hInstance;
      wc.hIcon      = LoadIcon(NULL, IDI_APPLICATION);
      wc.hCursor    = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = GetStockObject(WHITE_BRUSH);
      wc.lpszMenuName = NULL;
      wc.lpszClassName = "iconx";
      RegisterClass(&wc);
      }
   }
#endif					/* MSWindows */


#ifdef MSWindows
int_PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdParam, int nCmdShow)
#else					/* MSWindows */

novalue main(argc, argv)

int argc;
char **argv;
#endif					/* MSWindows */
   {
#ifdef PresentationManager
    threadargs *args;

    /* fill in structure to pass to the interp thread */
    args = (threadargs *)calloc(sizeof(threadargs), 1);
    args->argv = argv;
    args->argc = argc;
    /* initialize PM and startup interp thread */
    PMStartup((void *)args);
/* xxx should wait for the child thread to end */
    /* should never get here - but just in case, shut everything down */
    DosExit(EXIT_PROCESS, 1);
   } /* End of main - Presentation Manager version */

/* this is where the second thread will start up */
void InterpThreadStartup(void *args)
{
#endif					/* PresentationManager */
   int i, slen;

#if AMIGA
#if AZTEC_C
   struct Process *FindTask();
   struct Process *Process = FindTask(0L);
   ULONG stacksize = *((ULONG *)Process->pr_ReturnAddr);

   if (stacksize < ICONXMINSTACK) {
      fprintf(stderr,"Iconx needs \"stack %d\" to run\n",ICONXMINSTACK);
      exit(-1);
      }
#endif					/* AZTEC_C */
#endif					/* AMIGA */

#ifdef MSWindows
   int argc;
   char **argv;

   mswinInstance = hInstance;
   ncmdShow = nCmdShow;
   argc = CmdParamToArgv(lpszCmdParam, &argv);
   MSStartup(argc, argv, hInstance, hPrevInstance);
#endif					/* MSWindows */

#ifdef PresentationManager
   char **argv;
   int argc;

   /* extract the command line args */
   argv = ((threadargs *)args)->argv;
   argc = ((threadargs *)args)->argc;
   free(args);

   /* make our anchor block */
   HInterpAnchorBlock = WinInitialize(0);
   /* make our queue - we want to store more messages than default */
   HInterpMessageQueue = WinCreateMsgQueue(HInterpAnchorBlock, MAXMSGS);
#endif					/* PresentationManager */

#ifdef SuppressAlignmentMsg
   {
      /*
       * suppress "Unaligned access" messages on Dec Alpha.
       */
      int buf[] = {SSIN_UACPROC, UAC_NOPRINT};
      setsysinfo(SSI_NVPAIRS, buf, 1, 0, 0);
   }
#endif				/* SuppressAlignmentMsg */

#if SASC
   quiet(1);                    /* suppress C library diagnostics */
#endif					/* SASC */

   ipc.opnd = NULL;

   /*
    * Setup Icon interface.  It's done this way to avoid duplication
    *  of code, since the same thing has to be done if calling Icon
    *  is enabled.  See istart.c.
    */

#ifdef CRAY
   argv[0] = "iconx";
#endif					/* CRAY */

   icon_setup(argc, argv, &i);

   if (i < 0) {
      argc++;
      argv--;
      i++;
      }

   while (i--) {			/* skip option arguments */
      argc--;
      argv++;
      }

   if (argc <= 1) 
#ifdef PresentationManager
      error(NULL, "An icode file was not specified.\nExecution cannot proceed.");
#else					/* PresentationManager */
      error(NULL, "no icode file specified");
#endif					/* PresentationManager */
   /*
    * Call icon_init with the name of the icode file to execute.	[[I?]]
    */


   icon_init(argv[1], &argc, argv);

   /*
    *  Point sp at word after b_coexpr block for &main, point ipc at initial
    *	icode segment, and clear the gfp.
    */

   stackend = stack + mstksize/WordSize;
   sp = stack + Wsizeof(struct b_coexpr);

   ipc.opnd = istart;
   *ipc.op++ = Op_Invoke;				/*	[[I?]] */

#if AMIGA
   istart[0] = Op_Invoke;
   istart[1] = 1;
   istart[2] = Op_Quit;
#else					/* AMIGA */
   *ipc.opnd++ = 1;
   *ipc.op = Op_Quit;
   ipc.opnd = istart;
#endif					/* AMIGA */

   gfp = 0;

   /*
    * Set up expression frame marker to contain execution of the
    *  main procedure.  If failure occurs in this context, control
    *  is transferred to mterm, the address of an Op_Quit.
    */
   efp = (struct ef_marker *)(sp);
   efp->ef_failure.op = &mterm;
   efp->ef_gfp = 0;
   efp->ef_efp = 0;
   efp->ef_ilevel = 1;
   sp += Wsizeof(*efp) - 1;

   pfp = 0;
   ilevel = 0;

/*
 * We have already loaded the
 * icode and initialized things, so it's time to just push main(),
 * build an Icon list for the rest of the arguments, and called
 * interp on a "invoke 1" bytecode.
 */
   /*
    * The first global variable holds the value of "main".  If it
    *  is not of type procedure, this is noted as run-time error 117.
    *  Otherwise, this value is pushed on the stack.
    */
   if (globals[0].dword != D_Proc)
      fatalerr(117, NULL);
   PushDesc(globals[0]);
   PushNull;
   argp = (dptr)(sp - 1);

   /*
    * If main() has a parameter, it is to be invoked with one argument, a list
    *  of the command line arguments.  The command line arguments are pushed
    *  on the stack as a series of descriptors and Ollist is called to create
    *  the list.  The null descriptor first pushed serves as Arg0 for
    *  Ollist and receives the result of the computation.
    */

   if (((struct b_proc *)BlkLoc(globals[0]))->nparam > 0) {
      for (i = 2; i < argc; i++) {
         char *tmp;
         slen = strlen(argv[i]);
         PushVal(slen);
         Protect(tmp=alcstr(argv[i],(word)slen), fatalerr(0,NULL));
         PushAVal(tmp);
         }

      Ollist(argc - 2, argp);
      }


   sp = (word *)argp + 1;
   argp = 0;

   set_up = 1;			/* post fact that iconx is initialized */

   /*
    * Start things rolling by calling interp.  This call to interp
    *  returns only if an Op_Quit is executed.	If this happens,
    *  c_exit() is called to wrap things up.
    */

#ifdef CoProcesses
   codisp();    /* start up co-expr dispatcher, which will call interp */
#else					/* CoProcesses */
   interp(0,(dptr)NULL);                        /*      [[I?]] */
#endif					/* CoProcesses */

   c_exit(NormalExit);
}


/*
 * icon_setup - handle interpreter command line options.
 */
novalue icon_setup(argc,argv,ip)
int argc;
char **argv;
int *ip;
   {

#ifdef TallyOpt
   extern int tallyopt;
#endif					/* TallyOpt */

   *ip = 0;			/* number of arguments processed */

#ifdef ExecImages
   if (dumped) {
      /*
       * This is a restart of a dumped interpreter.  Normally, argv[0] is
       *  iconx, argv[1] is the icode file, and argv[2:(argc-1)] are the
       *  arguments to pass as a list to main().  For a dumped interpreter
       *  however, argv[0] is the executable binary, and the first argument
       *  for main() is argv[1].  The simplest way to handle this is to
       *  back up argv to point at argv[-1] and increment argc, giving the
       *  illusion of an additional argument at the head of the list.  Note
       *  that this argument is never referenced.
       */
      argv--;
      argc++;
      (*ip)--;
      }
#endif					/* ExecImages */

#ifdef MaxLevel
   maxilevel = 0;
   maxplevel = 0;
   maxsp = 0;
#endif					/* MaxLevel */

#if MACINTOSH
#if MPW
   InitCursorCtl(NULL);
   /*
    * To support the icode and iconx interpreter bundled together in
    * the same file, we might have to use this code file as the icode
    * file, too.  We do this if the command name is not 'iconx'.
    */
   {
   char *p,*q,c,fn[6];

   /*
    * Isolate the filename from the path.
    */
   q = strrchr(*argv,':');
   if (q == NULL)
       q = *argv;
   else
       ++q;
   /*
    * See if it's the real iconx -- case independent compare.
    */
   p = fn;
   if (strlen(q) == 5)
      while (c = *q++) *p++ = tolower(c);
   *p = '\0';
   if (strcmp(fn,"iconx") != 0) {
     /*
      * This technique of shifting arguments relies on the fact that
      * argv[0] is never referenced, since this will make it invalid.
      */
      --argv;
      ++argc;
      --(*ip);
      /*
       * We don't want to look for any command line options in this
       * case.  They could interfere with options for the icon
       * program.
       */
      NoOptions = 1;
      }
   }
#endif					/* MPW */
#endif                                  /* MACINTOSH */

/*
 * Handle command line options.
*/
#if MACINTOSH && MPW
   if (!NoOptions)
#endif					/* MACINTOSH && MPW */
   while ( argv[1] != 0 && *argv[1] == '-' ) {
      switch ( *(argv[1]+1) ) {

#ifdef TallyOpt
	/*
	 * Set tallying flag if -T option given
	 */
	case 'T':
	    tallyopt = 1;
	    break;
#endif					/* TallyOpt */

#ifdef MemMon
	/*
	 * Check for command-line event monitor enable
	 */
	case 'E': {
	    char *p;
	    if ( *(argv[1]+2) != '\0' )
	       p = argv[1]+2;
	    else {
	       argv++;
	       argc--;
               (*ip)++;
	       p = argv[1];
	       if ( !p )
		  error(NULL, "no file name given for event monitor file");
	       }
		monfname = p;
		break;
		}
#endif			/* MemMon */


      /*
       * Set stderr to new file if -e option is given.
       */
	 case 'e': {
	    char *p;
	    if ( *(argv[1]+2) != '\0' )
	       p = argv[1]+2;
	    else {
	       argv++;
	       argc--;
               (*ip)++;
	       p = argv[1];
	       if ( !p )
		  error(NULL, "no file name given for redirection of &errout");
	       }
            if (!redirerr(p))
               syserr("Unable to redirect &errout\n");
	    break;
	    }
        }
	argc--;
        (*ip)++;
	argv++;
      }
   }

/*
 * resolve - perform various fix-ups on the data read from the icode
 *  file.
 */
#ifdef MultiThread			/* MultiThread */
novalue resolve(pstate)
struct progstate *pstate;
#else					/* MultiThread */
novalue resolve()
#endif					/* MultiThread */

   {
   register word i, j;
   register struct b_proc *pp;
   register dptr dp;
   extern Omkrec();
   extern int ftsize;

#ifdef MultiThread
   register struct progstate *savedstate;
#endif					/* MultiThread */


#ifdef MultiThread
   savedstate = curpstate;
   if (pstate) curpstate = pstate;
#endif					/* MultiThread */

   /*
    * Relocate the names of the global variables.
    */
   for (dp = gnames; dp < egnames; dp++)
      StrLoc(*dp) = strcons + (uword)StrLoc(*dp);

   /*
    * Scan the global variable array for procedures and fill in appropriate
    *  addresses.
    */
   for (j = 0; j < n_globals; j++) {

      if (globals[j].dword != D_Proc)
         continue;

      /*
       * The second word of the descriptor for procedure variables tells
       *  where the procedure is.  Negative values are used for built-in
       *  procedures and positive values are used for Icon procedures.
       */
      i = IntVal(globals[j]);

      if (i < 0) {
         /*
          * globals[j] points to a built-in function; call (bi_)strprc
	  *  to look it up by name in the interpreter's table of built-in
	  *  functions.
          */
	 if((BlkLoc(globals[j])= (union block *)bi_strprc(gnames+j,0)) == NULL)
            globals[j] = nulldesc;		/* undefined, set to &null */
         }
      else {

         /*
          * globals[j] points to an Icon procedure or a record; i is an offset
          *  to location of the procedure block in the code section.  Point
          *  pp at the block and replace BlkLoc(globals[j]).
          */
         pp = (struct b_proc *)(code + i);
         BlkLoc(globals[j]) = (union block *)pp;

         /*
          * Relocate the address of the name of the procedure.
          */
         StrLoc(pp->pname) = strcons + (uword)StrLoc(pp->pname);

#ifdef MultiThread
         pp->program = curpstate;
#endif					/* MultiThread */

         if (pp->ndynam == -2) {
            /*
             * This procedure is a record constructor.	Make its entry point
             *	be the entry point of Omkrec().
             */
            pp->entryp.ccode = Omkrec;

	    /*
	     * Initialize field names
	     */
            for (i = 0; i < pp->nfields; i++)
               StrLoc(pp->lnames[i]) = strcons + (uword)StrLoc(pp->lnames[i]);

	    }
         else {
            /*
             * This is an Icon procedure.  Relocate the entry point and
             *	the names of the parameters, locals, and static variables.
             */
            pp->entryp.icode = code + pp->entryp.ioff;
            for (i = 0; i < abs((int)pp->nparam)+pp->ndynam+pp->nstatic; i++)
               StrLoc(pp->lnames[i]) = strcons + (uword)StrLoc(pp->lnames[i]);
            }
         }
      }

   /*
    * Relocate the names of the fields.
    */

   for (dp = fnames; dp < efnames; dp++)
      StrLoc(*dp) = strcons + (uword)StrLoc(*dp);

#ifdef MultiThread
   curpstate = savedstate;
#endif						/* MultiThread */
   }


/*
 * Free malloc-ed memory; the main regions then co-expressions.  Note:
 *  this is only correct if all allocation is done by routines that are
 *  compatible with free() -- which may not be the case if Allocreg()
 *  in rmemfix.c is defined to be other than malloc().
 */

novalue xmfree()
   {
#ifdef FixedRegions
   register struct b_coexpr **ep, *xep;
   register struct astkblk *abp, *xabp;

   if (mainhead != (struct b_coexpr *)NULL)
      free((pointer)mainhead->es_actstk);	/* activation block for &main */
   free((pointer)code);			/* icode */
   code = NULL;
   free((pointer)stack);		/* interpreter stack */
   stack = NULL;
   free((pointer)strbase);		/* allocated string region */
   strbase = NULL;
   free((pointer)blkbase);		/* allocated block region */
   blkbase = NULL;
   free((pointer)quallist);		/* qualifier list */
   quallist = NULL;

   /*
    * The co-expression blocks are linked together through their
    *  nextstk fields, with stklist pointing to the head of the list.
    *  The list is traversed and each stack is freeing.
    */
   ep = &stklist;
   while (*ep != NULL) {
      xep = *ep;
      *ep = (*ep)->nextstk;
       /*
        * Free the astkblks.  There should always be one and it seems that
        *  it's not possible to have more than one, but nonetheless, the
        *  code provides for more than one.
        */
 	 for (abp = xep->es_actstk; abp; ) {
            xabp = abp;
            abp = abp->astk_nxt;
            free((pointer)xabp);
            }

#ifdef CoProcesses
         coswitch(BlkLoc(k_current)->coexpr.cstate, xep->cstate, -1);
                /* terminate coproc for coexpression first */
#endif					/* CoProcesses */

      free((pointer)xep);
   stklist = NULL;
   }
#endif					/* Fixed Regions */

   }
#endif					/* !COMPILER */
