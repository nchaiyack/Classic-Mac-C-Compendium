/*
 *		mrtest.c
 *
 *	Test routine for mrandom.c 
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

# include <sys/file.h> /* we use access() */
# include "mrandom.h"

# define RNGFILENAME "RNGstatefile" /* where the RNG state is stored */

/* calling conventions */
void argerr(progname)
char *progname;
{
  printf("Usage: %s nn -snnnnnnnn -mnn R\n", progname);
  printf("  nn sets number of random generates\n");
  printf("  -snnnnnnnn seeds the RNG with the given value\n");
  printf("  -mnnnn adjusts the range of the RNG\n");
  exit(1);
} /* end argerr */

/* A simple test-driver */
int main(argc,argv)
int argc; char *argv[];
{
  /* command-line arguments, with default values */
  int reseeding=0; /* nonzero if RNG state will be re-initialized */
  int seed=1573122697; /* new seed for RNG */
  int n=10; /* number of randoms to generate */
  int m=100; /* desired range of random outputs: 0..m-1 */

  int i; /* temp */
  char rngdesc[RNGIDSTRLEN]; /* for a describe_rng() call */

  if(argc > 1) {
    for(i=1;i<argc;i++) {
      if(argv[i][0] >= '0' && argv[i][0] <= '9') {
	n = atoi(&(argv[i][0]));
      } else if(argv[i][0] == '-') {
	switch(argv[i][1]) {
	case 's': /* new seed for rng */
	  seed = atoi(&(argv[i][2]));
	  reseeding = 1;
	  break;
	case 'm': /* adjust range of rng */
	  m = atoi(&(argv[i][2]));
	  break;
	default:
	  argerr((char*) argv[0]);
	}
      } else {
	argerr((char*) argv[0]);
      }
    }
  }
  
  if (!reseeding ) {
    if (access(RNGFILENAME, R_OK)) {
      printf("There is no RNG statefile in this directory, so ");
      printf("I'll make one for you.\n");
      reseeding = 1;
    }
  }

  if (reseeding) { /* create a new statefile from scratch */
    if (!init_rng(seed,RNGFILENAME)) {
      exit(1);
    }
    printf("Initializing RNG.   ");
  } else { /* use an existing statefile */
    if (!restart_rng(RNGFILENAME)) {
      exit(1);
    }
    printf("Restarting RNG.  ");
  }
  printf(describe_rng(rngdesc));

  printf("Here are %d random values in the range 0 to %d:\n", n, m-1);
  for (i=0; i<n; i++) {
    printf("  %d\n", mrandom(m));
  }

  /* terminate job */
  printf("Final ");
  printf(describe_rng(rngdesc));

  return( save_rng(RNGFILENAME) );

} /* end main */
