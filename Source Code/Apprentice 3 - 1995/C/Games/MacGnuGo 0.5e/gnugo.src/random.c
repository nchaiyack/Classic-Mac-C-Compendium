#include "comment.header"

extern void seed(long int*);

void uRandom(long int *i)
/* random number generator */
  {
   if (*i == 0)
     seed(i);
   else
     {
      *i = *i * 137 % 3833;
      if (*i < 0) *i = -*i;
   }
}  /* end random */
