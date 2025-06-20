/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

int fab_getint(fabIFILE *ifile);
int getint(IFILE *, char *);
short fab_getnum(fabIFILE *ifile);
int getnum(IFILE *, char *);
int getchr(IFILE *, char *);
long fab_getlong(fabIFILE *ifile);
long getlong(IFILE *, char *);
char *fab_getstr(fabIFILE *ifile);
char *getstr(IFILE *, char *);
long fab_read_init(fabIFILE *ifile);
long read_init(IFILE *);
long fab_read_next(fabIFILE *ifile, long p);
long read_next(IFILE *, long);
void putnum(IFILE *, char *, int);
void putpos(IFILE *, char *, int);
void putstr(IFILE *, char *, char *);
void putchr(IFILE *, char *, int);

#define	fab_getchr(f)	fab_getnum(f)
