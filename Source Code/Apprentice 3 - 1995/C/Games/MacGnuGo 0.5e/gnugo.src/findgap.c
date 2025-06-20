#include "comment.header"
/* */
extern unsigned char p[19][19], l[19][19];
extern int MAXX, MAXY;
extern int currentStone, opposingStone;
extern long int rd;

extern int fiot(int , int , int );
extern int fioe(int , int );
extern void countlib(int , int , int );
static int tdif(int , int );
static int u[19][19];
extern void uRandom(long int*);

#define FMAX 10

/* a poor attempt at finding the boundries between possible territories */
/* genfield generates a field strength */
/* findfmove returns a move on the boundry */
/* 	1  i--	2	field directions
 * 	 j-- j++
 * 	3  i++	4
 */

static void field1(int i,int j,int n,int dir) 
{
    u[i][j] += n;
    if (n > 0) --n;
    if (n < 0) ++n;
    if (n == 0) return;
    if (i > 0 && dir < 3 && p[i-1][j] == 0) field1(i-1,j,n,dir);
    if (i < MAXX-1 && dir > 2 && p[i+1][j] == 0) field1(i+1,j,n,dir);
    if (j > 0 && (dir % 1) && p[i][j-1] == 0) field1(i,j-1,n,dir);
    if (j < MAXY-1 && !(dir % 1) && p[i][j+1] == 0) field1(i,j+1,n,dir);
}

int fieldval(int i,int j) { return(FMAX - u[i][j]); }

void genfield()
{
  int i,j;
  int c1,o1,n,m,s;
  long int max;
  int ave;

  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) u[i][j] = 0;

  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {
    if (p[i][j] == currentStone) {
      field1(i,j,6,1);
      field1(i,j,6,3);
      field1(i,j,6,2);
      field1(i,j,6,4);
    }
    if (p[i][j] == opposingStone) {
      field1(i,j,-6,1);
      field1(i,j,-6,3);
      field1(i,j,-6,2);
      field1(i,j,-6,4);
    }
  }
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (p[i][j] != 0) u[i][j] = 0;
  max = 0;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) max += u[i][j];
  ave = max / (MAXX * MAXY);

  max = -100;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {
    if (u[i][j] < ave) u[i][j] = ave - u[i][j];
    else u[i][j] = u[i][j] - ave;
    if (u[i][j] > max) max = u[i][j];
  }
  if (max > 0) for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++)
    u[i][j] = FMAX - (FMAX * u[i][j])/max;
  for (i=0;i<MAXX;i++) {
    if (u[i][0] > 1) u[i][0] /= 2;
    if (u[i][MAXY-1] > 1) u[i][MAXY-1] /= 2;
    if (u[0][i] > 1) u[0][i] /= 2;
    if (u[MAXX-1][i] > 1) u[MAXX-1][i] /= 2;
  }
  /* corner defense */
  s = (MAXX > 10) ? 5 : 1 + MAXX/2;c1 = 0; o1 = 0;
  c1 = 0; o1 = 0;
  for (i=0;i<s;i++) for (j=0;j<s;j++) {
    if (p[i][j] == currentStone) c1++;
    if (p[i][j] == opposingStone) o1++;
  }
  if (c1+1 >= o1 && o1*2 >= c1)
    for (i=1;i<s;i++) for (j=1;j<s;j++) u[i][j] += FMAX * 20;
  c1 = 0; o1 = 0;
  for (i=0;i<s;i++) for (j=MAXY-1;j>=MAXY-s;j--) {
    if (p[i][j] == currentStone) c1++;
    if (p[i][j] == opposingStone) o1++;
  }
  if (c1+1 >= o1 && o1*2 >= c1)
    for (i=1;i<s;i++) for (j=MAXY-2;j>=MAXY-s;j--) u[i][j] += FMAX * 20;
  c1 = 0; o1 = 0;
  for (i=MAXX-1;i>=MAXX-s;i--) for (j=0;j<s;j++) {
    if (p[i][j] == currentStone) c1++;
    if (p[i][j] == opposingStone) o1++;
  }
  if (c1+1 >= o1 && o1*2 >= c1)
    for (i=MAXX-2;i>=MAXX-s;i--) for (j=1;j<s;j++) u[i][j] += FMAX * 20;
  c1 = 0; o1 = 0;
  for (i=MAXX-1;i>=MAXX-s;i--) for (j=MAXY-1;j>=MAXY-s;j--) {
      if (p[i][j] == currentStone) c1++;
      if (p[i][j] == opposingStone) o1++;
    }
  if (c1+1 >= o1 && o1*2 >= c1)
    for (i=MAXX-2;i>=MAXX-s;i--)
      for (j=MAXY-2;j>=MAXY-s;j--) u[i][j] += FMAX * 20;
  
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) if (p[i][j] != 0) u[i][j] = 0;
#ifdef debug
  for (i = 0; i < MAXX; i++) {
      for (j = 0; j < MAXY; j++) printf("%2d ",u[i][j]);
      putchar(';');
      putchar('\n'); }
  putchar('\n');
#endif
}

extern int lib;
      
int findfmove(int *x, int *y)
{
  int i,j;
  int max, ti,tj;

  max = -100; ti = -1; tj = -1;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {

      /* avoid illegal move, liberty one or suicide, fill in own eye */
      if (p[i][j] != 0) continue;
      if (u[i][j] <= 0 || u[i][j] < max - 1) continue;
      if (fiot(i,j,3)) continue;
      if (fioe(i,j)) continue;
      lib = 0;
      countlib(i, j, currentStone);
      if (lib < 2) continue;

      if (u[i][j] > max) {
	  max = u[i][j]; ti = i; tj = j;
      } else if (u[i][j] >= max - 1) {
          uRandom(&rd);
	  if (rd%1) {
	    max = u[i][j]; ti = i; tj = j;
	  }
      }
  }
  *x = ti; *y = tj;
  if (max > 0) return(max);
  else return(0);
}

/* */

