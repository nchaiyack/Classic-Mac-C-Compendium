/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

/* libf - auxiliary procedures for reading in IFILE format files.
 * This is subset of procedures needed to read files generated in
 * library format without the need all the library primitives.
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef _STRING
#include <string.h>
#endif

#include "config.h"
#include "ifile.h"
#include "misc.h"
#include "libf.h"


long fab_read_init(fabIFILE *ifile)
{
/* initialize read, position at start of first record, return
 * offset of next record. return 0 if no first first record.
 * read first word in file that may have offset to slot info.
 */

long  pos;

if (sizeof(long) != fab_fread((char *) &pos, sizeof(long), ifile->fh_file)) {
	chaos(__FILE__, __LINE__, "read_init read failed ");
	pos = 0;
	}
return pos;
}

long fab_read_next(fabIFILE *ifile, long p)
{
long  pos;

fab_ifseek(ifile/*, "next-unit"*/, p);
if (p == ifile->fh_units_end) {
	pos = 0; /* if at end */
	}
else if (sizeof(long) != fab_fread((char *) &pos, sizeof(long), ifile->fh_file)) {
	chaos(__FILE__, __LINE__, "read_next read failure");
	pos = 0;
	}
return pos;
}

