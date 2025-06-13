/* ==========================================

	cmd_int.c - internal commands
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"

#include "interp.h"
#include "nsh.h"
#include "path.h"

#include "cmd.proto.h"
#include "cmd_int.proto.h"
#include "fss.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "buf_utl.proto.h"
#include "str_utl.proto.h"
#include "vlist.proto.h"

/* local types and structures */
				
typedef struct {
	Str32		name;
	int			value; } t_cmd_array;
				
t_cmd_array g_int_cmds[] = { 
	{ "\pset",		int_cmd_set },
	{ "\punset",	int_cmd_unset },
	{ "\penv",		int_cmd_env },
	{ "\ppathchk",	int_cmd_pathchk },
	{ "\pwhich",	int_cmd_which },
	{ "\pcd",		int_cmd_cd },
	{ "\ppwd",		int_cmd_pwd },
	{ "\ppath",		int_cmd_path },
	{ "\pexit",		int_cmd_exit },
	{ "\pexport",	int_cmd_export },
	{ "\p",			0 } };

/* GLOBALS */

extern	ShellH			pshell;			// only one shell is used within this module
extern	t_nshc_calls	nshc_calls;		// callback definitions form commands (READ ONLY)

/* ============================================= */

int	cmd_int_vname( int arg, Str32 name )
{
	int		index;
	int		count;
	char	c;
	
	count = 0;
	index = (**pshell).nshc.argv[arg];
	
	while ((count < 31) && (c = (**pshell).nshc.arg_buf[index++])) {
		if (vlist_char(c))
			name[++count] = c;
		else
			count = 32;
		}
	
	if (count < 31) {
		name[0] = count;
		return(1);
		}
	else {
		name[0] = 0;
		return(0);
		}
}

/* ========== Internal "set" Command. ========== */

void cmd_int_set( void )
{
	int			argc;
	int			error;
	Str32		name;
	Str255		value;
	
	error = NSHC_ERR_GENERAL;
	
	argc = (**pshell).nshc.argc;
	
	if ( (argc <2) || (argc > 3)) {
		cmd_putStr_err("\pUsage: set variable_name ""value""\r");
		error = NSHC_ERR_PARMS;
		}
	else {
		if ( cmd_int_vname( 1, name ) ) {
			if (argc == 2)
				value[0] = 0;
			else
				pStrFromC( value, &(**pshell).nshc.arg_buf[(**pshell).nshc.argv[2]] );
			error = cmd_var_set( name, value );
			}
		else {
			cmd_putStr_err("\pset: Bad variable name\r");
			error = NSHC_ERR_PARMS;
			}
		}

	cmd_int_return( error );
}

/* ========== Internal "unset" Command. ========== */

int cmd_int_unset_one( int arg )
{
	Str32	name;

	if ( cmd_int_vname( arg, name ) ) {
		cmd_var_unset( name );
		return(NSHC_NO_ERR);
		}
	else {
		cmd_putStr_err("\punset: Bad variable name\r");
		return(NSHC_ERR_PARMS);
		}
}

void cmd_int_unset( void )
{
	int i;
	int argc;
	int error;
	
	error = NSHC_NO_ERR;
	
	argc = (**pshell).nshc.argc;

	if (argc == 1) {
		cmd_putStr_err("\pUsage: unset variable_name [variable_name]\r");
		error = NSHC_ERR_PARMS;
		}
	else
		for (i=1; (( i < argc ) && ( !error )) ; i++)
			error = cmd_int_unset_one( i );

	cmd_int_return( error );
}

/* ========== Internal "env" Command. ========== */

void cmd_int_env_print( Str32 name, Str255 value )
{
	buf_putStr(name);	// could combine into a line buf for better speed
	buf_putchar('=');
	buf_putchar('"');
	buf_putStr(value);
	buf_putchar('"');
	buf_putchar('\r');
}

void cmd_int_env_all( void )
{
	Str32		name;
	Str255		value;
	t_vl_hndl	this;
	
	this = NULL;
	
	buf_init(&nshc_calls);

	do {

		this = vlist_next( pshell, this, name, value  );
			
		if (this) cmd_int_env_print( name, value );
				
	} while (this);

	buf_flush();
}

int cmd_int_env_one( int arg )
{
	int		error;
	int		found;
	Str32	name;
	Str255	value;
	
	error = NSHC_NO_ERR;
	found = 0;

	if ( cmd_int_vname( arg, name ) )
		found = !cmd_var_env( name, value );
	else {
		cmd_putStr_err("\penv: Bad variable name\r");
		error = NSHC_ERR_PARMS;
		}

	if (found && !error) {
		buf_init(&nshc_calls);
		cmd_int_env_print( name, value );
		buf_flush();
		}
		
	return(error);
}

void cmd_int_env( void )
{
	int i;
	int argc;
	int error;
	
	error = NSHC_NO_ERR;

	argc = (**pshell).nshc.argc;

	if (argc == 1)
		cmd_int_env_all();
	else
		for (i=1; (( i < argc ) && ( !error )) ; i++)
			error = cmd_int_env_one( i );

	cmd_int_return( error );
}

/* ========== Internal "which" Command. ========== */

void cmd_int_which( void )
{
	char		c;
	int			i,j;
	int			error;
	int			overrun;
	Str255		initial;
	t_int_cmd	command;
	
	overrun = 0;
	error = NSHC_ERR_GENERAL;

	if ((**pshell).nshc.argc == 2) {
			
		j = 0;
		i = (**pshell).nshc.argv[1];
	
		while ( c = (**pshell).nshc.arg_buf[i++] )
			if (j < 255)
				initial[++j] = c;
			else
				overrun = 1;

		if (overrun) {
			cmd_putStr_err("\pwhich: Command path too long.\r");
			error = NSHC_ERR_PARMS;
			}
		else {
			initial[0] = j;
			
			switch ( path_which(pshell, initial) ) {
				case which_internal:
					cmd_putStr(initial);
					cmd_putStr("\p is an internal command.\r");
					error = NSHC_NO_ERR;
					break;
				case which_external:
				case which_script:
					cmd_putStr(initial);
					cmd_putchar('\r');
					error = NSHC_NO_ERR;
					break;
				default:
					cmd_putStr("\pwhich: Could not locate this command.\r");
					break;
				}
			}
		}
	else {
		cmd_putStr_err("\pUsage: which command_name\r");
		error = NSHC_ERR_PARMS;
		}

	cmd_int_return( error );
}

/* ========== Internal "pathchk" Command. ========== */

void cmd_int_pathchk( void )
{
	char	c;
	int		i,j;
	int		error;
	int		overrun;
	Str255  initial;
	
	overrun = 0;
	error = NSHC_ERR_GENERAL;
	
	if ((**pshell).nshc.argc != 2) {
		cmd_putStr_err("\pUsage: pathchk file_name\r");
		cmd_int_return( NSHC_ERR_PARMS );
		return;
		}

	j = 0;
	i = (**pshell).nshc.argv[1];
	
	while ( c = (**pshell).nshc.arg_buf[i++] )
		if (j < 255)
			initial[++j] = c;
		else
			overrun = 1;
	
	if (overrun) {
		cmd_putStr_err("\ppathchk: Path is too long.\r");
		error = NSHC_ERR_PARMS;
		}
	else {
		initial[0] = j;
		
		if ( error = path_expand( pshell, initial ))
			cmd_putStr_err("\ppathchk: Error in path expansion.\r");
		else {
			cmd_putStr(initial);
			cmd_putchar('\r');
			}
		}
		
	cmd_int_return( error );
}

/* ========== Internal "cd" Command. ========== */

void cmd_int_cd( void )
{
	int			argc;
	Str255		value;
	int			error;
	
	error = NSHC_ERR_GENERAL;
	
	argc = (**pshell).nshc.argc;
	
	if ( argc > 2 ) {
		cmd_putStr_err("\pUsage: cd ""pathname""\r");
		error = NSHC_ERR_PARMS;
		}
	else {
		if ( argc == 1 ) {
			if (cmd_var_env( "\pHOME", value ))
				cmd_putStr_err("\pcd: HOME is not defined\r");
			}
		else
			pStrFromC( value, &(**pshell).nshc.arg_buf[(**pshell).nshc.argv[1]] );
			
		if ( value[0] )
			if (path_expand( pshell, value ))
				cmd_putStr_err("\pcd: Bad pathnam\r");
			else
				if ( !value[0] )
					cmd_putStr_err("\pcd: Directory path must contain a volume name.\r");
				else {
					if (value[value[0]] != ':')
						value[(++value[0])] = ':';
					if ( path_is_dir( value ) )
						error = cmd_var_set( "\pPWD", value );
					else
						cmd_putStr_err("\pcd: Directory not found.\r");
					}
		}

	cmd_int_return( error );
}

/* ========== Internal "pwd" Command. ========== */

void cmd_int_pwd( void )
{
	Str255		value;
	int			error;
	
	error = cmd_var_env( "\pPWD", value );
	
	if (!error) {
		cmd_putStr(value);
		cmd_putchar('\r');
		}
	
	cmd_int_return( error );
}

/* ========== Internal "path" Command. ========== */

int cmd_int_path_check( ShellH shell, Str255 path, int loud )
{
	char	c;
	int		count, finger, limit, success;
	Str255	one;
	
	success	= 1;
	finger	= 1;
	limit	= path[0];
	
	while (success && (finger <= limit)) {
		count = 0;
		while (( (c = path[finger]) != '/') && (finger <= limit)) {
			one[++count] = c;
			finger++;
			}
		if (count) {
			one[0] = count;
			success = !path_expand( shell, one );
			if (success)
				success = path_is_dir( one );
			}
		finger++;
		}
		
	if (!success && loud ) {
		pipe_putStr_err( shell, "\ppath: Could not find path = \"");
		pipe_putStr_err( shell, one);
		pipe_putc_err( shell, '"');
		}
	
	return( success );
}

void cmd_int_path( void )
{
	int			argc;
	Str255		value;
	int			error;
	
	error = NSHC_ERR_GENERAL;
	
	argc = (**pshell).nshc.argc;
	
	if ( argc > 2 ) {
		cmd_putStr_err("\pUsage: path [""path1/path2/...""]\r");
		error = NSHC_ERR_PARMS;
		}
	else
		if ( argc == 1) {
			error = cmd_var_env( "\pPATH", value );
			if (!error) {
				cmd_putStr(value);
				cmd_putchar('\r');
				}
			}
		else {
			pStrFromC( value, &(**pshell).nshc.arg_buf[(**pshell).nshc.argv[1]] );
			if (cmd_int_path_check( pshell, value, 1 ))
				error = cmd_var_set( "\pPATH", value );			
			}

	cmd_int_return( error );
}

/* ========== Internal "exit" Command. ========== */

void cmd_int_exit( void )
{
	char	c;
	char	*p,*q;
	long	result;
	int		negative;
	int		arg;

	(**(InterpH)(**pshell).interp).action = nsh_stop;

	arg = (**pshell).nshc.argc;
	
	if (arg == 1) {
		cmd_int_return( 0 );
		return;
		}
		
	if (arg > 2) {
		cmd_putStr_err("\pUsage: exit [return_status]\r");
		cmd_int_return( NSHC_ERR_PARMS );
		return;
		}
	
	p = q = &(**pshell).nshc.arg_buf[(**pshell).nshc.argv[1]];
	
	c = *p;
	
	if ( c == '-' )
		p++;
	
	while (c = *p++)
		if (( c < '0') || (c > '9')) {
			cmd_putStr_err("\pexit: Return status must be an integer.\r");
			cmd_int_return( NSHC_ERR_PARMS );
			return;
			}
	
	result = 0;
	negative = 0;
	
	c = *q;
	
	if ( c == '-' ) {
		negative = 1;
		q++;
		}
	
	while (c = *q++) {
		result *= 10;
		result += (c - '0');
		}
	
	if (negative)
		result *= -1;
		
	if ((result < -32766) || (result > 32767)) {
			cmd_putStr_err("\pexit: Return status is too large.\r");
			cmd_int_return( NSHC_ERR_PARMS );
			return;
			}

		
	cmd_int_return( result );
}

/* ========== Internal "export" Command. ========== */

void cmd_int_export( void )
{
	Str32	name;
	Str255	value;
	char	c;
	int		i,j;
	int		error;
	int		overrun;
	
	overrun = 0;
	error = 0;

	if ((**pshell).nshc.argc != 2) {
		cmd_putStr_err("\pUsage: export variable_name\r");
		error = NSHC_ERR_PARMS;
		}
	else {
			
		j = 0;
		i = (**pshell).nshc.argv[1];
	
		while ( c = (**pshell).nshc.arg_buf[i++] )
			if (j < 31)
				name[++j] = c;
			else
				overrun = 1;

		if (overrun) {
			cmd_putStr_err("\pexport: Variable name is too long.\r");
			error = NSHC_ERR_PARMS;
			}
		else {
			name[0] = j;
			
			error = vlist_env( pshell, name, value );
			
			if (error) {
				cmd_putStr_err("\pexport: Variable not declared = ");
				cmd_putStr_err(name);
				cmd_putchar_err('\r');
				cmd_int_return( NSHC_ERR_PARMS );
				return;
				}
			
			error = vlist_export( pshell, name, value );
			
			if (error == -1)
				cmd_putStr_err("\pexport: Script data not found.\r");
			else
				if (error)
					cmd_putStr_err("\pexport: Operation failed.\r");
			
			}
				
		}
		
	cmd_int_return( error );
}

/* ============================================= */

// Utility

int cmd_int_check( Str255 name )
{
	int			i;
	t_int_cmd	command;
	
	i = 0;
	command = int_cmd_none;
	
	while ( (command == int_cmd_none) && (g_int_cmds[i].value) )
		if (pStrEqual(g_int_cmds[i].name, name))
			command = g_int_cmds[i].value;
		else
			i++;
		
	return(command);
}

void cmd_int_return( int result )
{
	(**pshell).nshc.action = nsh_idle;
	(**pshell).nshc.result = result;
}

/* ============================================= */

// Internal Command Execution.

int cmd_int(void)
{
	t_int_cmd	command;
	
	command = cmd_int_check( (**pshell).cmd_path );
		
	switch (command) {
		case int_cmd_set:
			cmd_int_set();
			break;
		case int_cmd_unset:
			cmd_int_unset();
			break;
		case int_cmd_env:
			cmd_int_env();
			break;
		case int_cmd_pathchk:
			cmd_int_pathchk();
			break;
		case int_cmd_which:
			cmd_int_which();
			break;
		case int_cmd_cd:
			cmd_int_cd();
			break;
		case int_cmd_pwd:
			cmd_int_pwd();
			break;
		case int_cmd_path:
			cmd_int_path();
			break;
		case int_cmd_exit:
			cmd_int_exit();
			break;
		case int_cmd_export:
			cmd_int_export();
			break;
		default:
			break;
		}
	
	return( command != int_cmd_none );
}
