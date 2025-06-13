/*
                GNU GO - the game of Go (Wei-Chi)
                Version 1.1   last revised 3-1-89
           Copyright (C) Free Software Foundation, Inc.
                      written by Man L. Li
                      modified by Wayne Iba
                    documented by Bob Webber
		mac port by Ron Nicholson - 93Oct
*/
/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation - version 1.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License in file COPYING for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#define EMPTY 0
#define WHITE 1
#define BLACK 2

unsigned char p[19][19], l[19][19], ma[19][19], ml[19][19];
int mymove, umove;
int rd, lib, play, pass;
int mik, mjk, uik, ujk, mk, uk;  /* piece captured */
int opn[9];  /* opening pattern flag */

/* added from NextGo */
int MAXX, MAXY, blackCapturedKoI, blackCapturedKoJ, whiteCapturedKoI,
  whiteCapturedKoJ, blackCaptured, whiteCaptured,
  blackStones, whiteStones, currentStone, opposingStone, blackPassed, whitePassed;
int blackTerritory, whiteTerritory;

unsigned char capturedmat[19][19];
int mx, my;
int hcap;
int uflag;
int size = 12;

initstuff()
{
   int i, j;

/* init opening pattern numbers to search */
   for (i = 0; i < 9; i++)
        opn[i] = 1;
   opn[4] = 0;

/* init board */
   for (i = 0; i < 19; i++)
     for (j = 0; j < 19; j++) { p[i][j] = EMPTY; capturedmat[i][j] = 0; }
          
/* init global variables */
   mk = 0;  uk = 0;
   blackCaptured = 0, whiteCaptured = 0, 
   blackTerritory = 0; whiteTerritory = 0;
   blackStones = whiteStones = 0;

   play = 1;
   pass = 0;
   mik = -1; mjk = -1;
   uik = -1; ujk = -1;
   seed(&rd);	/* start random number seed */
   mx = my = -1;
}

setcolor(int color)
{
      if (color != WHITE) {
         mymove = WHITE;   /* computer white */
         umove = BLACK;   /* human black */
      } else {
         mymove = BLACK;   /* computer black */
         umove = WHITE;   /* human white */
      }
      currentStone = mymove; opposingStone = umove; 
}

init4(side)
int side;
{
   int i, j;
   char move[10], ans[5];
   
   if (MAXX < 5) MAXY = MAXX = 5;
   if (MAXX > 19) MAXY = MAXX = 19;
   if (hcap < 1) hcap = 0;
   if (hcap > MAXX) hcap = MAXX;
   
   initstuff();
   sethand(hcap);
   setcolor(side);
   initHistory();
  
   if (uflag)  /* start game */
     {
     if (side == 2) /* black */
        {
         if (hcap != 0)
	   {
            genmove(&i, &j);   /* computer move */
	        mx = i; my = j;
            p[i][j] = mymove;
            addHistory(WHITE,i,j);
          }
       }
      else
        {
        if (hcap == 0)
	   {
            genmove(&i, &j);   /* computer move */
	        mx = i; my = j;
            p[i][j] = mymove;
            addHistory(BLACK,i,j);
          }
       }
    }
}

score_game_c()
{
  int i,j;
  score_game();
  blackStones = whiteStones = 0;
  for (i=0;i<MAXX;i++) for (j=0;j<MAXY;j++) {
    if (p[i][j] == BLACK) blackStones++;
    if (p[i][j] == WHITE) whiteStones++;
  }
}  

movenplay(x,y)
int x,y;
{
  int i, j;
  i = x; j = y;
  if (i >= 0)   /* not pass */ {
	p[i][j] = umove; pass = 0;
	if (examboard(mymove)) RedrawScreen();	 /* remove my dead pieces */
  } else pass++;
  addHistory(opposingStone,i,j);
  if (pass == 2)
    score_game_c();
  else if (uflag == 0) {
    if (umove == WHITE)  setcolor(BLACK);
    else setcolor(WHITE);
    mx = i; my = j;
  } else {
	genmove(&i, &j);   /* computer move */
	mx = i; my = j;
	if (i >= 0)   /* not pass */ {
	  p[i][j] = mymove; pass = 0;
	} else pass++;
	examboard(umove);   /* remove your dead pieces */
	addHistory(currentStone,i,j);
  }
  if (pass == 2) score_game_c();
}

on_new() { init4(uflag); resize(); }

do_pass() { if (pass < 2) movenplay(-1,-1); RedrawScreen(); }

on_black() { uflag = BLACK; setcolor(uflag); refresh(); }
on_white() { uflag = WHITE; setcolor(uflag); refresh(); }
on_both()  { uflag = 0; }
on_bigger()  { MAXX = MAXY += 1+(MAXX > 8); /* */ on_new(); }
on_smaller() { MAXX = MAXY -= 1+(MAXX > 9); /* */ on_new(); }
on_more() { hcap++; on_new(); }
on_less() { hcap--; on_new(); }
void on_undo() { if (uflag) undo(2); else undo(1); refresh(); }
on_randomize(int flag) { return; }

extern char *AboutMenu;
extern char *AboutText;

main()
{
  MAXX = MAXY = 9;
  uflag = 1; hcap = 2;
  
  AboutMenu = "About GnuGo...;-";
  macinit("GnuGo", 2*size*(MAXX+2) + 120, 2*size*(MAXX+2));
  AboutText = "GnuGo V0.5e 93Nov17 ©1993 FSF";
  
  init4(uflag);
  while(1) {
    DoEvent();
  }
}

/*
 * current bugs:
 *   flicker on refresh
 * added:
 *   mark latest move, fiot, undo, show scored board
 *   play both sides, play 5x5, undo game over, chinese scoring, captured keepout
 * fixed:
 *   crash on zero handicap, suicide moves
 */
