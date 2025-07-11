 /* TermEmulation.h
 *
 * Private interface to the Terminal Emulation package for GW Ada
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

#include "TermEmulationIntf.h"

enum { stNONE, stESC, stCSI, stNUM1, stSEMI, stNUM2 };

// constants for the font settings
enum { ftNormal=0, ftUnderline=2, ftBlinking=4, ftReverse=8 };

// struct reordering for fastest 68K & PPC access [Fabrizio Oddone]
typedef struct {
	short	col, row;
	char	ch;
} Parms;

typedef Boolean (*CharCmp)(char ch);
typedef void (*ActionRtn)(Parms *p);

// struct reordering for fastest 68K & PPC access [Fabrizio Oddone]
typedef struct {
	short		state;
	short		gotoState;
	ActionRtn	action;
	char		chcmp[24];	// 25 is an arbitrary number => better a 4-even number [Fabrizio Oddone]
} State;


/* Internal Routines */
#define chANY NULL
#define	doNothing NULL
