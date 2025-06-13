/* ==========================================

	cmd.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include <stdarg.h>
#include <stdio.h>
#include <resources.h>

#include "multi.h"
#include "nsh.h"
#include "path.h"

#include "cmd.proto.h"
#include "cmd_int.proto.h"
#include "fss.proto.h"
#include "match.proto.h"
#include "path.proto.h"
#include "str_utl.proto.h"
#include "pipe.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"

/* GLOBALS */

ShellH			pshell;			// only one shell is used within this module
t_nshc_calls	nshc_calls;		// callback definitions form commands (READ ONLY)

/* GLOBALS for command caching */

#define		CMD_CACHE_SIZE	6

Str255		g_paths[CMD_CACHE_SIZE];
nshc_hdl	g_cmds[CMD_CACHE_SIZE];
long		g_times[CMD_CACHE_SIZE];
int			g_uses[CMD_CACHE_SIZE];

/* ========== These are the callback routines. ========== */

int cmd_getchar(void)
{
	return( pipe_getc(pshell) );
}

int cmd_gets(char *s, int limit)
{
	return( pipe_gets(pshell, s, limit) );
}
int cmd_getStr(Str255 s)
{
	return( pipe_getStr(pshell, s) );
}

void cmd_putchar(char c)
{
	pipe_putc(pshell,c);
}

void cmd_putchar_err(char c)
{
	pipe_putc_err(pshell,c);
}

void cmd_puts(char *p)
{
	pipe_puts(pshell,p);
}

void cmd_puts_err(char *p)
{
	pipe_puts_err(pshell,p);
}

void cmd_putStr(Str255 s)
{	
	pipe_putStr(pshell,s);
}

void cmd_putStr_err(Str255 s)
{	
	pipe_putStr_err(pshell,s);
}

void cmd_printf(const char *fmt, ...)
{
	char	s[LINE_MAX];
	int		success;
	
	success = vsprintf(s, fmt, __va(fmt));
	
	if (success == EOF)
		pipe_putStr_err(pshell,"\pprintf conversion failed\r");
	else
		pipe_puts(pshell,s);
}

void cmd_printf_err(const char *fmt, ...)
{
	char	s[LINE_MAX];
	int		success;
	
	success = vsprintf(s, fmt, __va(fmt));
	
	if (success == EOF)
		pipe_putStr_err(pshell,"\pprintf conversion failed\r");
	else
		pipe_puts_err(pshell,s);
}

int cmd_var_set( Str32 name, Str255 value )
{
	return( vlist_set( pshell, name, value ) );
}

int cmd_var_unset( Str32 name )
{
	return( vlist_unset( pshell, name ) );
}

int cmd_var_env( Str32 name, Str255 value )
{
	return( vlist_env( pshell, name, value ) );
}

int	cmd_path_expand(Str255 path)
{
	return( path_expand( pshell, path ) );
}

int	cmd_path_to_FSSpec( Str255 pathname, FSSpec *spec )
{
	return( fss_make( pathname, spec ) );
}

int cmd_path_which(Str255 path)
{
	switch( path_which( pshell, path ) ) {
		case which_none:
			return(  1 );
		case which_script:
		case which_internal:
		case which_external:
			return(  0 );
		default:
			return( -1 );
		}
}

void cmd_notify(Str255 s, int size)
{
	Notify(s,size);
}

int cmd_ask(Str255 s, int size)
{
	return(Ask(s,size));
}

int cmd_match( Str255 pattern, Str255 target )
{
	return( match( pattern, target ) );
}

/* ========== These are the callback hooks. ========== */

void cmd_init(void)
{
	int	i;
	
	// init command buffer

	for ( i = 0 ; i < CMD_CACHE_SIZE ; i++ ) {
		g_paths[i][0] = 0;
		g_cmds[i] = 0;
		g_uses[i] = 0;
		g_times[i] = 0;
		}

	// init callbacks

	nshc_calls.NSH_getchar = cmd_getchar;
	nshc_calls.NSH_gets = cmd_gets;
	nshc_calls.NSH_getStr = cmd_getStr;
	nshc_calls.NSH_putchar = cmd_putchar;
	nshc_calls.NSH_putchar_err = cmd_putchar_err;
	nshc_calls.NSH_printf = cmd_printf;
	nshc_calls.NSH_printf_err = cmd_printf_err;
	nshc_calls.NSH_puts = cmd_puts;
	nshc_calls.NSH_puts_err = cmd_puts_err;
	nshc_calls.NSH_putStr = cmd_putStr;
	nshc_calls.NSH_putStr_err = cmd_putStr_err;
	nshc_calls.NSH_var_env = cmd_var_env;
	nshc_calls.NSH_var_set = cmd_var_set;
	nshc_calls.NSH_var_unset = cmd_var_unset;
	nshc_calls.NSH_path_expand = cmd_path_expand;
	nshc_calls.NSH_path_to_FSSpec = cmd_path_to_FSSpec;
	nshc_calls.NSH_path_which = cmd_path_which;
	nshc_calls.NSH_notify = cmd_notify;
	nshc_calls.NSH_ask = cmd_ask;
	nshc_calls.NSH_match = cmd_match;
}

/* ========== Hacking Command Execution. ========== */

#if HACKING

void cmain(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);

void cmd_debugger( void )
{
	t_nshc_parms	*mp;
	
	HLock(pshell);
	TestResError();
		
	mp = &(**pshell).nshc;
	
	cmain(mp,&nshc_calls);   /* call it */
		
	HUnlock(pshell);
	TestResError();
}

#endif

/* ========== External Command Execution. ========== */

void cmd_run( nshc_hdl fp )
{
	t_nshc_parms	*mp;
	
	if (fp) {
	
		HLock(fp);
		TestResError();
		
		HLock(pshell);
		TestResError();
		
		mp = &(**pshell).nshc;
		(**fp)(mp,&nshc_calls);				/* call it */
		
		HUnlock(pshell);
		TestResError();
		
		HUnlock(fp);
		TestResError();
		
		}
	else {
		pipe_putStr_err(pshell,"\pCommand file found, but resource missing\r");
		(**pshell).nshc.action = nsh_idle;
		(**pshell).nshc.result = 1;
		}
}

nshc_hdl cmd_load( void )
{
	short		iFileRef;
	nshc_hdl	fp;
	int			i;
	int			looking;
	int			least_index;
	long		least_ticks;
	
	// if the command is in the cache, return it
	
	for ( i = 0 ; i < CMD_CACHE_SIZE ; i++ )
		if ( pStrEqual( g_paths[i], (**pshell).cmd_path ) ) {
			g_uses[i]++;
			return( g_cmds[i] );
			}
			
	// otherwise, read it from the file

	fp = NULL;

	HLock( pshell );
	iFileRef = OpenResFile( (**pshell).cmd_path );
	HUnlock( pshell );
	
	if ( iFileRef >= 0 ) {
		fp = (nshc_hdl)GetNamedResource( C_CMD, "\pcommand");
		if (fp) DetachResource( fp );
		CloseResFile(iFileRef);
		TestResError();
		}
		
	// bail if the code resource could not be found
		
	if ( !fp )
		return( NULL );
	
	// Otherwise, find the oldest, unused entry.
	
	looking = 1;	// true until one unused command is found
	
	for ( i = 0 ; i < CMD_CACHE_SIZE ; i++ )
		if ( !g_uses[i] ) {
			if ( looking ) {
				least_ticks = g_times[i];
				least_index = i;
				looking = 0;
				}
			if ( least_ticks > g_times[i] ) {
				least_ticks = g_times[i];
				least_index = i;
				}
			}
			
	// dispose oldest command and store new one in its slot
	
	if ( !looking ) {
		DisposHandle( g_cmds[least_index] );
		pStrCopy( g_paths[least_index], (**pshell).cmd_path );
		g_cmds[least_index] = fp;
		g_uses[least_index] = 1;
		}
	
	return( fp );
}

void cmd_unload( void )
{
	int			i;
	nshc_hdl	temp;
	
	temp = (**pshell).cmd_hndl;

	if (temp) {
	
		(**pshell).cmd_hndl = NULL;

		for ( i = 0 ; i < CMD_CACHE_SIZE ; i++ )
			if ( pStrEqual( g_paths[i], (**pshell).cmd_path ) ) {
				g_uses[i]--;
				g_times[i] = TickCount();
				return;
				}
		
		DisposHandle( temp );
		
		
		}
}

void cmd_unload_all( void )
{
	int			i;
	nshc_hdl	temp;

	for ( i = 0 ; i < CMD_CACHE_SIZE ; i++ )
		if ( temp = g_cmds[i] )
			DisposHandle( temp );
}

/* ========== cmd_a_bit ========== */

void cmd_a_bit(ShellH shell)
{
	pshell = shell;				// set up global shell for use by commands
	
	if (!cmd_int())				// if this is an internal command, process it
	
#if HACKING

	cmd_debugger();			// process debug command
	
#else
		
	// otherwise process an external command
	
	switch( (**pshell).nshc.action ) {
		case nsh_start:
			(**pshell).cmd_hndl = cmd_load();
			cmd_run( (**pshell).cmd_hndl );
			break;
		case nsh_continue:
		case nsh_stop:
			cmd_run( (**pshell).cmd_hndl );
			break;
		default:
			break;
		}
		
#endif
				
	if ((**pshell).nshc.action == nsh_idle) {
		
		// pick up results from any previous command
	
		(**(InterpH)((**pshell).interp)).result = (**pshell).nshc.result;
			
		// dispose of any previous command
		
		cmd_unload();
	
		}
}
