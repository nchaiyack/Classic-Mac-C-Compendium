#include "comment.header"

#ifndef _game_history_
#define _game_history_

typedef struct {
  int color, x, y;
} gameHistory;

#endif

#define WHITE 1
#define BLACK 2
extern unsigned char p[19][19];
extern int MAXX, MAXY;
static gameHistory gameMoves[600];
int lastMove;

initHistory()
{
  int i,j;
  lastMove = 0;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
    if (p[i][j] != 0) {
      addHistory(p[i][j],i,j);		/* adds handicap stones */
    }
  gameMoves[lastMove].color = -1;
}

addHistory(int color, int x, int y)
{
  if ((lastMove & 1) == 0 && color == WHITE) addHistory(BLACK,-1,-1);
  else if ((lastMove & 1) && color == BLACK) addHistory(WHITE,-1,-1);
  gameMoves[lastMove].color = color;
  gameMoves[lastMove].x = x;
  gameMoves[lastMove].y = y;
  if (lastMove < 599) lastMove++;
  gameMoves[lastMove].color = -1;
}

int getHistory(int i, int *color,int *x,int *y)
{
  *color = gameMoves[i].color ;
  *x = gameMoves[i].x ;
  *y = gameMoves[i].y ;
}

/* */
