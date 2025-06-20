/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

#ifndef _ifile_h
#include "ifile.h"
#endif

#ifndef __slot_h
#include "slot.h"
#endif

Segment segment_new(short, int);
Segment fab_segment_read(fabIFILE *file);
Segment segment_read(IFILE *);
void fab_read_axq(fabIFILE *axq_file, Axq axq);
void read_axq(IFILE *, Axq);
long fab_get_cde_slots(fabIFILE *file, Axq axq);
long get_cde_slots(IFILE *, Axq);
