/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
/* misc.c - miscellaneous programs */


#ifndef _STDLIB
#include <stdlib.h>
#endif
#ifndef _STDIO
#include <stdio.h>
#endif
#ifndef _STRING
#include <string.h>
#endif

#include "config.h"

#ifdef MAC_GWADA
#include "smalloc.h"
#include "MacMemory.h"
#include "_time.h"
#else
#include "time.h"
#endif

#include "ifile.h"
#include "misc.h"

#ifdef BSD
#include <strings.h>
#endif

#ifdef MAC_GWADA
#include "ShellInterface.h"
#include "ShellGlobals.h"
#include "FileMgr.h"
#include "FileNames.h"
#endif

#include "adafront.h"
#include "vars.h"

#ifndef LIBDIR
#ifdef MAC_GWADA
	/* � File: misc.c
	   � Problem:
	   � Default directory for library (system) is /usr/local/lib.  This
	   � doesn't make sense for the Mac.
	   � This shouldn't be here either.  This is the type of info that
	   � is required for configuration/portability.
	   �
	   � Solution:
	   � Set default library directory for the Mac to current folder
	   � to the name 'Predef Library'.  THIS IS NOT USED!
	   � */
#define LIBDIR ":Predef Library:"
#else
#define LIBDIR "/usr/local/lib"
#endif
#endif

#ifdef ADALIB
#define EXIT_INTERNAL_ERROR
#endif

#ifdef BINDER
#define EXIT_INTERNAL_ERROR
extern int adacomp_option;
#endif

#ifdef INT
#define EXIT_INTERNAL_ERROR
#endif

#ifndef EXPORT
#undef EXIT_INTERNAL_ERROR
#endif
#ifdef BSD
#include <sys/file.h>
#endif

char *LIBRARY_PREFIX= "";

/* PREDEFNAME gives directory path to predef files.
 * libset() is used to toggle between libraries (the users and predef).
 * tname = libset(lname) sets library prefix for ifopen, etc. to lname
 * and returns prior setting in tname.
 */

static void openerr(char *filename, const char *mode);

#ifdef SMALLOC
unsigned int smalloc_free = 0;
char	*smalloc_ptr;
#define SMALLOC_BLOCK 2000
char **smalloc_table = (char **)0;
unsigned smalloc_blocks = 0;
#endif

char *smalloc(unsigned n)										/*;smalloc*/
{
	/* variant of malloc for use for blocks that will never be freed,
	 * primarily blocks used for small strings. This permits allocation
	 * in larger blocks avoiding the malloc overhead required for each block.
	 */
#ifndef SMALLOC
	return emalloct(n, "smalloc");
#else
	char *p;
	if (n & 1) n+= 1;
#ifdef ALIGN4
	if (n & 2) n+= 2;
#endif

	if (n > SMALLOC_BLOCK) { /* large block allocated separately */
#ifdef DEBUG
		printf("smalloc: warning block %u exceeds %d SMALLOC_BLOCK\n",
		  n, SMALLOC_BLOCK);
#endif
		p = emalloct(n, "smalloc");
		return p;
	}
	if (n > smalloc_free) {
		smalloc_ptr = emalloct(SMALLOC_BLOCK, "smalloc-block");
		smalloc_free = SMALLOC_BLOCK;
		smalloc_blocks++;
		if (smalloc_blocks == 1) {
			smalloc_table = (char **) emalloct(sizeof (char **),
			  "smalloc-table");
		}
		else { /* reallocate blocks */
			smalloc_table = (char **) erealloct((char *)smalloc_table,
			  sizeof(char **) * (smalloc_blocks), "smalloc-table-realloc");
		}
		smalloc_table[smalloc_blocks-1] = smalloc_ptr;
	}
	p = smalloc_ptr;
	smalloc_ptr += n;
	smalloc_free -= n;
	return p;
#endif
}

#ifdef SMALLOC
#ifdef DEBUG
void smalloc_list()
{
	int i;
	char **st;
	st = smalloc_table;
	for (i = 0; i < smalloc_blocks; i++) {
		printf("%d %ld %x\n", i, *st, *st);
		st++;
	}
}
#endif
#endif

int is_smalloc_block(char *p)							/*;is_smalloc_block*/
{
	/* returns TRUE is p points within block allocated by smalloc */
#ifdef SMALLOC
	int i;
	char **st;

	st = smalloc_table;
	if (smalloc_blocks == 0) chaos(__FILE__, __LINE__, "is_malloc_block - no blocks");
	for (i = 0; i < smalloc_blocks; i++) {
		if (*st <= p && p  < (*st+(SMALLOC_BLOCK-1)))
			return TRUE;
		st++;
	}
	return FALSE;
#else
	return FALSE;
#endif
}

#ifdef MAC_GWADA
void close_mac_files(void);

void close_mac_files(void)
{
	Str255 tmpStr, fileNoExt;


	strcpy((char *)&tmpStr, FILENAME);
	CtoPstr((char *)&tmpStr);
	ParseFile(tmpStr, flPath+flName, fileNoExt);
	PtoCstr(fileNoExt);

	if (MSGFILE) {
		Boolean empty = ftell(MSGFILE) == 0;

//		fflush(errfile);	// why errfile? shouldn't it be MSGFILE?
							// in that case, this line is not necessary [Fabrizio Oddone]
		fclose(MSGFILE);

		if (empty) {
			char       *fname;

			fname = ifname((char *)&fileNoExt, "msg");
			remove(fname);
			efree(fname);
		}	

		MSGFILE = NULL;
	}

	if (errfile) {
		Boolean empty = ftell(errfile) == 0;

//		fflush(errfile);	// not necessary [Fabrizio Oddone]
		fclose(errfile);

		if (empty) {
			char       *fname;

			fname = ifname((char *)&fileNoExt, "err");
			remove(fname);
			efree(fname);
		}	

		errfile = NULL;
	}
}

#endif

void capacity(char *file, int line, char *s)				/*;capacity*/
{
	/* called  when compiler capacity limit exceeded.
	 * EXIT_INTERNAL_ERROR is defined when the module is run by itself
	 * (not spawned from adacomp) and DEBUG is not defined.
	 */
#ifdef MAC_GWADA
	ErrMsgs errors;
	
	errors[0].id = 3;
	errors[1].id = errors[2].id = errors[3].id = -1;
	errors[1].msg = s;
	errors[0].msg = errors[2].msg = errors[3].msg = NULL;

	InternalError(errors, RC_INTERNAL_ERROR, file, line);
#endif
	exitp(RC_INTERNAL_ERROR);
}

#ifdef CHAOS
void chaos(char *file, int line, char *s)								/*;chaos*/
{
	/* called when internal logic error detected and it is not meaningful
	 * to continue execution. This is never defined for the export version.
	 */
	
#ifdef MAC_GWADA
	ErrMsgs errors;
	
	errors[0].id = 4;
	errors[1].id = -1;
	errors[2].id = 5;
	errors[3].id = -1;
	errors[0].msg = NULL;
	errors[1].msg = s;
	errors[2].msg = errors[3].msg = NULL;

	InternalError(errors, RC_INTERNAL_ERROR, file, line);

#else
	exitp(RC_INTERNAL_ERROR);
#endif
}
#else
void exit_internal_error()						/*;exit_internal_error*/
{
	/* called when internal logic error detected and it is not meaningful
	 * to continue execution. This procedure is called by the export version.
	 * EXIT_INTERNAL_ERROR is defined when the module is run by itself
	 * (not spawned from adacomp) and EXPORT is defined.
	 * Now that adabind is a separate module which can be called by itself
	 * or spawned from adacomp, we must test the run time flag adacomp_option
	 * to determine which case it is.
	 */

#ifdef MAC_GWADA
	ErrMsgs errors;
	
	errors[0].id = 6;
	errors[1].id = errors[2].id = errors[3].id = -1;
	errors[0].msg =
		errors[1].msg = errors[2].msg = errors[3].msg = NULL;
//Debugger();
	InternalError(errors, RC_INTERNAL_ERROR, __FILE__, __LINE__);
#else
	exitp(RC_INTERNAL_ERROR);
#endif
}
#endif

void exitp(int n)												/*;exitp*/
{
#ifdef MAC_GWADA

	close_mac_files();

	// jump back to the main program passing whatever return
	// code we have here!
	longjmp(gJumpEnv, n+10);		// add 10, see note in ShellMain.c


#else

	/* synonym for exit() used so can trap exit() calls with debugger */
	exit(n);

#endif
}


char *ecalloc(unsigned nelem, unsigned nsize)			/*;ecalloc */
{
	/* calloc with error check if no more */

	char   *p;

	if (nelem > 60000) {
//		DebugStr("\pecalloc error");
		chaos(__FILE__, __LINE__, "ecalloc: ridiculous argument");
		}

//	p = calloc (nelem, nsize);
	p = mcalloc (nelem, nsize);
	if (p == nil)
		capacity(__FILE__, __LINE__, "out of memory");
	return p;
}

char *emalloc(unsigned n)										/*;emalloc */
{	/* avoid BUGS - use calloc which presets result to zero  ds 3 dec 84*/
	/* malloc with error check if no more */

	char   *p;

	if (n > 150000) {
//		DebugStr("\pemalloc error");
		chaos(__FILE__, __LINE__, "emalloc: ridiculous argument");
		}
//	p = calloc (1, n);
	p = ffcalloc(n);
	if (p == nil)
		capacity(__FILE__, __LINE__, "out of memory");
	return p;
}

char *erealloc(char *ptr, unsigned size)						/*;eralloc */
{
	/* realloc with error check if no more */

	char   *p;

//	p = realloc (ptr, size);
	p = mrealloc (ptr, size);
	if (p == NULL)
		capacity(__FILE__, __LINE__, "erealloc: out of memory");

	return (p);
}

char *strjoin(char *s1, char *s2)								/*;strjoin */
{
	/* return string obtained by concatenating argument strings
	 * watch for either argument being (char *)0 and treat this as null string
	 */

	char   *s;

	if (s1 == (char *)0) s1= "";
	if (s2 == (char *)0) s2 = "";
	s = smalloc((unsigned) strlen(s1) + strlen(s2) + 1);
	strcpy(s, s1);
	strcat(s, s2);
	return s;
}

int streq(char *a, char *b)											/*;streq*/
{
	/* test two strings for equality, allowing for null pointers */
	if (a == (char *)0 && b == (char *)0)
		return TRUE;
	else if (a == (char *)0 || b == (char *)0)
		return FALSE;
	else return (strcmp(a, b) == 0);
}

char *substr(char *s, int i, int j)								/*;substr */
{
	/* return substring s(i..j) if defined, else return null ptr*/

	int	n;
	char	*ts, *t;

	if (s == (char *)0) return (char *) 0;
	n = strlen(s);
	if (!(i > 0 && j <= n && i <= j)) return (char *)0;
	/* allocate result, including null byte at end */
	ts = smalloc((unsigned) j - i + 2);
	t = ts;
	s = s + (i - 1); /* point to start of source*/
	for (; i <= j; i++) *t++ = *s++; /* copy characters */
	*t = '\0'; /* terminate result */
	return ts;
}

/* getopt(3) procedure obtained from usenet */
/*
 * getopt - get option letter from argv
 */
#ifdef IBM_PC
#define nogetopt
#endif

#ifdef nogetopt
char   *optarg;				/* Global argument pointer. */
int	optind = 0;				/* Global argv index. */

static char *scan = NULL;	/* Private scan pointer. */

int getopt(int argc, char **argv, char *optstring)				/*;getopt */
{
	register char   c;
	register char  *place;
	optarg = NULL;

	if (scan == NULL || *scan == '\0') {
		if (optind == 0)
			optind++;

		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return (EOF);
		if (strcmp (argv[optind], "--") == 0) {
			optind++;
			return (EOF);
		}

		scan = argv[optind] + 1;
		optind++;
	}

	c = *scan++;
	place = strchr (optstring, c);

	if (place == NULL || c == ':') {
		fprintf(stdout, "%s: unknown option -%c\n", argv[0], c);
		return ('?');
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		}
		else {
			optarg = argv[optind];
			optind++;
		}
	}
	return (c);
}
#endif

char *greentime(int un)										/*;greentime*/
{
	/* get greenwich time in string of 23 characters.
	 * format of result is as follows
	 *	1984 10 02 16 30 36 nnn
	 *	123456789a123456789b123
	 *	year mo da hr mi se uni
	 *
	 * greenwich time is used to avoid problems with daylight savings time.
	 * The last three characters are the compilation unit number
	 * (left filled with zeros if necessary).
	 * NOTE: changed to use local time to give approx. same time as
	 * SETL version			ds  20 nov 84
	 */

	char	*s;
#ifndef IBM_PC
	long clock;
#else
	/* IBM_PC (Metaware) */
	time_t clock;
#endif
	/*struct tm *gmtime();*/
	struct tm *t;
#ifndef IBM_PC
	clock = time(0);
#else
	time(&clock);
#endif
	s = smalloc(24);
	/*t = gmtime(&clock);*/
	#ifdef MAC_GWADA
		t = localtime((time_t *)&clock);
	#else
		t = localtime(&clock);
	#endif
	sprintf(s,"%04d %02d %02d %02d %02d %02d %03d",
#ifdef IBM_PC
	  /* needed until Metaware fixes bug in tm_year field (ds 6-19-86) */
	  t->tm_year , t->tm_mon + 1, t->tm_mday,
#else
	  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
#endif
	  t->tm_hour, t->tm_min, t->tm_sec, un);
	return s;
}

FILE *efopenl(char *filename, char *suffix, char *type
#ifdef IBM_PC
, char *mode
#endif
)	/*;efopenl*/
{
	char       *fname;
	FILE       *f;

	fname = ifname(filename, suffix);
	f =  efopen(fname, type
#ifdef IBM_PC
	, mode
#endif
	);
	efree(fname);
	return f;
}

FILE *efopen(char *filename, char *type
#ifdef IBM_PC
, char *mode
#endif
)				/*;efopen*/
{
	FILE	*f;
#ifdef IBM_PC
	char    *p;
	/* mode only meaningful for IBM PC for now */

	p = emalloc((unsigned) (strlen(type) + strlen(mode) + 1));
	strcpy(p, type);
	strcat(p, mode);
	f = fopen(filename, p);
	efree(p);
#else
	f = fopen(filename, type);
#endif
	if (f == NULL)
		openerr(filename, type);
//	else
//		if (setvbuf(f, NULL, _IOFBF, 16*1024L))
#ifdef MAC_GWADA
//			FlashMenuBar(0)
#endif
//			;
	return f;
}

void efree(char *p)												/*;efree*/
{
	/* free with check that not trying to free null pointer*/
	if (p == (char *)0)
		chaos(__FILE__, __LINE__, "efree: trying to free null pointer");
//	free(p);
	mfree(p);
}

int strhash(char *s)										/*;strhash*/
{
	/* Hashing function from strings to numbers */

	register int hash = 0;
	char	t;	// does not access memory twice [Fabrizio Oddone]

	/* add character values together, adding in the cumulative hash code
	 * at each step so that 'ABC' and 'BCA' have different hash codes.
	 */
	while (t = *s++)
		hash += hash + t;
	if (hash < 0) hash = - hash; /* to avoid negative hash code */
	return hash;
}

char *unit_name_type(char *u)							/*;unit_name_type*/
{
	int	n;
	char	*s;

	n = strlen(u);
	if (n < 2) {
		s = smalloc(1); 
		*s = '\0'; 
		return s;
	}
	/* otherwise, return first two characters */
	s = smalloc(3);
	s[0] = u[0];
	s[1] = u[1];
	s[2] = '\0';
	return s;
}

#ifdef BSD
/* BSD doesn't support strchr() and strrchr(), but they are just
 * named index() and rindex(), respectively, so here is code for BSD
 */
char *strchr(const char *s, int c)
{
	return index(s, (char) c);
}

char *strrchr(const char *s, int c)
{
	return rindex(s, (char) c);
}
#endif

char *libset(char *lname)										/*;libset*/
{
	char *old_name;

	old_name = LIBRARY_PREFIX;
	LIBRARY_PREFIX = lname;
	return old_name;
}

char *ifname(char *filename, char *suffix)						/*;ifname*/
{
	char *fname;

	/* allow room for library prefix, file name and suffix */
	fname = emalloc((unsigned) (strlen(LIBRARY_PREFIX) + strlen(filename) +
	  strlen(suffix) + 3));
	if (strlen(LIBRARY_PREFIX)) { /* prepend library prefix if present */
		short len1, len2;
		strcpy(fname, LIBRARY_PREFIX);

		// MAC_GWADA - this is just a simple good idea.  Add DIR_DELIMITER
		// only if it is not there already
		len1 = strlen(LIBRARY_PREFIX);		// we know len > 0 (see if above)
		len2 = strlen(DIR_DELIMITER);
		if (strcmp(&fname[len1-len2], DIR_DELIMITER) != 0)
			strcat(fname, DIR_DELIMITER);

		strcat(fname, filename);
	}
	else {
		strcpy(fname, filename); /* copy name if no prefix */
	}
	if (strlen(suffix)) {
		strcat(fname, ".");
		strcat(fname, suffix);
	}
	return fname;
}

fabIFILE *fab_ifopenread(char *filename, char *suffix, int pass)
{
fabIFILE  *ifile;
fabFILE  *file;
char  *fname;
Boolean	error = FALSE;

fname = ifname(filename, suffix); /* expand file name */

file = fab_fopen(fname, fsRdPerm);
if (file == NULL) {
	if (pass)
		return NULL;
	else
		openerr(fname, "r");
	}
ifile = (fabIFILE *) emalloc(sizeof(fabIFILE));

if (sizeof(fabIFILE) != fab_fread((char *) ifile, sizeof(fabIFILE), file)) {
#ifdef DEBUG
	printf("ifopen - unable to read header\n");
#endif
	error = TRUE;
	}
if (error)
	openerr(fname, "r");

ifile->fh_file = file;
ifile->fh_mode = 'r';

efree(fname);
return ifile;
}

IFILE *ifopen(char *filename, char *suffix, const char *mode, int pass)	/*;ifopen*/
{
	FILE  *file;
	char  modec;
	char  *fname;
	long  s = 0L;
	int   nr, error = FALSE;
	IFILE  *ifile;
#ifdef IBM_PC
	char *t_name;
#endif

	modec= mode[0];

	fname = ifname(filename, suffix); /* expand file name */

#ifdef IBM_PC
	/* mode only meaningful for IBM PC for now */
	t_name = emalloc((unsigned) (strlen(mode) + 2));
	strcpy(t_name, mode);
	strcat(t_name, "b");
	file = fopen(fname, t_name);
	efree(t_name);
#else
	file = fopen(fname, mode);
#endif

	if (file == (FILE *)0) {
		if (pass)
			return (IFILE *) 0;
		else
			openerr(fname, mode);
		}
//	else
//		if (setvbuf(file, NULL, _IOFBF, 16*1024L))
#ifdef MAC_GWADA
//			FlashMenuBar(0)
#endif
//			;
	ifile = (IFILE *) emalloc(sizeof(IFILE));
	if (modec == 'w') { /* write header */
		/* write long at start to be replaced later with slots offset */
		ifile->fh_mode = modec;
		ifile->fh_slots = 0;
		ifile->fh_units_end = 0;
		/* will be updated on close */
		fwrite((char *) ifile, sizeof(IFILE), 1, file);
	}
	else if (modec == 'r') { /* read and check header */
		nr = fread((char *) ifile, sizeof(IFILE), 1, file);

		if (nr != 1) {
#ifdef DEBUG
			printf("ifopen - unable to read header\n");
#endif
			error = TRUE;
		}
	}
	if (error) {
		openerr(fname, mode);
	}
	ifile->fh_file = file;
	ifile->fh_mode = modec;

	efree(fname);
	return ifile;
}

static void openerr(char *filename, const char *mode)					/*;openerr*/
{
	/* EXIT_INTERNAL_ERROR is defined when the module is run by itself
	 * (not spawned from adacomp) and DEBUG is not defined.
	 */
#ifdef MAC_GWADA
	ErrMsgs errors;
	
	errors[0].id = 7;
	errors[1].id = -1;
	errors[2].id = 8;
	errors[3].id = -1;
	errors[0].msg = NULL;
	errors[1].msg = filename;
	errors[2].msg = NULL;
	errors[3].msg = mode;

	InternalError(errors, RC_ABORT, __FILE__, __LINE__);

#else

#ifdef EXIT_INTERNAL_ERROR
	fprintf(stdout, "Unable to open file %s for %s \n", filename,
	  (strcmp(mode, "w") == 0 ? "writing"
	  : (strcmp(mode, "r") == 0 ? "reading"
	  : (strcmp(mode, "a") == 0 ? "appending"
	  :  mode))));
	exit(RC_ABORT);
#else
	fprintf(stdout, "Unable to open file %s for %s \n", filename,
	  (strcmp(mode, "w") == 0 ? "writing"
	  : (strcmp(mode, "r") == 0 ? "reading"
	  : (strcmp(mode, "a") == 0 ? "appending"
	  :  mode))));
	exit(RC_ABORT);
#endif
#endif
}

void fab_ifclose(fabIFILE *ifile)
{
fabFILE *file;

if (ifile) {
	file = ifile->fh_file;
//		chaos(__FILE__, __LINE__, "ifclose: closing unopened file");
	if (file)
		fab_fclose(file);
	ifile->fh_file = NULL;
	}
}

void ifclose(IFILE *ifile)									/*;ifclose*/
{
	FILE *file;


	file = ifile->fh_file;
	/* write out file header if write mode */
	if (ifile->fh_mode == 'w') {
		ifile->fh_mode = '\0';
		ifseek(ifile, "update-header", 0L, 0);
		fwrite((char *)ifile, sizeof(IFILE), 1, file);
	}
	if (file == NULL)
		chaos(__FILE__, __LINE__, "ifclose: closing unopened file");
	fclose(file);
	ifile->fh_file = NULL;
}

void ifoclose(IFILE *ifile)									/*;ifoclose*/
{
	/* close file if still open */
	if (ifile && ifile->fh_file) {
		ifclose(ifile);
	}
}

void fab_ifseek(fabIFILE *ifile, Size offset)
{
fab_fseek(ifile->fh_file, offset);
}

long ifseek(IFILE *ifile, char *desc, long offset, int ptr)		/*;ifseek*/
{
long begpos, endpos, seekval;

	begpos = iftell(ifile);
	seekval = fseek(ifile->fh_file, offset, ptr);
	if (seekval == -1) chaos(__FILE__, __LINE__, "ifseek: improper seek");

	endpos = iftell(ifile);
return endpos;
}

Size fab_iftell(const fabIFILE *ifile)
{
return fab_ftell(ifile->fh_file);
}

long iftell(IFILE *ifile)									/*;iftell*/
{
/* ftell, but arg is IFILE */
return ftell(ifile->fh_file);
}

/* define MEAS_ALLOC to measure alloc performance */
#define MEAS_ALLOC
/* this causes each malloc action to write a line to standard output
 * formatted as follows:
 * code:one of a, r, f
 * a	allocate block
 * r	reallocate block
 * f	free block
 * the block address (integer)
 * the block length (or zero if not applicable)
 * the remainder of the line describes the action
 */

#ifndef EXPORT
char *emalloct(unsigned n, char *s)								/*;emalloct*/
{
return emalloc(n);
}
#endif

#ifndef EXPORT
char *malloct(unsigned n, char *s)		/*;malloct*/
{
/* like emalloct, but ok if not able to allocate block */
//	p = malloc(n);
return mmalloc(n);
}
#endif

#ifndef EXPORT
char *ecalloct(unsigned n, unsigned m, char *msg)
{
return ecalloc(n, m);
}
#endif

#ifndef EXPORT
char *fcalloct(size_t n, char *msg)
{
return ffcalloc(n);
}
#endif

#ifndef EXPORT
char *erealloct(char *ptr, unsigned size, char *msg)		/*;erealloct*/
{
return erealloc(ptr, size);
}
#endif

#ifndef EXPORT
void efreet(char *p, char *msg)									/*;efreet*/
{
efree(p);
}
#endif

#ifdef MAC_GWADA
static Str255	compilerDir;

// This is a Mac version of this routine.  It works in
// conjunction with the routine above set_predef
char *predef_env()			/*;predef_env*/
{
	Str255 str;

	GetCompilerPath(str);
	ParseFile(str, flPath, compilerDir);
	PtoCstr(compilerDir);
	return (char *)&compilerDir;

}
#endif

char *get_libdir()
{

#ifdef MAC_GWADA

	return predef_env();

#else

    char *s = getenv("ADAED");
    if (s == (char *)0) {
#ifndef IBM_PC
        return LIBDIR;
#else
        fprintf(stdout, "ADAED variable must be defined\n");
        exitp(RC_ABORT);
#endif
    }
    else
        return s;
#endif
}

char *parsefile(char *s, int *np, int *nb, int *ns)				/*;parsefile*/
{
	/* Parse file name s, returning the length of prefix, base part, and
	 * suffix in np, nb, and nl, respectively. A pointer to the start of
	 * the base part is returned, or the null pointer if no base part.
	 * The suffix is assumed to begin with period.
	 * The prefix ends with the last instance of any of the prefix characters.
	 */

#ifdef IBM_PC
    char   *prefix_chars = ":/\\";
#endif
#ifdef BSD
    char   *prefix_chars = "/";
#endif
#ifdef SYSTEM_V
    char   *prefix_chars = "/";
#endif
#ifdef MAC_GWADA
	char   *prefix_chars = DIR_DELIMITER;
#endif

    int    n,i;
    char   *pb;
    char   *p, *p2;
    char   *suffix_chars = ".";
    int    have_prefix = 0;

    n = strlen(s);
    pb = s; /* assume name starts with base */
    *ns = 0;
    p = s + n; /* point to last (null) character in s */
    /* find length of suffix */
    /* but if find a prefix character first, then no suffix possible */
    for (i = n - 1; i >= 0; i--) {
		p--; 
		for (p2 = prefix_chars; *p2 !='\0';) {
	    	if (*p == *p2++) {
		 		/* (p-s) gives number of characters before suffix */
		 		have_prefix = 1;
		 		break;
	    	}
		}
		if (!have_prefix) {
	    	for (p2 = suffix_chars; *p2 !='\0';) {
				if (*p == *p2++) {
		     		/* (p-s) gives number of characters before suffix */
		     		*ns = n - (p - s);
		     		break;
				}
	    	}
		}
    }
    /* find length of prefix */
    *np = 0;
    p = s + n;
    for (i = n - 1; i >= 0; i--) {
		p--; 
		for (p2 = prefix_chars; *p2 !='\0';) {
	    	if (*p == *p2++) {
		 		p++; /* include last delimiter in prefix */
		 		/* (p-s) now gives prefix length*/
		 		*np = (p - s);
		 		pb = p;
		 		break;
	    	}
		}
    }
    /* base is what remains after removing prefix and suffix*/
    *nb = n - (*np + *ns);
    if (*nb == 0)
		pb = (char *)0; /* if no base */
    return pb;
}
