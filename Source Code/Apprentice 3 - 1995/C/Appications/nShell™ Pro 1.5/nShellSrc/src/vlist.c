/* ==========================================

	vlist.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include "file.h"
#include "multi.h"

#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"

/* ========================================== */

int vlist_char( char c )
{
	if (((c >= '0') && (c <= '9')) ||
		((c >= 'a') && (c <= 'z')) ||
		((c >= 'A') && (c <= 'Z')) ||
		(c == '_'))
		return(1);
	else
		return(0);
}

/* ========================================== */

// These are the maintenance routines

t_vl_hndl vlist_add( ShellH shell, Str32 name, long data_type )
{
	t_vl_hndl	temp;
	t_vl_hndl	this;
		
	this = (t_vl_hndl)NewHandle( sizeof( t_vl_rec ) );
	
	if (!this) return(NULL);
	
	(**this).data_type = data_type;
	pStrCopy( (**this).name, name );
	(**this).value[0] = 0;
	
	(**this).next = NULL;
	(**this).prev = NULL;
	
	if (temp = (t_vl_hndl)(**shell).vlist) {	
		(**temp).prev = (Handle)this;
		(**this).next = (Handle)temp;
		}
		
	(**shell).vlist = (Handle)this;

	return(this);
}

t_vl_hndl vlist_find( ShellH shell, Str32 name )
{
	t_vl_hndl	found;
	t_vl_hndl	this;
	
	found = NULL;
		
	this = (t_vl_hndl)(**shell).vlist;
	
	while (this && !found) {
		if ((**this).data_type == POP_BEAD )
			this = nil;
		else {
			if ( pStrEqual( (**this).name, name ) )
				found = this;
			if (!found)
				this = (t_vl_hndl)(**this).next;
			}
		}
		
	return( found );
}

void vlist_del_all( ShellH shell )
{
	t_vl_hndl	this;
	t_vl_hndl	temp;
	
	this = (t_vl_hndl)(**shell).vlist;
	
	while (this) {
		temp = this;
		this = (t_vl_hndl)(**this).next;
		DisposeHandle( temp );
		}
}

int vlist_count( ShellH shell )
{
	t_vl_hndl	this;
	int	count = 0;
	
	this = (t_vl_hndl)(**shell).vlist;
	
	while (this)
		if ((**this).data_type == POP_BEAD )
			this = nil;
		else {
			count++;
			this = (t_vl_hndl)(**this).next;
			}
		
	return( count );
}

int vlist_write( ShellH shell, int fRef  )
{
	t_vl_hndl		this;
	t_file_variable	variable;
	long			size;
	int				error = 0;
	
	size = sizeof( t_file_variable );

	this = (t_vl_hndl)(**shell).vlist;
	
	while (this && !error)
		if ((**this).data_type == POP_BEAD )
			this = nil;
		else {
			pStrCopy( variable.name, (**this).name );
			pStrCopy( variable.value, (**this).value );
			error = FSWrite( fRef, &size, &variable );
			this = (t_vl_hndl)(**this).next;
			}
	
	return(error);
}

/* ========================================== */

// These are the access routines.

int vlist_set( ShellH shell, Str32 name, Str255 value )
{
	t_vl_hndl	found;

	found = vlist_find( shell, name );
	
	if (!found)
		found = vlist_add( shell, name, SHELL_VARIABLE );
	
	if (found) {
		pStrCopy( (**found).value, value );
		return( 0 );
		}
	else
		return( -1 );
}

int vlist_set_parm( ShellH shell, Str32 name, Str255 value )
{
	t_vl_hndl	found;

	found = vlist_find( shell, name );
	
	if (!found)
		found = vlist_add( shell, name, SHELL_PARAMETER );
	
	if (found) {
		pStrCopy( (**found).value, value );
		(**found).data_type = SHELL_PARAMETER;
		return( 0 );
		}
	else
		return( -1 );
}

int vlist_unset( ShellH shell, Str32 name )
{
	t_vl_hndl	found;
	t_vl_hndl	temp;
	
	if ( found = vlist_find( shell, name ) ) {
	
		if ( found == (t_vl_hndl)(**shell).vlist )
			(**shell).vlist = (**found).next;
	
		if ( temp = (t_vl_hndl)(**found).next )
			(**temp).prev = (**found).prev;
			
		if ( temp = (t_vl_hndl)(**found).prev )
			(**temp).next = (**found).next;
		
		DisposHandle( found );
		
		return( 0 );
		}
	else
		return( 1 );
}

int vlist_env( ShellH shell, Str32 name, Str255 value )
{
	InterpH		interp;
	t_vl_hndl	found;
	
	if (pStrEqual( name, "\p?" )) {
		interp = (InterpH)(**shell).interp;
		NumToString( (**interp).result, value );
		return( 0 );
		}
	
	if ( found = vlist_find( shell, name ) ) {
		pStrCopy( value, (**found).value );
		return( 0 );
		}
	else {
		value[0] = 0;
		return( 1 );
		}
}

t_vl_hndl vlist_next( ShellH shell, t_vl_hndl this, Str32 name, Str255 value  )
{
	int	looking;
	
	if (this)
		this = (t_vl_hndl)(**this).next;	// in progress
	else
		this = (t_vl_hndl)(**shell).vlist;	// just starting (no prior value)

	looking = 1;
	
	while (this && looking)
		switch ( (**this).data_type ) {
			case SHELL_VARIABLE:
			case SHELL_PARAMETER:	
				looking = 0;
				break;
			case POP_BEAD:
				this = nil;
				break;
			default:
				this = (t_vl_hndl)(**this).next;
			}
		
	if (this) {
		pStrCopy( name, (**this).name );
		pStrCopy( value, (**this).value );
		}
	else {
		name[0] = 0;
		value[0] = 0;
		}
		
	return( this );
}

/* ========================================== */

// export function

int vlist_export( ShellH shell, Str32 name, Str255 value )
{
	t_vl_hndl	new_root;
	t_vl_hndl	this;
	t_vl_hndl	found;
	
	// first, find the root to the parent vlist
	
	new_root = NULL;
	this = (t_vl_hndl)(**shell).vlist;
	
	while (this && !new_root)
		if ((**this).data_type == POP_BEAD )
			new_root = (t_vl_hndl)(**this).next;
		else
			this = (t_vl_hndl)(**this).next;

	// bail if no pop-beads are found

	if (!new_root)
		return( NSHC_ERR_GENERAL );
	
	// second, search the parent to find a matching name
	
	found = NULL;
	this = new_root;
	
	while (this && !found) {
		if ((**this).data_type == POP_BEAD )
			this = nil;
		else {
			if ( pStrEqual( (**this).name, name ) )
				found = this;
			if (!found)
				this = (t_vl_hndl)(**this).next;
			}
		}
	
	// if a match is found, copy the value string
	
	if (found) {
		pStrCopy( (**found).value, value );
		return( 0 );
		}
		
	// finally, if no match is found, insert a new variable at new_root
	
	found = (t_vl_hndl)NewHandle( sizeof( t_vl_rec ) );
	
	if (!found)
		return( NSHC_ERR_MEMORY );
	
	(**found).data_type = SHELL_VARIABLE;
	pStrCopy( (**found).name, name );
	pStrCopy( (**found).value, value );
	
	(**(t_vl_hndl)(**new_root).prev).next = (Handle)found;
	(**found).next = (Handle)new_root;	
	(**found).prev = (**new_root).prev;
	(**new_root).prev = (Handle)found;
	
	return(0);
}

/* ========================================== */

// bead management

int vlist_push_bead( ShellH shell )
{
	t_vbead_hndl	temp;
	t_vbead_hndl	this;
	t_vl_hndl		fork;
	
	this = (t_vbead_hndl)NewHandle( sizeof( t_vbead_rec ) );
	
	if (!this) return(-1);
	
	(**this).data_type = POP_BEAD;	
	(**this).next = NULL;
	(**this).prev = NULL;
	
	if (temp = (t_vbead_hndl)(**shell).vlist) {	
	
		(**temp).prev = (Handle)this;			// add bead to root of list
		(**this).next = (Handle)temp;
		
		fork = (t_vl_hndl)temp;		// get ready to fork
		
		}
		
	(**shell).vlist = (Handle)this;
	
	while (fork) {								// fork existing data
	
		if ( (**fork).data_type == POP_BEAD )	// quit when a bead is found
			fork = nil;
		else {
		
			if ( (**fork).data_type == SHELL_VARIABLE ) {	// but copy any variables
				HLock( fork );
				vlist_set( shell, (**fork).name, (**fork).value );
				HUnlock( fork );
				}
				
			fork = (t_vl_hndl)(**fork).next;	// pick up the next one
		
			}
		}
		
	return(0);
}

void vlist_pop_bead( ShellH shell )
{
	int				looking;
	t_vbead_hndl	this;
	t_vbead_hndl	temp;
	
	looking = 0;
	
	this = (t_vbead_hndl)(**shell).vlist;
	
	do {	// practice
	
		looking = ( (**this).data_type != POP_BEAD );
		this = (t_vbead_hndl)(**this).next;

	} while (looking && this);
	
	if (looking)
		return;
		
	this = (t_vbead_hndl)(**shell).vlist;
	
	do {	// for real
	
		looking = ( (**this).data_type != POP_BEAD );
		temp = this;
		this = (t_vbead_hndl)(**this).next;
		DisposeHandle( temp );
		
	} while (looking && this);
		
	(**shell).vlist = (Handle)this;
	(**this).prev = NULL;
}


