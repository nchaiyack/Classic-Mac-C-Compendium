/* ==========================================

	script.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

void script_close(ShellH shell);
int  script_open(void);
void script_read_file(void);
void script_start(void);
int  script_vars( void );
void script_set_pos( long pos );
void script_read_raw(long position);