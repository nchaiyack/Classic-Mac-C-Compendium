#include "comment.header"

#define EMPTY 0
#define WHITE 1
#define BLACK 2

extern unsigned char p[19][19], l[19][19], ma[19][19], ml[19][19];
extern int mymove, umove;
extern int rd, lib, play, pass;
extern int mik, mjk, uik, ujk, mk, uk;  /* piece captured */
extern int opn[9];  /* opening pattern flag */

/* added from NextGo */
extern int MAXX, MAXY, blackCapturedKoI, blackCapturedKoJ, whiteCapturedKoI,
  whiteCapturedKoJ, blackCaptured, whiteCaptured,
  blackStones, whiteStones, currentStone, opposingStone, blackPassed, whitePassed;
extern int blackTerritory, whiteTerritory;

extern unsigned char capturedmat[19][19];
extern int mx, my, atari;
extern int hcap;
extern int uflag;
extern int lastMove;

unsigned char op[19][19];
extern unsigned char ownermat[19][19];
extern int size;

drawterritory(x,y,color)
int x,y,color;
{
  x = 2*size*(x) + 1 + (3*size)/2;
  y = 2*size*(y) - 2 + (5*size)/2;
  MoveTo(x,y);
  TextFont(1); TextSize(12); TextMode(srcOr);
  if (color == WHITE) DrawChar('W');
  if (color == BLACK) DrawChar('B');
}

drawstone(x,y,color)
int x,y,color;
{
  Rect r,r2;
  x = 2*size*(x);
  y = 2*size*(y);
  SetRect(&r,x+size,y+size,x+3*size,y+3*size);
  if (color == 1) {
    FillOval(&r,white);
    FrameOval(&r);
  } else if (color == 2) {
    /* FillOval(&r,dkGray); */
    FillOval(&r,black);
      if (size > 7) SetRect(&r2,x+3*size/2-1,y+3*size/2-1,x+2*size-3,y+2*size-3);
      else SetRect(&r2,x+3*size/2,y+3*size/2,x+2*size-2,y+2*size-2);
      FillOval(&r2,white);
    FrameOval(&r);
  } else if (color == -1) {
    FillRect(&r,white);
  }
}

markStone(x,y,color)
int x,y,color;
{
  Rect r;
  if (x < 0) return;
  x = 2*size*(x);
  y = 2*size*(y);
  SetRect(&r,x+3*size/2+1,y+3*size/2+1,x+5*size/2-1,y+5*size/2-1);
  FillRect(&r,ltGray);
  if (color == WHITE) FrameRect(&r);  
}

RedrawScreen()
{
  int i,j;
  char buf[128];
  int slin = 20;
  
  for (i=0;i<MAXX;i++)
    for (j=0;j<MAXX;j++) {
      if (p[i][j] != op[i][j]) { drawstone(i,j,-1); op[i][j] = p[i][j]; }
    }
  for (i=0;i<MAXX;i++) {
    MoveTo(2*size,2*size+i*2*size); LineTo(2*size+(MAXX-1)*2*size,2*size+i*2*size);
    MoveTo(2*size+i*2*size,2*size); LineTo(2*size+i*2*size,2*size+(MAXX-1)*2*size);
  }
  for (i=0;i<MAXX;i++) for (j=0;j<MAXX;j++) {
    if (p[i][j] != 0)   drawstone(i,j,p[i][j]);
    else if (pass >= 2) drawterritory(i,j,ownermat[i][j]);
  }
  if (mx >= 0 && p[mx][my] != 0) markStone(mx,my,currentStone);
    
  TextFont(4); TextSize(9); TextMode(srcCopy);
  MoveTo(2*size*(MAXX+2),2*slin);
  if (atari) DrawCStr("Atari! ");
  else DrawCStr("GnuGo  ");
  DrawNum(MAXX); DrawChar('x'); DrawNum(MAXY);
  MoveTo(2*size*(MAXX+2),3*slin);
  i = mx; if (i > 7) i++;
  if (i >= 0 && pass < 2) {
    DrawCStr("My move: "); DrawChar('A'+i); DrawNum(MAXY - my); DrawCStr("   ");
  }
  else  DrawCStr("I pass      ");
  MoveTo(2*size*(MAXX+2),4*slin);
  if (opposingStone == BLACK) DrawChar('>');
  DrawCStr("black "); DrawNum((pass < 2) ? whiteCaptured : blackTerritory + blackStones);
  DrawCStr("    ");
  MoveTo(2*size*(MAXX+2),5*slin);
  if (opposingStone == WHITE) DrawChar('>');
  DrawCStr("white "); DrawNum((pass < 2) ? blackCaptured : whiteTerritory + whiteStones);
  DrawCStr("    ");
  MoveTo(2*size*(MAXX+2),6*slin); 
  if (pass >= 2) { DrawCStr("game over    "); }
  else { DrawCStr("move "); DrawNum(lastMove-2*hcap+(hcap>0)); DrawCStr("   "); }
  MoveTo(2*size*(MAXX+2),7*slin); 
  if (hcap > 0) { DrawCStr("handicap "); DrawNum(hcap); DrawCStr(" "); }
  return;
}

extern void Idle(int t);

clicktask(x,y,m) short x; short y; short m;
{
  x = ((x-10)/(2*size)); if (x < 0) x = 0; if (x >= MAXX) x = MAXX-1;
  y = ((y-10)/(2*size)); if (y < 0) y = 0; if (y >= MAXY) y = MAXY-1;
  if (p[x][y] != 0 || suicide(x,y) || pass >= 2) { MyBeep(1); return; }
  op[x][y] = umove; if (uflag) drawstone(x,y,umove);
  if (pass < 2) movenplay(x,y);
  /* Idle(30); */
  RedrawScreen();
  return; 
}

extern WindowPtr myWindow;
extern int nWidth, nHeight;

refresh() { Rect r; r = myWindow->portRect; EraseRect(&r); RedrawScreen(); }

resize()
{
  size = 7;
  if (MAXX <= 17) size = 8;
  if (MAXX <= 13) size = 10;
  if (MAXX <= 11) size = 12;
  if (MAXX <=  7) size = 18;
  SizeWindow(myWindow,nWidth = 2*size*(MAXX+2) + 120,
                      nHeight = 2*size*(MAXX+1) + (size/2) + 4, -1);
  refresh();
}

/* end */