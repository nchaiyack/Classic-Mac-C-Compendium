#include "comment.header"

#define MAC
#ifdef MAC
void seed(long int *i)
{ *i = TickCount(); }
#else

#include <sys/time.h>

void seed(long int *i)
/* start seed of random number generator for Sun */
  {
   struct timeval tp;
   struct timezone tzp;

   gettimeofday(&tp, &tzp);
   *i = tp.tv_usec;
}  /* end seed */

#endif

