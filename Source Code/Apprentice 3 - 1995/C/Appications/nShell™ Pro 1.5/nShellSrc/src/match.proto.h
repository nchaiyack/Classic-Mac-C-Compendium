/* ==========================================

	match.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

char match_next_p( int *i );
char match_next_t( int *i );
void match_measure( void );
int  match_here( int here );
void match_string( void );
int  match( Str255 pattern, Str255 target );
int  match_wild( char c );
int  match_set( void );
char match_peek_p( int *i );