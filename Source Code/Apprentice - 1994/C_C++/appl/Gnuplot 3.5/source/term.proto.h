
/* term.c */
int do_point(int, int, int);
int line_and_point(int, int, int);
int do_arrow(int, int, int, int, TBOOLEAN);
int options_null(void);
int list_terms(void);
int set_term(int);
int change_term(char *, int);
int init_terminal(void);
char *turboc_init(void);
char *ztc_init(void);
int UP_redirect(int);
int UP_redirect(int);
int test_term(void);
void reopen_binary(void);
char *vms_init(void);
void vms_reset(void);
void term_mode_tek(void);
void term_mode_native(void);
void term_pasthru(void);
void term_nopasthru(void);
void reopen_binary(void);
void fflush_binary(void);
