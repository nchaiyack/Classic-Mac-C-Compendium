/* ==========================================

	str_utl.c	

	Copyright (c) 1993,1994,1995 Newport Software Development

   ========================================== */

#include "str_utl.proto.h"

/* ========================================== */

void pStrFromC( StringPtr dst, char *src )
{
	int	 count = 0;
	char c;
		
	while (c = *src++)
		dst[++count] = c;
		
	dst[0] = count;
}

void pStrCopy( StringPtr dst, StringPtr src )
{
	int len;
	
	len = *dst++ = *src++;
	
	while (--len>=0)
		*dst++=*src++;
}

void pStrAppend( StringPtr dst, StringPtr new )
{
	int	old, add, i;
	
	old = dst[0];
	
	if (add = new[0])
		if ((old + add) < 256) {
			for (i = 1; i <= add; i++) dst[old+i] = new[i];
			dst[0] = old + add;
			}
}

void pStrAppendC( StringPtr dst, char *src )
{
	int	 count;
	char c;
	
	count = dst[0];
	
	while (c = *src++) dst[++count] = c;
		
	if (count < 256)
		dst[0] = count;
}

int pStrEqual( StringPtr one, StringPtr two )
{
	int len;
	
	len = one[0];
	
	if (len != two[0])
		return( 0 );
		
	while (len) {
		if ( one[len] != two[len] )
			return(0);
		len--;
		}
		
	return(1);
}

/* ========================================== */

int	cStrEqual( char *p, char *q )
{
	char c;
	
	do {
		c = *p++;
		if (c!=*q++) return(0);
	} while ( c != 0 );
			
	return(1);
}

int	cStrLen( char *p )
{
	int	len;
	
	len = 0;
	
	while ( *p++ ) len++;
			
	return( len );
}

void cStrFromNum( char *p, int width, long num )
{
	int		i,j;
	Str32	string;
	
	NumToString( num, string );
	
	j = string[0];
	
	if ( j > width )
		for ( i = 0 ; i < width ; i++ )		// not enough room
			p[i] = '*';
		
	i = width;
	
	while ( i )
		if ( j )
			p[--i] = string[j--];			// right justify
		else
			p[--i] = ' ';
}