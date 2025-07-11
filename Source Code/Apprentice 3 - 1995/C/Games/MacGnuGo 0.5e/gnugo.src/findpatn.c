#include "comment.header"

#define EMPTY 0

extern unsigned char p[19][19];
extern int opn[9];
extern int MAXX, MAXY, currentStone;
extern int opening(int*,int*,int*,int);
extern int openregion(int,int,int,int);
extern int matchpat(int,int,int*,int*,int*);

int findpatn(int *i, int *j, int *val)
     /* find pattern to match for next move */
{
  int m, n;
  int ti, tj, tval;
  static int cnd, mtype;  /* game tree node number, move type */
  /* mtype = 0, basic; 1, inverted; 2, reflected; 3, inverted & reflected */
  
  /* open game then occupy corners */
  if (opn[4])   /* continue last move */
    {
      opn[4] = 0;  /* clear flag */
      if (opening(i, j, &cnd, mtype)) opn[4] = 1; /* more move then reset flag */
      if (p[*i][*j] == EMPTY)  /* valid move */
	{
	  *val = 80;
	  return 1;
	}
      else
	opn[4] = 0;
    }
  
  if (opn[0])   /* Northwest corner */
    {
      opn[0] = 0;  /* clear flag */
      if (openregion(0, 0, 5, 5))
	{
	  cnd = 0;
	  mtype = 0;
	  opening(i, j, &cnd, mtype);  /* get new node for next move */
	  if (opening(i, j, &cnd, mtype)) opn[4] = 1;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[1])   /* Southwest corner */
    {
      opn[1] = 0;
      if (openregion(MAXX - 6, 0, MAXX - 1, 5))
	{
	  cnd = 0;
	  mtype = 1;
	  opening(i, j, &cnd, mtype);  /* get new node for next move */
	  if (opening(i, j, &cnd, mtype)) opn[4] = 1;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[2])   /* Northeast corner */
    {
      opn[2] = 0;
      if (openregion(0, MAXY - 6, 5, MAXY - 1))
	{
	  cnd = 0;
	  mtype = 2;
	  opening(i, j, &cnd, mtype);  /* get new node for next move */
	  if (opening(i, j, &cnd, mtype)) opn[4] = 1;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[3])   /* Northeast corner */
    {
      opn[3] = 0;
      if (openregion(MAXX - 6, MAXY - 6, MAXX - 1, MAXY - 1))
	{
	  cnd = 0;
	  mtype = 3;
	  opening(i, j, &cnd, mtype);  /* get new node for next move */
	  if (opening(i, j, &cnd, mtype)) opn[4] = 1;
	  *val = 80;
	  return 1;
	}
    }
  
  /* occupy edges */
  if (opn[5])   /* North edge */
    {
      opn[5] = 0;
      if (openregion(0, (MAXY/2) - 3, 4, (MAXY/2) + 2))
	{
	  *i = 3;
	  *j = MAXY/2;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[6])   /* South edge */
    {
      opn[6] = 0;
      if (openregion(MAXX - 1, (MAXY/2) - 3, MAXX - 5, (MAXY/2) + 2))
	{
	  *i = MAXX - 4;
	  *j = MAXY/2;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[7])   /* West edge */
    {
      opn[7] = 0;
      if (openregion((MAXX/2) - 3, 0, (MAXX/2) + 2, 4))
	{
	  *i = MAXX/2;
	  *j = 3;
	  *val = 80;
	  return 1;
	}
    }
  
  if (opn[8])   /* East edge */
    {
      opn[8] = 0;
      if (openregion((MAXX/2) - 3, MAXY - 1, (MAXX/2) + 2, MAXY - 5))
	{
	  *i = MAXX/2;
	  *j = MAXY - 4;
	  *val = 80;
	  return 1;
	}
    }
  
  *i = -1;
  *j = -1;
  *val = -1;
  
  /* find local pattern */
  for (m = 0; m < MAXX; m++)
    for (n = 0; n < MAXY; n++)
      if ((p[m][n] == currentStone) &&
	  (matchpat(m, n, &ti, &tj, &tval) && (tval > *val)))
	{
	  *val = tval;
	  *i = ti;
	  *j = tj;
	}
  if (*val > 0)  /* pattern found */
    return 1;
  else  /* no match found */
    return 0;
}  /* end findpatn */

