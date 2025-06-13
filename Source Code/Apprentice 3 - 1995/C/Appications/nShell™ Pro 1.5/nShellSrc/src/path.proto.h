/* ==========================================

	path.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int     path_append( StringPtr dst, StringPtr new );
void    path_compress( StringPtr s );
int     path_defaults( ShellH shell );
int     path_update( ShellH shell );
int     path_expand( ShellH shell, Str255 initial );
int     path_from_dir(  short vRefNum, long DirID, Str255 s );
int     path_get_home( void );
int     path_init( void );
int     path_is_dir( Str255 pathname );
int     path_is_full( StringPtr s );
int     path_is_partial( StringPtr s );
long    path_to_ftype( Str255 pathname );
int     path_which( ShellH shell, Str255 initial );
