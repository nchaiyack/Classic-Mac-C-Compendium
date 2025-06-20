#include "comment.header"
  
#define EMPTY 0
#define BLACKSTONE 2

extern unsigned char p[19][19];
extern int currentStone, opposingStone, MAXX, MAXY;
extern int rd, lib, blackPassed, whitePassed;
extern int selpat,selv;
extern int randomize_flag;
int atari;
extern void eval(int);
extern void markt(),genfield();
extern int fiot(int,int,int);
extern int findwinner(int*,int*,int*);
extern int findsaver(int*,int*,int*);
extern int findpatn(int*,int*,int*);
extern void countlib(int,int,int);
extern int fioe(int,int);
extern void uRandom(int*);
extern int findfmove(int*,int*);
extern int checkatari(int,int);
extern int randmove(int*,int*,int);

void genmove(int *i, int *j)
     /* generate computer move */
{
  int ti, tj, tval;
  int val;
  
  if (randomize_flag) { randmove(i,j,currentStone); return; }
  
  /* initialize move and value */
  *i = -1;  *j = -1;  val = -1; tval = -1; selpat = -1; selv = -1;
  
  /* re-evaluate liberty of opponent pieces */
  eval(opposingStone);
  eval(currentStone);

  /* mark surrounded territory */
  markt(); genfield();
  atari = 0;
  
  /* find opponent piece to capture or attack */
  if (findwinner(&ti, &tj, &tval))
    if (tval > val)
      {
	val = tval; selpat = 3000 + tval;
	*i = ti;
	*j = tj;
      }
  
  /* save any piece if threaten */
  if (findsaver(&ti, &tj, &tval))
    if (tval >= val)
      {
	val = tval; selpat = 2000 + tval;
	*i = ti;
	*j = tj;
      }
  
  /* try match local play pattern for new move */
  if (findpatn(&ti, &tj, &tval))
    if (tval > val && fiot(ti, tj, 3) == 0)
      {
	val = tval;
	*i = ti;
	*j = tj;
      }
  
  /* no urgent move then do random move */
  if (val < 0) {
      if (selpat = findfmove(&ti, &tj)) { *i = ti; *j = tj; }
      if (selpat <= 0 && findsplitter(&ti, &tj)) { *i = ti; *j = tj; }
      lib = 0;
      if (*i >= 0) countlib(*i, *j, currentStone);
      if ( (p[*i][*j] != EMPTY) || (lib < 2) || *i < 0 ||
           fioe(*i, *j) ) { *i = *j = -1; }
      /* avoid illegal move, liberty one or suicide, fill in own eye */
  }

  if (*i < 0) {
      if (currentStone == BLACKSTONE)
	blackPassed = 1;
      else
	whitePassed = 1;
      *i = *j = -1;			/* computer pass */
  }
  else   /* find valid move */
  {
      if (currentStone == BLACKSTONE)
	blackPassed = 0;
      else
	whitePassed = 0;
      eval(opposingStone);
      checkatari(*i,*j);
  }   

#ifdef debug
  printf("selpat = %d, %d\n",selpat, selv);
#endif
}
/* end genmove */
