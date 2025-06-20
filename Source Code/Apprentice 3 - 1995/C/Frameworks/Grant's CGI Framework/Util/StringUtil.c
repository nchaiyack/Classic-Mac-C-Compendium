/*****
 *
 *	StringUtil.c
 *
 *****/

#include <string.h>

#include "StringUtil.h"

/* returns the number of occurances of theChar in theString */
long
StringCountChar ( char *theString, char theChar )
{
	long	total;
	char *	tempStr;
	
	total	= (long)nil;
	tempStr	= theString;
	
	do
	{
		tempStr = strchr ( tempStr, theChar );
		
		if ( tempStr != nil )
		{
			tempStr++;
			total++;
		}
	} while ( tempStr != nil );
	
	return total;
} /* StringCountChar */


