/* ========================================

	buf_utl.c
	
	These routines allow buffered output in nShell(tm)
	commands.  To use this module, you must:
	
		1. Set up the A4 register for global storage.
		
		2. Call buf_init at the start of your command.
		
		3. Call buf_flush at the end of your command.
	
	Copyright (c) 1994,1995 Newport Software Development
		
   ======================================== */
   
#define BUF_SIZE	2048

#include "nshc.h"

#include "buf_utl.proto.h"

/* ======================================== */

// globals

int				 g_buf_ptr;			// next char to use in output buffer
char			 g_buf[BUF_SIZE];	// output buffer
t_nshc_calls	*g_buf_calls;		// global copy of nshc_calls to reduce parm passing

/* ======================================== */

void buf_init( t_nshc_calls *nshc_calls )
{
	g_buf_ptr = 0;
	g_buf_calls = nshc_calls;
}

/* ======================================== */

void buf_flush( void )
{
	if (g_buf_ptr > 0) {
		g_buf[g_buf_ptr++] = 0;
		g_buf_calls->NSH_puts(g_buf);
	}
	g_buf_ptr = 0;
}

/* ======================================== */

void buf_putchar( char ch )
{
	if (g_buf_ptr >= BUF_SIZE-1)
		buf_flush();
	g_buf[g_buf_ptr++] = ch;
}

/* ======================================== */

void buf_puts( char *p )
{
	char ch;
	
	while ( ch = *p++ ) {
	
		if (g_buf_ptr >= BUF_SIZE-1)
			buf_flush();
			
		g_buf[g_buf_ptr++] = ch;
	
		}
}

/* ======================================== */

void buf_putStr( StringPtr str )
{
	int 	i;
	char	*from;
	char	*to;

	if (g_buf_ptr + str[0] >= BUF_SIZE-1)
		buf_flush();
		
	from = (char *)&str[1];
	to = &(g_buf[g_buf_ptr]);
	
	for (i = str[0]; i > 0; i--)
		*to++ = *from++;
		
	g_buf_ptr += str[0];
}

/* ======================================== */
