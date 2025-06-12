/*
 * str.c
 */

#include <Types.h>
#include <ToolUtils.h>
#include <Memory.h>

#include "str.h"
#include "x.h"


void
StrGet ( short id , unsigned char * buf , short maxLen ) {

StringHandle sh = GetString ( id ) ;
register unsigned char * src , * dst ;

	FailNil ( sh ) ;
	if ( * * sh < maxLen ) {
		maxLen = * * sh ;
	}
	dst = buf ;
	src = ( * sh ) + 1 ;
	* ( dst ++ ) = maxLen ;
	while ( maxLen -- > 0 ) {
		* ( dst ++ ) = * ( src ++ ) ;
	}
}


void
CopyC2P ( char * cStr , unsigned char * pStr ) {

register char * src = cStr ;
register unsigned char * dst = pStr + 1 ;

	while ( * src ) {
		* ( dst ++ ) = * ( src ++ ) ;
	}
	* pStr = dst - pStr - 1 ;
}


#ifndef useasm
void
ClearMem ( void * mem , int size ) {

register char * ptr = mem ;
register int cnt = size ;
register long * lptr ;

	if ( size > 15 ) { /* Optimize by clearing longs */

		cnt = ( ( long ) ptr ) & 3 ; /* Align to longword */
		size -= cnt ;
		while ( cnt -- ) {
			* ( ptr ++ ) = 0 ;
		}

		cnt = size >> 2 ; /* Num longwords to clear */
		lptr = ( long * ) ptr ;
		while ( cnt -- ) {
			* ( lptr ++ ) = 0L ;
		}
		ptr = ( char * ) lptr ;

		cnt = size & 3 ; /* This many bytes left to clear */
	}

	while ( cnt -- ) { /* Clear bytewise */
		* ( ptr ++ ) = 0 ;
	}
}
#else
#define INT w
#error BUGGY
static void
AsmClearMem ( ) {
	asm {
extern ClearMem :
		move.INT	8(a7) ,	d0
		move.l		4(a7) ,	a0
		cmp.INT		#15 ,	d0
		bge			@1
		move.INT	a0 ,	d1
		and			#3 ,	d1
		sub			d1 ,	d0
		bra			@2
@3:		clr.b		(a0)+
@2:		dbra		d1 ,	@3
		move.INT	d0 ,	d1
		and			#3 ,	d0
		asr			#2 ,	d1
		bra			@4
@5:		clr.l		(a0)+
@4:		dbra		d1 ,	@5
		bra			@1
@6:		clr.b		(a0)+
@1:		dbra		d0 ,	@6
	}
}
#endif


void
ConcatPStrings ( unsigned char * dest , unsigned char * src ) {

int newLen = * dest + * src ;

	if ( newLen > 255 ) {
		newLen = 255 ;
	}
	MemCopy ( src + 1 , dest + * dest + 1 , newLen - * dest ) ;
	* dest = newLen ;
}


void
MemCopy ( void * src , void * dst , int len ) {

register int ll = len ;
register char * s = src ;
register char * d = dst ;

	if ( d == s || ll < 1 ) {
		return ;
	}
	if ( ll > 512 ) { /* Copy performance outweighs cache flush */
		BlockMove ( src , dst , len ) ;
		return ;
	}
	if ( d > s && d - ll < s ) {
		d += ll ;
		s += ll ;
		while ( ll -- ) {
			* ( -- d ) = * ( -- s ) ;
		}
	} else {
		while ( ll -- ) {
			* ( d ++ ) = * ( s ++ ) ;
		}
	}
}


void
CopyPString ( unsigned char * src , unsigned char * dst ) {

	MemCopy ( src , dst , ( * src ) + 1 ) ;
}
