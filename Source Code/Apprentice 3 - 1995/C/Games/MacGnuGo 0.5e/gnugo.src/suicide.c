#include "comment.header"

#define EMPTY 0
#define BLACKSTONE 2

extern unsigned char p[19][19], l[19][19];
extern int currentStone, opposingStone, MAXX, MAXY;
extern int lib;
extern int blackCapturedKoI, blackCapturedKoJ, whiteCapturedKoI, whiteCapturedKoJ;  /* piece captured */
extern void countlib(int, int, int);
extern void eval(int);

int suicide(int i, int j)
/* check for suicide move of opponent at p[i][j] */
{
 int m, n, k, uik, ujk;

/* check liberty of new move */
 lib = 0;
 countlib(i, j, opposingStone);
 if (lib == 0)
/* new move is suicide then check if kill my pieces and Ko possibility */
   {
/* assume alive */
    p[i][j] = opposingStone;

/* check opponent pieces */
    eval(currentStone);
    k = 0;

    for (m = 0; m < MAXX; m++)
      for (n = 0; n < MAXY; n++)
/* count pieces that will be killed */
	if ((p[m][n] == currentStone) && !l[m][n]) ++k;

    if (opposingStone == BLACKSTONE) {
      uik = blackCapturedKoI;
      ujk = blackCapturedKoJ;
    } else {
      uik = whiteCapturedKoI;
      ujk = whiteCapturedKoJ;
    }
    if ((k == 0) || (k == 1 && ((i == uik) && (j == ujk))))
/* either no effect on my pieces or an illegal Ko take back */
      {
       p[i][j] = EMPTY;   /* restore to open */
       return 1;
      }
    else
/* good move */
      return 0;
   }
 else
/* valid move */
   return 0;
}  /* end suicide */

