#include "comment.header"
  
#define EMPTY 0
#define WHITESTONE 1
#define BLACKSTONE 2

extern unsigned char p[19][19], l[19][19];
extern int currentStone, opposingStone, MAXX, MAXY;
extern long int rd;
extern int lib, blackPassed, whitePassed;
extern void countlib(int,int,int);
extern int fioe(int,int);
extern void uRandom(long int*);
extern int blackCapturedKoI, blackCapturedKoJ,
           whiteCapturedKoI, whiteCapturedKoJ;
int fioe2(int,int,int);

void randmove(int *i, int *j, int me)
     /* generate random move */
{
  int t, ti, tj;
  int maxtry = 3*MAXX*MAXY;
  int try;
  int lib2;
  int you;
  int uik,ujk;
  
  /* initialize move  */
  *i = -1;  *j = -1;
  
  if (me == opposingStone) you = currentStone;
  else                     you = opposingStone;

  if (me == BLACKSTONE) {
      uik = blackCapturedKoI;
      ujk = blackCapturedKoJ;
  } else {
      uik = whiteCapturedKoI;
      ujk = whiteCapturedKoJ;
  }
  
  /* printf("ko : %c%d  \n", 'a' + ujk, MAXX - uik);
    if (uik >= 0) getchar(); /* debug rhn xxx */
    
  eval(you);
  
  for (try = maxtry; try > 0; try--) {
      uRandom(&rd);
      t = (rd + *i + *j + 3) % (MAXX + 2);
      uRandom(&rd);
      *i = (t + rd) % MAXX;
      uRandom(&rd);
      *j = (t + rd) % MAXY;

      if (p[*i][*j] != EMPTY) continue;
      if ((*i == uik) && (*j == ujk)) continue;

      lib = 0;
      countlib(*i, *j, me);
      lib2 = lib;

      if (lib < 2) {
	if (*i > 0 && p[*i-1][*j] == you && l[*i-1][*j] == 1) { lib2 = 2; }
	if (*i < MAXX-1 && p[*i+1][*j] == you && l[*i+1][*j] == 1) { lib2 = 2; }
	if (*j > 0 && p[*i][*j-1] == you && l[*i][*j-1] == 1) { lib2 = 2; }
	if (*j < MAXY-1 && p[*i][*j+1] == you && l[*i][*j+1] == 1) { lib2 = 2; }
      }

      if ( (lib2 < 2) || fioe2(*i,*j,me) ) continue;
      else break;
   }
   if (try < 2) *i = *j = -1;
      /* avoid illegal move, liberty one or suicide, fill in own eye */
/* printf("rand move: %c%-2d  %d  \n", 'a' + *j, MAXX - *i, maxtry-try); /* */
}

int fioe2(int x,int y,int me)
{
  int i,j,a,n,m;
  n = 0; m = 0; a = 0;
  for (i = x-1; i < x+2; i++ ) for (j = y-1; j < y+2; j++) {
    if (i < 0 || i > MAXX-1 || j < 0 || j > MAXY-1) continue;
    else if (i == x && j == y) continue;
    else if (p[i][j] != me) { m++; if (i == x || j == y) a++; }
    else n++;
  }
  if (a > 0) return(0);
  else if (n < 6) {
    if (m > 0) return (0);
    else return(1);
  }
  else return (1);
}
/* end randmove */
