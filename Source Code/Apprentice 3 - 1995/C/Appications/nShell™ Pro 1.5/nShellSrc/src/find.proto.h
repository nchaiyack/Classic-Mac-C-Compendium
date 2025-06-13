/* ==========================================

	find.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

void find( void );
void find_init( void );
int  find_match( CharsHandle hChars, int length, long window );
void find_next( int direction );
int  find_ready( void );
void find_set( void );
