#include "comment.header"

extern unsigned char ma[19][19];
extern int MAXX, MAXY;

void initmark()
/* initialize all markings with zero */
{
int i, j;

  for (i = 0; i < MAXX; i++)
    for (j = 0; j < MAXY; j++)
      ma[i][j] = 0;
}  /* end initmark */

