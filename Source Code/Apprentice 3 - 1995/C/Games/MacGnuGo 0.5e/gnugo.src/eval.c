#include "comment.header"

extern unsigned char p[19][19], l[19][19];
extern int lib, MAXX, MAXY;
extern void countlib(int i, int j, int color);

void eval(int color)
     /* evaluate liberty of color pieces */
{
  int i, j;
  
  /* find liberty of each piece */
  for (i = 0; i < MAXX; i++)
    for (j = 0; j < MAXY; j++)
      if (p[i][j] == color)
	{
	  lib = 0;
	  countlib(i, j, color);
	  l[i][j] = lib;
	}
#ifdef debug
  for (i = 0; i < MAXX; i++) {
    for (j = 0; j < MAXY; j++) printf("%2d ",l[MAXX-i-1][j]);
    putchar('\n');
  }
  putchar('\n');
#endif
}  /* end eval */
