/*****************************************************************************\
* macprefs.c                                                                  *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It implements  *
* a the interface to the Preferences dialog box, which allows the user to set *
* some preferences which do remain set across sessions.                       *
\*****************************************************************************/

#include <limits.h>
#include <Folders.h>

#include "defs.h"

#include "macresources.h"
#include "mactypes.h"
#include "macdefaults.h"


#define CURRENT_PREFS_VERSION	5



/*************************** Globals *****************************/
preferences_handle prefs;			/* The preferences */
preferences_handle original_prefs;	/* Duplicate of preferences used to revert */

ControlHandle	preferences_okay_button_handle;	/* handle to the Okay button in preferences */
ControlHandle	preferences_defaults_button_handle;/* handle to the Defaults button in preferences */
ControlHandle	preferences_revert_button_handle;/* handle to the Revert button in preferences */
ControlHandle	allow_background_tasks_checkbox;
ControlHandle	show_image_rendering_checkbox;
ControlHandle	keep_image_in_memory_checkbox;
ControlHandle	show_status_window_checkbox;
ControlHandle	show_about_window_checkbox;
ControlHandle	hide_options_window_checkbox;
ControlHandle	use_textures_checkbox;
ControlHandle	textures_in_objects_radio;
ControlHandle	textures_after_objects_radio;

Str255			rtrace_prefs_filename = "\pMacRTrace Preferences";
short			preferences_file_refnum;

DialogPtr		preferences_dialog;		/* the preferences dialog */
DialogRecord	preferences_dialog_rec;	/* storage for the preferences dialog */

UserItemUPP		dim_relinquish_control_upp;
UserItemUPP		dim_show_image_rendering_upp;


/*************************** Externals *****************************/
extern Boolean		rendering;				/* TRUE while we are rendering */
extern Boolean		reread_scene;			/* TRUE if we should reread the scene in memory */
extern Boolean 		vbl_installed;			/* TRUE if the VBL event task is installed */
extern Boolean		find_folder_available;	/* TRUE if FindFolder is available */
extern short		blessed_folder_wd_id;	/* Working Directory refnum of blessed folder */
extern WindowPtr	log_window;				/* the log window */
extern DialogPtr	options_dialog;			/* the options dialog */
extern DialogPtr	status_dialog;			/* the status dialog */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern WindowPtr	image_window;			/* the image window data structures */
extern Boolean		f8bit_QD_available;		/* TRUE if 8-bit QuickDraw is available */
extern Boolean		f32bit_QD_available;	/* TRUE if 32-bit QuickDraw is available */


/*************************** Local Prototypes *****************************/
void write_preferences(void);
void handle_preferences_selection(short item_hit);
void show_preferences_dialog(void);
void write_preferences(void);
void get_preferences(void);
void update_preferences_handle(void);
void set_default_preferences(void);
void place_window (WindowPtr window, Rect *bounds);
void remember_window_position(WindowPtr window, Rect *dest_rect);
pascal void dim_relinquish_control (WindowPtr the_window, short item_number);
pascal void dim_keep_image_in_memory (WindowPtr the_window, short item_number);
pascal void dim_show_image_rendering (WindowPtr the_window, short item_number);
pascal void dim_textures_radio_buttons(WindowPtr the_window, short item_number);
void init_prefs_dialog(void);


/*************************** External Prototypes *****************************/
extern UserItemUPP draw_button_border_upp;
void terminal_startup_error(short error_code);
short make_working_directory(short vrefnum, long dir_id);
void setup_prefs_controls(void);
void update_prefs_dialog_from_prefs(void);
void update_prefs_from_prefs_dialog(void);
void set_dialog_text_to_int(DialogPtr theDialog, short theItem, long value);
long get_edittext_as_int(DialogPtr theDialog, int theItem);
void handle_checkbox_click(ControlHandle check_box);
Boolean is_valid_param_int(DialogPtr dialog, short text_item_id, short label_item_id,
							long min, long max);
void remove_get_event_vbl(void);
void install_get_event_vbl(void);
void dispose_offscreen_port (void);
void show_image_window(Boolean bring_to_front);
void hide_image_window(void);
void handle_prefs_checkbox_click(ControlHandle check_box);
void setup_prefs_useritems(void);
long get_edittext_as_int(DialogPtr theDialog, int theItem);



/*****************************************************************************\
* procedure init_prefs_dialog                                                 *
*                                                                             *
* Purpose: This procedure initializes the preferences dialog.                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 23, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_prefs_dialog(void)
{

	/* Read the dialog in from the resource file */
	preferences_dialog = GetNewDialog (PREFERENCES_DLG, &preferences_dialog_rec, (WindowPtr)-1L);

}	/* init_prefs_dialog() */



/*****************************************************************************\
* procedure get_preferences                                                   *
*                                                                             *
* Purpose: This procedure gets the preferences from the Preferences folder in *
*          the System Folder of the startup disk.  If we are dealing with a   *
*          pre-System 7 system which does not have such a folder, we just get *
*          the preferences from the blessed folder.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/10/92          Added support for pre-System 7 systems.     *
\*****************************************************************************/

void get_preferences(void)
{


	long	prefs_hard_dirid;
	long	num_bytes;
	short	prefs_folder_wd_id;
	short	prefs_vrefnum;
	short	error;
	
	preferences_struct *prefs_ptr;
	
	/* allocate the preferences structure */
	prefs = (preferences_struct **) NewHandle(sizeof(preferences_struct));

	/* If we have FindFolder, use it.  Otherwise, use the blessed folder */
	if (find_folder_available)
		{
	
		/* Find the Preferences folder */
		error = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
							&prefs_vrefnum, &prefs_hard_dirid);
		if (error) terminal_startup_error(error);
	
		/* Make it a working directory */
		prefs_folder_wd_id = make_working_directory(prefs_vrefnum, prefs_hard_dirid);
	
		}
	else

		/* use the blessed folder */
		prefs_folder_wd_id = blessed_folder_wd_id;


	/* Open "MacRTrace Preferences" file */
	error = FSOpen(rtrace_prefs_filename, prefs_folder_wd_id,
					&preferences_file_refnum);
	
	/* If the file does not exist, create it */
	if (error == fnfErr)

		{
			
		/* Create and open the file */
		error = FSDelete (rtrace_prefs_filename, prefs_folder_wd_id);
		error = Create (rtrace_prefs_filename, prefs_folder_wd_id, 'RTRC', 'PREF');
		if (error) terminal_startup_error(error);
		error = FSOpen (rtrace_prefs_filename, prefs_folder_wd_id, &preferences_file_refnum);
		if (error) terminal_startup_error(error);
	
		/* Place all windows in their default locations */
		place_window(log_window, (Rect *) NULL);
		place_window(options_dialog, (Rect *) NULL);
		place_window(status_dialog, (Rect *) NULL);
		place_window(image_window, (Rect *) NULL);
		place_window(animation_dialog, (Rect *) NULL);
	
		/* Set the default preferences */
		set_default_preferences();

		/* Set the preferences structure according to the current preferences */
		write_preferences();
		
		}
	else if (error) terminal_startup_error(error);

		
	/* the file exists-- read the resources into memory */
		
	/* Read the preferences structure */
	SetFPos(preferences_file_refnum, 1, 0);
	num_bytes = sizeof(preferences_struct);
	FSRead(preferences_file_refnum, &num_bytes, *prefs);
	
	/* lock the handle and dereference it */
	HLock((Handle) prefs);
	prefs_ptr = *prefs;
	
	/* Check the preferences version */
	if (prefs_ptr->prefs_version != CURRENT_PREFS_VERSION)
		{
		
		/* This an older preferences version-- delete the file and build
			a new one from defaults */
		error = FSClose (preferences_file_refnum);
		if (error) terminal_startup_error(error);
		error = FSDelete (rtrace_prefs_filename, prefs_folder_wd_id);
		if (error) terminal_startup_error(error);
		
		/* Get rid of handle, since we'll make a new one when we call ourselves */
		DisposHandle((Handle) prefs);
		
		/* Call ourselves-- this will create a new preferences file */
		get_preferences();
		return;
		
		}
	
	/* Place the windows according to the preferences structure */
	place_window (log_window, &(prefs_ptr->log_window_position));
	place_window (options_dialog, &(prefs_ptr->options_window_position));
	place_window (status_dialog, &(prefs_ptr->status_window_position));
	if (f8bit_QD_available)
		place_window (image_window, &(prefs_ptr->image_window_position));
	place_window (animation_dialog, &(prefs_ptr->animation_window_position));

	/* get rid of the handle */
	HUnlock((Handle) prefs);

}	/* get_preferences() */



/*****************************************************************************\
* procedure place_window                                                      *
*                                                                             *
* Purpose: This procedure moves and resizes a window to fit the passed Rect.  *
*          It ensure that the window has a dragable portion of its title bar  *
*          visible-- if not, it moves it to to main monitor.                  *
*                                                                             *
* Parameters: window: the window to move and resize                           *
*             bounds: the rectangle to move the window into.                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void place_window (WindowPtr window, Rect *bounds)
{

	Rect		title_bar;		/* the rectangle enclosing the title bar of the window */
	Rect		intersection;	/* used to test rectangle intersections */
	Rect		first_bounds;	/* the boundary of the main graphics device */
	short		window_height;	/* the desired height of the window */
	short		window_width;	/* the desired width of the window */
	GDHandle	first_device;	/* the first graphics device on the the list */
	GDHandle	graphics_device;/* a graphics device on the the list */
	Boolean		enough_intersection = FALSE;
	Rect		rect_storage;
	WStateData	**wstate_handle;
	
	/* Find the first graphics device, and its bounding rectangle */
	first_device = graphics_device = GetDeviceList ();
	BlockMove(&((*first_device)->gdRect), &first_bounds, 8);

	/* If the rectangle is null, or the pointer is null it means the window
		was never shown before its position was saved, so we need to compute
		a new position. */
	if (((!bounds->right) && (!bounds->left) && (!bounds->top) && (!bounds->bottom)) ||
			!bounds)
		{

		/* Find the bounds of the window */
		bounds = &rect_storage;
		BlockMove (&(window->portRect), bounds, 8);

		/* Find the window's height and width */
		window_height = bounds->bottom - bounds->top;
		window_width = bounds->right - bounds->left;


		if (window == log_window)
						
			/* Place the window in the upper left corner */
			OffsetRect(bounds, first_bounds.left + 4, first_bounds.top + 40);

		else if (window == options_dialog)			
			
			/* Place the window at the left, right below log window */
			OffsetRect(bounds, first_bounds.left + 4, log_window->portRect.bottom + 63);
		
		else if (window == status_dialog)
			
			/* Place the window at the top, to the right of the log window */
			OffsetRect(bounds, log_window->portRect.right + 10, first_bounds.top + 40);

		else if (window == image_window)
			
			/* Place the window below the status dialog and to the right of
				the options dialog */
			OffsetRect(bounds, options_dialog->portRect.right + 10,
						status_dialog->portRect.bottom + 63);

		else if (window == animation_dialog)
			
			/* Center the window on the main screen */
			OffsetRect(bounds, (first_bounds.right + first_bounds.left)/2 - (window_width/2),
						(first_bounds.bottom + first_bounds.top)/2 - (window_height/2));

		else 	/* any other window is centered on the screen */
					
			/* Place the window in the "alert" position, in the upper center of the
				main screen */
			OffsetRect(bounds, (first_bounds.right + first_bounds.left)/2 - (window_width/2),
						(first_bounds.bottom + first_bounds.top)/3 - (window_height/2));
		
		/* Now use this new rectangle as a first approximation to place the window */
		place_window(window, bounds);	
		return;
		
		} 

	/* Find the window's height and width */
	window_height = bounds->bottom - bounds->top;
	window_width = bounds->right - bounds->left;
		
	/* Find the window's title bar */
	BlockMove(bounds, &title_bar, 8);
	title_bar.bottom = title_bar.top;
	title_bar.top -= 16;
	
	/* Scan through all the devices.  If there is any one device which has more than
		a 4x4 block of the title bar in it, we're okay. */
	do
		{
		SectRect (&title_bar, &((*graphics_device)->gdRect), &intersection);
		enough_intersection = (((intersection.bottom - intersection.top) >= 4) &&
								((intersection.right - intersection.left) >= 4));

		graphics_device = (GDHandle) (*graphics_device)->gdNextGD;
		}
	while (graphics_device && (!enough_intersection));

	/* If no device has enough of an intersection, move this to the main window.
		We try to place it as close as possible to its original place, but we
		ensure that it's all on the main window (if possible). */
	if (!enough_intersection)
		{
		
		/* If the window is above the screen, pin it to the top */
		if ((title_bar.bottom - 4) <= first_bounds.top)
			{
			bounds->top = first_bounds.top + 41;
			bounds->bottom = bounds->top + window_height;
			}
		
		/* If the window is below screen, pin it to the bottom */
		if ((title_bar.top + 4) >= first_bounds.bottom)
			{
			bounds->bottom = first_bounds.bottom - 5;
			bounds->top = bounds->bottom - window_height;
			}

		/* If the window is to the left of the screen, pin it to the left */
		if ((title_bar.right - 4) < first_bounds.left)
			{
			bounds->left = first_bounds.left + 5;
			bounds->right = bounds->left + window_width;
			}
		
		/* If the window is to the right of the screen, pin it to the right */
		if ((title_bar.left + 4) >= first_bounds.right)
			{
			bounds->right = first_bounds.right - 8;
			bounds->left = bounds->right - window_width;
			}
		}

	/* Save the new location in the window record */
	wstate_handle = (WStateData **) ( (WindowPeek) window )->dataHandle;
	BlockMove(bounds, &(*wstate_handle)->userState, 8);

	/* Move the window to the new location */
	MoveWindow (window, bounds->left, bounds->top, TRUE);
	
	/* Resize the window */
	SizeWindow (window, window_width, window_height, TRUE);
	
}	/* place_window() */



/*****************************************************************************\
* procedure write_preferences                                                 *
*                                                                             *
* Purpose: This procedure writes the current preferences to the Preferences   *
*          Folder in the System Folder of the startup disk.                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void write_preferences(void)
{

	long	num_bytes;
	short	error;

	/* Set the preferences version */
	(*prefs)->prefs_version = CURRENT_PREFS_VERSION;

	/* Save the window positions */
	remember_window_position(options_dialog, &((*prefs)->options_window_position));
	remember_window_position(log_window, &((*prefs)->log_window_position));
	remember_window_position(status_dialog, &((*prefs)->status_window_position));
	remember_window_position(image_window, &((*prefs)->image_window_position));
	remember_window_position(animation_dialog, &((*prefs)->animation_window_position));

	/* Write the preferences to the resource file */
	num_bytes = sizeof(preferences_struct);
	error = SetFPos(preferences_file_refnum, 1, 0);
	if (error) terminal_startup_error(error);
	error = FSWrite(preferences_file_refnum, &num_bytes, *prefs);
	if (error) terminal_startup_error(error);

}	/* write_preferences() */



/*****************************************************************************\
* procedure set_default_preferences                                           *
*                                                                             *
* Purpose: This procedure sets the preferences to their default values.  Only *
*          those preferences which are set from the preferences dialog are    *
*          changed; things like window positions are not.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void set_default_preferences(void)
{

	preferences_struct *prefs_ptr;

	/* lock the handle and dereference it */
	HLock((Handle) prefs);
	prefs_ptr = *prefs;
	
	/* Set preferences to default values */
	prefs_ptr->show_image_rendering = TRUE;
	prefs_ptr->allow_background_tasks = TRUE;
	prefs_ptr->show_status_window_flag = TRUE;
	prefs_ptr->show_about_window_flag = TRUE;
	prefs_ptr->hide_options_window_flag = FALSE;
	prefs_ptr->time_between_events = 30;
	prefs_ptr->texture_mode = TEXTURE_MODE_DEFAULT;
	prefs_ptr->max_objects = MAX_OBJECTS_DEFAULT;
	prefs_ptr->max_lights = MAX_LIGHTS_DEFAULT;
	prefs_ptr->max_surfaces = MAX_SURFACES_DEFAULT;
	prefs_ptr->max_csg_levels = MAX_CSG_LEVELS_DEFAULT;

	/* Unlock the handle */
	HUnlock((Handle) prefs);

}	/* set_default_preferences() */



/*****************************************************************************\
* procedure remember_window_position                                          *
*                                                                             *
* Purpose: This procedure copies the current position of a window into a      *
*          rectangle structure, to save for later restoration.                *
*                                                                             *
* Parameters: window: the window whose position we are saving                 *
*             dest_rect: the rectangle to save it in                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void remember_window_position(WindowPtr window, Rect *dest_rect)
{

	WStateData **wstate_handle;

	/* If this is the image window, and if we're running in 1-bit QD, don't
		even try to save it-- it doesn't exist */
	if ((window == image_window) && (!f8bit_QD_available))
		
		dest_rect->left = dest_rect->right = dest_rect->top = dest_rect->bottom = 0;

	else
		{

		/* Get the Window State data */
		wstate_handle = (WStateData **) ( (WindowPeek) window )->dataHandle;

		/* Copy it to the rectangle */
		BlockMove(&(*wstate_handle)->userState, dest_rect, 8);

		}

}	/* remember_window_position() */



/*****************************************************************************\
* procedure setup_prefs_controls                                              *
*                                                                             *
* Purpose: This procedure sets up the controls in the preferences dialog.     *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void setup_prefs_controls(void)
{

	short	type;		/* the item type of a DITL item */
	Rect	box;		/* the bounding box of a DITL item */

	/* Read all the controls from the resources fork */
	GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_CHECKBOX, &type,
				(Handle *) &allow_background_tasks_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_IMAGE_RENDERING_CHECKBOX, &type,
				(Handle *) &show_image_rendering_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_STATUS_WINDOW_CHECKBOX, &type,
				(Handle *) &show_status_window_checkbox, &box);
	GetDItem (preferences_dialog, SHOW_ABOUT_WINDOW_CHECKBOX, &type,
				(Handle *) &show_about_window_checkbox, &box);
	GetDItem (preferences_dialog, HIDE_OPTIONS_WINDOW_CHECKBOX, &type,
				(Handle *) &hide_options_window_checkbox, &box);
	GetDItem (preferences_dialog, USE_TEXTURES_CHECKBOX, &type,
				(Handle *) &use_textures_checkbox, &box);
	GetDItem (preferences_dialog, TEXTURES_IN_OBJECTS_RADIO, &type,
				(Handle *) &textures_in_objects_radio, &box);
	GetDItem (preferences_dialog, TEXTURES_AFTER_OBJECTS_RADIO, &type,
				(Handle *) &textures_after_objects_radio, &box);
	GetDItem (preferences_dialog, OKAY_BUTTON, &type,
				(Handle *) &preferences_okay_button_handle, &box);
	GetDItem (preferences_dialog, PREFS_DEFAULTS_BUTTON, &type,
				(Handle *) &preferences_defaults_button_handle, &box);
	GetDItem (preferences_dialog, PREFS_REVERT_BUTTON, &type,
				(Handle *) &preferences_revert_button_handle, &box);

}	/* setup_prefs_check_boxes() */



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

	/* Copy the current prefs so we can revert if the user chooses to cancel */
	if (original_prefs)
		DisposeHandle((Handle) original_prefs);
	original_prefs = prefs;
	HandToHand((Handle *) &original_prefs);

	/* If events are disabled, select the dummy edittext item */
	if (!(*prefs)->allow_background_tasks)
		SelIText (preferences_dialog, DUMMY_EDITTEXT, 0, 32767);

	/* Set up the preferences dialog to reflect the preferences */
	update_prefs_dialog_from_prefs();

	/* Bring the preferences window to the front */
	ShowWindow (preferences_dialog);
	SelectWindow (preferences_dialog);

}	/* show_preferences_dialog() */



/*****************************************************************************\
* procedure update_prefs_dialog_from_prefs                                    *
*                                                                             *
* Purpose: This procedure changes the settings in the preferences dialog to   *
*          match the preferences.                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void update_prefs_dialog_from_prefs(void)
{

	Str255 string;
	short	type;
	Handle	handle;
	Rect	box;
	int		textures_hilite_value;

	/* set the checkboxes according the preferences */
	SetCtlValue (show_image_rendering_checkbox, (*prefs)->show_image_rendering);
	SetCtlValue (allow_background_tasks_checkbox, (*prefs)->allow_background_tasks);
	SetCtlValue (show_status_window_checkbox, (*prefs)->show_status_window_flag);
	SetCtlValue (hide_options_window_checkbox, (*prefs)->hide_options_window_flag);
	SetCtlValue (show_about_window_checkbox, (*prefs)->show_about_window_flag);
	SetCtlValue (use_textures_checkbox, ((*prefs)->texture_mode != 0));
	SetCtlValue (textures_in_objects_radio, ((*prefs)->texture_mode == 1));
	SetCtlValue (textures_after_objects_radio, ((*prefs)->texture_mode != 1));

	/* Dim the textures radio controls if necessary */
	textures_hilite_value = GetCtlValue(use_textures_checkbox) ? 0 : 255;
	HiliteControl(textures_in_objects_radio, textures_hilite_value);
	HiliteControl(textures_after_objects_radio, textures_hilite_value);
	
	/* Set the relinquish control text according to the preferences */
	NumToString ((*prefs)->time_between_events, string);
	GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &type, &handle, &box);
	SetIText (handle, string);
	
	/* Set the max values */
	set_dialog_text_to_int(preferences_dialog, MAX_OBJECTS_NUM, (*prefs)->max_objects);
	set_dialog_text_to_int(preferences_dialog, MAX_LIGHTS_NUM, (*prefs)->max_lights);
	set_dialog_text_to_int(preferences_dialog, MAX_SURFACES_NUM, (*prefs)->max_surfaces);
	set_dialog_text_to_int(preferences_dialog, MAX_CSG_LEVELS_NUM, (*prefs)->max_csg_levels);

}	/* update_prefs_dialog_from_prefs() */



/*****************************************************************************\
* procedure update_prefs_from_prefs_dialog                                    *
*                                                                             *
* Purpose: This procedure changes the preferences to match the settings in    *
*          the preferences dialog.                                            *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void update_prefs_from_prefs_dialog(void)
{

	/* Get the new values of those prefs which affect how a scene is read */
	long max_objects = get_edittext_as_int(preferences_dialog, MAX_OBJECTS_NUM);
	long max_lights = get_edittext_as_int(preferences_dialog, MAX_LIGHTS_NUM);
	long max_surfaces = get_edittext_as_int(preferences_dialog, MAX_SURFACES_NUM);
	long max_csg_levels = get_edittext_as_int(preferences_dialog, MAX_CSG_LEVELS_NUM);
	char texture_mode;
	if (GetCtlValue(use_textures_checkbox))
		texture_mode = GetCtlValue(textures_in_objects_radio) ? 1 : 2;
	else
		texture_mode = 0;
	
	/* Get the number of ticks between events */
	(*prefs)->time_between_events =
		get_edittext_as_int(preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM);
	
	/* If any of these prefs have changed, we will need to read the scene in again */
	if ((max_objects != (*prefs)->max_objects) ||
		(max_lights != (*prefs)->max_lights) ||
		(max_surfaces != (*prefs)->max_surfaces) ||
		(max_csg_levels != (*prefs)->max_csg_levels) ||
		(texture_mode != (*prefs)->texture_mode))
		reread_scene = TRUE;

	/* Update these prefs */
	(*prefs)->max_objects = max_objects;
	(*prefs)->max_lights = max_lights;
	(*prefs)->max_surfaces = max_surfaces;
	(*prefs)->max_csg_levels = max_csg_levels;
	(*prefs)->texture_mode = texture_mode;

	/* Update the rest of the prefs from the check boxes */
	(*prefs)->show_image_rendering = GetCtlValue(show_image_rendering_checkbox);
	(*prefs)->allow_background_tasks = GetCtlValue (allow_background_tasks_checkbox);
	(*prefs)->show_status_window_flag = GetCtlValue (show_status_window_checkbox);
	(*prefs)->hide_options_window_flag = GetCtlValue (hide_options_window_checkbox);
	(*prefs)->show_about_window_flag = GetCtlValue (show_about_window_checkbox);

}	/* update_prefs_from_prefs_dialog() */



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
		
		case SHOW_STATUS_WINDOW_CHECKBOX:
			handle_checkbox_click (show_status_window_checkbox);
			break;			
		
		case HIDE_OPTIONS_WINDOW_CHECKBOX:
			handle_checkbox_click (hide_options_window_checkbox);
			break;			
		
		case SHOW_ABOUT_WINDOW_CHECKBOX:
			handle_checkbox_click (show_about_window_checkbox);
			break;			
		
		case USE_TEXTURES_CHECKBOX:
			handle_checkbox_click (use_textures_checkbox);
			break;			
		
		case TEXTURES_IN_OBJECTS_RADIO:
			SetCtlValue(textures_in_objects_radio, TRUE);
			SetCtlValue(textures_after_objects_radio, FALSE);
			break;			
		
		case TEXTURES_AFTER_OBJECTS_RADIO:
			SetCtlValue(textures_after_objects_radio, TRUE);
			SetCtlValue(textures_in_objects_radio, FALSE);
			break;			
		
		case OKAY_BUTTON:

			/* Don't let user close window until the ticks field is valid */
			if ((GetCtlValue(allow_background_tasks_checkbox)
				&& !is_valid_param_int(preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
										ALLOW_BACKGROUND_TASKS_LABEL, 1, INT_MAX))
				||
	
				/* Don't let user close window until the max objects field is valid */
				(!is_valid_param_int(preferences_dialog, MAX_OBJECTS_NUM,
										MAX_OBJECTS_LABEL, 1, INT_MAX))

				||
			
				/* Don't let user close window until the max lights field is valid */
				(!is_valid_param_int(preferences_dialog, MAX_LIGHTS_NUM,
										MAX_LIGHTS_LABEL, 1, INT_MAX))

				||
							
				/* Don't let user close window until the max surfaces field is valid */
				(!is_valid_param_int(preferences_dialog, MAX_SURFACES_NUM,
										MAX_SURFACES_LABEL, 1, INT_MAX))

				||
			
				/* Don't let user close window until the max csg levels field is valid */
				(!is_valid_param_int(preferences_dialog, MAX_CSG_LEVELS_NUM,
										MAX_CSG_LEVELS_LABEL, 1, INT_MAX)))
				
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
				
				/* Update the prefs from the settings in the window */
				update_prefs_from_prefs_dialog();
				
				/* If we are rendering, reinstall the VBL task at this new frequency */
				if (rendering)
					{
					
					/* If there is a VBL, get rid of it */
					if (vbl_installed)
						remove_get_event_vbl();
			
					/* If there should be a VBL now, install it */
					if ((*prefs)->allow_background_tasks)
						install_get_event_vbl();
					
					}
				
				/* If there should no longer be an image window, hide it. */
				if (((*prefs)->show_image_rendering) && (!GetCtlValue(show_image_rendering_checkbox)))
					hide_image_window();
				if ((!(*prefs)->show_image_rendering) && (GetCtlValue(show_image_rendering_checkbox)))
					show_image_window(FALSE);
				
				/* Write the preferences to the preferences file */
				write_preferences();
				
				}
			
			/* If we're faking a button click, dehilight it */
			HiliteControl(preferences_okay_button_handle, 0);
	
			break;

		case PREFS_DEFAULTS_BUTTON:
		
			/* Set preferences to their default values */
			set_default_preferences();
			
			/* Update the window to reflect the preferences */
			update_prefs_dialog_from_prefs();

			break;

		case PREFS_REVERT_BUTTON:
		
			/* Copy the data back from the original prefs, reverting */
			DisposeHandle((Handle) prefs);
			prefs = original_prefs;
			HandToHand((Handle *) &prefs);

			/* Update the window to reflect the preferences */
			update_prefs_dialog_from_prefs();

			break;

		default:;	/* all other items take care of themselves */
		}
	
}	/* handle_preferences_selection() */



/*****************************************************************************\
* procedure handle_prefs_checkbox_click                                       *
*                                                                             *
* Purpose: This procedure handles a click in a checkbox.  Note that           *
*          handle_checkbox_click does the standard stuff (toggling, etc);     *
*          this just handles special cases.                                   *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void handle_prefs_checkbox_click(ControlHandle check_box)
{

	short	type;
	Handle	handle;
	Rect	rect;
	int		textures_hilite_value;
	
	/* If this is the use textures check box, set things up so that
		the textures radio buttons and the dimming useritem get refreshed. */
	if (check_box == use_textures_checkbox)
		{
		
		/* Dim or undim the texture radio controls */
		textures_hilite_value = GetCtlValue(use_textures_checkbox) ? 0 : 255;
		HiliteControl(textures_in_objects_radio, textures_hilite_value);
		HiliteControl(textures_after_objects_radio, textures_hilite_value);

		}
	
	/* If the user has changed whether background tasks will be allowed,
		dim or undim the ticks item */
	else if (check_box == allow_background_tasks_checkbox)
		{
		
		/* Get value of the checkbox */
		if (GetCtlValue (allow_background_tasks_checkbox))	
			{
			
			/* Checkbox is on; select the text */
			SelIText (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, 0, 32767);

			}
			
		else
			{
			
			/* Checkbox is off; select something else */
			SelIText (preferences_dialog, DUMMY_EDITTEXT, 0, 32767);

			}

		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &type,
					&handle, &rect);
					
		/* extend it to the left to encompass the text */
		rect.left -= 300;
		
		/* make the whole area invalid so it gets updated later */
		SetPort (preferences_dialog);
		InvalRect (&rect);
		
		}

}	/* handle_prefs_checkbox_click() */



/*****************************************************************************\
* procedure setup_prefs_useritems                                             *
*                                                                             *
* Purpose: This procedure sets up the UserItems in the preferences dialog.    *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void setup_prefs_useritems(void)
{

	short	type;
	Handle	handle;
	Rect	rect;
	
	/* First, initialize our UPPs */
	dim_relinquish_control_upp = NewUserItemProc(dim_relinquish_control);
	dim_show_image_rendering_upp = NewUserItemProc(dim_show_image_rendering);

	/* Set up the border around the default button in the preferences dialog */
	GetDItem (preferences_dialog, PREF_BUTTON_BORDER_USERITEM,
				&type, &handle, &rect);
	SetDItem (preferences_dialog, PREF_BUTTON_BORDER_USERITEM,
				type, (Handle) draw_button_border_upp, &rect);

	/* Set up the dimming of the relinquish control items */
	GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &type,
				&handle, &rect);
	SetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, userItem,
				(Handle) dim_relinquish_control_upp, &rect);

	/* Set up the dimming of the Show Image Rendering checkbox */
	GetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, &type,
				&handle, &rect);
	SetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, userItem,
				(Handle) dim_show_image_rendering_upp, &rect);

}	/* setup_prefs_useritems() */



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

	short	type;		/* the item type of a DITL item */
	Handle	handle;	/* a handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */


	/* Get the value of the Allow Background Tasks check box; if it is checked,
		enable the item.  If it is not checked, disable it and grey it out. */
	if (GetCtlValue (allow_background_tasks_checkbox))	
		{
		
		/* Enable the EditText item */
		GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &type,
					&handle, &box);
		SetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
					editText, handle, &box);
		
		/* Draw the frame */
		InsetRect (&box, -3, -3);
		FrameRect (&box);
		
		}
	else		
		{
	
		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_ALLOW_BACKGROUND_TASKS_USERITEM, &type,
					&handle, &box);
					
		/* extend it to the left to encompass the text */
		box.left -= 300;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(&qd.gray);
		PaintRect (&box);
		PenMode(patCopy);
		
		/* Disable the EditText item */
		GetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM, &type,
					&handle, &box);
		SetDItem (preferences_dialog, ALLOW_BACKGROUND_TASKS_NUM,
					(statText | itemDisable), handle, &box);
		
		/* Draw the frame in gray */
		InsetRect (&box, -3, -3);
		FrameRect (&box);
		PenPat(&qd.black);

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

	short	type;		/* the item type of a DITL item */
	Handle	handle;	/* a handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */


	/* If Color QuickDraw is not available, dim the Show Image Rendering checkbox */
	if (!f8bit_QD_available)
		{
				
		/* Get the useritem rectangle */
		GetDItem (preferences_dialog, DIM_SHOW_IMAGE_RENDERING_USERITEM, &type,
					&handle, &box);

		/* extend it to the left to encompass the checkbox */
		box.left -= 300;

		/* Set the pen mode to Bic, draw over the check boxes in gray */
		PenMode(patBic);
		PenPat(&qd.gray);
		PaintRect (&box);
		PenPat(&qd.black);
		PenMode(patCopy);
		
		/* Disable the checkbox */
		HiliteControl(show_image_rendering_checkbox, 255);
		SetCtlValue(show_image_rendering_checkbox, FALSE);
		
		}

}	/* dim_show_image_rendering() */



