#include <PrintTraps.h>

#define	MAX_ARGS 20
#undef stderr
#define stderr stdout

/* ------------- prototypes of global routines ------------- */
void	Kill_dvi( char *msg );
void	Print_page(		void );
void	Close_printer(	void );
void	Show_error( 	char *msg );
char	**get_str_resource(
						char *rsrc_name );
Boolean User_wants_out( void );
pascal void	Printer_idle( void );
void	Get_resource(				/* find a resource by name */
						OSType resource_type,
						char *resource_name,
						Handle *resource_handle,
						short *resource_id);
int		Get_resource_id(
						OSType rsrc_type,
						char *rsrc_name);
void	Set_pic_procs( void );
Point	Get_SF_place( short rsrc_id );

/* ------------------------ Stack_space ----------------- */
#pragma parameter __D0   Stack_space
long	Stack_space( void ) = { 0x200F, 0x90B8, 0x0130 };
//	move.L	SP, D0
//	SUB.L	ApplLimit, D0

/* -------------- global variables ------------------ */
extern Boolean		g_abort_dvi;
extern int			g_dpi;	/* bitmap resolution */
extern int			g_printer_dpi;	/* device resolution */
extern GrafPort		g_offscreen_GrafPort;
extern Boolean		g_draw_offscreen;
extern Boolean		g_preview;
extern short	 	g_pref_resfile;	/* reference number of preferences file */
extern short		g_app_resfile; /* refnum of application */
extern THPrint		g_print_rec_h;
extern DialogPtr	g_cancel_dialog;
extern short		g_pref_vRefNum;	/* vRefNum of preferences file */
extern WindowPtr	g_page_window;
extern TPPrPort		g_print_port_p;
extern short		g_paper_width;
extern short		g_paper_length;
extern short		g_print_status;