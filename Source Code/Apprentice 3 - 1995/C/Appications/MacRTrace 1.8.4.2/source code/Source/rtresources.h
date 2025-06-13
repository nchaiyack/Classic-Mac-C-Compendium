/*****************************************************************************\
* resources.h                                                                 *
*                                                                             *
* This file contains the resource IDs of resources used by RTrace.            *
\*****************************************************************************/

/* Dialog IDs */
#define OPTIONS_DLG				128
#define ABOUT_DLG				129
#define PREFERENCES_DLG			130
#define SAVE_DIALOG				131
#define ABORT_RENDER_DIALOG		132
#define STATUS_DIALOG			133
#define ANIMATION_DIALOG		134
#define SAVE_ANIM_DIALOG		135
#define SAVING_FRAME_DIALOG		136
#define PARAM_ERROR_DIALOG		137
#define INVALID_NUM_DIALOG		138
#define PROMPT_SAVE_DIALOG		139
#define CANCEL_DIALOG			200
#define ERROR_DIALOG			201
#define OFFER_TO_ABORT_DIALOG	202

/* Dialog items in about dialog */
#define OKAY_BUTTON						1
#define MORE_BUTTON						2
#define ABOUT_BUTTON_BORDER_USERITEM	3
#define ABOUT_PICT_USERITEM				4
#define ABOUT_TEXT_PICT_USERITEM		5
#define ABOUT_CREDITS_PICT_USERITEM		6

/* Dialog items in save... dialog */
#define SAVE_FILENAME					7
#define SAVE_FILE_TYPE_TEXT				9
#define SAVE_FILE_TYPE_POPUP			10

/* Dialog items in Prompt to Save (before quitting) dialog */
#define PROMPT_SAVE_SAVE_BUTTON			1
#define PROMPT_SAVE_CANCEL_BUTTON		2

/* Dialog items in preferences dialog */
#define PREF_BUTTON_BORDER_USERITEM			2
#define ALLOW_BACKGROUND_TASKS_CHECKBOX		3
#define ALLOW_BACKGROUND_TASKS_NUM			5
#define DIM_ALLOW_BACKGROUND_TASKS_USERITEM	7
#define ALLOW_BACKGROUND_TASKS_LABEL		8
#define SHOW_IMAGE_RENDERING_CHECKBOX		9
#define DIM_SHOW_IMAGE_RENDERING_USERITEM	10
#define KEEP_IMAGE_IN_MEMORY_CHECKBOX		11
#define DIM_KEEP_IMAGE_IN_MEMORY_USERITEM	12
#define SHOW_STATUS_WINDOW_CHECKBOX			13
#define HIDE_OPTIONS_WINDOW_CHECKBOX		14
#define SHOW_ABOUT_WINDOW_CHECKBOX			15
#define USE_TEXTURES_CHECKBOX				16
#define TEXTURES_IN_OBJECTS_RADIO			17
#define TEXTURES_AFTER_OBJECTS_RADIO		18
#define DIM_TEXURES_RADIO_BUTTONS_USERITEM	19
#define DUMMY_EDITTEXT						20

/* Dialog items in status dialog */
#define STATUS_TEXT					1
#define PROGRESS_BAR_USERITEM		4
#define MEMORY_BAR_USERITEM			6

/* Dialog items in animation dialog */
#define CANCEL_BUTTON				2
#define T_RANGES_FROM_EQ			5
#define T_RANGES_TO_EQ				6
#define NUM_FRAMES_LABEL			9
#define NUM_FRAMES_NUM				10
#define X_EYE_EQ					14
#define Y_EYE_EQ					15
#define Z_EYE_EQ					16
#define X_UP_EQ						21
#define Y_UP_EQ						22
#define Z_UP_EQ						23
#define X_LOOK_EQ					28
#define Y_LOOK_EQ					29
#define Z_LOOK_EQ					30
#define ANGLE_X_EQ					33
#define ANGLE_Y_EQ					36

/* Dialog items in options dialog */
#define RENDER_BUTTON					1
#define DEFAULTS_BUTTON					2
#define BUTTON_BORDER_USERITEM			3
#define IMAGE_WIDTH_NUM					4
#define IMAGE_WIDTH_LABEL				5
#define IMAGE_HEIGHT_NUM				6
#define IMAGE_HEIGHT_LABEL				7
#define INTERSECT_ADJUST_CHECKBOX		8
#define CORRECT_TEXTURE_NORMAL_CHECKBOX	9
#define USE_JITTERED_SAMPLE_CHECKBOX	10
#define INTERSECT_ALL_OBJECTS_CHECKBOX	11
#define ANIMATE_CHECKBOX				12
#define CHECK_ALL_NORMALS_CHECKBOX		13
#define ALIASING_THRESHOLD_NUM			14
#define ALIASING_THRESHOLD_LABEL		15
#define DIM_ALIASING_THRESHOLD_USERITEM	16
#define SHADING_THRESHOLD_NUM			17
#define SHADING_THRESHOLD_LABEL			18
#define AMBIENT_THRESHOLD_NUM			19
#define AMBIENT_THRESHOLD_LABEL			20
#define AMBIENT_SAMPLES_NUM				21
#define AMBIENT_SAMPLES_LABEL			22
#define AMBIENT_LEVELS_NUM				23
#define AMBIENT_LEVELS_LABEL			24
#define SHADING_LEVELS_NUM				25
#define SHADING_LEVELS_LABEL			26
#define CLUSTER_SIZE_NUM				27
#define CLUSTER_SIZE_LABEL				28
#define FOCAL_APERATURE_NUM				29
#define FOCAL_APERATURE_LABEL			30
#define TRANSLUCENCE_POPUP				31
#define TRANSLUCENCE_POPUP_TEXT			32
#define SAMPLING_LEVEL_POPUP			33
#define SAMPLING_LEVEL_TEXT				34
#define VIEW_POPUP						35
#define VIEW_POPUP_TEXT					36
#define SHADING_POPUP					37
#define SHADING_POPUP_TEXT				38
#define ANTIALIASING_POPUP				39
#define ANTIALIASING_POPUP_TEXT			40
#define BACKFACE_POPUP					41
#define BACKFACE_POPUP_TEXT				42
#define WALK_MODE_POPUP					43
#define WALK_MODE_POPUP_TEXT			44
#define INTERSECT_POPUP					45
#define INTERSECT_POPUP_TEXT			46

#define FOCAL_DISTANCE_NUM				47
#define FOCAL_DISTANCE_LABEL			48
#define STEREO_SEPARATION_POPUP			49
#define STEREO_SEPARATION_NUM			50
#define STEREO_SEPARATION_LABEL			51
#define DIM_STEREO_SEPARATION_USERITEM	52
#define MAX_OBJECTS_NUM					53
#define MAX_OBJECTS_LABEL				54
#define MAX_LIGHTS_NUM					55
#define MAX_LIGHTS_LABEL				56
#define MAX_SURFACES_NUM				57
#define MAX_SURFACES_LABEL				58
#define MAX_CSG_LEVELS_NUM				59
#define MAX_CSG_LEVELS_LABEL			60
#define DIM_FOCAL_DISTANCE_USERITEM		61

/* Items in Alerts Dialogs */
#define	OFFER_ABORT_BUTTON					2
#define ABORT_RENDER_CANCEL					1
#define ABORT_RENDER_FRAME_BUTTON_USERITEM	5
#define CANCEL_FRAME_BUTTON_USERITEM		5
#define ERROR_FRAME_BUTTON_USERITEM			4
#define OFFER_ABORT_FRAME_BUTTON_USERITEM	5
#define DONT_ABORT_BUTTON					1

/* Window resources */
#define	LOG_WINDOW				128

/* PICT resources */
#define DOWN_ARROW_PICT			128
#define DOWN_ARROW_PICT_GRAYED	129
#define ABOUT_PICT_32			130
#define ABOUT_TEXT_PICT_32		131
#define ABOUT_PICT_8			132
#define ABOUT_TEXT_PICT_8		133
#define ABOUT_PICT_1			134
#define ABOUT_TEXT_PICT_1		135
#define ABOUT_CREDITS_1			136
#define ABOUT_CREDITS_8			137

/* Icon resources */
#define NOTIFICATION_ICON		128

/* Popup MENU resources */
#define STEREO_SEPARATION_MENU		128
#define ANTIALIASING_MENU			129
#define TRANSLUCENCE_MENU			130
#define NORMAL_MENU					131
#define TEXTURE_MENU				132
#define VIEW_MENU					133
#define INTERSECT_MENU				134
#define SHADING_MENU				135
#define SAVE_FILE_TYPE_MENU			136
#define SAMPLING_LEVEL_MENU			137
#define SAVE_ANIM_FILE_TYPE_MENU	138
#define BACKFACE_MENU				139
#define WALK_MODE_MENU				140

/* Pulldown Menu resources */
#define APPLE_MENU		1001
#define FILE_MENU		1002
#define EDIT_MENU		1003
#define WINDOWS_MENU	1004

/* Pulldown menu items in the Apple menu */
#define ABOUT_ITEM	1

/* Pulldown menu items in the File menu */
#define OPEN_ITEM	1
#define SAVE_ITEM	2
#define QUIT_ITEM	4

/* Pulldown menu items in the Edit menu */
#define UNDO_ITEM	1
#define CUT_ITEM	3
#define COPY_ITEM	4
#define PASTE_ITEM	5
#define CLEAR_ITEM	6

/* Pulldown menu items in the Windows menu */
#define SHOW_OPTIONS_ITEM	1
#define SHOW_STATUS_ITEM	2
#define SHOW_LOG_ITEM		3
#define SHOW_ANIMATION_ITEM	4
#define SHOW_IMAGE_ITEM		5

/* Pulldown menu items in the Edit menu */
#define PREFERENCES_ITEM	8
