
/* ==========================================

	multi.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "resource.h"
#include "interp.h"

#include "cmd.proto.h"
#include "file.proto.h"
#include "flow.proto.h"
#include "interp.proto.h"
#include "multi.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"
#include "wind.proto.h"

int		generation;
int		next_name;

ShellH	shells[MAXSHELL];

extern Boolean	gEventQuit;

/* ========================================== */
// multi-shell low level routines
/* ========================================== */

void multi_init( void )
{
	int	i;
	
	for (i=0; i<MAXSHELL; i++) shells[i] = (ShellH)0;
	
	generation = 0;
	next_name = 1;
}

/* ========================================== */

void multi_dispose(ShellH shell)
{
	int position;
	
	position = (**shell).ShellNo;
	
	if ( interp_dispose( shell ) )
		error_note( "\pmore than one interp record on dispose" );

	TEDispose((**shell).Text);
	DisposeControl( (**shell).Scroll );
	DisposeWindow( (**shell).WindPtr );
	vlist_del_all( shell );
	DisposHandle( shell );
	
	shells[position] = (ShellH)0;
}

/* ========================================== */

ShellH multi_allocate( void )
{
	int		i,pos;
	ShellH	shell;

	for (i=0; i < MAXSHELL; i++ )
		if (!shells[i]) {

			shell = (t_shell_data **)NewHandleClear(sizeof(t_shell_data));
			
			if (!shell) {
				error_note( "\pcould not allocate shell memory" );
				return(shell);
				}
		
			(**shell).ShellNo = i;
			(**shell).action = nsh_start;
			(**shell).nshc.action = nsh_idle;

			if (interp_init( shell )) {
				error_note("\pcould not allocate interpreter data");
				DisposHandle( shell );
				return( shell );
				}
			
			if (path_defaults( shell )) {
				error_note("\pcould not initialize shell variables");
				vlist_del_all( shell );
				DisposHandle( shell );
				return( shell );
				}
			
			if (wind_new( shell, ++generation )) {
				error_note("\pcould not open a new window");
				DisposHandle( shell );
				}
			else
				shells[i] = shell;
				
			return( shell );
			}
			
	error_note( "\pthe maximum number of shells are now open" );
	return(NULL);
}

/* ========================================== */

ShellH multi_find( WindowPtr wind )
{
	int	i = -1;
	int	found = 0;
	
	while ( !found && (i<MAXSHELL-1) )
		if (shells[++i])
			found = wind == (**shells[i]).WindPtr;

	if (found) return(shells[i]); else return(NULL);
}

/* ========================================== */

ShellH multi_front( void )
{
	return( multi_find( FrontWindow() ) );
}

/* ========================================== */
// multi-shell high level routines
/* ========================================== */

void multi_set_name( ShellH shell )
{
	Str255	StrNo;
	Str255  ShellName;
	int		duplicate;
	int		i,this,tries;

	tries = 0;
	this = (**shell).ShellNo;
	
	do {
	
		pStrCopy(ShellName,"\pshell ");
		NumToString( next_name, StrNo );
		pStrAppend( ShellName, StrNo );
	
		duplicate = 0;
		for (i=0;i<MAXSHELL;i++)
			if ( (i != this) && shells[i] )
				if ( pStrEqual( ShellName, (**shells[i]).shell_fss.name ) )
					duplicate = 1;
					
		if (duplicate)
			next_name++;
			
		tries++;
			
	} while (duplicate && (tries < 50));
	
	pStrCopy((**shell).shell_fss.name, ShellName);
	SetWTitle((**shell).WindPtr, ShellName);
	ShowWindow((**shell).WindPtr);
}


/* ========================================== */

void multi_cancel_one( ShellH shell )
{
	if ((**shell).nshc.action) {
		(**shell).nshc.action = nsh_stop;
		cmd_a_bit(shell);
		}

	interp_cancel(shell);
}

/* ========================================== */

int multi_stopped( ShellH shell )
{
	Str255	prompt;
	int 	stopped;

	stopped = 1;

	if ( (**shell).nshc.action || (**(InterpH)(**shell).interp).action ) {
		pStrCopy( prompt, (**shell).shell_fss.name);
		ParamText(prompt, "\p", "\p", "\p");
		SetCursor(&arrow);
		if ( Alert( StopIt, 0L ) == 2 ) {
			multi_cancel_one(shell);
			stopped = 1;
			}
		else
			stopped = 0;
		}
		
	return(stopped);
}

/* ========================================== */

int multi_close( ShellH shell )
{	
	int 	close;
	Str255	prompt;
			
	close = multi_stopped( shell );
	
#if FULL_APP

	if (close && (**shell).Changed) {
		prompt[0] = 0;
		pStrAppend( prompt, "\pSave changes for �");
		pStrAppend( prompt, (**shell).shell_fss.name);
		pStrAppend( prompt, "\p�?");
		switch(Ask(prompt,0)) {
			case askYES:
				close = !mf_save( shell );
			 	break;
			 case askCANCEL:
				close = 0;
				break;
			 case askNO:
				close = 1;
			 	break;
			}
		}

#endif
	
	if(close)
		multi_dispose(shell);
		
	return(close);
}

/* ========================================== */

int multi_close_all( void )
{
	int	i;
	int closed;
	
	i = 0;
	closed = 1;
	
	while ( (i < MAXSHELL ) && closed ) {
		if ( shells[i] )
			closed = multi_close( shells[i] );
		i++;
		}
	
	return(closed);
}


/* ========================================== */
// multi-shell commander routines
/* ========================================== */

void multi_task( void )
{
	int		i;
	ShellH	shell;

	for (i=0; i<MAXSHELL; i++)
		if (shell = shells[i])
			multi_commander(shell);
		
}

/* ========================================== */

void multi_commander( ShellH shell )
{
	int		count;
	InterpH	interp;

	if (shell == multi_front())
		TEIdle((**shell).Text);

	if ((**shell).nshc.action)
		cmd_a_bit(shell);

	if ((**shell).nshc.action)
		return;

	interp = (InterpH)(**shell).interp;
	
	if (interp)
		if ((**interp).action) {
			interp_a_bit(shell);
			if ((**shell).nshc.action)
				cmd_a_bit(shell);
			return;
			}
		
	if ((**shell).action) {
		multi_prompt(shell);
		(**shell).action = nsh_idle;
		}
		
	if (count = multi_has_line(shell)) {
		if ( count < LINE_MAX)
			multi_parse_setup(shell);
		else {
			multi_skip_line(shell);
			pipe_putStr_err( shell, "\pCommand line too long.\r" );
			}
		(**shell).action = nsh_start;
		return;
		}
}

/* ========================================== */

void multi_prompt( ShellH shell )
{
#if FULL_APP

	CharsHandle	hChars;
	int start,end,offset;
	
	offset = 2;
	
	start = (**(**shell).Text).selStart;
	end = (**(**shell).Text).selEnd;

	hChars = TEGetText( (**shell).Text );

	if ((**hChars)[(**shell).Finger-1] != RETURN_KEY ) {
		pipe_putc( shell, RETURN_KEY );
		offset++;
		}
		
	pipe_putc( shell, '%' );
	pipe_putc( shell, ' ' );
	
	TESetSelect(start+offset, end+offset, (**shell).Text);
#else
	gEventQuit = 1;
#endif
}

/* ========================================== */

void multi_cancel( void )
{	
	int		i;
	ShellH	shell;
	
	for (i=0;i<MAXSHELL;i++)
		if (shell = shells[i]) {
			multi_cancel_one(shell);
			(**shell).action = nsh_start;
			}
}

/* ========================================== */
// multi-shell activation routines
/* ========================================== */

void multi_activate( ShellH shell )
{
	TEActivate((**shell).Text);
	ShowControl((**shell).Scroll);
	TEFromScrap();
}

/* ========================================== */

void multi_deactivate( ShellH shell )
{
	TEDeactivate((**shell).Text);
	HideControl((**shell).Scroll);
	ZeroScrap();
	TEToScrap();
}

/* ========================================== */
// keyboard input handler
/* ========================================== */

void multi_key( char theChar )
{
	int		start,end,finger;
	ShellH	shell;
	
	shell = multi_front();
	
	if (shell) {
	
		if (theChar == CONTROL_C) {
			multi_cancel_one( shell );
			pipe_putc_wind( shell, '^' );
			pipe_putc_wind( shell, 'C' );
			pipe_putc_wind( shell, '\r' );
			return;
			}

		if (theChar == NEWLINE_KEY)
			theChar = RETURN_KEY;
			
		start = (**(**shell).Text).selStart;
		end = (**(**shell).Text).selEnd;
		finger = (**shell).Finger;
			
		if (theChar == '\b')
			if ( (start < finger) || ((start == finger) && (end == finger)) )
				return;
	
		multi_discard_text(shell, 1);
		
		if ((theChar < 28) || (theChar > 31)) 	// allow arrow keys anywhere (hack)
			multi_force_insert(shell);
			
		TEKey(theChar, (**shell).Text);
		
		wind_show_sel(shell);

		(**shell).Changed = 1;

		}
}

/* ========================================== */
// multi-shell data extraction routines
/* ========================================== */

void multi_parse_setup(ShellH shell)
{
	char		c;
	int			i,j;
	int			control_d;
	CharsHandle	hChars;
	InterpH		interp;
	
	control_d = 0;

	if (shell) {
	
		interp = (InterpH)(**shell).interp;
		
		if (interp) {
		
			j = 0;
			i = (**shell).Finger;
			hChars = TEGetText( (**shell).Text );
		
			do {
				c = (**hChars)[i++];
				if( (c == RETURN_KEY) || (c == CONTROL_D) ) {
					if (c == CONTROL_D)
						control_d = 1;
					c = '\0';
					}
				else
					(**interp).fbuf[j++] = c;
			} while (c);
		
			(**shell).Finger = i;
			(**interp).fbuf_chars = j;
			(**interp).fbuf_ptr = 0;
			(**interp).source = 0;
			(**interp).action = nsh_start;
			
			if (control_d) {
				pipe_putc_wind( shell, '^' );
				pipe_putc_wind( shell, 'D' );
				pipe_putc_wind( shell, '\r' );
				}
			}
		
		}
}

/* ========================================== */

void multi_skip_line(ShellH shell)
{
	char		c;
	int			i;
	CharsHandle	hChars;

	if (shell) {
		i = (**shell).Finger;
		hChars = TEGetText( (**shell).Text );
		
		do 
			c = (**hChars)[i++];
		while ( (c != RETURN_KEY) && ( c != CONTROL_D ) );
		
		(**shell).Finger = i;
		
		if (c == CONTROL_D) {
			pipe_putc_wind( shell, '^' );
			pipe_putc_wind( shell, 'D' );
			pipe_putc_wind( shell, '\r' );
			}
		}
}

/* ========================================== */

int multi_fetch_line(ShellH shell, char *buf, int *count)
{
	char		c;
	int			i;
	int			limit;
	CharsHandle	hChars;

	int			j = 0;
	int			control_d = 0;
	int			found = 0;

	if (shell) {
	
		limit = multi_has_line(shell);
		
		if (!limit) return( 0 );
		
		if ( *count < limit ) limit = *count;
		
		i = (**shell).Finger;
		hChars = TEGetText( (**shell).Text );
		
		do {
			c = (**hChars)[i++];
			if( c == RETURN_KEY )
				found++;
			if( c == CONTROL_D ) {
				found++;
				control_d++;
				}
			else {
				*buf++ = c;
				j++;
				}
		} while (( !found ) && ( j < limit ));
		
		(**shell).Finger = i;
		}
		
	*count = j;
	
	if (control_d) {
		pipe_putc_wind( shell, '^' );
		pipe_putc_wind( shell, 'D' );
		pipe_putc_wind( shell, '\r' );
		return( -1 );
		}
	else
		return( j );
}

/* ========================================== */

int multi_has_line(ShellH shell)
{
	char		c;
	int			i;
	int			last;
	CharsHandle	hChars;

	int			count = 0;
	int			found = 0;

	if (shell) {
	
		i = (**shell).Finger;
		last = (**(**shell).Text).teLength;
		hChars = TEGetText( (**shell).Text );
		
		while ((i<last) && (!found)) {
			c = (**hChars)[i++];
			if( (c == RETURN_KEY) ||  (c == CONTROL_D) )
				found++;
			count++;
			}
		}

	if (found)
		return( count );
	else
		return( 0 );
}

/* ========================================== */
// multi-shell utiliity routines
/* ========================================== */

void multi_discard_text(ShellH shell, int req_chars)
{
	long	last;
	int		del_size,old_start,old_end;
	
	last = (**(**shell).Text).teLength;

	if ((last + req_chars) > SHELL_CHARS) {
		del_size = (last - SHELL_CHARS) + req_chars;
		if (del_size < 2000) del_size = 2000;
		old_start = (**(**shell).Text).selStart - del_size;
		old_end = (**(**shell).Text).selEnd - del_size;
		TESetSelect(0, del_size, (**shell).Text);
		TEDelete((**shell).Text);
		TESetSelect(old_start, old_end, (**shell).Text);
		(**shell).Finger -= del_size;
		}
}

/* ========================================== */

int multi_can_insert( ShellH shell )
{
	return( (**(**shell).Text).selStart >= (**shell).Finger );
}

/* ========================================== */

void multi_force_insert( ShellH shell )
{	
	if (!multi_can_insert(shell))
		TESetSelect(32767, 32767, (**shell).Text);
}
