/*****************************************************************************\
* macstuff.c                                                                  *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It contains    *
* procedures which initialize rtrace as a macintosh application.              *
\*****************************************************************************/

#include "macresources.h"
#include "defs.h"
#include "extern.h"
#include "mactypes.h"
#include "macerrors.h"
#include <setjmp.h>
#include <Retrace.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <AppleEvents.h>
#include <Movies.h>
#include <LowMem.h>
#include <Icons.h>

/* Globals */
PicHandle		down_arrow_picture;		/* Pictures used for popup menus */
PicHandle		down_arrow_picture_grayed;
short			current_line;			/* used to trace the line being drawn */
Rect			drag_window_rect;		/* the area in which windows may be dragged */
WindowPtr		image_window;			/* the image window data structures */
CWindowRecord	image_window_rec;
CGrafPort 		image_port_rec;			/* offscreen port which contains the image */
CGrafPtr  		image_port = &image_port_rec;
MenuHandle		apple_menu;				/* handles to pulldown menus */
MenuHandle		file_menu;
MenuHandle		edit_menu;
MenuHandle		windows_menu;
long			before_time;			/* used to time processes */
double		    t;						/* current value of t during animation */
long			image_width;			/* size of the image to be generated */
long			image_height;
PicHandle		bitmap_pict;			/* The Picture which contains the image bitmap */
long			*offscreen_bitmap_base;	/* Base address of the Picture bitmap */


/* externals */
extern MenuHandle	save_anim_file_type_menu;
extern long			image_width;			/* size of the image to be generated */
extern long			image_height;
extern short		save_file_type_menu_selection;
extern short		save_anim_file_type_menu_selection;
extern char			last_log_line[];		/* The last line in the log text */
extern WindowPtr	 log_window;
extern FILE			*stderr_file;			/* file to which stderr is redirected */
extern fpos_t		current_stderr_read_pos;/* position we're reading in stderr */
extern Boolean		status_dialog_visible;	/* TRUE if the status dialog is visible */
extern DialogPtr	options_dialog;			/* the options dialog */
extern DialogPtr	preferences_dialog;		/* the preferences dialog */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern DialogPtr 	status_dialog;			/* the status dialog */
extern DialogPtr	cancel_dialog;			/* the Cancel dialog */
extern DialogPtr	abort_render_dialog;	/* the Abort Render dialog */
extern DialogPtr	prompt_save_dialog;		/* the Prompt To Save dialog */
extern DialogPtr	quit_during_render_dialog; /* Dialog to prompt user who quit during render */
extern ControlHandle render_button_handle;	/* handle to the Render button */
extern ControlHandle animation_okay_button_handle;/* handle to the Okay button in animation */
extern ControlHandle preferences_okay_button_handle;/* handle to the Okay button in preferences */
extern ControlHandle animate_checkbox;		/* handle to the animate checkbox in animation */
extern short		blessed_folder_wd_id;	/* Working Directory refnum of blessed folder */
extern short		rtrace_wd_id;			/* Working Directory refnum of RTrace folder */
extern short		temp_folder_wd_id;	    /* Working Directory refnum of Temporary Items */
extern char         last_error_message[];   /* The most recent error message */
extern preferences_handle prefs;			/* The preferences */
extern pixel_ptr    temp_image;				/* RTrace's internal temporary image bitmap */

Boolean	fpu_available;					/* TRUE if the machine has a math coprocessor */
Boolean	mc68020_available;				/* TRUE if the machine has a 68020 or better */
Boolean	f8bit_QD_available;				/* TRUE if 8-bit QuickDraw is available */
Boolean	f32bit_QD_available;			/* TRUE if 32-bit QuickDraw is available */
Boolean	find_folder_available;			/* TRUE if FindFolder is available */
Boolean	notification_manager_available;	/* TRUE if the Notification Manager is available */
Boolean	apple_events_available;			/* TRUE if Apple Events are available */
Boolean	quicktime_available;			/* TRUE if QuickTime is available */
Boolean	dither_copy_available;			/* TRUE if QuickDraw supports dithered copying */

Boolean	rendering;						/* TRUE while we are rendering */
Boolean	reading_scene;					/* TRUE while we are reading the scene */
Boolean converting_scene = FALSE;		/* TRUE while we are converting scn to sff */
Boolean reread_scene = FALSE;			/* TRUE if we should reread the scene in memory */
Boolean	render_next = FALSE;			/* TRUE if we should render as soon as the scene is read */
Boolean building_quicktime_movie = FALSE;/* TRUE if we're building a QuickTime movie */
Boolean	image_complete = FALSE;			/* TRUE if the image is completely rendered */
Boolean get_another_event = FALSE;		/* TRUE if we're due to get another event */
Boolean vbl_installed = FALSE;			/* TRUE if the VBL event task is installed */
Boolean scene_in_memory = FALSE;		/* TRUE if the scene for the current file is in memory */
Boolean notification_installed = FALSE;	/* TRUE when a notification is installed */
Boolean	in_background = FALSE;			/* TRUE when the app is in the background */
Boolean	starting_up = TRUE;				/* TRUE if we're still starting up */
Boolean	image_saved = TRUE;				/* TRUE if the image has been saved since last modification */

NMRec	notification;					/* A notification */
Handle	notification_icon;				/* Handle to the notification icon family */

long	total_time;						/* Used to time rendering */
jmp_buf	environment;					/* Used to store environment to restore on error */

char	scene_filename[50];				/* filename of scene to render */
char	scene_name[50];					/* The name of the current scene */
short	scene_file_wd_id;				/* The working directory containing the scene file */
long	frames;							/* The number of frames in the animation sequence */


/* Prototypes */
extern void init_mac(void);
void init_mgrs(void);
void init_menus(void);
void do_mouse_down(EventRecord *event);
void open_scene_file(void);
void do_menu(long command);
void process_mac_event(void);
void debug_exit (void);
void handle_update_event(WindowPtr the_window);
void create_offscreen_pict(Rect *bounds_rect);
void main(void);
void handle_save(void);
void do_edit(short item);
pascal void scroll_action_proc(void);
#if __powerc
pascal void set_event_flag(VBLTaskPtr vblTaskPtr);
#else
pascal void set_event_flag(void);
#endif
void remove_get_event_vbl(void);
void install_get_event_vbl(void);
void handle_key_down(EventRecord *event);
void handle_multifinder_event(EventRecord *event);
Boolean handle_dialog_event(EventRecord *event);
pascal OSErr open_app_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
pascal OSErr quit_app_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
pascal OSErr open_docs_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
void init_apple_events(void);
void handle_high_level_event(EventRecord *event);
OSErr got_required_parameters(const AppleEvent *aevent);
Boolean prepare_to_terminate(void);
void read_scene(void);
void render_scene(void);


/* External Prototypes */
extern void init_alert_dialogs(void);
extern Boolean choose_scene_file(void);
extern void init_dialogs(void);
extern void show_about_dialog(void);
extern void add_parameter(params_struct *params, char *param);
extern void tc_ray_trace (void);
extern void get_parameters (int, char_ptr[]);
extern void get_scene (void);
extern void ray_trace (void);
extern void do_preferences_dialog(void);
extern ModalFilterUPP save_image_filter_upp;
extern char *get_save_filename (void);
extern void cleanup(Boolean natural);
extern void do_stdout_check(void);
extern void setup_log_window(void);
extern void update_log_window (void);
extern void handle_log_click(Point where);
extern void save_pict_file(SFReply	*my_reply);
extern void save_quicktime_movie(SFReply *my_reply);
extern void handle_log_window_zoom(Point where, short part_code);
extern void grow_log_window(Point where);
extern void update_scroll_bar(void);
extern void update_scroll_bar_with_active(Boolean active);
extern void mac_halt(void);
extern void show_options_dialog(Boolean bring_to_front);
extern void hide_options_dialog(void);
extern void show_animation_dialog(Boolean bring_to_front);
extern void hide_animation_dialog(void);
extern void show_log_window(Boolean bring_to_front);
extern void hide_log_window(void);
extern void show_status_dialog(Boolean bring_to_front);
extern void hide_status_dialog(void);
extern void handle_animation_selection(short item_hit);
extern void handle_options_selection(short item_hit);
extern void update_render_button(void);
extern void preprocess_sff_file(long *num_lights, long *num_surfaces, long *num_objects);
extern void generate_params(params_struct *params);
extern void add_line_to_log_window (char *line);
extern void set_status_num_objects(long num_objects);
extern void get_preferences(void);
extern void write_preferences(void);
extern void place_window (WindowPtr window, Rect *bounds);
extern void get_system_info(void);
extern void show_image_window(Boolean bring_to_front);
extern void hide_image_window(void);
extern void get_animation_info(real *tstart, real *tend, long *frames);
extern Boolean find_variable_value (char *var_name, double *value);
extern void setup_frame(void);
extern void setup_temp_folder(void);
extern void delete_temp_files(Boolean movie_too);
extern short make_working_directory(short vrefnum, long dir_id);
extern void log_window_resized(void);
extern void show_preferences_dialog(void);
extern void handle_preferences_selection(short item_hit);
extern void set_default_preferences(void);
extern void set_memory_bar_max(long value);
extern void input_parameters(int *parameters, char_ptr parameter[]);
extern void set_progress_bar_max(long value);
extern void init_progress_bar_thermometer_mode(void);
extern Boolean are_valid_params(void);
extern void read_view_info(void);
extern void set_dialog_text_to_real(DialogPtr theDialog, short theItem, double value);
extern void init_quicktime_save( SFReply *inReply );
extern void compress_one_movie_frame(long frame);
extern void finish_movie(void);
extern void movie_cleanup(void);
extern void free_all(void);
extern void set_render_button_name(void);
extern void scn2sff(short scn_wd_id, char *scn_filename, short sff_wd_id, char *sff_filename);


/* procedures in scn2sff segment, used only to unload the segments */
int yyparse(void);
void create_sphere(void *center, double radius);



/*****************************************************************************\
* procedure main                                                              *
*                                                                             *
* Purpose: This procedure initializes the mac application, and then goes off  *
*          to the event loop.                                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void main(void)
{

	long	i;
	Str255	pict_filename;
	char	ppm_filename[255];

	/* Initialize */
	init_mac();

	/* We're no longer starting up */
	starting_up = FALSE;

	/* Put up the about dialog as an intro, if prefs say to */
	if ((*prefs)->show_about_window_flag)
		show_about_dialog();

	/* If there is an error later, this is where we want to return to.  The
		error handler will return here by calling longjmp() */
	setjmp(environment);
	
	/* Not rendering now */
	rendering = FALSE;
	
	/* Go to the event loop, never return */
	process_mac_event();
	
}



/*****************************************************************************\
* procedure init_mac                                                          *
*                                                                             *
* Purpose: This procedure initializes rtrace as a mac program, and sets up    *
*          the menus.                                                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_mac()
{

	Rect	image_window_frame = {0, 0, 256, 256};
	Str255	volume_name;
	short	rtrace_vrefnum;
	long	rtrace_dirid;
	short	error;
	short	attribs;
	WDPBRec	pb;

	/* initialize the toolbox */
	init_mgrs();
	
	/* Grow the application zone to the maximum */
	MaxApplZone();

	/* Get the amount of free memory now, and use it as the max */
	set_memory_bar_max(FreeMem());

	/* Initialize the alert dialogs in case we need them for errors */
	init_alert_dialogs();

	/* find what kind of system we have here */
	get_system_info();

	/* initialize the menus */
	init_menus();
	
	/* Initialize the dialogs */
	init_dialogs();

	/* Initialize the apple events handlers, if apple events are available */
	if (apple_events_available)
		init_apple_events();

	/* Initialize QuickTime, if we have it */
	if (quicktime_available)
		{
		error = EnterMovies();
		if (error) terminal_error(error);
		}
	
	/* If there's no QuickTime, don't let user save as QT movie */
	else
		DisableItem(save_anim_file_type_menu, 2);
	
	/* If it is possible to have an Image Window, create it */
	if (f8bit_QD_available)
		image_window = NewCWindow (&image_window_rec, &image_window_frame, "\pImage",
									FALSE, 0, (WindowPtr) -1L, TRUE, 0);
	else
		/* Dim the Show Image Window menu item */
		DisableItem (windows_menu, SHOW_IMAGE_ITEM);

	/* Get the down arrow pictures for the popup menus */
	down_arrow_picture = (PicHandle) GetResource ('PICT', DOWN_ARROW_PICT);
	down_arrow_picture_grayed = (PicHandle) GetResource ('PICT', DOWN_ARROW_PICT_GRAYED);

	/* Get the small icon for the notification */
	error = GetIconSuite(&notification_icon, NOTIFICATION_ICON, svAllSmallData);
	HNoPurge(notification_icon);

	/* Set up window dragging region */
	drag_window_rect = qd.screenBits.bounds;
	InsetRect (&drag_window_rect, 4, 4);

	/* Find the default (MacRTrace) directory */
	error = HGetVol (volume_name, &rtrace_vrefnum, &rtrace_dirid);
	if (error) terminal_error(error);

	/* Make it a working directory */
	rtrace_wd_id = make_working_directory(rtrace_vrefnum, rtrace_dirid);

	/* Set up the log window */
	setup_log_window();

	/* Get the preferences */
	get_preferences();
	
	/* Move the scroll bars in the log window to agree with its new size */
	log_window_resized();
	
	/* Show the log window */
	show_log_window(FALSE);

	/* Set up our temporary items folder */
	setup_temp_folder();

	/* update the free memory bar */
	update_memory_bar_value();

}	/* init_mac() */



/*****************************************************************************\
* procedure get_system_info()                                                 *
*                                                                             *
* Purpose: This procedure determines what kind of system is installed, and    *
*          various other system-specific tidbits.  It sets globals so that we *
*          can determine later what the capabilities of the system are.       *
*                                                                             *
* Created by: Reid Judd                                                       *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/10/92          Interfaced with my code, added other checks *
\*****************************************************************************/

void get_system_info(void)
{

#define Gestalttest             0xA1AD
#define NoTrap                  0xA89F

	short		error;
	long		feature;
	long		quickdraw_version;
	SysEnvRec	sys_environ;

	/* Find the blessed folder */
	error = SysEnvirons (1, &sys_environ);
	if (error) terminal_error(error);
	blessed_folder_wd_id = sys_environ.sysVRefNum;

	/* Check for availability of Gestalt, alert user if not present */
	if (NGetTrapAddress(Gestalttest, ToolTrap) == NGetTrapAddress(NoTrap, ToolTrap))
		{

		/* No Gestalt-- find the info from SysEnvirons */
		fpu_available = sys_environ.hasFPU;
		mc68020_available = ((sys_environ.processor != 1) && (sys_environ.processor != 2));
		f8bit_QD_available = sys_environ.hasColorQD;
		f32bit_QD_available = FALSE;			/* only on machines with Gestalt */
		dither_copy_available = FALSE;			/* only on machines with Gestalt */
		find_folder_available = FALSE;			/* only on machines with Gestalt */
		notification_manager_available = FALSE;	/* only on machines with Gestalt */
		apple_events_available = FALSE;			/* only on machines with Gestalt */
		quicktime_available = FALSE;			/* only on machines with Gestalt */
				
		}

	else	/* Use Gestalt */
		{
		
		/* Find out whether QuickTime is available */
		error = Gestalt(gestaltQuickTime, &feature);
		if (error == gestaltUndefSelectorErr)
			quicktime_available = FALSE;
		else
			{
			if (error) terminal_error(error);
			quicktime_available = (feature != 0);
			}
	
		/* Find out whether Apple Events are available */
		error = Gestalt(gestaltAppleEventsAttr, &feature);
		if (error == gestaltUndefSelectorErr)
			apple_events_available = FALSE;
		else
			{
			if (error) terminal_error(error);
			apple_events_available = feature & 1<<gestaltAppleEventsPresent;
			}
	
		/* Find out whether FindFolder is available */
		error = Gestalt(gestaltFindFolderAttr, &feature);
		if (error == gestaltUndefSelectorErr)
			find_folder_available = FALSE;
		else
			{
			if (error) terminal_error(error);
			find_folder_available = feature & 1<<gestaltFindFolderPresent;
			}
	
		/* Find out whether the Notification Manager is available */
		error = Gestalt(gestaltNotificationMgrAttr, &feature);
		if (error == gestaltUndefSelectorErr)
			notification_manager_available = FALSE;
		else
			{
			if (error) terminal_error(error);
			notification_manager_available = feature & 1<<gestaltNotificationPresent;
			}
	
		/* Find the processor type */
		error = Gestalt(gestaltProcessorType, &feature);
		if (error) terminal_error(error);
		mc68020_available = (feature >= gestalt68020);
	
		/* Find out whether there is a fpu */
		error = Gestalt(gestaltFPUType, &feature);
		if (error) terminal_error(error);
		fpu_available = (feature != gestaltNoFPU);
	
		/* Find the version of QuickDraw */
		error = Gestalt(gestaltQuickdrawVersion, &quickdraw_version);
		if (error) terminal_error(error);
		f8bit_QD_available = (quickdraw_version >= gestalt8BitQD);
		f32bit_QD_available = (quickdraw_version >= gestalt32BitQD);
		dither_copy_available = (quickdraw_version >= gestalt32BitQD13);
		
		}

#if !__powerc
	/* If this isn't at least a 68020, abort */
	if (!mc68020_available)
		terminal_string_error("MacRTrace requires at least a 68020 processor.");		

	/* If there isn't an fpu, abort */
	if (!fpu_available)
		terminal_string_error("MacRTrace requires a math coprocessor.");		
#endif

	/* If this is Original QuickDraw, warn the user */
	if (!f8bit_QD_available)
		{
		possibly_terminal_string_error("MacRTrace prefers Color QuickDraw.  You may continue, \
but you will not be able to see the image or to use the clipboard.");		
		}

	/* If this is 8-bit QuickDraw, warn the user */
	else if (!f32bit_QD_available)
		{
		terminal_string_error("MacRTrace requires 32-bit Color QuickDraw.");		
		}
		
}	/* get_system_info() */



/*****************************************************************************\
* procedure init_mgrs                                                         *
*                                                                             *
* Purpose: This procedure initializes the ToolBox managers so RTrace can look *
*          and act like a standard Mac application.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_mgrs(void)
{

	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitDialogs(0);
	InitCursor();
	InitMenus();
	TEInit();

}	/* init_mgrs() */



/*****************************************************************************\
* procedure init_apple_events                                                 *
*                                                                             *
* Purpose: This procedure installs the handlers for the basic Apple Events.   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void init_apple_events(void)
{

	short error;
	
	/* Install the OpenApplication event handler */
	error = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
				NewAEEventHandlerProc(open_app_ae_handler), 0, FALSE);
	if (error) terminal_error(error);

	/* Install the OpenDocuments event handler */
	error = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
				NewAEEventHandlerProc(open_docs_ae_handler), 0, FALSE);
	if (error) terminal_error(error);
	
	/* Install the QuitApplication event handler */
	error = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
				NewAEEventHandlerProc(quit_app_ae_handler), 0, FALSE);
	if (error) terminal_error(error);
	
}	/* init_apple_events() */



/*****************************************************************************\
* procedure init_menus                                                        *
*                                                                             *
* Purpose: This procedure initializes the mac pull-down menus.                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_menus()
{

	/* Read the menus in to the menu list */
	InsertMenu(apple_menu = GetMenu(APPLE_MENU), 0);
	InsertMenu(file_menu = GetMenu(FILE_MENU), 0);
	InsertMenu(edit_menu = GetMenu(EDIT_MENU), 0);
	InsertMenu(windows_menu = GetMenu(WINDOWS_MENU), 0);
	DrawMenuBar();	/* Draw the new menu bar */
	
	/* add the desk accessories */
	AddResMenu(apple_menu, 'DRVR');

	/* Disable the editing commands in the Edit menu */
	DisableItem(edit_menu, UNDO_ITEM);
	DisableItem(edit_menu, CUT_ITEM);
	DisableItem(edit_menu, COPY_ITEM);
	DisableItem(edit_menu, PASTE_ITEM);
	DisableItem(edit_menu, CLEAR_ITEM);

	/* Disable the Save... command in the File menu */
	DisableItem(file_menu, SAVE_ITEM);

}	/* init_menus() */



/*****************************************************************************\
* procedure mac_halt                                                          *
*                                                                             *
* Purpose: This procedure is called then rtrace tries to HALT (i.e. when an   *
*          error occurs).  It puts up a dialog and attempts to continue.      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 31, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void mac_halt(void)
{

	/* Put up an error alert, clean up, and go back to the event loop */
	abortive_string_error(last_error_message);

}	/* mac_halt() */



/*****************************************************************************\
* procedure process_mac_event                                                 *
*                                                                             *
* Purpose: This processes a single mac event.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 24, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void process_mac_event(void)

{
	EventRecord	event;
	long 		menu_result;
	Boolean		valid;
	DialogPtr	dialog;
	short		item_hit;

	/* if we aren't rendering, this is a loop.  If we are, it's just a
		subroutine, and returns after one iteration */
	do
		{
		
		/* If we need to reread the scene, do it now, unless we're rendering */
		if ((reread_scene) && (!rendering))
			{
			reread_scene = FALSE;
			read_scene();
			}

		/* Don't call again for a bit */
		get_another_event = FALSE;
	
		/* Get an event, allow background tasks time to work.  While rendering, we give
			background tasks minimum time; while not rendering, we give them a little more. */
		
		if ((rendering) || (reading_scene))
			valid = WaitNextEvent(everyEvent, &event, 0, (RgnHandle) 0L);
	
		else
			valid = WaitNextEvent(everyEvent, &event, 20, (RgnHandle) 0L);
	
		/* Check for event in the Options Dialog */
		if (handle_dialog_event(&event))
			continue;
			
		if (valid)
			{
		
			switch (event.what)
				{
				case nullEvent:
					break;
				case mouseDown:
					do_mouse_down(&event);
					break;
				case mouseUp:
					break;
				case keyDown:
				
					handle_key_down(&event);
					break;
				
				case keyUp:
				case autoKey:
					break;
				case updateEvt:
					handle_update_event((WindowPtr) event.message);
					break;
				case diskEvt:
					break;
				case activateEvt:
					if (((WindowPtr) event.message) == log_window)
						update_scroll_bar_with_active(event.modifiers & activeFlag);
						DrawGrowIcon(log_window);
					break;
				case networkEvt:
				case driverEvt:
				case app1Evt:
				case app2Evt:
				case app3Evt:
					break;
				case app4Evt:
		
					handle_multifinder_event(&event);
		
				case kHighLevelEvent:
				
					handle_high_level_event(&event);
					
				default:
					break;
				}
			}
		}
		
	while ((!rendering) && (!reading_scene) && (!converting_scene));
	
	/* Set the GrafPort to the image window, in case the event changed it */ 
	SetPort(image_window);

}	/* process_mac_event() */



/*****************************************************************************\
* procedure open_app_ae_handler                                               *
*                                                                             *
* Purpose: This procedure handles an OpenApplication AppleEvent.              *
*                                                                             *
* Parameters: aevent: the OpenApplication AppleEvent.                         *
*             reply:  the reply AppleEvent.                                   *
*             handler_refcon: reference constant                              *
*             returns an error code                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal OSErr open_app_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
{

	short	error;

	/* Get the required parameters */
	error = got_required_parameters(aevent);

	/* If there was an error, return it */
	if (error)
		return error;

	/* No error */
	return noErr;

}	/* open_app_ae_handler() */



/*****************************************************************************\
* procedure quit_app_ae_handler                                               *
*                                                                             *
* Purpose: This procedure handles a QuitApplication AppleEvent.               *
*                                                                             *
* Parameters: aevent: the QuitApplication AppleEvent.                         *
*             reply:  the reply AppleEvent.                                   *
*             handler_refcon: reference constant                              *
*             returns an error code                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal OSErr quit_app_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
{

	short	error;
	Boolean	user_cancelled;

	/* check for missing required parameters */
	error = got_required_parameters(aevent);
	if (error)
		return error;
		
	else
		{
		
		/* If there is an unsaved image, check if the user really wants to quit */
		user_cancelled = prepare_to_terminate();
		
		if (user_cancelled)
			return userCanceledErr;		/* user didn't want to quit */
		
		else
			return noErr;				/* user wanted to quit */

		}

}	/* quit_app_ae_handler() */



/*****************************************************************************\
* procedure open_docs_ae_handler                                              *
*                                                                             *
* Purpose: This procedure handles a OpenDocuments AppleEvent.                 *
*                                                                             *
* Parameters: aevent: the OpenDocuments AppleEvent.                           *
*             reply:  the reply AppleEvent.                                   *
*             handler_refcon: reference constant                              *
*             returns an error code                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal OSErr open_docs_ae_handler(const AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
{

	short		error;
	AEDescList	document_list;
	long		num_docs;
	long		i;
	Size		actual_size;
	AEKeyword	keyword;
	DescType	returned_type;
	char		open_doc_error_message[] = "Error opening documents.";
	FSSpec		fss;

	/* Get the direct parameter--a descriptor list-- and put it into document_list */
	error = AEGetParamDesc(aevent, keyDirectObject, typeAEList, &document_list);
	if (error)
		abortive_error(error);

	/* check for missing required parameters */
	error = got_required_parameters(aevent);
	if (error)
		abortive_error(error);

	/* Count the number of document in the list */
	error = AECountItems(&document_list, &num_docs);
	if (error)
		abortive_error(error);
	
	/* Get each descriptor record from the list, and open the first file
		ending in ".sff".  We can only have one scene open at a time, and we
		can only open .sff files. */
	for (i = 1; i <= num_docs; i++)
		{
	
		/* Get the info for this file */
		error = AEGetNthPtr(&document_list, i, typeFSS, &keyword, &returned_type,
							(Ptr)&fss, sizeof(FSSpec), &actual_size);
		if (error)
			abortive_error(error);
		
		/* Does this file end in .sff or .scn?  If so, open it and we're done. */
		p2cstr(fss.name);
		if ((!strcmp((char *) (fss.name + strlen((char*)fss.name) - 4), ".sff")) ||
			(!strcmp((char *) (fss.name + strlen((char*)fss.name) - 4), ".scn")))
			{
			/* Make a working directory for the directory this scene's in */
			scene_file_wd_id = make_working_directory(fss.vRefNum, fss.parID);
	
			/* Save the scene file's filename */
			strcpy(scene_filename, (char *) fss.name);
	
			/* Open the scene */
			open_scene_file();
			break;
			}
		else
			abortive_string_error("MacRTrace can only open files ending in .sff or .scn.");
		}

}	/* open_docs_ae_handler() */



/*****************************************************************************\
* procedure got_required_parameters                                           *
*                                                                             *
* Purpose: This procedure checks for any further required parameters in an    *
*          Apple Event.                                                       *
*                                                                             *
* Parameters: aevent: the Apple Event.                                        *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

OSErr got_required_parameters(const AppleEvent *aevent)
{

	DescType	returned_type;
	Size		actual_size;
	OSErr		error;

	/* Check for any further required parameters */
	error = AEGetAttributePtr(aevent, keyMissedKeywordAttr, typeWildCard,
								&returned_type, NULL, 0, &actual_size);

	/* If another parameter was not found, there are no more, so we're done. */
	if (error = errAEDescNotFound)
		return (noErr);

	else if (error = noErr)
		return errAEEventNotHandled;	/* there's still a parameter */

	else
		return noErr;
		
}	/* got_required_parameters() */



/*****************************************************************************\
* procedure prepare_to_terminate                                              *
*                                                                             *
* Purpose: This procedure prompt the user whether or not to save the image or *
*          animation before quitting.                                         *
*                                                                             *
* Parameters: returns TRUE if user cancelled (doesn't want to quit)           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean prepare_to_terminate(void)
{

	short item_hit;

	/* If there is an unsaved image, prompt to save it. */
	if ((image_complete) && (!image_saved) && !GetCtlValue(animate_checkbox))
		{
		
		/* Choose the correct word, depending on whether it's an animation or
			a still image */
		ParamText(GetCtlValue(animate_checkbox) ? "\panimation sequence" : "\pimage", 0, 0, 0);

		/* Prompt the user whether (s)he wants to save the image/animation and quit,
			discard it and quit, or cancel the quit process */
		ShowWindow(prompt_save_dialog);
		SelectWindow(prompt_save_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item_hit);
		HideWindow(prompt_save_dialog);
		
		if (item_hit == PROMPT_SAVE_CANCEL_BUTTON)
			return TRUE;
		
		else if (item_hit == PROMPT_SAVE_SAVE_BUTTON)
			handle_save();

		}

	/* If there is a render or a scene read going on, prompt before quitting. */
	if ((rendering) || (reading_scene))
		{
		
		/* Choose the correct phrase, depending on whether we're rendering or
			reading the scene */
		ParamText(rendering ? "\pa render" : "\preading a scene", 0, 0, 0);

		/* Prompt the user whether (s)he wants to save the image/animation and quit,
			discard it and quit, or cancel the quit process */
		ShowWindow(quit_during_render_dialog);
		SelectWindow(quit_during_render_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item_hit);
		HideWindow(quit_during_render_dialog);
		
		if (item_hit == CANCEL_BUTTON)
			return TRUE;

		}

	/* User does want to quit */
	return FALSE;

}	/* prepare_to_terminate() */


/*****************************************************************************\
* procedure handle_key_down                                                   *
*                                                                             *
* Purpose: This procedure handles a key down event.                           *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 14, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void handle_key_down(EventRecord *event)
{

	short 	item_hit;
	long	menu_result;

	/* Check for menu shortcut */
	if (event->modifiers & cmdKey)
		{
		
		/* Command-period during render cancels */
		if ((event->message & charCodeMask) == '.')

			{

			/* If we're not rendering, don't do anything */
			if (!rendering)
				return;

			/* verify that user really wants to cancel */
			ShowWindow(cancel_dialog);
			SelectWindow(cancel_dialog);
			ModalDialog ((ModalFilterUPP) NULL, &item_hit);
			HideWindow(cancel_dialog);

			if (item_hit != DONT_ABORT_BUTTON)
				{
				cleanup(FALSE);
				
				/* Render cancelled-- restore startup environment
					and start again */
  				free_type(PARSE_TYPE);
 				free_type(TRACE_TYPE);
  				free_type(OTHER_TYPE);
				longjmp(environment, 1);
				
				}
			
			return;
			
			}
		
		/* Find menu equivalent */
		menu_result = MenuKey(event->message);
		
		/* Pretend user clicked menu */
		if (HiWord(menu_result) != 0)	/* valid key? */
			do_menu (menu_result);		/* yes, map it to menu item */
		}

}	/* handle_key_down() */



/*****************************************************************************\
* procedure handle_high_level_event                                           *
*                                                                             *
* Purpose: This procedure handles a high level event.                         *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void handle_high_level_event(EventRecord *event)
{
	
	/* Process this AppleEvent */
	AEProcessAppleEvent(event);
	
}	/* handle_high_level_event() */



/*****************************************************************************\
* procedure handle_multifinder_event                                          *
*                                                                             *
* Purpose: This procedure handles a multifinder event.                        *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 14, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void handle_multifinder_event(EventRecord *event)
{

	/* Check for multifinder event */
	if (event->message & 0x01000000)
		{

		/* Check for coming to foreground event */
		if (event->message & 0x00000001)
			{
			
			/* Coming to foreground... */
			in_background = FALSE;
			
			/* If there's a notification installed, remove it */
			if (notification_installed)
				{
				NMRemove(&notification);
				notification_installed = FALSE;
				}

			/* Activate log window if it's front */
			if (FrontWindow() == log_window)
				{
				
				update_scroll_bar_with_active(TRUE);
				DrawGrowIcon(log_window);
				
				}
			}
			
		else
			{
			
			/* Going to background... */
			in_background = TRUE;
			
			/* Deactivate the log window */
			if (FrontWindow() == log_window)
				{
				
				update_scroll_bar_with_active(FALSE);
				DrawGrowIcon(log_window);
				
				}
			}
		}
		
}	/* handle_multifinder_event() */



/*****************************************************************************\
* procedure handle_dialog_event                                               *
*                                                                             *
* Purpose: This procedure handles a dialog event.                             *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 14, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean handle_dialog_event(EventRecord *event)
{
	
	DialogPtr	dialog;
	short		item_hit;
	
	/* Find if there's really a dialog event */
	if (IsDialogEvent(event))
		{
	
		/* If it's a return or an enter, treat it like a click of the default button */
		if ((event->what == keyDown) &&
			(((event->message & charCodeMask) == '\r') ||
				((event->message & keyCodeMask) == 0x4C00)))
			
			{
			/* If it's in the options dialog, fake a Render click */
			if (FrontWindow() == options_dialog)
				{
				
				/* Fake a click in the Render button */
				HiliteControl(render_button_handle, inButton);
				
				/* handle a click in the Render button */
				handle_options_selection(RENDER_BUTTON);
				
				}
			
			/* if it's in the Animation dialog, fake an Okay click */
			else if (FrontWindow() == animation_dialog)
				{
				
				/* Fake a click in the Okay button */
				HiliteControl(animation_okay_button_handle, inButton);
				
				/* handle a click in the Okay button */
				handle_animation_selection(OKAY_BUTTON);
				
				}
			
			/* if it's in the Animation dialog, fake an Okay click */
			else if (FrontWindow() == preferences_dialog)
				{
				
				/* Fake a click in the Okay button */
				HiliteControl(preferences_okay_button_handle, inButton);
				
				/* handle a click in the Okay button */
				handle_preferences_selection(OKAY_BUTTON);
				
				}
			
			/* Don't handle this as a normal event */
			return TRUE;
			
			}

		if (DialogSelect (event, &dialog, &item_hit))
			
			{
			
			/* Something was selected-- handle it */
			if (dialog == options_dialog)
				handle_options_selection (item_hit);
	
			else if (dialog == animation_dialog)
				handle_animation_selection (item_hit);
			
			else if (dialog == preferences_dialog)
				handle_preferences_selection (item_hit);
			
			/* Handle this event normally if it's a menu shortcut.  If we don't
				do this here, menu shortcuts will not be available when a
				dialog is front and the cursor is in a textedit box */
			return !((event->what == keyDown) && (event->modifiers & cmdKey));
			
			}
		}
	
		
	/* Handle this as a normal event */
	return FALSE;

}	/* handle_dialog_event() */



/*****************************************************************************\
* procedure handle_update_event                                               *
*                                                                             *
* Purpose: This procedure handles a window update event.                      *
*                                                                             *
* Parameters: the_window: the window being updated.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 24, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_update_event(WindowPtr the_window)
{

	short copy_mode;

	/* Check for update of the image window */
	if (the_window == image_window)
		{
		/* start the update event */
		BeginUpdate(the_window);
		
		/* Prepare to draw in the image window */
		SetPort(image_window);
			
		/* Draw the offscreen picture in the window */
		DrawPicture(bitmap_pict, &((GrafPtr) the_window)->portRect);
		
		/* end the update event */
		EndUpdate (the_window);
		}

	/* Check for update of the log window */
	else if (the_window == log_window)
		update_log_window();

}	/* handle_update_event() */


/*****************************************************************************\
* procedure do_mouse_down                                                     *
*                                                                             *
* Purpose: This procedure handles a mouse down event.                         *
*                                                                             *
* Parameters: event: the event                                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void do_mouse_down(EventRecord *event)
{
	WindowPtr	mouse_window;
	int 		screen_area = FindWindow (event->where, &mouse_window);
	WStateData	**wstate_handle;
	GrafPtr		temp_port;
	
	switch (screen_area)
	{
		case inDesk:
			break;
		case inMenuBar:
			do_menu(MenuSelect(event->where));
			break;
		case inSysWindow:
			SystemClick(event, mouse_window);
			break;
		case inContent:
			if (mouse_window != FrontWindow())
				{
				SelectWindow (mouse_window);
				SetPort(mouse_window);
				}
			else if (mouse_window == log_window)
				handle_log_click(event->where);
			break;
		case inDrag:
		
			/* Bring window to front (this shouldn't be necessary, but without it
				a click in inDrag doesn't select a window during rendering) */
			if (mouse_window != FrontWindow())
				{
				SelectWindow (mouse_window);
				SetPort(mouse_window);
				}

			/* Let the user draw the window around */
			DragWindow (mouse_window, event->where, &drag_window_rect);
			
			/* If it's the options dialog, redraw it so all the popups
				find their new upper left point */
			if (mouse_window == options_dialog)
				DrawDialog(options_dialog);
			
			/* Find the new window position, and save it (update WStateData) */
			wstate_handle = (WStateData **) ( (WindowPeek) mouse_window )->dataHandle;

			/* Save the window position, in local coordinates */
			BlockMove(&(mouse_window->portRect), &(*wstate_handle)->userState, 8);

			/* Convert the position to global coordinates */
			GetPort(&temp_port);
			SetPort(mouse_window);
			LocalToGlobal( (Point *) &(*wstate_handle)->userState);
			LocalToGlobal( (Point *) &((*wstate_handle)->userState.bottom));
			SetPort(temp_port);
			
			break;
		case inGrow:
			grow_log_window(event->where);
			break;
		case inGoAway:
		
			/* Track the goaway box until user releases mouse button */
			if (TrackGoAway (mouse_window, event->where))
				{
				
				/* User released button inside goaway box-- which window
					should we hide? */
				if (mouse_window == options_dialog)
					hide_options_dialog();
				else if (mouse_window == status_dialog)
					hide_status_dialog();
				else if (mouse_window == animation_dialog)
					hide_animation_dialog();
				else if (mouse_window == image_window)
					hide_image_window();
				else if (mouse_window == log_window)
					hide_log_window();
				
				}
				
			break;
		case inZoomIn:
		case inZoomOut:
			
			/* Only the log window has a zoom box */
			handle_log_window_zoom(event->where, screen_area);

		default:
			break;
	}
	
}	/* do_mouse_down() */



/*****************************************************************************\
* procedure do_menu                                                           *
*                                                                             *
* Purpose: This procedure handles a pulldown menu selection.                  *
*                                                                             *
* Parameters: command: the menu selection; menu id in low word, menu item in  *
*                      high word                                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void do_menu(long command)
{
	int		menu_id = HiWord(command);
	int		item = LoWord(command);
	Str255	item_name;
	char	pathname[200];
	
	switch(menu_id)
	{
		case APPLE_MENU:
			if (item == ABOUT_ITEM) 
				show_about_dialog();
			else
			{
				GetItem(GetMHandle(menu_id), item, item_name);
				OpenDeskAcc(item_name);
			}
				break;
				
		case FILE_MENU:
			
			switch (item)
			{
				case OPEN_ITEM:
					/* Get the filename of the scene file */
					if (!choose_scene_file())
						return;

					open_scene_file();
					break;

				case SAVE_ITEM:
					handle_save();
					break;	
				case QUIT_ITEM:
					
					/* Make sure user has a change to save file, or to abort the
						quit */
					if (!prepare_to_terminate())
						{
						
						/* Get rid of any temp files left over from last render,
							and get rid of the movie file */
						delete_temp_files(TRUE);
				
						/* Write the preferences to disk */
						write_preferences();
						
						/* Quit */
						ExitToShell();

						}
				
			}
			
			break;

		case EDIT_MENU:
		
			do_edit(item);

			break;

		case WINDOWS_MENU:	/* Show/Hide Windows */
		
			switch (item)
			{
			case SHOW_OPTIONS_ITEM:
				
				/* Toggle the visibility of the options dialog */
				if (((WindowPeek)options_dialog)->visible)
					hide_options_dialog();
					
				else
					show_options_dialog(TRUE);
			
				break;

			case SHOW_LOG_ITEM:
				
				/* Toggle the visibility of the log window */
				if (((WindowPeek)log_window)->visible)
					hide_log_window();
					
				else
					show_log_window(TRUE);
			
				break;

			case SHOW_STATUS_ITEM:
				
				/* Toggle the visibility of the status dialog */
				if (((WindowPeek)status_dialog)->visible)
					hide_status_dialog();
					
				else
					show_status_dialog(TRUE);
			
				break;

			case SHOW_ANIMATION_ITEM:
				
				/* Toggle the visibility of the animation dialog */
				if (((WindowPeek)animation_dialog)->visible)
					hide_animation_dialog();
					
				else
					show_animation_dialog(TRUE);
			
				break;

			case SHOW_IMAGE_ITEM:
				
				/* Toggle the visibility of the image window */
				if (((WindowPeek)image_window)->visible)
					hide_image_window();
					
				else
					show_image_window(TRUE);
			
				break;
								
			case CLOSE_ITEM:

				if (FrontWindow() == options_dialog)
					hide_options_dialog();
				else if (FrontWindow() == log_window)
					hide_log_window();
				else if (FrontWindow() == status_dialog)
					hide_status_dialog();
				else if (FrontWindow() == animation_dialog)
					hide_animation_dialog();
				else if (FrontWindow() == image_window)
					hide_image_window();

				break;
				

			}

	}
	
	HiliteMenu(0);
}



/*****************************************************************************\
* procedure do_edit                                                           *
*                                                                             *
* Purpose: This procedure handles an Edit menu selection.                     *
*                                                                             * 
* Parameters: item: the item selected.                                        *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void do_edit(short item)
{
	
	OpenCPicParams	pic_params;		/* parameters for opening a picture */


	if (item == PREFERENCES_ITEM)
		show_preferences_dialog();	/* bring up the preferences dialog */
	
	if (item == COPY_ITEM)
		{
		
		/* Move the picture to the desk scrap */
		ZeroScrap();
		PutScrap(GetHandleSize((Handle) bitmap_pict), 'PICT', (Ptr) *bitmap_pict);

		}
		
}	/* do_edit() */



/*****************************************************************************\
* procedure open_scene_file                                                   *
*                                                                             *
* Purpose: This procedure opens the scene file.  The scene file has already   *
*          been selected, either by double-clicking/dragging in the Finder,   *
*          or by explicitly Opening from the File menu.                       *                                                  *
*                                                                             *                                                                             *
* Parameters: none; uses globals scene_wd_id and scene_filename.              *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  8/24/92          moved ray tracing code to ray_trace_sff     *
*   Greg Ferrar  1/26/95          Added support for SCN files                 *
\*****************************************************************************/

void	open_scene_file(void)
{

	char 	window_name[100];
	short	error;
	FInfo	finder_info;
	char	sff_filename[50];
	short	scene_filename_length;

	/* We're trying to read a scene; so the old one is gone now */
	scene_in_memory = FALSE;

	/* Get the length of the filename */
	scene_filename_length = strlen(scene_filename);

	/* Get the name of the scene by chopping off the .sff or .scn */
	strcpy(scene_name, scene_filename);
	scene_name[strlen(scene_name) - 4] = 0;

	/* Change the creator of the scene file to RTRC */
	c2pstr(scene_filename);
	error = GetFInfo((unsigned char *) scene_filename, scene_file_wd_id, &finder_info);
	if (error) abortive_error(error);
	finder_info.fdCreator = 'RTRC';
	error = SetFInfo ((unsigned char *) scene_filename, scene_file_wd_id, &finder_info);
	if (error) abortive_error(error);
	p2cstr((unsigned char *) scene_filename);

	/* Check if it's really a .scn file, rather than an .sff file */
	if (!strcmp(scene_filename + scene_filename_length - 4, ".scn"))
		{
		
		/* Generate the name of the temporary .sff file */
		sprintf(sff_filename, "rtrace_%s.sff", scene_name);
		
		/* Show the status dialog, if we're supposed to */
		if ((*prefs)->show_status_window_flag)
			{
			set_progress_bar_value(0);
			show_status_dialog(FALSE);
			}
		
		/* Tell the user we're converting */
		set_status_text("Converting .scn file to .sff file...");
		
		/* We're about to start converting the scene file */
		converting_scene = TRUE;
		
		/* Convert the .scn file to an .sff file */
		scn2sff(scene_file_wd_id, scene_filename, temp_folder_wd_id, sff_filename);
		
		/* Cleanup after the conversion */
		cleanup(TRUE);

		/* Unload the scn2sff segments.  We do this after each call, since
		   scn2sff it probably seldom needed and takes up a lot of space */
		UnloadSeg(scn2sff);
		UnloadSeg(create_sphere);
		UnloadSeg(yyparse);
		
		/* Now use the translated .sff file as the real scene file */
		strcpy(scene_filename, sff_filename);
		scene_file_wd_id = temp_folder_wd_id;

		}

	/* read in the eye, look, up and angles from the scene */
	read_view_info();

	/* Set the animation dialog fields according to the data read from the
		scene file */
	set_dialog_text_to_real(animation_dialog, X_EYE_EQ, eye.x);
	set_dialog_text_to_real(animation_dialog, Y_EYE_EQ, eye.y);
	set_dialog_text_to_real(animation_dialog, Z_EYE_EQ, eye.z);
	set_dialog_text_to_real(animation_dialog, X_LOOK_EQ, look.x);
	set_dialog_text_to_real(animation_dialog, Y_LOOK_EQ, look.y);
	set_dialog_text_to_real(animation_dialog, Z_LOOK_EQ, look.z);
	set_dialog_text_to_real(animation_dialog, X_UP_EQ, up.x);
	set_dialog_text_to_real(animation_dialog, Y_UP_EQ, up.y);
	set_dialog_text_to_real(animation_dialog, Z_UP_EQ, up.z);
	set_dialog_text_to_real(animation_dialog, ANGLE_X_EQ, 180*view_angle_x/PI);
	set_dialog_text_to_real(animation_dialog, ANGLE_Y_EQ, 180*view_angle_y/PI);

	/* Change the name of the options dialog to reflect the scene name */
	sprintf(window_name, "Options for %s", scene_name);
	SetWTitle (options_dialog, c2pstr(window_name));	
	
	/* Show the Options dialog if it's not visible */
	show_options_dialog(TRUE);

	/* Allow user to show or hide Options dialog from now on */
	EnableItem (windows_menu, SHOW_OPTIONS_ITEM);

	/* Start reading in the scene */
	read_scene();
	
}	/* handle_open() */


/*****************************************************************************\
* procedure handle_save                                                       *
*                                                                             *
* Purpose: This procedure is called when the user selects Save... from the    *
*          file menu.  It allows the user to select an output file and an     *
*          output file type.  It then saves the image to disk.                *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void	handle_save(void)
{
	
	Point	dialog_upper_left = {130, 80};
	SFReply	my_reply;			/* save file info */
	Str255	filename;			/* the filename of the file to save to */
	
	/* Use the name of the Image window as the filename */
	GetWTitle(image_window, filename);
	p2cstr(filename);
	strcat((char *) filename, ".pict");
	c2pstr((char *) filename);
	
	/* Put up the image save... dialog */
	SFPPutFile(dialog_upper_left, "\pSave image as:",
				filename, (DlgHookUPP) NULL, &my_reply, SAVE_DIALOG,
				save_image_filter_upp);

	/* Don't do anything if user cancelled */
	if (!my_reply.good)
		return;

	/* save the file according to its type */
	switch (save_file_type_menu_selection)
		{
		case 1:	save_pict_file(&my_reply);
				break;
		}

	/* Remember that the image has been saved */
	image_saved = TRUE;

}	/* handle_save() */

/*****************************************************************************\
* procedure install_get_event_vbl                                             *
*                                                                             *
* Purpose: This procedure installs the VBL task which makes get_another_event *
*          TRUE every few ticks.                                              *
*                                                                             *
* Parameters: how_often: how often to give up control (if -1, REMOVE the task)*
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 31, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

/* We use this structure so that the VBL task can find our A5 and use it to
	access our globals */
typedef struct
	{
	VBLTask the_vbl_task;
	long application_A5;		/* this field is unused on the Power Mac */
	} VBL_task_A5_struct;

VBL_task_A5_struct		event_call_task;	/* installed task, if any */

void install_get_event_vbl(void)
{

	/* Install the screen refresh interrupt */
	event_call_task.the_vbl_task.qType = vType;
	event_call_task.the_vbl_task.vblAddr = NewVBLProc(set_event_flag);
	event_call_task.the_vbl_task.vblCount = (*prefs)->time_between_events;
	event_call_task.the_vbl_task.vblPhase = 0; 
	
#if __powerc
	event_call_task.application_A5 = 0L;
#else
	/* Save the applicationsŐs A5 where the VBL tasks can find it; right 
	 	before the VBL Task structure.  We need the VBL Tasks to have access
	 	to the application globals, so they need to know what the applicationŐs
	 	A5 global pointer is. */
	event_call_task.application_A5 = (long) LMGetCurrentA5();	/* a Macintosh global */
#endif

	/* Install the task */		
	VInstall((QElemPtr)&event_call_task.the_vbl_task);

	/* Remember that the vbl task is installed */
	vbl_installed = TRUE;

}	/* install_get_event_vbl() */



/*****************************************************************************\
* procedure remove_get_event_vbl                                              *
*                                                                             *
* Purpose: This procedure removes the VBL task which makes get_another_event  *
*          TRUE every few ticks.                                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 31, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void remove_get_event_vbl(void)
{
		
	/* Remove the vbl task */
	VRemove ((QElemPtr)&event_call_task.the_vbl_task);
	
	/* Dispose of the VBL Proc's routine descriptor */
	DisposeRoutineDescriptor(event_call_task.the_vbl_task.vblAddr);

	/* The vbl task is no longer installed */
	vbl_installed = FALSE;

}	/* remove_get_event_vbl() */



/*****************************************************************************\
* procedure set_event_flag                                                    *
*                                                                             *
* Purpose: This is the VBL task which makes get_another_event TRUE every few  *
*          ticks.                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 31, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/
#if __powerc

/* PowerPC version */
pascal void set_event_flag(VBLTaskPtr vblTaskPtr)
{
	get_another_event = TRUE;	/* tell rtrace to get another event */
	
	/* Start another countdown until the next time we get called */
	vblTaskPtr->vblCount = (*prefs)->time_between_events;

}	/* set_event_flag() */

#else

pascal long GetVBLRec (void) = 0x2E88;

/* 68k version */
pascal void set_event_flag(void)
{
long					curA5;
VBL_task_A5_struct		*tempVBL;

	/* The base address of this tasks VBL structure is now in A0.  We can use
		this to get the applicationŐs A5 global pointer.  We need this to be
		able to access gZ80Yield */
	
	tempVBL = (VBL_task_A5_struct *) GetVBLRec ();
	curA5 = SetA5(tempVBL->application_A5);
/*	asm {
		move.l A5, -(SP)			; save current A5
		move.l -8(A0), A5			; get applicationsŐs A5
		}
*/
	get_another_event = TRUE;	/* tell rtrace to get another event */
	
	/* Start another countdown until the next time we get called */
	event_call_task.the_vbl_task.vblCount = (*prefs)->time_between_events;
	
	curA5 = SetA5(curA5);
/*	asm {
		move.l (SP)+, A5		; restore the old A5
		}
*/

}	/* set_event_flag() */

#endif


/*****************************************************************************\
* procedure read_scene                                                        *
*                                                                             *
* Purpose: This procedure reads in the scene from the .sff file.              *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void read_scene(void)
{

	params_struct	params;
	Rect			image_frame = {0, 0, 0, 0};
	short			item_hit;
	char			string[100];
	long			scene_time;
	double			tstart, tend;
	double			tstep;
	long			frame_number;
	short 			error;
	char			frame_filename[50];
	ParamBlockRec	pbRec;
	VolumeParam		*vPB = (VolumeParam *) &pbRec;
	long			picture_size;
	int				num_parameters;
	
	/* Free any RTrace memory, in case there is a scene already in memory */
	free_all();
	
	/* Show the status dialog, if we're supposed to */
	if ((*prefs)->show_status_window_flag)
		{
		set_progress_bar_value(0);
		show_status_dialog(FALSE);
		}
		
	/* Set the Render Button name appropriately */
	set_render_button_name();
	
	/* Get the time when we start, so we can tell how long it took */
	total_time = TickCount();

	/* Set the status to "Reading Scene..." */
	set_status_text("Reading Scene...");
	
	/* update the free memory bar */
	update_memory_bar_value();
	
	/* Remember that we're reading the scene */
	reading_scene = TRUE;
	
	/* Get an event whenever possible (maximize interaction during scene read) */
	get_another_event = TRUE;

	/* Add a header to the log window */
	add_line_to_log_window ("\r");
	sprintf(string, "#### Reading SFF file %s\r", scene_filename);
	add_line_to_log_window (string);
	add_line_to_log_window ("\r");

	/* If we're ever giving control to the background, install the VBL
		which will make get_another_event TRUE every time_between_events
		ticks */
	if ((*prefs)->allow_background_tasks)
		install_get_event_vbl();

	/* Don't EVER get an event if we're not supposed to. */
	get_another_event = FALSE;

	/* Set the status to "Setting up parameters..." */
	set_status_text("Setting up parameters...");

	/* Get the options for this file */
	params.current_param = params.current_loc = 0;
	generate_params(&params);

	/* Add parameter which tell RTrace to save this as a pmm file */
//	add_parameter (&params, "O1");
	
	/* Suppress output of parameters */
	add_parameter (&params, "V0");
	
	/* Add parameters which specify input sff filename and output ppm filename */
	add_parameter (&params, scene_filename);
	add_parameter (&params, "rtrace_temp");

	/* Set up the parameters (let RTrace know about the parameters) */
	num_parameters = params.current_param;
	get_parameters(num_parameters, params.parameters);
	
	/* Open the scene file */
	OPEN(scene, scene_filename, READ_TEXT);
	
	/* RTrace doesn't write to a picture file on the Mac, so we should close the
		picture file. */
	if (picture)
		CLOSE(picture);
	picture = NULL;

	/* Allocate some memory structures */
	ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX, PARSE_TYPE);
	ALLOCATE(light, light_struct, lights_max, PARSE_TYPE);
	ALLOCATE(surface, surface_ptr, surfaces_max + 1, PARSE_TYPE);
	ALLOCATE(object, object_ptr, objects_max, PARSE_TYPE);
	ALLOCATE(true_color, rgb_struct, screen_size_x + 1, TRACE_TYPE);
	ALLOCATE(back_mask, real, screen_size_x + 1, TRACE_TYPE);

	/* update the free memory in the status dialog */
	update_memory_bar_value();

	/* Read in the scene */	
	get_scene();
	setup_scene();

	/* Clean up after scene read */
	cleanup(TRUE);

	/* Tell the user that we're done.  If there's a notification manager, we use it.
		We only notify if we're in the background and we're stopping now (not
		immediately rendering) */
	if ((in_background) && (notification_manager_available) && !render_next)
		{
		
		/* Set up notification to flash an icon in the menu bar */
		notification.qType = nmType;				/* Notification Mgr Queue */
		notification.nmMark = 1;					/* Put diamond by appl name */
		notification.nmIcon = notification_icon;	/* flash this icon */
		notification.nmSound = (Handle) -1L;		/* beep */
		notification.nmStr = NULL;					/* no dialog */
		notification.nmResp = NULL;					/* no respose procedure */
		
		/* Install the notification */
		error = NMInstall(&notification);
		if (!error)
			notification_installed = TRUE;
		
		}


	/* If we should render now, do it */
	if (render_next)
		{
		
		/* Don't render after next scene read unless user wants it again */
		render_next = FALSE;
		
		/* Make sure the parameters are still valid; if so, render the scene. */
		if (are_valid_params())
			render_scene();
		
		else	/* Parameters were invalid */
		
			/* Change the button to "Render" or "Render & Save" */
			SetCTitle(render_button_handle,
						GetCtlValue(animate_checkbox) ? "\pRender & Save" : "\pRender");

		}

}	/* read_scene() */
	


/*****************************************************************************\
* procedure render_scene                                                      *
*                                                                             *
* Purpose: This renders the scene.                                            *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   December 28, 1994 by Greg Ferrar                                          *
*     made changes necessary to interface with new version 8.4.2 of RTrace.   *
*   January 3, 1995 by Greg Ferrar                                            *
*     separated code from ray_trace_sff into read_scene and render_scene.     *
\*****************************************************************************/

void render_scene(void)
{

	params_struct	params;
	Rect			image_frame = {0, 0, 0, 0};
	short			item_hit;
	char			string[100];
	long			scene_time;
	double			tstart, tend;
	double			tstep;
	long			frame_number;
	short 			error;
	char			frame_filename[50];
	ParamBlockRec	pbRec;
	VolumeParam		*vPB = (VolumeParam *) &pbRec;
	long			picture_size;
	Point			dialog_upper_left = {130, 80};
	int				num_parameters;
	char			*filename;
	SFReply			my_reply;

	
	/* If we're already rendering, prompt before we abort and start over */
	if (rendering)
		{
		
		/* Put up the Abort Rendering? alert */
		ShowWindow(abort_render_dialog);
		SelectWindow(abort_render_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item_hit);
		HideWindow(abort_render_dialog);

		/* If user cancelled, don't do anything */
		if (item_hit == ABORT_RENDER_CANCEL)
			return;
		
		/* Terminate the previous render */
		cleanup(FALSE);
		
		}

	/* If we need to reread the scene, do it now */
	if (reread_scene)
		{
		read_scene();
		reread_scene = FALSE;
		}

	/* We're just starting a new image; it's not complete */
	image_complete = FALSE;
		
	/* Get the time when we start, so we can tell how long it took */
	total_time = TickCount();

	/* Show the status dialog, if we're supposed to */
	if ((*prefs)->show_status_window_flag)
		{
		set_progress_bar_value(0);
		show_status_dialog(FALSE);
		}
		
	/* Hide the options dialog, if we're supposed to */
	if ((*prefs)->hide_options_window_flag)
		hide_options_dialog();
		
	/* Set the status to "Preparing to Render..." */
	set_status_text("Preparing to Render...");
	
	/* update the free memory bar */
	update_memory_bar_value();
	
	/* remember we are rendering */
	rendering = TRUE;
		
	/* Set the Render Button name appropriately */
	set_render_button_name();
	
	/* Get the options for this file */
	params.current_param = params.current_loc = 0;
	generate_params(&params);
	
	/* Add parameters which specify input sff filename and output ppm filename */
	add_parameter (&params, scene_filename);
	add_parameter (&params, "rtrace_temp");

	/* Set up the parameters (let RTrace know about the parameters) */
	num_parameters = params.current_param;
	get_parameters(num_parameters, params.parameters);
	
	/* RTrace doesn't write to a picture file on the Mac, so we should close the
		picture file. */
	if (picture)
		CLOSE(picture);
	picture = NULL;

	/* Get rid of any temp files left over from last render.  This also deletes
		the temporary .sff file, if we were reading a .scn file. */
	delete_temp_files(FALSE);

	/* Make a rectangle which encloses the image */
	image_frame.right = image_width;
	image_frame.bottom = image_height;

	/* Create an offscreen PICT */
	create_offscreen_pict(&image_frame);

	/* If we're supposed to animate, initialize the animation */
	if (GetCtlValue(animate_checkbox))
		{
		
		/* get the filename of the file to save to */
		filename = get_save_filename();
	
		/* Choose a location to save the animation */
		SFPPutFile(dialog_upper_left, "\pSave animation as:",
					c2pstr(filename), (DlgHookUPP) NULL, &my_reply, SAVE_ANIM_DIALOG,
					save_image_filter_upp);
		
		/* Don't do anything if user cancelled */
		if (!my_reply.good)
			{
			cleanup(false);
			return;
			}
		
		/* save the file according to its type */
		switch (save_anim_file_type_menu_selection)
			{
			case 1:
				
				/* We will saving a series of PICT files, not building a movie */
				building_quicktime_movie = FALSE;

				/* Remember the filename */
				p2cstr(my_reply.fName);
				strcpy(filename, (char *) my_reply.fName);
				
				break;
				
			case 2:
				init_quicktime_save(&my_reply);
				building_quicktime_movie = TRUE;
				break;
				
			}

		/* Get the animation info from the animation dialog */
		get_animation_info(&tstart, &tend, &frames);

		}

	/* Open the image window, if preferences say so. */
	if ((*prefs)->show_image_rendering)
		{

		int length;

		/* Get the title of the window */
		strcpy(string, scene_filename);
		
		/* If it ends with .sff, chop off the .sff */
		length = strlen(string);
		if ((length > 4) && (string[length-4] == '.') && (string[length-3] == 's') &&
			 (string[length-2] == 'f') && (string[length-1] == 'f'))
			 string[length-4] = 0;
		
		/* Set the window title */
		SetWTitle (image_window, c2pstr(scene_name));
		p2cstr((unsigned char *) scene_name);
			
		/* Resize the Image Window to the correct size */
		SizeWindow (image_window, image_width, image_height, FALSE);

		/* Make the image window visible */
		show_image_window(FALSE);
		
		/* Bring it to the front */
		SelectWindow(image_window);
		
		/* Make sure the entire contents gets updated (it normally
			wouldn't if the window were already visible) */
		InvalRect(&(image_window->portRect));
		}

	/* disable the open and save items in the file menu */
	DisableItem(file_menu, OPEN_ITEM);
	DisableItem(file_menu, SAVE_ITEM);

	/* If we're ever giving control to the background, install the VBL
		which will make get_another_event TRUE every time_between_events
		ticks */
	if ((*prefs)->allow_background_tasks)
		install_get_event_vbl();

	/* Don't EVER get an event if we're not supposed to. */
	get_another_event = FALSE;

	/* Set up the walk mode */
	if (walk_mode == 0)
		serp_make_pixel_list(screen_size_x + 1, screen_size_y + 1);
	else
		hilb_make_pixel_list(screen_size_x + 1, screen_size_y + 1);

	/* If we are animating, we need to generate a bunch of images */
	if (GetCtlValue(animate_checkbox))
		{

		frame_number = 1;
		tstep = (tend - tstart) / (frames - 1);
		for (t = tstart; t < tend*1.001; t += tstep, frame_number++)
			{

			/* Tell RTrace to draw as it renders */
			verbose_mode = -1;
	
			/* Set up the scene for this frame */
			setup_frame();

			/* update the free memory in the status dialog */
			update_memory_bar_value();

			/* Update the status window to say we're rendering this frame */
			sprintf(string, "Rendering frame %ld of %ldÉ", frame_number, frames);
			set_status_text(string);
			
			/* Render the scene */
			make_view();
			init_globals();
			ray_trace();
			
			/* Update the status window to say we're compressing this frame */
			sprintf(string, "Compressing frame %ld of %ldÉ", frame_number, frames);
			set_status_text(string);

			/* If we're building a quicktime movie, add a frame */
			if (building_quicktime_movie)
				compress_one_movie_frame(frame_number);
			
			/* Otherwise, save this frame as a PICT file */
			else
				{
				/* Generate the filename */
				sprintf((char *) my_reply.fName, "%s%d", filename, frame_number);
				c2pstr((char *) my_reply.fName);

				/* Save the Picture to a PICT file */				
				save_pict_file(&my_reply);
				
				}
			
			/* update the amount of free memory */
			update_memory_bar_value();
	
			}
		
		/* If we're building a movie, finish it now */
		if (building_quicktime_movie)
			finish_movie();
		
		}

	else
		{
		
		/* Update the status window to say we're rendering */
		set_status_text("RenderingÉ");
	
		/* Tell RTrace to draw as it renders */
		verbose_mode = -1;
	
		/* update the free memory in the status dialog */
		update_memory_bar_value();

		/* Render the scene */
		make_view();
		init_globals();
		ray_trace();
		
		}

	/* Do post-render code */ 
	cleanup(TRUE);

	/* Tell the user that we're done.  If there's a notification manager,
		we use it. */
	if ((in_background) && (notification_manager_available))
		{
		
		/* Set up notification to flash an icon in the menu bar */
		notification.qType = nmType;				/* Notification Mgr Queue */
		notification.nmMark = 1;					/* Put diamond by appl name */
		notification.nmIcon = notification_icon;	/* flash this icon */
		notification.nmSound = (Handle) -1L;		/* beep */
		notification.nmStr = NULL;					/* no dialog */
		notification.nmResp = NULL;					/* no respose procedure */
		
		/* Install the notification */
		error = NMInstall(&notification);
		if (!error)
			notification_installed = TRUE;
		
		}

	/* Remember that the image has been modified and should be saved */
	image_saved = FALSE;

}	/* render_scene() */
	


/*****************************************************************************\
* procedure cleanup                                                           *
*                                                                             *
* Purpose: This procedure cleans up after a scene read or a render.           *
*                                                                             *                                                                             *
* Parameter: natural: TRUE if rendering completed natually, FALSE if it was   *
*                     aborted or terminated by error.                         *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
*   January 3, 1995 by Greg Ferrar                                            *
*     Added cleanup for scene read                                            *
\*****************************************************************************/

void cleanup(Boolean natural)
{

	char	string[100];

	/* Remove the VBL task, if any */
	if (vbl_installed)
		remove_get_event_vbl();

	/* Clean up after converting an scn file to sff */
	if (converting_scene)
		{
		/* We're no longer converting the scene */
		converting_scene = FALSE;
		
		/* Free all the memory allocated while converting the scene */
		free_all();
		}

	/* Clean up after reading scene */
	if (reading_scene)
		{

		/* Set the progress bar back to thermometer mode */
		init_progress_bar_thermometer_mode();

		/* We're no longer reading the scene */
		reading_scene = FALSE;

		if (natural)
			{
			
			/* Scene was successfully read */
			scene_in_memory = TRUE;

			/* Activate render button */
			HiliteControl(render_button_handle, 0);

			/* Set the status to "Done reading scene.  Ready to render." */
			set_status_text("Done reading scene.  Ready to render.");
			
			}
		
		/* Scene read terminated with an error */
		else
			{
			
			/* Dim render button */
			HiliteControl(render_button_handle, 255);

			/* Hide the options window */
			hide_options_dialog();
			
			/* Set the status to "Scene read failed.  Ready to read scene." */
			set_status_text("Scene read failed.  Ready to read scene");
			
			}
		}
	
	/* Clean up after a render */
	if (rendering)
		{			
				
		/* Set the status string to say we're idle */
		set_status_text("Ready to render scene again.");

		/* we're no longer rendering */
		rendering = FALSE;

		/* Free the pixel list */
		if (walk_mode == 0)
			serp_free_pixel_list();
		else
			hilb_free_pixel_list();
		
		/* enable the open, close, and save items in the file menu */
		EnableItem(file_menu, OPEN_ITEM);
		EnableItem(edit_menu, PREFERENCES_ITEM);

		/* If there's a complete image to save or copy, enable the Save...
			and Copy... items */
		if (natural)
			{
			EnableItem(file_menu, SAVE_ITEM);
			EnableItem(edit_menu, COPY_ITEM);
	
			/* Remember that the image is complete */
			image_complete = TRUE;
		
			}
	
		/* If there was an error, free RTrace's internal temporary image */
		else
			{

			/* Free the temp image */
			if (temp_image)
				{
				FREE(temp_image);
				temp_image = NULL;
				}
		
			}
	
		/* Display the total elapsed time */
		sprintf(string, "Total time elapsed: %ld seconds\r",
							(TickCount() - total_time)/60);
		add_line_to_log_window (string);

		/* Set the progress bars to zero */
		set_progress_bar_value(0);

		/* If we were building a movie, clean up after it */
		if (building_quicktime_movie)
			{
			movie_cleanup();
			building_quicktime_movie = FALSE;
			}
		}

	/* Close files if they were left open */
	if (picture)
		CLOSE(picture);
	picture = NULL;
	if (scene)
		CLOSE(scene);
	scene = NULL;

	/* update the free memory bar */
	update_memory_bar_value();

	/* Set the Render Button name appropriately */
	set_render_button_name();
	
}	/* cleanup() */


#if __powerc
#pragma options align=mac68k
#endif

/* This represents the structure of a PICT containing a 24-bit bitmap */
typedef struct
	{
	short	size;
	Rect	frame;
	long	version;
	short	headerOpcode;
	short	version2;
	short	reserved;
	long	nativeHRes;
	long	nativeVRes;
	Rect	srcRect;
	long	reserved2;
	short	opcode1;
	long	opcode2;
	Rect	clipRect;
	short	opcode3;
	long	baseAddr;
	short	rowBytes;
	Rect	srcRect2;
	short	pixMapVersion;
	short	packingType;
	long	dataSize;
	long	horizRes;
	long	vertRes;
	short	pixelMode;
	short	bitsPerPixel;
	short	colorCompsPerPixel;
	short	colorCompSize;
	long	planeBytes;
	long	colorTableSeed;
	short	colorTableFlags;
	short	colorTableSize;
	Rect	srcRect3;
	Rect	srcRect4;
	short	transferMode;
	short	pixelData;
	} OffscreenPICTStruct;
	
#if __powerc
#pragma options align=reset
#endif

/*****************************************************************************\
* procedure create_offscreen_pict                                             *
*                                                                             *
* Purpose: This procedure creates an offscreen Picture which will be used to  *
*          keep the ray-traces image.                                         *
*                                                                             *                                                                             *
* Parameters: bounds_rect: the size of the desired bitmap.                    *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 24, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void create_offscreen_pict(Rect *bounds_rect)
{

	Rect			huge_rect = {-32767, -32767, 32767, 32767};
	short			width = bounds_rect->right - bounds_rect->left;
	short			height = bounds_rect->bottom - bounds_rect->top;
    short			row_bytes = 4*width;
	long			size = sizeof(OffscreenPICTStruct) + row_bytes*(height + 1);
    OffscreenPICTStruct	*pict;
	CGrafPtr 		temp_port;
	PixMapHandle	pixmap;
	RGBColor		rgb_color;

	/* If there's already an offscreen picture, free it */
	if (bitmap_pict)
		DisposeHandle((Handle) bitmap_pict);

	/* Allocate memory for the picture.  We need enough memory to hold
		the PICT header and the actual picture data.  This also clears
		the bitmap to white. */
	bitmap_pict = (PicHandle) NewHandleClear(size);

	/* Check for out of memory error */
	if (MemError() != noErr)
		{
		/* report error, and abort */
		abortive_string_error("Sorry, there is not enough memory to allocate a bitmap that large.");
		}

	/* Lock and dereference the handle.  The handles does not get unlocked. */
	HLock((Handle) bitmap_pict);
	pict = (OffscreenPICTStruct	*) *bitmap_pict;

	/* Fill in the pict header */
	pict->size = size;					/* Set the picture size */
	pict->frame = *bounds_rect;			/* Set the picture frame */
	pict->version = 0x001102FF;			/* Set the version opcode and number */
	pict->headerOpcode = 0x0C00;		/* Set the HeaderOp opcode */
	pict->version2 = 0xFFFE;			/* Set to -2 for extended version 2 picture */
	pict->reserved = 0;
	pict->nativeHRes = 0x00480000;		/* Horizontal and vertical resolutions 72dpi */
	pict->nativeVRes = 0x00480000;
	pict->srcRect = *bounds_rect;		/* Set the native source rectangle */
	pict->reserved2 = 0;
	pict->opcode1 = 0x001E;				/* DefHilite opcode */
	pict->opcode2 = 0x0001000A;			/* Clip opcode */
	pict->clipRect = huge_rect;			/* Clip to all QD space */
	pict->opcode3 = 0x009A;				/* DirectBitsRect opcode */
	pict->baseAddr = 0x000000FF;		/* For compatibility with earlier systems */
	pict->rowBytes = row_bytes | 0x8000;/* Bytes per row */
	pict->srcRect2 = *bounds_rect;		/* Source rectangle */
	pict->pixMapVersion = 0;			/* Version 0 PixMap */
	pict->packingType = 1;				/* No packing */
	pict->dataSize = row_bytes * height;/* Size of data */
	pict->horizRes = 0x00480000;		/* Horizontal and vertical resolutions 72dpi */
	pict->vertRes = 0x00480000;
	pict->pixelMode = RGBDirect;		/* Use direct pixels */
	pict->bitsPerPixel = 32;			/* 32 bit pixels */
	pict->colorCompsPerPixel = 3;		/* red, green, and blue components */
	pict->colorCompSize = 8;			/* 8 bits per channel */
	pict->planeBytes = 0;				/* offset to next plane */
	pict->colorTableSeed = 0x645970;	/* This value seems to work as a color table seed */
	pict->colorTableFlags = 0;			/* All flags 0 */
	pict->colorTableSize = 0;			/* Empty color table */
	pict->srcRect3 = *bounds_rect;		/* Source rectangle */
	pict->srcRect4 = *bounds_rect;		/* Destination rectangle */
	pict->transferMode =
		dither_copy_available ?
			ditherCopy : srcCopy;		/* Use dithering if available, else srcCopy */
	
	/* Put the End Of Picture opcode at the very end */
	*((short *) (((char *) &pict->pixelData) + pict->dataSize)) = 0x00FF;

	/* Get the offset of the bitmap data from the start of the PICT */
	offscreen_bitmap_base = (long *) &(pict->pixelData);

	/*******************************************************/
	/* We have made a Picture for the offscreen bitmap;    */
	/* now we make a PixMap which points to the same thing */
	/*******************************************************/
	
	/* Save the current grafPort */
	GetPort((GrafPtr *) &temp_port);
	
	/* Open a new port */
	OpenCPort(image_port);
	
	/* Get a pointer to the pixel map */
	pixmap = image_port->portPixMap;
	
	/* create a bitmap of the desired size */
	image_port->portRect = *bounds_rect;
	(*pixmap)->bounds = *bounds_rect;
	RectRgn(image_port->clipRgn, bounds_rect);
	RectRgn(image_port->visRgn, bounds_rect);
	
	/* Set the the pixmap to display direct 32-bit color */
	(*pixmap)->pmVersion = 4;
	(*pixmap)->pixelType = RGBDirect;
	(*pixmap)->pixelSize = 32;
	(*pixmap)->cmpCount = 3;
	(*pixmap)->cmpSize = 8;
	
	/* set the number of rows to the number of pixels per row * 4 */
	(*pixmap)->rowBytes = (bounds_rect->right - bounds_rect->left) * 4;
	
	/* Use the Picture bitmap for this PixMap's bitmap data */
	(*pixmap)->baseAddr = (char *) offscreen_bitmap_base;
	
	/* Set the 15th bit of rowBytes, so this is a PixMap */
	(*pixmap)->rowBytes |= 32768;

	/* Set the foreground color to black and the background color to white */
	SetPort ((GrafPtr) image_port);
	rgb_color.red = rgb_color.green = rgb_color.blue = 0xFFFF;
	RGBBackColor (&rgb_color);
	rgb_color.red = rgb_color.green = rgb_color.blue = 0x0000;
	RGBForeColor (&rgb_color);

	/* Clear the bitmap to white */
	EraseRect(bounds_rect);
	InvertRect(bounds_rect);
	
	/* Restore the port */
	SetPort((GrafPtr) temp_port);

}	/* create_offscreen_pict() */



#if 0

/*****************************************************************************\
* procedure create_offscreen_grafport                                         *
*                                                                             *
* Purpose: This procedure creates an offscreen grafport which will be used to *
*          keep the ray-traces image.                                         *
*                                                                             *                                                                             *
* Parameters: bounds_rect: the size of the desired bitmap.                    *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 24, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void create_offscreen_grafport(Rect *bounds_rect)
{

	CGrafPtr 		temp_port;
	PixMapHandle	pixmap;
	short			error;
	RGBColor		rgb_color;
	
	/* Save the current grafPort */
	GetPort((GrafPtr *) &temp_port);
	
	/* Open the new port */
	OpenCPort (image_port);
	
	/* Get a pointer to the pixel map */
	pixmap = image_port->portPixMap;
	
	/* create a bitmap of the desired size */
	image_port->portRect = *bounds_rect;
	(*pixmap)->bounds = *bounds_rect;
	RectRgn(image_port->clipRgn, bounds_rect);
	RectRgn(image_port->visRgn, bounds_rect);
	
	/* Set the the pixmap to display direct 32-bit color */
	(*pixmap)->pmVersion = 4;
	(*pixmap)->pixelType = RGBDirect;
	(*pixmap)->pixelSize = 32;
	(*pixmap)->cmpCount = 3;
	(*pixmap)->cmpSize = 8;
	
	/* set the number of rows to the number of pixels per row * 4 */
	(*pixmap)->rowBytes = (bounds_rect->right - bounds_rect->left) * 4;
	
	/* allocate space for the bitmap */
	offscreen_bitmap_base = (long *) NewPtr ((*pixmap)->rowBytes *
											(long) (bounds_rect->bottom - bounds_rect->top) );
	(*pixmap)->baseAddr = (char *) offscreen_bitmap_base;
	
	/* Check for out of memory error */
	if (MemError() != noErr)
		{
		/* releases memory used by CPort */
		CloseCPort(image_port);
		
		/* report error, and abort */
		abortive_string_error("Sorry, there is not enough memory to allocate the offscreen bitmap.");
		}

	/* Set the 15th bit of rowBytes, so this is a PixMap */
	(*pixmap)->rowBytes |= 32768;

	/* Set the foreground color to black and the background color to white */
	SetPort ((GrafPtr) image_port);
	rgb_color.red = rgb_color.green = rgb_color.blue = 0xFFFF;
	RGBBackColor (&rgb_color);
	rgb_color.red = rgb_color.green = rgb_color.blue = 0x0000;
	RGBForeColor (&rgb_color);

	/* Clear the bitmap to white */
	EraseRect(bounds_rect);
	InvertRect(bounds_rect);
	
	/* Remember that there is an offscreen port */
	is_offscreen_port = TRUE;

	/* Restore the port */
	SetPort((GrafPtr) temp_port);

}	/*	create_offscreen_grafport()	*/



/*****************************************************************************\
* procedure dispose_offscreen_port                                            *
*                                                                             *
* Purpose: This procedure gets rid of the offscreen grafport created by       *
*          create_offscreen_grafport.                                         *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 24, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void dispose_offscreen_port (void)
{

	/* Only dispose it if it exists */
	if (is_offscreen_port);
		{
		/* free the memory the bitmap used */
		DisposPtr ((*(image_port->portPixMap))->baseAddr);

		/* close the port */
		ClosePort ((GrafPtr) image_port);
	
		/* There's no longer an offscreen port */
		is_offscreen_port = FALSE;
		}
		
}	/* dispose_offscreen_port() */
#endif


