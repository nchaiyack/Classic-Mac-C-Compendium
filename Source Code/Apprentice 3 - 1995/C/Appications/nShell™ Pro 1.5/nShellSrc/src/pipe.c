/* ==========================================

	pipe.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "pipe.h"

#include "fss.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "multi.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"
#include "wind.proto.h"

extern	Boolean	gHasFSSpec;	// true if the calls are available

/* ========================================== */

// pipe utility routines

void pipe_make_name( ShellH shell, Str255 path )
{
	Str32	ticks;
	
	vlist_env( shell, "\pTMP", path );
	
	pStrAppend( path, "\ptmp." );
	
	NumToString( TickCount(), ticks );
	pStrAppend( path, ticks );
}
	

/* ========================================== */

// pipe redirection routines

OSErr pipe_open_input( ShellH shell, Str255 filename, int inState, int scope )
{
	OSErr	error;
	int		fRef;
	InterpH	interp;
	
	if (scope == redir_by_script)
		interp = (InterpH)(**shell).interp;
	else
		interp = (InterpH)(**shell).root_interp;
	
	if ((**interp).inState) {
		pipe_putStr_err( shell, "\ppipe: A stdin file is already open\r" );
		return( NSHC_ERR_GENERAL );
		}
	
	error = path_expand(shell, filename);
	
	if (!error)
		error = FSOpen( filename, 0, &fRef );
	
	if (error) {
		pipe_putStr_err( shell, "\ppipe: Could not open stdin\r" );
		return( error );
		}
	
	pStrCopy( (**interp).inName, filename );
	(**interp).inRefNum = fRef;
	(**interp).inState = inState;
	(**interp).inBufChars = 0;
	(**interp).inBufPtr = 0;
	(**interp).inBufEOF = 0;
	
	return( noErr );
}

void pipe_close_input( ShellH shell, int scope )
{
	OSErr	error;
	InterpH	interp;
	
	if (scope == redir_by_script)
		interp = (InterpH)(**shell).interp;
	else
		interp = (InterpH)(**shell).root_interp;
	
	if ((**interp).inState) {
	
		error = FSClose((**interp).inRefNum);
	
		if (error) {
			pipe_putStr_err( shell, "\ppipe: Could not close stdin\r" );
			return;
			}
		}
		
	(**interp).inName[0] = 0;
	(**interp).inRefNum = 0;
	(**interp).inState = redir_none;
}

void pipe_delete_input( ShellH shell, int scope )
{
	Str255	path;
	FSSpec	fss;
	OSErr	error;
	InterpH	interp;
	
	if (scope == redir_by_script)
		interp = (InterpH)(**shell).interp;
	else
		interp = (InterpH)(**shell).root_interp;
	
	if ((**interp).inState) {
	
		pStrCopy( path, (**interp).inName );
		pipe_close_input( shell, scope );
		
		if ((**interp).result >= 0) {
			error = fss_make( path, &fss );
			if (!error) error = fss_Delete(gHasFSSpec, &fss);
			if (error) pipe_putStr_err( shell, "\ppipe: Could not delete pipe file.\r" );
			}
		}
}

OSErr pipe_open_output( ShellH shell, Str255 filename, int outState, int scope, int append )
{
	OSErr	error;
	int		fRef;
	InterpH	interp;
	
	if (scope == redir_by_script)
		interp = (InterpH)(**shell).interp;
	else
		interp = (InterpH)(**shell).root_interp;
		
	if ((**interp).outState) {
		pipe_putStr_err( shell, "\ppipe: A stdout file is already open\r" );
		return( NSHC_ERR_GENERAL );
		}
	
	if (outState >= redir_out_null) {
		(**interp).outState = outState;
		return( noErr );
		}

	if ( fRef = pipe_open_util(shell, filename, append) ) {
		
		if (append)
			error = SetFPos(fRef,fsFromLEOF,0);
		else
			error = SetEOF(fRef,0);
		
		if (error)
			pipe_putStr_err( shell, "\ppipe: Could not set file position for stdout\r" );
		else {
			pStrCopy( (**interp).outName, filename );
			(**interp).outRefNum = fRef;
			(**interp).outState = outState;
			}

		return( error );
		}
			
	return( NSHC_ERR_GENERAL );
}

void pipe_close_output( ShellH shell, int scope )
{
	OSErr	error;
	int		state;
	short	fRef;
	InterpH	interp;
	
	if (scope == redir_by_script)
		interp = (InterpH)(**shell).interp;
	else
		interp = (InterpH)(**shell).root_interp;
	
	fRef = (**interp).outRefNum;
	
	if (fRef) {
	
		error = FSClose(fRef);
	
		if (error) {
			pipe_putStr_err( shell, "\ppipe: Could not close stdout\r" );
			return;
			}
		}
		
	(**interp).outName[0] = 0;
	(**interp).outRefNum = 0;
	(**interp).outState = redir_none;
}

int pipe_open_util( ShellH shell, Str255 filename, int append )
{
	OSErr	error;
	int		fRef;
	
	error = fRef = 0;
	
	if (!path_expand(shell, filename)) {
	
		if (!append) {
			error = FSDelete(filename, 0);
			if (!error || (error == fnfErr))
				error = Create(filename,0,TEACHTEXT_SIGNATURE,'TEXT');
			}

		error = FSOpen( filename, 0, &fRef );
	
		if (error == fnfErr) {
			error = Create(filename,0,TEACHTEXT_SIGNATURE,'TEXT');
			if (!error)
				error = FSOpen( filename, 0, &fRef );
			}
	
		if (error) {
			pipe_putStr_err( shell, "\ppipe: Could not create stdout\r" );
			return(0);
			}
			
		}
	
	return( fRef );
}

/* ========================================== */

// pipe input routines

int pipe_getc(ShellH shell)
{
	InterpH	interp;
	
	interp = (InterpH)(**shell).root_interp;

	if (!(**interp).inState)
		interp = (InterpH)(**shell).interp;
	
	if ( (**interp).inState )
		return( pipe_getc_file( shell, interp ) );
	else
		return( pipe_getc_wind( shell ) );
}

int pipe_gets(ShellH shell, char *s, int limit)
{	
	int		count, result;
	InterpH	interp;
	
	count = limit - 1;

	interp = (InterpH)(**shell).root_interp;

	if (!(**interp).inState)
		interp = (InterpH)(**shell).interp;
	
	if ( (**interp).inState )
		result = pipe_fetch_line( shell, interp, s, &count );
	else
		result = multi_fetch_line( shell, s, &count );
	
	if (result) s[count] = '\0';
	
	return( result );
}

int pipe_getStr(ShellH shell, Str255 s)
{
	int		count, result;
	InterpH	interp;
	
	count = 255;
	
	interp = (InterpH)(**shell).root_interp;

	if (!(**interp).inState)
		interp = (InterpH)(**shell).interp;
	
	if ( (**interp).inState )
		result = pipe_fetch_line( shell, interp, (char *)&(s[1]), &count );
	else 
		result = multi_fetch_line( shell, (char *)&(s[1]), &count );
	
	if (result) s[0] = count;
			
	return( result );
}

/* ========================================== */

// pipe input from window

int pipe_getc_wind(ShellH shell)
{
	int			result;
	int			last;
	int			finger;
	CharsHandle	hChars;
	
	result = 0;
		
	last = (**(**shell).Text).teLength;
	finger = (**shell).Finger;

	if ( finger < last ) {
		hChars = TEGetText( (**shell).Text );
		result = (**hChars)[finger++];
		(**shell).Finger = finger;
		}
	
	if ( result == CONTROL_D ) {
		pipe_putc_wind( shell, '^' );
		pipe_putc_wind( shell, 'D' );
		pipe_putc_wind( shell, '\r' );
		return( -1 );
		}

	return( result );
}

/* ========================================== */

// pipe stdin from file

int pipe_getc_file(ShellH shell, InterpH interp)
{
	int		i;
	char	c;
	OSErr	error;
	long	my_count;
	
	if (!(**interp).inRefNum) {
		pipe_putStr_err( shell, "\ppipe: stdin file is not open\r" );
		return(-1);
		}

	if ( !(**interp).inBufChars || ( (**interp).inBufPtr >= (**interp).inBufChars ) ) {
	
		(**interp).inBufChars = 0;
		(**interp).inBufPtr = 0;

		if ( (**interp).inBufEOF )
			return(-1);

		my_count = INBUF_SIZE;
		error = FSRead( (**interp).inRefNum, &my_count, (**interp).inBuf );
		
		if (!error || (error == eofErr))
			(**interp).inBufChars = my_count;
		
		if (error == eofErr) {
			(**interp).inBufEOF = 1;
			error = 0;
			}
		}
		
	if (error) {
		pipe_putStr_err( shell, "\ppipe: Could not get char on stdin\r" );
		return( -1 );
		}
		
	if ( (**interp).inBufPtr < (**interp).inBufChars )
		return ( (**interp).inBuf[ (**interp).inBufPtr++ ] );
	else
		return( -1 );
}

int pipe_fetch_line(ShellH shell, InterpH interp, char *buf, int *count)
{
	int		i;
	char	c;
	OSErr	error;
	long	my_count;
	
	if (!(**interp).inRefNum) {
		pipe_putStr_err( shell, "\ppipe: stdin file is not open\r" );
		return(-1);
		}
		
	i = 0;

	do {

		if ( (**interp).inBufChars )
			while ( ( (**interp).inBufPtr < (**interp).inBufChars ) && ( i < *count ) ) {
				c = (**interp).inBuf[ (**interp).inBufPtr++ ];
				*buf++ = c;
				i++;
				if ( c == '\r' ) {
					*count = i;
					return( i );
					}
				}
		
		if ( i < *count ) {
		
			(**interp).inBufChars = 0;
			(**interp).inBufPtr = 0;

			if ( (**interp).inBufEOF ) {
				*count = i;
				return(-1);
				}

			my_count = INBUF_SIZE;
			error = FSRead( (**interp).inRefNum, &my_count, (**interp).inBuf );
			
			if (!error || (error == eofErr))
				(**interp).inBufChars = my_count;
			
			if (error == eofErr) {
				(**interp).inBufEOF = 1;
				error = 0;
				}
				
			}
	
	} while ( !error && (i < *count ));
	
	if (error) {
		pipe_putStr_err( shell, "\ppipe: Could not get char on stdin\r" );
		return( -1 );
		}
		
	return(i);
}

/* ========================================== */

// pipe stdout routines

void pipe_putc( ShellH shell, char ch )
{
	InterpH	interp;
	
	interp = (InterpH)(**shell).root_interp;

	if (!(**interp).outState)
		interp = (InterpH)(**shell).interp;

	switch ((**interp).outState) {
		case redir_none:
		case redir_out_tty:
		case redir_outerr_tty:
			pipe_putc_wind( shell, ch );
			break;
		case redir_out:
		case redir_outerr:
		case redir_out_pipe:
		case redir_outerr_pipe:
			pipe_putc_file( shell, interp, ch );
			break;
		default:
			break;
		}
}

void pipe_puts( ShellH shell, char *text )
{
	char	*p;
	int		length;
	
	p=text;
	length = 0;
	while (*p++) length++;
	
	pipe_puts_util( shell, text, length );
}

void pipe_putStr( ShellH shell, Str255 string )
{
	pipe_puts_util( shell, (char *)&string[1], string[0] );
}

void pipe_puts_util( ShellH shell, char *p, long count )
{
	InterpH	interp;
	
	interp = (InterpH)(**shell).root_interp;

	if (!(**interp).outState)
		interp = (InterpH)(**shell).interp;

	switch ((**interp).outState) {
		case redir_none:
		case redir_out_tty:
		case redir_outerr_tty:
			pipe_puts_wind( shell, p, count );
			break;
		case redir_out:
		case redir_outerr:
		case redir_out_pipe:
		case redir_outerr_pipe:
			pipe_puts_file( shell, interp, p, count );
			break;
		default:
			break;
		}
}

/* ========================================== */

// pipe stderr routines

void pipe_putc_err( ShellH shell, char ch )
{
	int		state;
	InterpH	interp;
	
	interp = (InterpH)(**shell).root_interp;

	state = (**interp).outState;

	switch (state) {
		case redir_none:
		case redir_out:
		case redir_out_pipe:
		case redir_out_tty:
		case redir_out_null:
			interp = (InterpH)(**shell).interp;
			state = (**interp).outState;
			break;
		default:
			break;
		}
		
	switch (state) {
		case redir_none:
		case redir_out:
		case redir_out_pipe:
		case redir_out_tty:
		case redir_out_null:
		case redir_outerr_tty:
			pipe_putc_wind( shell, ch );
			break;
		case redir_outerr:
		case redir_outerr_pipe:
			pipe_putc_file( shell, interp, ch );
			break;
		default:
			break;
		}
}

void pipe_puts_err( ShellH shell, char *text )
{
	char	*p;
	int		length;
	
	p=text;
	length = 0;
	while (*p++) length++;
	
	pipe_puts_err_util( shell, text, length );
}

void pipe_putStr_err( ShellH shell, Str255 string )
{
	pipe_puts_err_util( shell, (char *)&string[1], string[0] );
}

void pipe_puts_err_util( ShellH shell, char *p, long count )
{
	int		state;
	InterpH	interp;
	
	interp = (InterpH)(**shell).root_interp;
	
	state = (**interp).outState;

	switch (state) {
		case redir_none:
		case redir_out:
		case redir_out_pipe:
		case redir_out_tty:
		case redir_out_null:
			interp = (InterpH)(**shell).interp;
			state = (**interp).outState;
			break;
		default:
			break;
		}
		
	switch (state) {
		case redir_none:
		case redir_out:
		case redir_out_pipe:
		case redir_out_tty:
		case redir_out_null:
		case redir_outerr_tty:
			pipe_puts_wind( shell, p, count );
			break;
		case redir_outerr:
		case redir_outerr_pipe:
			pipe_puts_file( shell, interp, p, count );
			break;
		default:
			break;
		}
}

/* ========================================== */

// pipe output to window

void pipe_putc_wind( ShellH shell, char ch )
{
	int		finger;
	
	multi_discard_text(shell, 1);
	finger = (**shell).Finger;
	TESetSelect( finger, finger, (**shell).Text );
	TEKey(ch, (**shell).Text);
	(**shell).Finger = finger + 1;
	wind_show_sel(shell);
}

void pipe_puts_wind( ShellH shell, char *p, long count )
{
	int		finger;

	multi_discard_text(shell, count);
	finger = (**shell).Finger;
	TESetSelect( finger, finger, (**shell).Text );
	TEInsert( p, count, (**shell).Text );
	(**shell).Finger = (**(**shell).Text).selStart;
	wind_show_sel(shell);
}

/* ========================================== */

// pipe output to file

void pipe_putc_file( ShellH shell, InterpH interp, char ch )
{
	OSErr	error;
	long	count = 1;
	
	if (!(**interp).outRefNum) {
		pipe_putStr_err( shell, "\ppipe: stdout file is not open\r" );
		return;
		}
	
	error = FSWrite((**interp).outRefNum,&count,&ch);
	
	if (error)
		pipe_putStr_err( shell, "\ppipe: Could not put char on stdout\r" );
}

void pipe_puts_file( ShellH shell, InterpH interp, char *p, long count )
{
	OSErr	error;
	
	if (!(**interp).outRefNum) {
		pipe_putStr_err( shell, "\ppipe: stdout file is not open\r" );
		return;
		}
	
	error = FSWrite((**interp).outRefNum,&count,p);
	
	if (error)
		pipe_putStr_err( shell, "\ppipe: Could not put string on stdout\r" );
}




