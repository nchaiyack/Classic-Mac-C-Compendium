/*
 * tmain.c - main program for translator and linker.
 */

#include "::h:gsupport.h"
#include "tproto.h"

/*
 * Prototypes.
 */

hidden	novalue	execute	Params((char *ofile,char *efile,char * *args));
hidden	novalue	report Params((char *s));
hidden	novalue	rmfiles Params((char **p));
hidden	novalue	usage Params((noargs));

/*
 * The following code is operating-system dependent [@tmain.01].  Include
 *  files and such.
 */

#if PORT
Deliberate syntax error
#endif					/* PORT */

#if AMIGA
#include <libraries/dosextens.h>
#endif					/* AMIGA */

#if ARM || MVS || UNIX || VM || VMS
/* nothing is needed */
#endif					/* ARM || ... */

#if MSDOS
   char pathToIconDOS[129];
#endif					/* MSDOS */

#if ATARI_ST
char *patharg;
#endif					/* ATARI_ST */

#if MACINTOSH
#if MPW
#include <fcntl.h>	/* MPW3 - for unlink() */
#include <CursorCtl.h>
void SortOptions();
#endif					/* MPW */
#endif					/* MACINTOSH */

#if OS2
#include <process.h>
#endif					/* OS2 */
/*
 * End of operating-system specific code.
 */

#if IntBits == 16
#ifdef strlen
#undef strlen				/* pre-defined in some contexts */
#endif					/* strlen */
#endif					/* Intbits == 16 */

/*
 *  Define global variables.
 */

#define Global
#define Init(v) = v
#include "tglobals.h"

char *ofile = NULL;			/* linker output file name */

char patchpath[MaxPath+18] = "%PatchStringHere->";

/*
 * The following code is operating-system dependent [@tmain.02].  Definition
 *  of refpath.
 */

#if PORT
   /* something is needed */
Deliberate Syntax Error
#endif					/* PORT */

#if UNIX || AMIGA || ATARI_ST || MACINTOSH || MSDOS || MVS || OS2 || VM
char *refpath = RefPath;
#endif					/* UNIX ... */

#if VMS
char *refpath = "ICON_BIN:";
#endif					/* VMS */

/*
 * End of operating-system specific code.
 */

/*
 * getopt() variables
 */
extern int optindex;		/* index into parent argv vector */
extern int optopt;		/* character checked for validity */
extern char *optarg;		/* argument associated with option */

/*
 *  main program
 */
novalue main(argc,argv)
int argc;
char **argv;
   {
   int nolink = 0;			/* suppress linking? */
   int errors = 0;			/* translator and linker errors */
   char **tfiles, **tptr;		/* list of files to translate */
   char **lfiles, **lptr;		/* list of files to link */
   char **rfiles, **rptr;		/* list of files to remove */
   char *efile = NULL;			/* stderr file */
   char buf[MaxFileName];		/* file name construction buffer */
   int c, n;
   char ch;
   struct fileparts *fp;

#if AMIGA
#if AZTEC_C
   struct Process *FindTask();
   struct Process *Process = FindTask(0L);
   ULONG stacksize = *((ULONG *)Process->pr_ReturnAddr);

   if (stacksize < ICONTMINSTACK) {
      fprintf(stderr,"Icont needs \"stack %d\" to run\n",ICONTMINSTACK);
      exit(-1);
      }
#endif					/* AZTEC_C */
#endif					/* AMIGA */


#if MACINTOSH
#if MPW
   InitCursorCtl(NULL);
   SortOptions(argv);
#endif					/* MPW */
#endif					/* MACINTOSH */

   iconxloc = (char *)alloc((unsigned)strlen(refpath) + 6);
   strcpy(iconxloc, refpath);
   strcat(iconxloc, "iconx");

   if ((int)strlen(patchpath) > 18)
      iconxloc = patchpath+18;

   /*
    * Process options.
    */
   while ((c = getopt(argc,argv,IconOptions)) != EOF)
      switch (c) {
         case 'C':			/* Ignore: compiler only */
            break;
         case 'E':			/* -E: preprocess only */
	    pponly = 1;
	    nolink = 1;
            break;

         case 'L':			/* -L: enable linker debugging */

#ifdef DeBugLinker
            Dflag = 1;
#endif					/* DeBugLinker */

            break;

         case 'S':			/* -S */
            fprintf(stderr, "Warning: -S option is obsolete\n");
            break;

#if MSDOS
         case 'X':			/* -X */
#if ZTC_386
            fprintf(stderr, "Warning: -X option is not available\n");
#else					/* ZTC_386 */
            makeExe = 1;
#endif					/* ZTC_386 */
            break;
         case 'I':			/* -C */
            makeExe = 0;
            break;
#endif					/* MSDOS */

         case 'c':			/* -c: compile only (no linking) */
            nolink = 1;
            break;
         case 'e':			/* -e file: redirect stderr */
            efile = optarg;
            break;
         case 'f':			/* -f features: enable features */
            if (index(optarg, 's') || index(optarg, 'a'))
               strinv = 1;		/* this is the only icont feature */
            break;

#if OS2
	 case 'i':                      /* -i: Don't create .EXE file */
	    noexe = 1;
	    break;
#endif					/* OS2 */

         case 'm':			/* -m: preprocess using m4(1) [UNIX] */
            m4pre = 1;
            break;
         case 'n':			/* Ignore: compiler only */
            break;
         case 'o':			/* -o file: name output file */
            ofile = optarg;
            break;

         case 'p':			/* -p path: iconx path [ATARI] */

#if ATARI_ST
            patharg = optarg;
#endif					/* ATARI_ST */

            break;

         case 'r':			/* Ignore: compiler only */
            break;
         case 's':			/* -s: suppress informative messages */
            silent = 1;
            verbose = 0;
            break;
         case 't':			/* -t: turn on procedure tracing */
            trace = -1;
            break;
         case 'u':			/* -u: warn about undeclared ids */
            uwarn = 1;
            break;
         case 'v':			/* -v n: set verbosity level */
            if (sscanf(optarg, "%d%c", &verbose, &ch) != 1)
               quitf("bad operand to -v option: %s",optarg);
            if (verbose == 0)
               silent = 1;
            break;
         default:
         case 'x':			/* -x illegal until after file list */
            usage();
         }

#if MSDOS

      /*
       * Define pathToIconDOS as a global accessible from inside
       * separately-compiled compilation units.
       */

      if( makeExe ){
         char *pathCursor;

         strcpy (pathToIconDOS, argv[0]);
         pathCursor = (char *)strrchr (pathToIconDOS, '\\');
         if (!pathCursor) {
            fprintf (stderr,
               "Can't understand what directory icont was run from.\n");
            exit(ErrorExit);
            }
            strcpy (++pathCursor, "ixhdr.exe");
         }
#endif                                  /* MSDOS */

   /*
    * Allocate space for lists of file names.
    */
   n = argc - optindex + 1;
   tptr = tfiles = (char **)alloc((unsigned int)(n * sizeof(char *)));
   lptr = lfiles = (char **)alloc((unsigned int)(n * sizeof(char *)));
   rptr = rfiles = (char **)alloc((unsigned int)(2 * n * sizeof(char *)));

   /*
    * Scan file name arguments.
    */
   while (optindex < argc)  {
      if (strcmp(argv[optindex],"-x") == 0)	/* stop at -x */
         break;
      else if (strcmp(argv[optindex],"-") == 0) {

#if ARM
	/* Different file naming, so we need a different strategy... */
	*tptr++ = "-";
	/* Use makename(), pretending we had an input file named "Stdin" */
	makename(buf,TargetDir,"Stdin",U1Suffix);
	*lptr++ = *rptr++ = salloc(buf);	/* link & remove .u1 */
	makename(buf,TargetDir,"Stdin",U2Suffix);
	*rptr++ = salloc(buf);		/* also remove .u2 */

#else					/* ARM */

         *tptr++ = "-";				/* "-" means standard input */
         *lptr++ = *rptr++ = "stdin.u1";
         *rptr++ = "stdin.u2";
#endif					/* ARM */

         }
      else {
         fp = fparse(argv[optindex]);		/* parse file name */
         if (*fp->ext == '\0' || smatch(fp->ext, SourceSuffix)) {
            makename(buf,SourceDir,argv[optindex], SourceSuffix);
#if VMS
	    strcat(buf, fp->version);
#endif					/* VMS */
            *tptr++ = salloc(buf);		/* translate the .icn file */
            makename(buf,TargetDir,argv[optindex],U1Suffix);
            *lptr++ = *rptr++ = salloc(buf);	/* link & remove .u1 */
            makename(buf,TargetDir,argv[optindex],U2Suffix);
            *rptr++ = salloc(buf);		/* also remove .u2 */
            }
         else if (smatch(fp->ext,U1Suffix) || smatch(fp->ext,U2Suffix)
               || smatch(fp->ext,USuffix)) {
            makename(buf,TargetDir,argv[optindex],U1Suffix);
            *lptr++ = salloc(buf);
            }
         else
            quitf("bad argument %s",argv[optindex]);
         }
      optindex++;
      }
 
   *tptr = *lptr = *rptr = NULL;	/* terminate filename lists */
   if (lptr == lfiles)
      usage();				/* error -- no files named */

   /*
    * Round hash table sizes to next power of two, and set masks for hashing.
    */
   lchsize = round2(lchsize);  cmask = lchsize - 1;
   fhsize = round2(fhsize);  fmask = fhsize - 1;
   ghsize = round2(ghsize);  gmask = ghsize - 1;
   ihsize = round2(ihsize);  imask = ihsize - 1;
   lhsize = round2(lhsize);  lmask = lhsize - 1;

   /*
    * Translate .icn files to make .u1 and .u2 files.
    */
   if (tptr > tfiles) {
      if (!silent && !pponly)
         report("Translating");
      errors = trans(tfiles);
      if (errors > 0)			/* exit if errors seen */
         exit(ErrorExit);
      }

   /*
    * Link .u1 and .u2 files to make an executable.
    */
   if (nolink) {			/* exit if no linking wanted */

#if MACINTOSH
#if MPW
      /*
       *  Set type of translator output ucode (.u) files
       *  to 'TEXT', so they can be easily viewed by editors.
       */
      {
      char **p;
      void setfile();
      for (p = rfiles; *p; ++p)
         setfile(*p,'TEXT','UCOD');
      }
#endif					/* MPW */
#endif					/* MACINTOSH */

      exit(NormalExit);
      }

#if MSDOS

   if (ofile == NULL)  {                /* if no -o file, synthesize a name */
      ofile = salloc(makename(buf,TargetDir,lfiles[0],
                              makeExe ? ".Exe" : IcodeSuffix));
   } else {                             /* add extension if necessary */
      fp = fparse(ofile);
      if (*fp->ext == '\0' && *IcodeSuffix != '\0') /* if no ext given */
         ofile = salloc(makename(buf,NULL,ofile,
                                 makeExe ? ".Exe" : IcodeSuffix));
   }

#else                                   /* MSDOS */

   if (ofile == NULL)  {		/* if no -o file, synthesize a name */
      ofile = salloc(makename(buf,TargetDir,lfiles[0],IcodeSuffix));
   } else {				/* add extension in necessary */
      fp = fparse(ofile);
      if (*fp->ext == '\0' && *IcodeSuffix != '\0') /* if no ext given */
         ofile = salloc(makename(buf,NULL,ofile,IcodeSuffix));
   }

#endif					/* MSDOS */

   if (!silent)
      report("Linking");
   errors = ilink(lfiles,ofile);	/* link .u files to make icode file */

   /*
    * Finish by removing intermediate files.
    *  Execute the linked program if so requested and if there were no errors.
    */

#if MACINTOSH
#if MPW
   /* Set file type to TEXT so it will be executable as a script. */
   setfile(ofile,'TEXT','ICOD');
#endif					/* MPW */
#endif					/* MACINTOSH */

   rmfiles(rfiles);			/* remove intermediate files */
   if (errors > 0) {			/* exit if linker errors seen */
      unlink(ofile);
      exit(ErrorExit);
      }

#if !(MACINTOSH && MPW)
   if (optindex < argc)  {
      if (!silent)
         report("Executing");
      execute (ofile, efile, argv+optindex+1);
      }
#endif					/* !(MACINTOSH && MPW) */

   exit(NormalExit);
   }

/*
 * execute - execute iconx to run the icon program
 */
static novalue execute(ofile,efile,args)
char *ofile, *efile, **args;
   {

#if !(MACINTOSH && MPW)
   int n;
   char **argv, **p;

   for (n = 0; args[n] != NULL; n++)	/* count arguments */
      ;
   p = argv = (char **)alloc((unsigned int)((n + 5) * sizeof(char *)));

   *p++ = iconxloc;			/* set iconx pathname */
   if (efile != NULL) {			/* if -e given, copy it */
      *p++ = "-e";
      *p++ = efile;
      }
   *p++ = ofile;			/* pass icode file name */

#if AMIGA && LATTICE
   *p = *args;
   while (*p++) {
      *p = *args;
      args++;
   }
#else					/* AMIGA && LATTICE */
   while (*p++ = *args++)		/* copy args into argument vector */
      ;
#endif					/* AMIGA && LATTICE */

   *p = NULL;

/*
 * The following code is operating-system dependent [@tmain.03].  It calls
 *  iconx on the way out.
 */

#if PORT
   /* something is needed */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA
#if AZTEC_C
      execvp(iconxloc,argv);
      return;
#endif					/* AZTEC_C */
#if LATTICE
      {
      struct ProcID procid;
      if (forkv(iconxloc,argv,NULL,&procid) == 0) { 
         wait(&procid);
         return;
         }
      }
#endif					/* LATTICE */
#endif					/* AMIGA */

#if ARM
   {
      int i = 7 + strlen(iconxloc);
      int j;
      char *s;
      char buffer[255];
      extern int armquote(char *, char **);

      sprintf(buffer, "Chain:%s ", iconxloc);
      for (p = argv + 1; *p; ++p)
      {
         j = armquote(*p, &s);

         if (j == -1 || i + j >= 255)
         {
            fprintf(stderr, "Cannot execute: command line too long");
            fflush(stderr);
            return;
         }

         strcpy(buffer + i, s);
         i += j;
         buffer[i] = ' ';
      }
      buffer[i] = '\0';
      system(buffer);
   }
#endif					/* ARM */

#if ATARI_ST || MACINTOSH
      fprintf(stderr,"-x not supported\n");
      fflush(stderr);
#endif					/* ATARI_ST || ... */

#if MSDOS
      /* No special handling is needed for an .exe files, since iconx
       * recognizes it from the extension andfrom internal .exe data.
       */
#if MICROSOFT || TURBO
      execvp(iconxloc,argv);	/* execute with path search */
#endif					/* MICROSOFT || ... */
#if INTEL_386 || ZTC_386 || HIGHC_386 || WATCOM
      fprintf(stderr,"-x not supported\n");
      fflush(stderr);
#endif					/* INTEL_386 || ... */
#endif					/* MSDOS */

#if MVS || VM
#if SASC
	  exit(sysexec(iconxloc, argv));
#endif					/* SASC */
      fprintf(stderr,"-x not supported\n");
      fflush(stderr);
#endif                                  /* MVS || VM */

#if OS2
#ifdef PresentationManager
      fputs("-x not supported\n", stderr);
#else					/* PresentationManager */
      execvp(iconxloc,argv);	/* execute with path search */
#endif					/* PresentationManager */
#endif					/* OS2 */

#if UNIX
      /*
       * If an ICONX environment variable is defined, use that.
       *  If not, first try the predefined path, then search $PATH via execvp. 
       */
      if ((argv[0] = getenv("ICONX")) != NULL && argv[0][0] != '\0') {
         execv(argv[0], argv);	/* exec file specified by $ICONX */
         quitf("cannot execute $ICONX (%s)", argv[0]);
         }

#ifdef HardWiredPaths
#ifdef CRAY
      argv[0] = "iconx";
      execv(iconxloc, argv);
#else					/* CRAY */
      argv[0] = iconxloc;		/* try predefined file */
      execv(argv[0], argv);
#endif					/* CRAY */
#endif					/* HardWiredPaths */

      argv[0] = "iconx";
      execvp(argv[0], argv);	/* if no iconxloc, search path for "iconx" */

#ifdef HardWiredPaths
      quitf("cannot run %s", iconxloc);
#else					/* HardWiredPaths */
      quitf("cannot find iconx", "");
#endif					/* HardWiredPaths */
#endif					/* UNIX */

#if VMS
      execv(iconxloc,argv);
#endif					/* VMS */

/*
 * End of operating-system specific code.
 */

   quitf("could not run %s",iconxloc);

#else					/* !(MACINTOSH && MPW) */
   printf("-x not supported\n");
#endif					/* !(MACINZTOSH && MPW) */

   }

static novalue report(s)
char *s;
   {

   fprintf(stderr,"%s:\n",s);

   }

/*
 * rmfiles - remove a list of files
 */

static novalue rmfiles(p)
char **p;
   {
   for (; *p; p++) {
      unlink(*p);
      }
   }

/*
 * Print an error message if called incorrectly.  The message depends
 *  on the legal options for this system.
 */
static novalue usage()
   {

#if MVS || VM
   fprintf(stderr,"usage: %s %s file ... <-x args>\n", progname, TUsage);
#elif MPW
   fprintf(stderr,"usage: %s %s file ...\n", progname, TUsage);
#else
   fprintf(stderr,"usage: %s %s file ... [-x args]\n", progname, TUsage);
#endif

   exit(ErrorExit);
   }
