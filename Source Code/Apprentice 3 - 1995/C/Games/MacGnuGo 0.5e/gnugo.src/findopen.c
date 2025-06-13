#include "comment.header"

#define EMPTY 0
#define BLACKSTONE 2

extern unsigned char p[19][19], ma[19][19];
extern int MAXX, MAXY;
extern int currentStone, blackCapturedKoI, blackCapturedKoJ;  /* piece captured */
extern int whiteCapturedKoI, whiteCapturedKoJ;
extern int getcsize(int, int);

int findopen(int m, int n, int i[], int j[], int color, int minlib, int *ct)
     /* find all open spaces i, j from m, n */
{
  int mik, mjk;
  
  if (getcsize(m,n) < 2) {
    if (currentStone == BLACKSTONE) {
      mik = blackCapturedKoI;
      mjk = blackCapturedKoJ;
    } else {
      mik = whiteCapturedKoI;
      mjk = whiteCapturedKoJ;
    }
  } else { mik = mjk = -1; }
  
  /* mark this one */
  ma[m][n] = 1;
  
  /* check North neighbor */
  if (m != 0)
    {
      if ((p[m - 1][n] == EMPTY) && (((m - 1) != mik) || (n != mjk)))
	{
	  i[*ct] = m - 1;
	  j[*ct] = n;
	  ++*ct;
	  if (*ct == minlib) return 1;
	}
      else
	if ((p[m - 1][n] == color) && !ma[m - 1][n])
	  if (findopen(m - 1, n, i, j, color, minlib, ct) && (*ct == minlib))
	    return 1;
    }
  
  /* check South neighbor */
  if (m != MAXY - 1)
    {
      if ((p[m + 1][n] == EMPTY) && (((m + 1) != mik) || (n != mjk)))
	{
	  i[*ct] = m + 1;
	  j[*ct] = n;
	  ++*ct;
	  if (*ct == minlib) return 1;
	}
      else
	if ((p[m + 1][n] == color) && !ma[m + 1][n])
	  if (findopen(m + 1, n, i, j, color, minlib, ct) && (*ct == minlib))
	    return 1;
    }
  
  /* check West neighbor */
  if (n != 0)
    {
      if ((p[m][n - 1] == EMPTY) && ((m != mik) || ((n - 1) != mjk)))
	{
	  i[*ct] = m;
	  j[*ct] = n - 1;
	  ++*ct;
	  if (*ct == minlib) return 1;
	}
      else
	if ((p[m][n - 1] == color) && !ma[m][n - 1])
	  if (findopen(m, n - 1, i, j, color, minlib, ct) && (*ct == minlib))
	    return 1;
    }
  
  /* check East neighbor */
  if (n != MAXX - 1)
    {
      if ((p[m][n + 1] == EMPTY) && ((m != mik) || ((n + 1) != mjk)))
	{
	  i[*ct] = m;
	  j[*ct] = n + 1;
	  ++*ct;
	  if (*ct == minlib) return 1;
	}
      else
	if ((p[m][n + 1] == color) && !ma[m][n + 1])
	  if (findopen(m, n + 1, i, j, color, minlib, ct) && (*ct == minlib))
	    return 1;
    }
  
  /* fail to find open space */
  return 0;
}  /* end findopen */
