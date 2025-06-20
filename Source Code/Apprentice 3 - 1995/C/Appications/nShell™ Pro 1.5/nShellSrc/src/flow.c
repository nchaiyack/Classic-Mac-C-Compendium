/* ==========================================

	flow.c - control of flow commands
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include "nsh.h"

#include "flow.h"
#include "multi.h"
#include "interp.h"

#include "flow.proto.h"
#include "parse.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"

/* local types and structures */
				
typedef struct {
	Str32		name;
	int			value; } t_cmd_array;
				
t_cmd_array g_flow_cmds[] = { 
	{ "\pif",		flow_cmd_if },
	{ "\pthen",		flow_cmd_then },
	{ "\pelse",		flow_cmd_else },
	{ "\pendif",	flow_cmd_endif },
	{ "\pwhile",	flow_cmd_while },
	{ "\puntil",	flow_cmd_until },
	{ "\pdo",		flow_cmd_do },
	{ "\pdone",		flow_cmd_done },
	{ "\p", 0 } };


/* ============================================= */

extern	InterpH	i_interp;
extern	ShellH	i_shell;

/* ============================================= */

t_fl_hndl flist_add( InterpH interp );
void flist_del( InterpH interp, t_fl_hndl this );
t_fl_hndl flist_find_type( InterpH interp, int type, int slot );
t_fl_hndl flist_find_do( InterpH interp, int slot );
long flow_position( void );

/* ============================================= */

// flow_list management routines:

/* ============================================= */

#if PROFESSIONAL

/* ============================================= */

t_fl_hndl flist_add( InterpH interp )
{
	int			i;
	t_fl_hndl	temp;
	t_fl_hndl	this;
		
	this = (t_fl_hndl)NewHandleClear( sizeof( t_fl_rec ) );
	
	if (!this) return(NULL);
	
	(**this).next = NULL;
	(**this).prev = NULL;
	(**this).id = TickCount();
	
	for (i=0;i<FLOW_POSITIONS;i++)
		(**this).position[i] = -1;
	
	if (temp = (t_fl_hndl)(**interp).flow_list) {	
		(**temp).prev = (Handle)this;
		(**this).next = (Handle)temp;
		}
		
	(**interp).flow_list = (Handle)this;

	return(this);
}

t_fl_hndl flist_find_do( InterpH interp, int slot )
{
	t_fl_hndl	found;
	t_fl_hndl	this;
	long		pos;
	
	found = NULL;
	this = (t_fl_hndl)(**interp).flow_list;
	pos = (**i_interp).fpos + (**i_interp).fbuf_ptr;
	
	while (this && !found)
		if (((**this).type == flow_cmd_while ) && ((**this).position[slot] == pos))
			found = this;
		else
			this = (t_fl_hndl)(**this).next;
		
	return( found );
}

void flist_del( InterpH interp, t_fl_hndl this )
{
	t_fl_hndl	temp;
	
	if ( this == (t_fl_hndl)(**interp).flow_list)
		(**interp).flow_list = (**this).next;

	if ( temp = (t_fl_hndl)(**this).next )
		(**temp).prev = (**this).prev;
		
	if ( temp = (t_fl_hndl)(**this).prev )
		(**temp).next = (**this).next;
	
	DisposHandle( this );
}

t_fl_hndl flist_find_type( InterpH interp, int type, int slot )
{
	t_fl_hndl	found;
	t_fl_hndl	this;
	
	found = NULL;
		
	this = (t_fl_hndl)(**interp).flow_list;
	
	while (this && !found)
		if (((**this).type == type) && ((**this).position[slot] == -1))
			found = this;
		else
			this = (t_fl_hndl)(**this).next;
		
	return( found );
}

/* ============================================= */

#endif

/* ============================================= */

void flist_del_all( InterpH interp )
{
	t_fl_hndl	this;
	t_fl_hndl	temp;
	
	this = (t_fl_hndl)(**interp).flow_list;
	
	while (this) {
		temp = this;
		this = (t_fl_hndl)(**this).next;
		DisposeHandle( temp );
		}

	(**interp).flow_list = NULL;
}

/* ============================================= */

int flow_check( Str255 name )
{
	int			i;
	t_flow_cmd	command;
	
	i = 0;
	command = flow_cmd_none;
	
	while ( (command == flow_cmd_none) && (g_flow_cmds[i].value) )
		if (pStrEqual(g_flow_cmds[i].name, name))
			command = g_flow_cmds[i].value;
		else
			i++;
		
	return(command);
}

/* ============================================= */

#if PROFESSIONAL

void flow_if( void )
{
	t_fl_hndl this;
	
	this = flist_add( i_interp );
	
	if (this) {
	
		(**this).type = flow_cmd_if;
		(**this).position[0] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_then( void )
{
	int			result;
	t_fl_hndl	this;
	
	this = flist_find_type( i_interp, flow_cmd_if, 1 );
	
	if (this) {
	
		result = (**i_interp).result;
	
		(**this).result = result;
		(**this).position[1] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		
		if ( result )
			parse_then();
	
		}
	else {
		pipe_putStr_err( i_shell, "\pflow: 'then' encountered before 'if'.\r");
		(**i_interp).result = NSHC_ERR_GENERAL;
		(**i_interp).action = nsh_stop;
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_else( void )
{
	t_fl_hndl this;
	
	this = flist_find_type( i_interp, flow_cmd_if, 2 );
	
	if (this) {
	
		if ((**this).position[1] < 0) {
			pipe_putStr_err( i_shell, "\pflow: 'else' encountered before 'then'.\r");
			(**i_interp).result = NSHC_ERR_GENERAL;
			(**i_interp).action = nsh_stop;
			}
		else {
			(**this).position[2] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
			if ( !(**this).result )
				parse_else();
			}
		}
	else {
		pipe_putStr_err( i_shell, "\pflow: 'else' encountered before 'if'.\r");
		(**i_interp).result = NSHC_ERR_GENERAL;
		(**i_interp).action = nsh_stop;
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_endif( void )
{
	t_fl_hndl this;
	
	this = flist_find_type( i_interp, flow_cmd_if, 3 );
	
	if (this) {
	
		flist_del( i_interp, this );
	
		}
	else {
		pipe_putStr_err( i_shell, "\pflow: 'endif' encountered before 'if'.\r");
		(**i_interp).result = NSHC_ERR_GENERAL;
		(**i_interp).action = nsh_stop;
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_while( void )
{
	t_fl_hndl this;
	
	this = flist_add( i_interp );
	
	if (this) {
	
		(**this).type = flow_cmd_while;
		(**this).not = 0;
		(**this).position[0] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_until( void )
{
	t_fl_hndl this;
	
	this = flist_add( i_interp );
	
	if (this) {
	
		(**this).type = flow_cmd_while;
		(**this).not = 1;
		(**this).position[0] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_do( void )
{
	int			pos;
	int			result;
	t_fl_hndl	this;
	
	this = flist_find_do( i_interp, 1 );
	
	if (!this) {
		this = flist_find_type( i_interp, flow_cmd_while, 1 );
		if (this)
			(**this).position[1] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		}

	if (this) {
	
		result = (**i_interp).result;
	
		(**this).result = result;
		
		if ( (**this).not )
			result = !result;
		
		if ( result ) {
			pos = (**this).position[2];
			if ( pos >= 0 )
				parse_set_pos( (**this).position[2] );
			else
				parse_do();
			flist_del( i_interp, this );
			}
	
		}
	else {
		pipe_putStr_err( i_shell, "\pflow: 'do' encountered before 'while' or 'until'.\r");
		(**i_interp).result = NSHC_ERR_GENERAL;
		(**i_interp).action = nsh_stop;
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow_done( void )
{
	t_fl_hndl this;
	
	this = flist_find_do( i_interp, 2 );
	
	if (!this) {
		this = flist_find_type( i_interp, flow_cmd_while, 2 );
		if (this)
			(**this).position[2] = (**i_interp).fpos + (**i_interp).fbuf_ptr;
		}

	if (this) {
	
		parse_set_pos( (**this).position[0] );
		
		}
	else {
		pipe_putStr_err( i_shell, "\pflow: 'done' encountered before 'while' or 'until'.\r");
		(**i_interp).result = NSHC_ERR_GENERAL;
		(**i_interp).action = nsh_stop;
		}
}

#endif

/* ============================================= */

#if PROFESSIONAL

void flow( void )
{
	int ft;
	
	ft = flow_check( (**i_shell).cmd_path );
	
	switch( ft ) {
		case flow_cmd_while:
			flow_while();
			break;
		case flow_cmd_do:
			flow_do();
			break;
		case flow_cmd_done:
			flow_done();
			break;
		case flow_cmd_if:
			flow_if();
			break;
		case flow_cmd_then:
			flow_then();
			break;
		case flow_cmd_else:
			flow_else();
			break;
		case flow_cmd_endif:
			flow_endif();
			break;
		case flow_cmd_until:
			flow_until();
			break;
		}
}

#endif