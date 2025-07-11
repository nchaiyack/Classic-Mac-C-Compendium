/*****************************************************************************\
* macdialogs.c                                                                *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It implements  *
* all the common code in the interface to the dialog boxes, and fully         *
* implements the code for all the dialog boxes except the preferences and     *
* options dialogs.                                                            *
\*****************************************************************************/

#include "macresources.h"
#include "defs.h"
#include "mactypes.h"
#include "macdefaults.h"
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <setjmp.h>
#include <Movies.h>



/********************************** Types *****************************/

static pascal void MenuProc( short message, MenuHandle menu, Rect *menuRect,
								Point hitPt, short *whichItem);


/********************************** Globals *****************************/

ControlHandle	animation_okay_button_handle;	/* handle to the Okay button in animation */
ControlHandle	about_more_button_handle;		/* handle to the More button in About... */

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
DialogPtr		quit_during_render_dialog;		/* the dialog which prompts users who quit during render */
DialogRecord	quit_during_render_dialog_rec;	/* storage for the Quit During Render dialog */
DialogPtr		param_error_dialog;		/* the Parameter Error dialog */
DialogRecord	param_error_dialog_rec;	/* storage for the Parameter Error dialog */
DialogPtr		invalid_num_dialog;		/* the Invalid Number Error dialog */
DialogRecord	invalid_num_dialog_rec;	/* storage for the Invalid Number Error dialog */
DialogPtr		offer_to_abort_dialog;	/* the Offer to Abort dialog */
DialogRecord	offer_to_abort_dialog_rec;/* storage for the Offer to Abort dialog */
DialogPtr		saving_frame_dialog;	/* the Saving Frame dialog */
DialogRecord	saving_frame_dialog_rec;/* storage for the Saving Frame dialog */

MenuHandle	save_file_type_menu;		/* Popup menu in Save dialog, used when saving image */
MenuHandle	save_anim_file_type_menu;	/* Popup menu in Save dialog, used when saving animation */
short		save_file_type_menu_selection = 1;			/* Default selections in save popups */
short		save_anim_file_type_menu_selection = 1;
Point		save_file_type_menu_upper_left;				/* Positions of save popups */
Point		save_anim_file_type_menu_upper_left;

Boolean	status_dialog_visible = FALSE;	/* TRUE if the status dialog is visible */
Boolean	animation_dialog_visible = FALSE;/* TRUE if the animation dialog is visible */
Boolean	image_window_visible = FALSE;	/* TRUE if the image window is visible */

long	progress_bar_max = 1;			/* maximum value of the progress bar */
long	progress_bar_lower_value = 0;	/* current value of the left side of the shaded part of the progress bar */
long	progress_bar_value = 0;			/* current value of the right side of the shaded part of the progress bar */
short	progress_bar_length;			/* length of the progress bar in the dialog */
short	progress_bar_lower_dividing_line = 0; /* left side of the filled part of the progress bar */
short	progress_bar_dividing_line;		/* right side of the filled part of the progress bar */

long	memory_bar_max = 1;				/* maximum value of the memory bar */
long	memory_bar_value = 0;			/* current_value of the memory bar */
short	memory_bar_length;				/* length of the memory bar in the dialog */
short	memory_bar_dividing_line;		/* length of the filled part of the memory bar */

PicHandle about_picture;				/* Handle to picture in about... dialog */
PicHandle about_text_picture;			/* Handle to text picture in about... dialog */
PicHandle about_credits_picture;		/* Handle to credits picture in about... dialog */

RGBColor bar_blue = {0xD000, 0xD000, 0xFFFF};	/* colors of the progress bars */
RGBColor bar_gray = {0x4000, 0x4000, 0x4000};
RGBColor bar_black = {0x0000, 0x0000, 0x0000};

Rect	about_rtrace_image_bounds;		/* Enclosing rectangle of the RTrace image in About... */

UserItemUPP draw_button_border_upp;
UserItemUPP draw_about_picture_upp;
UserItemUPP draw_about_text_picture_upp;
UserItemUPP draw_about_credits_picture_upp;
UserItemUPP draw_progress_bar_upp;
UserItemUPP draw_memory_bar_upp;

ModalFilterUPP save_image_filter_upp;

/* externals */

extern Boolean		image_complete;		/* TRUE if the image is completely rendered */
extern char			scene_filename[];	/* filename of open scene file */
extern Boolean 		is_offscreen_port;	/* TRUE when there is an offscreen CGrafPort */
extern WindowPtr	image_window;		/* the image window data structures */
extern Boolean		scene_in_memory;	/* TRUE if the scene for the current file is in memory */
extern Boolean		rendering_same_file;/* TRUE if the file is same as last render */
extern PicHandle	down_arrow_picture;	/* down arrows for popup */
extern PicHandle	down_arrow_picture_grayed;
extern jmp_buf		environment;		/* Used to store environment to restore on error */
extern MenuHandle	file_menu;
extern MenuHandle	windows_menu;		/* the Windows pulldown menu */
extern Boolean		f8bit_QD_available;	/* TRUE if 8-bit QuickDraw is available */
extern Boolean		f32bit_QD_available;/* TRUE if 32-bit QuickDraw is available */
extern Boolean		quicktime_available;/* TRUE if QuickTime is available */
extern Boolean 		vbl_installed;		/* TRUE if the VBL event task is installed */
extern Boolean		rendering;			/* TRUE while we are rendering */
extern short		temp_folder_wd_id;	/* Working Directory refnum of Temporary Items */
extern DialogPtr	preferences_dialog;		/* the preferences dialog */
extern DialogRecord	preferences_dialog_rec;	/* storage for the preferences dialog */
extern preferences_handle prefs;			/* The preferences */
extern ControlHandle animate_checkbox;


/********************** Local Prototypes ************************/
void init_alert_dialogs(void);
void setup_useritems(void);
void draw_popup (Rect *the_box, Str255 the_string, short enabled);
void setup_popup_menu(DialogPtr theDialog, short useritem_id, UserItemUPP proc);
void handle_popup_click(DialogPtr the_dialog, MenuHandle popup_menu, short menu_id,
						short popup_text_id, short popup_useritem_id,
						Point menu_upper_left, short *menu_selection);
void set_dialog_text_to_int(DialogPtr theDialog, short theItem, long value);
void set_dialog_text_to_real(DialogPtr theDialog, short theItem, double value);
long get_edittext_as_int(DialogPtr theDialog, int theItem);
double get_edittext_as_real(DialogPtr theDialog, int theItem);
void draw_any_popup (DialogPtr the_dialog, short item_number, MenuHandle menu,
							short menu_selection, Point *upper_left);
pascal void draw_button_border (WindowPtr the_window, short item_number);
pascal void draw_about_picture (WindowPtr the_window, short item_number);
pascal void draw_about_text_picture (WindowPtr the_window, short item_number);
pascal void draw_about_credits_picture (WindowPtr the_window, short item_number);
pascal void draw_progress_bar (WindowPtr the_window, short item_number);
pascal void draw_memory_bar (WindowPtr the_window, short item_number);
void install_popup_menu_hook(MenuHandle popup_menu);
void FlushCache(void);
Boolean TrapAvailable( short theTrap);
void setup_check_boxes(void);
void handle_checkbox_click(ControlHandle check_box);
void show_about_dialog(void);
void init_dialogs(void);
pascal Boolean save_image_filter (DialogPtr the_dialog, EventRecord *the_event,
									int *the_item);
char *get_save_filename (void);
void show_status_dialog(Boolean bring_to_front);
void hide_status_dialog(void);
void show_image_window(Boolean bring_to_front);
void hide_image_window(void);
void set_dialog_text(DialogPtr dialog, short item, Str255 string);
void set_dialog_pict(DialogPtr dialog, short item, short pic_id);
void set_status_text(char *string);
void set_progress_bar_value(long value);
void set_progress_bar_max(long value);
void update_progress_bar (void);
void update_memory_bar_value(void);
void set_memory_bar_max(long value);
void update_memory_bar(void);
void show_animation_dialog(Boolean bring_to_front);
void hide_animation_dialog(void);
void handle_animation_selection(short item_hit);
Boolean is_valid_real(char *string);
Boolean is_valid_int(char *string);
Boolean is_valid_param_real(DialogPtr dialog, short text_item_id, short label_item_id,
							real min, real max, Boolean include_min, Boolean include_max);
Boolean is_valid_param_int(DialogPtr dialog, short text_item_id, short label_item_id,
							long min, long max);
void run_rtrace_movie(void);
void init_progress_bar_thermometer_mode(void);
void init_progress_bar_step_mode(void);


/********************** External Prototypes ************************/
void place_window (WindowPtr window, Rect *bounds);
void init_options_dialog(void);
void init_prefs_dialog(void);
void load_options_popup_menus(void);
void setup_options_controls(void);
void setup_prefs_controls(void);
void setup_prefs_useritems(void);
void setup_options_useritems(void);
void handle_prefs_checkbox_click(ControlHandle check_box);
void handle_options_popup_click(short menu_id, short old_selection, short new_selection);
void handle_options_checkbox_click(ControlHandle check_box);



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
	
	/* Initialize our UPPs for alert items */
	draw_button_border_upp = NewUserItemProc(draw_button_border);
	
	/* Set up the border around the default button in the Offer To Abort dialog */
	GetDItem (offer_to_abort_dialog, OFFER_ABORT_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (offer_to_abort_dialog, OFFER_ABORT_FRAME_BUTTON_USERITEM, item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Error dialog */
	GetDItem (error_dialog, ERROR_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (error_dialog, ERROR_FRAME_BUTTON_USERITEM, item_type, (Handle) draw_button_border_upp, &item_box);

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
	
	/* Initialize the about, progress and memory UPPs */
	draw_about_picture_upp = NewUserItemProc(draw_about_picture);
	draw_about_text_picture_upp = NewUserItemProc(draw_about_text_picture);
	draw_about_credits_picture_upp = NewUserItemProc(draw_about_credits_picture);
	draw_progress_bar_upp = NewUserItemProc(draw_progress_bar);
	draw_memory_bar_upp = NewUserItemProc(draw_memory_bar);

	/* Initialize the options dialog */
	init_options_dialog();
	
	/* Initialize the preferences dialog */
	init_prefs_dialog();

	/* Read in the dialog boxes.  They are invisible */
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
	quit_during_render_dialog = GetNewDialog (QUIT_DURING_RENDER_DIALOG,
											&quit_during_render_dialog_rec, (WindowPtr)-1L);
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
	place_window(quit_during_render_dialog, (Rect *) NULL);
	place_window(invalid_num_dialog, (Rect *) NULL);
		
	/* Read the pictures into memory */
	about_picture = GetPicture(ABOUT_PICT_32);
	about_text_picture = GetPicture(ABOUT_TEXT_PICT_32);
	about_credits_picture = GetPicture(ABOUT_CREDITS_8);
	
	/* Hide the credits picture for starters */
	HideDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM);
	
	/* read in the popup menu resources for the options dialog box, and
		install the menu hooks */
	load_options_popup_menus();
	
	/* set up the check boxes in the options dialog box */
	setup_check_boxes();
	
	/* set up the popup menus and the frame around the active button */
	setup_useritems();

	/* Set up popup menu used during image save */
	save_file_type_menu = GetMenu (SAVE_FILE_TYPE_MENU);
	install_popup_menu_hook(save_file_type_menu);
	
	/* Set up popup menu used during animation save */
	save_anim_file_type_menu = GetMenu (SAVE_ANIM_FILE_TYPE_MENU);
	install_popup_menu_hook(save_anim_file_type_menu);
	
	/* Initailize the UPP for our Save dialog filter proc */
	save_image_filter_upp = NewModalFilterProc(save_image_filter);
}	/* init_dialogs() */



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
	if (bring_to_front)
		SelectWindow(status_dialog);
	else
		HiliteWindow(status_dialog, status_dialog == FrontWindow());
	
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
	if (bring_to_front)
		SelectWindow (animation_dialog);
	else
		HiliteWindow(animation_dialog, animation_dialog == FrontWindow());
	
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
	if (bring_to_front)
		SelectWindow (image_window);
	else
		HiliteWindow(image_window, image_window == FrontWindow());
	
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
						
				/* Okay was clicked-- hide the dialog, and turn on the animate checkbox... */
				hide_animation_dialog();
				SetCtlValue(animate_checkbox, TRUE);

				}
			/* If we're faking a button click, dehilight it */
			HiliteControl(animation_okay_button_handle, 0);

			break;
			
		case CANCEL_BUTTON:
			
			/* Cancel was clicked-- hide the dialog, and turn off the animate checkbox. */
			hide_animation_dialog();
			SetCtlValue(animate_checkbox, FALSE);			

			break;

		default:;	/* all other items take care of themselves */
		}

}	/* handle_animation_selection() */



/*****************************************************************************\
* procedure set_dialog_text_to_int                                            *
*                                                                             *
* Purpose: This procedure changes a dialog text item to display an integer    *
*          value.                                                             *
*                                                                             *
* Parameters: theDialog: the dialog in which the text item appears.           *
*             theItem:   the text item in the dialog to change.               *
*             value:     the value to change the text to.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_text_to_int(DialogPtr theDialog, short theItem, long value)
{

	short	type;		/* the item type of a DITL item */
	Handle	handle;		/* the handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */
	Str255	intString;
	
	/* Convert the integer to a string */
	NumToString(value, intString);

	/* Set the EditText item */
	GetDItem (theDialog, theItem, &type, &handle, &box);
	SetIText(handle, intString);

}  /* set_dialog_text_to_int() */



/*****************************************************************************\
* procedure set_dialog_text_to_real                                           *
*                                                                             *
* Purpose: This procedure changes a dialog text item to display a real        *
*          value.                                                             *
*                                                                             *
* Parameters: theDialog: the dialog in which the text item appears.           *
*             theItem:   the text item in the dialog to change.               *
*             value:     the value to change the text to.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void set_dialog_text_to_real(DialogPtr theDialog, short theItem, double value)
{

	short	type;		/* the item type of a DITL item */
	Handle	handle;		/* the handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */
	Str255	realString;
	
	/* Convert the real to a string */
	sprintf((char *) realString, "%g", value);
	c2pstr((char *) realString);

	/* Set the EditText item */
	GetDItem (theDialog, theItem, &type, &handle, &box);
	SetIText(handle, realString);

}  /* set_dialog_text_to_real() */



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
	Str255	number_string;		/* the text of the item */
	long	number;				/* the number itself */
	Str255	label;				/* the label for the item */
	short	item;				/* item hit in ModalDialog call */
	Str255	string;				/* temporary variable */

	/* Get the text of the item */
	GetDItem (dialog, text_item_id, &type, &handle, &box);
	GetIText (handle, number_string);

	/* Convert it to a C-style string */
	p2cstr(number_string);

	/* Get the text of the label */
	GetDItem (dialog, label_item_id, &type, &handle, &box);
	GetIText (handle, label);

	/* Chop off the colon at the end of the label */
	label[0]--;

	/* Verify that the number is a valid integer */
	if (!is_valid_int((char *) number_string))
		{
		
		/* Not a valid integer-- notify user */
		ParamText(label, "\pan integer", 0, 0);
		ShowWindow(invalid_num_dialog);
		SelectWindow(invalid_num_dialog);
		ModalDialog((ModalFilterUPP) NULL, &item);
		HideWindow(invalid_num_dialog);
	
		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Convert it to an integer */
	StringToNum(c2pstr((char *) number_string), &number);
	
	/* Check it against its upper bound */
	if (number > max)
		{
		
		/* To large-- notify user */
		NumToString(max, string);
		ParamText(label, "\pless than or equal to", string, 0);
		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item);
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
		ModalDialog ((ModalFilterUPP) NULL, &item);
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
	Str255	number_string;		/* the text of the item */
	real	number;				/* the number itself */
	Str255	label;				/* the label for the item */
	short	item;				/* item hit in ModalDialog call */
	Str255	string;				/* temporary variable */

	/* Get the text of the item */
	GetDItem (dialog, text_item_id, &type, &handle, &box);
	GetIText (handle, number_string);

	/* Convert it to a C-style string */
	p2cstr(number_string);

	/* Get the text of the label */
	GetDItem (dialog, label_item_id, &type, &handle, &box);
	GetIText (handle, label);

	/* Chop off the colon at the end of the label */
	label[0]--;

	/* Verify that the number is a valid real number */
	if (!is_valid_real((char *) number_string))
		{
		
		/* Not a valid real-- notify user */
		ParamText(label, "\pa number", 0, 0);
		ShowWindow(invalid_num_dialog);
		SelectWindow(invalid_num_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item);
		HideWindow(invalid_num_dialog);
	
		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Convert it to an real number */
	number = atof((char *) number_string);
	
	/* Check it against its upper bound */
	if ((number > max) || ((number == max ) && (!include_max)))
		{
		
		/* To large-- notify user */

		/* Convert max to a string */
		sprintf((char *) string, "%lg", max);

		/* Use "less than or equal to" if the max is included; otherwise, just
			use "less than" */
		if (include_max)
			ParamText(label, "\pless than or equal to", c2pstr((char *) string), 0);
		else
			ParamText(label, "\pless than", c2pstr((char *) string), 0);

		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item);
		HideWindow(param_error_dialog);

		/* Tell caller that there was a problem */
		return FALSE;	
		}

	/* Check it against its lower bound */
	if ((number < min) || ((number == min ) && (!include_min)))
		{
		
		/* To small-- notify user */
		
		/* Convert min to a string */
		sprintf((char *) string, "%lg", min);

		/* Use "greater than or equal to" if the min is included; otherwise, just
			use "greater than" */
		if (include_min)
			ParamText(label, "\pgreater than or equal to", c2pstr((char *) string), 0);
		else
			ParamText(label, "\pgreater than", c2pstr((char *) string), 0);

		ShowWindow(param_error_dialog);
		SelectWindow(param_error_dialog);
		ModalDialog ((ModalFilterUPP) NULL, &item);
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

	/* Set up the options dialog controls */
	setup_options_controls();

	/* Set up the preferences dialog controls */
	setup_prefs_controls();

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

	/* Set up the useritems in the options dialog */
	setup_options_useritems();

	/* Set up the useritems in the preferences dialog */
	setup_prefs_useritems();

	/* Set up the border around the default button in the animation dialog */
	GetDItem (animation_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (animation_dialog, BUTTON_BORDER_USERITEM, item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the about dialog */
	GetDItem (about_dialog, ABOUT_BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_BUTTON_BORDER_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Cancel dialog */
	GetDItem (cancel_dialog, CANCEL_FRAME_BUTTON_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (cancel_dialog, CANCEL_FRAME_BUTTON_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Abort Render dialog */
	GetDItem (abort_render_dialog, ABORT_RENDER_FRAME_BUTTON_USERITEM,
				&item_type, &item_handle, &item_box);
	SetDItem (abort_render_dialog, ABORT_RENDER_FRAME_BUTTON_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Parameter Error dialog */
	GetDItem (param_error_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (param_error_dialog, BUTTON_BORDER_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Prompt to Save dialog */
	GetDItem (prompt_save_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (prompt_save_dialog, BUTTON_BORDER_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the QUIT_DURING_RENDER dialog */
	GetDItem (quit_during_render_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (quit_during_render_dialog, BUTTON_BORDER_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the border around the default button in the Invalid Number Error dialog */
	GetDItem (invalid_num_dialog, BUTTON_BORDER_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (invalid_num_dialog, BUTTON_BORDER_USERITEM,
				item_type, (Handle) draw_button_border_upp, &item_box);

	/* Set up the picture useritems in the About... dialog */
	GetDItem (about_dialog, ABOUT_PICT_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_PICT_USERITEM, item_type, (Handle) draw_about_picture_upp, &item_box);
	GetDItem (about_dialog, ABOUT_TEXT_PICT_USERITEM, &item_type, &item_handle,
				&about_rtrace_image_bounds);
	SetDItem (about_dialog, ABOUT_TEXT_PICT_USERITEM, item_type, (Handle) draw_about_text_picture_upp,
				&about_rtrace_image_bounds);
	GetDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (about_dialog, ABOUT_CREDITS_PICT_USERITEM, item_type,
				(Handle) draw_about_credits_picture_upp, &item_box);
	
	/* Set up the progress bar in the status dialog */
	GetDItem (status_dialog, PROGRESS_BAR_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (status_dialog, PROGRESS_BAR_USERITEM, item_type, (Handle) draw_progress_bar_upp, &item_box);
	progress_bar_length = item_box.right - item_box.left - 2;

	/* Set up the memory bar in the status dialog */
	GetDItem (status_dialog, MEMORY_BAR_USERITEM, &item_type, &item_handle, &item_box);
	SetDItem (status_dialog, MEMORY_BAR_USERITEM, item_type, (Handle) draw_memory_bar_upp, &item_box);
	memory_bar_length = item_box.right - item_box.left - 2;

	/* Get the handle to the Okay button in the animation dialog */
	GetDItem (animation_dialog, OKAY_BUTTON, &item_type, (Handle *) &animation_okay_button_handle,
				&item_box);

	/* Get the handle to the More button in the about dialog */
	GetDItem (about_dialog, MORE_BUTTON, &item_type, (Handle *) &about_more_button_handle,
				&item_box);

}	/* setup_useritems() */



/*****************************************************************************\
* procedure setup_popup_menu                                                  *
*                                                                             *
* Purpose: This procedure sets up a popup menu.                               *
*                                                                             *
* Parameters: theDialog:   the dialog box the popup will be in                *
*             useritem_id: the ID of the useritem which is the bounding box   *
*                          of the popup menu.                                 *
*             proc:        the procedure to draw the useritem (called by the  *
*                          Dialog Manager).                                   *
*             upper_left:  the upper left corner of the popup menu.           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
*   January 4, 1995 by Greg Ferrar                                            *
*     Added theDialog parameter to allow popups in any window.                *
\*****************************************************************************/

void setup_popup_menu(DialogPtr theDialog, short useritem_id, UserItemUPP proc)
{

	short	item_type;		/* the item type of a DITL item */
	Handle	item_handle;	/* a handle to a DITL item */
	Rect	item_box;		/* the bounding box of a DITL item */

	/* Set up the popup menu */
	GetDItem (theDialog, useritem_id, &item_type, &item_handle, &item_box);
	SetDItem (theDialog, useritem_id, userItem, (Handle) proc, &item_box);

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
	Rect	pict_rect;

	/* Find the item's rectangle */
	GetDItem (the_window, ABOUT_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Find the PICT's rectangle, and translate it until it's upper left
		corner matches that of the item's rectangle */
	pict_rect = (*about_picture)->picFrame;
	OffsetRect(&pict_rect, -pict_rect.left, -pict_rect.top);
	OffsetRect(&pict_rect, item_box.left, item_box.top);

	/* Draw the picture in the rectangle */
	DrawPicture (about_picture, &pict_rect);

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
	Rect	pict_rect;

	/* Find the item's rectangle */
	GetDItem (the_window, ABOUT_TEXT_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Find the PICT's rectangle, and translate it until it's upper left
		corner matches that of the item's rectangle */
	pict_rect = (*about_text_picture)->picFrame;
	OffsetRect(&pict_rect, -pict_rect.left, -pict_rect.top);
	OffsetRect(&pict_rect, item_box.left, item_box.top);

	/* Draw the picture in the rectangle */
	DrawPicture (about_text_picture, &pict_rect);

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
	Rect	pict_rect;

	/* Find the item's rectangle */
	GetDItem (the_window, ABOUT_CREDITS_PICT_USERITEM, &item_type, &item_handle,
				&item_box);

	/* Find the PICT's rectangle, and translate it until it's upper left
		corner matches that of the item's rectangle */
	pict_rect = (*about_credits_picture)->picFrame;
	OffsetRect(&pict_rect, -pict_rect.left, -pict_rect.top);
	OffsetRect(&pict_rect, item_box.left, item_box.top);

	/* Draw the picture in the rectangle */
	DrawPicture (about_credits_picture, &pict_rect);

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

	short			type;
	Handle			item;
	Rect			box;
	Rect			progress_bar_box;
	

	/* Find the new length of the filled part of the bar */
	progress_bar_dividing_line = (progress_bar_length * progress_bar_value)/progress_bar_max;
	progress_bar_lower_dividing_line = (progress_bar_length * progress_bar_lower_value)/progress_bar_max;
	
	/* Set the port to draw in the status dialog */
	SetPort (status_dialog);
	
	/* get the outline of the bar */
	GetDItem (status_dialog, PROGRESS_BAR_USERITEM, &type, &item, &progress_bar_box);
	
	/* outline the bar */
	FrameRect (&progress_bar_box);
	
	/* Get the interior of the bar */
	InsetRect (&progress_bar_box, 1, 1);
	
	/* Copy the interior rectangle */
	BlockMove(&progress_bar_box, &box, sizeof(Rect));

	/* If Color QuickDraw is available, color the filled part solid dark gray */
	if (f8bit_QD_available)
		RGBForeColor (&bar_gray);
		
	/* Otherwise, use Original QD dithered gray */
	else
		PenPat(&qd.gray);

	/* Shade the filled part */
	box.right = box.left + progress_bar_dividing_line;
	box.left += progress_bar_lower_dividing_line;
	PaintRect(&box);
	
	/* If Color QuickDraw is available, color the empty part light blue */
	if (f8bit_QD_available)
		RGBForeColor (&bar_blue);

	/* Otherwise, use Original QD white */
	else
		PenPat (&qd.white);

	/* Erase the unfilled part at the left */
	BlockMove(&progress_bar_box, &box, sizeof(Rect));
	box.right = box.left + progress_bar_lower_dividing_line;
	PaintRect (&box);

	/* Erase the unfilled part at the right */
	BlockMove(&progress_bar_box, &box, sizeof(Rect));
	box.left = box.left + progress_bar_dividing_line;
	PaintRect (&box);

	/* Restore the foreground color to normal */
	if (f8bit_QD_available)
		RGBForeColor (&bar_black);
	else
		PenPat(&qd.black);

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

	GrafPtr temp_port;

	/* Draw the bar if its appearance has changed */
	if ((progress_bar_dividing_line != 
			(progress_bar_length * progress_bar_value)/progress_bar_max) ||
		(progress_bar_lower_dividing_line !=
			(progress_bar_length * progress_bar_lower_value)/progress_bar_max))

		{
		
		/* Save the current GrafPort */
		GetPort(&temp_port);
		
		/* Draw the progress bar in the status dialog */
		SetPort(status_dialog);
		draw_progress_bar(status_dialog, PROGRESS_BAR_USERITEM);
	
		/* Restore the original GrafPort */
		SetPort(temp_port);
	
		}
	
}	/* update_progress_bar() */



/*****************************************************************************\
* procedure draw_memory_bar                                                   *
*                                                                             *
* Purpose: This procedure draws the memory bar in the status dialog.          *
*                                                                             *
* Parameters: all ignored                                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_memory_bar (WindowPtr the_window, short item_number)
{

	short			type;
	Handle			item;
	Rect			box;
	

	/* Find the new length of the filled part of the bar */
	memory_bar_dividing_line = (memory_bar_length * memory_bar_value)/memory_bar_max;
	
	/* Set the port to draw in the status dialog */
	SetPort (status_dialog);
	
	/* get the outline of the bar */
	GetDItem (status_dialog, MEMORY_BAR_USERITEM, &type, &item, &box);
	
	/* outline the bar */
	FrameRect (&box);
	
	/* Get the interior of the bar */
	InsetRect (&box, 1, 1);

	/* If Color QuickDraw is available, color the filled part solid dark gray */
	if (f8bit_QD_available)
		RGBForeColor (&bar_gray);
		
	/* Otherwise, use Original QD dithered gray */
	else
		PenPat(&qd.gray);

	/* Shade the filled part */
	box.right = box.left + memory_bar_dividing_line;
	PaintRect(&box);
	
	/* If Color QuickDraw is available, color the empty part light blue */
	if (f8bit_QD_available)
		RGBForeColor (&bar_blue);

	/* Otherwise, use Original QD white */
	else
		PenPat (&qd.white);

	/* Erase the unfilled part */
	box.right = box.left + memory_bar_length;
	box.left += memory_bar_dividing_line;
	PaintRect (&box);

	/* Restore the foreground color to normal */
	if (f8bit_QD_available)
		RGBForeColor (&bar_black);
	else
		PenPat(&qd.black);

}	/* draw_memory_bar() */



/*****************************************************************************\
* procedure update_memory_bar                                                 *
*                                                                             *
* Purpose: This procedure draws the memory bar in the status dialog, but      *
*          only if it has changed since last drawing.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_memory_bar (void)
{

	GrafPtr temp_port;

	/* Draw the bar if its appearance has changed */
	if (memory_bar_dividing_line != 
			(memory_bar_length * memory_bar_value)/memory_bar_max)
		{
		
		/* Save the current GrafPort */
		GetPort(&temp_port);
		
		/* Draw the memory bar in the status dialog */
		SetPort(status_dialog);
		draw_memory_bar(status_dialog, MEMORY_BAR_USERITEM);
	
		/* Restore the original GrafPort */
		SetPort(temp_port);
	
		}
	
}	/* update_memory_bar() */


#if 0

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

#endif


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

void set_status_text(char *string)
{

	/* set the text (converting string to a pascal string) */
	set_dialog_text(status_dialog, STATUS_TEXT, c2pstr(string));

	/* Convert string back to a C string */
	p2cstr((unsigned char *) string);

}	/* set_status_text() */



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
* procedure init_progress_bar_thermometer_mode                                *
*                                                                             *
* Purpose: This procedure prepares the progress bar to enter thermometer      *
*          mode, where it shows the value of some parameter by drawing a      *
*          dark leftmost part of proportional length.                         *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_progress_bar_thermometer_mode(void)
{

	/* Make the lower value zero (it always is, in thermometer mode) */
	progress_bar_lower_value = 0;
	
	/* Initialize the value to 0 */
	progress_bar_value = 0;

	/* Make the maximum some arbitrary number */
	progress_bar_max = 100;

	/* update the progress bar */
	update_progress_bar();

}	/* init_progress_bar_thermometer_mode() */



/*****************************************************************************\
* procedure init_progress_bar_step_mode                                       *
*                                                                             *
* Purpose: This procedure prepares the progress bar to enter step mode, where *
*          a small band of dark marches across the bar, moving a little for   *
*          each step.                                                         *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_progress_bar_step_mode(void)
{

	/* Set the maximum */
	progress_bar_max = 20;
	
	/* Start the band at the left */
	progress_bar_lower_value = 0;
	progress_bar_value = 1;

	/* update the progress bar */
	update_progress_bar();

}	/* init_progress_bar_step_mode() */



/*****************************************************************************\
* procedure step_progress_bar                                                 *
*                                                                             *
* Purpose: This procedure advances the progress bar one step.  You should     *
*          call init_progress_bar_step mode before calling this.              *
*                                                                             *
* Parameters: value: the new progress bar value                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void step_progress_bar(void)
{

	/* Go forward one step */
	progress_bar_value++;
	
	/* Wrap around if necessary */
	if (progress_bar_value > progress_bar_max)
		progress_bar_value = 1;
	
	/* Match the lower value to the upper one */
	progress_bar_lower_value = progress_bar_value - 1;
	
	/* update the progress bar */
	update_progress_bar();

}	/* init_progress_bar_step_mode() */



/*****************************************************************************\
* procedure update_memory_bar_value                                           *
*                                                                             *
* Purpose: This procedure sets the value of the memory bar in the status      *
*          dialog to the current amount of free memory, and redraws the       *
*          memory bar.                                                        *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_memory_bar_value(void)
{

	/* Set the bar to the current amount of OCCUPIED memory */
	memory_bar_value = memory_bar_max - FreeMem();

	/* update the memory bar */
	update_memory_bar();

}	/* update_memory_bar_value() */



/*****************************************************************************\
* procedure set_memory_bar_max                                                *
*                                                                             *
* Purpose: This procedure sets the maximum value of the memory bar in the     *
*          status dialog.  It does not redraw the memory bar.                 *
*                                                                             *
* Parameters: value: the new memory bar value                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_memory_bar_max(long value)
{

	/* Set the bar to the new value */
	memory_bar_max = value;

}	/* set_memory_bar_max() */



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

	short	type;
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

	short		type;
	Handle		handle;
	PicHandle	pic_handle;
	Rect		box;

	/* Read in the picture */
	pic_handle = GetPicture (pic_id);

	/* Set the DITL item to be the picture */
	GetDItem (dialog, item, &type, &handle, &box);
	SetDItem (dialog, item, type, (Handle) pic_handle, &box);

}	/* set_dialog_pict() */



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
	Pattern *draw_pat = (enabled) ? &qd.black : &qd.gray;
	
	/* make room for the outline */
	
	the_box->right--;
	the_box->bottom--;
	
	/* Outline rectangle and add drop box */
	PenPat(draw_pat);
	OffsetRect (the_box, 1, 1);
	FrameRect (the_box);
	OffsetRect (the_box, -1, -1);
	PenPat(&qd.white);
	PaintRect (the_box);
	PenPat(draw_pat);
	FrameRect (the_box);

	/* Draw the text in the rectangle, if there's room */
	if (the_box->right - the_box->left > 50)
		{
		MoveTo (the_box->left+15, the_box->bottom-6);
		DrawString (the_string);
		}

	/* Draw the down arrow at the right of the box */
	OffsetRect (&down_arrow_rect, the_box->right-17, the_box->bottom-13);
	if (enabled)
		DrawPicture(down_arrow_picture, &down_arrow_rect);
	else
		DrawPicture(down_arrow_picture_grayed, &down_arrow_rect);

	PenPat(&qd.black);
	
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

#if __powerc
#pragma options align=mac68k
#endif

	typedef struct tMenuHook	/* The structure of the menu hook routine */
	{
		long	lea;
		short	movea;
		short	jmp;
		long	addr;
		Handle	realMenuProc;
		
	} tMenuHook, **tMenuHookHndl;

#if __powerc
#pragma options align=reset
#endif
	
void install_popup_menu_hook(MenuHandle popup_menu)
{

	tMenuHookHndl	theHook = (tMenuHookHndl) NewHandle ( sizeof( tMenuHook));
	
	/* install a hook which just jumps to MenuProc */
	(**theHook).lea = 0x41FA0006;	/* LEA PC+8, A0 */
	(**theHook).movea = 0x2050;		/* MOEVA.L (a0), A0 */
	(**theHook).jmp = 0x4EF9;		/* JMP instruction */
	(**theHook).addr = (long) NewMenuDefProc(MenuProc);;
	(**theHook).realMenuProc = (**popup_menu).menuProc;	
	
	(**popup_menu).menuProc = (Handle) theHook;

#if !__powerc	
	FlushCache();	/* After modifying code, we must flush the cache on 68040s */
#endif

}	/* install_popup_menu_hook() */


#if !__powerc
/******************************************************************************
 FlushCache		TCL 1.1.1 DLP 9/27/91
 
 	Flush the CPU cache(s). This is required on the 68040 after modifying
 	code. 
 ******************************************************************************/

void FlushCache(void)
{
#ifdef THINK_C								// for old univ headers [THINK_C]
	#define _FlushCodeCache		0xA0BD
	if (TrapAvailable(_FlushCodeCache))
		asm
		{
			dc.w _FlushCodeCache
		}
#else										// for new univ headers [CW]
	FlushCodeCache();
#endif
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
#endif	/* !__powerc */


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
	CallMenuDefProc( (MenuDefUPP)(*realMenuProc), message, menu, menuRect, hitPt, whichItem);
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
		
		/* If the user clicked it on, show the Animation dialog.  Otherwise,
			hide the dialog. */
		if (!current_value)
			show_animation_dialog(TRUE);
		else
			hide_animation_dialog();
		}


	/* Let the preferences dialog deal with this click, if it wants to */
	handle_prefs_checkbox_click(check_box);
	
	/* Let the options dialog deal with this click, if it wants to */
	handle_options_checkbox_click(check_box);
	
}	/* handle_checkbox_click() */



/*****************************************************************************\
* procedure handle_popup_click                                                *
*                                                                             *
* Purpose: This procedure handles a click in a popup menu.  It pops up the    *
*          menu and saves the new selection.                                  *
*                                                                             *
* Parameters: the_dialog:      the dialog box in which the click occured      *
*             popup_menu:      the menu itself                                *
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
	Rect		invert_rect;
	Handle		handle;
	short 		type;
	long		result;
	short		new_selection;
	
	/* Make sure we draw in the dialog box */
	SetPort (the_dialog);

	/* Invert the popup text, if any */
	if (popup_text_id)
		{
		/* Find the rectangle of the text item to invert */
		GetDItem (the_dialog, popup_text_id, &type, &handle,
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
	
		/* Let the options dialog handle this selection, if it wants to */
		handle_options_popup_click(menu_id, *menu_selection, new_selection);
	
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
* procedure draw_any_popup                                                    *
*                                                                             *
* Purpose: This procedure draws a popup menus.                                *
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
	short 		item_type;
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
	strcpy ((char *) fsspec.name, (char *) "rtrace_movie");
	c2pstr ((char *) fsspec.name);

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
* procedure get_edittext_as_int                                               *
*                                                                             *
* Purpose: This procedure gets the value of an EditText item, assuming that   *
*          it is an integer.                                                  *
*                                                                             *                                                                             *
* Parameters: theDialog: the dialog the EditText item appears in.             *
*             theItem:   the item number of the EditText item                 *
*             returns the value of the EditText item.                         *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

long get_edittext_as_int(DialogPtr theDialog, int theItem)
{

	Rect		box;
	Handle		item;
	short		type;
	Str255		string;
	long		value;

	/* Get a handle to the item */
	GetDItem (theDialog, theItem, &type, &item, &box);
	
	/* Get the text of the item */
	GetIText (item, string);
	
	/* Convert the text of the item to a long */
	StringToNum (string, &value);
	
	/* Return the value */
	return value;

}	/* get_edittext_as_int() */



/*****************************************************************************\
* procedure get_edittext_as_real                                              *
*                                                                             *
* Purpose: This procedure gets the value of an EditText item, assuming that   *
*          it is a real number.                                               *
*                                                                             *                                                                             *
* Parameters: theDialog: the dialog the EditText item appears in.             *
*             theItem:   the item number of the EditText item                 *
*             returns the value of the EditText item.                         *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

double get_edittext_as_real(DialogPtr theDialog, int theItem)
{

	Rect		box;
	Handle		item;
	short		type;
	Str255		string;

	/* Get a handle to the item */
	GetDItem (theDialog, theItem, &type, &item, &box);
	
	/* Get the text of the item */
	GetIText (item, string);
	
	/* Convert the text of the item to a C string */
	p2cstr(string);
	
	/* Convert the text of the item to a double, and return it */
	return atof((char *) string);

}	/* get_edittext_as_real() */



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
*   WHO          WHEN             WHAT                                        *
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
	if ((the_event->what == keyDown) && ((the_event->message & charCodeMask) == 0xD))
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
			SetIText (item, c2pstr(filename));
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
	strcpy (pathname, scene_filename);
	
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