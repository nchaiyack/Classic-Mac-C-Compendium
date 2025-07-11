/* ========================================

	walk_utl.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ======================================== */
   
   
// utility routines

void        walk_copy( FSSpec *dest_fss, FSSpec *src_fss );

// public access routines

t_walk_hndl walk_init( FSSpec *start_fss );
OSErr       walk_next( t_walk_hndl wData, FSSpec *fss, short *level, short *isDir );
