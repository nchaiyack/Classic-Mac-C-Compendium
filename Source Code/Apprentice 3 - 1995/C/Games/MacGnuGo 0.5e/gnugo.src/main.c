/*
undo(int i) { return; }
saveSmartGoFile(char* i) { return; }
initHistory() { return; }
addHistory(int i, int j, int k) { return; }
*/
/*
                GNU GO - the game of Go (Wei-Chi)
                Version 1.1   last revised 3-1-89
           Copyright (C) Free Software Foundation, Inc.
                      written by Man L. Li
                      modified by Wayne Iba
                    documented by Bob Webber
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

Please report any bug/fix, modification, suggestion to

mail address:   Man L. Li
                Dept. of Computer Science
                University of Houston
                4800 Calhoun Road
                Houston, TX 77004

e-mail address: manli@cs.uh.edu         (Internet)
                coscgbn@uhvax1.bitnet   (BITNET)
                70070,404               (CompuServe)
*/

#include <stdio.h>

#define EMPTY 0
#define WHITE 1
#define BLACK 2

unsigned char p[19][19], l[19][19], ma[19][19], ml[19][19];
int mymove, umove;
long int rd;
int lib, play, pass;
int mik, mjk, uik, ujk, mk, uk;  /* piece captured */
int opn[9];  /* opening pattern flag */

/* added from NextGo */
int MAXX, MAXY, blackCapturedKoI, blackCapturedKoJ, whiteCapturedKoI,
 whiteCapturedKoJ, blackCaptured, whiteCaptured, currentStone, 
 opposingStone, blackPassed, whitePassed;

int blackTerritory, whiteTerritory;
int blackStones, whiteStones;
int mx, my;
int hcap2, hcap;
unsigned char capturedmat[19][19];
int rflag = 0;
extern int lastMove;
int randomize_flag = 0;

void initstuff()
{
   int i, j;
/* init opening pattern numbers to search */
      for (i = 0; i < 9; i++)
        opn[i] = 1;
      opn[4] = 0;

/* init board */
      for (i = 0; i < 19; i++)
        for (j = 0; j < 19; j++)
          capturedmat[i][j] = p[i][j] = EMPTY;

/* init global variables */
   mk = 0;  uk = 0;
   blackCaptured = 0; whiteCaptured = 0;

/* init global variables */
   play = 1;
   pass = 0;
   mik = -1; mjk = -1;
   uik = -1; ujk = -1;
   seed(&rd);   /* start random number seed */
}

main(argc, argv)  int argc;  char *argv[];
{
   FILE *fp;
   int i, j;
   char ans[5];
   char *fname;
   int cont = 0;
   int argn = 1;

   if (argc > argn && argv[argn][0] > '9') fname = argv[argn++];
   else fname = NULL;

/* show instruction */
   /* showinst(); */
   printf("GnuGo, Copyright 1989,1993 Free Software Foundation, Inc.\n");

/* ask for board size */
   if (argc > argn) sscanf(argv[argn++],"%d", &i);
   else {
      printf("board size (5 to 19)? ");
      scanf("%d", &i);
      getchar();
   }
   if (i < 5) i = 5; if (i > 19) i = 19;
   MAXX = MAXY = i;

   initstuff();

   if (1)  /* new game */ {
/* ask for handicap */
      if (argc > argn) sscanf(argv[argn++],"%d", &hcap2);
      else {
        printf("\nNumber of handicap for black (0 to 17)? ");
        scanf("%d", &hcap2);
        getchar();
      }

/* choose color */
      if (argc > argn) sscanf(argv[argn++],"%c", ans);
      else {
        printf("\nChoose side(b or w)? ");
        scanf("%c",ans);
        getchar();
        printf("\n");
      }

      if (argc > argn) {
        if (strcmp(argv[argn], "rand") == 0) { argn++; rflag = 1; }
        else if (strcmp(argv[argn], "rand2") == 0) { argn++; rflag = 2; }
        else if (strcmp(argv[argn], "eval3") == 0) { argn++; rflag = 3; }
      }
      if (hcap2 < 0) hcap2 = 0; hcap = hcap2; if (hcap > MAXX) hcap = MAXX;
      if (hcap > 17) hcap = 17;
      if (rflag > 0 && MAXX > 8) { hcap = hcap > 4 ? 4 : 0; }
      sethand(hcap);
      initHistory();

      if (ans[0] == 'b') {
         mymove = WHITE;   /* computer white */
         umove = BLACK;   /* human black */
         currentStone = mymove; opposingStone = umove; 
         if (hcap != 0 && hcap2 == hcap) {
            genmove(&i, &j);   /* computer move */
            p[i][j] = mymove;
	    addHistory(WHITE,i,j);
         }
      } else {
         mymove = BLACK;   /* computer black */
         umove = WHITE;   /* human white */
         currentStone = mymove; opposingStone = umove; 
         if (hcap == 0) {
            genmove(&i, &j);   /* computer move */
            p[i][j] = mymove;
	    addHistory(BLACK,i,j);
         }
      }
    }

/* display game board */

   showboard(-1,-1);

/* main loop */
   while (play > 0)
     {
      if (currentStone == BLACK && hcap2-1 > hcap) { i = j = -1; hcap++; }
      else getmove(&i, &j);   /* read human move */
      addHistory(opposingStone,i,j);
      if (play > 0)
	{
	 if (i >= 0) {  /* not pass */
	    p[i][j] = umove;
	    pass = 0;
	 }
	 examboard(mymove);	 /* remove my dead pieces */
	 if (pass != 2)
	   {
	    if (currentStone == WHITE && hcap2-1 > hcap)
	      { i = j = -1; hcap++; }
	    else genmove(&i, &j);   /* computer move */
            addHistory(currentStone,i,j);
	    mx = i; my = j;
	    if (i >= 0) {   /* not pass */
	       p[i][j] = mymove;
	       pass = 0;
	    } else pass++;
	    examboard(umove);   /* remove your dead pieces */
#ifdef MAC
            if (rflag && Button() ) getchar();
#endif
	  }
	 showboard(mx,my);
	 /* printf(" %d pass = %d   ",lastMove,pass); */
       }
      if (pass >= 2 || (rflag > 0 && lastMove > 3*MAXX*MAXY/2))
	play = 0;	/* both pass then stop game */
    }

 if (play == 0)
   {
/* finish game and count pieces */
    ans[0] = 'y';
    /*
    printf("Do you want to count score (y or n)? ");
    scanf("%c",ans);
    */
    /* if (ans[0] == 'y') endgame(); */
    if (ans[0] != 'n' && ans[0] != 'N') {
      score_game_c();
      display_mat();
      printf("j; black=%3d  white=%3d\n", blackTerritory + whiteCaptured, 
              whiteTerritory + blackCaptured);
      i = blackTerritory + blackStones; j = whiteTerritory + whiteStones;
      printf("%dx%d, %c, h=%d, black=%3d  white=%3d, m=%d, gnu=%c, %d \n",
	MAXX, MAXY, (i>j) ? 'b' : 'w', hcap2, i, j, lastMove,
	(currentStone == BLACK) ? 'b' : 'w', rflag);
    }
    if (fname != NULL) saveSmartGoFile(fname);
  }
}
/* end main */

score_game_c()
{
  int i,j;
  score_game();
  blackStones = whiteStones = 0;
  for (i = 0; i < MAXX; i++) for (j = 0; j < MAXY; j++) {
    if (p[i][j] == BLACK) blackStones++;
    if (p[i][j] == WHITE) whiteStones++;
  }
}
