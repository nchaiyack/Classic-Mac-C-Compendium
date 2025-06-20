/* ==========================================

	multi.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

void    multi_activate(ShellH shell);
ShellH  multi_allocate(void);
void    multi_cancel( void );
void    multi_cancel_one( ShellH shell );
int     multi_can_insert( ShellH shell );
int     multi_close(ShellH shell);
int     multi_close_all(void);
void    multi_commander(ShellH shell);
void    multi_deactivate(ShellH shell);
void    multi_discard_text(ShellH shell, int req_chars);
void    multi_dispose(ShellH shell);
int     multi_fetch_line(ShellH shell, char *buf, int *count);
ShellH  multi_find(WindowPtr wind);
void    multi_force_insert( ShellH shell );
ShellH  multi_front(void);
int     multi_has_line(ShellH shell);
void    multi_init(void);
void    multi_key(char theChar);
void    multi_parse_setup(ShellH shell);
void    multi_prompt(ShellH shell);
void    multi_set_name( ShellH shell );
void    multi_skip_line(ShellH shell);
int		multi_stopped( ShellH shell );
void    multi_task(void);

void    mf_new(void);
void    mf_1st_new(void);
void    mf_open(void);
void    mf_open_file( FSSpec *fss );
void    mf_open_script( FSSpec *fss );
void    mf_revert(ShellH shell);
int     mf_save(ShellH shell);
int     mf_save_as(ShellH shell);
