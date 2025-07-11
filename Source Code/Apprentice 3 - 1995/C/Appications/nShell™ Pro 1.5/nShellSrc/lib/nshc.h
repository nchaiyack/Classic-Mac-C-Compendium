/* =============== the nshc interface: ===============

	nshc.h - where entry points to NSH commands are kept.

	Copyright (c) 1993,1994,1995 Newport Software Development
	
   =============== the nshc interface: =============== */

#define __NSHC__

/* ========== Interface Constants and Types ========== */

#define NSHC_VERSION	10			// version of this header file

#define MAX_ARGS		100			// maximum value of argc
#define LINE_MAX		2048		// maximum input line to be expected by all cmds

typedef enum { nsh_idle, nsh_start, nsh_continue, nsh_stop } t_nsh_state;

/* ======== Define some common return values ======== */

#define	NSHC_NO_ERR			 0		// success
#define	NSHC_ERR_GENERAL	-1		// unspecified error
#define	NSHC_ERR_VERSION	-2		// NSHC_VERSION of application and command don't match
#define	NSHC_ERR_PARMS		-3		// parameters don't match command definition
#define	NSHC_ERR_MEMORY		-4		// memory allocation error
#define	NSHC_ERR_FILE		-5		// file access error

/* ========== The Data Communication Block ========== */

typedef struct {

	// version is always first, so it can be used to detect a mismatch.

	int		version;
	
	// requested actions
	
	t_nsh_state	action;
	
	// returned status
	
	int		result;
	
	// passed parameters
	
	int		argc;
	int		argv[MAX_ARGS];
	char	arg_buf[LINE_MAX];
	
	// a handle at which the command may allocate (AND DISPOSE) storage
	
	Handle	data;
	
} t_nshc_parms;

/* ========== The Call-Back Communication Block ========== */

typedef struct {

	// NSH_putErrStr is always first, so it can be used to notify the operator
	// of a version mismatch. NO OTHER CALLS ARE GUARANTEED ACROSS REVSIONS.

	void 	(*NSH_putStr_err)(Str255 s);
	
	// output to stderr
	
	void 	(*NSH_putchar_err)(char c);
	void 	(*NSH_puts_err)(char *);
	void	(*NSH_printf_err)(const char *fmt, ...);
	
	// output to stdout
	
	void 	(*NSH_putchar)(char c);
	void 	(*NSH_puts)(char *);
	void 	(*NSH_putStr)(Str255 s);
	void	(*NSH_printf)(const char *fmt, ...);
	
	// input from stdin
	
	int 	(*NSH_getchar)(void);
	int 	(*NSH_gets)(char *s, int max_len);
	int 	(*NSH_getStr)(Str255 s);
	
	// variable access functions
	
	int		(*NSH_var_set)(Str32 name, Str255 value);
	int		(*NSH_var_unset)(Str32 name);
	int		(*NSH_var_env)(Str32 name, Str255 value);
	
	// path expansion functions
	
	int		(*NSH_path_expand)(Str255 path);
	int		(*NSH_path_to_FSSpec)( Str255 pathname, FSSpec *spec );
	int		(*NSH_path_which)(Str255 path);
	
	// dialog functions
	
	void 	(*NSH_notify)(Str255 s, int size);
	int 	(*NSH_ask)(Str255 s, int size);

	// misc
	
	int 	(*NSH_match)( Str255 pattern, Str255 target );

} t_nshc_calls;

/* ========== The External Command Definition ========== */

typedef void (**nshc_hdl)(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);
