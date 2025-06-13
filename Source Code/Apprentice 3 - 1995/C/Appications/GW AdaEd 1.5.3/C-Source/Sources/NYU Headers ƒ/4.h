/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
#ifndef _4_h
#define _4_h

#ifndef _hdr_h
#include "hdr.h"
#endif

#ifndef _libhdr_h
#include "libhdr.h"
#endif

#ifndef _vars_h
#include "vars.h"
#endif


/* TODO;


	REVIEW VALID_OP_TYPES  CAREFULLY!!

	slide_context (is global integer) 
	may_slide (is local, copy of slide_context

 Need to clarify attribute encoding, not strings ("LAST"), not
 symbols (symbol_last), and not even Rosen codes, but rather full
 set of integer codes covering all cases, and including Rosen codes
 as subset.
 The N_VAL(node) field of node has attribute name, often (but not 
always)
 interpreted as a string.
 */


typedef struct Triplet {
    int   inf;
    int   sup;
    Node  choice_node;
    }Triplet;

#endif
