/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

#ifndef _ifile_h
#include "ifile.h"
#endif

char *smalloc(unsigned);
#ifdef DEBUG
void smalloc_list(void);
#endif
int is_smalloc_block(char *);
void capacity(char *file, int line, char *msg);
#ifdef CHAOS
void chaos(char *file, int line, char *msg);
#else
void exit_internal_error(void);
#endif
void exitp(int);
char *ecalloc(unsigned, unsigned);
char *emalloc(unsigned);
char *erealloc(char *, unsigned);
char *strjoin(char *, char *s2);
int streq(char *, char *);
char *substr(char *, int, int);
//#ifdef nogetopt
int getopt(int, char **, char *);
//#endif
char *greentime(int);
FILE *efopenl(char *, char *, char *
#ifdef IBM_PC
, char *
#endif
);
FILE *efopen(char *, char *
#ifdef IBM_PC
, char *
#endif
);
void efree(char *);
int strhash(char *);
char *unit_name_type(char *);
#ifdef BSD
char *strchr(const char *, int);
char *strrchr(const char *, int);
#endif
char *libset(char *);
char *ifname(char *, char *);
fabIFILE *fab_ifopenread(char *filename, char *suffix, int pass);
IFILE *ifopen(char *, char *, const char *, int);
void fab_ifclose(fabIFILE *ifile);
void ifclose(IFILE *);
void ifoclose(IFILE *);
void fab_ifseek(fabIFILE *ifile, Size offset);
long ifseek(IFILE *, char *, long, int);
Size fab_iftell(const fabIFILE *ifile);
long iftell(IFILE *);
char *emalloct(unsigned, char *);
#ifndef EXPORT
char *malloct(unsigned, char *);
char *ecalloct(unsigned, unsigned, char *);
char *fcalloct(size_t n, char *msg);
char *erealloct(char *, unsigned, char *);
void efreet(char *, char *);
#endif

void  set_predef(char *lib);
char *predef_env(void);
char *get_libdir(void);
char *parsefile(char *, int *, int *, int *);
