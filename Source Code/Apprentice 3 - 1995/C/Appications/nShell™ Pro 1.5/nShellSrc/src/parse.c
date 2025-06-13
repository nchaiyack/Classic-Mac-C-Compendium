/* ==========================================

	parse.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <string.h>

#include "nsh.h"
#include "multi.h"

#include "interp.h"
#include "path.h"
#include "pipe.h"

#include "cmd_int.proto.h"
#include "flow.proto.h"
#include "script.proto.h"
#include "parse.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "vlist.proto.h"
#include "wild.proto.h"

/* ========================================== */

	int		gerr;		// error status (0 = no error)
	int		gt;			// type of current command

	char	gc;			// current character
	int		gi;			// position in input array to read next
	int		gm;			// number of characters available

	int		gj;			// position in output array to write next

	char	gbuf[LINE_MAX];	// a buffer used for variable expansion, etc.

	Str255	gInFile;	// holds path for script stdin
	int		gInState;	// holds state for script in-pipe

	Str255	gOutFile;	// holds path for script stdout
	int		gOutState;	// holds state for script out-pipe
	int		gOutAppend;	// holds append flag for script stdout
	
/* ========================================== */

extern	InterpH	i_interp;
extern	ShellH	i_shell;

/* ========================================== */

int	parse_in_body( char c, char quote )
{
	if (quote)
		if ((c == quote) || (c == '\0'))
			return(0);
		else
			return(1);
		
	switch (c) {
		case ' ':
		case '|':
		case ';':
		case '"':
		case '\'':
		case '\0':
		case '\t':
		case '\r':
			return(0);
		default:
			return(1);
		}
}

/* ========================================== */

int parse_whitespace( char c )
{
	if ((c == ' ') || (c == '\t'))
		return(1);
	else
		return(0);
}

/* ========================================== */

char parse_next_fbuf( void )
{
	if ((gi >= gm) || gerr )
		return( '\0' );
	else
		return( (**i_interp).fbuf[gi++] );
}

/* ========================================== */

char parse_peek_fbuf( void )
{
	if ((gi >= gm) || gerr )
		return( '\0' );
	else
		return( (**i_interp).fbuf[gi] );
}

/* ========================================== */

void parse_add_buf( char c )
{
	if (!gerr)
		if ( gj < LINE_MAX ) 
			(**i_interp).buf[gj++] = c;
		else {
			pipe_putStr_err( i_shell, "\pparse: Line exceeds maximum size.\r" );
			gerr = NSHC_ERR_GENERAL;
			}
}

/* ========================================== */

void parse_set_pos( long position )
{
	long fpos;
	
	if ((**i_interp).source == 0) {
		(**i_interp).fbuf_ptr = position;
		return;
		}
	
	fpos = (**i_interp).fpos;
	if ( ( position >= fpos ) && ( position < fpos + (**i_interp).fbuf_chars ) ) {
		(**i_interp).fbuf_ptr = position - fpos;
		return;
		}
	
	script_set_pos( position );
}

/* ========================================== */

void parse_a_word( int skip_semis )
{
	char	quote;
	int		bad_quote;
	int		state;
	
	do {
		gc = parse_next_fbuf();
		if ( (gc == ';') && skip_semis )
			gc = ' ';
	} while (parse_whitespace(gc));

	if ( gc == '|' )
		return;

	if ( gc == '#' ) {
		while ( gc && (gc != '\r'))
			gc = parse_next_fbuf();
		return;
		}
			
	if ( gc == '<' ) {
		parse_add_buf( gc );
		gc = parse_next_fbuf();
		}
	else
		if ( gc == '>' ) {
			parse_add_buf( gc );
			gc = parse_next_fbuf();
			if ( gc == '>' ) {
				parse_add_buf( gc );
				gc = parse_next_fbuf();
				}
			if ( gc == '&' ) {
				parse_add_buf( gc );
				gc = parse_next_fbuf();
				}
			}
		
	if ( ( gc == '"' ) || ( gc == '\'' ) ) {
		quote = gc;
		parse_add_buf( gc );
		gc = parse_next_fbuf();
		}
	else
		quote = 0;
				
	while ( parse_in_body(gc,quote) ) {
		parse_add_buf( gc );
		gc = parse_next_fbuf();
		}
					
	bad_quote = 0;
	
	if (quote) {
		if ( gc != quote )
			bad_quote = 1;
		}
	else {
		if ( ( gc == '"' ) || ( gc == '\'' ) )
			bad_quote = 1;
		}
	
	if ( ( gc == '"' ) || ( gc == '\'' ) ) {
		parse_add_buf( gc );
		gc = parse_peek_fbuf();
		if (parse_in_body(gc,0))
			bad_quote = 1;
		}
			
	if (bad_quote) {
		pipe_putStr_err( i_shell, "\pparse: Unbalanced quotes.\r" );
		gerr = NSHC_ERR_GENERAL;
		}
}

/* ========================================== */

void parse_command( void )
{
	int		i;

	parse_a_word(1);
	
	if (!gerr) {
	
		if (gj > 0)
			if ((**i_interp).buf[0] == '|') {
				pipe_putStr_err( i_shell, "\pparse: Source of pipe not found.\r" );
				gerr = NSHC_ERR_GENERAL;
				}
			else {
				gt = ct_tbd;
				if (gj < 32) {
					for (i=0; i<gj; i++)
						(**i_shell).cmd_path[i+1] = (**i_interp).buf[i];
					(**i_shell).cmd_path[0] = gj;
					if ( flow_check( (**i_shell).cmd_path ) ) gt = ct_flow;
					}
				}
		}
}

/* ========================================== */

void parse_parameters( void )
{
	int		sav_j;
	int		sav_i;
	int		len;
	int		i,j;
	int		done;
	Str32	name;
	
	done = 0;
	
	if (gc == '|') {
		parse_add_buf( ' ' );
		parse_add_buf( '|' );
		if ( parse_peek_fbuf() == '&' ) {
			parse_add_buf( '&' );
			gi++;
			}
		return;
		}

	while ( gc && (gc != ';') && (gc != '|') && (gc != '\r')) {
		parse_add_buf( ' ' );
		sav_j = gj;
		sav_i = gi;
		parse_a_word( 0 );
		len = gj - sav_j;
		if ((len > 0) && (len < 32)) {
			i = sav_j;
			for (j=1; j <= len; j++)
				name[j] = (**i_interp).buf[i++];
			name[0] = len;
			if ( flow_check( name ) ) {
				gi = sav_i;
				gj = sav_j;
				gc = 0;
				}
			}
		}
		
	if (gc == '|') {
		parse_add_buf( ' ' );
		parse_add_buf( '|' );
		if ( parse_peek_fbuf() == '&' ) {
			parse_add_buf( '&' );
			gi++;
			}
		}
}

/* ========================================== */

int parse_pass_1( void )
{	
	gt = ct_null;
		
	if ((**i_interp).fbuf_ptr >= (**i_interp).fbuf_chars)
		if ((**i_interp).source == 0) {
			if ((**i_interp).flow_list) {
				pipe_putStr_err( i_shell, "\pparse: Uncompleted flow-of-control.\r" );
				(**i_interp).result = NSHC_ERR_GENERAL;
				(**i_interp).action = nsh_stop;
				}
			else
				(**i_interp).action = nsh_idle;
			}
		else
			script_read_file();
	
	if ((**i_interp).action != nsh_continue)	
		return( ct_null );

	gerr = 0;
	gInState = redir_none;
	
	gc = 0;
	gj = 0;
	
	gi = (**i_interp).fbuf_ptr;
	gm = (**i_interp).fbuf_chars;
	
	parse_command();
	
	if (gt == ct_tbd)
		parse_parameters();
		
	(**i_interp).fbuf_chars = gm;
	(**i_interp).fbuf_ptr = gi;
	(**i_interp).buf_chars = gj;
	
	if (gerr) {
		(**i_interp).action = nsh_stop;
		(**i_interp).result = gerr;
		}

	return( gt );
}

/* ========================================== */

void parse_pipe_fitter( void )
{
	int		pipeState;
	Str255	pipeName;

	if ( (**i_interp).pipeState ) {
		HLock(i_interp);
		pStrCopy( pipeName, (**i_interp).pipeName );
		HUnlock(i_interp);
		if (gt == ct_script) {
			pStrCopy( gInFile, pipeName );
			gInState = redir_pipe_in;
			}
		else
			gerr = pipe_open_input( i_shell, pipeName, redir_pipe_in, redir_by_command );
		}
		
	gOutState = redir_none;
	gOutAppend = 0;
	(**i_interp).pipeState = redir_none;
}

/* ========================================== */

void parse_vars( void )
{
	char	c;
	char	terminator;
	int		i;
	int		j;
	int		k;
	int		first;
	int		last;
	int		length;
	int		overrun;
	Str32	name;
	Str255	value;
	
	gm = (**i_interp).buf_chars;
	
	i = 0;
	j = 0;
	
	while (i < gm) {
		c = (**i_interp).buf[i++];
		
		if (c == '\\' ) {
			gbuf[j++] = c;
			if ( i < gm ) {
				c = (**i_interp).buf[i++];
				gbuf[j++] = c;
				}
			continue;
			}
			
		if (c == '\'' ) {
			gbuf[j++] = c;
			if (i < gm)
				do {
					c = (**i_interp).buf[i++];
					gbuf[j++] = c;
				} while (( c != '\'' ) && ( i < gm ));
			continue;
			}
			
		if (c == '$') {
			first = i-1;
			if ((**i_interp).buf[i] == '{') {
				terminator = '}';
				i++;
				}
			else
				terminator = ' ';
			length = 0;
			overrun = 0;
			c = (**i_interp).buf[i++];
			if ( ( c == '?' ) || ( c == '#' ) ) {
				name[++length] = c;
				c = (**i_interp).buf[i++];
				}
			else
				while ( vlist_char(c) && (i <= gm) ) {
					if (length < 31)
						name[++length] = c;
					else
						overrun = 1;
					c = (**i_interp).buf[i++];
					}
			last = i - 2;
			if (terminator == '}')
				if (c == '}')
					last++;
				else{
					pipe_putStr_err( i_shell, "\pparse: Unmatched '}'\r" );
					gerr = NSHC_ERR_GENERAL;
					return;
					}
			if (overrun) {
  				pipe_putStr_err( i_shell, "\pparse: Variable name too long\r" );
				gerr = NSHC_ERR_GENERAL;
				return;
				}
			else {
				name[0] = length;
				if (vlist_env( i_shell, name, value )) {
					for (k=first;k<=last;k++)
						gbuf[j++] = (**i_interp).buf[k];
					}
				else {
					length = value[0];
					for (k=1;k<=length;k++)
						gbuf[j++] = value[k];
					}
				i = last + 1;
				}
			}
		else
			gbuf[j++] = c;
	}
	
	gm = j;
	gi = 0;
	gj = 0;
}

/* ========================================== */

char parse_nextc( void )
{
	if ((gi >= gm) || gerr )
		return( '\0' );
	else
		return( gbuf[gi++] );
}

/* ========================================== */

void parse_add( char c )
{
	if (!gerr)
		if ( gj < LINE_MAX ) 
			(**i_shell).nshc.arg_buf[gj++] = c;
		else {
			pipe_putStr_err( i_shell, "\pparse: Line exceeds maximum size.\r" );
			gerr = NSHC_ERR_GENERAL;
			}
}

/* ========================================== */

void parse_get_path( Str255 filename )
{
 	char		quote;
	int			overrun;
	int			length;
	
 	quote = 0;
  	length = 0;
  	overrun = 0;
  	
  	while (parse_whitespace(gc))
  		gc = parse_nextc(); 

	while ( (!parse_whitespace(gc) || quote) && gc) {
  	
		if (length >= 255)
			overrun = 1;
		else
			if ((gc == '"') || (gc == '\'')) {
				if (!quote)
					quote = gc;
				else
					if (gc == quote)
						quote = 0;
					else
						filename[++length] = gc;
				}
			else
				if ((quote == '\'') || (gc != '\\'))
					filename[++length] = gc;
						
		gc = parse_nextc();

		}

  	if (overrun) {
  		pipe_putStr_err( i_shell, "\pparse: Pathname for redirection too long.\r" );
		gerr = NSHC_ERR_GENERAL;
  		}
  	else
  		filename[0] = length;
}

/* ========================================== */

void parse_set_input( void )
{
	Str255		filename;
	
  	filename[0] = 0;
  	
	gc = parse_nextc();

	parse_get_path( filename );
	
	if (filename[0]) {
		if (gt == ct_script) {
			pStrCopy( gInFile, filename );
			gInState = redir_in;
			}
		else
			gerr = pipe_open_input( i_shell, filename, redir_in, redir_by_command );
		}
  	else {
	  	pipe_putStr_err( i_shell, "\pparse: Bad pathname for stdin.\r" );
		gerr = NSHC_ERR_GENERAL;
	  	}
}

/* ========================================== */

void parse_set_output( void )
{
	int			append;
	int			outState;
	Str255		filename;
	
	filename[0] = 0;
	
   	append = 0;
   	outState = redir_out;
  	
	gc = parse_nextc();

  	if (gc == '>') {
		gc = parse_nextc();
		append = 1;
  		}
  	
  	if (gc == '&') {
		gc = parse_nextc();
		outState = redir_outerr;
  		}
  	
	parse_get_path( filename );
	
	if (filename[0]) {
	
		if ( pStrEqual( filename, "\pdev:null" ) )
			if ( outState == redir_outerr )
				outState = redir_outerr_null;
			else
				outState = redir_out_null;
				
		if ( pStrEqual( filename, "\pdev:tty" ) )
			if ( outState == redir_outerr )
				outState = redir_outerr_tty;
			else
				outState = redir_out_tty;
				
		if (gt == ct_script) {
			pStrCopy( gOutFile, filename );
			gOutState = outState;
			gOutAppend = append;
			}
		else
			gerr = pipe_open_output( i_shell, filename, outState, redir_by_command, append );
		}
  	else {
	  	pipe_putStr_err( i_shell, "\pparse: Bad pathname for stdout.\r" );
		gerr = NSHC_ERR_GENERAL;
	  	}
}

/* ========================================== */

void parse_set_pipe( void )
{
	int		pipeState;
	Str255	pipeName;

	gc = parse_nextc();

	if (gc == '&')
		pipeState = redir_outerr_pipe;
	else
		pipeState = redir_out_pipe;
		
	pipe_make_name( i_shell, pipeName );
	
	if (gt == ct_script) {
		pStrCopy( gOutFile, pipeName );
		gOutState = pipeState;
		gOutAppend = 0;
		}
	else
		gerr = pipe_open_output( i_shell, pipeName, pipeState, redir_by_command, 0 );
		
	HLock(i_interp);
	pStrCopy( (**i_interp).pipeName, pipeName );
	HUnlock(i_interp);
	
	(**i_interp).pipeState = pipeState;
}

/* ========================================== */

void parse_get_arg( void )
{
	char		quote;
	int			argc;
	
  	quote = 0;
  	
  	argc = (**i_shell).nshc.argc;
  	
  	if ((**i_shell).nshc.argc >= MAX_ARGS) {
		pipe_putStr_err( i_shell, "\pparse: Too many arguments.\r" );
		gerr = NSHC_ERR_GENERAL;
		return;
		}
		
 	(**i_shell).nshc.argv[argc] = gj;
 		
  	(**i_shell).nshc.argc = argc + 1;
  	
	while ( (!parse_whitespace(gc) || quote) && gc) {
 
		if ((gc == '"') || (gc == '\'')) {
			if (!quote)
				quote = gc;
			else
				if (gc == quote)
					quote = 0;
				else
					parse_add( gc );
			}
		else
			if ((quote == '\'') || (gc != '\\'))
				parse_add( gc );

		gc = parse_nextc();

	}
	
	parse_add( '\0' );
}

/* ========================================== */

void parse_cmd_path( void )
{
	Str255 path;
	
	if ( cStrLen( (**i_shell).nshc.arg_buf ) > 255 ) {
		pipe_putStr_err( i_shell, "\pparse: Command path is too long\r");
		gerr = NSHC_ERR_GENERAL;
		}
	else
		pStrFromC( path, (**i_shell).nshc.arg_buf );

	if (!gerr)
		switch (path_which( i_shell, path )) {
			case which_internal:
				gt = ct_internal;
				break;
			case which_external:
				gt = ct_external;
				break;
			case which_script:
				gt = ct_script;
				break;
			default:
				gt = ct_not_found;
				break;
			}

	if (!gerr)
		pStrCopy( (**i_shell).cmd_path, path );
}

/* ========================================== */

void parse_args( void )
{
	while ( gc = parse_nextc() ) {
	
  		switch (gc) {
 			case ' ':
 			case '\t':
  				break;
 			case '<':
 				parse_set_input();
 				break;
 			case '>':
 				parse_set_output();
 				break;
 			case '|':
 				parse_set_pipe();
 				break;
 			default:
 				parse_get_arg();
 				if ( (**i_shell).nshc.argc == 1 ) {
					if (!gerr)
						parse_cmd_path();
					if (!gerr)
						parse_pipe_fitter();
					}
 				break;
 			}
 			
 			
 		}
}

/* ========================================== */

int parse_pass_2( void )
{
	Str255	path;
	
	gt = ct_null;
	gerr = 0;

	HLock(i_shell);
	memset( &((**i_shell).nshc), '\0', sizeof(t_nshc_parms) );
	HUnlock(i_shell);
	
	(**i_shell).nshc.version = NSHC_VERSION;

	if (!gerr)
		parse_vars();	

	if (!gerr)
		gerr = wildcard();

	if (!gerr)
		parse_args();
				
	if (gerr) {
		(**i_interp).action = nsh_stop;
		(**i_interp).result = gerr;
		gt = ct_null;
		}
	
	return( gt );
}

/* ========================================== */

#if PROFESSIONAL

/* ========================================== */

int parse_flow( char *skip1, char *skip2, char *match1, char *match2 )
{
	int	temp;
	int	found;
	int	count;
	
	found = count = gerr = 0;
	
	gc = 0;
	
	gi = (**i_interp).fbuf_ptr;
	gm = (**i_interp).fbuf_chars;
	
	do {
		gj = 0;

		parse_a_word(1);
		
		if ( !gerr && !gc && ((**i_interp).source != 0) ) {
			script_set_pos( (**i_interp).fpos + gi );
			gi = (**i_interp).fbuf_ptr;
			gm = (**i_interp).fbuf_chars;
			}
			
		if (gerr)
			break;
			
		if ((**i_interp).buf[0] == '#')
			break;
		
		parse_add_buf( '\0' );

		temp = cStrEqual((**i_interp).buf,skip1);
		if (skip2 && !temp) temp = cStrEqual((**i_interp).buf,skip2);
			
		if ( temp )
			count++;
			
		temp = cStrEqual((**i_interp).buf,match1);

		if ( temp )
			if (count)
				count--;
			else 
				found = 1;		// matches match1
			
		if (match2 && !temp) {
			temp = cStrEqual((**i_interp).buf,match2);
			if (temp && !count)
				found = 2;		// matches match2
			}

	}while ( gc && !found && !gerr );
	
	return(found);
}

/* ========================================== */

void parse_then( void )
{
	int	found;
	
	found = parse_flow( "if", (char *)0, "endif", "else" );

	if (!gerr)
		if (found) {
			(**i_interp).fbuf_ptr = gi;
			if (found == 1)
				flow_endif();
			}
		else {
			pipe_putStr_err( i_shell, "\pparse: 'else' or 'endif' expected.\r" );
			gerr = NSHC_ERR_GENERAL;
			}

	if (gerr) {
		(**i_interp).action = nsh_stop;
		(**i_interp).result = gerr;
		}
}

/* ========================================== */

void parse_else( void )
{
	int	found;
	
	found = parse_flow( "if", (char *)0, "endif", (char *)0 );
	
	if (!gerr)
		if (found) {
			flow_endif();
			(**i_interp).fbuf_ptr = gi;
			}
		else {
			pipe_putStr_err( i_shell, "\pparse: 'endif' expected.\r" );
			gerr = NSHC_ERR_GENERAL;
			}

	if (gerr) {
		(**i_interp).action = nsh_stop;
		(**i_interp).result = gerr;
		}
}

/* ========================================== */

void parse_do( void )
{
	int	found;
	
	found = parse_flow( "while", "until", "done", (char *)0 );	
	
	if (!gerr)
		if (found)
			(**i_interp).fbuf_ptr = gi;
		else {
			pipe_putStr_err( i_shell, "\pparse: 'done' expected.\r" );
			gerr = NSHC_ERR_GENERAL;
			}

	if (gerr) {
		(**i_interp).action = nsh_stop;
		(**i_interp).result = gerr;
		}
}

/* ========================================== */

#endif

