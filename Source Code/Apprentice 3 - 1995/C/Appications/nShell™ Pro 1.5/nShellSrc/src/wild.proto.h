/* ==========================================

	wild.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

char wild_nextc( void );
void wild_add( char c );
int  wild_pattern( int start, int end );
int  wild_match( int start, int end, int quote );
void wild_word( void );
int  wildcard( void );
void wild_redir( void );
int  wild_child( Str255 ref_path, Str255 pattern, int quote );