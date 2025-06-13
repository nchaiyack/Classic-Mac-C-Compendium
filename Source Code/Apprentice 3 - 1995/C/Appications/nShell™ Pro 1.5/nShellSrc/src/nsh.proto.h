/* ==========================================

	nsh.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int  main( void );

int  nsh_app_files( void );
void nsh_cursor( void );
void nsh_events( EventRecord *myEvent );
void nsh_idle_task( void );
void nsh_init( void );
void nsh_loop( void );
void nsh_mouse( int windowPart, WindowPtr whichWindow, EventRecord *myEvent );
void nsh_quit( void );
long nsh_sleep( void );