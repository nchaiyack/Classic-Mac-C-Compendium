/*****************************************************************************\
* macstuff.c                                                                  *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It contains    *
* procedures which initialize rtrace as a macintosh application.              *
\*****************************************************************************/

#include "rtresources.h"
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

/*	Think C likes all caps for routines in converted .o files. */

#ifdef	THINK_C
#define	OpenStdCompression	OPENSTDCOMPRESSION
#endif

#include "StdCompression.h"


/* externals */

extern ComponentInstance	ci;				/* returned from OpenStdCompression(), used in
											 *   QuickTime movie save dialog.
											 */
extern MenuHandle	save_anim_file_type_menu;
extern long			image_width;			/* size of the image to be generated */
extern long			image_height;
extern Boolean		show_image_rendering;	/* TRUE if we show the image in a window */
extern Boolean		keep_image_in_memory;	/* TRUE if we keep a pixmap in memory */
extern short		save_file_type_menu_selection;
extern short		save_anim_file_type_menu_selection;
extern char			last_log_line[];		/* The last line in the log text */
extern WindowPtr	 log_window;
extern FILE			*stderr_file;			/* file to which stderr is redirected */
extern fpos_t		current_stderr_read_pos;/* position we're reading in stderr */
extern Boolean		allow_background_tasks;	/* TRUE if we handle events and give time to
												background processed while rendering */
extern Boolean		show_status_window_flag;/* TRUE if the status dialog is shown each render */
extern Boolean		show_about_window_flag;	/* TRUE if the about window is shown at startup */
extern Boolean		hide_options_window_flag;/* TRUE if the options dialog is hidden each render */
extern long			time_between_events;	/* number of ticks between calls to WaitNextEvent */
extern DialogPtr	options_dialog;			/* the options dialog */
extern DialogPtr	preferences_dialog;		/* the preferences dialog */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern DialogPtr 	status_dialog;			/* the status dialog */
extern DialogPtr	cancel_dialog;			/* the Cancel dialog */
extern DialogPtr	abort_render_dialog;	/* the Abort Render dialog */
extern DialogPtr	prompt_save_dialog;		/* the Prompt To Save dialog */
extern ControlHandle render_button_handle;	/* handle to the Render button */
extern ControlHandle animation_okay_button_handle;/* handle to the Okay button in animation */
extern ControlHandle preferences_okay_button_handle;/* handle to the Okay button in preferences */
extern ControlHandle animate_checkbox;		/* handle to the animate checkbox in animation */
extern short		blessed_folder_wd_id;	/* Working Directory refnum of blessed folder */
extern short		rtrace_wd_id;			/* Working Directory refnum of RTrace folder */

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
Boolean	do_enclose;						/* TRUE if ray_trace() should enclose */
Boolean is_offscreen_port = FALSE;		/* TRUE when there is an offscreen CGrafPort */
Boolean	image_complete = FALSE;			/* TRUE if the image is completely rendered */
Boolean get_another_event = FALSE;		/* TRUE if we're due to get another event */
Boolean vbl_installed = FALSE;			/* TRUE if the VBL event task is installed */
Boolean scene_in_memory = FALSE;		/* TRUE if the scene for the current file is in memory */
Boolean rendering_same_file = FALSE;	/* TRUE if the file is same as last render */
Boolean	texture_mode_changed = TRUE;	/* TRUE if the texture mode is different than last render */
Boolean notification_installed = FALSE;	/* TRUE when a notification is installed */
Boolean	in_background = FALSE;			/* TRUE when the app is in the background */
Boolean	scene_file_open = FALSE;		/* TRUE when the scene file is open */
Boolean	picture_file_open = FALSE;		/* TRUE when the picture file is open */
Boolean	starting_up = TRUE;				/* TRUE if we're still starting up */
Boolean	image_saved = TRUE;				/* TRUE if the image has been saved since last modification */

short   display_depth;                  /* Actual display depth in bits: 1, 4, 8, or 32 bits */

NMRec	notification;					/* A notification */
Handle	notification_icon;				/* Handle to the notification icon */

long	number_noncluster_objects;		/* number of objects which are not cluster objects */
long	total_time;						/* Used to time rendering */
jmp_buf	environment;					/* Used to store environment to restore on error */

short	sff_file_vrefnum = 0;			/* The volume refnum of the sff file directory */
long	num_lights, num_surfaces;		/* The number of lights and surfaces in this scene */
long	frames;							/* The number of frames in the animation sequence */

/* rtrace data */
extern short int previous_repetitions;
short	SCREEN_SIZE_X_MAX;
short	SCREEN_SIZE_Y_MAX;
short	OBJECTS_MAX = 10000;	/* default */

char	sff_filename[200];		/* pathname of file to render */

/* Prototypes */
extern void init_mac(void);
void init_mgrs(void);
void init_menus(void);
void do_mouse_down(EventRecord *event);
void handle_open(FSSpec *file_spec);
void do_menu(long command);
void process_mac_event(void);
void debug_exit (void);
void handle_update_event(WindowPtr the_window);
void dispose_offscreen_port (void);
void create_offscreen_grafport(Rect *bounds_rect);
void do_mac(void);
void handle_save(void);
void do_edit(short item);
pascal void scroll_action_proc(void);
pascal void set_event_flag(void);
void remove_get_event_vbl(void);
void install_get_event_vbl(void);
void handle_key_down(EventRecord *event);
void handle_multifinder_event(EventRecord *event);
Boolean handle_dialog_event(EventRecord *event);
pascal OSErr open_app_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
pascal OSErr quit_app_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
pascal OSErr open_docs_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon);
void init_apple_events(void);
void handle_high_level_event(EventRecord *event);
OSErr got_required_parameters(AppleEvent *aevent);
Boolean prepare_to_terminate(void);


/* External Prototypes */
extern void init_alert_dialogs(void);
extern Boolean open_sff_file(char *filename);
extern void init_dialogs(void);
extern void show_about_dialog(void);
extern void add_parameter(params_struct *params, char *param);
extern void tc_ray_trace (void);
extern void get_parameters (int, char_ptr[]);
extern void get_scene (void);
extern void ray_trace (void);
extern void do_preferences_dialog(void);
extern void ray_trace_sff(void);
extern pascal Boolean save_image_filter (DialogPtr the_dialog, EventRecord *event,
											int *the_item);
extern char *get_save_filename (void);
extern void post_render(Boolean natural);
extern void do_stdout_check(void);
extern void setup_log_window(void);
extern void update_log_window (void);
extern void handle_log_click(Point where);
extern void save_pict_file(SFReply	*my_reply);
extern void save_ppm_file(SFReply *my_reply);
extern void save_quicktime_movie(SFReply *my_reply);
extern void save_pict_files(SFReply *my_reply);
extern void free_all(void);
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
extern void init_mac_alloc(void);
extern void preprocess_sff_file(long *num_lights, long *num_surfaces, long *num_objects);
extern void generate_params(params_struct *params);
extern void set_status_image_data_size(long image_size);
extern void add_line_to_log_window (char *line);
extern void set_status_num_objects(long num_objects);
extern void get_preferences(void);
extern void write_preferences(void);
extern void place_window (WindowPtr window, Rect *bounds);
extern void set_dialog_real(DialogPtr dialog, short item, real number);
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



/*****************************************************************************\
* procedure do_mac                                                            *
*                                                                             *
* Purpose: This procedure initializes the mac application, and then goes off  *
*          to the event loop.                                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void do_mac(void)
{

	long	i;
	Str255	pict_filename;
	char	ppm_filename[255];

DB(" before init_mac ");
	
 	/* Initialize */
	init_mac();

DB(" after init_mac ");
 
	/* We're no longer starting up */
	starting_up = FALSE;

	/* Put up the about dialog as an intro, if prefs say to */
	if (show_about_window_flag)
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
*   Reid Judd	Oct. 18, '92	Fix bug in QuickTime movie output. Init the   *
*								  ComponentInstance for OpenStdCompression()  *
*								  before EnterMovies()						  *
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
		ci = OpenStdCompression();
		if (ci == NULL) 
			terminal_string_error("Can't initialize QuickTime Standard Compression.");		
				 
		error = EnterMovies();
		if (error) 
			terminal_error(error);  
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
	notification_icon = GetResource ('ICN#', NOTIFICATION_ICON);
	attribs = GetResAttrs (notification_icon);
	SetResAttrs (notification_icon, attribs & (!resPurgeable));
	LoadResource(notification_icon);

	/* Set up window dragging region */
	drag_window_rect = screenBits.bounds;
	InsetRect (&drag_window_rect, 4, 4);

	/* Init the memory managment mechanism */
	init_mac_alloc();

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
	
	/* Grow the application zone to the maximum */
	MaxApplZone();

	/* Set up our temporary items folder */
	setup_temp_folder();

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

    GDHandle    gdh; 

	short		error;
	long		feature;
	long		quickdraw_version;
	SysEnvRec	sys_environ;
	
	gdh = GetGDevice();	
	display_depth = (**(**gdh).gdPMap).pixelSize;

	/* Find the blessed folder */
	error = SysEnvirons (1, &sys_environ);
	if (error) terminal_error(error);
	blessed_folder_wd_id = sys_environ.sysVRefNum;

	/* Check for availability of Gestalt, alert user if not present */
	if (GetTrapAddress(Gestalttest) == GetTrapAddress(NoTrap))
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

	/* If this isn't at least a 68020, abort */
	if (!mc68020_available)
		terminal_string_error("MacRTrace requires at least a 68020 processor.");		

	/* If there isn't an fpu, abort */
	if (!fpu_available)
		terminal_string_error("MacRTrace requires a math coprocessor.");		
	
	/* If this is Original QuickDraw, warn the user */
	if (!f8bit_QD_available)
		{
		possibly_terminal_string_error("MacRTrace prefers Color QuickDraw.  You may continue, \
but you will not be able to see the image or to use the clipboard.");		
		}

	/* If this is 8-bit QuickDraw, warn the user */
	else if (!f32bit_QD_available)
		{
		possibly_terminal_string_error("MacRTrace prefers 32-bit Color QuickDraw.  You may continue, \
but you will not be able to keep the image in memory or to use the clipboard.");		
		}
		
		
	DB(" This system has these capabilities......\n");	
	if (fpu_available)
	  DB("fpu_available.")
	else
	  DB("NO fpu_available.");
	
	if (mc68020_available)
	  DB("mc68020_available.")
	else
	  DB("NO mc68020_available.");
	
 	if (f8bit_QD_available)
 	  DB("f8bit_QD_available.")
 	else
 	  DB("NO f8bit_QD_available.");
 	
 	if (f32bit_QD_available)
 	  DB("f32bit_QD_available.")
 	else
 	  DB("NO f32bit_QD_available.");
 	
 	if (dither_copy_available)
 	  DB("dither_copy_available ")
 	else
 	  DB("NO dither_copy_available ");
	
 	if (find_folder_available)
 	  DB("find_folder_available.")
 	else
 	  DB("find_folder_available.");
 	
 	if (notification_manager_available)
 	  DB("notification_manager_available.")
 	else
 	  DB("NO notification_manager_available.");
 	
 	if (apple_events_available)
 	  DB("apple_events_available.")
 	else
 	  DB("NO apple_events_available.");
 	
 	if (quicktime_available)
 	  DB("quicktime_available.")
 	else
 	  DB("NO quicktime_available.");
 		
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

	InitGraf(&thePort);
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
				open_app_ae_handler, 0, FALSE);
	if (error) terminal_error(error);
	
	/* Install the OpenDocuments event handler */
	error = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
				open_docs_ae_handler, 0, FALSE);
	if (error) terminal_error(error);
	
	/* Install the QuitApplication event handler */
	error = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
				quit_app_ae_handler, 0, FALSE);
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

	char	error_message[100];

	/* look in stderr for an error message.  If fgets returns NULL, 
		there is nothing there. */
	fsetpos(stderr_file, &current_stderr_read_pos);
	if (!fgets (error_message, 10000, stderr_file))
		{

		/* Look at the end of the log text for an error message.  The
			last line is an error message if it begins with "Error:" */
		if (strstr(last_log_line, "Error:") == last_log_line)
			strcpy (error_message, last_log_line);
			
		else	/* unidentified error */
			strcpy (error_message, "Sorry, an unknown type of error occurred!");
		
		}
	
	else
		
		/* Chop of the newline from the stderr_file message */
		error_message[strlen(error_message)-1] = 0;
	
	/* Put up an error alert, clean up, and go back to the event loop */
	abortive_string_error(error_message);

	/* remember where we are in the stderr_file for later */
	fgetpos(stderr_file, &current_stderr_read_pos);
	
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
		
		/* Don't call again for a bit */
		get_another_event = FALSE;
	
		/* Get an event, allow background tasks time to work.  While rendering, we give
			background tasks minimum time; while not rendering, they get as a little more. */
		
		if (rendering)
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
				case mouseUp:
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
				case app4Evt:	
 					handle_multifinder_event(&event);
		
				case kHighLevelEvent:
  					handle_high_level_event(&event);
					break;
					
				default:
					break;
				}
			}
		}
		
	while (!rendering);
	
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

pascal OSErr open_app_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
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

pascal OSErr quit_app_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
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
*   Reid Judd   Oct 18,'92		Added 'if (error)' test before calling        *
*                                 abortive_error().                           *
\*****************************************************************************/

pascal OSErr open_docs_ae_handler(AppleEvent *aevent, AppleEvent *reply, long handler_refcon)
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
 		
		/* Does this file end in .sff?  If so, open it and we're done. */
		PtoCstr(fss.name);
		if (!strcmp((char *) (fss.name + strlen((char*)fss.name) - 4), ".sff"))
			{ 			
			CtoPstr(fss.name);
			handle_open(&fss);
			break;
			}
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

OSErr got_required_parameters(AppleEvent *aevent)
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

	/* If the image/animation is saved, don't do anything */
	if (!image_saved)
		{
		
		/* Choose the correct word, depending on whether it's an animation or
			a still image */
		ParamText(GetCtlValue(animate_checkbox) ? "\panimation sequence" : "\pimage", 0, 0, 0);

		/* Prompt the user whether (s)he wants to save the image/animation and quit,
			discard it and quit, or cancel the quit process */
		ShowWindow(prompt_save_dialog);
		SelectWindow(prompt_save_dialog);
		ModalDialog ((ProcPtr) NULL, &item_hit);
		HideWindow(prompt_save_dialog);
		
		if (item_hit == PROMPT_SAVE_CANCEL_BUTTON)
			return TRUE;
		
		else if (item_hit == PROMPT_SAVE_SAVE_BUTTON)
			handle_save();

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
			ModalDialog ((ProcPtr) NULL, &item_hit);
			HideWindow(cancel_dialog);

			if (item_hit != DONT_ABORT_BUTTON)
				{
				post_render(FALSE);
				
				/* Render cancelled-- restore startup environment
					and start again */
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
	static long	num_calls = 0;
	
	/* Find if there's really a dialog event */
	if (IsDialogEvent(event))
		{
	
		/* If it's a return, treat it like a click of the default button */
		if ( (event->what == keyDown) && 
		     ( ((event->message & charCodeMask) == '\r') ||
		       ((event->message & charCodeMask) == '\3') ) )			
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


		num_calls++;
		
		if (DialogSelect (event, &dialog, &item_hit))
			
			{
			
			/* Something was selected-- handle it */
			if (dialog == options_dialog)
				handle_options_selection (item_hit);
	
			else if (dialog == animation_dialog)
				handle_animation_selection (item_hit);
			
			else if (dialog == preferences_dialog)
				handle_preferences_selection (item_hit);
			
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
		
		/* Dither the image if dithering is available */
		copy_mode = (dither_copy_available) ? ditherCopy : srcCopy;
		
		/* If there's an offscreen pixmap, use it */
		if (is_offscreen_port)
			{
				
			/* copy the offscreen pixmap to the window */
			CopyBits (&((GrafPtr) image_port)->portBits,
						&((GrafPtr) the_window)->portBits,
						&(image_port->portRect), &(image_port->portRect),
						copy_mode, the_window->visRgn);
			}
		
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
*   Reid Judd	Oct.16'92	  Check for "if (mouse_window == log_window)" if  *
* 								we are doing an inGrow mouse event.           *
*                                                                             *
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
		    if (mouse_window == log_window)
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
	char	item_name[32];
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
					handle_open((FSSpec *) NULL);
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
	
	PicHandle		bitmap_picture;	/* a picture to be copied to the clipboard */
	OpenCPicParams	pic_params;		/* parameters for opening a picture */


	if (item == PREFERENCES_ITEM)
		show_preferences_dialog();	/* bring up the preferences dialog */
	
	if ((item == COPY_ITEM) && (keep_image_in_memory))
		{
		
		/* It's in memory-- make a PICT of it */
		BlockMove (&image_port->portRect, &pic_params.srcRect, 8);	/* copy the rect */
		pic_params.hRes = pic_params.vRes = (Fixed) 72 << 16;
		pic_params.version = -2;
		bitmap_picture = OpenCPicture (&pic_params);		

		/* copy the offscreen pixmap to the window */
		CopyBits (&((GrafPtr) image_port)->portBits,
					&((GrafPtr) image_window)->portBits,
					&(image_port->portRect), &(image_port->portRect),
					srcCopy, ((GrafPtr) image_window)->visRgn);
	
		/* The picture is done-- close it */
		ClosePicture();

		/* Move the picture to the desk scrap */
		ZeroScrap();
		HLock (bitmap_picture);
		PutScrap(GetHandleSize(bitmap_picture), 'PICT', *bitmap_picture);
		HUnlock (bitmap_picture);
		
		/* Free the picture */
		KillPicture(bitmap_picture);

		}
}



/*****************************************************************************\
* procedure handle_open                                                       *
*                                                                             *
* Purpose: This procedure is called when the user selects Open... from the    *
*          file menu.  It allows the user to select a file, and gets the      *
*          pathname of the file to give to RTrace.  It then calls RTrace to   *
*          create the image.                                                  *
*                                                                             *                                                                             *
* Parameters: file_spec: the file to open, NULL if we should prompt user.     *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  8/24/92          moved ray tracing code to ray_trace_sff     *
\*****************************************************************************/

void	handle_open(FSSpec *file_spec)
{

	char 	window_name[200];
	short	error;
	FInfo	finder_info;

 	if (file_spec)
		{
		
		/* Create a new working directory for the .sff directory */
		error = OpenWD (file_spec->vRefNum, file_spec->parID, 'RTRC', &sff_file_wd_id);
		if (error) abortive_error(error);
		
		/* Set the default directory to the .sff directory */
		SetVol ((StringPtr) NULL, sff_file_wd_id);

		/* Save the filename of this .sff file */
		strcpy (sff_filename, PtoCstr(file_spec->name));

		}
	
	else	/* Prompt the user for a file */
		{
		/* Get the filename of the .sff file */
		if (!open_sff_file(sff_filename))
			return;
		}

	/* Change the creator to RTRC */
	CtoPstr(sff_filename);
	error = GetFInfo (sff_filename, sff_file_wd_id, &finder_info);
	if (error) abortive_error(error);
	finder_info.fdCreator = 'RTRC';
	error = SetFInfo (sff_filename, sff_file_wd_id, &finder_info);
	if (error) abortive_error(error);
	PtoCstr(sff_filename);

	/* count the number of objects, lights, and surfaces in the sff file, and
		read in the eye, look, up and angles from the scene */
	preprocess_sff_file(&num_lights, &num_surfaces, &number_noncluster_objects);

	/* Set the animation dialog fields according to the data read from the
		sff file */
	set_dialog_real(animation_dialog, X_EYE_EQ, eye.x);
	set_dialog_real(animation_dialog, Y_EYE_EQ, eye.y);
	set_dialog_real(animation_dialog, Z_EYE_EQ, eye.z);
	set_dialog_real(animation_dialog, X_LOOK_EQ, look.x);
	set_dialog_real(animation_dialog, Y_LOOK_EQ, look.y);
	set_dialog_real(animation_dialog, Z_LOOK_EQ, look.z);
	set_dialog_real(animation_dialog, X_UP_EQ, up.x);
	set_dialog_real(animation_dialog, Y_UP_EQ, up.y);
	set_dialog_real(animation_dialog, Z_UP_EQ, up.z);
	set_dialog_real(animation_dialog, ANGLE_X_EQ, 180*view_angle_x/PI);
	set_dialog_real(animation_dialog, ANGLE_Y_EQ, 180*view_angle_y/PI);

	/* Change the name of the options dialog to reflect the filename */
	strcpy(window_name, "Options for ");
	strcat(window_name, sff_filename);
	window_name[strlen(window_name) - 4] = 0;	/* chop off .sff */

	/* Rename the window */
	SetWTitle (options_dialog, CtoPstr(window_name));	
	
	/* Remember that this new scene is not memory */
	scene_in_memory = FALSE;
	
	/* Remember that we are dealing with a new file now */
	rendering_same_file = FALSE;
	
	/* Update the render button to say "Render" */
	update_render_button();
	
	/* Show the Options dialog if it's not visible */
	show_options_dialog(TRUE);

	/* Allow user to show or hide Options dialog from now on */
	EnableItem (windows_menu, SHOW_OPTIONS_ITEM);


}


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
	char	*filename;			/* the filename of the file to save to */
	
	/* get the filename of the file to save to */
	filename = get_save_filename();
	
	/* If this is animated, put up the animation save... dialog.  Otherwise,
		put up the image save... dialog. */
	if (GetCtlValue(animate_checkbox))
		{
		
		/* Put up the animation save... dialog */
		SFPPutFile(dialog_upper_left, "\pSave animation as:",
					CtoPstr(filename), (ProcPtr) 0, &my_reply, SAVE_ANIM_DIALOG,
					save_image_filter);
		
		/* Don't do anything if user cancelled */
		if (!my_reply.good)
			return;
		
		/* save the file according to its type */
		switch (save_anim_file_type_menu_selection)
			{
			case 1:	save_pict_files(&my_reply);
					break;
			case 2: save_quicktime_movie(&my_reply);
			}
		}
	else
		{
		
		/* Put up the image save... dialog */
		SFPPutFile(dialog_upper_left, "\pSave image as:",
					CtoPstr(filename), (ProcPtr) 0, &my_reply, SAVE_DIALOG,
					save_image_filter);

		/* Don't do anything if user cancelled */
		if (!my_reply.good)
			return;

		/* save the file according to its type */
		switch (save_file_type_menu_selection)
			{
			case 1:	save_pict_file(&my_reply);
					break;
			case 2: save_ppm_file(&my_reply);
			}

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
	long application_A5;
	VBLTask the_vbl_task;
	} VBL_task_A5_struct;

VBL_task_A5_struct		event_call_task;	/* installed task, if any */

void install_get_event_vbl(void)
{

	/* Install the screen refresh interrupt */
	event_call_task.the_vbl_task.qType = vType;
	event_call_task.the_vbl_task.vblAddr = (ProcPtr) set_event_flag;
	event_call_task.the_vbl_task.vblCount = time_between_events;
	event_call_task.the_vbl_task.vblPhase = 0; 
	
	/* Save the applicationsŐs A5 where the VBL tasks can find it; right 
	 	before the VBL Task structure.  We need the VBL Tasks to have access
	 	to the application globals, so they need to know what the applicationŐs
	 	A5 global pointer is. */
	event_call_task.application_A5 = (long) CurrentA5;	/* a Macintosh global */

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

pascal void set_event_flag(void)
{

	/* The base address of this tasks VBL structure is now in A0.  We can use
		this to get the applicationŐs A5 global pointer.  We need this to be
		able to access gZ80Yield */
	asm {
		move.l A5, -(SP)			; save current A5
		move.l -8(A0), A5			; get applicationsŐs A5
		}
	
	get_another_event = TRUE;	/* tell rtrace to get another event */
	
	/* Start another countdown until the next time we get called */
	event_call_task.the_vbl_task.vblCount = time_between_events;
	
	asm {
		move.l (SP)+, A5		; restore the old A5
		}

}	/* set_event_flag() */



/*****************************************************************************\
* procedure ray_trace_sff                                                     *
*                                                                             *
* Purpose: This procedure is basically the interface between the mac aspect   *
*          of this program and the unix-style aspect.  It sets up the         *
*          variables for rtrace to do its ray tracing, and then calls it.     *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void ray_trace_sff(void)
	
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
	volumeParam		pb;
	long			picture_size;

	/* If we're already rendering, prompt before we abort and start over */
	if (rendering)
		{
		
		/* Put up the Abort Rendering? alert */
		ShowWindow(abort_render_dialog);
		SelectWindow(abort_render_dialog);
		ModalDialog ((ProcPtr) NULL, &item_hit);
		HideWindow(abort_render_dialog);

		/* If user cancelled, don't do anything */
		if (item_hit == ABORT_RENDER_CANCEL)
			return;
		
		/* Terminate the previous render */
		post_render(FALSE);
		
		}

	/* Get rid of any temp files left over from last render */
	delete_temp_files(FALSE);

	/* Get the time when we start, so we can tell how long it took */
	total_time = TickCount();

	/* If we're supposed to animate, get the information from the animation dialog */
	if (GetCtlValue(animate_checkbox))
		{
		get_animation_info(&tstart, &tend, &frames);

		/* Find how much space a single picture takes on disk */
		picture_size = ((image_width - 1) * (image_height - 1) * 3 + 20);
	
		/* Generate an error if there's not enough space on the disk to
			store all animations */
		pb.ioVolIndex = 0;					/* use vrefnum */
		pb.ioVRefNum = temp_folder_wd_id;	/* the directory ~~temp files go to */
		error = PBGetVInfo(&pb, FALSE);
		if (error) abortive_error(error);

		/* If the amount of free space isn't enough, error */
		if ((pb.ioVAlBlkSiz * pb.ioVFrBlk) < (frames * picture_size))
			abortive_string_error("Not enough disk space to generate animation.");

		}


	/* Show the status dialog, if we're supposed to */
	if (show_status_window_flag)
		{
		set_progress_bar_value(0);
		show_status_dialog(FALSE);
		}
		
	/* Hide the options dialog, if we're supposed to */
	if (hide_options_window_flag)
		hide_options_dialog();
		
	/* Set the prompt to "Preparing for render" */
	if (status_dialog_visible) set_status_text("\pPreparing for RenderÉ");
	
	/* update the amount of free memory */
	update_status_free_memory();
	
	/* Get the options for this file */
	params.current_param = params.current_loc = 0;
	generate_params(&params);

	/* Add parameter which tell RTrace to save this as a pmm file */
	add_parameter (&params, "O1");
	
	/* Add parameters which specify input sff filename and output ppm filename */
	add_parameter (&params, sff_filename);
	add_parameter (&params, "~~rttemp.ppm");

	/* Make a rectangle which encloses the image */
	image_frame.right = image_width;
	image_frame.bottom = image_height;

	/* If we are supposed to keep an offscreen PixMap of the image, create
		it now.  If we don't have 32-bit QD, don't do it regardless. */
	if (keep_image_in_memory && f32bit_QD_available)
		{
		/* If there's already an offscreen PixMap, dispose of it */
		if (is_offscreen_port)
			dispose_offscreen_port();
		
		/* Create an offscreen CGrafPort */
		create_offscreen_grafport(&image_frame);

		/* Update the image data size in the status dialog */
		set_status_image_data_size(
						GetPtrSize( ((*(image_port->portPixMap))->baseAddr)) +
							sizeof(CGrafPort));

		}
	
	/* Open the image window, if preferences say so.  If we don't have color
		QuickDraw, don't do it regardless */
	if (show_image_rendering && f8bit_QD_available)
		{

		/* Get the title of the window */
		strcpy(string, sff_filename);
		string[strlen(string) - 4] = 0;	/* chop off .sff */
		
		/* Set the window title */
		SetWTitle (image_window, CtoPstr(string));
			
		/* Resize the Image Window to the correct size */
		SizeWindow (image_window, image_width, image_height, FALSE);

		/* Make the image window visible */
		show_image_window(FALSE);
		}

	/* disable the open and save items in the file menu */
	DisableItem(file_menu, OPEN_ITEM);
	DisableItem(file_menu, SAVE_ITEM);
	
	/* If we're faking a button click, dehilight it */
	HiliteControl(render_button_handle, 0);

	/* remember we are rendering */
	rendering = TRUE;
		
	/* assume that the image is complete until user cancels */
	image_complete = TRUE;
		
	/* start drawing at the top of the window */
	current_line = 0;

	/* Set the "maximum" number of objects and image size to the
		actual values for this scene */
	SCREEN_SIZE_X_MAX = image_width + 1;
	SCREEN_SIZE_Y_MAX = image_height + 1;

	/* Set variables which depend on these maximums */
	previous_repetitions = SCREEN_SIZE_X_MAX;

	/* If we're ever giving control to the background, install the VBL
		which will make get_another_event TRUE every time_between_events
		ticks */
	if (allow_background_tasks)
		install_get_event_vbl();

	/* Don't EVER get an event if we're not supposed to. */
	get_another_event = FALSE;

	/* Load in scene information, unless we are Rendering Again.  We Render
		Again when there is no need to read scene information, which happens
		when the scene is still in memory, and we are rendering the same file,
		and the texture mode has not changed */
	if ((!scene_in_memory) || (!rendering_same_file) || (texture_mode_changed))
		{

		/* Add a header to the log window */
		add_line_to_log_window ("\r");
		sprintf(string, "#### Rendering SFF file %s\r", sff_filename);
		add_line_to_log_window (string);
		add_line_to_log_window ("\r");

		/* Set the status string to say we're preprocessing the sff file */
		if (status_dialog_visible) set_status_text("\pPreprocessing .sff fileÉ");
		
		/* Use twice the number of scene objects as an upper bound for the
			total possible number of objects (including cluster objects) */
		OBJECTS_MAX = 2*number_noncluster_objects + 1000;

		/* Set the number of objects text in the status dialog */
		set_status_num_objects(number_noncluster_objects);

		/* Remove all previous rtrace allocations from memory */
		free_all();
		
		/* update the free memory in the status dialog */
		update_status_free_memory();
		
		/* Allocate data structures that depend on SCREEN_SIZE_X_MAX */
		ALLOCATE(true_color, rgb_struct, SCREEN_SIZE_X_MAX);
		ALLOCATE(back_mask, real, SCREEN_SIZE_X_MAX);

		/* Tell ray_trace() to enclose as usual */
		do_enclose = TRUE;

		/* Tell rtrace the parameters */
		get_parameters(params.current_param - 1, &(params.parameters[1]));

		/* The files are open and haven't been closed yet */
		scene_file_open = picture_file_open = TRUE;

		/* Remember current time so we can later display time to read scene */
		scene_time = TickCount();

		/* Read in the new scene */
		get_scene();
		
		/* The scene file is now closed */
		scene_file_open = FALSE;
		
		/* Display the number of seconds it took */
		sprintf(string, "Time to read scene: %ld seconds\r",
							(TickCount() - scene_time)/60);
		add_line_to_log_window (string);

		/* Remember that the scene is in memory */
		scene_in_memory = TRUE;

		/* Assume we are rendering the same file until user changes
			file selection */
		rendering_same_file = TRUE;
		
		/* Assume textures mode will not change until user actually
			selects a new mode from the menu */
		texture_mode_changed = FALSE;
		
		}
		
	else
		{

		/* Add a header to the log window */
		add_line_to_log_window ("\r");
		sprintf(string, "#### Rerendering SFF file %s\r", sff_filename);
		add_line_to_log_window (string);
		add_line_to_log_window ("\r");

		/* Free previous data structures that depend on SCREEN_SIZE_X_MAX */
		FREE(true_color);
		FREE(back_mask);

		/* Allocate data structures that depend on SCREEN_SIZE_X_MAX */
		ALLOCATE(true_color, rgb_struct, SCREEN_SIZE_X_MAX);
		ALLOCATE(back_mask, real, SCREEN_SIZE_X_MAX);

		/* Tell ray_trace() NOT to enclose */
		do_enclose = FALSE;
		
		/* Just tell rtrace the new parameters */
		get_parameters(params.current_param - 1, &(params.parameters[1]));

		/* The files are open and haven't been closed yet */
		scene_file_open = picture_file_open = TRUE;

		}

	/* If we are animating, we need to generate a bunch of images */
	if (GetCtlValue(animate_checkbox))
		{

		/* we don't want to use ~~rttemp.ppm after all */
		CLOSE(picture);
		picture_file_open = FALSE;

		frame_number = 1;
		tstep = (tend - tstart) / (frames - 1);
		for (t = tstart; t < tend*1.001; t += tstep, frame_number++)
			{

			/* Don't enclose after the first frame */
			if (frame_number != 1)
				do_enclose = FALSE;

			/* Set up the scene for this frame */
			setup_frame();

			/* Open a new one with a numbered name */
			sprintf(string, "~~rttemp%ld.ppm", frame_number);
    		OPEN(picture, string, WRITE_BINARY);
			picture_file_open = TRUE;
			
			/* Also update the status window to say we're rendering */
			sprintf(string, "Rendering frame %ld of %ldÉ", frame_number, frames);
			if (status_dialog_visible) set_status_text(CtoPstr(string));
			
			/* Generator the scene */
			ray_trace();
			
			/* The picture file is closed now */
			picture_file_open = FALSE;
			
			/* update the amount of free memory */
			update_status_free_memory();
	
			}
			
		}

	else
		{
		
		/* Update the status window to say we're rendering */
		if (status_dialog_visible) set_status_text("\pRenderingÉ");
	
		/* Generate a single image */
		ray_trace();
		picture_file_open = FALSE;

		}
	
	/* Do post-render code */ 
	post_render(TRUE);

	/* Tell the user that we're done.  If there's a notification manager,
		we use it. */
	if ((in_background) && (notification_manager_available))
		{
		
		/* Set up notification to flash an icon in the menu bar */
		notification.qType = 8;				/* Notification Mgr Queue */
		notification.nmMark = 1;			/* Put diamond by appl name */
		notification.nmIcon = notification_icon;	/* flash this icon */
		notification.nmSound = NULL;		/* no sound */
		notification.nmStr = NULL;			/* no dialog */
		notification.nmResp = NULL;			/* no respose procedure */
		
		/* Install the notification */
		error = NMInstall(&notification);
		if (!error)
			notification_installed = TRUE;
		
		}

	/* Remember that the image has been modified and should be saved */
	image_saved = FALSE;

}	/* ray_trace_sff() */
	


/*****************************************************************************\
* procedure post_render                                                       *
*                                                                             *
* Purpose: This procedure is called after the rendering is done.              *
*                                                                             *                                                                             *
* Parameter: natural: TRUE if rendering completed natually, FALSE if it was   *
*                     aborted or terminated by error.                         *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
*   Reid Judd   Oct. 26,1992   Call CopyBits() with mode==ditherCopy after    *
*                               image has rendered so that it will be         *
*                               properly dithered on non 24-bit displays.	  *
\*****************************************************************************/

void post_render(Boolean natural)

{

	char	string[100];

	/* Set the status string to say we're idle */
	if (status_dialog_visible) set_status_text("\pIdle.");

	/* Update the free memory text */
	update_status_free_memory();
	
	/* Remove the VBL task, if any */
	if (vbl_installed)
		remove_get_event_vbl();

	/* Update the Render button to say "Render Again" */
	update_render_button();

	/* we're no longer rendering */
	rendering = FALSE;

	/* enable the open, close, and save items in the file menu */
	EnableItem(file_menu, OPEN_ITEM);
	EnableItem(edit_menu, PREFERENCES_ITEM);

	/* If there's a complete image to save or copy, enable the Save...
		and Copy... items */
	if (image_complete)
	{
		EnableItem(file_menu, SAVE_ITEM);
		EnableItem(edit_menu, COPY_ITEM);
	}
	
	/* Display the total elapsed time */
	sprintf(string, "Total time elapsed: %ld seconds\r",
						(TickCount() - total_time)/60);
	add_line_to_log_window (string);

	/*  Assume that you've got a display less than 32 bits deep,
	 *  the image is complete, and we're rendering to the 
	 *  display.  Copy the image from the offscreen bitmap to 
	 *  the display one more time using ditherCopy mode.
	 */		
	if ( (display_depth < 32) && (image_complete) && 
	     (keep_image_in_memory) && (is_offscreen_port) )
	{
	  CGrafPtr 	save_port;
	  Rect srcRect, dstRect;

      /* Save the current port */
	  GetPort (&save_port);

	  /* Set the grafport to the image_port */
  	  SetPort (image_window);
  	  
  	  srcRect = (**(*image_port).portPixMap).bounds;
	  dstRect = (*image_window).portRect;
  	   	 
  	  /* copy the offscreen pixmap to the window */
  	  
  	  if ( LockPixels( (*image_port).portPixMap ))
  	  {
	    CopyBits (&((GrafPtr) image_port)->portBits,
				&((GrafPtr) image_window)->portBits,
				&(image_port->portRect), &(image_port->portRect),
				ditherCopy, NULL );
  	  }
 	  UnlockPixels( (*image_port).portPixMap );
						 
	  /* Restore the grafport to whatever it was before */
  	  SetPort (save_port);   	  
    }

	/* Set the progress bars to zero */
	set_progress_bar_value(0);
	set_sub_progress_bar_value(0);

	/* Close files if they were left open */
	if (picture_file_open)
		CLOSE(picture);
	if (scene_file_open)
		CLOSE(scene);

}	/* post_render() */


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
	GetPort(&temp_port);
	
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
	(*pixmap)->pixelType = RGBDirect;
	(*pixmap)->pixelSize = 32;
	(*pixmap)->cmpCount = 3;
	(*pixmap)->cmpSize = 8;
	
	/* set the number of rows to the number of pixels per row * 4 */
	(*pixmap)->rowBytes = (bounds_rect->right - bounds_rect->left) * 4;
	
	/* allocate space for the bitmap */
	(*pixmap)->baseAddr = NewPtr ((*pixmap)->rowBytes *
								(long) (bounds_rect->bottom - bounds_rect->top) );
	
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
	SetPort (image_port);
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
	SetPort(temp_port);

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
		ClosePort (image_port);
	
		/* There's no longer an offscreen port */
		is_offscreen_port = FALSE;
		}
		
}	/* dispose_offscreen_port() */
