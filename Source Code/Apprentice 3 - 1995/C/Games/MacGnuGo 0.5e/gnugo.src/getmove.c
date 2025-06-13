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

#define writeMGTexists
#ifdef writeMGTexists
extern char *fname;
#else
extern int mymove, umove;
extern int mk, uk;	/* piece captured */
extern int opn[9];
#endif

#define EMPTY 0

extern unsigned char p[19][19];
extern int play, pass;

static char fnameBuffer[128];
extern int rflag, opposingStone;
extern int randmove(int*,int*,int);

getmove(int *i, int *j)
/* interpret response of human move to board position */
{
   FILE *fp;
   int m, n;
   char move[128];

   if (rflag) { randmove(i, j, opposingStone); return; }
   move[0] = 0;
   printf("your move? ");
   gets(move);

   if      (strcmp(move, "stop") == 0) play = 0; /* stop game */
   else if (strcmp(move, "quit") == 0) play = 0; /* stop game */
   else if (strcmp(move, "exit") == 0) play = 0; /* stop game */
   else if (strncmp(move, "rand", 4) == 0) { rflag = 2; getmove(i, j); }
   else if (strcmp(move, "undo") == 0) {
     undo(2);
     showboard();
     getmove(i,j);
   }
   else if (strcmp(move, "save") == 0) {
#ifdef writeMGTexists
     fnameBuffer[0] = 0;
     if (scanf("%s",fnameBuffer) == 1) {
       saveSmartGoFile(fnameBuffer);
       printf("saved game to file %s\n",fnameBuffer);
     }
     getmove(i,j);
#else
/* save data and stop game */
	 fp = fopen("gnugo.dat", "w");
/* save board configuration */
	 for (m = 0; m < 19; m++)
	   for (n = 0; n < 19; n++)
	       fprintf(fp, "%c", p[m][n]);
/* my color, pieces captured */
         fprintf(fp, "%d %d %d ", mymove, mk, uk);
/* opening pattern flags */
         for (m = 0; m < 9; m++)
           fprintf(fp, "%d ", opn[m]);

	 fclose(fp);
	 play = -1;
#endif
     }
     else if (strcmp(move, "pass") == 0 || move[0] == 0) { /* human pass */
	    pass++;
	    *i = -1;   /* signal pass */
     }
     else {
	  pass = 0;
		/* move[0] from A to T, move[1] move[2] from 1 to 19 */
		/* convert move to coordinate */
	  if (!getij(move, i, j) || (p[*i][*j] != EMPTY) || suicide(*i, *j))
	  {
	       printf("illegal move !\n");
	       getmove(i, j);
          }
     }
}  /* end getmove */
