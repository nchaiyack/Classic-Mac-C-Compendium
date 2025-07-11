/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

#ifndef __slot_h
#include	"slot.h"
#endif

void initialize_1(void);
void initialize_2(void);
int max_index(int);
Slot slot_new(Symbol, int);
void remove_slots(Tuple, int);
void remove_interface(Tuple tup, int);
void private_exchange(Symbol);
void private_install(Symbol);
int init_slots(int);

extern Segment	CODE_SEGMENT, DATA_SEGMENT, DATA_SEGMENT_MAIN;
extern Segment FIELD_TABLE, VARIANT_TABLE;
extern Tuple units_in_compilation;
