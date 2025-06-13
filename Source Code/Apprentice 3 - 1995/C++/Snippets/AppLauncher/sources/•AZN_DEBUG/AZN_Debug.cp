/*
*	AZN_Debug.cp
*
*	Utility functions for debugging apps
*	© Andrew Nemeth, Warrimoo Australia, 1995
*
*	File Created:		6 Mar 95
*	File Ammended:		6, 7, 11, 12 Mar 95.
*/


#include	"AZN_Debug.h"									//	debugging utilites

#ifdef	DEBUG



//#define	MAC_LOW_LEVEL									//	use for low-level debugger

#include	<new.h>										//	ANSI operator 'new' errors
#include	<stdio.h>										//	ANSI sprintf()
#include	<stdlib.h>									//	ANSI exit()
#include	<string.h>									//	ANSI memset()


//	FILE FUNCTIONS
//
static void	noMoreMem( void );




void		INIT_DEBUG( void )
//
//	set the function ptr for
//	global operator new failures
//
{
	set_new_handler( noMoreMem );
}



void		noMoreMem( void )
//
//	Signal memory failure
//
{
#ifdef MAC_LOW_LEVEL
	::DebugStr( "\pAZN¥Operator New failed!" );
#else
	::printf( "%s", "AZN¥Operator New failed!" );
	::exit( 0 );
#endif
}



void		myAlert( char *txtFile, unsigned ushLine )
//
//	Signal assertion failure
//
{
#ifdef MAC_LOW_LEVEL
	char		txtPrompt[256] = { "\0" };

	::sprintf( txtPrompt, "~Assertion Failed: %s, line %u", txtFile, ushLine );
//	note the above is a trick!  Use '~' as first char
//	as this is ASCII 126.  When we come to typecase the
//	char to a 'StringPtr' below, it will fool DebugStr into
//	thinking that it has a pascal string 126 chars long!
//					
	::DebugStr( (StringPtr)&txtPrompt[0] );
#else
	::printf( "Assertion Failed: %s, line %u", txtFile, ushLine );
	::exit( 0 );
#endif
}




void		SALT_MEMORY( void *vptrBuff, long lgSize )
//
//	Salt deadly value into a block of memory.
//	Value used is '0xA3', which if de-referenced
//	by accident will play merry havoc (on Macs!)
//	See Steve Maguire "Writing Solid Code" p.49
//
{
	::memset( vptrBuff, 0xA3, lgSize );	
}

#endif
