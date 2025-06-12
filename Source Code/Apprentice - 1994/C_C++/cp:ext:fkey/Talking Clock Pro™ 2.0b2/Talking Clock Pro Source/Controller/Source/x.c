/*
 * x.c ©1993 Jon WŠtte (h+@nada.kth.se) All Rights Reserved
 *
 * Please read the paragraph marked $$$ in x.h before you use this source
 * file.
 *
 * This file may be used and distributed for free, if certain conditions
 * are met. These conditions are outlined in the accompanying file x.h.
 * This file may NOT be distributed if it is in any way modified from the
 * original as distributed by Jon WŠtte. Also it may only be distributed
 * accompanying the file x.h. See x.h for details.
 *
 * v1.0 930826 - Initial release
 */

#include <Types.h>
#include "x.h"


JBT * __cur_buf = NULL ;
#ifdef FAILINFO
char * __err_file = NULL ;
int __err_line = 0 ;
#endif


#ifdef FAILINFO
void
__FailNil ( void * p , char * file , int line ) {
	if ( ! p ) {
		__err_file = file ;
		__err_line = line ;
		if ( __cur_buf ) {
			longjmp ( CB2JB __cur_buf , -108 ) ;
		}
		DebugStr ( "\pException uncaught!" ) ;
	}
}


void
__FailErr ( short err , char * file , int line ) {
	if ( err ) {
		__err_file = file ;
		__err_line = line ;
		if ( __cur_buf ) {
			longjmp ( CB2JB __cur_buf , err ) ;
		}
		DebugStr ( "\pException uncaught!" ) ;
	}
}
#else
void
FailNil ( void * p ) {
	if ( ! p ) {
		longjmp ( * __cur_buf , -108 ) ;
	}
	DebugStr ( "\pException uncaught!" ) ;
}


void
FailErr ( short err ) {
	if ( err ) {
		if ( __cur_buf ) {
			longjmp ( * __cur_buf , err ) ;
		}
		DebugStr ( "\pException uncaught!" ) ;
	}
}
#endif
