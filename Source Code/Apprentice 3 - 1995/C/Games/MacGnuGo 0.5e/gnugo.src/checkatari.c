#include "comment.header"

#define EMPTY 0

extern unsigned char p[19][19], l[19][19];
extern int currentStone, opposingStone, MAXX, MAXY;
extern int atari;

int checkatari(int i, int j)
{
    if (i > 0 && p[i-1][j] == opposingStone && l[i-1][j] == 2) atari = 1;
    if (i < MAXX-1 && p[i+1][j] == opposingStone && l[i+1][j] == 2) atari = 1;
    if (j > 0 && p[i][j-1] == opposingStone && l[i][j-1] == 2) atari = 1;
    if (j < MAXY-1&& p[i][j+1] == opposingStone && l[i][j+1] == 2) atari = 1;
    return (atari);
}

/* */

