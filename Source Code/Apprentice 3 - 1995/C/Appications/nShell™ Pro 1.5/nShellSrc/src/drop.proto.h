/* ==========================================

	drop.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#ifndef __INTERP__
#include "interp.h"
#endif

void   drop_fss( FSSpec *fss );
void   drop_init( void );
void   drop_open( void );
int    drop_vars( ShellH shell, InterpH interp );
void   drop_filename( Str255 filename );
int    drop_search_one( short vol );
int    drop_search_others( Boolean remote_search );
void   drop_get_home( void );
