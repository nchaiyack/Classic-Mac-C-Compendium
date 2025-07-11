/*
 * link.c -- linker main program that controls the linking process.
 */

#include "link.h"
#include "tproto.h"
#include "tglobals.h"
#include "::h:header.h"

#ifdef Header
#ifndef ShellHeader
#include "hdr.h"
#endif					/* ShellHeader */

#ifndef MaxHeader
#define MaxHeader MaxHdr
#endif					/* MaxHeader */

#endif					/* Header */

/*
 * Prototype.
 */

hidden	novalue	setexe	Params((char *fname));


/*
 * The following code is operating-system dependent [@link.01].  Include
 *  system-dependent files and declarations.
 */

#if PORT
   /* nothing to do */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ATARI_ST || MACINTOSH || VM || VMS
   /* nothing to do */
#endif					/* AMIGA || ATARI_ST || ... */

#if ARM
#include "kernel.h"
#include "swis.h"
#endif					/* ARM */

#if MSDOS
   extern char pathToIconDOS[];
#if MICROSOFT || TURBO
#include <fcntl.h>
#endif					/* MICROSOFT || TURBO */
#endif					/* MSDOS */

#if MVS
char *routname;				/* real output file name */
#endif					/* MVS */

#if OS2
#if MICROSOFT || CSET2
#include <fcntl.h>
#endif					/* MICROSOFT || CSET2 */
#endif					/* OS2 */

#if UNIX
#ifdef CRAY
#define word word_fubar
#include <sys/types.h>
#include <sys/stat.h>
#undef word
#else					/* CRAY */
#ifndef XWindows
#include <sys/types.h>
#endif					/* XWindows */
#include <sys/stat.h>
#endif					/* CRAY */
#endif					/* UNIX */

/*
 * End of operating-system specific code.
 */

FILE *infile;				/* input file (.u1 or .u2) */
FILE *outfile;				/* interpreter code output file */

#ifdef DeBugLinker
FILE *dbgfile;				/* debug file */
static char dbgname[MaxFileName];	/* debug file name */
#endif					/* DeBugLinker */

char inname[MaxFileName];		/* input file name */
static char icnname[MaxFileName];	/* icon source file name */

struct lfile *llfiles = NULL;		/* List of files to link */

int colmno = 0;				/* current source column number */
int lineno = 0;				/* current source line number */
int fatals = 0;				/* number of errors encountered */

/*
 *  ilink - link a number of files, returning error count
 */
int ilink(ifiles,outname)
char **ifiles;
char *outname;
   {


   int i;
   struct lfile *lf,*lfls;
   char *filename;			/* name of current input file */

   linit();				/* initialize memory structures */
   while (*ifiles)
      alsolink(*ifiles++);		/* make initial list of files */

   /*
    * Phase I: load global information contained in .u2 files into
    *  data structures.
    *
    * The list of files to link is maintained as a queue with llfiles
    *  as the base.  lf moves along the list.  Each file is processed
    *  in turn by forming .u2 and .icn names from each file name, each
    *  of which ends in .u1.  The .u2 file is opened and globals is called
    *  to process it.  When the end of the list is reached, lf becomes
    *  NULL and the loop is terminated, completing phase I.  Note that
    *  link instructions in the .u2 file cause files to be added to list
    *  of files to link.
    */
   for (lf = llfiles; lf != NULL; lf = lf->lf_link) {
      filename = lf->lf_name;
      makename(inname, SourceDir, filename, U2Suffix);
      makename(icnname, TargetDir, filename, SourceSuffix);

#if MVS || VM
/*
 * Even though the ucode data is all reasonable text characters, use
 *  of text I/O may cause problems if a line is larger than LRECL.
 *  This is likely to be true with any compiler, though the precise
 *  disaster which results may vary.
 */
      infile = fopen(inname, ReadBinary);
#else
      infile = fopen(inname, ReadText);
#endif					/* MVS || VM */

      if (infile == NULL)
         quitf("cannot open %s",inname);
      readglob();
      fclose(infile);
      }

   /* Phase II (optional): scan code and suppress unreferenced procs. */
   if (!strinv)
      scanrefs();

   /* Phase III: resolve undeclared variables and generate code. */

   /*
    * Open the output file.
    */

#if MVS
   routname = outname;
   outfile = tmpfile();         /* write icode to temporary file to
                                   avoid fseek-PDS limitations */
#else					/* MVS */
   outfile = fopen(outname, WriteBinary);
#endif					/* MVS */

/*
 * The following code is operating-system dependent [@link.02].  Set
 *  untranslated mode if necessary.
 */

#if PORT
   /* probably nothing */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ARM || ATARI_ST || MACINTOSH || MVS || UNIX || VM || VMS
   /* nothing to do */
#endif					/* AMIGA || ARM || ATARI_ST || ... */

#if MSDOS

#if MICROSOFT || TURBO
   setmode(fileno(outfile),O_BINARY);	/* set for untranslated mode */
#endif					/* MICROSOFT || TURBO */
#endif					/* MSDOS */

#if OS2
#if MICROSOFT || CSET2
   setmode(fileno(outfile),O_BINARY);
#endif					/* MICROSOFT || CSET2 */
#endif					/* OS2 */

/*
 * End of operating-system specific code.
 */
 
   if (outfile == NULL) 
      quitf("cannot create %s",outname);

#if MSDOS

   /*
    * This prepends ixhdr.exe to outfile, so it'll be executable.
    *
    * I don't know what that #if Header stuff was about since my MSDOS
    * distribution didn't include "hdr.h", but it looks very similar to
    * what I'm doing, so I'll put my stuff here, & if somebody who
    * understands all the multi-operating-system porting thinks my code could
    * be folded into it, having it here should make it easy. -- Will Mengarini.
    */

   if (makeExe) {
      FILE *fIconDOS = fopen(pathToIconDOS, "rb");
      char bytesThatBeginEveryExe[2] = {0,0}, oneChar;
      unsigned short originalExeBytesMod512, originalExePages;
      unsigned long originalExeBytes, byteCounter;

      if (!fIconDOS)
         quit("unable to find ixhdr.exe in same dir as icont");
      if (setvbuf(fIconDOS, 0, _IOFBF, 4096))
         if (setvbuf(fIconDOS, 0, _IOFBF, 128))
            quit("setvbuf() failure");
      fread (&bytesThatBeginEveryExe, 2, 1, fIconDOS);
      if (bytesThatBeginEveryExe[0] != 'M' ||
          bytesThatBeginEveryExe[1] != 'Z')
         quit("ixhdr header is corrupt");
      fread (&originalExeBytesMod512, sizeof originalExeBytesMod512,
            1, fIconDOS);
      fread (&originalExePages,       sizeof originalExePages,
            1, fIconDOS);
      originalExeBytes = (originalExePages - 1)*512 + originalExeBytesMod512;
      if (ferror(fIconDOS) || feof(fIconDOS) || !originalExeBytes)
         quit("ixhdr header is corrupt");
      fseek (fIconDOS, 0, 0);
      for (byteCounter = 0; byteCounter < originalExeBytes; byteCounter++) {
         oneChar = fgetc (fIconDOS);
         if (ferror(fIconDOS) || feof(fIconDOS) || ferror(outfile))
            quit("Error copying ixhdr.exe");
         fputc (oneChar, outfile);
         }
      fclose (fIconDOS);
      fileOffsetOfStuffThatGoesInICX = ftell (outfile);
      }

#endif                                  /* MSDOS */

#ifdef Header
   /*
    * Write the bootstrap header to the output file.
    */

#ifdef ShellHeader
   /*
    * Write a short shell header terminated by \n\f\n\0.
    * Use magic "#!/bin/sh" to ensure that $0 is set when run via $PATH.
    * Pad header to a multiple of 8 characters.
    */
   {
   char cmd[MaxPath + 100], script[2 * MaxPath + 200];

   sprintf(cmd, "exec \"${ICONX-%s}\" \"$0\"", iconxloc);
   sprintf(script, "%s   0) %s;;\n   *) %s \"$@\";;%s\n",
      "#!/bin/sh\n\ncase $# in\n", cmd, cmd,
      "\nesac\n\n[executable Icon binary follows]");
   strcat(script, "        \n\f\n" + ((int)(strlen(script) + 4) % 8));
   hdrsize = strlen(script) + 1;	/* length includes \0 at end */
   fwrite(script, hdrsize, 1, outfile);	/* write header */
   }
#else					/* ShellHeader */
   /*
    *  Always write MaxHeader bytes.
    */
   fwrite(iconxhdr, sizeof(char), MaxHeader, outfile);
   hdrsize = MaxHeader;
#endif					/* ShellHeader */
#endif					/* Header */

   for (i = sizeof(struct header); i--;)
      putc(0, outfile);
   fflush(outfile);
   if (ferror(outfile) != 0)
      quit("unable to write to icode file");

#ifdef DeBugLinker
   /*
    * Open the .ux file if debugging is on.
    */
   if (Dflag) {
      makename(dbgname, TargetDir, llfiles->lf_name, ".ux");
      dbgfile = fopen(dbgname, WriteText);
      if (dbgfile == NULL)
         quitf("cannot create %s", dbgname);
      }
#endif					/* DeBugLinker */

   /*
    * Loop through input files and generate code for each.
    */
   lfls = llfiles;
   while (lf = getlfile(&lfls)) {
      filename = lf->lf_name;
      makename(inname, SourceDir, filename, U1Suffix);
      makename(icnname, TargetDir, filename, SourceSuffix);

#if MVS || VM
      infile = fopen(inname, ReadBinary);
      if (infile != NULL)         /* discard the extra blank we had */
         (void) getc(infile);     /* to write to make it non-empty  */
#else                                   /* MVS || VM */
      infile = fopen(inname, ReadText);
#endif                                  /* MVS || VM */

      if (infile == NULL)
         quitf("cannot open %s", inname);
      gencode();


      fclose(infile);
      }


   gentables();		/* Generate record, field, global, global names,
			   static, and identifier tables. */
   fclose(outfile);
   lmfree();
   if (fatals > 0)
      return fatals;
   setexe(outname);
   return 0;
   }

/*
 * lwarn - issue a linker warning message.
 */
novalue lwarn(s1, s2, s3)
char *s1, *s2, *s3;
   {
   fprintf(stderr, "%s: ", icnname);
   if (lineno)
      fprintf(stderr, "Line %d # :", lineno);
   fprintf(stderr, "\"%s\": %s%s\n", s1, s2, s3);
   fflush(stderr);
   }

/*
 * lfatal - issue a fatal linker error message.
 */

novalue lfatal(s1, s2)
char *s1, *s2;
   {

   fprintf(stderr, "%s: ", icnname);
   if (lineno)
      fprintf(stderr, "Line %d # : ", lineno);
   fprintf(stderr, "\"%s\": %s\n", s1, s2);
   fatals++;
   }

/*
 * setexe - mark the output file as executable
 */

static novalue setexe(fname)
char *fname;
   {

/*
 * The following code is operating-system dependent [@link.03].  It changes the
 *  mode of executable file so that it can be executed directly.
 */

#if PORT
   /* something is needed */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA
   /* not necessary */
#endif					/* AMIGA */

#if ARM
   {
      _kernel_swi_regs regs;

      regs.r[0] = 31;
      regs.r[1] = (int)"Icon";
      if (_kernel_swi(OS_FSControl,&regs,&regs) == NULL)
      {
         regs.r[0] = 18;
         regs.r[1] = (int)fname;
         _kernel_swi(OS_File,&regs,&regs);
      }
   }
#endif					/* ARM */

#if ATARI_ST || MSDOS || MVS || VM || VMS 
   /*
    * can't be made executable
    * note: VMS files can't be made executable, but see "iexe.com" under VMS.
    */
#endif					/* ATARI_ST || MSDOS || ... */

#if MACINTOSH
#if MPW
   /* Nothing to do here -- file is set to type TEXT
      (so it can be executed as a script) in tmain.c.
   */
/* #pragma unused(fname) */
#endif					/* MPW */
#endif					/* MACINTOSH */

#if MSDOS
#if MICROSOFT || TURBO
   chmod(fname,0755);	/* probably could be smarter... */
#endif					/* MICROSOFT || TURBO */
#endif					/* MSDOS */

#if OS2
    /*
     *	Obtain the EXE stub resource from icont (or xicont)
     *	and write it out as the executable name.  Invoke the resource
     *	compiler to add the icode file as a resource to the executable
     *	This should be portable to Windows NT I believe.  Cheyenne.
     */
    {
	char	*exeres;		/* EXE stub resource pointer */
	unsigned long exereslen;	/* Length of resource */
	char loadmoderr[256];

	char exename[256];
	char rcname[256];
	char cmdbuffer[256];
	FILE *exefile;
	FILE *rcfile;

	if( noexe ) return;		/* Nothing to do.. */

	DosGetResource(0,0x4844,1,&exeres);
	DosQueryResourceSize(0,0x4844,1,&exereslen);

	makename(exename,NULL,fname,".exe");
	exefile = fopen(exename,WriteBinary);
	fwrite( exeres, sizeof(char), exereslen, exefile);
	fclose(exefile);
	DosFreeResource(exeres);

	makename(rcname,NULL,fname,".rc");
	rcfile = fopen(rcname,WriteText);

	fprintf(rcfile,"RESOURCE 0x4843 1 %s\n",fname);
	fclose(rcfile);

	sprintf(cmdbuffer,"rc %s %s",rcname,exename);

	system(cmdbuffer);

	unlink(rcname);
	makename(rcname,NULL,fname,".res");
	unlink(rcname);
	unlink(fname);
    }
#endif					/* OS2 */

#if UNIX
      {
      struct stat stbuf;
      int u, r, m;
      /*
       * Set each of the three execute bits (owner,group,other) if allowed by
       *  the current umask and if the corresponding read bit is set; do not
       *  clear any bits already set.
       */
      umask(u = umask(0));		/* get and restore umask */
      if (stat(fname,&stbuf) == 0)  {	/* must first read existing mode */
         r = (stbuf.st_mode & 0444) >> 2;	/* get & position read bits */
         m = stbuf.st_mode | (r & ~u);		/* set execute bits */
         chmod(fname,m);		 /* change file mode */
         }
      }
#endif					/* UNIX */

/*
 * End of operating-system specific code.
 */
   }
