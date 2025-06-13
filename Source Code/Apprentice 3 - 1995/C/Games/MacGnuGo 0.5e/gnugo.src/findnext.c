#include "comment.header"

#define EMPTY 0

extern unsigned char p[19][19], ma[19][19];
extern int currentStone, MAXX, MAXY;
extern int lib;
extern countlib(int,int,int);
extern int getcsize(int,int);

int fval(int newlib, int minlib, int csize)
     /* evaluate new move */
{
  int k, val;
  
  if (newlib <= minlib)
    val = -1;
  else
    {
      k = newlib - minlib;
      /* val = 40 + (k - 1) * 50 / (minlib * minlib * minlib);*/
      val = 20 + csize * (20 + (k - 1) * 50 / (minlib * minlib));
    }
  return (val);
}  /* end fval */


int findnextmove(int m, int n, int *i, int *j, int *val, int minlib)
     /* find new move i, j from group containing m, n */
{
  int ti, tj, tval;
  int found = 0;
  int csize = getcsize(m,n);
  
  *i = -1;   *j = -1;	*val = -1;
  /* mark current position */
  ma[m][n] = 1;
  
  /* check North neighbor */
  if (m != 0)
    if (p[m - 1][n] == EMPTY)
      {
	ti = m - 1;
	tj = n;
	lib = 0;
	countlib(ti, tj, currentStone);
	tval = fval(lib, minlib, csize);
	found = 1;
      }
    else
      if ((p[m - 1][n] == currentStone) && !ma[m - 1][n])
	if (findnextmove(m - 1, n, &ti, &tj, &tval, minlib))
	  found = 1;
  
  if (found)
    {
      found = 0;
      if (tval > *val)
	{
	  *val = tval;
	  *i = ti;
	  *j = tj;
	}
      if (minlib == 1) return 1;
    }
  
  /* check South neighbor */
  if (m != MAXY - 1)
    if (p[m + 1][n] == EMPTY)
      {
	ti = m + 1;
	tj = n;
	lib = 0;
	countlib(ti, tj, currentStone);
	tval = fval(lib, minlib, csize);
	found = 1;
      }
    else
      if ((p[m + 1][n] == currentStone) && !ma[m + 1][n])
	if (findnextmove(m + 1, n, &ti, &tj, &tval, minlib))
	  found = 1;
  
  if (found)
    {
      found = 0;
      if (tval > *val)
	{
	  *val = tval;
	  *i = ti;
	  *j = tj;
	}
      if (minlib == 1) return 1;
    }
  
  /* check West neighbor */
  if (n != 0)
    if (p[m][n - 1] == EMPTY)
      {
	ti = m;
	tj = n - 1;
	lib = 0;
	countlib(ti, tj, currentStone);
	tval = fval(lib, minlib, csize);
	found = 1;
      }
    else
      if ((p[m][n - 1] == currentStone) && !ma[m][n - 1])
	if (findnextmove(m, n - 1, &ti, &tj, &tval, minlib))
	  found = 1;
  
  if (found)
    {
      found = 0;
      if (tval > *val)
	{
	  *val = tval;
	  *i = ti;
	  *j = tj;
	}
      if (minlib == 1) return 1;
    }
  
  /* check East neighbor */
  if (n != MAXX - 1)
    if (p[m][n + 1] == EMPTY)
      {
	ti = m;
	tj = n + 1;
	lib = 0;
	countlib(ti, tj, currentStone);
	tval = fval(lib, minlib, csize);
	found = 1;
      }
    else
      if ((p[m][n + 1] == currentStone) && !ma[m][n + 1])
	if (findnextmove(m, n + 1, &ti, &tj, &tval, minlib))
	  found = 1;
  
  if (found)
    {
      found = 0;
      if (tval > *val)
	{
	  *val = tval;
	  *i = ti;
	  *j = tj;
	}
      if (minlib == 1) return 1;
    }
  
  if (*val > 0)	/* found next move */
    return 1;
  else	/* next move failed */
    return 0;
}  /* end findnextmove */

