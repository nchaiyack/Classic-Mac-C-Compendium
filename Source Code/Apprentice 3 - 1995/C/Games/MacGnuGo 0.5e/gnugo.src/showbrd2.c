/* */
#ifdef __THINK__
#define MAC
#endif
#ifdef MAC
#include <console.h>
#endif

#include <stdio.h>

#define EMPTY 0
#define WHITE 1
#define BLACK 2
#define NEUTRAL_TERR    3

extern unsigned char p[19][19];
extern int mymove, umove;
extern int blackCaptured, whiteCaptured;
extern int MAXX, MAXY;
extern int pass;
extern int hcap2;
extern int moveNumber, atari;
extern unsigned char ownermat[19][19];
static int display_inited = 0;

showboard(mx,my) int mx,my; { display(mx,my,0); }

display_mat() { display(-1,-1,1); }

display(mx,my,scoreflag)
int mx,my;
int scoreflag;
{
  int r;
  if (!display_inited) { display_inited = 1; clear(); }
  gotoxy(0,0);
  displaytop();
  for (r=0;r<MAXX;r++) {
    displayrow(r,scoreflag);
    if (r == 2) {
      if (my > 7) my++;
      if (mx >= 0 && pass < 2) {
	   printf("     My move: %c%d ",'a' + my,MAXX - mx);
           if (atari) printf(" Atari!");
           else printf("        ");
      }
      else printf("     I pass        ");
    }
    if (r == 3) {
      printf("     Your color: ");
      if (umove == 1) printf(" White O");
      else printf(" Black #");
    }
    if (r == 4) {
      printf("     My color:   ");
      if (mymove == 1) printf(" White O");
      else printf(" Black #");
    }
    if (r == 5) {
      printf("     Black has captured %d pieces  ", whiteCaptured);
    }
    if (r == 6) {
      printf("     White has captured %d pieces  ", blackCaptured);
    }
    if (r == 7) {
      printf("     handicap %d pieces  ", hcap2);
    }
    printf("\n");
  }
  displaybottom();
}

displaytop()
{
  int i;
  char c;
  printf("   ");
  for (i=0;i<MAXX;i++) {
    c = i + 'a';
    if (c >= 'i') c++;
    putchar(c); putchar(' ');
  }
  putchar('\n');
}

displaybottom() { displaytop(); }

displayrow(r,scoreflag)
int r;
int scoreflag;
{
  int i;
  printf("%2d ",MAXY - r);
  for (i=0;i<MAXX;i++) {
    if(p[r][i] == BLACK) putchar('#');
    else if(p[r][i] == WHITE) putchar('O');
    else if (scoreflag) {
      if(ownermat[r][i] == BLACK) putchar('B');
      else if(ownermat[r][i] == WHITE) putchar('W');
      else displayempty(r,i);
    }
    else displayempty(r,i);
    if (i<MAXX-1) putchar(' ');
  }
  printf(" %2d",MAXY - r);
}

displayempty(r,c)
int r,c;
{
  int m,center;
  m = (MAXX < 13) ? 2 : 3;
  center = MAXX / 2;
  if      ( (r == m || r == center || r == MAXX - m - 1) &&
	    (c == m || c == center || c == MAXX - m - 1) )
    putchar('+');
  else
    putchar('.');
}

clear()
{
#ifdef VT100
  fputs('\033[H',stdout);
  fputs('\033[J',stdout); fflush(stdout);
#endif
#ifdef MAC
  ccleos(stdout);
#endif
  return;
}

gotoxy(int r, int c)
{
#ifdef VT100
  putchar('\033'); putchar('[');
  if (r >= 10) putchar(r/10 + '0'); putchar(r%10 + '0');
  putchar(';');
  if (c >= 10) putchar(c/10 + '0'); putchar(c%10 + '0');
  putchar('H');
#endif
#ifdef MAC
  if (Button()) { printf("paused "); getchar(); }
  cgotoxy(c+1,r+1,stdout);
#endif
  return;
}

/* */
