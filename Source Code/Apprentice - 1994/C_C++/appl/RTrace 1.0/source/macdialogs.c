/*****************************************************************************\
* macdialogs.c                                                                *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It implements  *
* a macintosh interface for RTrace, which gets user input from a dialog box   *
* and dumps it into rtrace as a command line.                                 *
\*****************************************************************************/

#include "rtresources.h"
#include "defs.h"
#include "mactypes.h"
#include "macdefaults.h"
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <setjmp.h>
#include <Movies.h>


/* Globals */
MenuHandle	antialiasing_menu;		/* Handles to the MENU resources of */
MenuHandle	lighting_menu;			/* the popup menus in the options   */
MenuHandle	normal_menu;			/* dialog.                          */
MenuHandle	intersect_menu;
MenuHandle	texture_menu;
MenuHandle	shading_menu;
MenuHandle	view_menu;
MenuHandle	stereo_separation_menu;
MenuHandle	sampling_level_menu;
MenuHandle	save_file_type_menu;
MenuHandle	save_anim_file_type_menu;

short		antialiasing_menu_selection = 1;	/* current selections in  */
short		lighting_menu_selection = 1;		/* the popup menus in the */
short		normal_menu_selection = 1;			/* options dialog.        */
short		intersect_menu_selection = 1;
short		texture_menu_selection = 1;
short		shading_menu_selection = 2;
short		view_menu_selection = 1;
short		stereo_separation_menu_selection = 1;
short		sampling_level_menu_selection = 1;
short		save_file_type_menu_selection = 1;
short		save_anim_file_type_menu_selection = 1;

Point		antialiasing_menu_upper_left;	/* Upper left corner of the   */
Point		lighting_menu_upper_left;		/* popup menus in the options */
Point		normal_menu_upper_left;			/* dialog (global coords).    */
Point		intersect_menu_upper_left;
Point		texture_menu_upper_left;
Point		shading_menu_upper_left;
Point		view_menu_upper_left;
Point		stereo_separation_menu_upper_left;
Point		sampling_level_menu_upper_left;
Point		save_file_type_menu_upper_left;
Point		save_anim_file_type_menu_upper_left;

ControlHandle	intersect_adjust_checkbox;		/* handles to the checkbox controls */
ControlHandle	use_jittered_sample_checkbox;
ControlHandle	correct_texture_normal_checkbox;
ControlHandle	specify_focal_distance_checkbox;
ControlHandle	allow_background_tasks_checkbox;
ControlHandle	show_image_rendering_checkbox;
ControlHandle	keep_image_in_memory_checkbox;
ControlHandle	show_status_window_checkbox;
ControlHandle	show_about_window_checkbox;
ControlHandle	hide_options_window_checkbox;
ControlHandle	animate_checkbox;

ControlHandle	render_button_handle;			/* handle to the Render button */
ControlHandle	animation_okay_button_handle;	/* handle to the Okay button in animation */
ControlHandle	preferences_okay_button_handle;	/* handle to the Okay button in preferences */
ControlHandle	about_more_button_handle;		/* handle to the More button in About... */

DialogPtr		options_dialog;			/* the options dialog */
DialogRecord	options_dialog_rec;		/* storage for the options dialog */
DialogPtr		preferences_dialog;		/* the preferences dialog */
DialogRecord	preferences_dialog_rec;	/* storage for the preferences dialog */
DialogPtr		status_dialog;			/* the status dialog */
DialogRecord	status_dialog_rec;		/* storage for the status dialog */
DialogPtr		animation_dialog;		/* the animation dialog */
DialogRecord	animation_dialog_rec;	/* storage for the animation dialog */
DialogPtr		about_dialog;			/* the about dialog */
DialogRecord	about_dialog_rec;		/* storage for the about dialog */
DialogPtr		abort_render_dialog;	/* the Abort Render dialog */
DialogRecord	abort_render_dialog_rec;/* storage for the Abort Render dialog */
DialogPtr		cancel_dialog;			/* the Cancel dialog */
DialogRecord	cancel_dialog_rec;		/* storage for the Cancel dialog */
DialogPtr		error_dialog;			/* the Error dialog */
DialogRecord	error_dialog_rec;		/* storage for the Error dialog */
DialogPtr		prompt_save_dialog;		/* the Prompt to Save dialog */
DialogRecord	prompt_save_dialog_rec;	/* storage for the Prompt to Save dialog */
DialogPtr		param_error_dialog;		/* the Parameter Error dialog */
DialogRecord	param_error_dialog_rec;	/* storage for the Parameter Error dialog */
DialogPtr		invalid_num_dialog;		/* the Invalid Number Error dialog */
DialogRecord	invalid_num_dialog_rec;	/* storage for the Invalid Number Error dialog */
DialogPtr		offer_to_abort_dialog;	/* the Offer to Abort dialog */
DialogRecord	offer_to_abort_dialog_rec;/* storage for the Offer to Abort dialog */
DialogPtr		saving_frame_dialog;	/* the Saving Frame dialog */
DialogRecord	saving_frame_dialog_rec;/* storage for the Saving Frame dialog */

long	image_width;	/* size of the image to be generated */
long	image_height;
long	time_between_events = 30;		/* number of ticks between calls to WaitNextEvent */
long	last_texture_selection = -1;	/* the last setting of the textures mode */
Boolean	allow_background_tasks = TRUE;	/* TRUE if we handle events and give time to
											background processed while rendering */
Boolean	show_image_rendering = TRUE;	/* TRUE if we show the image in a window */
Boolean	keep_image_in_memory = TRUE;	/* TRUE if we keep a pixmap of the image in memory */
Boolean	status_dialog_visible = FALSE;	/* TRUE if the status dialog is visible */
Boolean	animation_dialog_visible = FALSE;/* TRUE if the animation dialog is visible */
Boolean	image_window_visible = FALSE;	/* TRUE if the image window is visible */
Boolean	show_status_window_flag = TRUE;	/* TRUE if the status dialog is shown each render */
Boolean	show_about_window_flag = TRUE;	/* TRUE if the about window is shown at startup */
Boolean	hide_options_window_flag = FALSE;/* TRUE if the options dialog is hidden each render */

long	progress_bar_max = 1;			/* maximum value of the progress bar */
long	progress_bar_value = 0;			/* current_value of the progress bar */
short	progress_bar_length;			/* length of the progress bar in the dialog */
short	progress_bar_dividing_line;		/* length of the filled part of the progress bar */

long	sub_progress_bar_max = 1;		/* maximum value of the sub-progress bar */
long	sub_progress_bar_value = 0;		/* current_value of the sub-progress bar */
short	sub_progress_bar_length;		/* length of the sub-progress bar in the dialog */
short	sub_progress_bar_dividing_line;	/* length of the filled part of the sub-progress bar */

PicHandle about_picture;				/* Handle to picture in about... dialog */
PicHandle about_text_picture;			/* Handle to text picture in about... dialog */
PicHandle about_credits_picture;		/* Handle to credits picture in about... dialog */

RGBColor bar_blue = {0xD000, 0xD000, 0xFFFF};	/* colors of the progress bars */
RGBColor bar_gray = {0x4000, 0x4000, 0x4000};
RGBColor bar_black = {0x0000, 0x0000, 0x0000};

Rect	about_rtrace_image_bounds;		/* Enclosing rectangle of the RTrace image in About... */

/* externals */

extern Boolean		image_complete;		/* TRUE if the image is completely rendered */
extern char			sff_filename[200];	/* pathname of open .sff file */
extern Boolean 		is_offscreen_port;	/* TRUE when there is an offscreen CGrafPort */
extern WindowPtr	image_window;		/* the image window data structures */
extern Boolean		scene_in_memory;	/* TRUE if the scene for the current file is in memory */
extern Boolean		rendering_same_file;/* TRUE if the file is same as last render */
extern Boolean		texture_mode_changed;/* TRUE if the texture mode is different than last render */
extern PicHandle	down_arrow_picture;	/* down arrows for popup */
extern PicHandle	down_arrow_picture_grayed;
extern MenuHandle	windows_menu;		/* the Windows pulldown menu */
extern jmp_buf		environment;		/* Used to store environment to restore on error */
extern MenuHandle	file_menu;
extern Boolean		f8bit_QD_available;	/* TRUE if 8-bit QuickDraw is available */
extern Boolean		f32bit_QD_available;/* TRUE if 32-bit QuickDraw is available */
extern Boolean		quicktime_available;/* TRUE if QuickTime is available */
extern Boolean 		vbl_installed;		/* TRUE if the VBL event task is installed */
extern Boolean		rendering;			/* TRUE while we are rendering */
extern short		temp_folder_wd_id;	/* Working Directory refnum of Temporary Items */

extern short		display_depth;      /* Actual display depth in bits: 1, 4, 8, or 32 bits */


/* definition of the callback routine which the Dialog Manager uses
   when it calls a user item drawing routine */

typedef pascal void (*UserItemProc) (WindowPtr, short);


/* Prototypes */
void init_alert_dialogs(void);
void setup_useritems(void);
void show_options_dialog(Boolean bring_to_front);
void hide_options_dialog(void);
void handle_options_selection(short item_hit);
void draw_popup (Rect *the_box, Str255 the_string, short enabled);
void handle_antialiasing_popup_click(void);
void setup_popup_menu (short useritem_id, UserItemProc *proc);
void handle_popup_click(DialogPtr the_dialog, MenuHandle popup_menu, short menu_id,
						short popup_text_id, short popup_useritem_id,
						Point menu_upper_left, short *menu_selection);
void invert_item (short item_number);
void draw_any_popup (DialogPtr the_dialog, short item_number, MenuHandle menu,
							short menu_selection, Point *upper_left);
pascal void draw_button_border (WindowPtr the_window, short item_number);
pascal void draw_about_picture (WindowPtr the_window, short item_number);
pascal void draw_about_text_picture (WindowPtr the_window, short item_number);
pascal void draw_about_credits_picture (WindowPtr the_window, short item_number);
pascal void draw_progress_bar (WindowPtr the_window, short item_number);
pascal void draw_sub_progress_bar (WindowPtr the_window, short item_number);
pascal void draw_log_text (WindowPtr the_window, short item_number);
pascal void draw_antialiasing_popup (WindowPtr the_window, short item_number);
pascal void draw_lighting_popup (WindowPtr the_window, short item_number);
pascal void draw_normal_popup (WindowPtr the_window, short item_number);
pascal void draw_intersect_popup (WindowPtr the_window, short item_number);
pascal void draw_texture_popup (WindowPtr the_window, short item_number);
pascal void draw_shading_popup (WindowPtr the_window, short item_number);
pascal void draw_view_popup (WindowPtr the_window, short item_number);
pascal void draw_stereo_separation_popup (WindowPtr the_window, short item_number);
pascal void draw_sampling_level_popup (WindowPtr the_window, short item_number);
void install_popup_menu_hook(MenuHandle popup_menu);
static pascal void MenuProc( short message, MenuHandle menu, Rect *menuRect,
					Point hitPt, short *whichItem);
void FlushCache(void);
Boolean TrapAvailable( short theTrap);
void load_popup_menus(void);
pascal void dim_focal_distance (WindowPtr the_window, short item_number);			
pascal void dim_stereo_separation (WindowPtr the_window, short item_number);
pascal void dim_relinquish_control (WindowPtr the_window, short item_number);
pascal void dim_aliasing_threshold (WindowPtr the_window, short item_number);	
pascal void dim_keep_image_in_memory (WindowPtr the_window, short item_number);
pascal void dim_show_image_rendering (WindowPtr the_window, short item_number);
void setup_check_boxes(void);
void generate_params(params_struct *params);
void add_parameter(params_struct *params, char *param);
void handle_checkbox_click(ControlHandle check_box);
void show_about_dialog(void);
void init_dialogs(void);
void do_preferences_dialog(void);
pascal Boolean save_image_filter (DialogPtr the_dialog, EventRecord *the_event,
									int *the_item);
char *get_save_filename (void);
void post_render(Boolean natural);
void update_render_button(void);
void ray_trace_sff(void);
void reset_defaults(void);
void dispose_offscreen_port (void);
void show_options_dialog(Boolean bring_to_front);
void hide_options_dialog(void);
void show_status_dialog(Boolean bring_to_front);
void hide_status_dialog(void);
void show_image_window(Boolean bring_to_front);
void hide_image_window(void);
Boolean process_mac_event(void);
void update_status_free_memory (void);
void set_status_num_objects(long num_objects);
void set_status_image_data_size(long image_size);
void set_dialog_num(DialogPtr dialog, short item, long number);
void set_dialog_real(DialogPtr dialog, short item, real number);
void set_dialog_text(DialogPtr dialog, short item, Str255 string);
void set_dialog_pict(DialogPtr dialog, short item, short pic_id);
void set_status_text(Str255 string);
void set_progress_bar_value(long value);
void set_progress_bar_max(long value);
void set_sub_progress_bar_value(long value);
void set_sub_progress_bar_max(long value);
void update_sub_progress_bar (void);
void update_progress_bar (void);
void place_window (WindowPtr window, Rect *bounds);
void write_preferences(void);
void show_animation_dialog(Boolean bring_to_front);
void hide_animation_dialog(void);
void handle_animation_selection(short item_hit);
void remove_get_event_vbl(void);
void install_get_event_vbl(void);
Boolean is_valid_real(char *string);
Boolean is_valid_int(char *string);
Boolean is_valid_param_real(DialogPtr dialog, short text_item_id, short label_item_id,
							real min, real max, Boolean include_min, Boolean include_max);
Boolean is_valid_param_int(DialogPtr dialog, short text_item_id, short label_item_id,
							long min, long max);
Boolean are_valid_params(void);
void run_rtrace_movie(void);
void handle_preferences_selection(short item_hit);
void show_preferences_dialog(void);



/*****************************************************************************\
* procedure init_alert_dialogs                                                *
*                                                                             *
* Purpose: This procedure initializes the alert-style dialogs by loading them *
*          into memory and setting them up.                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 23, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_alert_dialogs(void)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	error_dialog = GetNewDialog (ERROR_DIALOG, &error_dialog_rec, (WindowPtr)-1L);
	offer_to_abort_dialog = GetNewDialog (OFFER_TO_ABORT_DIALOG, &offer_to_abort_dialog_rec, (WindowPtr)-1L);

	/* Center the windows on the screen */
	place_window(error_dialog, (Rect *) NULL);
	place_window(offer_to_abort_dialog, (Rect *) NULL);

	/* Set up the border around the default button in the Offer To Abort dialog */
	GetDItem (offer_to_abort_dialog, OFFER_ABORT_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (offer_to_abort_dialog, OFFER_ABORT_FRAME_BUTTON_USERITEM, item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Error dialog */
	GetDItem (error_dialog, ERROR_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (error_dialog, ERROR_FRAME_BUTTON_USERITEM, item_type, draw_button_border, &item_box);

}	/* init_alert_dialogs() */



/*****************************************************************************\
* procedure init_dialogs                                                      *
*                                                                             *
* Purpose: This procedure initializes the dialogs by loading them into memory *
*          and setting them up.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 23, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_dialogs(void)
{

	short	about_pict_id;
	short	about_text_pict_id;
	short	about_credits_pict_id;

	/* Read in the dialog boxes.  They are invisible */
	options_dialog = GetNewDialog (OPTIONS_DLG, &options_dialog_rec, (WindowPtr)-1L);
	preferences_dialog = GetNewDialog (PREFERENCES_DLG, &preferences_dialog_rec, (WindowPtr)-1L);
	status_dialog = GetNewDialog (STATUS_DIALOG, &status_dialog_rec, (WindowPtr)-1L);
	about_dialog = GetNewDialog (ABOUT_DLG, &about_dialog_rec, (WindowPtr)-1L);
	animation_dialog = GetNewDialog (ANIMATION_DIALOG, &animation_dialog_rec, (WindowPtr)-1L);
	abort_render_dialog = GetNewDialog (ABORT_RENDER_DIALOG, &abort_render_dialog_rec,
											(WindowPtr)-1L);
	cancel_dialog = GetNewDialog (CANCEL_DIALOG, &cancel_dialog_rec, (WindowPtr)-1L);
	saving_frame_dialog = GetNewDialog (SAVING_FRAME_DIALOG, &saving_frame_dialog_rec,
											(WindowPtr)-1L);
	param_error_dialog = GetNewDialog (PARAM_ERROR_DIALOG,
											&param_error_dialog_rec, (WindowPtr)-1L);
	prompt_save_dialog = GetNewDialog (PROMPT_SAVE_DIALOG,
											&prompt_save_dialog_rec, (WindowPtr)-1L);
	invalid_num_dialog = GetNewDialog (INVALID_NUM_DIALOG,
											&invalid_num_dialog_rec, (WindowPtr)-1L);
	
	/* Center the dialogs which should be centered */
	place_window(preferences_dialog, (Rect *) NULL);
	place_window(abort_render_dialog, (Rect *) NULL);
	place_window(cancel_dialog, (Rect *) NULL);
	place_window(saving_frame_dialog, (Rect *) NULL);
	place_window(about_dialog, (Rect *) NULL);
	place_window(param_error_dialog, (Rect *) NULL);
	place_window(prompt_save_dialog, (Rect *) NULL);
	place_window(invalid_num_dialog, (Rect *) NULL);

	/* Set the picture in the About... dialog to be as colorful as the
		monitor permits */
	if (display_depth >= 4)
		{
		if (display_depth == 32)
			{
			about_pict_id = ABOUT_PICT_32;
			about_text_pict_id = ABOUT_TEXT_PICT_32;
			about_credits_pict_id = ABOUT_CREDITS_8;
			}
			
		else
			{
			about_pict_id = ABOUT_PICT_8;
			about_text_pict_id = ABOUT_TEXT_PICT_8;
			about_credits_pict_id = ABOUT_CREDITS_8;
			}
		}
	else
		{
		about_pict_id = ABOUT_PICT_1;
		about_text_pict_id = ABOUT_TEXT_PICT_1;
		about_credits_pict_id = ABOUT_CREDITS_1;
		}
	
	/* Read the pictures into memory */
	about_picture = GetPicture(about_pict_id);
	about_text_picture = GetPicture(about_text_pict_id);
	about_credits_picture = GetPicture(about_credits_pict_id);
	
	/* Hide the credits picture for starters */
	HideDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM);
	
	/* Select the first EditText item in the options dialog box */
	SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);
	
	/* read in the popup menu resources for the options dialog box, and
		install the menu hooks */
	load_popup_menus();
	
	/* set up the check boxes in the options dialog box */
	setup_check_boxes();
	
	/* set up the popup menus and the frame around the active button */
	setup_useritems();

}	/* init_dialogs() */



/*****************************************************************************\
* procedure show_options_dialog                                               *
*                                                                             *
* Purpose: This procedure displays the options dialog.                        *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void show_options_dialog(Boolean bring_to_front)
{

	/* Show the window */
	ShowWindow (options_dialog);

	/* Bring it to the front if we're supposed to */
	SelectWindow (options_dialog);
	
	/* Select the Windows menu */
	EnableItem (windows_menu, 0);
	
	/* Set the text to "Hide Options Window" */
	SetItem (windows_menu, SHOW_OPTIONS_ITEM, "\pHide Options Window");
	
}	/* show_options_dialog() */



/*****************************************************************************\
* procedure hide_options_dialog                                               *
*                                                                             *
* Purpose: This procedure hides the options dialog.                           *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void hide_options_dialog(void)
{

	/* Hide the window */
	HideWindow (options_dialog);
	
	/* Set the text of the Windows menu to "Show Options Window" */
	SetItem (windows_menu, SHOW_OPTIONS_ITEM, "\pShow Options Window");
	
}	/* hide_options_dialog() */



/*****************************************************************************\
* procedure show_status_dialog                                                *
*                                                                             *
* Purpose: This procedure displays the status dialog.                         *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void show_status_dialog(Boolean bring_to_front)
{

	/* Show the window */
	ShowWindow (status_dialog);

	/* Bring it to the front if we're supposed to */
	SelectWindow (status_dialog);
	
	/* Set the text to "Hide Status Window" */
	SetItem (windows_menu, SHOW_STATUS_ITEM, "\pHide Status Window");
	
	/* Remember that the status dialog is visible */
	status_dialog_visible = TRUE;
	
}	/* show_status_dialog() */



/*****************************************************************************\
* procedure hide_status_dialog                                                *
*                                                                             *
* Purpose: This procedure hides the status dialog.                            *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void hide_status_dialog(void)
{

	/* Hide the window */
	HideWindow (status_dialog);
	
	/* Set the text of the Windows menu to "Show Status Window" */
	SetItem (windows_menu, SHOW_STATUS_ITEM, "\pShow Status Window");

	/* Remember that the status dialog isn't visible */
	status_dialog_visible = FALSE;

}	/* hide_status_dialog() */



/*****************************************************************************\
* procedure show_animation_dialog                                             *
*                                                                             *
* Purpose: This procedure displays the animation dialog.                      *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void show_animation_dialog(Boolean bring_to_front)
{

	/* Show the window */
	ShowWindow (animation_dialog);

	/* Bring it to the front if we're supposed to */
	SelectWindow (animation_dialog);
	
	/* Set the text to "Hide Animation Window" */
	SetItem (windows_menu, SHOW_ANIMATION_ITEM, "\pHide Animation Window");
	
	/* Remember that the animation dialog is visible */
	animation_dialog_visible = TRUE;
	
}	/* show_animation_dialog() */



/*****************************************************************************\
* procedure hide_animation_dialog                                             *
*                                                                             *
* Purpose: This procedure hides the animation dialog.                         *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void hide_animation_dialog(void)
{

	/* Hide the window */
	HideWindow (animation_dialog);
	
	/* Set the text of the Windows menu to "Show Animation Window" */
	SetItem (windows_menu, SHOW_ANIMATION_ITEM, "\pShow Animation Window");

	/* Remember that the animation dialog isn't visible */
	animation_dialog_visible = FALSE;

}	/* hide_animation_dialog() */



/*****************************************************************************\
* procedure show_image_window                                                 *
*                                                                             *
* Purpose: This procedure displays the image window.                          *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 11, 1992                                              *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void show_image_window(Boolean bring_to_front)
{

	/* Show the window */
	ShowWindow (image_window);

	/* Bring it to the front if we're supposed to */
	SelectWindow (image_window);
	
	/* Set the text to "Hide Image Window" */
	SetItem (windows_menu, SHOW_IMAGE_ITEM, "\pHide Image Window");
	
	/* Remember that the image window is visible */
	image_window_visible = TRUE;
	
}	/* show_image_window() */



/*****************************************************************************\
* procedure hide_image_window                                                 *
*                                                                             *
* Purpose: This procedure hides the image window.                             *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 11, 1992                                              *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void hide_image_window(void)
{

	/* Hide the window */
	HideWindow (image_window);
	
	/* Set the text of the Windows menu to "Show Image Window" */
	SetItem (windows_menu, SHOW_IMAGE_ITEM, "\pShow Image Window");

	/* Remember that the image window isn't visible */
	image_window_visible = FALSE;

}	/* hide_image_window() */



/*****************************************************************************\
* procedure handle_options_selection                                          *
*                                                                             *
* Purpose: This procedure handles a selection in the options dialog.          *
*                                                                             *
* Parameters: item_hit: the item which was selected.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/1/92           Made options dialog modeless                *
\*****************************************************************************/

void handle_options_selection(short item_hit)
{

	switch (item_hit)
		{
		
		/* If it was one of the popup menus, pop it up and get a selection */
		case ANTIALIASING_POPUP:
			handle_popup_click (options_dialog, antialiasing_menu, ANTIALIASING_MENU,
								ANTIALIASING_POPUP_TEXT, ANTIALIASING_POPUP,
								antialiasing_menu_upper_left,
								&antialiasing_menu_selection);
			break;

		case LIGHTING_POPUP:
			handle_popup_click (options_dialog, lighting_menu, LIGHTING_MENU,
								LIGHTING_POPUP_TEXT, LIGHTING_POPUP,
								lighting_menu_upper_left, &lighting_menu_selection);
			break;

		case NORMAL_POPUP:
			handle_popup_click (options_dialog, normal_menu, NORMAL_MENU,
								NORMAL_POPUP_TEXT, NORMAL_POPUP,
								normal_menu_upper_left, &normal_menu_selection);
			break;

		case INTERSECT_POPUP:
			handle_popup_click (options_dialog, intersect_menu, INTERSECT_MENU,
								INTERSECT_POPUP_TEXT, INTERSECT_POPUP,
								intersect_menu_upper_left, &intersect_menu_selection);
			break;

		case TEXTURE_POPUP:
			handle_popup_click (options_dialog, texture_menu, TEXTURE_MENU,
								TEXTURE_POPUP_TEXT, TEXTURE_POPUP,
								texture_menu_upper_left, &texture_menu_selection);
			break;

		case VIEW_POPUP:
			handle_popup_click (options_dialog, view_menu, VIEW_MENU,
								VIEW_POPUP_TEXT, VIEW_POPUP,
								view_menu_upper_left, &view_menu_selection);
			break;

		case STEREO_SEPARATION_POPUP:
			handle_popup_click (options_dialog, stereo_separation_menu,
								STEREO_SEPARATION_MENU, 0, STEREO_SEPARATION_POPUP,
								stereo_separation_menu_upper_left,
								&stereo_separation_menu_selection);
			break;

		case SHADING_POPUP:
			handle_popup_click (options_dialog, shading_menu, SHADING_MENU,
								SHADING_POPUP_TEXT, SHADING_POPUP,
								shading_menu_upper_left, &shading_menu_selection);
			break;

		case SAMPLING_LEVEL_POPUP:
			handle_popup_click (options_dialog, sampling_level_menu,
								SAMPLING_LEVEL_MENU,
								SAMPLING_LEVEL_TEXT, SAMPLING_LEVEL_POPUP,
								sampling_level_menu_upper_left,
								&sampling_level_menu_selection);
			break;

		/* If it was one of the checkboxes, toggle it */
		
		case INTERSECT_ADJUST_CHECKBOX:
			handle_checkbox_click (intersect_adjust_checkbox);
			break;
		
		case CORRECT_TEXTURE_NORMAL_CHECKBOX:
			handle_checkbox_click (correct_texture_normal_checkbox);
			break;
		
		case SPECIFY_FOCAL_DISTANCE_CHECKBOX:
			handle_checkbox_click (specify_focal_distance_checkbox);
			break;			
		
		case USE_JITTERED_SAMPLE_CHECKBOX:
			handle_checkbox_click (use_jittered_sample_checkbox);
			break;			
		
		case ANIMATE_CHECKBOX:
			handle_checkbox_click (animate_checkbox);
			break;			
		
		case RENDER_BUTTON:
		
			/* If the parameters are all valid, then render */
			if (are_valid_params())

				ray_trace_sff();

			else	/* there was an error */
				
				/* If we're faking a button click, dehilight it */
				HiliteControl(render_button_handle, 0);
			
			break;
			
		case DEFAULTS_BUTTON:
			reset_defaults();
			break;

		case ANIMATION_BUTTON:
			show_animation_dialog(TRUE);
			break;

		default:;	/* all other items take care of themselves */
		}

}	/* handle_options_selection() */



/*****************************************************************************\
* procedure handle_animation_selection                                        *
*                                                                             *
* Purpose: This procedure handles a selection in the animation dialog.        *
*                                                                             *
* Parameters: item_hit: the item which was selected.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void handle_animation_selection(short item_hit)
{

	switch (item_hit)
		{
		
		case OKAY_BUTTON:
		
			/* Check for invalid number of frames */
			if (!(is_valid_param_int(animation_dialog, NUM_FRAMES_NUM, NUM_FRAMES_LABEL,
										1, INT_MAX)))
				/* Don't quit just yet */
				item_hit = 0;

			else
				{
						
				/* Okay was clicked-- hide the dialog, turn on the
					animate checkbox, and change Save Image... to Save Animation... */
				hide_animation_dialog();
				SetCtlValue(animate_checkbox, TRUE);
				SetItem(file_menu, SAVE_ITEM, "\pSave Animation�");

				}
			/* If we're faking a button click, dehilight it */
			HiliteControl(animation_okay_button_handle, 0);

			break;
			
		case CANCEL_BUTTON:
			
			/* Cancel was clicked-- hide the dialog, turn off the
				animate checkbox, and change Save Animation... to
				Save Image... */
			hide_animation_dialog();
			SetCtlValue(animate_checkbox, FALSE);			
			SetItem(file_menu, SAVE_ITEM, "\pSave Image�");

			break;

		default:;	/* all other items take care of themselves */
		}

}	/* handle_animation_selection() */



/*****************************************************************************\
* procedure reset_defaults                                                    *
*                                                                             *
* Purpose: This procedure resets the options dialog to its default values.    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 1, 199                                                *
* Modified:                                                                   *
*   reid judd   Oct 13'92 	restore PtoCstr for default string values         *
\*****************************************************************************/

void	reset_defaults(void)
{

	short	type;		 /* the item type of a DITL item */
	Handle	handle;		 /* the handle to a DITL item */
	Rect	box;		 /* the bounding box of a DITL item */
	char    tmpstr[250]; /* temporary string to store default items in */

	/* This procedure is just a list of assignments, as each item in the
		dialog is reset to its default value */

	/* Set image width to default */
	GetDItem (options_dialog, IMAGE_WIDTH_NUM, &type, &handle, &box);
	strcpy( tmpstr, IMAGE_WIDTH_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 	
	/* Set image height to default */
	GetDItem (options_dialog, IMAGE_HEIGHT_NUM, &type, &handle, &box);
	strcpy( tmpstr, IMAGE_HEIGHT_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set aliasing threshold to default */
	GetDItem (options_dialog, ALIASING_THRESHOLD_NUM, &type, &handle, &box);
	strcpy( tmpstr, ALIASING_THRESHOLD_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set shading threshold to default */
	GetDItem (options_dialog, SHADING_THRESHOLD_NUM, &type, &handle, &box);
	strcpy( tmpstr, SHADING_THRESHOLD_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set ambient threshold to default */
	GetDItem (options_dialog, AMBIENT_THRESHOLD_NUM, &type, &handle, &box);
	strcpy( tmpstr, AMBIENT_THRESHOLD_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set focal aperature to default */
	GetDItem (options_dialog, FOCAL_APERATURE_NUM, &type, &handle, &box);
	strcpy( tmpstr, FOCAL_APERATURE_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set stereo separation to default */
	GetDItem (options_dialog, STEREO_SEPARATION_NUM, &type, &handle, &box);
	strcpy( tmpstr, STEREO_SEPARATION_DEFAULT );
	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
	
	/* Set cluster size to default */
	GetDItem (options_dialog, CLUSTER_SIZE_NUM, &type, &handle, &box);
	strcpy( tmpstr, CLUSTER_SIZE_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set ambient levels to default */
	GetDItem (options_dialog, AMBIENT_LEVELS_NUM, &type, &handle, &box);
	strcpy( tmpstr, AMBIENT_LEVEL_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set ambient samples to default */
	GetDItem (options_dialog, AMBIENT_SAMPLES_NUM, &type, &handle, &box);
	strcpy( tmpstr, AMBIENT_SAMPLES_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set shading levels to default */
	GetDItem (options_dialog, SHADING_LEVELS_NUM, &type, &handle, &box);
	strcpy( tmpstr, SHADING_LEVELS_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Set focal distance to default */
	GetDItem (options_dialog, FOCAL_DISTANCE_NUM, &type, &handle, &box);
	strcpy( tmpstr, FOCAL_DISTANCE_DEFAULT );
 	SetIText(handle, CtoPstr( tmpstr ) );
    PtoCstr( tmpstr );
 		
	/* Uncheck all menus */
	CheckItem (antialiasing_menu, antialiasing_menu_selection, FALSE);
	CheckItem (lighting_menu, lighting_menu_selection, FALSE);
	CheckItem (normal_menu, normal_menu_selection, FALSE);
	CheckItem (intersect_menu, intersect_menu_selection, FALSE);
	CheckItem (texture_menu, texture_menu_selection, FALSE);
	CheckItem (shading_menu, shading_menu_selection, FALSE);
	CheckItem (view_menu, view_menu_selection, FALSE);
	CheckItem (stereo_separation_menu, stereo_separation_menu_selection, FALSE);
	CheckItem (sampling_level_menu, sampling_level_menu_selection, FALSE);
	
	/* Set the menus to their default settings */
	antialiasing_menu_selection = ANTIALIASING_DEFAULT + 1;
	lighting_menu_selection = LIGHTING_DEFAULT + 1;
	normal_menu_selection = NORMAL_DEFAULT + 1;
	intersect_menu_selection = INTERSECT_DEFAULT + 1;
	texture_menu_selection = TEXTURE_DEFAULT + 1;
	shading_menu_selection = SHADING_DEFAULT + 1;
	view_menu_selection = VIEW_DEFAULT + 1;
	stereo_separation_menu_selection = STEREO_SEPARATION_MENU_DEFAULT + 1;
	sampling_level_menu_selection = SAMPLING_LEVEL_DEFAULT + 1;

	/* Check the new selections in all menus */
	CheckItem (antialiasing_menu, antialiasing_menu_selection, TRUE);
	CheckItem (lighting_menu, lighting_menu_selection, TRUE);
	CheckItem (normal_menu, normal_menu_selection, TRUE);
	CheckItem (intersect_menu, intersect_menu_selection, TRUE);
	CheckItem (texture_menu, texture_menu_selection, TRUE);
	CheckItem (shading_menu, shading_menu_selection, TRUE);
	CheckItem (view_menu, view_menu_selection, TRUE);
	CheckItem (stereo_separation_menu, stereo_separation_menu_selection, TRUE);
	CheckItem (sampling_level_menu, sampling_level_menu_selection, TRUE);
	
	/* Set the checkboxed to their defaults */
	SetCtlValue (correct_texture_normal_checkbox, CORRECT_TEXTURE_NORMAL_DEFAULT);
	SetCtlValue (intersect_adjust_checkbox, INTERSECT_ADJUST_DEFAULT);
	SetCtlValue (use_jittered_sample_checkbox, USE_JITTERED_SAMPLE_DEFAULT);
	SetCtlValue (specify_focal_distance_checkbox, SPECIFY_FOCAL_DISTANCE_DEFAULT);
	
	/* Update the dialog */
	UpdtDialog(options_dialog, options_dialog->visRgn);

}



/*****************************************************************************\
* procedure generate_params                                                   *
*                                                                             *
* Purpose: This procedure generates a parameter list for RTrace by looking at *
*          the options dialog.                                                *
*                                                                             *
* Parameters: params: the parameters list structure to create                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void	generate_params(params_struct *params)
{

	short	type;		/* the item type of a DITL item */
	Handle	handle;		/* the handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */
	char	current_setting_str[200];	/* current setting of an option */
	long	current_setting_num;		/* current setting of a menu option */
	char	this_param[200];			/* parameter we're generating */

	/* This ugly procedure is a long list of comparisons which follow the
	    following general format:
	    
	    	get_current_setting
			if {current_setting != default setting}
	        	add_option_to_command_line								*/

	/* start with a "RTrace" */
	add_parameter (params, "RTrace");
	
	/* do Image Width */
	GetDItem (options_dialog, IMAGE_WIDTH_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	StringToNum (current_setting_str, &image_width);
	if (strcmp (PtoCstr(current_setting_str), IMAGE_WIDTH_DEFAULT))
		{
		strcpy (this_param, "w");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Image Height */
	GetDItem (options_dialog, IMAGE_HEIGHT_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	StringToNum (current_setting_str, &image_height);
	if (strcmp (PtoCstr(current_setting_str), IMAGE_HEIGHT_DEFAULT))
		{
		strcpy (this_param, "h");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Aliasing Threshold */
	if (sampling_level_menu_selection != 1)
		{
		GetDItem (options_dialog, ALIASING_THRESHOLD_NUM, &type, &handle, &box);
		GetIText (handle, current_setting_str);
		if (strcmp (PtoCstr(current_setting_str), ALIASING_THRESHOLD_DEFAULT))
			{
			strcpy (this_param, "A");
			strcat (this_param, current_setting_str);
			add_parameter (params, this_param);
			}
		}

	/* do Shading Threshold */
	GetDItem (options_dialog, SHADING_THRESHOLD_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), SHADING_THRESHOLD_DEFAULT))
		{
		strcpy (this_param, "S");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Ambient Threshold */
	GetDItem (options_dialog, AMBIENT_THRESHOLD_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), AMBIENT_THRESHOLD_DEFAULT))
		{
		strcpy (this_param, "T");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Focal Aperature */
	GetDItem (options_dialog, FOCAL_APERATURE_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), FOCAL_APERATURE_DEFAULT))
		{
		strcpy (this_param, "P");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Stereo Separation */
	if (view_menu_selection != 1)
		{
		GetDItem (options_dialog, STEREO_SEPARATION_NUM, &type, &handle, &box);
		GetIText (handle, current_setting_str);
		if (strcmp (PtoCstr(current_setting_str), STEREO_SEPARATION_DEFAULT))
			{
			strcpy (this_param, "E");
			if (stereo_separation_menu_selection == 2)
				strcat (this_param, "-");
			strcat (this_param, current_setting_str);
			add_parameter (params, this_param);
			}
		}

	/* do Cluster Size */
	GetDItem (options_dialog, CLUSTER_SIZE_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), CLUSTER_SIZE_DEFAULT))
		{
		strcpy (this_param, "c");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Ambient Level */
	GetDItem (options_dialog, AMBIENT_LEVELS_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), AMBIENT_LEVEL_DEFAULT))
		{
		strcpy (this_param, "d");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Ambient Samples */
	GetDItem (options_dialog, AMBIENT_SAMPLES_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), AMBIENT_SAMPLES_DEFAULT))
		{
		strcpy (this_param, "D");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Shading Levels */
	GetDItem (options_dialog, SHADING_LEVELS_NUM, &type, &handle, &box);
	GetIText (handle, current_setting_str);
	if (strcmp (PtoCstr(current_setting_str), SHADING_LEVELS_DEFAULT))
		{
		strcpy (this_param, "s");
		strcat (this_param, current_setting_str);
		add_parameter (params, this_param);
		}

	/* do Focal Distance */
	if (GetCtlValue (specify_focal_distance_checkbox))
		{
		GetDItem (options_dialog, FOCAL_DISTANCE_NUM, &type, &handle, &box);
		GetIText (handle, current_setting_str);
		if (strcmp (PtoCstr(current_setting_str), FOCAL_DISTANCE_DEFAULT))
			{
			strcpy (this_param, "F");
			strcat (this_param, current_setting_str);
			add_parameter (params, this_param);
			}
		}

	/* do Sampling Levels */
	current_setting_num = sampling_level_menu_selection - 1;
	if (current_setting_num != SAMPLING_LEVEL_DEFAULT)
		{
		strcpy (this_param, "p");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Antialiasing mode */
	current_setting_num = antialiasing_menu_selection - 1;
	if (current_setting_num != ANTIALIASING_DEFAULT)
		{
		strcpy (this_param, "a");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Lighting Mode */
	current_setting_num = lighting_menu_selection - 1;
	if (current_setting_num != LIGHTING_DEFAULT)
		{
		strcpy (this_param, "l");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Normal Mode */
	current_setting_num = normal_menu_selection - 1;
	if (current_setting_num != NORMAL_DEFAULT)
		{
		strcpy (this_param, "n");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Texture Mode (also remember the current setting for later) */
	last_texture_selection = texture_menu_selection - 1;
	if (last_texture_selection != TEXTURE_DEFAULT)
		{
		strcpy (this_param, "t");
		NumToString (last_texture_selection, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do View Mode */
	current_setting_num = view_menu_selection - 1;
	if (current_setting_num != VIEW_DEFAULT)
		{
		strcpy (this_param, "v");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Intersect Mode */
	current_setting_num = intersect_menu_selection - 1;
	if (current_setting_num != INTERSECT_DEFAULT)
		{
		strcpy (this_param, "i");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Shading Mode */
	current_setting_num = shading_menu_selection - 1;
	if (current_setting_num != SHADING_DEFAULT)
		{
		strcpy (this_param, "m");
		NumToString (current_setting_num, current_setting_str);
		strcat (this_param, PtoCstr(current_setting_str));
		add_parameter (params, this_param);
		}

	/* do Correct Textures Normal */
	if (GetCtlValue (correct_texture_normal_checkbox))
		add_parameter (params, "z1");

	/* do Intersect Adjust */
	if (GetCtlValue (intersect_adjust_checkbox))
		add_parameter (params, "I1");

	/* do Jittered Sample */
	if (GetCtlValue (use_jittered_sample_checkbox))
		add_parameter (params, "j1");

}	/* generate_params() */



/*****************************************************************************\
* procedure add_parameter                                                     *
*                                                                             *
* Purpose: This procedure adds the passed parameter to the command line       *
*          parameters array.                                                  *
*                                                                             *
* Parameters: params: the existing parameters structure.                      *
*             param:  parameter to add                                        *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void	add_parameter(params_struct *params, char *param)
{

	/* copy this parameter into the parameter storage block */
	strcpy (params->storage + params->current_loc, param);
	
	/* Put a pointer to this parameter in the parameters array */
	params->parameters[params->current_param++] = params->storage + params->current_loc;
	
	/* mark the storage as used */
	params->current_loc += strlen (param) + 1;	

}	/* add_parameter() */



/*****************************************************************************\
* procedure are_valid_params                                                  *
*                                                                             *
* Purpose: This procedure verifies that all the editable text parameters in   *
*          the options dialog are valid.  It returns TRUE if they are.        *
*                                                                             *
* Parameters: returns TRUE parameters are valid.                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
*   Reid Judd    Oct 16, '92    Allow Thresholds (Aliasing, Shading, and      *
*                       		  ambient to range from 0.00001 to 0.99999    *
\*****************************************************************************/

Boolean are_valid_params(void)
{

	/* Check the Image Width */
	if (!is_valid_param_int(options_dialog, IMAGE_WIDTH_NUM, IMAGE_WIDTH_LABEL, 1, INT_MAX))
		return FALSE;

	/* Check the Image Height */
	if (!is_valid_param_int(options_dialog, IMAGE_HEIGHT_NUM, IMAGE_HEIGHT_LABEL, 1, INT_MAX))
		return FALSE;
		
	/* Check the Focal Aperature */
	if (!is_valid_param_real(options_dialog, FOCAL_APERATURE_NUM, FOCAL_APERATURE_LABEL,
								0, DBL_MAX, TRUE, TRUE))
		return FALSE;

	/* Check the Cluster Size */
	if (!is_valid_param_int(options_dialog, CLUSTER_SIZE_NUM, CLUSTER_SIZE_LABEL, 1, INT_MAX))
		return FALSE;
		
	/* Check the Shading Levels */
	if (!is_valid_param_int(options_dialog, SHADING_LEVELS_NUM, SHADING_LEVELS_LABEL, 1, INT_MAX))
		return FALSE;
		
	/* Check the Shading Threshold */
	if (!is_valid_param_real(options_dialog, SHADING_THRESHOLD_NUM, SHADING_THRESHOLD_LABEL,
								0.0000001, 1.0, TRUE, TRUE))
		return FALSE;

	/* Check the Ambient Samples */
	if (!is_valid_param_int(options_dialog, AMBIENT_SAMPLES_NUM, AMBIENT_SAMPLES_LABEL, 1, INT_MAX))
		return FALSE;
	
	/* Check the Ambient Levels */
	if (!is_valid_param_int(options_dialog, AMBIENT_LEVELS_NUM, AMBIENT_LEVELS_LABEL, 0, INT_MAX))
		return FALSE;
	
	/* Check the Ambient Threshold */
	if (!is_valid_param_real(options_dialog, AMBIENT_THRESHOLD_NUM, AMBIENT_THRESHOLD_LABEL,
								0.0, 1.0, TRUE, TRUE))
			return FALSE;

	/* Check the Aliasing Threshold, if any */
	if ((sampling_level_menu_selection != 1)
		&& (!is_valid_param_real(options_dialog, ALIASING_THRESHOLD_NUM, ALIASING_THRESHOLD_LABEL,
								0.0, 1.0, TRUE, TRUE)))
		return FALSE;

	/* Check the Stereo Separation, if any */
	if ((view_menu_selection != 1)
		&& (!is_valid_param_real(options_dialog, STEREO_SEPARATION_NUM,
									STEREO_SEPARATION_LABEL, 0, DBL_MAX, TRUE, TRUE)))
		return FALSE;

	/* Check the Focal Distance, if any */
	if ((GetCtlValue(specify_focal_distance_checkbox))
		&& (!is_valid_param_real(options_dialog, FOCAL_DISTANCE_NUM,
									FOCAL_DISTANCE_LABEL, 0, DBL_MAX, TRUE, TRUE)))
		return FALSE;
	
	/* Everything is okay */
	return TRUE;
	
}	/* are_valid_params() */



/*****************************************************************************\
* procedure is_valid_param_int                                                *
*                                                                             *
* Purpose: This procedure verifies that a specified dialog text item is a     *
*          valid integer, and that it is in the correct range.  If it is      *
*          invalid, or out or range, the user is informed with a dialog, and  *
*          FALSE is returned.  Otherwise, FALSE is returned.                  *
*                                                                             *
* Parameters: dialog: the dialog which contains the item.                     *
*             text_item_id: the DITL id of the text item to check             *
*             label_item_id: the DITL id of the label for this text item      *
*             min:           the minimum allowable value                      *
*             max:           the maximum allowable value                      *
*             returns TRUE if it is valid and in range.                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean is_valid_param_int(DialogPtr dialog, short text_item_id, short label_item_id,
						long min, long max)
{

	short	type;				/* the item type of a DITL item */
	Rect	box;				/* the bounding box of a DITL item */
	Handle	handle;				/* the handle to a DITL item */
	char	number_string[256];	/* the text of the item */
	long	number;				/* the number itself */
	Str255	label;				/* the label for the item */
	short	item;				/* item hit in ModalDialog call */
	char	string[256];		/* temporary variable */

	/* Get the text of the item */
	GetDItem (dialog, text_item_id, &type, &handle, &box);
	GetIText (handle, number_string);

	/* Convert it to a C-style string */
	PtoCstr(number_string);

	/* Get the text of the label */
	GetDItem (dialog, label_item_id, &type, &handle, &box);
	GetIText (handle, label);

	/* Chop off the colon at the end of the label */
	label[0]--;

	/* Verify that the number is a valid integer */
	if (!is_valid_int(number_string))
		{
		
		/* Not a valid integer-- notify user */
		ParamText(label, "\pan integer", 0, 0);
		ShowWindow(invalid_num_dialog);
		SelectWindow(invalid_num_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(invalid_num_dialog);
	
		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Convert it to an integer */
	StringToNum(CtoPstr(number_string), &number);
	
	/* Check it against its upper bound */
	if (number > max)
		{
		
		/* To large-- notify user */
		NumToString(max, string);
		ParamText(label, "\pless than or equal to", string, 0);
		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(param_error_dialog);

		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Check it against its lower bound */
	if (number < min)
		{
		
		/* To small-- notify user */
		NumToString(min, string);
		ParamText(label, "\pgreater than or equal to", string, 0);
		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(param_error_dialog);

		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* There was no problem */
	return TRUE;

}	/* is_valid_param_int() */



/*****************************************************************************\
* procedure is_valid_param_real                                               *
*                                                                             *
* Purpose: This procedure verifies that a specified dialog text item is a     *
*          valid real number, and that it is in the correct range.  If it is  *
*          invalid, or out or range, the user is informed with a dialog, and  *
*          FALSE is returned.  Otherwise, TRUE is returned.                   *
*                                                                             *
* Parameters: dialog: the dialog which contains the item.                     *
*             text_item_id: the DITL id of the text item to check             *
*             label_item_id: the DITL id of the label for this text item      *
*             min:           the minimum allowable value                      *
*             max:           the maximum allowable value                      *
*             include_min:   if this is TRUE, a value of min is acceptable.   *
*             include_max:   if this is TRUE, a value of max is acceptable.   *
*             returns TRUE if it is valid and in range.                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean is_valid_param_real(DialogPtr dialog, short text_item_id, short label_item_id,
							real min, real max, Boolean include_min, Boolean include_max)
{

	short	type;				/* the item type of a DITL item */
	Rect	box;				/* the bounding box of a DITL item */
	Handle	handle;				/* the handle to a DITL item */
	char	number_string[256];	/* the text of the item */
	real	number;				/* the number itself */
	Str255	label;				/* the label for the item */
	short	item;				/* item hit in ModalDialog call */
	char	string[256];		/* temporary variable */

	/* Get the text of the item */
	GetDItem (dialog, text_item_id, &type, &handle, &box);
	GetIText (handle, number_string);

	/* Convert it to a C-style string */
	PtoCstr(number_string);

	/* Get the text of the label */
	GetDItem (dialog, label_item_id, &type, &handle, &box);
	GetIText (handle, label);

	/* Chop off the colon at the end of the label */
	label[0]--;

	/* Verify that the number is a valid real number */
	if (!is_valid_real(number_string))
		{
		
		/* Not a valid real-- notify user */
		ParamText(label, "\pa number", 0, 0);
		ShowWindow(invalid_num_dialog);
		SelectWindow(invalid_num_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(invalid_num_dialog);
	
		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Convert it to an real number */
	number = atof(number_string);
	
	/* Check it against its upper bound */
	if ((number > max) || ((number == max ) && (!include_max)))
		{
		
		/* To large-- notify user */

		/* Convert max to a string */
		sprintf(string, "%lg", max);

		/* Use "less than or equal to" if the max is included; otherwise, just
			use "less than" */
		if (include_max)
			ParamText(label, "\pless than or equal to", CtoPstr(string), 0);
		else
			ParamText(label, "\pless than", CtoPstr(string), 0);

		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(param_error_dialog);

		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Check it against its lower bound */
	if ((number < min) || ((number == min ) && (!include_min)))
		{
		
		/* To small-- notify user */
		
		/* Convert min to a string */
		sprintf(string, "%lg", min);

		/* Use "greater than or equal to" if the min is included; otherwise, just
			use "greater than" */
		if (include_min)
			ParamText(label, "\pgreater than or equal to", CtoPstr(string), 0);
		else
			ParamText(label, "\pgreater than", CtoPstr(string), 0);

		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ProcPtr) NULL, &item);
		HideWindow(param_error_dialog);

		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* There was no problem */
	return TRUE;

}	/* is_valid_param_real() */



/*****************************************************************************\
* procedure is_valid_int                                                      *
*                                                                             *
* Purpose: This procedure returns TRUE if the passed string is a valid        *
*          integer.  If it is not a valid integer, it returns FALSE.          *
*                                                                             *
* Parameters: string: the string to check.                                    *
*             return TRUE if valid.                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean is_valid_int(char *string)
{

	short	i = 0;	/* index */
	
	/* accept unary - at beginning */
	if (string[i] == '-')
		i++;
	
	/* Check that every character is a digit */
	while (string[i])
		if (!isdigit(string[i++]))
			return FALSE;
	
	/* All characters are digits */
	return TRUE;

}	/* is_valid_int() */



/*****************************************************************************\
* procedure is_valid_real                                                     *
*                                                                             *
* Purpose: This procedure returns TRUE if the passed string is a valid        *
*          real number.  If it is not a valid real number, it returns FALSE.  *
*                                                                             *
* Parameters: string: the string to check.                                    *
*             return TRUE if valid.                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 20, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean is_valid_real(char *string)
{

	short	i = 0;					/* index */
	short	predecimal_length = 0;	/* number of digits before decimal point */
	short	postdecimal_length = 0;	/* number of digits after decimal point */
	
	/* accept unary - at beginning */
	if (string[i] == '-')
		i++;
	
	/* there should be a sequence of 0 or more digits next */
	while (isdigit(string[i]))
		{
		predecimal_length++;
		i++;
		}
	
	/* next there could be a decimal point... */
	if (string[i] == '.')
		{
		
		/* Go to next character */
		i++;
		
		/* Read 0 or more digits after the decimal point */
		while (isdigit(string[i]))
			{
			postdecimal_length++;
			i++;
			}
		
		}
	
	/* If the character after the optional - isn't a digit or a .,
		OR if there was just a . with no numbers, it's an error */
	if ((!predecimal_length) && (!postdecimal_length))
		return FALSE;
		
	/* If we get this far, we know we have a number with an optional unary -,
		followed by a valid mantissa */
	
	/* next there could be an exponent */
	if ((string[i] == 'e') || (string[i] == 'E'))
		{
		
		/* Go to next token */
		i++;
		
		/* there could be an unary - before the exponent */
		if (string[i] == '-')
			i++;

		/* Read 1 or more digits after the exponent point */
		if (!isdigit(string[i]))
			return FALSE;	/* error- no digits in exponent */
		do
			i++;
		while (isdigit(string[i]));
		
		}
	
	/* That's all folks-- anything else is illegal */
	if (string[i])
		return FALSE;
	
	/* no error encountered */
	return TRUE;
	
}	/* is_valid_real() */



/*****************************************************************************\
* procedure load_popup_menus                                                  *
*                                                                             *
* Purpose: This procedure loads the popup menus from the resource file and    *
*          installs hooks in the MenuProcs so that they will be wide enough,  *
*          when popped up, to accommodate the down arrow.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void load_popup_menus(void)
{

	antialiasing_menu = GetMenu (ANTIALIASING_MENU);
	install_popup_menu_hook(antialiasing_menu);
	lighting_menu = GetMenu (LIGHTING_MENU);
	install_popup_menu_hook(lighting_menu);
	normal_menu = GetMenu (NORMAL_MENU);
	install_popup_menu_hook(normal_menu);
	intersect_menu = GetMenu (INTERSECT_MENU);
	install_popup_menu_hook(intersect_menu);
	texture_menu = GetMenu (TEXTURE_MENU);
	install_popup_menu_hook(texture_menu);
	shading_menu = GetMenu (SHADING_MENU);
	install_popup_menu_hook(shading_menu);
	view_menu = GetMenu (VIEW_MENU);
	install_popup_menu_hook(view_menu);
	stereo_separation_menu = GetMenu (STEREO_SEPARATION_MENU);
	install_popup_menu_hook(stereo_separation_menu);
	sampling_level_menu = GetMenu (SAMPLING_LEVEL_MENU);
	install_popup_menu_hook(sampling_level_menu);
	save_file_type_menu = GetMenu (SAVE_FILE_TYPE_MENU);
	install_popup_menu_hook(save_file_type_menu);
	save_anim_file_type_menu = GetMenu (SAVE_ANIM_FILE_TYPE_MENU);
	install_popup_menu_hook(save_anim_file_type_menu);

}	/* load_popup_menus() */



/*****************************************************************************\
* procedure setup_check_boxes                                                 *
*                                                                             *
* Purpose: This procedure loads in the check box controls for the dialog      *
*          boxes.  It saves the handles for later manipulation.               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 2, 1992                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void setup_check_boxes(void)
{

	short	type;		/* the item type of a DITL item */
	Rect	box;		/* the bounding box of a DITL item */

	/* Load the check box handles into memory */
	GetDItem (options_dialog, INTERSECT_ADJUST_CHECKBOX, &type,
				&intersect_adjust_checkbox, &box);
	GetDItem (options_dialog, USE_JITTERED_SAMPLE_CHECKBOX, &type,
				&use_jittered_sample_checkbox, &box);
	GetDItem (options_dialog, SPECIFY_FOCAL_DISTANCE_CHECKBOX, &type,
				&specify_focal_distance_checkbox, &box);
	GetDItem (options_dialog, CORRECT_TEXTURE_NORMAL_CHECKBOX, &type,
				&correct_texture_normal_checkbox, &box);
	GetDItem (options_dialog, SPECIFY_FOCAL_DISTANCE_CHECKBOX, &type,
				&specify_focal_distance_checkbox, &box);
	GetDItem (options_dialog, ANIMATE_CHECKBOX, &type,
				&animate_checkbox, &box);
	GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_CHECKBOX, &type,
				&allow_background_tasks_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_IMAGE_RENDERING_CHECKBOX, &type,
				&show_image_rendering_checkbox, &box);
	GetDItem (preferences_dialog, KEEP_IMAGE_IN_MEMORY_CHECKBOX, &type,
				&keep_image_in_memory_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_STATUS_WINDOW_CHECKBOX, &type,
				&show_status_window_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_ABOUT_WINDOW_CHECKBOX, &type,
				&show_about_window_checkbox, &box);
	GetDItem (preferences_dialog, HIDE_OPTIONS_WINDOW_CHECKBOX, &type,
				&hide_options_window_checkbox, &box);

}	/* setup_check_boxes() */



/*****************************************************************************\
* procedure setup_useritems                                                   *
*                                                                             *
* Purpose: This procedure sets up the user items in the dialog boxes.         *
*          These user item are used to implement the popup menus, the         *
*          border around the default button, and the dimming of the "Focal    *
*          distance text.                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void setup_useritems(void)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Set up the border around the default button in the options dialog */
	GetDItem (options_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (options_dialog, BUTTON_BORDER_USERITEM, item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the preferences dialog */
	GetDItem (preferences_dialog, PREF_BUTTON_BORDER_USERITEM,
				&item_type, &item_handle, &item_box);
	SetDItem (preferences_dialog, PREF_BUTTON_BORDER_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the animation dialog */
	GetDItem (animation_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (animation_dialog, BUTTON_BORDER_USERITEM, item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the about dialog */
	GetDItem (about_dialog, ABOUT_BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_BUTTON_BORDER_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Cancel dialog */
	GetDItem (cancel_dialog, CANCEL_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (cancel_dialog, CANCEL_FRAME_BUTTON_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Abort Render dialog */
	GetDItem (abort_render_dialog, ABORT_RENDER_FRAME_BUTTON_USERITEM,
				&item_type, &item_handle, &item_box);
	SetDItem (abort_render_dialog, ABORT_RENDER_FRAME_BUTTON_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Parameter Error dialog */
	GetDItem (param_error_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (param_error_dialog, BUTTON_BORDER_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Prompt to Save dialog */
	GetDItem (prompt_save_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (prompt_save_dialog, BUTTON_BORDER_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the border around the default button in the Invalid Number Error dialog */
	GetDItem (invalid_num_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (invalid_num_dialog, BUTTON_BORDER_USERITEM,
				item_type, draw_button_border, &item_box);

	/* Set up the picture useritems in the About... dialog */
	GetDItem (about_dialog, ABOUT_PICT_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_PICT_USERITEM, item_type, draw_about_picture, &item_box);
	GetDItem (about_dialog, ABOUT_TEXT_PICT_USERITEM, &item_type, &item_handle,
				&about_rtrace_image_bounds);
	SetDItem (about_dialog, ABOUT_TEXT_PICT_USERITEM, item_type, draw_about_text_picture,
				&about_rtrace_image_bounds);
	GetDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM, item_type,
				draw_about_credits_picture, &item_box);
	
	/* Set up the progress bar in the status dialog */
	GetDItem (status_dialog, PROGRESS_BAR_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (status_dialog, PROGRESS_BAR_USERITEM, item_type, draw_progress_bar, &item_box);
	progress_bar_length = item_box.right - item_box.left - 2;

	/* Set up the sub-progress bar in the status dialog */
	GetDItem (status_dialog, SUB_PROGRESS_BAR_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (status_dialog, SUB_PROGRESS_BAR_USERITEM, item_type, draw_sub_progress_bar, &item_box);
	sub_progress_bar_length = item_box.right - item_box.left - 2;

	/* Set up the dimming of the focal distance text */
	GetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, item_type, dim_focal_distance, &item_box);

	/* Set up the dimming of the stereo separation items */
	GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, userItem, dim_stereo_separation, &item_box);

	/* Set up the dimming of the relinquish control items */
	GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &item_type,
				&item_handle, &item_box);
	SetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, userItem,
				dim_relinquish_control, &item_box);

	/* Set up the dimming of the Keep Image In Memory checkbox */
	GetDItem (preferences_dialog, DIM_KEEP_IMAGE_IN_MEMORY_USERITEM, &item_type,
				&item_handle, &item_box);
	SetDItem (preferences_dialog, DIM_KEEP_IMAGE_IN_MEMORY_USERITEM, userItem,
				dim_keep_image_in_memory, &item_box);

	/* Set up the dimming of the Show Image Rendering checkbox */
	GetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, &item_type,
				&item_handle, &item_box);
	SetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, userItem,
				dim_show_image_rendering, &item_box);

	/* Set up the dimming of the aliasing threshold items */
	GetDItem (options_dialog, DIM_ALIASING_THRESHOLD_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (options_dialog, DIM_ALIASING_THRESHOLD_USERITEM, userItem, dim_aliasing_threshold, &item_box);

	/* Set up antialiasing popup menu */
	setup_popup_menu (ANTIALIASING_POPUP, draw_antialiasing_popup);

	/* Set up lighting popup menu */
	setup_popup_menu (LIGHTING_POPUP, draw_lighting_popup);

	/* Set up normal popup menu */
	setup_popup_menu (NORMAL_POPUP, draw_normal_popup);
				
	/* Set up intersect popup menu */
	setup_popup_menu (INTERSECT_POPUP, draw_intersect_popup);
				
	/* Set up texture popup menu */
	setup_popup_menu (TEXTURE_POPUP, draw_texture_popup);
				
	/* Set up view popup menu */
	setup_popup_menu (VIEW_POPUP, draw_view_popup);
				
	/* Set up stereo separation popup menu */
	setup_popup_menu (STEREO_SEPARATION_POPUP, draw_stereo_separation_popup);

	/* Set up sampling level popup menu */
	setup_popup_menu (SAMPLING_LEVEL_POPUP, draw_sampling_level_popup);

	/* Set up shading popup menu */
	setup_popup_menu (SHADING_POPUP, draw_shading_popup);

	/* Get the handle to the Render button */
	GetDItem (options_dialog, RENDER_BUTTON, &item_type, &render_button_handle,
				&item_box);

	/* Get the handle to the Okay button in the animation dialog */
	GetDItem (animation_dialog, OKAY_BUTTON, &item_type, &animation_okay_button_handle,
				&item_box);

	/* Get the handle to the Okay button in the preferences dialog */
	GetDItem (preferences_dialog, OKAY_BUTTON, &item_type, &preferences_okay_button_handle,
				&item_box);

	/* Get the handle to the More button in the about dialog */
	GetDItem (about_dialog, MORE_BUTTON, &item_type, &about_more_button_handle,
				&item_box);

}	/* setup_useritems() */



/*****************************************************************************\
* procedure setup_popup_menu                                                  *
*                                                                             *
* Purpose: This procedure sets up a popup menu in the options dialog box.     *
*                                                                             *
* Parameters: useritem_id: the ID of the useritem which is the bounding box   *
*                          of the popup menu.                                 *
*             proc:        the procedure to draw the useritem (called by the  *
*                          Dialog Manager).                                   *
*             upper_left:  the upper left corner of the popup menu.           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void setup_popup_menu (short useritem_id, UserItemProc *proc)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Set up the popup menu */
	GetDItem (options_dialog, useritem_id, &item_type, &item_handle, &item_box);
	SetDItem (options_dialog, useritem_id, userItem, proc, &item_box);

}	/* setup_popup_menu() */



/*****************************************************************************\
* procedure draw_button_border                                                *
*                                                                             *
* Purpose: This procedure draws the border around the default button in a     *
*          dialog.                                                            *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_button_border (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* draw the outline */
	GetDItem (the_window, OKAY_BUTTON, &item_type, &item_handle,
				&item_box);
	PenSize(3, 3);
	InsetRect (&item_box, -4, -4);
	FrameRoundRect (&item_box, 16, 16);
	PenSize(1, 1);

}	/* draw_button_border() */



/*****************************************************************************\
* procedure draw_about_picture                                                *
*                                                                             *
* Purpose: This procedure draws the picture in the about dialog.              *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 15, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_about_picture (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Find the rectangle */
	GetDItem (the_window, ABOUT_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Draw the picture in the rectangle */
	DrawPicture (about_picture, &item_box);

}	/* draw_about_picture() */



/*****************************************************************************\
* procedure draw_about_text_picture                                           *
*                                                                             *
* Purpose: This procedure draws the text picture in the about dialog.         *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 15, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_about_text_picture (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Find the rectangle */
	GetDItem (the_window, ABOUT_TEXT_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Draw the picture in the rectangle */
	DrawPicture (about_text_picture, &item_box);

}	/* draw_about_text_picture() */



/*****************************************************************************\
* procedure draw_about_credits_picture                                        *
*                                                                             *
* Purpose: This procedure draws the credits picture in the about dialog.      *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 15, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_about_credits_picture (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Find the rectangle */
	GetDItem (the_window, ABOUT_CREDITS_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Draw the picture in the rectangle */
	DrawPicture (about_credits_picture, &item_box);

}	/* draw_about_credits_picture() */



/*****************************************************************************\
* procedure draw_progress_bar                                                 *
*                                                                             *
* Purpose: This procedure draws the progress bar in the status dialog.        *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_progress_bar (WindowPtr the_window, short item_number)
{

	int				type;
	Handle			item;
	Rect			box;
	

	/* Find the new length of the filled part of the bar */
	progress_bar_dividing_line = (progress_bar_length * progress_bar_value)/progress_bar_max;
	
	/* Set the port to draw in the status dialog */
	SetPort (status_dialog);
	
	/* get the outline of the bar */
	GetDItem (status_dialog, PROGRESS_BAR_USERITEM, &type, &item, &box);
	
	/* outline the bar */
	FrameRect (&box);
	
	/* Get the interior of the bar */
	InsetRect (&box, 1, 1);

	/* If Color QuickDraw is available, color the filled part solid dark gray */
	if (f8bit_QD_available)
		RGBForeColor (&bar_gray);
		
	/* Otherwise, use Original QD dithered gray */
	else
		PenPat (gray);

	/* Shade the filled part */
	box.right = box.left + progress_bar_dividing_line;
	PaintRect(&box);
	
	/* If Color QuickDraw is available, color the empty part light blue */
	if (f8bit_QD_available)
		RGBForeColor (&bar_blue);

	/* Otherwise, use Original QD white */
	else
		PenPat (white);

	/* Erase the unfilled part */
	box.right = box.left + progress_bar_length;
	box.left += progress_bar_dividing_line;
	PaintRect (&box);

	/* Restore the foreground color to normal */
	if (f8bit_QD_available)
		RGBForeColor (&bar_black);
	else
		PenPat(&black);

}	/* draw_progress_bar() */



/*****************************************************************************\
* procedure update_progress_bar                                               *
*                                                                             *
* Purpose: This procedure draws the progress bar in the status dialog, but    *
*          only if it has changed since last drawing.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_progress_bar (void)
{

	/* Draw the bar if its appearance has changed */
	if (progress_bar_dividing_line != 
			(progress_bar_length * progress_bar_value)/progress_bar_max)
	
		draw_progress_bar(status_dialog, PROGRESS_BAR_USERITEM);
	
}	/* update_progress_bar() */



/*****************************************************************************\
* procedure draw_sub_progress_bar                                             *
*                                                                             *
* Purpose: This procedure draws the sub-progress bar in the status dialog.    *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_sub_progress_bar (WindowPtr the_window, short item_number)
{

	int				type;
	Handle			item;
	Rect			box;
	
	/* Find the new length of the filled part of the bar */
	sub_progress_bar_dividing_line =
				(sub_progress_bar_length * sub_progress_bar_value)/sub_progress_bar_max;
	
	/* Set the port to draw in the status dialog */
	SetPort (status_dialog);
	
	/* get the outline of the bar */
	GetDItem (status_dialog, SUB_PROGRESS_BAR_USERITEM, &type, &item, &box);
	
	/* outline the bar */
	FrameRect (&box);
	
	/* Get the interior of the bar */
	InsetRect (&box, 1, 1);

	/* If Color QuickDraw is available, color the filled part solid dark gray */
	if (f8bit_QD_available)
		RGBForeColor (&bar_gray);
		
	/* Otherwise, use Original QD dithered gray */
	else
		PenPat (gray);

	/* Shade the filled part */
	box.right = box.left + sub_progress_bar_dividing_line;
	PaintRect(&box);
	
	/* If Color QuickDraw is available, color the empty part light blue */
	if (f8bit_QD_available)
		RGBForeColor (&bar_blue);

	/* Otherwise, use Original QD white */
	else
		PenPat (white);

	/* Erase the unfilled part */
	box.right = box.left + sub_progress_bar_length;
	box.left += sub_progress_bar_dividing_line;
	PaintRect (&box);

	/* Restore the foreground color to normal */
	if (f8bit_QD_available)
		RGBForeColor (&bar_black);
	else
		PenPat(&black);

}	/* draw_sub_progress_bar() */



/*****************************************************************************\
* procedure update_sub_progress_bar                                           *
*                                                                             *
* Purpose: This procedure draws the sub-progress bar in the status dialog,    *
*          but only if it has changed since last drawing.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 4, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_sub_progress_bar (void)
{

	/* Draw the bar if its appearance has changed */
	if (sub_progress_bar_dividing_line != 
			(sub_progress_bar_length * sub_progress_bar_value)/sub_progress_bar_max)
	
		draw_sub_progress_bar(status_dialog, SUB_PROGRESS_BAR_USERITEM);
	
}	/* update_sub_progress_bar() */



/*****************************************************************************\
* procedure update_status_free_memory                                         *
*                                                                             *
* Purpose: This procedure updates the amount of free memory shown in the      *
*          status dialog.                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_status_free_memory (void)
{

	long	free_mem;
	Str255	free_mem_string;

	/* Get the number of K free as a string */
	free_mem = FreeMem();
	free_mem /= 1024;
	NumToString (free_mem, free_mem_string);
	
	/* Add 'K' to the end */
	free_mem_string[0]++;
	free_mem_string[free_mem_string[0]] = 'K';

	/* Change the text of the dialog to reflect the current amount of memory */
	set_dialog_text(status_dialog, FREE_MEMORY_NUM, free_mem_string);

}	/* update_status_free_memory() */



/*****************************************************************************\
* procedure set_status_image_data_size                                        *
*                                                                             *
* Purpose: This procedure updates the image data size text in the status      *
*          dialog.                                                            *
*                                                                             *
* Parameters: size: the new image data size                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_status_image_data_size (long size)
{

	Str255	image_size_string;

	/* Get the number of K free as a string */
	NumToString (size/1024L, image_size_string);
	
	/* Add 'K' to the end */
	image_size_string[0]++;
	image_size_string[image_size_string[0]] = 'K';

	/* Change the text of the dialog to reflect the current amount of memory */
	set_dialog_text(status_dialog, IMAGE_MEMORY_NUM, image_size_string);

}	/* set_status_image_data_size() */



/*****************************************************************************\
* procedure set_status_text                                                   *
*                                                                             *
* Purpose: This procedure sets the status text in the status dialog.          *
*                                                                             *
* Parameters: string: the text to use                                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_status_text(Str255 string)
{

	/* set the text */
	set_dialog_text(status_dialog, STATUS_TEXT, string);

}	/* set_status_text() */



/*****************************************************************************\
* procedure set_status_num_objects                                            *
*                                                                             *
* Purpose: This procedure sets number of objects text in the status dialog.   *
*                                                                             *
* Parameters: num_objects: the new number of objects                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_status_num_objects(long num_objects)
{

	/* set the number of objects text */
	set_dialog_num(status_dialog, NUM_OBJECTS_NUM, num_objects);

}	/* set_status_num_objects() */



/*****************************************************************************\
* procedure set_progress_bar_value                                            *
*                                                                             *
* Purpose: This procedure sets the value of the progress bar in the status    *
*          dialog, and redraws the progress bar.                              *
*                                                                             *
* Parameters: value: the new progress bar value                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_progress_bar_value(long value)
{

	/* Set the bar to the new value */
	progress_bar_value = value;

	/* update the progress bar */
	update_progress_bar();

}	/* set_progress_bar_value() */



/*****************************************************************************\
* procedure set_progress_bar_max                                              *
*                                                                             *
* Purpose: This procedure sets the maximum value of the progress bar in the   *
*          status dialog.  It does not redraw the progress bar.               *
*                                                                             *
* Parameters: value: the new progress bar value                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_progress_bar_max(long value)
{

	/* Set the bar to the new value */
	progress_bar_max = value;

}	/* set_progress_bar_max() */



/*****************************************************************************\
* procedure set_sub_progress_bar_value                                        *
*                                                                             *
* Purpose: This procedure sets the value of the sub-progress bar in the       *
*          status dialog, and redraws the sub-progress bar.                   *
*                                                                             *
* Parameters: value: the new sub-progress bar value                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 4, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_sub_progress_bar_value(long value)
{

	/* Set the bar to the new value */
	sub_progress_bar_value = value;

	/* update the sub-progress bar */
	update_sub_progress_bar();

}	/* set_sub_progress_bar_value() */



/*****************************************************************************\
* procedure set_sub_progress_bar_max                                          *
*                                                                             *
* Purpose: This procedure sets the maximum value of the sub-progress bar in   *
*          the status dialog.  It does not redraw the progress bar.           *
*                                                                             *
* Parameters: value: the new sub-progress bar maximum value                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 4, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_sub_progress_bar_max(long value)
{

	/* Set the bar to the new value */
	sub_progress_bar_max = value;

}	/* set_sub_progress_bar_max() */



/*****************************************************************************\
* procedure set_dialog_num                                                    *
*                                                                             *
* Purpose: This procedure sets the text of a dialog text item to the value    *
*           of the passed number.                                             *
*                                                                             *
* Parameters: dialog: the dialog to change                                    *
*             item:   the item number to change                               *
*             number: the number to change item to                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_num(DialogPtr dialog, short item, long number)
{

	Str255	number_string;
	
	/* Convert number to a string */
	NumToString(number, number_string);

	/* Change the dialog text */
	set_dialog_text(dialog, item, number_string);

}	/* set_dialog_num() */



/*****************************************************************************\
* procedure set_dialog_real                                                   *
*                                                                             *
* Purpose: This procedure sets the text of a dialog text item to the value    *
*           of the passed real number.                                        *
*                                                                             *
* Parameters: dialog: the dialog to change                                    *
*             item:   the item number to change                               *
*             number: the real number to change item to                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_real(DialogPtr dialog, short item, real number)
{

	Str255	number_string;
	
	/* Convert number to a string */
	sprintf((char *)number_string, "%lg", number);

	/* Change the dialog text */
	set_dialog_text(dialog, item, CtoPstr(number_string));

}	/* set_dialog_real() */



/*****************************************************************************\
* procedure set_dialog_text                                                   *
*                                                                             *
* Purpose: This procedure sets the text of a dialog text item to the passed   *
*           string.                                                           *
*                                                                             *
* Parameters: dialog: the dialog to change                                    *
*             item:   the item number to change                               *
*             string: the string to change item to                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_text(DialogPtr dialog, short item, Str255 string)
{

	int		type;
	Handle	handle;
	Rect	box;

	/* Change the dialog text */
	GetDItem (dialog, item, &type, &handle, &box);
	SetIText (handle, string);

}	/* set_dialog_text() */



/*****************************************************************************\
* procedure set_dialog_pict                                                   *
*                                                                             *
* Purpose: This procedure sets a picture in a dialog box to be the specified  *
*          PICT resource.                                                     *
*                                                                             *
* Parameters: dialog: the dialog to change                                    *
*             item:   the item number to change                               *
*             string: the string to change item to                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_pict(DialogPtr dialog, short item, short pic_id)
{

	int			type;
	Handle		handle;
	PicHandle	pic_handle;
	Rect		box;

	/* Read in the picture */
	pic_handle = GetPicture (pic_id);

	/* Set the DITL item to be the picture */
	GetDItem (dialog, item, &type, &handle, &box);
	SetDItem (dialog, item, type, pic_handle, &box);

}	/* set_dialog_pict() */



/*****************************************************************************\
* procedure dim_focal_distance                                                *
*                                                                             *
* Purpose: This procedure is used to dim the focal distance text and edittext *
*          when appropriate.  The useritem is located directly below the two  *
*          text items.  When the "Specify Focal Distance" check box is        *
*          checked, the text items are selected.  When it is not checked, the *
*          text items are dimmed.                                             *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_focal_distance (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */


	/* Get the value of the Specify Focal Distance check box; if it is checked,
		enable the item.  If it is not checked, disable it and grey it out. */
	if (GetCtlValue (specify_focal_distance_checkbox))	
		{
		
		/* Enable the EditText item */
		GetDItem (options_dialog, FOCAL_DISTANCE_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, FOCAL_DISTANCE_NUM,
					editText, item_handle, &item_box);
		
		/* Draw the frame */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		
		}
	else		
		{
	
		/* Get the useritem rectangle */
		GetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, &item_type,
					&item_handle, &item_box);
					
		/* extend it to the right to encompass the text */
		item_box.right += 200;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenMode(patCopy);
		
		/* Disable the EditText item */
		GetDItem (options_dialog, FOCAL_DISTANCE_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, FOCAL_DISTANCE_NUM,
					(statText | itemDisable), item_handle, &item_box);
		
		/* Draw the frame in gray */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		PenPat(black);
		
		/* Select something else */
		SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);

		}

}	/* dim_focal_distance() */





/*****************************************************************************\
* procedure dim_aliasing_threshold                                            *
*                                                                             *
* Purpose: This procedure is used to dim the aliasing threshold text and      *
*          edittext when appropriate.  The useritem is located directly to    *
*          the right of the two text items.  When the "Sampling Level" menu   *
*          is set to 2 or 3, the text items are selected.  When it set to 1,  *
*          the text items are dimmed.                                         *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_aliasing_threshold (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */


	/* Get the value of the Sampling Levels menu; if there is sampling,
		enable the item.  Otherwise, disable it and grey it out. */
	if (sampling_level_menu_selection != 1)	
		{
		
		/* Enable the EditText item */
		GetDItem (options_dialog, ALIASING_THRESHOLD_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, ALIASING_THRESHOLD_NUM,
					editText, item_handle, &item_box);
		
		/* Draw the frame */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		
		}
	else		
		{
	
		/* Get the useritem rectangle */
		GetDItem (options_dialog, DIM_ALIASING_THRESHOLD_USERITEM, &item_type,
					&item_handle, &item_box);
					
		/* extend it to the left to encompass the text */
		item_box.left -= 200;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenMode(patCopy);
		
		/* Disable the EditText item */
		GetDItem (options_dialog, ALIASING_THRESHOLD_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, ALIASING_THRESHOLD_NUM,
					(statText | itemDisable), item_handle, &item_box);
		
		/* Draw the frame in gray */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		PenPat(black);
		
		/* Select something else */
		SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);

		}

}	/* dim_aliasing_threshold() */




/*****************************************************************************\
* procedure dim_relinquish_control                                            *
*                                                                             *
* Purpose: This procedure is used to dim the relinquish control text and      *
*          edittext.  The useritem is located directly to the right of the    *
*          text items.  When the "Allow Background Tasks" check box is        *
*          checked, the text items are selected.  When it is not checked, the *
*          text items are dimmed.                                             *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_relinquish_control (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */


	/* Get the value of the Allow Background Tasks check box; if it is checked,
		enable the item.  If it is not checked, disable it and grey it out. */
	if (GetCtlValue (allow_background_tasks_checkbox))	
		{
		
		/* Enable the EditText item */
		GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
					editText, item_handle, &item_box);
		
		/* Draw the frame */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		
		/* Select the editText */
		SelIText (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, 0, 32767);

		}
	else		
		{
	
		/* Select something else */
		SelIText (preferences_dialog, DUMMY_EDITTEXT, 0, 32767);

		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &item_type,
					&item_handle, &item_box);
					
		/* extend it to the left to encompass the text */
		item_box.left -= 300;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenMode(patCopy);
		
		/* Disable the EditText item */
		GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
					(statText | itemDisable), item_handle, &item_box);
		
		/* Draw the frame in gray */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		PenPat(black);

		}

}	/* dim_relinquish_control() */



/*****************************************************************************\
* procedure dim_show_image_rendering                                          *
*                                                                             *
* Purpose: This procedure is used to dim the Show Image Rendering option when *
*          Color QuickDraw is not available.                                  *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_show_image_rendering (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */


	/* If Color QuickDraw is not available, dim the Show Image Rendering checkbox */
	if (!f8bit_QD_available)
		{
				
		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, &item_type,
					&item_handle, &item_box);

		/* extend it to the left to encompass the checkbox */
		item_box.left -= 300;

		/* Set the pen mode to Bic, draw over the check boxes in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenPat(black);
		PenMode(patCopy);
		
		/* Disable the checkbox */
		HiliteControl(show_image_rendering_checkbox, 255);
		SetCtlValue(show_image_rendering_checkbox, FALSE);
		
		}

}	/* dim_show_image_rendering() */



/*****************************************************************************\
* procedure dim_keep_image_in_memory                                          *
*                                                                             *
* Purpose: This procedure is used to dim the Keep Image In Memory option when *
*          32-bit QuickDraw is not available.                                 *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_keep_image_in_memory (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */


	/* If 32-bit QuickDraw is not available, dim the Keep Image In Memory checkbox */
	if (!f32bit_QD_available)
		{
				
		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_KEEP_IMAGE_IN_MEMORY_USERITEM, &item_type,
					&item_handle, &item_box);

		/* extend it to the left to encompass the checkbox */
		item_box.left -= 300;

		/* Set the pen mode to Bic, draw over the check boxes in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenPat(black);
		PenMode(patCopy);
		
		/* Disable the checkbox */
		HiliteControl(keep_image_in_memory_checkbox, 255);
		SetCtlValue(keep_image_in_memory_checkbox, FALSE);
		
		}

}	/* dim_keep_image_in_memory() */



/*****************************************************************************\
* procedure dim_stereo_separation                                             *
*                                                                             *
* Purpose: This procedure is used to dim the stereo separation items          *
*          when appropriate.  The useritem is located directly to the right   *
*          of the items.  When the View mode is "right eye" or "left eye,"    *
*          the text items are selected.  When it is "normal," the items are   *
*          dimmed.                                                            *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void dim_stereo_separation (WindowPtr the_window, short item_number)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */
	Rect	edittext_box;
	char	num_text[100];	/* temporary text storage */


	/* If the view mode is normal, disable the stereo separation items.
		Otherwise, enable them. */
	if (view_menu_selection != 1)	
		{
		
		/* Enable the EditText item */
		GetDItem (options_dialog, STEREO_SEPARATION_NUM, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, STEREO_SEPARATION_NUM,
					editText, item_handle, &item_box);
		
		/* If the text is 0.0 or 0, set it to 1.0 (it's not legal to have no
			separation in stereoscopic mode) */
		GetIText (item_handle, num_text);
		if ( (!strcmp(PtoCstr(num_text), "0")) || (!strcmp(PtoCstr(num_text), "0.0")) )
			SetIText (item_handle, "\p1.0");
	
		/* Draw the frame */
		InsetRect (&item_box, -3, -3);
		FrameRect (&item_box);
		
		/* Enable the stereo separation menu */
		GetDItem (options_dialog, STEREO_SEPARATION_POPUP, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, STEREO_SEPARATION_POPUP,
					userItem, item_handle, &item_box);
		
		}
	else		
		{
	
		/* Disable the EditText item */
		GetDItem (options_dialog, STEREO_SEPARATION_NUM, &item_type,
					&item_handle, &edittext_box);
		SetDItem (options_dialog, STEREO_SEPARATION_NUM,
					(statText | itemDisable), item_handle, &edittext_box);
		
		/* If the text isn't 0.0 or 0, set it to 0 (it's not legal to have
			stereo separation in monoscopic mode) */
		GetIText (item_handle, num_text);
		if ( (strcmp(PtoCstr(num_text), "0")) && (strcmp(PtoCstr(num_text), "0.0")) )
			SetIText (item_handle, "\p0.0");
	
		/* Disable the stereo separation menu */
		GetDItem (options_dialog, STEREO_SEPARATION_POPUP, &item_type,
					&item_handle, &item_box);
		SetDItem (options_dialog, STEREO_SEPARATION_POPUP,
					userItem + itemDisable, item_handle, &item_box);
		
		/* Get the useritem rectangle */
		GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &item_type,
					&item_handle, &item_box);
					
		/* extend it to the right to encompass the text */
		item_box.right += 350;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(gray);
		PaintRect (&item_box);
		PenMode(patCopy);
		
		/* Draw the frame in gray */
		InsetRect (&edittext_box, -3, -3);
		FrameRect (&edittext_box);
		PenPat(black);
		
		}

}	/* dim_stereo_separation() */



/*****************************************************************************\
* procedure draw_popup_menu                                                   *
*                                                                             *
* Purpose: This procedure draws a popup menu in the_box, with the_string as   *
*          the current menu text.  It also draws the down arrow at the right. *
*          If enabled is false, it is drawn grayed out.                       *
*                                                                             *
* Parameters: the_box:    the bounding box of the popup menu                  *
*             the_string: the text of the current selection (pascal string)   *
*             enabled:    TRUE if this menu is enabled, FALSE if grayed.      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void draw_popup (Rect *the_box, Str255 the_string, short enabled)
{
	Rect down_arrow_rect = {0, 0, 6, 12};
	Point pen_pos;
	Pattern *draw_pat = (enabled) ? &black : &gray;
	
	/* make room for the outline */
	
	the_box->right--;
	the_box->bottom--;
	
	/* Outline rectangle and add drop box */
	PenPat (draw_pat);
	OffsetRect (the_box, 1, 1);
	FrameRect (the_box);
	OffsetRect (the_box, -1, -1);
	PenPat (white);
	PaintRect (the_box);
	PenPat (draw_pat);
	FrameRect (the_box);

	/* Draw the text in the rectangle */
	MoveTo (the_box->left+15, the_box->bottom-6);
	DrawString (the_string);

	/* Draw the down arrow at the right of the box */
	OffsetRect (&down_arrow_rect, the_box->right-17, the_box->bottom-13);
	if (enabled)
		DrawPicture(down_arrow_picture, &down_arrow_rect);
	else
		DrawPicture(down_arrow_picture_grayed, &down_arrow_rect);

	PenPat (black);
	
}	/* draw_popup() */



/*****************************************************************************\
* procedure install_popup_menu_hook                                           *
*                                                                             *
* Purpose: This procedure installs a hook into the menu defproc of the popup  *
*          menu in order to force the width of the menu to allow for the      *
*          standard down arrow within the popup.                              *
*                                                                             *
* Parameters: popup_menu:      the menu itself                                *
*             menu_id:         the resource ID of the menu                    *
*             popup_text_id:   the ID of the text item in the options dialog  *
*                              which should be inverted while the menu is     *
*                              popped up.  (0 if nothing should be inverted)  *
*             menu_upper_left: the upper left corner of the popup box, in     *
*                              global coordinates.                            *
*             menu_selection:  the current selection in the menu              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

	typedef struct tMenuHook	/* The structure of the menu hook routine */
	{
		short	jmp;
		long	addr;
		Handle	realMenuProc;
		
	} tMenuHook, **tMenuHookHndl;
	
void install_popup_menu_hook(MenuHandle popup_menu)
{

	tMenuHookHndl	theHook = (tMenuHookHndl) NewHandle ( sizeof( tMenuHook));
	
	/* install a hook which just jumps to MenuProc */
	(**theHook).jmp = 0x4EF9;		/* JMP instruction */
	(**theHook).addr = (long) MenuProc;
	(**theHook).realMenuProc = (**popup_menu).menuProc;	
	
	(**popup_menu).menuProc = (Handle) theHook;
	
	FlushCache();	/* After modifying code, we must flush the cache on 68040s */

}	/* install_popup_menu_hook() */


/******************************************************************************
 FlushCache		TCL 1.1.1 DLP 9/27/91
 
 	Flush the CPU cache(s). This is required on the 68040 after modifying
 	code. 
 ******************************************************************************/

#define _CacheFlushTrap		0xA0BD

void FlushCache(void)
{
	if (TrapAvailable( _CacheFlushTrap))
		asm
		{
			dc.w _CacheFlushTrap
		}
}


/******************************************************************************
 TrapAvailable
 
 	Check whether a certain trap exists on this machine. This function uses
 	the new method as per IM VI, p. 3-8.
 	
 ******************************************************************************/

#define _InitGraf						0xA86E
#define _Unimplemented					0xA89F

Boolean TrapAvailable( short theTrap)
{
	TrapType tType;
	short    numToolBoxTraps;
	
				// first determine the trap type
				
	tType = (theTrap & 0x800) > 0 ? ToolTrap : OSTrap;
	
				// next find out how many traps there are
				
	if (NGetTrapAddress( _InitGraf, ToolTrap) == NGetTrapAddress( 0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;
	
				// check if the trap number is too big for the
				// current trap table
				
	if (tType == ToolTrap)
	{
		theTrap &= 0x7FF;
		if (theTrap >= numToolBoxTraps)
			theTrap = _Unimplemented;
	}
	
				// the trap is implemented if its address is
				// different from the unimplemented trap
				
	return (NGetTrapAddress( theTrap, tType) != 
			NGetTrapAddress(_Unimplemented, ToolTrap));
}


/* This is the custom menu defproc that is called by the Menu Manager	*/
/* All it does is call the original defproc and then check if the		*/
/* message was mSizeMsg. If it was, it patches the menuWidth to allow	*/
/* room for the down arrow												*/

static pascal void MenuProc( short message, MenuHandle menu, Rect *menuRect,
					Point hitPt, short *whichItem)
{
	tMenuHookHndl	menuHook;
	Handle 			realMenuProc;
	SignedByte		state;
	
	/* get handle to the real menu proc */					
	menuHook = (tMenuHookHndl) (**menu).menuProc;
	realMenuProc = (**menuHook).realMenuProc;

	/* Load the real menu proc into memory, and call it */
	LoadResource( realMenuProc);
	state = HGetState( realMenuProc);
	HLock( realMenuProc);
	CallPascal( message, menu, menuRect, hitPt, whichItem, *realMenuProc);
	HSetState( realMenuProc, state);

	/* If it is a mSizeMsg, change the size */	
	if (message == mSizeMsg)
		(**menu).menuWidth += 14;

}	/* MenuProc() */



/*****************************************************************************\
* procedure handle_checkbox_click                                             *
*                                                                             *
* Purpose: This procedure handles a click in a check box in the a             *
*          dialog box.  It toggles the value of the check box.  If it is the  *
*          "Specify Focal Distance" check box, it also updates.               *
*                                                                             *
* Parameters: check_box:      the check box itself                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_checkbox_click(ControlHandle check_box)
{

	short	type;			/* the item type of a DITL item */
	Handle	handle;			/* a handle to a DITL item */
	Rect	rect;			/* the bounding box of a DITL item */
	short	current_value;	/* the current value of the checkbox */

	/* Get current value of the checkbox */
	current_value = GetCtlValue (check_box);

	/* Set the value to the opposite */
	SetCtlValue (check_box, !current_value);

	/* If this is the "Animate" checkbox, show or hide the Animation dialog */
	if (check_box == animate_checkbox)
		{
		
		/* Set the Save... text appropriately */
		SetItem(file_menu, SAVE_ITEM, (current_value) ? "\pSave Image�" : "\pSave Animation�");
		
		/* If the user clicked it on, show the Animation dialog.  Otherwise,
			hide the dialog. */
		if (!current_value)
			show_animation_dialog(TRUE);
		else
			hide_animation_dialog();
		}

	/* If this is the "Specify Focal Distance" check box, set things up so that
		the "Focal Distance" text and the dimming useritem get refreshed. */
	else if (check_box == specify_focal_distance_checkbox)
		{
		
		/* Select something else */
		SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);
		
		/* Get the rectangle of the useritem (just beneath the text in the DITL */
		GetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, &type,
					&handle, &rect); 
					
		/* extend the rectangle to the right so it encloses the text */
		rect.right += 200;
		
		/* make the whole area invalid to it gets updated later */
		SetPort (options_dialog);
		InvalRect (&rect);
		}
		
	/* If this is the "Allow Background Tasks" check box, set things up so that
		the relinquish control text and the dimming useritem get refreshed. */
	else if (check_box == allow_background_tasks_checkbox)
		{
		
		/* Get the rectangle of the useritem (just to the right of the text
			in the DITL) */
		GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &type,
					&handle, &rect); 
					
		/* extend the rectangle to the right so it encloses the text */
		rect.left -= 300;
		
		/* make the whole area invalid to it gets updated later */
		SetPort (preferences_dialog);
		InvalRect (&rect);
		}
		
}	/* handle_checkbox_click() */



/*****************************************************************************\
* procedure handle_popup_click                                                *
*                                                                             *
* Purpose: This procedure handles a click in a popup menu in the any          *
*          dialog box.  It pops up the menu and saves the new selection.      *
*                                                                             *
* Parameters: popup_menu:      the menu itself                                *
*             menu_id:         the resource ID of the menu                    *
*             popup_text_id:   the ID of the text item in the options dialog  *
*                              which should be inverted while the menu is     *
*                              popped up.  (0 if nothing should be inverted)  *
*             menu_upper_left: the upper left corner of the popup box, in     *
*                              global coordinates.                            *
*             menu_selection:  the current selection in the menu              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_popup_click(DialogPtr the_dialog, MenuHandle popup_menu, short menu_id,
						short popup_text_id, short popup_useritem_id,
						Point menu_upper_left, short *menu_selection)
{
	Rect		item_rect, invert_rect;
	Handle		item_handle;
	int 		item_type;
	long		result;
	short		new_selection;
	
	/* Make sure we draw in the dialog box */
	SetPort (the_dialog);

	/* Invert the popup text, if any */
	if (popup_text_id)
		{
		/* Find the rectangle of the text item to invert */
		GetDItem (the_dialog, popup_text_id, &item_type, &item_handle,
					&invert_rect); 
		InvertRect (&invert_rect);
		}

	/* Pop up the menu */
	InsertMenu (popup_menu, -1);
	result = PopUpMenuSelect (popup_menu, menu_upper_left.v, menu_upper_left.h,
								*menu_selection);
	DeleteMenu (menu_id);

	/* find the new selection, if any */
	new_selection = LoWord (result);

	/* If there was a selection, and it is different from the old one, update
		the menu. */
	if (result && (new_selection != *menu_selection))
		{
	
		/* Uncheck the old selection, and check the new */
		CheckItem (popup_menu, *menu_selection, FALSE);
		CheckItem (popup_menu, new_selection, TRUE);
	
		/* If this is the View popup, and there was a switch from monoscopic to
			stereoscopic or back, refresh the Stereo separation items. */
		if ((menu_id == VIEW_MENU) && ((*menu_selection == 1) || (new_selection == 1)))
			{
			
			/* Get the rectangle of the useritem (just to the right of the items */
			GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &item_type,
						&item_handle, &item_rect); 
						
			/* extend the rectangle rightward so it encloses the text */
			item_rect.right += 350;
			
			/* make the whole area invalid to it gets updated later */
			SetPort (options_dialog);
			InvalRect (&item_rect);
			
			}

		/* If this is the Sampling Level popup, and there was a switch Level 0
			to level 1, 2 or 3, or from 1, 2 or 3 to 0, refresh the Aliasing
			Threshold items. */
		if ((menu_id == SAMPLING_LEVEL_MENU) &&
							((*menu_selection == 1) || (new_selection == 1)))
			{
			
			/* Get the rectangle of the useritem (just to the right of the items */
			GetDItem (options_dialog, DIM_ALIASING_THRESHOLD_USERITEM, &item_type,
						&item_handle, &item_rect); 
						
			/* extend the rectangle leftward so it encloses the text */
			item_rect.left -= 200;
			
			/* make the whole area invalid to it gets updated later */
			SetPort (options_dialog);
			InvalRect (&item_rect);
			
			}
			
		/* If this is the Textures popup, set the texture_mode_changed flag,
			and update the Render Button */
		if (menu_id == TEXTURE_MENU)
			{
			
			/* set the texture_mode_changed depending on whether the selected
				texture mode is different from last render */
			texture_mode_changed = (new_selection != (last_texture_selection + 1));
			
			/* Update the "Render" button */
			update_render_button();

			}

		/* Save the new selection */
		*menu_selection = new_selection;
	
		/* Update the popup menu */
			draw_any_popup (the_dialog, popup_useritem_id, popup_menu,
							*menu_selection, &menu_upper_left);		
		}	
	
	/* Invert the popup text, if any */
	if (popup_text_id)
		InvertRect (&invert_rect);


}	/* handle_popup_click() */



/*****************************************************************************\
* procedure update_render_button                                              *
*                                                                             *
* Purpose: This procedure updates the title of the Render button.  The button *
*          is called Render Again if the scene from last render is still in   *
*          memory, and the same file is being rendered, and the texture mode  *
*          has not changed.                                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 1, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_render_button(void)
{

	if ((scene_in_memory) && (rendering_same_file) && (!texture_mode_changed))
		SetCTitle(render_button_handle, "\pRender Again");
	
	else
		SetCTitle(render_button_handle, "\pRender");

}	/* update_render_button() */



/*****************************************************************************\
* procedure draw_any_popup                                                    *
*                                                                             *
* Purpose: This procedure draws any of the popup menus in a dialog box.       *
*                                                                             *
* Parameters: item_number:    the ID of the user item                         *
*             menu:           the menu                                        *
*             menu_selection: the current selection in the menu               *
*             upper_left:     the upper left corner of the popup box          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void draw_any_popup (DialogPtr the_dialog, short item_number, MenuHandle menu,
							short menu_selection, Point *upper_left)
{
	Rect		item_rect;
	Handle		item_handle;
	int 		item_type;
	Str255		the_string;

	SetPort(the_dialog);

	/* Find the surrounding rectangle */
	GetDItem (the_dialog, item_number, &item_type, &item_handle,
				&item_rect); 

	/* the currently selected item in the popup */
	GetItem(menu, menu_selection, the_string);
	
	/* Draw the popup */
	draw_popup (&item_rect, the_string, TRUE);

	/* Find the upper left corner so we can pop it up later */
	upper_left->h = item_rect.left + 1;
	upper_left->v = item_rect.top + 1;
	LocalToGlobal (upper_left);

}	/* draw_any_popup() */



/*****************************************************************************\
* procedure draw_antialiasing_popup                                           *
*                                                                             *
* Purpose: This procedure draws the antialiasing popup menu in the options    *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_antialiasing_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, antialiasing_menu,
					antialiasing_menu_selection, &antialiasing_menu_upper_left);

}	/* draw_antialiasing_popup() */



/*****************************************************************************\
* procedure draw_lighting_popup                                               *
*                                                                             *
* Purpose: This procedure draws the lighting popup menu in the options        *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_lighting_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, lighting_menu,
					lighting_menu_selection, &lighting_menu_upper_left);

}	/* draw_lighting_popup() */



/*****************************************************************************\
* procedure draw_normal_popup                                                 *
*                                                                             *
* Purpose: This procedure draws the normal popup menu in the options          *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_normal_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, normal_menu,
					normal_menu_selection, &normal_menu_upper_left);

}	/* draw_normal_popup() */



/*****************************************************************************\
* procedure draw_intersect_popup                                              *
*                                                                             *
* Purpose: This procedure draws the intersect popup menu in the options       *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_intersect_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, intersect_menu,
					intersect_menu_selection, &intersect_menu_upper_left);

}	/* draw_intersect_popup() */



/*****************************************************************************\
* procedure draw_texture_popup                                                *
*                                                                             *
* Purpose: This procedure draws the texture popup menu in the options         *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_texture_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, texture_menu,
					texture_menu_selection, &texture_menu_upper_left);

}	/* draw_texture_popup() */



/*****************************************************************************\
* procedure draw_shading_popup                                                *
*                                                                             *
* Purpose: This procedure draws the shading popup menu in the options         *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_shading_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, shading_menu,
					shading_menu_selection, &shading_menu_upper_left);

}	/* draw_shading_popup() */



/*****************************************************************************\
* procedure draw_view_popup                                                   *
*                                                                             *
* Purpose: This procedure draws the view popup menu in the options            *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_view_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, view_menu, view_menu_selection,
					&view_menu_upper_left);

}	/* draw_view_popup() */



/*****************************************************************************\
* procedure draw_stereo_separation_popup                                      *
*                                                                             *
* Purpose: This procedure draws the stereo separation popup menu in the       *
*          options dialog box.                                                *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_stereo_separation_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, stereo_separation_menu,
					stereo_separation_menu_selection,
					&stereo_separation_menu_upper_left);

}	/* draw_stereo_separation_popup() */



/*****************************************************************************\
* procedure draw_sampling_level_popup                                         *
*                                                                             *
* Purpose: This procedure draws the sampling level popup menu in the          *
*          options dialog box.                                                *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_sampling_level_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, sampling_level_menu,
					sampling_level_menu_selection,
					&sampling_level_menu_upper_left);

}	/* draw_sampling_level_popup() */



/*****************************************************************************\
* procedure show_about_dialog                                                 *
*                                                                             *
* Purpose: This procedure displays the About... dialog.                       *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void show_about_dialog()
{

	short	item;
	Boolean	credits_showing = FALSE;
	GrafPtr	temp_port;

	/* Show the About... dialog */	
	ShowWindow(about_dialog);
	SelectWindow(about_dialog);
	GetPort(&temp_port);
	SetPort(about_dialog);
		
	/* Wait for user to click Okay! */
	do 
		{
	
		/* Wait for a click in a button */	
		ModalDialog (0, &item);

		/* If user clicked on the picture, and QuickTime is available, run RTrace movie */
		if ((item == ABOUT_TEXT_PICT_USERITEM) && (quicktime_available))
			run_rtrace_movie();

		/* Check for click in the More or Back button */
		if (item == MORE_BUTTON)
			{
			
			/* Is it More or Back? */
			if (credits_showing)
		
				/* It's Back */		
				{

				/* Hide the credits and show the About pictures */
				HideDItem(about_dialog, ABOUT_CREDITS_PICT_USERITEM);
				ShowDItem(about_dialog, ABOUT_PICT_USERITEM);
				ShowDItem(about_dialog, ABOUT_TEXT_PICT_USERITEM);
				
				/* Change the button from Back to More */
				SetCTitle(about_more_button_handle, "\pMore");
				
				/* Remember that the credits aren't showing anymore */
				credits_showing = FALSE;
				
				}
			
			else	/* It's More */
				{

				/* Hide the About pictures and show the credits */
				HideDItem(about_dialog, ABOUT_PICT_USERITEM);
				HideDItem(about_dialog, ABOUT_TEXT_PICT_USERITEM);
				ShowDItem(about_dialog, ABOUT_CREDITS_PICT_USERITEM);
				
				/* Change the button from More to Back */
				SetCTitle(about_more_button_handle, "\pBack");
				
				/* Remember that the credits are showing */
				credits_showing = TRUE;
				
				}
			}
		}
	while (item != OKAY_BUTTON);
	
	/* If they exited from the Credits view, change things back */
	if (credits_showing)
		{
		HideDItem(about_dialog, ABOUT_CREDITS_PICT_USERITEM);
		ShowDItem(about_dialog, ABOUT_PICT_USERITEM);
		ShowDItem(about_dialog, ABOUT_TEXT_PICT_USERITEM);
		SetCTitle(about_more_button_handle, "\pMore");
		}
	
	/* Hide the dialog */
	HideWindow (about_dialog);

	/* Restore the GrafPort */
	SetPort(temp_port);


}	/* show_about_dialog() */



/*****************************************************************************\
* procedure run_rtrace_movie                                                  *
*                                                                             *
* Purpose: This runs the RTrace movie when the user clicks on the text.       *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void run_rtrace_movie(void)
{

	FSSpec	fsspec;
	Movie	movie;
	Rect	movie_bounds;
	long	proc_id;
	short	error;
	short	resource_refnum;

	/* Create the FSSpec for the movie file */
	error = GetWDInfo (temp_folder_wd_id, &(fsspec.vRefNum), &(fsspec.parID), &proc_id);
	if (error) return;
	strcpy ((char *) fsspec.name, (char *) "~~rtrace_movie");
	CtoPstr ((char *) fsspec.name);

	/* Open the movie file */
	error = OpenMovieFile(&fsspec, &resource_refnum, 0);
	if (error) return;

	error = NewMovieFromFile( &movie, resource_refnum, NULL, NULL, 0, NULL);
	if (error) return;

	/* Move the movie to the upper left corner of the window */
	GetMovieBox( movie, &movie_bounds);
	OffsetRect(&movie_bounds, about_rtrace_image_bounds.left - movie_bounds.left + 8,
								about_rtrace_image_bounds.top - movie_bounds.top - 16);
	SetMovieBox(movie, &movie_bounds);

	/* Set up the window for the movie to play in */
	SetMovieGWorld(movie, NULL, NULL);

	/* Play the movie */
	GoToBeginningOfMovie(movie);
	PrerollMovie(movie, 0, 0);
	SetMovieActive(movie, TRUE);
	StartMovie(movie);

	/* Keep playing until we're done */
	while (!IsMovieDone(movie))
		MoviesTask(movie, 0);

	/* Release memory used by movie */
	DisposeMovie(movie);

	/* Close the movie file */
	CloseMovieFile(resource_refnum);

	/* Redraw the PICT version of the image */
	draw_about_text_picture (about_dialog, 0);

}	/* run_rtrace_movie() */



/*****************************************************************************\
* procedure show_preferences_dialog                                           *
*                                                                             *
* Purpose: This procedure displays the preferences dialog, and sets the       *
*          dialog items according to the preferences.                         *
*          change the preferences.                                            *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 25, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/
void show_preferences_dialog(void)
{

	short	type;
	Handle	handle;
	Rect	box;
	Str255	string;

	/* set the checkboxes according the preferences */
	SetCtlValue (show_image_rendering_checkbox, show_image_rendering);
	SetCtlValue (allow_background_tasks_checkbox, allow_background_tasks);
	SetCtlValue (keep_image_in_memory_checkbox, keep_image_in_memory);
	SetCtlValue (show_status_window_checkbox, show_status_window_flag);
	SetCtlValue (hide_options_window_checkbox, hide_options_window_flag);
	SetCtlValue (show_about_window_checkbox, show_about_window_flag);
	
	/* Set the relinquish control text according to the preferences */
	NumToString (time_between_events, string);
	GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &type, &handle, &box);
	SetIText (handle, string);
	
	/* If events are disabled, select the dummy edittext item */
	if (!allow_background_tasks)
		SelIText (preferences_dialog, DUMMY_EDITTEXT, 0, 32767);

	/* Bring the preferences window to the front */
	ShowWindow (preferences_dialog);
	SelectWindow (preferences_dialog);

}	/* show_preferences_dialog() */



/*****************************************************************************\
* procedure handle_preferences_selection                                      *
*                                                                             *
* Purpose: This procedure handles a click in an item in the preferences       *
*          dialog.                                                            *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 25, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/
void handle_preferences_selection(short item_hit)
{

	Boolean	new_show_image_rendering;
	Boolean	new_keep_image_in_memory;
	short	type;
	Handle	handle;
	Rect	box;
	Str255	string;		/* temporary string */

	switch (item_hit)
		{
		
			case ALLOW_BACKGROUND_TASKS_CHECKBOX:
			handle_checkbox_click (allow_background_tasks_checkbox);
			break;			
		
			case SHOW_IMAGE_RENDERING_CHECKBOX:
			handle_checkbox_click (show_image_rendering_checkbox);
			break;			
		
			case KEEP_IMAGE_IN_MEMORY_CHECKBOX:
			handle_checkbox_click (keep_image_in_memory_checkbox);
			break;			
		
			case SHOW_STATUS_WINDOW_CHECKBOX:
			handle_checkbox_click (show_status_window_checkbox);
			break;			
		
			case HIDE_OPTIONS_WINDOW_CHECKBOX:
			handle_checkbox_click (hide_options_window_checkbox);
			break;			
		
			case SHOW_ABOUT_WINDOW_CHECKBOX:
			handle_checkbox_click (show_about_window_checkbox);
			break;			
		
			case OKAY_BUTTON:
			/* Don't let user close window until the ticks between events number is
				valid */
			if (GetCtlValue(allow_background_tasks_checkbox)
				&& !is_valid_param_int(preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
										ALLOW_BACKGROUND_TASKS_LABEL, 1, INT_MAX))
				{
				
				/* Redraw the dialog in case it was obscured by the error dialog */
				DrawDialog(preferences_dialog);
				
				/* We're not done yet */
				item_hit = 0;
				
				}
			
			else	/* Close the preferences dialog, and accept the new choices */
				{
				
				/* Hide the preferences dialog */
				HideWindow (preferences_dialog);
			
				/* get the new preferences selections from the check boxes */
				new_show_image_rendering = GetCtlValue (show_image_rendering_checkbox);
				allow_background_tasks = GetCtlValue (allow_background_tasks_checkbox);
				new_keep_image_in_memory = GetCtlValue (keep_image_in_memory_checkbox);
				show_status_window_flag = GetCtlValue (show_status_window_checkbox);
				hide_options_window_flag = GetCtlValue (hide_options_window_checkbox);
				show_about_window_flag = GetCtlValue (show_about_window_checkbox);
			
				/* If there was an offscreen image, but there isn't anymore, get rid of
					the one that's there. */
				if ((keep_image_in_memory) && (!new_keep_image_in_memory) && (is_offscreen_port))
					dispose_offscreen_port();
			
				/* If there should no longer be an image window, hide it. */
				if ((show_image_rendering) && (!new_show_image_rendering))
					hide_image_window();
				if ((!show_image_rendering) && (new_show_image_rendering))
					show_image_window(FALSE);
			
				/* If there are background tasks now, get the number of ticks between. */
				GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &type, &handle, &box);
				GetIText (handle, string);
				StringToNum (string, &time_between_events);
			
				/* If we are rendering, reinstall the VBL task at this new frequency */
				if (rendering)
					{
					
					/* If there is a VBL, get rid of it */
					if (vbl_installed)
						remove_get_event_vbl();
			
					/* If there should be a VBL now, install it */
					if (allow_background_tasks)
						install_get_event_vbl();
					
					}
			
				/* Save the new preferences */
				show_image_rendering = new_show_image_rendering;
				keep_image_in_memory = new_keep_image_in_memory;
				
				/* Write the preferences to the preferences file */
				write_preferences();
				
				/* If we're faking a button click, dehilight it */
				HiliteControl(preferences_okay_button_handle, 0);

				}
			
		default:;	/* all other items take care of themselves */
		}
	
}	/* handle_preferences_selection() */



/*****************************************************************************\
* procedure save_image_filter                                                 *
*                                                                             *
* Purpose: This procedure filters events in the save... dialog.               *
*                                                                             *                                                                             *
* Parameters: the_dialog: the save... dialog.                                 *
*             the_event:  the event which occurred                            *
*             the_item:   the item which was hit, if any                      *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
*   Reid Judd  Oct 18,'92   Check for <Enter> key='\3' on extended keyboard   *
\*****************************************************************************/

pascal Boolean
save_image_filter (DialogPtr the_dialog, EventRecord *the_event, int *the_item)
{

	GrafPtr		temp_port;
	Rect		box;
	Handle		item;
	short		type;
	char 		*filename;
	MenuHandle	menu;
	short		useritem_id;
	Point		*upper_left;
	short		*selection;
	short		menu_id;
	
	/* Decide which menu to use, depending on whether there is animation */
	if (GetCtlValue(animate_checkbox))
		{
		
		/* Use animation menu */
		menu = save_anim_file_type_menu;
		useritem_id = SAVE_FILE_TYPE_POPUP;
		upper_left = &save_anim_file_type_menu_upper_left;
		selection = &save_anim_file_type_menu_selection;
		menu_id = SAVE_ANIM_FILE_TYPE_MENU;
		
		}
		
	else
		{
		
		/* Use image menu */
		menu = save_file_type_menu;
		useritem_id = SAVE_FILE_TYPE_POPUP;
		upper_left = &save_file_type_menu_upper_left;
		selection = &save_file_type_menu_selection;
		menu_id = SAVE_FILE_TYPE_MENU;
		
		}
		
	/* Save the grafport */
	GetPort (&temp_port);

	/* Set the grafport to the dialog */
	SetPort (the_dialog);

	/* Handle the standard event: Return hit = item 1 hit */
    if ( (the_event->what == keyDown) && 
		     ( ((the_event->message & charCodeMask) == '\r') ||
		       ((the_event->message & charCodeMask) == '\3') ) )			
		{
		*the_item = 1;
		return TRUE;
		}

	/* Handle an update event for the popup menu */
	if (the_event->what == updateEvt)
		draw_any_popup (the_dialog, useritem_id, menu, *selection, upper_left);

	/* Handle a click in the popup menu */
	if (the_event->what == mouseDown)
		{
		
		/* Get the popup menu rectangle in global coordinates */
		GetDItem (the_dialog, useritem_id, &type, &item, &box);
		LocalToGlobal ((Point *) &box);
		LocalToGlobal ((Point *) &(box.bottom));
		
		if (PtInRect (the_event->where, &box))
			{
			
			/* Popup the menu, get a selection, and save it */
			handle_popup_click(the_dialog, menu, menu_id,
								SAVE_FILE_TYPE_TEXT, useritem_id, *upper_left, selection);
			
			/* Update the filename */
			filename = get_save_filename ();
			GetDItem (the_dialog, SAVE_FILENAME, &type, &item, &box);
			SetIText (item, CtoPstr(filename));
			SelIText (the_dialog, SAVE_FILENAME, 0, 32767);

			/* Update the popup menu in the dialog */
			InvalRect (&box);
			}
		}

	SetPort (temp_port);

	return FALSE;

}	/* save_image_filter() */



/*****************************************************************************\
* procedure get_save_filename                                                 *
*                                                                             *
* Purpose: This procedure generates an appropriate filename to save the image *
*          as, given the filename of the .sff image, and the save type.       *
*                                                                             *                                                                             *
* Parameters: returns filename to save as                                     *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 27, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

char *get_save_filename (void)
{

	static char pathname[200];
	
	/* update the filename to reflect the file type */
	strcpy (pathname, sff_filename);
	
	/* Chop off the .sff ending */
	pathname[strlen(pathname) - 4] = 0;
	
	/* If we're animating, the menu selections mean different things that
		when we're saving an image file */
	if (GetCtlValue(animate_checkbox))
		{

		/* If it's a series of PICTs, add .pict */
		if (save_anim_file_type_menu_selection == 1)
			strcat (pathname, ".pict");
		else
			strcat (pathname, ".qt");		/* Quicktime movie */

		}
	
	else
		{
	
		/* If it's a ppm file, add .pmm */
		if (save_file_type_menu_selection == 2)
			strcat (pathname, ".ppm");
		else
			strcat (pathname, ".PICT");		/* PICT file */
	
		}
	
	/* return the filename */
	return pathname;
	
}	/* get_save_filename() */