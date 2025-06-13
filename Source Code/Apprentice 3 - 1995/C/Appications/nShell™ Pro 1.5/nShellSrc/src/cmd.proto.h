/* ==========================================

	cmd.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int      cmd_match( Str255 pattern, Str255 target );
int      cmd_ask(Str255 s, int size);
void     cmd_a_bit(ShellH shell);
void     cmd_debugger( void );
nshc_hdl cmd_load( void );
int      cmd_getchar(void);
int      cmd_gets(char *s, int limit);
int      cmd_getStr(Str255 s);
void     cmd_init(void);
void     cmd_notify(Str255 s, int size);
int	     cmd_path_expand(Str255 path);
int      cmd_path_to_FSSpec( Str255 pathname, FSSpec *spec );
int	     cmd_path_which(Str255 path);
void     cmd_printf(const char *fmt, ...);
void     cmd_printf_err(const char *fmt, ...);
void     cmd_putchar(char c);
void     cmd_putchar_err(char c);
void     cmd_puts(char *p);
void     cmd_putStr( Str255 string );
void     cmd_putStr_err( Str255 string );
void     cmd_puts_err(char *p);
void     cmd_run( nshc_hdl fp );
void     cmd_unload( void );
void     cmd_unload_all( void );
int      cmd_var_env( Str32 name, Str255 value );
int      cmd_var_set( Str32 name, Str255 value );
int      cmd_var_unset( Str32 name );
int      cmd_which( void );
