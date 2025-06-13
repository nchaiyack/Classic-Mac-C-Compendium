/* ========================================

	walk_utl.proto.h
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own programs.
	
	All other rights are reserved.
	
   ======================================== */
   
   
// utility routines

void        walk_copy( FSSpec *dest_fss, FSSpec *src_fss );

// public access routines

t_walk_hndl walk_init( FSSpec *start_fss );
OSErr       walk_next( t_walk_hndl wData, FSSpec *fss, short *level, short *isDir );
