/*****************************************************************************\
* macoptions.c                                                                *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It implements  *
* a the interface to the Options dialog box, which allows the user to set     *
* the rendering options.                                                      *
\*****************************************************************************/

#include <limits.h>
#include <float.h>

#include "defs.h"

#include "macresources.h"
#include "macdefaults.h"
#include "mactypes.h"



/* definition of the callback routine which the Dialog Manager uses
   when it calls a user item drawing routine */

typedef pascal void (*UserItemProc) (WindowPtr, short);


/*************************** Globals *****************************/
DialogPtr		options_dialog;			/* the options dialog */
DialogRecord	options_dialog_rec;		/* storage for the options dialog */

ControlHandle	render_button_handle;			/* handle to the Render button */
MenuHandle	antialiasing_menu;		/* Handles to the MENU resources of */
MenuHandle	translucence_menu;		/* the popup menus in the options   */
MenuHandle	shading_menu;			/* dialog.                          */
MenuHandle	view_menu;
MenuHandle	stereo_separation_menu;
MenuHandle	sampling_level_menu;
MenuHandle	walk_mode_menu;
MenuHandle	backface_menu;
MenuHandle	aliasing_threshold_menu;
MenuHandle	shading_threshold_menu;
MenuHandle	ambient_threshold_menu;

short		antialiasing_menu_selection = 1;	/* current selections in  */
short		translucence_menu_selection = 1;	/* the popup menus in the */
short		shading_menu_selection = 2;			/* options dialog.        */
short		view_menu_selection = 1;
short		stereo_separation_menu_selection = 1;
short		sampling_level_menu_selection = 4;
short		walk_mode_menu_selection = 2;
short		backface_menu_selection = 1;
short		aliasing_threshold_menu_selection = 3;
short		shading_threshold_menu_selection = 5;
short		ambient_threshold_menu_selection = 7;

Point		antialiasing_menu_upper_left;	/* Upper left corner of the   */
Point		translucence_menu_upper_left;	/* popup menus in the options */
Point		shading_menu_upper_left;		/* dialog (global coords).    */
Point		view_menu_upper_left;
Point		stereo_separation_menu_upper_left;
Point		sampling_level_menu_upper_left;
Point		walk_mode_menu_upper_left;
Point		backface_menu_upper_left;
Point		aliasing_threshold_menu_upper_left;
Point		shading_threshold_menu_upper_left;
Point		ambient_threshold_menu_upper_left;

ControlHandle	intersect_adjust_checkbox;		/* handles to the checkbox controls */
ControlHandle	use_jittered_sample_checkbox;
ControlHandle	correct_texture_normals_checkbox;
ControlHandle	animate_checkbox;
ControlHandle	intersect_all_objects_checkbox;
ControlHandle	only_correct_inside_checkbox;
ControlHandle	specify_focal_distance_checkbox;

UserItemUPP dim_stereo_separation_upp;
UserItemUPP dim_focal_distance_upp;
UserItemUPP draw_antialiasing_popup_upp;
UserItemUPP draw_translucence_popup_upp;
UserItemUPP draw_shading_popup_upp;
UserItemUPP draw_backface_popup_upp;
UserItemUPP draw_aliasing_threshold_popup_upp;
UserItemUPP draw_shading_threshold_popup_upp;
UserItemUPP draw_ambient_threshold_popup_upp;
UserItemUPP draw_walk_mode_popup_upp;
UserItemUPP draw_view_popup_upp;
UserItemUPP draw_stereo_separation_popup_upp;
UserItemUPP draw_sampling_level_popup_upp;


/*************************** Externals *****************************/
extern MenuHandle	windows_menu;		/* the Windows pulldown menu */
extern long			image_width;		/* size of the image to be generated */
extern long			image_height;
extern preferences_handle prefs;		/* The preferences */
extern Boolean	rendering;				/* TRUE while we are rendering */
extern Boolean	reading_scene;			/* TRUE while we are reading the scene */
extern Boolean	render_next;			/* TRUE if we should render as soon as the scene is read */
extern MenuHandle	file_menu;			/* The File menu */



/*************************** Local Prototypes *****************************/
void init_options_dialog(void);
void load_options_popup_menus(void);
void show_options_dialog(Boolean bring_to_front);
void hide_options_dialog(void);
void handle_options_selection(short item_hit);
void	reset_defaults(void);
void	add_parameter(params_struct *params, char *param);
Boolean are_valid_params(void);
void setup_options_controls(void);
void setup_options_useritems(void);
pascal void dim_stereo_separation (WindowPtr the_window, short item_number);
pascal void dim_focal_distance (WindowPtr the_window, short item_number);
void handle_options_popup_click(short menu_id, short old_selection, short new_selection);
void handle_options_checkbox_click(ControlHandle check_box);
pascal void draw_antialiasing_popup (WindowPtr the_window, short item_number);
pascal void draw_translucence_popup (WindowPtr the_window, short item_number);
pascal void draw_intersect_popup (WindowPtr the_window, short item_number);
pascal void draw_shading_popup (WindowPtr the_window, short item_number);
pascal void draw_backface_popup (WindowPtr the_window, short item_number);
pascal void draw_aliasing_threshold_popup (WindowPtr the_window, short item_number);
pascal void draw_shading_threshold_popup (WindowPtr the_window, short item_number);
pascal void draw_ambient_threshold_popup (WindowPtr the_window, short item_number);
pascal void draw_walk_mode_popup(WindowPtr the_window, short item_number);
pascal void draw_view_popup (WindowPtr the_window, short item_number);
pascal void draw_stereo_separation_popup (WindowPtr the_window, short item_number);
pascal void draw_sampling_level_popup (WindowPtr the_window, short item_number);
void	generate_params(params_struct *params);


/*************************** External Prototypes *****************************/
extern UserItemUPP draw_button_border_upp;
void install_popup_menu_hook(MenuHandle popup_menu);
void handle_popup_click(DialogPtr the_dialog, MenuHandle popup_menu, short menu_id,
						short popup_text_id, short popup_useritem_id,
						Point menu_upper_left, short *menu_selection);
void set_dialog_text_to_real(DialogPtr theDialog, short theItem, double value);
double get_edittext_as_real(DialogPtr theDialog, int theItem);
long get_edittext_as_int(DialogPtr theDialog, int theItem);
void handle_checkbox_click(ControlHandle check_box);
void show_animation_dialog(Boolean bring_to_front);
void render_scene(void);
Boolean is_valid_param_int(DialogPtr dialog, short text_item_id, short label_item_id,
							long min, long max);
Boolean is_valid_param_real(DialogPtr dialog, short text_item_id, short label_item_id,
							real min, real max, Boolean include_min, Boolean include_max);
void setup_popup_menu(DialogPtr theDialog, short useritem_id, UserItemUPP proc);
void draw_any_popup (DialogPtr the_dialog, short item_number, MenuHandle menu,
							short menu_selection, Point *upper_left);
void set_render_button_name(void);



/*****************************************************************************\
* procedure init_options_dialog                                               *
*                                                                             *
* Purpose: This procedure initializes the options dialog.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 23, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void init_options_dialog(void)
{

	/* Read the dialog in from the resource file */
	options_dialog = GetNewDialog (OPTIONS_DLG, &options_dialog_rec, (WindowPtr)-1L);
	
	/* Select the first EditText item in the options dialog box */
	SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);

}	/* init_options_dialog() */



/*****************************************************************************\
* procedure load_options_popup_menus                                          *
*                                                                             *
* Purpose: This procedure loads the popup menus from the resource file and    *
*          installs hooks in the MenuProcs so that they will be wide enough,  *
*          when popped up, to accommodate the down arrow.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void load_options_popup_menus(void)
{

	antialiasing_menu = GetMenu (ANTIALIASING_MENU);
	install_popup_menu_hook(antialiasing_menu);
	
	translucence_menu = GetMenu (TRANSLUCENCE_MENU);
	install_popup_menu_hook(translucence_menu);
	
	walk_mode_menu = GetMenu (WALK_MODE_MENU);
	install_popup_menu_hook(walk_mode_menu);
	
	backface_menu = GetMenu (BACKFACE_MENU);
	install_popup_menu_hook(backface_menu);

	aliasing_threshold_menu = GetMenu (ALIASING_THRESHOLD_MENU);
	install_popup_menu_hook(aliasing_threshold_menu);
	
	shading_threshold_menu = GetMenu (SHADING_THRESHOLD_MENU);
	install_popup_menu_hook(shading_threshold_menu);
	
	ambient_threshold_menu = GetMenu (AMBIENT_THRESHOLD_MENU);
	install_popup_menu_hook(ambient_threshold_menu);

	shading_menu = GetMenu (SHADING_MENU);
	install_popup_menu_hook(shading_menu);
	
	view_menu = GetMenu (VIEW_MENU);
	install_popup_menu_hook(view_menu);
	
	stereo_separation_menu = GetMenu (STEREO_SEPARATION_MENU);
	install_popup_menu_hook(stereo_separation_menu);
	
	sampling_level_menu = GetMenu (SAMPLING_LEVEL_MENU);
	install_popup_menu_hook(sampling_level_menu);
	
}	/* load_options_popup_menus() */



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
	if (bring_to_front)
		SelectWindow (options_dialog);
	else
		HiliteWindow(options_dialog, options_dialog == FrontWindow());
	
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

		case TRANSLUCENCE_POPUP:
			handle_popup_click (options_dialog, translucence_menu, TRANSLUCENCE_MENU,
								TRANSLUCENCE_POPUP_TEXT, TRANSLUCENCE_POPUP,
								translucence_menu_upper_left, &translucence_menu_selection);
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

		case BACKFACE_POPUP:
			handle_popup_click (options_dialog, backface_menu,
								BACKFACE_MENU,
								BACKFACE_POPUP_TEXT, BACKFACE_POPUP,
								backface_menu_upper_left,
								&backface_menu_selection);
			break;

		case WALK_MODE_POPUP:
			handle_popup_click (options_dialog, walk_mode_menu,
								WALK_MODE_MENU,
								WALK_MODE_POPUP_TEXT, WALK_MODE_POPUP,
								walk_mode_menu_upper_left,
								&walk_mode_menu_selection);
			break;

		case ALIASING_THRESHOLD_POPUP:
			handle_popup_click (options_dialog, aliasing_threshold_menu,
								ALIASING_THRESHOLD_MENU,
								0, ALIASING_THRESHOLD_POPUP,
								aliasing_threshold_menu_upper_left,
								&aliasing_threshold_menu_selection);

			break;

		case SHADING_THRESHOLD_POPUP:
			handle_popup_click (options_dialog, shading_threshold_menu,
								SHADING_THRESHOLD_MENU,
								0, SHADING_THRESHOLD_POPUP,
								shading_threshold_menu_upper_left,
								&shading_threshold_menu_selection);

			break;

		case AMBIENT_THRESHOLD_POPUP:
			handle_popup_click (options_dialog, ambient_threshold_menu,
								AMBIENT_THRESHOLD_MENU,
								0, AMBIENT_THRESHOLD_POPUP,
								ambient_threshold_menu_upper_left,
								&ambient_threshold_menu_selection);

			break;

		/* If it was one of the checkboxes, toggle it */
		
		case INTERSECT_ADJUST_CHECKBOX:
			handle_checkbox_click (intersect_adjust_checkbox);
			break;
		
		case CORRECT_TEXTURE_NORMALS_CHECKBOX:
			handle_checkbox_click (correct_texture_normals_checkbox);
			break;
		
		case USE_JITTERED_SAMPLE_CHECKBOX:
			handle_checkbox_click (use_jittered_sample_checkbox);
			break;			
		
		case ANIMATE_CHECKBOX:
			handle_checkbox_click(animate_checkbox);
			if (GetCtlValue(animate_checkbox))		
				{
				/* Dim the Save item; we can't Save an animation; it must be
					generated on the fly */
				DisableItem(file_menu, SAVE_ITEM);
				
				}
			else
				{
				/* Animation is off; we can now save again */
				EnableItem(file_menu, SAVE_ITEM);
				
				}

				/* Set the Render Button name appropriately */
				set_render_button_name();

			break;			
		
		case INTERSECT_ALL_OBJECTS_CHECKBOX:
			handle_checkbox_click (intersect_all_objects_checkbox);
			break;			
		
		case ONLY_CORRECT_INSIDE_CHECKBOX:
			handle_checkbox_click (only_correct_inside_checkbox);
			break;			
		
		case SPECIFY_FOCAL_DISTANCE_CHECKBOX:
			handle_checkbox_click(specify_focal_distance_checkbox);
			break;			
		
		case RENDER_BUTTON:
		
			/* If we are faking a click, unclick */
			HiliteControl(render_button_handle, 0);

			/* If we're set up to render next (button reads "Don�t Render"),
			   don't render next after all */
			if (render_next)
				render_next = FALSE;
			
			/* If we're reading the scene (button reads "Render Next"),
				render after we've read the scene */
			else if (reading_scene)
				render_next = TRUE;
				
			else
				{
				/* Make sure the parameters are all valid */
				if (are_valid_params())
					render_scene();

				else	/* there was an error */
					
					/* If we're faking a button click, dehilight it */
					HiliteControl(render_button_handle, 0);
				
				}
				
			/* Set the Render Button name appropriately */
			set_render_button_name();
				
			break;
			
		case DEFAULTS_BUTTON:
			reset_defaults();
			break;

		default:;	/* all other items take care of themselves */
			
		}

}	/* handle_options_selection() */



/*****************************************************************************\
* procedure set_render_button_name                                            *
*                                                                             *
* Purpose: This procedure sets the name of the Render button according to     *
*          what's going on.                                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 25, 1995                                                *
* Modified:                                                                   *
\*****************************************************************************/

void set_render_button_name(void)
{

	if (reading_scene)
		if (render_next)
			SetCTitle(render_button_handle, "\pDon�t Render");
		else
			SetCTitle(render_button_handle, "\pRender Next");
	else if (rendering)
		SetCTitle(render_button_handle, "\pAbort & Render");
	else
		{
		if (GetCtlValue(animate_checkbox))
			SetCTitle(render_button_handle, "\pRender & Save");
		else
			SetCTitle(render_button_handle, "\pRender");
		}

}	/* set_render_button_name() */


/*****************************************************************************\
* procedure reset_defaults                                                    *
*                                                                             *
* Purpose: This procedure resets the options dialog to its default values.    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 1, 199                                                *
* Modified:                                                                   *
\*****************************************************************************/

void	reset_defaults(void)
{

	short	type;		/* the item type of a DITL item */
	Handle	handle;		/* the handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */

	/* This procedure is just a list of assignments, as each item in the
		dialog is reset to its default value */

	/* Set the EditText items to their default values */
	set_dialog_text_to_real(options_dialog, IMAGE_WIDTH_NUM, IMAGE_WIDTH_DEFAULT);
	set_dialog_text_to_real(options_dialog, IMAGE_HEIGHT_NUM, IMAGE_HEIGHT_DEFAULT);
	set_dialog_text_to_real(options_dialog, ALIASING_THRESHOLD_NUM, ALIASING_THRESHOLD_DEFAULT);
	set_dialog_text_to_real(options_dialog, SHADING_THRESHOLD_NUM, SHADING_THRESHOLD_DEFAULT);
	set_dialog_text_to_real(options_dialog, AMBIENT_THRESHOLD_NUM, AMBIENT_THRESHOLD_DEFAULT);
	set_dialog_text_to_real(options_dialog, FOCAL_APERATURE_NUM, FOCAL_APERATURE_DEFAULT);
	set_dialog_text_to_real(options_dialog, STEREO_SEPARATION_NUM, STEREO_SEPARATION_DEFAULT);
	set_dialog_text_to_real(options_dialog, CLUSTER_SIZE_NUM, CLUSTER_SIZE_DEFAULT);
	set_dialog_text_to_real(options_dialog, AMBIENT_LEVELS_NUM, AMBIENT_LEVEL_DEFAULT);
	set_dialog_text_to_real(options_dialog, AMBIENT_SAMPLES_NUM, AMBIENT_SAMPLES_DEFAULT);
	set_dialog_text_to_real(options_dialog, SHADING_LEVELS_NUM, SHADING_LEVELS_DEFAULT);
	set_dialog_text_to_real(options_dialog, FOCAL_DISTANCE_NUM, FOCAL_DISTANCE_DEFAULT);

	/* Uncheck all menus */
	CheckItem (antialiasing_menu, antialiasing_menu_selection, FALSE);
	CheckItem (translucence_menu, translucence_menu_selection, FALSE);
	CheckItem (shading_menu, shading_menu_selection, FALSE);
	CheckItem (view_menu, view_menu_selection, FALSE);
	CheckItem (stereo_separation_menu, stereo_separation_menu_selection, FALSE);
	CheckItem (sampling_level_menu, sampling_level_menu_selection, FALSE);
	CheckItem (backface_menu, backface_menu_selection, FALSE);
	CheckItem (walk_mode_menu, walk_mode_menu_selection, FALSE);
	CheckItem (aliasing_threshold_menu, aliasing_threshold_menu_selection, FALSE);
	CheckItem (shading_threshold_menu, shading_threshold_menu_selection, FALSE);
	CheckItem (ambient_threshold_menu, ambient_threshold_menu_selection, FALSE);
	
	/* Set the menus to their default settings */
	antialiasing_menu_selection = ANTIALIASING_DEFAULT + 1;
	translucence_menu_selection = TRANSLUCENCE_DEFAULT + 1;
	shading_menu_selection = SHADING_DEFAULT + 1;
	view_menu_selection = VIEW_DEFAULT + 1;
	stereo_separation_menu_selection = STEREO_SEPARATION_MENU_DEFAULT + 1;
	sampling_level_menu_selection = (4 - SAMPLING_LEVEL_DEFAULT);
	backface_menu_selection = BACKFACE_DEFAULT + 1;
	walk_mode_menu_selection = WALK_MODE_DEFAULT + 1;
	aliasing_threshold_menu_selection = 1;
	shading_threshold_menu_selection = 1;
	ambient_threshold_menu_selection = 1;

	/* Check the new selections in all menus */
	CheckItem (antialiasing_menu, antialiasing_menu_selection, TRUE);
	CheckItem (translucence_menu, translucence_menu_selection, TRUE);
	CheckItem (shading_menu, shading_menu_selection, TRUE);
	CheckItem (view_menu, view_menu_selection, TRUE);
	CheckItem (stereo_separation_menu, stereo_separation_menu_selection, TRUE);
	CheckItem (sampling_level_menu, sampling_level_menu_selection, TRUE);
	CheckItem (backface_menu, backface_menu_selection, TRUE);
	CheckItem (walk_mode_menu, walk_mode_menu_selection, TRUE);
	CheckItem (aliasing_threshold_menu, aliasing_threshold_menu_selection, TRUE);
	CheckItem (shading_threshold_menu, shading_threshold_menu_selection, TRUE);
	CheckItem (ambient_threshold_menu, ambient_threshold_menu_selection, TRUE);
	
	/* Set the checkboxes to their defaults */
	SetCtlValue (correct_texture_normals_checkbox, CORRECT_TEXTURE_NORMAL_DEFAULT);
	SetCtlValue (intersect_adjust_checkbox, INTERSECT_ADJUST_DEFAULT);
	SetCtlValue (use_jittered_sample_checkbox, USE_JITTERED_SAMPLE_DEFAULT);
	SetCtlValue (intersect_all_objects_checkbox, INTERSECT_ALL_OBJECTS_DEFAULT);
	SetCtlValue (only_correct_inside_checkbox, ONLY_CORRECT_INSIDE_DEFAULT);
	SetCtlValue (specify_focal_distance_checkbox, SPECIFY_FOCAL_DISTANCE_DEFAULT);
	
	/* Update the dialog */
	UpdtDialog(options_dialog, options_dialog->visRgn);

}	/* reset_defaults() */



/*****************************************************************************\
* procedure generate_params                                                   *
*                                                                             *
* Purpose: This procedure generates a parameter list for RTrace by looking at *
*          the options dialog and the preferences.                            *
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
	Str255	current_setting_str;		/* current setting of an option */
	long	current_setting_num;		/* current setting of a menu option */
	char	this_param[200];			/* parameter we're generating */
	double	value;
	int		int_value;

	/* This ugly procedure is a long list of comparisons which follow the
	    following general format:
	    
	    	get_current_setting
			if {current_setting != default setting}
	        	add_option_to_command_line								*/

	/* do Image Width */
	image_width = get_edittext_as_int(options_dialog, IMAGE_WIDTH_NUM);
	if (image_width != IMAGE_WIDTH_DEFAULT)
		{
		sprintf(this_param, "w%d", image_width);
		add_parameter (params, this_param);
		}

	/* do Image Height */
	image_height = get_edittext_as_int(options_dialog, IMAGE_HEIGHT_NUM);
	if (image_height != IMAGE_WIDTH_DEFAULT)
		{
		sprintf(this_param, "h%d", image_height);
		add_parameter (params, this_param);
		}

	/* do Aliasing Threshold */
	value = get_edittext_as_real(options_dialog, ALIASING_THRESHOLD_NUM);
	if (value != ALIASING_THRESHOLD_DEFAULT)
		{
		sprintf(this_param, "A%g", value);
		add_parameter (params, this_param);
		}

	/* do Shading Threshold */
	value = get_edittext_as_real(options_dialog, SHADING_THRESHOLD_NUM);
	if (value != SHADING_THRESHOLD_DEFAULT)
		{
		sprintf(this_param, "S%g", value);
		add_parameter (params, this_param);
		}

	/* do Ambient Threshold */
	value = get_edittext_as_real(options_dialog, AMBIENT_THRESHOLD_NUM);
	if (value != AMBIENT_THRESHOLD_DEFAULT)
		{
		sprintf(this_param, "T%g", value);
		add_parameter (params, this_param);
		}

	/* do Focal Aperature */
	value = get_edittext_as_real(options_dialog, FOCAL_APERATURE_NUM);
	if (value != FOCAL_APERATURE_DEFAULT)
		{
		sprintf(this_param, "T%g", value);
		add_parameter (params, this_param);
		}

	/* do Stereo Separation */
	if (view_menu_selection != 1)
		{
		value = get_edittext_as_real(options_dialog, STEREO_SEPARATION_NUM);
		if (value != STEREO_SEPARATION_DEFAULT)
			{
			sprintf(this_param, "A%g", (stereo_separation_menu_selection == 2) ?
										-value : value);
			add_parameter (params, this_param);
			}
		}

	/* do Cluster Size */
	int_value = get_edittext_as_int(options_dialog, CLUSTER_SIZE_NUM);
	if (int_value != CLUSTER_SIZE_DEFAULT)
		{
		sprintf(this_param, "c%d", int_value);
		add_parameter (params, this_param);
		}

	/* do Ambient Level */
	int_value = get_edittext_as_int(options_dialog, AMBIENT_LEVELS_NUM);
	if (int_value != AMBIENT_LEVEL_DEFAULT)
		{
		sprintf(this_param, "d%d", int_value);
		add_parameter (params, this_param);
		}

	/* do Ambient Samples */
	int_value = get_edittext_as_int(options_dialog, AMBIENT_SAMPLES_NUM);
	if (int_value != AMBIENT_SAMPLES_DEFAULT)
		{
		sprintf(this_param, "D%d", int_value);
		add_parameter (params, this_param);
		}

	/* do Shading Levels */
	int_value = get_edittext_as_int(options_dialog, SHADING_LEVELS_NUM);
	if (int_value != SHADING_LEVELS_DEFAULT)
		{
		sprintf(this_param, "s%d", int_value);
		add_parameter (params, this_param);
		}

	/* do Max Objects */
	if ((*prefs)->max_objects != MAX_OBJECTS_DEFAULT)
		{
		sprintf(this_param, "+O%ld", (*prefs)->max_objects);
		add_parameter (params, this_param);
		}

	/* do Max Lights */
	if ((*prefs)->max_lights != MAX_LIGHTS_DEFAULT)
		{
		sprintf(this_param, "+L%ld", (*prefs)->max_lights);
		add_parameter (params, this_param);
		}

	/* do Max Surfaces */
	if ((*prefs)->max_surfaces != MAX_SURFACES_DEFAULT)
		{
		sprintf(this_param, "+S%ld", (*prefs)->max_surfaces);
		add_parameter (params, this_param);
		}

	/* do Max CSG Levels */
	if ((*prefs)->max_csg_levels != MAX_CSG_LEVELS_DEFAULT)
		{
		sprintf(this_param, "+C%ld", (*prefs)->max_csg_levels);
		add_parameter (params, this_param);
		}

	/* do Textures mode */
	if ((*prefs)->texture_mode != TEXTURE_MODE_DEFAULT)
		{
		sprintf(this_param, "t%d", (int) (*prefs)->texture_mode);
		add_parameter (params, this_param);
		}

	/* do Focal Distance */
	if (GetCtlValue (specify_focal_distance_checkbox))
		{
		value = get_edittext_as_real(options_dialog, FOCAL_DISTANCE_NUM);
		if (value != FOCAL_DISTANCE_DEFAULT)
			{
			sprintf (this_param, "F%g", value);
			add_parameter (params, this_param);
			}
		}

	/* do Sampling Levels */
	current_setting_num = 4 - sampling_level_menu_selection;
	if (current_setting_num != SAMPLING_LEVEL_DEFAULT)
		{
		sprintf(this_param, "p%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Antialiasing mode */
	current_setting_num = antialiasing_menu_selection - 1;
	if (current_setting_num != ANTIALIASING_DEFAULT)
		{
		sprintf(this_param, "a%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Backface mode */
	current_setting_num = backface_menu_selection - 1;
	if (current_setting_num != BACKFACE_DEFAULT)
		{
		sprintf(this_param, "b%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Walk mode.  Note that we don't use WALK_MODE_DEFAULT
		here because rtrace's default is Serpentine but our
		default is Hilbert. */
	current_setting_num = walk_mode_menu_selection - 1;
	if (current_setting_num != 0)
		{
		sprintf(this_param, "x%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Translucence Mode */
	current_setting_num = translucence_menu_selection - 1;
	if (current_setting_num != TRANSLUCENCE_DEFAULT)
		{
		sprintf(this_param, "l%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do View Mode */
	current_setting_num = view_menu_selection - 1;
	if (current_setting_num != VIEW_DEFAULT)
		{
		sprintf(this_param, "v%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Shading Mode */
	current_setting_num = shading_menu_selection - 1;
	if (current_setting_num != SHADING_DEFAULT)
		{
		sprintf(this_param, "m%d", current_setting_num);
		add_parameter (params, this_param);
		}

	/* do Correct Textures Normal */
	if (GetCtlValue (correct_texture_normals_checkbox))
		add_parameter (params, "z1");

	/* do Intersect All Objects */
	if (GetCtlValue (intersect_all_objects_checkbox))
		add_parameter (params, "i1");

	/* do Intersect Adjust */
	if (GetCtlValue (intersect_adjust_checkbox))
		add_parameter (params, "I1");

	/* do Jittered Sample */
	if (GetCtlValue (use_jittered_sample_checkbox))
		add_parameter (params, "j1");

	/* do Only Correct Normals Inside */
	if (GetCtlValue (only_correct_inside_checkbox))
		add_parameter (params, "n1");

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
								0.001, 0.01, TRUE, TRUE))
		return FALSE;

	/* Check the Ambient Samples */
	if (!is_valid_param_int(options_dialog, AMBIENT_SAMPLES_NUM, AMBIENT_SAMPLES_LABEL, 1, INT_MAX))
		return FALSE;
	
	/* Check the Ambient Levels */
	if (!is_valid_param_int(options_dialog, AMBIENT_LEVELS_NUM, AMBIENT_LEVELS_LABEL, 0, INT_MAX))
		return FALSE;
	
	/* Check the Ambient Threshold */
	if (!is_valid_param_real(options_dialog, AMBIENT_THRESHOLD_NUM, AMBIENT_THRESHOLD_LABEL,
								0, 0.01, TRUE, TRUE))
			return FALSE;

	/* Check the Aliasing Threshold, if any */
	if (!is_valid_param_real(options_dialog, ALIASING_THRESHOLD_NUM, ALIASING_THRESHOLD_LABEL,
								0.03, 0.1, TRUE, TRUE))
		return FALSE;

	/* Check the Stereo Separation, if any */
	if ((view_menu_selection != 1)
		&& (!is_valid_param_real(options_dialog, STEREO_SEPARATION_NUM,
									STEREO_SEPARATION_LABEL, 0, DBL_MAX, TRUE, TRUE)))
		return FALSE;

	/* Check the Focal Distance */
	if (GetCtlValue (specify_focal_distance_checkbox))
		{
		if (!is_valid_param_real(options_dialog, FOCAL_DISTANCE_NUM,
									FOCAL_DISTANCE_LABEL, 0, DBL_MAX, FALSE, TRUE))
		return FALSE;
		}
	
	/* Everything is okay */
	return TRUE;
	
}	/* are_valid_params() */



/*****************************************************************************\
* procedure setup_options_controls                                            *
*                                                                             *
* Purpose: This procedure sets up the controls in the options dialog.         *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void setup_options_controls(void)
{

	short	type;		/* the item type of a DITL item */
	Rect	box;		/* the bounding box of a DITL item */

	/* Load the check box handles into memory */
	GetDItem (options_dialog, INTERSECT_ADJUST_CHECKBOX, &type,
				(Handle *) &intersect_adjust_checkbox, &box);
	GetDItem (options_dialog, USE_JITTERED_SAMPLE_CHECKBOX, &type,
				(Handle *) &use_jittered_sample_checkbox, &box);
	GetDItem (options_dialog, CORRECT_TEXTURE_NORMALS_CHECKBOX, &type,
				(Handle *) &correct_texture_normals_checkbox, &box);
	GetDItem (options_dialog, ANIMATE_CHECKBOX, &type,
				(Handle *) &animate_checkbox, &box);
	GetDItem (options_dialog, SPECIFY_FOCAL_DISTANCE_CHECKBOX, &type,
				(Handle *) &specify_focal_distance_checkbox, &box);
	GetDItem (options_dialog, INTERSECT_ALL_OBJECTS_CHECKBOX, &type,
				(Handle *) &intersect_all_objects_checkbox, &box);
	GetDItem (options_dialog, ONLY_CORRECT_INSIDE_CHECKBOX, &type,
				(Handle *) &only_correct_inside_checkbox, &box);
				
}	/* setup_options_controls() */



/*****************************************************************************\
* procedure setup_options_useritems                                           *
*                                                                             *
* Purpose: This procedure sets up the UserItems in the options dialog.        *
*                                                                             *
* Parameters: none                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 4, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/
void setup_options_useritems(void)
{

	short	type;
	Handle	handle;
	Rect	rect;
	
	/* First, initialize our UPPs */
	dim_stereo_separation_upp = NewUserItemProc(dim_stereo_separation);
	dim_focal_distance_upp = NewUserItemProc(dim_focal_distance);
	draw_antialiasing_popup_upp = NewUserItemProc(draw_antialiasing_popup);
	draw_translucence_popup_upp = NewUserItemProc(draw_translucence_popup);
	draw_shading_popup_upp = NewUserItemProc(draw_shading_popup);
	draw_backface_popup_upp = NewUserItemProc(draw_backface_popup);
	draw_aliasing_threshold_popup_upp = NewUserItemProc(draw_aliasing_threshold_popup);
	draw_shading_threshold_popup_upp = NewUserItemProc(draw_shading_threshold_popup);
	draw_ambient_threshold_popup_upp = NewUserItemProc(draw_ambient_threshold_popup);
	draw_walk_mode_popup_upp = NewUserItemProc(draw_walk_mode_popup);
	draw_view_popup_upp = NewUserItemProc(draw_view_popup);
	draw_stereo_separation_popup_upp = NewUserItemProc(draw_stereo_separation_popup);
	draw_sampling_level_popup_upp = NewUserItemProc(draw_sampling_level_popup);

	/* Set up the border around the default button in the options dialog */
	GetDItem (options_dialog, BUTTON_BORDER_USERITEM, &type, &handle, &rect);
	SetDItem (options_dialog, BUTTON_BORDER_USERITEM, type, (Handle) draw_button_border_upp, &rect);

	/* Set up the dimming of the stereo separation items */
	GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &type, &handle, &rect);
	SetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, userItem, (Handle) dim_stereo_separation_upp, &rect);

	/* Set up the dimming of the focal distance number */
	GetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, &type, &handle, &rect);
	SetDItem (options_dialog, DIM_FOCAL_DISTANCE_USERITEM, userItem, (Handle) dim_focal_distance_upp, &rect);

	/* Get the handle to the Render button */
	GetDItem (options_dialog, RENDER_BUTTON, &type, (Handle *) &render_button_handle, &rect);

	/* Set up antialiasing popup menu */
	setup_popup_menu(options_dialog, ANTIALIASING_POPUP, draw_antialiasing_popup_upp);

	/* Set up translucence popup menu */
	setup_popup_menu (options_dialog, TRANSLUCENCE_POPUP, draw_translucence_popup_upp);

	/* Set up view popup menu */
	setup_popup_menu (options_dialog, VIEW_POPUP, draw_view_popup_upp);
				
	/* Set up stereo separation popup menu */
	setup_popup_menu (options_dialog, STEREO_SEPARATION_POPUP, draw_stereo_separation_popup_upp);

	/* Set up sampling level popup menu */
	setup_popup_menu (options_dialog, SAMPLING_LEVEL_POPUP, draw_sampling_level_popup_upp);

	/* Set up shading popup menu */
	setup_popup_menu (options_dialog, SHADING_POPUP, draw_shading_popup_upp);

	/* Set up backface popup menu */
	setup_popup_menu (options_dialog, BACKFACE_POPUP, draw_backface_popup_upp);

	/* Set up aliasing threshold menu */
	setup_popup_menu (options_dialog, ALIASING_THRESHOLD_POPUP, draw_aliasing_threshold_popup_upp);

	/* Set up shading threshold menu */
	setup_popup_menu (options_dialog, SHADING_THRESHOLD_POPUP, draw_shading_threshold_popup_upp);

	/* Set up ambient threshold menu */
	setup_popup_menu (options_dialog, AMBIENT_THRESHOLD_POPUP, draw_ambient_threshold_popup_upp);

	/* Set up shading popup menu */
	setup_popup_menu (options_dialog, WALK_MODE_POPUP, draw_walk_mode_popup_upp);

}	/* setup_options_useritems() */



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
		PenPat(&qd.gray);
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
		PenPat(&qd.black);
		
		}

}	/* dim_focal_distance() */



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

	short	type;		/* the item type of a DITL item */
	Handle	handle;	/* a handle to a DITL item */
	Rect	box;		/* the bounding box of a DITL item */
	Rect	edittext_box;
	Str255	num_text;		/* temporary text storage */


	/* If the view mode is normal, disable the stereo separation items.
		Otherwise, enable them. */
	if (view_menu_selection != 1)	
		{
		
		/* Enable the EditText item */
		GetDItem (options_dialog, STEREO_SEPARATION_NUM, &type,
					&handle, &box);
		SetDItem (options_dialog, STEREO_SEPARATION_NUM,
					editText, handle, &box);
		
		/* If the text is 0.0 or 0, set it to 1.0 (it's not legal to have no
			separation in stereoscopic mode) */
		GetIText (handle, num_text);
		if ( (!strcmp(p2cstr(num_text), "0")) || (!strcmp(p2cstr(num_text), "0.0")) )
			SetIText (handle, "\p1.0");
	
		/* Draw the frame */
		InsetRect (&box, -3, -3);
		FrameRect (&box);
		
		/* Enable the stereo separation menu */
		GetDItem (options_dialog, STEREO_SEPARATION_POPUP, &type,
					&handle, &box);
		SetDItem (options_dialog, STEREO_SEPARATION_POPUP,
					userItem, handle, &box);
		
		}
	else		
		{
	
		/* Disable the EditText item */
		GetDItem (options_dialog, STEREO_SEPARATION_NUM, &type,
					&handle, &edittext_box);
		SetDItem (options_dialog, STEREO_SEPARATION_NUM,
					(statText | itemDisable), handle, &edittext_box);
		
		/* If the text isn't 0.0 or 0, set it to 0 (it's not legal to have
			stereo separation in monoscopic mode) */
		GetIText (handle, num_text);
		if ( (strcmp(p2cstr(num_text), "0")) && (strcmp(p2cstr(num_text), "0.0")) )
			SetIText (handle, "\p0.0");
	
		/* Disable the stereo separation menu */
		GetDItem (options_dialog, STEREO_SEPARATION_POPUP, &type,
					&handle, &box);
		SetDItem (options_dialog, STEREO_SEPARATION_POPUP,
					userItem + itemDisable, handle, &box);
		
		/* Get the useritem rectangle */
		GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &type,
					&handle, &box);
					
		/* extend it to the right to encompass the text */
		box.right += 195;
		
		/* Set the pen mode to Bic, draw over the text in gray */
		PenMode(patBic);
		PenPat(&qd.gray);
		PaintRect (&box);
		PenMode(patCopy);
		
		/* Draw the frame in gray */
		InsetRect (&edittext_box, -3, -3);
		FrameRect (&edittext_box);
		PenPat(&qd.black);
		
		}

}	/* dim_stereo_separation() */



/*****************************************************************************\
* procedure handle_options_popup_click                                        *
*                                                                             *
* Purpose: This procedure deals with a popup menu selection in the options    *
*          dialog.  Note that the obvious stuff has already been done in      *
*          handle_popup_click; this handles special cases.                    *
*                                                                             *
* Parameters: menu_id:       the id of the menu which was clicked.            *
*             old_selection: the selection before the menu was clicked        *
*             new_selection: the new selection.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_options_popup_click(short menu_id, short old_selection, short new_selection)
{

	Rect		rect;
	Handle		handle;
	short 		type;

	static float aliasing_thresholds[6] = {0, 0.03, 0.04, 0.05, 0.07, 0.1};
	static float shading_thresholds[6] = {0, 0.001, 0.002, 0.004, 0.007, 0.01};
	static float ambient_thresholds[8] = {0, 0.00001, 0.00005, 0.0001, 0.001, 0.01, 0, 0};


	/* If this is the View popup, and there was a switch from monoscopic to
		stereoscopic or back, refresh the Stereo separation items. */
	if ((menu_id == VIEW_MENU) && ((old_selection == 1) || (new_selection == 1)))
		{
		
		/* Get the rectangle of the useritem (just to the right of the items */
		GetDItem (options_dialog, DIM_STEREO_SEPARATION_USERITEM, &type,
					&handle, &rect); 
					
		/* extend the rectangle rightward so it encloses the text */
		rect.right += 350;
		
		/* make the whole area invalid to it gets updated later */
		SetPort (options_dialog);
		InvalRect (&rect);
		
		}

	/* Set the aliasing threshold text to match the menu selection */
	if (menu_id == ALIASING_THRESHOLD_MENU)
		set_dialog_text_to_real(options_dialog, ALIASING_THRESHOLD_NUM,
							aliasing_thresholds[new_selection]);
			
	/* Set the shading threshold text to match the menu selection */
	if (menu_id == SHADING_THRESHOLD_MENU)
		set_dialog_text_to_real(options_dialog, SHADING_THRESHOLD_NUM,
							shading_thresholds[new_selection]);
			
	/* Set the ambient threshold text to match the menu selection */
	if (menu_id == AMBIENT_THRESHOLD_MENU)
		set_dialog_text_to_real(options_dialog, AMBIENT_THRESHOLD_NUM,
							ambient_thresholds[new_selection]);
			
}	/* handle_options_popup_click() */



/*****************************************************************************\
* procedure handle_options_checkbox_click                                     *
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
void handle_options_checkbox_click(ControlHandle check_box)
{

	short	type;
	Handle	handle;
	Rect	rect;
	
	/* If this is the focal distance check box, set things up so that
		the focal distance text and the dimming useritem get refreshed. */
	if (check_box == specify_focal_distance_checkbox)
		{
		
		/* If this was selected, select something else */
		if (((DialogPeek) options_dialog)->editField == FOCAL_DISTANCE_NUM - 1)
			/* Select the image width number */
			SelIText (options_dialog, IMAGE_WIDTH_NUM, 0, 32767);

		/* Get the rectangle of the useritem (just to the left of the text
			in the DITL) */
		GetDItem (options_dialog, FOCAL_DISTANCE_NUM, &type,
					&handle, &rect); 
					
		/* extend the rectangle so it covers the text border */
		InsetRect(&rect, -3, -3);
		
		/* make the whole area invalid so it gets updated later */
		SetPort (options_dialog);
		InvalRect (&rect);
		}

}	/* handle_options_checkbox_click() */



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
* procedure draw_translucence_popup                                           *
*                                                                             *
* Purpose: This procedure draws the translucence popup menu in the options    *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_translucence_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, translucence_menu,
					translucence_menu_selection, &translucence_menu_upper_left);

}	/* draw_translucence_popup() */



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
* procedure draw_backface_popup                                               *
*                                                                             *
* Purpose: This procedure draws the backface popup menu in the options        *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_backface_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, backface_menu,
					backface_menu_selection, &backface_menu_upper_left);

}	/* draw_backface_popup() */



/*****************************************************************************\
* procedure draw_aliasing_threshold_popup                                     *
*                                                                             *
* Purpose: This procedure draws the aliasing threshold popup menu in the      *
*          options dialog box.                                                *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_aliasing_threshold_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, aliasing_threshold_menu,
					aliasing_threshold_menu_selection, &aliasing_threshold_menu_upper_left);

}	/* draw_aliasing_threshold_popup() */



/*****************************************************************************\
* procedure draw_shading_threshold_popup                                      *
*                                                                             *
* Purpose: This procedure draws the shading threshold popup menu in the       *
*          options dialog box.                                                *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_shading_threshold_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, shading_threshold_menu,
					shading_threshold_menu_selection, &shading_threshold_menu_upper_left);

}	/* draw_shading_threshold_popup() */



/*****************************************************************************\
* procedure draw_ambient_threshold_popup                                      *
*                                                                             *
* Purpose: This procedure draws the ambient threshold popup menu in the       *
*          options dialog box.                                                *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1995                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_ambient_threshold_popup (WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, ambient_threshold_menu,
					ambient_threshold_menu_selection, &ambient_threshold_menu_upper_left);

}	/* draw_ambient_threshold_popup() */



/*****************************************************************************\
* procedure draw_walk_mode_popup                                              *
*                                                                             *
* Purpose: This procedure draws the walk mode popup menu in the options       *
*          dialog box.                                                        *
*                                                                             *
* Parameters: item_number: the ID of the user item used as a popup menu       *
*             the_window: ignored.                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 31, 1994                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void draw_walk_mode_popup(WindowPtr the_window, short item_number)
{

	/* Draw the popup */
	draw_any_popup (options_dialog, item_number, walk_mode_menu,
					walk_mode_menu_selection, &walk_mode_menu_upper_left);

}	/* draw_walk_mode_popup() */



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



