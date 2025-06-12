/*****************************************************************************
* 
*			  NCSA HDF Vset release 2.1
*					May 1991
*
* NCSA HDF Vset release 2.1 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
* 
* We ask, but do not require, that the following message be included in all
* derived works:
* 
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
* 
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
* 
*****************************************************************************
* Likkai Ng May 1991 NCSA
*
* vparse.c
* Part of the HDF VSet interface.
*
************************************************************************/

#include <stdio.h>
#include "vg.h"

#ifdef SUN
#include <ctype.h> 
#endif

#define ISCOMMA(c) ( (c==',')?1:0)

/* ------------------------------------------------------------------ */

/*
** Given a string (attrs) , the routine parses it into token strings,
** and returns a ptr (attrv) to an array of ptrs where the tokens 
** are stored.  The no of tokens are returned in attrc.
**
** Currently used only by routines that manipulate field names.
** As such each field string is truncated to a max length of
** FIELDNAMELENMAX (as defined in vg.h). For most cases, this
** truncation doesn't happen because FIELDNAMELENMAX is a big number.
**
** RETURN -1 if error.
** RETURN 1 if ok.
**
** Current implementation: all strings inputs converted to uppercase.    
** tokens must be separated by COMMAs.
**
** Tokens are stored in static area sym , and pointers are returned
** to calling routine. Hence, tokens must be used before next call 
** to scanattrs.
**
*/

PRIVATE 	char* 	symptr[50];
PRIVATE 	char 		sym[50][FIELDNAMELENMAX+1];
PRIVATE	int 		nsym;

int scanattrs (attrs,attrc,attrv)       /*@@*/

char	*attrs;				/* field string (input) */
int	*attrc;				/* # of fields (output) */
char	***attrv;			/* array of char ptrs to fields (output) */
{
	char *s, *s0, *ss;
	int i,slen,len;

	s = attrs;
	slen = strlen(s);
	nsym = 0;

	s0 = s;
	for (i=0;i<slen;i++,s++)
		if ( ISCOMMA(*s) ) {
			len = s-s0; 
			if (len<=0) return(FAIL);
			/* save that token */
			ss = symptr[nsym] = sym[nsym]; 
			nsym++;

			if ( len > FIELDNAMELENMAX) len = FIELDNAMELENMAX;
			strncpy(ss, s0, len );
			ss[len] = '\0';

			s0 = s+1;
		}

	/* save the last token */
	len = s-s0; 
	if (len<=0) return(FAIL);
	ss = symptr[nsym] = sym[nsym]; 
	nsym++;

	if ( len > FIELDNAMELENMAX) len = FIELDNAMELENMAX;
	strncpy(ss, s0, len);
	ss[len] = '\0';

	/* convert all fields tokens to uppercase */
	for (i=0;i<nsym;i++) {
		s = symptr[i];
		while(*s!='\0') {
			if (*s >='a' && *s <='z') *s=toupper(*s);
			s++;
		}
	}

	symptr[nsym] = NULL;
	*attrc = nsym;
	*attrv = (char**) symptr;

	return(1); /* ok */

} /* scanattrs */

/* ------------------------------------------------------------------ */
