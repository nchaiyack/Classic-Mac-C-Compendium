/* ========================================

	buf_utl.proto.h
	
	These routines allow buffered output in nShell(tm)
	commands.  To use this module, you must:
	
		1. Set up the A4 register for global storage.
		
		2. Call buf_init at the start of your command.
		
		3. Call buf_flush at the end of your command.
	
	Copyright (c) 1994,1995 Newport Software Development
		
   ======================================== */

void buf_init( t_nshc_calls *nshc_calls );
void buf_flush( void );
void buf_putchar( char ch );
void buf_puts( char *p );
void buf_putStr( StringPtr str );
