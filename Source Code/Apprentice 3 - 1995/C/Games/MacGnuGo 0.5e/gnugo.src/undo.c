#include "comment.header"

#define EMPTY 0
#define WHITE 1
#define BLACK 2

extern unsigned char p[19][19];
extern int lastMove;
extern int hcap, pass, mx, my;
extern void initstuff();

undo(int n)
{
   int i, j, x, y, color, m;
   int cont = 0;

   if (lastMove < n || lastMove < 2*hcap) return;
   initstuff();
   m = lastMove - n;	/* discard a pair of moves */
   for (i=0;i<m;i++) {
     getHistory(i, &color, &x, &y);
     if (x >= 0) {
       p[x][y] = color;
       examboard((color == WHITE) ? BLACK : WHITE); /* remove dead pieces */
     }
     mx = x, my = y;
   }
   lastMove = m;
   pass = (pass - n > 0) ? (pass - n) : 0;
}

/* end undo */
