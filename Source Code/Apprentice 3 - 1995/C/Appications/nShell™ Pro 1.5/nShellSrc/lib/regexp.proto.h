/*
 =
 = This file was originally "regproto.h", by Henry Spencer.
 =
 = See the "regexp.c" in this package for more information.
 =
*/

// Routines found in regexp.c:

regexp *regcomp(char *exp);
long regexec(register regexp *prog, register char *string);

static char *reg(long paren, long *flagp);
static char *regbranch(long *flagp);
static char *regpiece(long *flagp);
static char *regatom(long *flagp);
static char *regnode(char op);
static void regc(char b);
static void reginsert(char op, char *opnd);
static void regtail(char *p, char *val);
static void regoptail(char *p, char *val);
static long regtry(regexp *prog, char *string);
static long regmatch(char *prog);
static long regrepeat(char *p);
static char *regnext(register char *p);

// Define this in your calling module, so you can report errors

void regerror(char *s);
