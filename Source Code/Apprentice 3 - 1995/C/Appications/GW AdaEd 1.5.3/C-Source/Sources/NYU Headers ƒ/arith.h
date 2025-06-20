/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
/* Translation of module ada - arith to C */
/* David Shields  CIMS	11 Nov 83 */

/* translation of arith and multi-precision package to C.
 *
 */
#ifndef _arith_h
#define _arith_h
//#include <math.h>

/*
  Some of the procedures want to signal overflow by returning the
  string 'OVERFLOW'. In C we do this by setting global arith_overflow
  to non-zero if overflow occurs, zero otherwise
 */
extern int arith_overflow ;

extern int	ADA_MIN_INTEGER;
extern int	ADA_MAX_INTEGER;
extern int    *ADA_MAX_INTEGER_MP;
extern int    *ADA_MIN_INTEGER_MP;
extern long	ADA_MIN_FIXED, ADA_MAX_FIXED;
extern int	*ADA_MIN_FIXED_MP, *ADA_MAX_FIXED_MP;

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SIGN(x) ((x)<0 ? -1 : (x)==0 ? 0 : 1)

/* Constants for use by arithmetic packages: */

typedef struct Rational_s
{
	int	*rnum;	/* numerator */
	int	*rden;	/* denominator */
} Rational_s;
typedef Rational_s *Rational;

/* Macros for access to rational numbers: */

#define num(x) x->rnum
#define den(x) x->rden


extern Rational RAT_TWO;
extern	Rational ADA_MAX_FLOAT;

#endif

int *int_abs(int *);
int *int_add(int *, int *);
int int_eql(int *, int *);
int *int_exp(int *, int *);
int *int_fri(int);
int *int_frs(char *);
int int_geq(int *, int *);
int int_gtr(int *, int *);
int int_len(int *);
int int_leq(int *, int *);
int int_lss(int *, int *);
int *int_mod(int *, int *);
int *int_mul(int *, int *);
int int_neq(int *, int *);
int *int_quo(int *, int *);
int *int_rem(int *, int *);
int *int_sub(int *, int *);
int int_toi(int *);
#ifdef MAX_INTEGER_LONG
long int_tol(int *);
#else
long int_tol(int *);
#endif
char *int_tos(int *);
int *int_umin(int *);
int value(char *);
int *int_con(int);
int *int_copy(int *);
int int_eqz(int *);
int int_nez(int *);
#ifdef DEBUG
void int_print(int *);
#endif
void rat_init(void);
Rational rat_new(int *, int *);
#ifdef DEBUG
void rat_print(Rational);
#endif
Rational rat_abs(Rational);
Rational rat_add(Rational, Rational);
Rational rat_div(Rational, Rational);
int rat_eql(Rational, Rational);
Rational rat_exp(Rational, int *);
Rational rat_fri(int *, int *);
Rational rat_frr(double);
Rational rat_frs(char *);
int rat_geq(Rational, Rational);
int rat_gtr(Rational, Rational);
int rat_leq(Rational, Rational);
int rat_lss(Rational, Rational);
Rational rat_mul(Rational, Rational);
int rat_neq(Rational, Rational);
Rational rat_rec(Rational);
Rational rat_red(int *, int *);
Rational rat_sub(Rational, Rational);
double rat_tor (Rational/*, int*/);
int rat_toi(Rational);
long rat_tol(Rational);
Rational rat_umin(Rational);
