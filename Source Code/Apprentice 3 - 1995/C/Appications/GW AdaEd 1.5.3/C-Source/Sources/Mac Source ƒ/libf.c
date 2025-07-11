/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

/* libf - auxiliary procedures for reading in IFILE format files.
 * This is subset of procedures needed to read files generated in
 * library format without the need all the library primitives.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ifile.h"
#include "misc.h"
#include "libf.h"
#ifndef TRUE
#define TRUE 1
#endif

int fab_getint(fabIFILE *ifile)
{
/* read int  from input file */
int si;

if (sizeof(int) != fab_fread((char *) &si, sizeof(int), ifile->fh_file)) {
	chaos(__FILE__, __LINE__, "libr.c: read_ais - unable to read int value");
	}

return si;
}

int getint(IFILE *ifile, char *desc)							/*;getint*/
{
	/* read int  from input file */
	int si;
	int n = 0;
	int	nr;

	nr = fread((char *) &si, sizeof(int), 1, ifile->fh_file);
	if (nr != 1) {
		chaos(__FILE__, __LINE__, "libr.c: read_ais - unable to read int value");
	}
	n = si;
	return n;
}

short fab_getnum(fabIFILE *ifile)
{
short si;

fab_fread((char *)&si, sizeof(short), ifile->fh_file);
return si;
}

int getnum(IFILE *ifile, char *desc)								/*;getnum*/
{
/* read integer (only 2 bytes) from input file */

short si;
//	int n = 0;
//DebugStr("\pAha! Bastard getnum!");
fread((char *) &si, sizeof(short), 1, ifile->fh_file);
//	n = si;
return si;
}

int getchr(IFILE *ifile, char *desc)								/*;getchr*/
{
	/* This is variant of getnum used when reading character values */
	/* read integer (only 2 bytes) from input file */

	short si;
//	int n = 0;

	fread((char *) &si, sizeof(short), 1, ifile->fh_file);
//	n = si;
	return si;
}

long fab_getlong(fabIFILE *ifile)
{
/* read long  from input file */

long si;

if (sizeof(long) != fab_fread((char *) &si, sizeof(long), ifile->fh_file))
	chaos(__FILE__, __LINE__, "libr.c: read_ais - unable to read long value");
return si;
}

long getlong(IFILE *ifile, char *desc)							/*;getlong*/
{
	/* read long  from input file */

	long si;
	long n = 0;
	int	nr;

	nr = fread((char *) &si, sizeof(long), 1, ifile->fh_file);
	if (nr != 1) {
		chaos(__FILE__, __LINE__, "libr.c: read_ais - unable to read long value");
	}
	n = si;
	return n;
}

char *fab_getstr(fabIFILE *ifile)
{
char	*s/*, *p*/;
int		n/*, i*/;

n = fab_getnum(ifile);
if (n == 0) return NULL;
s = (char *) smalloc((unsigned) n);
//p = s;
//for (i = 1; i < n; i++)
//	*p++ = fab_getc(ifile->fh_file);
BlockMoveData(ifile->fh_file->Current, s, n - 1);
ifile->fh_file->Current += n - 1;
//*p = '\0'; /* set end of string*/
s[n] = 0;
return s;
}

char *getstr(IFILE *ifile, char *desc)							/*;getstr*/
{
	char	*s, *p;
	int		n, i;


	n = getnum(ifile, "");
	if (n == 0) return (char *)0;
	s = (char *) smalloc((unsigned) n);
	p = s;
	for (i = 1; i < n; i++) {
		*p++ = getc(ifile->fh_file);
	}
	*p = '\0'; /* set end of string*/

	return s;
}

long read_init(IFILE *ifile)								/*;read_init*/
{
	/* initialize read, position at start of first record, return
	 * offset of next record. return 0 if no first first record.
	 * read first word in file that may have offset to slot info.
	 */

	long  pos/*, start*/;
	int   nr;

	nr = fread((char *) &pos, sizeof(long), 1, ifile->fh_file);
	if (nr != 1) {
		chaos(__FILE__, __LINE__, "read_init read failed ");
		return 0;
	}
	return pos;
}

long read_next(IFILE *ifile, long p)						/*;read_next*/
{
	int	 nr;
	long  pos;

	ifseek(ifile, "next-unit", p, 0);
	if (p == ifile->fh_units_end) {
		return 0; /* if at end */
	}
	nr = fread((char *) &pos, sizeof(long), 1, ifile->fh_file);
	if (nr != 1) {
		chaos(__FILE__, __LINE__, "read_next read failure");
		return 0;
	}
	return pos;
}

void putnum(IFILE *ofile, char *desc, int n)						/*;putnum*/
{
	/* write integer (as a short) to output file */

	short s;
	s = n;

	fwrite((char *) &s, sizeof(short), 1, ofile->fh_file);
}

void putpos(IFILE *ofile, char *desc, int n)					/*;putpos*/
{
	/* like putnum, but verifies that argument positive */
	/* write integer (as a short) to output file */

	if (n < 0) chaos(__FILE__, __LINE__, "putpos: negative argument");
	putnum(ofile, desc, n);
}


void putstr(IFILE *ofile, char *desc, char *s)					/*;putstr*/
{

	if (s == nil) {
		putnum(ofile, "", 0);
	}
	else {
		putnum(ofile, "", strlen(s)+1);
		fputs(s, ofile->fh_file);
	}
}

void putchr(IFILE *ofile, char *desc, int n)					/*;putchr*/
{
	/* variant of putnum used when writing character value */
	/* write integer (as a short) to output file */

	short s = n;

	fwrite((char *) &s, sizeof(short), 1, ofile->fh_file);
}
