#include "comment.header"

#define EMPTY 0

extern unsigned char p[19][19];

int openregion(int i1, int j1, int i2, int j2)
     /* check if region from i1, j1 to i2, j2 is open */
{
  int minx, maxx, miny, maxy, x, y;
  
  /* exchange upper and lower limits */
  
  if (i1 < i2)
    {
      miny = i1;
      maxy = i2;
    }
  else
    {
      miny = i2;
      maxy = i1;
    }
  
  if (j1 < j2)
    {
      minx = j1;
      maxx = j2;
    }
  else
    {
      minx = j2;
      maxx = j1;
    }
  
  /* check for empty region */
  for (y = miny; y <= maxy; y++)
    for (x = minx; x <= maxx; x++)
      if (p[y][x] != EMPTY) return 0;
  return 1;
}  /* end openregion */
