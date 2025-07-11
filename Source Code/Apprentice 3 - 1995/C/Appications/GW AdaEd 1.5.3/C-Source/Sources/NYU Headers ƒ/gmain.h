/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

void fold_lower(char *);
void user_error(char *);
void user_info(char *);

#ifndef _ifile_h
#include	"ifile.h"
#endif

#ifdef MAC_GWADA
extern fabIFILE *LIBFILE;
extern fabIFILE *TREFILE;
#else
extern IFILE *LIBFILE;
extern IFILE *TREFILE;
#endif


extern IFILE	*AISFILE, *AXQFILE, *STUBFILE;
extern int list_unit_0; /* set by gmain.c to list unit 0 structure */
extern int peep_option;
