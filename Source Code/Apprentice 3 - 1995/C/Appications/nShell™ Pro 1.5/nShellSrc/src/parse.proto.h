/* ==========================================

	parse.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

void parse_set_pos( long pos );
int  parse_flow( char *skip1, char *skip2, char *match1, char *match2 );
void parse_add_buf( char c );
int  parse_pass_1( void );
int  parse_pass_2( void );
void parse_command( void );
int  parse_in_body( char c, char qoute );
char parse_next_fbuf( void );
char parse_peek_fbuf( void );
int  parse_whitespace( char c );
void parse_a_word( int skip );
void parse_parameters( void );
void parse_vars( void );
void parse_args( void );
char parse_nextc( void );
void parse_get_path( Str255 filename );
void parse_set_input( void );
void parse_set_output( void );
void parse_set_pipe( void );
void parse_get_arg( void );
void parse_do( void );
void parse_cmd_path( void );
void parse_add( char c );
void parse_pipe_fitter( void );
void parse_then( void );
void parse_else( void );