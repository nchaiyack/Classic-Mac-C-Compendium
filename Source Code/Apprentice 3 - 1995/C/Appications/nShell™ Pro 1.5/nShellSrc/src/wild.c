/* ==========================================

	wild.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <string.h>

#include "multi.h"
#include "interp.h"

#include "fss.proto.h"
#include "match.proto.h"
#include "parse.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "vlist.proto.h"
#include "wild.proto.h"

/* ========================================== */

extern	InterpH	i_interp;
extern	ShellH	i_shell;

extern	int		gm;
extern	char	gbuf[LINE_MAX];

/* ========================================== */

char		wbuf[LINE_MAX];

char		wc;
int			wi;
int			wo;
int			wl;
int			werr;

/* ========================================== */

char wild_nextc( void )
{
	if ( ( wi >= wl ) || werr)
		return( '\0' );
	else
		return( gbuf[wi++] );
}

/* ========================================== */

void wild_add( char c )
{
	if (wo < LINE_MAX)
		wbuf[wo++] = c;
	else
		werr = -1;
}

/* ========================================== */

int wild_pattern( int start, int end )
{
	int		i;
	char	c;

	// pass one, skip the directory path
	
	for (i = start; i < end; i++) {
		c = gbuf[i];
		if ( c== ':' )
			start = i + 1;
		}
		
	// pass two, explicit reasons to wildcard
	
	for (i = start; i < end; i++) {
		c = gbuf[i];
		if ( match_wild(c) )
			return(1);
		if (( c == '\\' ) && ( i+1 < end ))
			i++;
		}
		
	return(0);
}

/* ========================================== */

int wild_child( Str255 ref_path, Str255 pattern, int quote )
{
	int				merr;
	int				i,j,k;
	int				found;
	int				got_space;
	int				got_double;
	int				got_single;
	CInfoPBRec		cipbr;
	HFileInfo		*fpb;
	short			idx;
	long			foundDir;
	Str255			working_path;
	Str255			result_path;
	short			volume;
	
	found = 0;
	
	if ( ref_path[0] ) {
		pStrCopy( working_path, ref_path );
		if ( path_expand( i_shell, working_path ) )
			return( 0 );
		}
	else {
		if (vlist_env( i_shell, "\pPWD", working_path ))
			return(0);
		}
	
	if ( fss_VRefNum( working_path, &volume ) )
		return(0);

	fpb = (HFileInfo *)&cipbr;
	fpb->ioVRefNum = volume;
	fpb->ioDirID = 0;
	fpb->ioFDirIndex = 0;
	fpb->ioNamePtr = result_path;
	pStrCopy( result_path, working_path );

	if (PBGetCatInfo( &cipbr, FALSE ))
		return(0);
		
	if (fpb->ioFlAttrib & 16) {
		foundDir = fpb->ioDirID;

		for( idx=1; TRUE; idx++) {						/* indexing loop */
			fpb->ioVRefNum = volume;
			fpb->ioDirID = foundDir;					/* must set on each loop */
			fpb->ioFDirIndex = idx;
			pStrCopy( result_path, working_path );
		
			if (PBGetCatInfo( &cipbr, FALSE )) break;	/* exit when no more entries */

			merr = match( pattern, result_path );
			
			if ( merr < 0 ) {
				pipe_putStr_err( i_shell, "\pwildcard: Bad pattern.\r" );
				werr = -1;
				return(0);
				}
			
			if ( merr == 0 ) {				// looks good, get ready to insert it
			
				j = result_path[0];
				k = ref_path[0];
				
				// determine what quoting is required
				
				got_space = got_double = got_single = 0;
				
				for (i = 1; i < k; i++)
					switch (ref_path[i]) {
						case ' ':
							got_space = 1;
							break;
						case '"':
							got_double = 1;
							break;
						case '\'':
							got_single = 1;
							break;
						default:;
						}
						
				for (i = 1; i < j; i++)
					switch (result_path[i]) {
						case ' ':
							got_space = 1;
							break;
						case '"':
							got_double = 1;
							break;
						case '\'':
							got_single = 1;
							break;
						default:;
						}
						
				if ( got_space || got_single ) quote = '"';
				if ( got_double ) quote = '\'';
											
				if ( got_single && got_double ) {
					pipe_putStr_err( i_shell, "\pwildcard: Skipping the name " );
					if (k) {
						pipe_putStr_err( i_shell, ref_path );
						pipe_putc_err( i_shell, ':' );
						}
					pipe_putStr_err( i_shell, result_path );
					pipe_putStr_err( i_shell, "\p which contains \" and '.\r" );
					}
				else {
					if ( quote )				// insert string, possibly with quotes
						wild_add( quote );
					if ( k ) {
						for (i = 1; i < k; i++)
							wild_add( ref_path[i] );
						wild_add(':');
						}
					for (i = 1 ; i <= j ; i++ )
						wild_add( result_path[i] );
					if ( quote )
						wild_add( quote );
					wild_add( ' ' );
					found = 1;
					}
				}

			}
		}
	
	if (werr)
		pipe_putStr_err( i_shell, "\pwildcard: Too many matches.\r" );

	return(found);
}

/* ========================================== */

int wild_match( int start, int end, int quote )
{
	int		i,j,k;
	int		is_path;
	char	c;
	Str255	path;
	Str255	pattern;
	
	is_path = 0;
	
	j = k = 0;
	
	for (i = start; i < end; i++) {
		c = gbuf[i];
		path[++j] = c;
		if ( c == ':' ) {
			is_path = 1;
			k = 0;
			}
		else
			pattern[++k] = c;
		}
		
	pattern[0] = k;
	
	if ( is_path )
		path[0] = j - k;
	else
		path[0] = 0;
	
	return ( wild_child( path, pattern, quote ) );
}

/* ========================================== */

void wild_word( void )
{
	int start;
	int	end;
	int quote;
	int	i;
		
	if ( ( wc == '"' ) || ( wc == '\'' ) ) {
		quote = wc;
		wc = wild_nextc();
		}
	else
		quote = 0;
		
	start = wi - 1;
	
	while ( wc && parse_in_body( wc, quote ) )
		wc = wild_nextc();
		
	if (wc)
		end = wi - 1;
	else
		end = wi;
	
	if ( quote != '\'' )
		if (wild_pattern(start,end))
			if (wild_match(start,end,quote))
				return;

	if (quote)
		wild_add(quote);
		
	for (i = start ; i < end ; i ++ )
		wild_add( gbuf[i] );

	if (quote)
		wild_add(quote);
}

/* ========================================== */

void wild_redir( void )
{
	int quote;
	
	while ( wc && (parse_whitespace(wc) || (wc == '>') || (wc == '<') || (wc == '&')) ) {
		wild_add( wc );
		wc = wild_nextc();
		}
		
	if ( ( wc == '"' ) || ( wc == '\'' ) ) {
		wild_add( wc );
		quote = wc;
		wc = wild_nextc();
		}
	else
		quote = 0;
	
	while ( wc && parse_in_body( wc, quote ) ) {
		wild_add( wc );
		wc = wild_nextc();
		}
		
	if (quote)
		wild_add(quote);
}

/* ========================================== */

int wildcard( void )
{
	wi = wo = werr = 0;
	wl = gm;
	
	wc = wild_nextc();
	
	while (wc) {
	
		while ( wc ) {
			if ( ( wc == '"' ) || ( wc == '\'' ) )
				break;	
		 	if ( parse_in_body( wc, 0 ) )
		 		break;
			wild_add( wc );
			wc = wild_nextc();
			}
		
		switch (wc) {
			case '\0':
				break;
			case '>':
			case '<':
				wild_redir();
				break;
			default:
				wild_word();
				break;
			}
			
		if ( ( wc == '"' ) || ( wc == '\'' ) )
			wc = wild_nextc();
	
		}
		
	if (!werr) {
		memcpy( gbuf, wbuf, wo );
		gm = wo;
		}
		
	return( werr );
}

/* ========================================== */