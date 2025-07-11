/*
 *		mrandom.c
 *
 *	Wrapper for random(), allowing safe restart.
 *
 *	Original Implementation:
 *		Clark Thomborson, September 1991
 *
 *	This material is based upon work supported by the National
 *	Science Foundation under grant number MIP-9023238.  The
 *	Government has certain rights in this material.
 *
 *	Any opinions, findings, and conclusions or recommendations
 *	expressed in this material are those of the author and do
 *	not necessarily reflect the view of the National Science
 *	Foundation.
 *
 *	This code is neither copyrighted nor patented.
 */

#include <values.h> /* we need MAXLONG */
#include <stdio.h>  /* we need FILE */
#include "mrandom.h"

#define RNGSTATELENGTH	32 /* legal values are 2, 8, 16, 32, 64 */
#define RNGSTATE0	3  /* rngstate[0] for table length 32 */
#define BILLION	1000000000 /* convenient modulus for a 32-bit counter */

/* Data in our RNG statefile */
long	RNGseed;    /* the seed originally used to initialize rngstate */
long	RNGcount1;  /* mod-BILLION counter of random() calls */
long	RNGcount2;  /* div-BILLION counter */
long	rngstate[RNGSTATELENGTH]; /* an array passed to random() */
long	RNGnextval; /* random()'s next output */

/* Format of our RNG statefile */
#define RNGfileLINE1	"Initial seed = %d\n"
#define RNGfileLINE2	\
	"Number of mrandom() calls after seeding = %ld billion + %ld\n"
#define RNGfileLINE3	"RNG state table =\n"
#define RNGfileLINEn	"   %08lx %08lx %08lx %08lx\n"
#define RNGfileLINE12	"Next value in this pseudorandom sequence = %08lx\n"

/* Write a RNG state identifier into the user-supplied string rngid,
 * which must be of length at least RNGIDSTRLEN.  If the user has not
 * initialized the rng with init_rng(), restart_rng(), or reconstruct_rng(),
 * abort with an error message to stderr.  Otherwise return the value of rngid.
 */
extern char *describe_rng (rngid)
char rngid[RNGIDSTRLEN];
{
  if (rngstate[0] != RNGSTATE0) {
    fprintf(stderr, "RNG has not been initialized!\n");
    fflush(stderr);
    exit(1);
  }
  sprintf(rngid, "RNG state identifier is (%ld, %ld, %ld)\n",
		 RNGseed, RNGcount1, RNGcount2);
  return(rngid);
}

/* Create a random number statefile initialized with the given seed.
 * Return 1 if file is successfully created, 0 otherwise.
 */
extern int init_rng (seed, filename)
int seed;
char *filename;
{
  reconstruct_rng(seed,0,0);
  return(save_rng(filename));
} /* end init_rng */

/* Rebuild a random() state by reseeding the generator, then making
 * (count2*1e9 + count1) calls to mrandom().  Useful for error-checking
 * and error-recovery routines, although it is very slow for large counts.
 */
extern void reconstruct_rng (seed, count1, count2)
long seed, count1, count2;
{
  double a;
  initstate(seed, rngstate, RNGSTATELENGTH*4);
  RNGseed = seed; /* keep a record of the seed */
  RNGcount1 = 0;  /* mod-billion counter */
  RNGcount2 = 0;  /* div-billion counter */
  /* watch out for infinite loops */
  if (count1 >= 0 && count1 < BILLION && count2 >= 0) {
    if (count2 != 0) {
      fprintf(stderr, "Warning: this reconstruction will take a LONG time!\n");
      fflush(stderr);
    }
    for ( ; (RNGcount1 != count1) || (RNGcount2 != count2) ; ) {
      a = frandom();
    }
  }
} /* end reconstruct_rng */

/* Return the next random() output without disturbing the RNG state.
   This procedure will only work properly if random's state is in rngstate[].
 */
int nextval()
{
  long state[RNGSTATELENGTH];
  long i, r, retval;
  for (i=0; i<RNGSTATELENGTH; i++) {
    state[i] = rngstate[i];
  }
  retval = random(); /* next value in this pseudorandom sequence */
  for (i=1; i<RNGSTATELENGTH-1; i++) {
    r = random(); /* these calls are necessary to restore random()'s state */
  }
  for (i=0; i<RNGSTATELENGTH; i++) {
    rngstate[i] = state[i];
  }
  return(retval);
}

/* Restart a generator from a statefile.  Print a message on stderr
 * if the restart failed due to a garbled or non-existent statefile, 
 * and return 0.  Otherwise return 1.
 */
extern int restart_rng(filename)
char *filename;
{
  FILE *fp;
  long i, m, newstate[RNGSTATELENGTH], r; /* temps */
  int errflag; /* initially 0, becomes 1 if a problem is discovered */

  /* restore random()'s internal variables to their initial state */
  m = (RNGcount1%(RNGSTATELENGTH-1)) + RNGcount2*(BILLION%(RNGSTATELENGTH-1));
  for ( ; m%(RNGSTATELENGTH-1) != 0; m++) {
    r = random();
  }

  /* restore counter values, retrieve original RNG seed and current state */
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "There is no RNG statefile in this directory!\n");
    fflush(stderr);
    exit(1);
  }
  fscanf(fp, RNGfileLINE1, &RNGseed);
  fscanf(fp, RNGfileLINE2, &RNGcount2, &RNGcount1);
  fscanf(fp, RNGfileLINE3);
  for (i=0; i<RNGSTATELENGTH; i++) {
    fscanf(fp, "%lx", &newstate[i]);
  }
  fscanf(fp, "\n");
  fscanf(fp, RNGfileLINE12, &RNGnextval);
  fclose(fp);

  /* error check: the first word of a 32-word rngstate is always 3 */
  if (newstate[0] != RNGSTATE0) {
    errflag = 1;
  } else {
    errflag = 0;
  }

  /* tell random() we have a 32-word rngstate */
  rngstate[0] = RNGSTATE0;
  setstate(rngstate);

  /* If reconstruction will be rapid, do it as an additional error check. */ 
  if (RNGcount1 < 50 && RNGcount2 == 0) {
    reconstruct_rng(RNGseed, RNGcount1, RNGcount2);
    /* see if we got to the same state */
    for (i=0; i<RNGSTATELENGTH; i++) {
      if (newstate[i] != rngstate[i]) {
        errflag = 1;
      }
    }
  } else {
    /* quickly modify random()'s internal state to line up with RNGcount info */
    m = (RNGcount1%(RNGSTATELENGTH-1)) + RNGcount2*(BILLION%(RNGSTATELENGTH-1));
    for (i=0; i < m%(RNGSTATELENGTH-1); i++) {
      r = random();
    }
  }

  /* copy in the new state */
  for (i=0; i<RNGSTATELENGTH; i++) {
    rngstate[i] = newstate[i];
  }

  /* Check nextval() operation, and verify RNGnextval */
  if (nextval() != nextval() || RNGnextval != nextval()) {
    errflag = 1;
  }

  if (errflag) {
    fprintf(stderr,
	"Warning: RNG statefile is inconsistent.  Did you edit it?\n");
    fprintf(stderr,
	"If not, check your program to make sure you:\n");
    fprintf(stderr,
	"   1. use frandom() or mrandom(m), not random();\n");
    fprintf(stderr,
	"   2. use init_rng(seed, filename), not srandom(seed);\n");
    fprintf(stderr,
	"   3. use restart_rng(filename), not setstate(state); and\n");
    fprintf(stderr,
	"   4. don't overwrite the private storage of mrandom.\n");
    fflush(stderr);
    return(0);
  } else {
    return(1); /* everything looks ok */
  }
} /* end restart_rng */

/* Generate a uniformly-distributed number a, 0.0 <= a < 1.0, using
 * the 4.3bsd additive congruential generator random().
 *
 * This routine keeps track of the number of calls to random().
 *
 * From inspection of the object code for random() in a Sun-3 release,
 * I deduce the following.  A call to initstate() or setstate()
 *   1. defines the location of the state array (rngstate in the code below);
 *   2. defines the length of the state array (32 words in the code below;
 *	other possibilities are 2, 8, 16, or 64 words);
 *   3. initializes several static variables to point at the state array
 *	(the indices j and k in the code below);
 *   4. defines the randomization algorithm (a linear congruential
 *	generator is used if the state array has length 2).
 * Additionally, initstate(seed) fills the state array with the output
 * of a linear congruential generator that is seeded with the given seed.
 *
 * Here is a disassembled and decompiled listing of random(), as
 * it would operate when initialized with my init_rng routine:
 *
 * long random()
 * {
 *   static int j=4, k=1;
 *   long r;   
 *   rngstate[j] += rngstate[k];
 *   r = (rngstate[j]>>1) & 0x7fffffff;
 *   j++;
 *   if (j>31) j=1;
 *   k++;
 *   if (k>31) k=1;
 *   return(r);
 * }
 *
 * Note that j and k return to their original values after every 31
 * calls to random().  This property allows me to write a restart_rng()
 * routine even though I can't read the values of j and k. 
 *
 * Also note that this is a very poor RNG algorithm if it is only
 * called once between successive setstate() calls.  In this pathological
 * case, each output of random() will differ from the prior random() output
 * by rngstate[1]/2 or rngstate[1]/2+1, mod 2^31-1.  Even in non-pathological
 * calling sequences the long-period property of an additive congruential
 * generator is only guaranteed if j and k always move through the array
 * in the manner shown.  Thus the need for counting random() calls
 * (or for writing your own time-optimized additive congruential generator).
 *
 * Finally, note that integer overflow is a frequent occurrence in
 * rngstate[k] += rngstate[j].  Thus random() could conceivably
 * behave differently on different machines.
 */
extern double frandom ()
{
  RNGcount1++; /* mod-billion counter */
  if (RNGcount1 == BILLION) {
    RNGcount1 = 0;
    RNGcount2++; /* an overflow is unlikely in my lifetime */
  }
  return( (double) random()/MAXLONG );
} /* end frandom */

/* Generate a random integer, uniformly distributed in the range 0..m-1.
 * We use the most-significant bits of the result of a random() call,
 * although this may only marginally improve the quality of our output.
 * You may wish to rewrite this routine if you don't have hardware
 * floating point.  If you do, be sure to include the counter-bumping
 * code from frandom(). 
 */
extern long mrandom (m)
long m;
{
  return( (int) (frandom()*m) );
} /* end mrandom */

/* Save the RNG state to a statefile, after calling random() enough
 * times to reset its internal state variables to their initial values.
 * Check to be sure the RNG can be restarted by calling restart_rng().
 * Return 0 if a problem is detected, printing an error message on stderr.
 * Otherwise return 1.
 */
extern int save_rng(filename)
char *filename;
{
  FILE *fp;
  long i;

  /* write the statefile */
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Trouble opening RNG statefile %s for writing.", filename);
    fflush(stderr);
    return(0);
  }
  fprintf(fp, RNGfileLINE1, RNGseed);
  fprintf(fp, RNGfileLINE2, RNGcount2, RNGcount1);
  fprintf(fp, RNGfileLINE3);
  for (i=0; i<RNGSTATELENGTH; i+=4) {
    fprintf(fp, RNGfileLINEn,
	    rngstate[i], rngstate[i+1], rngstate[i+2], rngstate[i+3]);
  }
  fprintf(fp, RNGfileLINE12, nextval());
  fclose(fp);

  /* Return after checking that state was saved correctly. */
  return( restart_rng(filename) );

} /* end save_rng */

/* end mrandom.c */
