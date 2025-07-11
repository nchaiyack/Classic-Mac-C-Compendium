/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains utility routines for the 68k code generator
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"

#pragma segment GenBLD


LabSYMVia_t
AddLabel(char *name)
/* Adds the name given to the label list. Returns the label record. */
{
    LabSYMVia_t                     lab = NULL;
    lab = LabTableSearch(Labels, name);
#ifdef Undefined
    assert(!lab);
#endif
    if (!lab) {
	lab = LabTableAdd(Labels, name);
    }
    return lab;
}

LabSYMVia_t
MakeSysLabel(char *name)
{
    return AddLabel(name);
}

LabSYMVia_t
MakeUserLabel(char *name)
/*
 * Takes a name, prepends an underscore, adds the name to the label list, and
 * returns the label record.
 */
{
    return AddLabel(name);
}

LabSYMVia_t
MakeFloatLitLabel(int num)
/*
 * Given a float literal number, generates a string literal label of the form
 * FLOATnum, adds it to the label list, and returns the label record.
 */
{
    char                            lab[25];
    char                            lab2[25];
    strcpy(lab2, "F");
    sprintf(lab, "%d", num);
    strcat(lab2, lab);
    return AddLabel(lab2);
}

LabSYMVia_t
MakeLitLabel(int num)
/*
 * Given a string literal number, generates a string literal label of the
 * form LITnum, adds it to the label list, and returns the label record.
 */
{
    char                            lab[25];
    char                            lab2[25];
    strcpy(lab2, "S");
    sprintf(lab, "%d", num);
    strcat(lab2, lab);
    return AddLabel(lab2);
}

LabSYMVia_t
MakeEccLabel(int num)
/*
 * Given an ecc label number, generates a label of the form LABnum, adds it
 * to the label list, and returns the label record.  Ecc labels are generated
 * for a number of circumstances including loops, ifthenelse, switch, etc...
 */
{
    char                            lab[25];
    char                            lab2[25];
    strcpy(lab2, "L");
    sprintf(lab, "%d", num);
    strcat(lab2, lab);
    return AddLabel(lab2);
}
