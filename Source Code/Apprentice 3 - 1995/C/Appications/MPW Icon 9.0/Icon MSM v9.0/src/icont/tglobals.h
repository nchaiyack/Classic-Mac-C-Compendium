/*
 *  Global variables.
 */

#ifndef Global
#define Global extern
#define Init(v)
#endif					/* Global */

/*
 * Masks for accessing hash tables.
 */
Global int cmask;			/* mask for constant table hash */
Global int fmask;			/* mask for field table hash */
Global int gmask;			/* mask for global table hash */	
Global int imask;			/* mask for identifier table hash */
Global int lmask;			/* mask for local table hash */

/*
 * Array sizes for various linker tables that can be expanded with realloc().
 */
Global unsigned int csize	Init(100);	/* constant table */
Global unsigned int lsize	Init(100);	/* local table */
Global unsigned int nsize	Init(1000);	/* ipc/line num. assoc. table */
Global unsigned int stsize	Init(20000);	/* string space */
Global unsigned int maxcode	Init(15000);	/* code space */
Global unsigned int fnmsize	Init(10);	/* ipc/file name assoc. table */
Global unsigned int maxlabels	Init(500);	/* maximum num of labels/proc */

/*
 * Sizes of various hash tables.
 */
Global unsigned int lchsize	Init(128);	/* constant hash table */
Global unsigned int fhsize	Init(32);	/* field hash table */
Global unsigned int ghsize	Init(128);	/* global hash table */
Global unsigned int ihsize	Init(128);	/* identifier hash table */
Global unsigned int lhsize	Init(128);	/* local hash table */

/*
 * Variables related to command processing.
 */
Global char *progname	Init("icont");	/* program name for diagnostics */

#if ATARI_ST
Global char *patharg	Init(0);	/* -p: path for finding iconx */
#endif					/* ATARI_ST */

#if MSDOS
#if ZTC_386
Global int makeExe	Init(0);	/* -I: create .icx instead of .exe */
#else
Global int makeExe	Init(1);	/* -X: create .exe instead of .icx */
#endif					/* ZTC_386 */
Global long fileOffsetOfStuffThatGoesInICX
			Init(0);	/* remains 0 -f -X is not used */
#endif					/* MSDOS */

					/* set in link.c; used in lcode.c */
Global int silent	Init(0);	/* -s: suppress info messages? */
Global int m4pre	Init(0);	/* -m: use m4 preprocessor? [UNIX] */
Global int uwarn	Init(0);	/* -u: warn about undefined ids? */
Global int trace	Init(0);	/* -t: initial &trace value */
Global int pponly	Init(0);	/* -E: preprocess only */
Global int strinv	Init(0);	/* -f s: allow full string invocation */
Global int verbose	Init(1);	/* -v n: verbosity of commentary */


#if OS2
Global int noexe	Init(0);	/* -i: stop with .ICX file */
#endif

#ifdef DeBugLinker
Global int Dflag	Init(0);	/* -L: linker debug (write .ux file) */
#endif					/* DeBugLinker */

/*
 * Files and related globals.
 */
Global FILE *codefile	Init(0);	/* current ucode output file */
Global FILE *globfile	Init(0);	/* current global table output file */

Global char *iconxloc;			/* path to iconx */
Global long hdrsize;			/* size of iconx header */
