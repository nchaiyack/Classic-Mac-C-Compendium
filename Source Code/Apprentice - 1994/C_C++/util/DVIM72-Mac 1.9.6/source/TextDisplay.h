#if STACK_PROBLEMS
#define CHK(x)	TD_printf( "(" x " %ld) ", StackSpace() )
#else
#define CHK(x)
#endif


#define INACTIVE	255
#define ACTIVE		0

#define TEXT_MARGIN			10

#ifndef GROW_ICON_SIZE
#define GROW_ICON_SIZE		16
#endif

#ifndef nil
#define nil 0L
#endif

typedef struct {
	WindowRecord	window;
	TEHandle		text;
	int				max_text;
	int				min_text;
} TD_record, *TD_peek;

extern  WindowPtr	g_console_window;

WindowPtr TD_new(
	int		window_id,		/* ID of WIND resource */
	int		max_text,		/* keep at most this many characters */
	int		min_text,		/* don't discard any text until so much */
	int		text_size,		/* font size */
	OSErr	*status );		/* error code */
void	TD_resize( void);
void	TD_printf( char *format, ... );
void	TD_update( void );
void	TD_click( Point where );
void	TD_activate( Boolean activate );
void	TD_close_log( void );

#ifndef TD_COPYABLE
#define TD_COPYABLE 1
#endif

#if TD_COPYABLE
void	TD_copy( void );
#endif