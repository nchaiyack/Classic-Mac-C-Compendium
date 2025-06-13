/*
 * trans.c - main control of the translation process.
 */

#include "::h:gsupport.h"
#include "tproto.h"
#include "::h:version.h"
#include "tglobals.h"
#include "tsym.h"
#include "tree.h"
#include "ttoken.h"

/*
 * Prototypes.
 */

hidden	novalue	trans1		Params((char *filename));

int tfatals;			/* number of fatal errors in file */
int afatals;			/* total number of fatal errors */
int nocode;			/* non-zero to suppress code generation */
int in_line;			/* current input line number */
int incol;			/* current input column number */
int peekc;			/* one-character look ahead */

/*
 * translate a number of files, returning an error count
 */
int trans(ifiles)
char **ifiles;
   {
   tmalloc();			/* allocate memory for translation */

   afatals = 0;

#ifdef MultipleRuns
   yylexinit();			/* initialize lexical analyser */
   tcodeinit();			/* initialize code generator */
#endif					/* Multiple Runs */

   while (*ifiles) {
      trans1(*ifiles++);	/* translate each file in turn */
      afatals += tfatals;
      }
   tmfree();			/* free memory used for translation */

   /*
    * Report information about errors and warnings and be correct about it.
    */
   if (afatals == 1)
      fprintf(stderr, "1 error\n");
   else if (afatals > 1)
      fprintf(stderr, "%d errors\n", afatals);
   else if (!silent)
      fprintf(stderr, "No errors\n");

   return afatals;
   }

/*
 * translate one file.
 */
static novalue trans1(filename)
char *filename;
{
   char oname1[MaxFileName];	/* buffer for constructing file name */
   char oname2[MaxFileName];	/* buffer for constructing file name */

   tfatals = 0;			/* reset error counts */
   nocode = 0;			/* allow code generation */
   in_line = 1;			/* start with line 1, column 0 */
   incol = 0;
   peekc = 0;			/* clear character lookahead */

   if (!ppinit(filename,m4pre))
      quitf("cannot open %s",filename);
   ppdef("_INTERPRETED", "1");

   if (strcmp(filename,"-") == 0)
      filename = "stdin";
   if (!silent)
      fprintf(stderr, "%s:\n",filename);

   if (pponly) {
      ppecho();
      return;
      }

#ifndef VarTran
   /*
    * Form names for the .u1 and .u2 files and open them.
    *  Write the ucode version number to the .u2 file.
    */

   makename(oname1, TargetDir, filename, U1Suffix);

#if MVS || VM
/*
 * Even though the ucode data is all reasonable text characters, use
 *  of text I/O may cause problems if a line is larger than LRECL.
 *  This is likely to be true with any compiler, though the precise
 *  disaster which results may vary.
 *
 * On CMS (and sometimes on MVS), empty files are not readable later.
 *  Since the .U1 file may be empty, we start it off with an extra
 *  blank (thrown away on input) to make sure there's something there.
 */
   codefile = fopen(oname1, WriteBinary);   /* avoid line splits */
   if (codefile != NULL)
      putc(' ', codefile);
#else					/* MVS || VM */
   codefile = fopen(oname1, WriteText);
#endif					/* MVS || VM */

   if (codefile == NULL)
      quitf("cannot create %s", oname1);

   makename(oname2, TargetDir, filename, U2Suffix);

#if MVS || VM
   globfile = fopen(oname2, WriteBinary);
#else					/* MVS || VM */
   globfile = fopen(oname2, WriteText);
#endif					/* MVS || VM */

   if (globfile == NULL)
      quitf("cannot create %s", oname2);
   writecheck(fprintf(globfile,"version\t%s\n",UVersion));
#endif					/* VarTran */

   tok_loc.n_file = filename;
   in_line = 1;

   tminit();				/* Initialize data structures */
   yyparse();				/* Parse the input */

   /*
    * Close the output files.
    */

#ifndef VarTran
   if (fclose(codefile) != 0 || fclose(globfile) != 0)
      quit("cannot close ucode file");
#endif					/* VarTran */

   if (tfatals) {
      unlink(oname1);
      unlink(oname2);
      }
   }

/*
 * writecheck - check the return code from a stdio output operation
 */
novalue writecheck(rc)
int rc;
   {
   if (rc < 0)
      quit("cannot write to ucode file");
}
