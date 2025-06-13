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

extern unsigned char p[19][19];
extern int mymove, umove;
extern int mk, uk;  /* piece captured */
extern int MAXX, MAXY;

showboard()
/* show go board */
  {
   int i, j, ii;

/* p = 0 for empty ,p = 1 for white piece, p = 2 for black piece */
   printf("   A B C D E F G H J K L M N O P Q R S T\n");
/* row 19 to 17 */
   for (i = 0; i < 3; i++)
     {
      ii = MAXY - i;
      printf("%2d",ii);

      for (j = 0; j < MAXY; j++)
	if (p[i][j] == EMPTY)
	   printf(" -");
	else if (p[i][j] == WHITE)
		printf(" O");
	     else printf(" X");

      printf("%2d",ii);
      printf("\n");
     }
/* row MAXX-3 */
   printf("%2d",MAXX-3);

   for (j = 0; j < 3; j++)
     if (p[3][j] == EMPTY)
	printf(" -");
     else if (p[3][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[3][3] == 0)
      printf(" +");
   else if (p[3][3] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 4; j < MAXX/2; j++)
     if (p[3][j] == EMPTY)
	printf(" -");
     else if (p[3][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[3][MAXX/2] == EMPTY)
      printf(" +");
   else if (p[3][MAXX/2] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 1+MAXX/2; j < MAXX-4; j++)
     if (p[3][j] == EMPTY)
	printf(" -");
     else if (p[3][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[3][MAXX-4] == EMPTY)
      printf(" +");
   else if (p[3][MAXX-4] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = MAXX-3; j < MAXY; j++)
     if (p[3][j] == EMPTY)
	printf(" -");
     else if (p[3][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   printf("%2d",MAXX-3);
   if (umove == 1)
      printf("     Your color: White O\n");
   else
      if (umove == 2)
	 printf("     Your color: Black X\n");
      else
	 printf("\n");
/* row MAXX-4 to 11 */
   for (i = 4; i < MAXX/2; i++)
     {
      ii = MAXY - i;
      printf("%2d",ii);

      for (j = 0; j < MAXY; j++)
	if (p[i][j] == EMPTY)
	   printf(" -");
	else if (p[i][j] == WHITE)
		printf(" O");
	     else printf(" X");

      printf("%2d",ii);
      if (i == 4)
	{
	 if (mymove == 1)
	    printf("     My color:   White O\n");
	 else
	    if (mymove == 2)
	       printf("     My color:   Black X\n");
	    else
	       printf("\n");
       }
      else
	 if (i != 8)
	    printf("\n");
	 else
	    printf("     You have captured %d pieces\n", mk);
     }
/* row 1+MAXX/2 */
   printf("%2d",1+MAXX/2);

   for (j = 0; j < 3; j++)
     if (p[MAXX/2][j] == EMPTY)
	printf(" -");
     else if (p[MAXX/2][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX/2][3] == EMPTY)
      printf(" +");
   else if (p[MAXX/2][3] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 4; j < MAXX/2; j++)
     if (p[MAXX/2][j] == EMPTY)
	printf(" -");
     else if (p[MAXX/2][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX/2][MAXX/2] == EMPTY)
      printf(" +");
   else if (p[MAXX/2][MAXX/2] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 1+MAXX/2; j < MAXX-4; j++)
     if (p[MAXX/2][j] == EMPTY)
	printf(" -");
     else if (p[MAXX/2][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX/2][MAXX-4] == EMPTY)
      printf(" +");
   else if (p[MAXX/2][MAXX-4] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = MAXX-3; j < MAXY; j++)
     if (p[MAXX/2][j] == EMPTY)
	printf(" -");
     else if (p[MAXX/2][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   printf("%2d",1+MAXX/2);
   printf("     I have captured %d pieces\n", uk);
/* row MAXX/2 to 5 */
   for (i = 1+MAXX/2; i < MAXX-4; i++)
     {
      ii = MAXY - i;
      printf("%2d",ii);

      for (j = 0; j < MAXY; j++)
	if (p[i][j] == EMPTY)
	   printf(" -");
	else if (p[i][j] == WHITE)
		printf(" O");
	     else printf(" X");

      printf("%2d",ii);
      printf("\n");
     }
/* row 4 */
   printf(" 4");

   for (j = 0; j < 3; j++)
     if (p[MAXX-4][j] == EMPTY)
	printf(" -");
     else if (p[MAXX-4][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX-4][3] == EMPTY)
      printf(" +");
   else if (p[MAXX-4][3] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 4; j < MAXX/2; j++)
     if (p[MAXX-4][j] == EMPTY)
	printf(" -");
     else if (p[MAXX-4][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX-4][MAXX/2] == EMPTY)
      printf(" +");
   else if (p[MAXX-4][MAXX/2] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = 1+MAXX/2; j < MAXX-4; j++)
     if (p[MAXX-4][j] == EMPTY)
	printf(" -");
     else if (p[MAXX-4][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   if (p[MAXX-4][MAXX-4] == EMPTY)
      printf(" +");
   else if (p[MAXX-4][MAXX-4] == WHITE)
	   printf(" O");
	else printf(" X");

   for (j = MAXX-3; j < MAXY; j++)
     if (p[MAXX-4][j] == EMPTY)
	printf(" -");
     else if (p[MAXX-4][j] == WHITE)
	     printf(" O");
	  else printf(" X");

   printf(" 4");
   printf("\n");
/* row 3 to 1 */
   for (i = MAXX-3; i < MAXY; i++)
     {
      ii = MAXY - i;
      printf("%2d",ii);

      for (j = 0; j < MAXY; j++)
	if (p[i][j] == EMPTY)
	   printf(" -");
	else if (p[i][j] == WHITE)
		printf(" O");
	     else printf(" X");

      printf("%2d",ii);
      printf("\n");
     }
   printf("   A B C D E F G H J K L M N O P Q R S T\n\n");
 }  /* end showboard */
