#include "::h:gsupport.h"

/*
 * redirerr - redirect error output to the named file. '-' indicates that
 *  it should be redirected to standard out.
 */
int redirerr(p)
char *p;
   {
   if ( *p == '-' ) { /* let - be stdout */
/*
 * The following code is operating-system dependent [@redirerr.01].  Redirect
 *  stderr to stdout.
 */

#if PORT
   /* may not be possible */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA
#if AZTEC_C
       /*
        * Try the same hack as above for Manx and cross fingers.
        * If it doesn't work, try trick used for HIGH_C, below.
        */
       stderr->_unit  = stdout->_unit;
       stderr->_flags = stdout->_flags;
#endif					/* AZTEC C */
#if LATTICE
      /*
       * The following code is for Lattice 4.0.  It was different
       *  for Lattice 3.10 and probably won't work for other
       *  C compilers.
       */
      stderr->_file = 1;
      stderr->_flag = stdout->_flag;
#endif					/* LATTICE */
#endif					/* AMIGA */

#if ARM || MVS || VM
   /* cannot do this */
#endif					/* ARM || MVS || VM */

#if ATARI_ST || MSDOS || OS2 || VMS
#if HIGHC_386 || MSNT
      /*
       * Don't like doing this, but it seems to work.
       */
      setbuf(stdout,NULL);
      setbuf(stderr,NULL);
      stderr->_file = stdout->_file;                
#else					/* HIGHC_386 */
      dup2(fileno(stdout),fileno(stderr));
#endif					/* HIGHC_386 */
#endif					/* ATARI_ST || MSDOS || OS2 ... */


#if MACINTOSH
#if LSC
   /* cannot do */
#endif				/* LSC */
#if MPW
      close(fileno(stderr));
      dup(fileno(stdout));
#endif				/* MPW */
#endif                         /* MACINTOSH */

#if UNIX
      /*
       * This relies on the way UNIX assigns file numbers.
       */
      close(fileno(stderr));
      dup(fileno(stdout));
#endif				/* UNIX */

/*
 * End of operating-system specific code.
 */

       }
    else    /* redirecting to named file */
       if (freopen(p, "w", stderr) == NULL)
          return 0;
   return 1;
   }
