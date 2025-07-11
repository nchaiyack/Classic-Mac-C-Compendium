#include <ctype.h>
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"

/* -*-C-*- strchr.h */
/*-->strchr*/
/**********************************************************************/
/******************************* strchr *******************************/
/**********************************************************************/

#ifndef KCC_20
#define KCC_20 0
#endif

#ifndef IBM_PC_MICROSOFT
#define IBM_PC_MICROSOFT 0
#endif

#ifndef OS_VAXVMS
#define OS_VAXVMS 0
#endif


#if    (KCC_20 | IBM_PC_MICROSOFT | OS_VAXVMS | OS_THINKC)
#else
char*
strchr(s,c)	/* return address of c in s[], or (char*)NULL if not found. */
register char *s;/* c may be NUL; terminator of s[] is included in search. */
register char c;
{
    while ((*s) && ((*s) != c))
	++s;

    if ((*s) == c)
	return (s);
    else
	return ((char*)NULL);
}
#endif

/* -*-C-*- strcm2.h */
/*-->strcm2*/
/**********************************************************************/
/******************************* strcm2 *******************************/
/**********************************************************************/


/***********************************************************************
 Compare strings (ignoring case), and return:
	s1>s2:	>0
	s1==s2:  0
	s1<s2:	<0
***********************************************************************/

/* toupper() is supposed to work for all letters, but PCC-20 does it
incorrectly if the argument is not already lowercase; this definition
fixes that. */

#define UC(c) (islower(c) ? toupper(c) : c)

int
strcm2(s1, s2)
register char *s1, *s2;

{
    while ((*s1) && (UC(*s1) == UC(*s2)))
    {
	s1++;
	s2++;
    }
    return((int)(UC(*s1) - UC(*s2)));
}
#undef UC

/* -*-C-*- strid2.h */
/*-->strid2*/
/**********************************************************************/
/******************************* strid2 *******************************/
/**********************************************************************/

/* toupper() is supposed to work for all letters, but PCC-20 does it
incorrectly if the argument is not already lowercase; this definition
fixes that. */

#define UC(c) (islower(c) ? toupper(c) : c)

int
strid2(string,substring)/* Return index (0,1,...) of substring in string */
char string[];		/* or -1 if not found.  Letter case is IGNORED. */
char substring[];
{
    register int k;	/* loop index */
    register int limit;	/* loop limit */
    register char *s;
    register char *sub;

    limit = (int)strlen(string) - (int)strlen(substring);

    for (k = 0; k <= limit; ++k)/* simple (and slow) linear search */
    {
	s = &string[k];

	for (sub = &substring[0]; (UC(*s) == UC(*sub)) && (*sub); (++s, ++sub))
	    /* NO-OP */ ;

	if (*sub == '\0')	/* then all characters match */
	    return(k);		/* success -- match at index k */
    }

    return(-1);			/* failure */
}

/* -*-C-*- strrchr.h */
/*-->strrchr*/
/**********************************************************************/
/****************************** strrchr *******************************/
/**********************************************************************/

#if    (KCC_20 | IBM_PC_MICROSOFT | OS_VAXVMS | OS_THINKC)
#else
char*
strrchr(s,c)	/* return address of last occurrence of c in s[], */
		/* or (char*)NULL if not found.  c may be NUL; */
		/* terminator of s[] is included in search. */
register char *s;
register char c;
{
    register char *t;

    t = (char *)NULL;
    for (;;)		/* loop forever */
    {
	if (*s == c)
	    t = s;	/* remember last match position */
	if (!*s)
	    break;	/* exit when NULL reached */
	++s;		/* advance to next character */
    }
    return (t);
}
#endif

