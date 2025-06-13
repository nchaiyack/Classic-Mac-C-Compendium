#include "comment.header"
extern unsigned char p[19][19], l[19][19];
extern int MAXX, MAXY;
extern int currentStone, opposingStone;
extern long int rd;
extern int lastMove;

/* don't make moves inside territory of chains with 2 eyes */

/*
  t	mark all enclosed territories with a unique number
  t1	mark all enclosed territories again
  tc	mark a territory if it includes a corner
  trs	mark territories owned by one side ( + current , - opponent )
  cn	mark all chains
  csafe	mark chains touching 2 owned territories
  csize mark size of chains
  tsize mark size of territories
  tsafe	mark size of territories enclosed by safe chains
*/

static int t[19][19];		/* any territory */
static int t1[19][19];		/* any territory */
static int tc[19][19];		/* any territory */
static int trs[19][19];		/* surrounded territory */
static int cn[19][19];		/* chains */
static int csize[19][19];	/* size of chains */
static int csafe[19][19];	/* safe chains */
static int tsize[19][19];	/* territory size */
static int tsafe[19][19];	/* safe territory */
static int ttime[19][19];	/* when was it safe/ */
static int numt, c1, o1, a1, minlib1;

static void setmin1(int n) { if (n < minlib1) minlib1 = n; }

static void mter(int i, int j, int n)
{
  t[i][j] = n;
  if (i > 0 && t[i-1][j] == 0 && p[i-1][j] == 0) mter(i-1,j,n);
  if (i < MAXX-1 && t[i+1][j] == 0 && p[i+1][j] == 0) mter(i+1,j,n);
  if (j > 0 && t[i][j-1] == 0 && p[i][j-1] == 0) mter(i,j-1,n);
  if (j < MAXY-1 && t[i][j+1] == 0 && p[i][j+1] == 0) mter(i,j+1,n);
  if (i > 0 && t[i-1][j] == 0 && p[i-1][j] == currentStone) o1 = 0;
  if (i < MAXX-1 && t[i+1][j] == 0 && p[i+1][j] == currentStone) o1 = 0;
  if (j > 0 && t[i][j-1] == 0 && p[i][j-1] == currentStone) o1 = 0;
  if (j < MAXY-1 && t[i][j+1] == 0 && p[i][j+1] == currentStone) o1 = 0;
  if (i > 0 && t[i-1][j] == 0 && p[i-1][j] == opposingStone) c1 = 0;
  if (i < MAXX-1 && t[i+1][j] == 0 && p[i+1][j] == opposingStone) c1 = 0;
  if (j > 0 && t[i][j-1] == 0 && p[i][j-1] == opposingStone) c1 = 0;
  if (j < MAXY-1 && t[i][j+1] == 0 && p[i][j+1] == opposingStone) c1 = 0;
  if (i > 0 && t[i-1][j] == 0 && p[i-1][j] != 0) setmin1(l[i-1][j]);
  if (i < MAXX-1 && t[i+1][j] == 0 && p[i+1][j] != 0) setmin1(l[i+1][j]);
  if (j > 0 && t[i][j-1] == 0 && p[i][j-1] != 0) setmin1(l[i][j-1]);
  if (j < MAXY-1 && t[i][j+1] == 0 && p[i][j+1] != 0) setmin1(l[i][j+1]);
}

static void mts(int i, int j, int n)
{
  t1[i][j] = n;
  if (i > 0 && t1[i-1][j] == 0 && p[i-1][j] == 0) mts(i-1,j,n);
  if (i < MAXX-1 && t1[i+1][j] == 0 && p[i+1][j] == 0) mts(i+1,j,n);
  if (j > 0 && t1[i][j-1] == 0 && p[i][j-1] == 0) mts(i,j-1,n);
  if (j < MAXY-1 && t1[i][j+1] == 0 && p[i][j+1] == 0) mts(i,j+1,n);
  if (trs[i][j] != 0) {
    if (i > 0 && csafe[i-1][j] > 0) a1 = 1;
    if (i < MAXX-1 && csafe[i+1][j] > 0) a1 = 1;
    if (j > 0 && csafe[i][j-1] > 0) a1 = 1;
    if (j < MAXY-1 && csafe[i][j+1] > 0) a1 = 1;
  }
}

static void mcn(int i, int j, int n)
{
  int a;

  a = p[i][j];
  cn[i][j] = n;
  if (i > 0 && cn[i-1][j] == 0 && p[i-1][j] == a) mcn(i-1,j,n);
  if (i < MAXX-1 && cn[i+1][j] == 0 && p[i+1][j] == a) mcn(i+1,j,n);
  if (j > 0 && cn[i][j-1] == 0 && p[i][j-1] == a) mcn(i,j-1,n);
  if (j < MAXY-1 && cn[i][j+1] == 0 && p[i][j+1] == a) mcn(i,j+1,n);

  if (a == currentStone && a1 > -1) {
    if (i > 0 && p[i-1][j] == 0 && trs[i-1][j] > 0 && t[i-1][j] != a1)
      { if (a1 == 0) a1 = t[i-1][j]; else a1 = -1; }
    if (i < MAXX-1 && p[i+1][j] == 0 && trs[i+1][j] > 0 && t[i+1][j] != a1)
      { if (a1 == 0) a1 = t[i+1][j]; else a1 = -1; }
    if (j > 0 && p[i][j-1] == 0 && trs[i][j-1] > 0 && t[i][j-1] != a1)
      { if (a1 == 0) a1 = t[i][j-1]; else a1 = -1; }
    if (j < MAXY-1 && p[i][j+1] == 0 && trs[i][j+1] > 0 && t[i][j+1] != a1)
      { if (a1 == 0) a1 = t[i][j+1]; else a1 = -1; }
  }

  if (a == opposingStone && a1 > -1) {
    if (i > 0 && p[i-1][j] == 0 && trs[i-1][j] < 0 && t[i-1][j] != a1)
      { if (a1 == 0) a1 = t[i-1][j]; else a1 = -1; }
    if (i < MAXX-1 && p[i+1][j] == 0 && trs[i+1][j] < 0 && t[i+1][j] != a1)
      { if (a1 == 0) a1 = t[i+1][j]; else a1 = -1; }
    if (j > 0 && p[i][j-1] == 0 && trs[i][j-1] < 0 && t[i][j-1] != a1)
      { if (a1 == 0) a1 = t[i][j-1]; else a1 = -1; }
    if (j < MAXY-1 && p[i][j+1] == 0 && trs[i][j+1] < 0 && t[i][j+1] != a1)
      { if (a1 == 0) a1 = t[i][j+1]; else a1 = -1; }
  }
}

static void copytrs(int n, int v)
{
  int i,j;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) 
      if (t[i][j] == n) trs[i][j] = v;
}

static void copycsafe(int n, int v)
{
  int i,j;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) 
      if (cn[i][j] == n) csafe[i][j] = v;
}

static void copytsafe(int n, int v)
{
  int i,j;
  int m = 0;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) 
      if (t1[i][j] == n) m++;
  if (v < 0) m = -m;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) 
      if (t1[i][j] == n) {
	tsafe[i][j] = m;
	ttime[i][j] = lastMove;
      }
}

void markcorners()
{
  int i,j,n;
  if ((n = t[0][0]) != 0)
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (t[i][j] == n) tc[i][j] = 2;
  if ((n = t[0][MAXX-1]) != 0)
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (t[i][j] == n) tc[i][j] = 2;
  if ((n = t[0][MAXY-1]) != 0)
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (t[i][j] == n) tc[i][j] = 2;
  if ((n = t[MAXX-1][MAXY-1]) != 0)
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (t[i][j] == n) tc[i][j] = 2;
}

/* marks territory inside an eye (maybe a false eye though) */

void markt()
{
  int i,j,k,n,m;

  numt = 1;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {
    t[i][j] = 0; t1[i][j] = 0; tc[i][j] = 0; trs[i][j] = 0;
    cn[i][j] = 0; csafe[i][j] = 0; csize[i][j] = 0;
    tsize[i][j] = 0; tsafe[i][j] = 0; ttime[i][j] = -10;
  }
  for (i=0;i<MAXX;i++)
    for (j=0;j<MAXY;j++) 
      if (t[i][j] == 0 && p[i][j] == 0) {
        o1 = 1;
	c1 = 1;
	minlib1 = 300;
	mter(i,j,numt);
	if (c1 && !o1) { copytrs(numt, minlib1); }
	if (o1 && !c1) { copytrs(numt, -minlib1); }
        numt++;
      }
  for (k=1;k<numt;k++) {
    n = 0;
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
      if (t[i][j] == k) n++;
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
      if (t[i][j] == k) tsize[i][j] = n;
  }
  m = numt;
  for (i=0;i<MAXX;i++)
    for (j=0;j<MAXY;j++) 
      if (cn[i][j] == 0 && p[i][j] != 0) {
        a1 = 0;
	mcn(i,j,numt);
	if (a1 < 0) { copycsafe(numt, p[i][j]); }
        numt++;
      }
  for (k=m;k<numt;k++) {
    n = 0;
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
      if (cn[i][j] == k) n++;
    for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
      if (cn[i][j] == k) csize[i][j] = n;
  }
      
  for (i=0;i<MAXX;i++)
    for (j=0;j<MAXY;j++) 
      if (t1[i][j] == 0 && trs[i][j] != 0) {
	a1 = 0;
	mts(i,j,numt);
	if (a1 == 1) { copytsafe(numt, trs[i][j]); }
	numt++;
      }
   markcorners();
}

/* count number of stones surrounding a territory */
static int countwall(int x, int y)
{
  int t2[19][19];	/* surrounding stones */
  int i,j,n,m;
  if (p[x][y] != 0) return(0);
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) t2[i][j] = 0;
  m = t[x][y];
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
    if (t[i][j] == m) {
      if (i > 0 && p[i-1][j] != 0) t2[i-1][j] = 1;
      if (i < MAXX-1 && p[i+1][j] != 0) t2[i+1][j] = 1;
      if (j > 0 && p[i][j-1] != 0) t2[i][j-1] = 1;
      if (j < MAXY-1 && p[i][j+1] != 0) t2[i][j+1] = 1;
    }
  for (n=0,i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
    if (t2[i][j] == 1) n++;
  return(n);
}

int throwin(int i, int j)
{
  if (csize[i][j] == 1 && ttime[i][j] > lastMove - 5) return (1);
  else return (0);
}

int locallibs(int i, int j)
{ int n = 0;
  if (i > 0 && p[i-1][j] == 0) n++;
  if (i < MAXX-1 && p[i+1][j] == 0) n++;
  if (j > 0 && p[i][j-1] == 0) n++;
  if (j < MAXY-1 && p[i][j+1] == 0) n++;
  return(n);
}

int getcsize(int i, int j) { return (csize[i][j]); }

int findsplitter(int *x, int *y)
{
  int i,j,n,val;
  val = 0; *x = *y = -1;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {
    if ( tsafe[i][j] == 0 &&
         ( (trs[i][j] > 0 && tsize[i][j] >= 3) ||
	   (trs[i][j] < 0 && tsize[i][j] == 3) )) {
      n = locallibs(i,j);
      if (n > val) { val = n; *x = i; *y = j; }
    }
  }
  return(val > 1);
}

/* returns true if move is inside an eye (not too large an eye, though) */

#ifdef debug
#include <stdio.h>
int debug_dump() { int i,j; putchar('\n');
  for (i = 0; i < MAXX; i++) {
      for (j = 0; j < MAXY; j++) printf("%2d ",trs[i][j]);
      putchar(';');
      for (j = 0; j < MAXY; j++) printf("%2d ",tsize[i][j]);
      putchar('\n'); }
  putchar('\n');
}
#else
int debug_dump() { return 0; }
#endif

#define NOT_OK 1
#define OK 0

/* return 1 if inside inside strong territory */
int fiot(int i, int j, int k)
{
  int n,s;
#ifdef debug
  printf("%d %d; ",i,j);
#endif
  if (trs[i][j] == 0) return(OK);	/* not anybodies terrirory */
  if (tsafe[i][j] > 0 && tsafe[i][j] <  9) return(NOT_OK); /* my eye */
  if (tsafe[i][j] < 0 && tsafe[i][j] > -9) return(NOT_OK); /* opponent eye */
  s = tsize[i][j];
  if (locallibs(i,j) < 2) return(NOT_OK);
  if (tsafe[i][j] != 0 && trs[i][j] > 0) {	/* my territory */
    if (s < 3) return(NOT_OK);
    n = countwall(i,j);
    if (s > MAXX*MAXY/3) return(OK);
    if (tc[i][j] == 2) {
         if (n*n/4 > s) return(NOT_OK);	/* in the corner */
    }
    else if (n*n*n/27 > s) return(NOT_OK);
  }
  else if (trs[i][j] < 0) {			/* opponent territory */
    if (tsafe[i][j] == 0 && tsize[i][j] <= 3) return(OK);
    n = countwall(i,j);
    if (tc[i][j] == 2) {
         if (n*n/4 > s) return(NOT_OK);
    }
    else if (n*n*n/27 > s) return(NOT_OK);
  }
  return(OK);
}

/* */
