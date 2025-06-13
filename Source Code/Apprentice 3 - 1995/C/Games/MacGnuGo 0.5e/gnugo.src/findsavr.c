#include "comment.header"

extern unsigned char p[19][19], l[19][19];
extern int currentStone, MAXX, MAXY;
extern void initmark();
extern int findnextmove(int,int,int*,int*,int*,int);

int findsaver(int *i, int *j, int *val)
     /* find move if any pieces are threatened */
{
  int m, n, minlib;
  int ti, tj, tval;
  
  *i = -1;   *j = -1;	 *val = -1;
  for (minlib = 1; minlib < 4; minlib++)
    {
      /* count piece with minimum liberty */
      for (m = 0; m < MAXX; m++)
	for (n = 0; n < MAXY; n++)
	  if ((p[m][n] == currentStone) && (l[m][n] == minlib))
	    /* find move to save pieces */
	    {
	      initmark();
	      if (findnextmove(m, n, &ti, &tj, &tval, minlib) && (tval > *val))
		{
		  *val = tval;
		  *i = ti;
		  *j = tj;
		}
	    }
    }
  if (*val > 0)   /* find move */
    return 1;
  else	    /* move not found */
    return 0;
}  /* findsaver */

