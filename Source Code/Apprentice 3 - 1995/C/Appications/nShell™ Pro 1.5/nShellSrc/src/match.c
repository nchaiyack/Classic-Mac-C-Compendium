/* ==========================================

	match.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <string.h>

#include "match.proto.h"

/* ========================================== */

#define NO_MATCH	1
#define MATCH		0

/* ========================================== */

typedef enum {	match_immediate, match_first, match_last } t_match;

/* ========================================== */

char		mpc;		// character last read from pattern
char		mtc;		// character last read from target

int			mpi;		// index into pattern
int			mti;		// index into target

int			mstart;		// start of fixed pattern
int			mend;		// end of fixed pattern
int			mlen;		// length of fixed pattern in real chars

int			mps;		// size of pattern
int			mts;		// size of target

int			mtype;		// type of string match

StringPtr	mp_str;		// pointer to pattern string
StringPtr	mt_str;		// pointer to target string

int			mm;			// result of wildcard match;

/* ========================================== */

int match_wild( char c )
{
	if ((c == '*') || (c == '?') || (c == '['))
		return(1);
	else
		return(0);
}
			
/* ========================================== */

char match_next_p( int *i )
{
	if ( *i < mps )
		return( mp_str[ ++(*i) ] );
	else
		return(0);
}
			
/* ========================================== */

char match_peek_p( int *i )
{
	if ( *i < mps )
		return( mp_str[ *i + 1 ] );
	else
		return(0);
}
			
/* ========================================== */

char match_next_t( int *i )
{
	if ( *i < mts )
		return( mt_str[ ++(*i) ] );
	else
		return(0);
}
			
/* ========================================== */

// set mstart and mend to first and last char of a mask.

void match_measure( void )
{
	int	bracket;
	
	mlen = 0;
	mstart = mpi;
	bracket = 0;
	
	do {
		mpc = match_next_p(&mpi);
		if (bracket) {
			if ( mpc == ']')
				bracket = 0;
			if ( mpc == '[') {
				mm = -1;
				return;
				}
			}
		else
			if (mpc)
				mlen++;
		if ( mpc == '[' )
			bracket = 1;
		if ( mpc == '\\' ) {
			mpc = match_next_p(&mpi);	// make sure there is a \x pair
			if ( !mpc ) {
				mm = -1;
				return;
				}
			mpc = ' ';	// and skip it
			}
	} while ( mpc && ( mpc != '*' ) );

	if (bracket)
		mm = -1;

	mend = mpi;

	if (mpc == '*') {
		mlen--;
		mend--;
		}
}

/* ========================================== */

// compare a set at mp_str[ mpi ] with the character mtc

int match_set( void )
{
	char	jim,bob;
	int		not;
	int		found;
	int		range;

	found = not = 0;
	
	if ( match_peek_p(&mpi) == '^' ) {
		not = 1;
		mpi++;
		}

	while ( 1 ) {
	
		jim = match_next_p(&mpi);
		
		if (!jim)  {
			mm = -1;
			return( 0 );
			}
	
		if ( jim == ']' ) {
			if (not)
				found = !found;
			return( found );
			}

		if ( ( jim == '-' ) || match_wild( jim ) ) {
			mm = -1;
			return( 0 );
			}

		if ( jim == '\\' ) {
			jim = match_next_p(&mpi);	
			if (!jim)  {
				mm = -1;
				return( 0 );
				}
			}

		range = 0;
		
		if ( mpi < mps )
			if ( mp_str[ mpi + 1 ] == '-' ) {
			
				range = 1;
				mpi++;
				
				bob = match_next_p(&mpi);	
				if (!bob)  {
					mm = -1;
					return( 0 );
					}
					
				if ( bob == '\\' )
					if ( mpi < mps )
						bob = mp_str[ ++mpi ];
					else {
						mm = -1;
						return( 0 );
						}
				}
				
		if (range) {
			if (( mtc >= jim ) && ( mtc <= bob )) found = 1;
			}
		else {
			if ( mtc == jim ) found = 1;
			}
		}
}

/* ========================================== */

// match a string mp_str[ mstart .. mend ] at mt_str[ here ]

int match_here( int here )
{
	int		sav_p;
	int		i,j,k;
	char	sav_c;
	char	c;
	
	sav_c = mpc;
	sav_p = mpi;
	mpi = mstart;
	mti = here;
	
	while ( mpi < mend ) {
	
		mpc = match_next_p(&mpi);
		mtc = match_next_t(&mti);
		
		switch (mpc) {
			case '?':
				if (!mtc) {
					mpc = sav_c;
					mpi = sav_p;
					return(0);
					}
				break;
			case '[':
				if (!match_set()) {
					mpc = sav_c;
					mpi = sav_p;
					return(0);
					}
				break;
			case '\\':
				mpc = match_next_p(&mpi);
			default:
				if (mpc != mtc) {
					mpc = sav_c;
					mpi = sav_p;
					return(0);
					}
				break;
			}

		}
		
	mpc = sav_c;
	mpi = sav_p;
	return(1);
}

/* ========================================== */

// match a string mp_str[ mstart .. mend ]

void match_string( void )
{
	int i;
	
	switch (mtype) {
		case match_first:
			for ( i = mti; i < mts; i++ )
				if ( match_here(i) )
					return;
			mm = NO_MATCH;
			break;
		case match_last:
			if ( ( mts - mti ) < mlen )	{
				mm = NO_MATCH;
				return;
				}
			mti = mts - mlen;
		default:
			if ( !match_here( mti ) ) {
				mm = NO_MATCH;
				return;
				}
			break;
		}
}
		
/* ========================================== */

// main entry retruns < 0 = error, MATCH (0), NO_MATCH (1)

int match( Str255 pattern, Str255 target )
{
	mm = MATCH;
	
	mp_str = pattern;
	mt_str = target;
	
	mps = mp_str[0];
	mts = mt_str[0];
	
	mpi = mti = 0;
	
	mpc = match_next_p( &mpi );
	
	while ( mpc ) {
	
		switch (mpc) {
			case ']':
				return( -1 );
				break;
			case '*':
				match_measure();
				if ( mm == MATCH ) {
					if ( mpc == '*' )
						mtype = match_first;
					else
						mtype = match_last;
					match_string();
					}
				break;
			default:
				mpi--;
				match_measure();
				if ( mm == MATCH ) {
					mtype = match_immediate;
					match_string();
					if ( ( !mpc ) && ( mti < mts ) )
						mm = NO_MATCH;
					}
				break;
			}
			
		if ( mm != MATCH )
			return( mm );

	}
		
	return( mm );
}

/* ========================================== */