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
#else
extern IFILE *LIBFILE;
#endif


extern IFILE	*AISFILE, *AXQFILE, *STUBFILE, *TREFILE;
