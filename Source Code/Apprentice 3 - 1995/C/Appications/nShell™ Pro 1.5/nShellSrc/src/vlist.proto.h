/* ==========================================

	vlist.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#ifndef __VLIST__
#include "vlist.h"
#endif

t_vl_hndl   vlist_add( ShellH shell, Str32 name, long data_type );
int         vlist_char( char c );
int         vlist_count( ShellH shell );
void        vlist_del_all( ShellH shell );
int         vlist_env( ShellH shell, Str32 name, Str255 value );
int         vlist_export( ShellH shell, Str32 name, Str255 value );
t_vl_hndl   vlist_find( ShellH shell, Str32 name );
t_vl_hndl   vlist_next( ShellH shell, t_vl_hndl this, Str32 name, Str255 value  );
void        vlist_pop_bead( ShellH shell );
int         vlist_push_bead( ShellH shell );
int         vlist_set( ShellH shell, Str32 name, Str255 value );
int         vlist_set_parm( ShellH shell, Str32 name, Str255 value );
int         vlist_unset( ShellH shell, Str32 name );
int	        vlist_write( ShellH shell, int fRef  );
