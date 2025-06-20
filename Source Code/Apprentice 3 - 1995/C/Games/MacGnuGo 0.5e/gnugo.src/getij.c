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

extern int MAXX,MAXY;

getij(move, i, j)
/* convert string input to i, j coordinate */
char move[];
int *i, *j;
{
 int k,n;

 n = 0;
 while (move[n] == ' ') n++;
 if ((move[n] >= 65) && (move[n] <= 72))
    *j = move[n] - 65;
 else
    if ((move[n] >= 74) && (move[n] <= 84))
       *j = move[n] - 66;
    else
       if ((move[n] >= 97) && (move[n] <= 104))
	  *j = move[n] - 97;
       else
	  if ((move[n] >= 106) && (move[n] <= 116))
	     *j = move[n] - 98;
	  else
	     return 0;
 n++;
 while (move[n] == ' ') n++;
 if (move[n]) k = move[n] - 48;
 else return 0;
 n++;
 if (move[n]) k = k * 10 + move[n] - 48;
 *i = MAXX - k;
 if ((*i >= 0) && (*i < MAXX) && (*j >= 0) && (*j < MAXY))
    return 1;
 else
    return 0;
}  /* end getij */
