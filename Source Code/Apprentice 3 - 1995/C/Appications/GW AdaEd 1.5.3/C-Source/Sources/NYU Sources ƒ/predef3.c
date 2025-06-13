/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

/*    +---------------------------------------------------+
      |                                                   |
      |          I N T E R P     P R E D E F S            |
      |            Part 3: Utility Procedures             |
      |                  (C Version)                      |
      |                                                   |
      |   Adapted From Low Level SETL version written by  |
      |                                                   |
      |                  Monte Zweben                     |
      |               Philippe Kruchten                   |
      |               Jean-Pierre Rosen                   |
      |                                                   |
      |    Original High Level SETL version written by    |
      |                                                   |
      |                   Clint Goss                      |
      |               Tracey M. Siesser                   |
      |               Bernard D. Banner                   |
      |               Stephen C. Bryant                   |
      |                  Gerry Fisher                     |
      |                                                   |
      |              C version written by                 |
      |                                                   |
      |               Robert B. K. Dewar                  |
      |                                                   |
      +---------------------------------------------------+
*/

/*  This module contains routines for the implementation of some of
 *  the predefined Ada packages and routines, namely SEQUENTIAL_IO,
 *  DIRECT_IO, TEXT_IO, and CALENDAR. Part 3 contains utility routines
 *  for access to parameters on the stack and for returning values.
 */

#include <stdlib.h>

#include "config.h"
#include "ipredef.h"
#include "intc.h"
#include "predef.h"

#if	defined(IBM_PC) || defined(SUNOS5) || defined(MAC_GWADA)
#include <string.h>
#else
#include <strings.h>
#endif

/* GET_ARGUMENT_PTR */

/* Procedure to get argument address. The parameter is the offset from the
 * TOS to the base/offset entry. The corresponding pointer is returned.
 */

int *get_argument_ptr(int offset)			          /*;get_argument_addr*/
{
return ADDR(TOSM(offset + 1), TOSM(offset));
}


/* GET_STRING_VALUE */

/* Procedure to get argument value of type string. The parameter is the offset
 * from the TOS to the entry for the string (consisting of a descriptor and a
 * value. The string value is converted from internal Ada form to standard C
 * form and stored in work_string.
 */

void get_string_value(int offset)		               /*;get_string_value*/
{
	int     displ, a_base, size;
	char   *cp;
	int    *ip;

	displ = TOSM(offset);       /* base + offset of template address */
	a_base = TOSM(offset + 1);
	size = SIZE(ADDR(a_base, displ));

	displ = TOSM(offset + 2);   /* base + offset of array */
	a_base = TOSM(offset + 3);

	ip = ADDR(a_base, displ);
	cp = work_string;
	while(size--)
		*cp++ = *ip++;
	*cp++ = 0;
}


/* MAKE_STRING */

/* This procedure takes the string in work_string, allocates a block to hold
 * it and then copies the string to this block, returning its address. The
 * caller should eventually free this space using predef_free.
 */

char *make_string(void)							                  /*;make_string*/
{
	char   *s;

	s = predef_alloc(strlen(work_string) + 1);
	strcpy(s, work_string);
	return s;
}

/* GET_ARGUMENT_VALUE */

/* Procedure to get argument value of type int. The parameter is the offset
 * from the TOS to the base/offset address. The integer value stored at this
 * address is returned as the result.
 */

int get_argument_value(int offset)			         /*;get_argument_value*/
{
	return *ADDR(TOSM(offset + 1), TOSM(offset));
}

/* GET_FLOAT_ARGUMENT_VALUE */

/* Procedure to get argument value of type float. The parameter is the offset
 * from the TOS to the base/offset address. The float value stored at this
 * address is returned as the result.
 */

float get_float_argument_value(int offset)		 /*;get_float_argument_value*/
{
	return *((float *)(ADDR(TOSM(offset + 1), TOSM(offset))));
}

/* GET_LONG_ARGUMENT_VALUE */

/* Procedure to get argument value of type long. The parameter is the offset
 * from the TOS to the base/offset address. The long value stored at this
 * address is returned as the result.
 */

long get_long_argument_value(int offset)		   /*;get_long_argument_value*/
{
	return *((long *)(ADDR(TOSM(offset + 1), TOSM(offset))));
}

/* GET_ACCESS_ARGUMENT_VALUE and GET-ACCESS-RETURN */

/* These are used for passing/returning access values to/from interface
 * procedures. The access value (base/offset pair) is to be compressed into
 * a long. I reckon on 12 bits for the base, leaving 20 bits for the offset
 */

long get_access_argument_value(int offset)
{
int *bseptr = ADDR(TOSM(offset + 1), TOSM(offset));
int *offptr = bseptr + 1;

return ((*bseptr) << 20) + (*offptr);
}

struct pr {int bs,of;} get_access_return(long a)
{
struct pr pair;

pair.bs = a >> 20;
pair.of = a & 03777777;
return pair;
}


/* GET_FILENUM */

/* Get an integer value using the base and offset values on top of the
 * stack and store the result in filenum.
 */

void get_filenum(void)					                          /*;get_filenum */
{
	filenum = *ADDR(TOSM(1),TOS);
}


/* GET_FILE_ARGUMENT_OR_DEFAULT */

/* Retrieves file argument if necessary, else provides default file.
 * Sets file_offset to 2 if there is a file argument, 0 otherwise.
 * The PREDEF operation codes are arranged so that range tests can be
 * used to determine whether or not a file argument is present, and
 * if not, whether the default file is the current in or out file.
 */

void get_file_argument_or_default()         /*;get_file_argument_or_default*/
{
	if (operation <= P_P_FILE) {
		filenum = *ADDR(TOSM(1),TOS);
		file_offset = 2;
	}
	else {
		filenum = (operation <= P_P_IN) ? current_in_file : current_out_file;
		file_offset = 0;
	}
}


/* RETURN_STRING */

/* S is a C string that is first converted to an Ada string. The parameter
 * param_off is the offset in the stack where the place holder begins. Puts
 * on the stack necessary information to return string s. Used by functions
 * which return a string result.
 */

void return_string(char *s, int param_off)		            /*;return_string*/
{
	int     length, i;
	int     bse, off;
	int    *ptr;

	length = strlen(s);
	exec_create(length, &bse, &off, &ptr);

	for (i = 0; i < length; i++) {
		*ptr++ = *s++;
	}

	TOSM(param_off + 3) = bse;
	TOSM(param_off + 2) = off;

	exec_create(WORDS_S_ARRAY, &bse, &off, &ptr);
	S_ARRAY(ptr) -> ttype = TT_S_ARRAY;
	S_ARRAY(ptr) -> object_size = length;
	S_ARRAY(ptr) -> index_size = 1;
	S_ARRAY(ptr) -> salow = 1;
	S_ARRAY(ptr) -> sahigh = length;

	TOSM(param_off + 1) = bse;
	TOSM(param_off) = off;
}
