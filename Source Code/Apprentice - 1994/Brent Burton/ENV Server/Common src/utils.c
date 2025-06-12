/*
** utils.c
**
** Contains code to check if a trap exists and a few small string
** functions put here for convenience.
*/

#include <OSUtils.h>
#include <Traps.h>
#include <Types.h>
#include "utils.h"


static TrapType GetTrapType(int);
/*=================================================== */
/*
** this is the code supporting Apple's official method
** of checking for the existence of a specific trap.
** See p. 3-8 of IM-VI for the details.
*/

#define NumToolboxTraps ( (NGetTrapAddress(_InitGraf, ToolTrap)==   \
			             NGetTrapAddress(0xAA6e, ToolTrap)) ? 0x0200 : 0x0400)


static TrapType GetTrapType(int theTrap)
{
	unsigned TrapMask = 0x0800;
	return ((theTrap & TrapMask)>0 ? ToolTrap : OSTrap);
} /* GetTrapType() */


Boolean TrapAvailable(int theTrap)
{
	TrapType tType = GetTrapType( theTrap);
	if (tType == ToolTrap) {
		theTrap = (int)theTrap & (int)0x07ff;
		if (theTrap >= NumToolboxTraps)
			theTrap = _Unimplemented;
	}
	return (Boolean)(NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap));
} /* TrapAvailable() */


/*=================================================== */

/**************************
** mystrlen
** mystrcpy
** mystrcat
**
** String routines put here for convenience.
** s2 is usually the "source" and s1 is the "destination."
** Would somebody like to recode these in assembly??
***************************/
long mystrlen( char *s)
{
	register char *e = s;

	while (*e) e++;		/* find ending null */
	return (e - s);		/* length = end - start */
} /* mystrlen */


char *mystrcpy( char *s1, char *s2)
				/* s2 into s1 */
{
	register char *dst = s1,   *src = s2;
	while ( *dst++ = *src++) ;
	return s1;
} /* mystrcpy */


char *mystrcat( char *s1, char *s2)
				/* s2 onto end of s1 */
{
	char *p;
	p = s1 + mystrlen( s1);		/* p points to s1's nul */
	mystrcpy( p, s2);			/* copy s2 onto end of s1; programmer must
	                            ** ensure s1 is long enough - not my job! */
	return s1;
} /* mystrcat */


char *mystrchr( char *s1, char c)
				/* find 1st occurrance of c in s1 */
{
	register char *p=s1;
	while (*p && (*p != c)) p++;
	if (*p == c)
		return p;			/* if we found what we're looking for, rtn ptr */
	else
		return NULL;			/* if not found, return NULL */
} /* mystrchr */



