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
 * This file contains routines for handling compiler warnings.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment Warnings

void
PrintWarningText(enum Ecc_Warnings whichone)
{
    switch (whichone) {
	case WARN_emptystatement:
	UserMesg("Empty expression stmt");
	break;
    case WARN_multicharconstant:
	UserMesg("Multi-character constant");
	break;
    case WARN_redundantcast:
	UserMesg("Redundant cast");
	break;
    case WARN_floateqcompare:
	UserMesg("Equivalence test of floating type");
	break;
    case WARN_discardfuncresult:
	UserMesg("Discarded function result");
	break;
    case WARN_assignif:
	UserMesg("Assignment as if conditional");
	break;
    case WARN_nonvoidreturn:
	UserMesg("Non-void function has no return statement");
	break;
    case WARN_constantif:
	UserMesg("Constant expression as if condition");
	break;
    case WARN_comparepointerint:
	UserMesg("Comparison of pointer and integer");
	break;
    case WARN_pointernonequivassign:
	UserMesg("Assignment of nonequivalent type to a pointer");
	break;
    case WARN_pointernonequivreturn:
	UserMesg("Return of nonequivalent type to a pointer");
	break;
    case WARN_pointernonequivarg:
	UserMesg("Pass of nonequivalent type to a pointer");
	break;
    case WARN_constantswitch:
	UserMesg("Constant expression as switch condition");
	break;
    case WARN_constantwhile:
	UserMesg("Constant expression as while loop condition");
	break;
    case WARN_constantdowhile:
	UserMesg("Constant expression as do-while loop condition");
	break;
    case WARN_constantfor:
	UserMesg("Constant expression as for loop condition");
	break;
    case WARN_nonintegralswitch:
	UserMesg("Switch expression not of integral type.");
	break;
    case WARN_novolatile:
	UserMesg("Volatile is not handled by this compiler");
	break;
	/* Missing warnings here AVAILABLE1 */
    case WARN_unusedvariable:
	UserMesg("Unused variable: ");
	break;
    case WARN_deadcode:
	UserMesg("Dead code");
	break;
    case WARN_implicitdecl:
	UserMesg("Implicit decl : ");
	break;
    case WARN_preprocredef:
	UserMesg("Re#definition : ");
	break;
    case WARN_nestedcomment:
	UserMesg("Possible nested comment");
	break;
	/* Missing warnings here AVAILABLE2 */
    case WARN_goto:
	UserMesg("goto statement found");
	break;
    case WARN_multireturn:
	UserMesg("Multiple function returns");
	break;
    case WARN_emptycompound:
	UserMesg("Empty compound statement");
	break;
    case WARN_missingreturntype:
	UserMesg("Missing function return type - default to int");
	break;
    case WARN_trigraphs:
	UserMesg("Trigraph found");
	break;
	/* Missing warnings here AVAILABLE3 */
    case WARN_pascal:
	UserMesg("pascal keyword found");
	break;
	/* Missing warnings here AVAILABLE4 */
    case WARN_semiafterfunction:
	UserMesg("Semicolon after function body");
	break;
    default:
	UserMesg("Unimplemented warning");
	break;
    }
}

void
GetWarningText(char *buf, int whichone)
{
    switch (whichone) {
	case WARN_emptystatement:
	strcat(buf,"Empty expression stmt");
	break;
    case WARN_multicharconstant:
	strcat(buf,"Multi-character constant");
	break;
    case WARN_redundantcast:
	strcat(buf,"Redundant cast");
	break;
    case WARN_floateqcompare:
	strcat(buf,"Equivalence test of floating type");
	break;
    case WARN_discardfuncresult:
	strcat(buf,"Discarded function result");
	break;
    case WARN_assignif:
	strcat(buf,"Assignment as if conditional");
	break;
    case WARN_nonvoidreturn:
	strcat(buf,"Non-void function has no return statement");
	break;
    case WARN_constantif:
	strcat(buf,"Constant expression as if condition");
	break;
    case WARN_comparepointerint:
	strcat(buf,"Comparison of pointer and integer");
	break;
    case WARN_pointernonequivassign:
	strcat(buf,"Assignment of nonequivalent type to a pointer");
	break;
    case WARN_pointernonequivreturn:
	strcat(buf,"Return of nonequivalent type to a pointer");
	break;
    case WARN_pointernonequivarg:
	strcat(buf,"Pass of nonequivalent type to a pointer");
	break;
    case WARN_constantswitch:
	strcat(buf,"Constant expression as switch condition");
	break;
    case WARN_constantwhile:
	strcat(buf,"Constant expression as while loop condition");
	break;
    case WARN_constantdowhile:
	strcat(buf,"Constant expression as do-while loop condition");
	break;
    case WARN_constantfor:
	strcat(buf,"Constant expression as for loop condition");
	break;
    case WARN_nonintegralswitch:
	strcat(buf,"Switch expression not of integral type.");
	break;
    case WARN_novolatile:
	strcat(buf,"Volatile is not handled by this compiler");
	break;
	/* Missing warnings here AVAILABLE1 */
    case WARN_unusedvariable:
	strcat(buf,"Unused variable: ");
	break;
    case WARN_deadcode:
	strcat(buf,"Dead code");
	break;
    case WARN_implicitdecl:
	strcat(buf,"Implicit decl : ");
	break;
    case WARN_preprocredef:
	strcat(buf,"Re#definition : ");
	break;
    case WARN_nestedcomment:
	strcat(buf,"Possible nested comment");
	break;
	/* Missing warnings here AVAILABLE2 */
    case WARN_goto:
	strcat(buf,"goto statement found");
	break;
    case WARN_multireturn:
	strcat(buf,"Multiple function returns");
	break;
    case WARN_emptycompound:
	strcat(buf,"Empty compound statement");
	break;
    case WARN_missingreturntype:
	strcat(buf,"Missing function return type - default to int");
	break;
    case WARN_trigraphs:
	strcat(buf,"Trigraph found");
	break;
	/* Missing warnings here AVAILABLE3 */
    case WARN_pascal:
	strcat(buf,"pascal keyword found");
	break;
	/* Missing warnings here AVAILABLE4 */
    case WARN_semiafterfunction:
	strcat(buf,"Semicolon after function body");
	break;
    default:
	strcat(buf,"Unimplemented warning");
	break;
    }
}
