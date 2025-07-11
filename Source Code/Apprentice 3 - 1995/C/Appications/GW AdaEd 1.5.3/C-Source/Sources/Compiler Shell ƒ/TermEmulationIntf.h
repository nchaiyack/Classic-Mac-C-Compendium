/* TermEmulationIntf.h
 *
 * Public interface to the Terminal Emulation package for GW Ada
 *
 * Manuel A. Perez
 * 9/3/93
 */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

#include <stdio.h>

/* ASCII control characters */

#define  BELL 0x07
#define  BS   0x08
#define  HT   0x09
#define  LF   0x0a
#define  FF   0x0c
#define  CR   0x0d
#define  ESC  0x1b

		/* Variables - scrBuffer can be used for
		 * temporary string storage.  The scr
		 * module doesn't use it...
		 */
extern	char	scrBuffer[300];

		/* Routines */
char	scrGetc(void);
void	scrPutc(char c);
void	scrPutStr(char *str);

WindowPtr	scrCreateWindow(short font, short size/*,	short rows, short cols*/);

void	scrCopyToFile(FILE *fp);
void	scrDrawScreen(void);
void	scrClearScreen(void);
void	scrGotoXY(short r, short c);
