/* ==========================================

	cmd_int.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int  cmd_int(void);
void cmd_int_cd( void );
int  cmd_int_check( Str255 name );
void cmd_int_env( void );
void cmd_int_env_all( void );
int  cmd_int_env_one( int arg );
void cmd_int_env_print( Str32 name, Str255 value );
void cmd_int_exit( void );
void cmd_int_export( void );
void cmd_int_path( void );
void cmd_int_pathchk( void );
int  cmd_int_path_check( ShellH shell, Str255 path, int loud );
void cmd_int_pwd( void );
void cmd_int_return( int success );
void cmd_int_set( void );
void cmd_int_unset( void );
int  cmd_int_unset_one( int arg );
int  cmd_int_vname( int arg, Str32 name );
void cmd_int_which( void );

