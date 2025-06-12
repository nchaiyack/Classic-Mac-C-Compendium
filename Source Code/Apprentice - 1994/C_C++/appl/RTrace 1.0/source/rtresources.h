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
#define ALLOW_BACKGROUND_TASKS_CHECKBOX		2
#define SHOW_IMAGE_RENDERING_CHECKBOX		3
#define PREF_BUTTON_BORDER_USERITEM			4
#define KEEP_IMAGE_IN_MEMORY_CHECKBOX		5
#define ALLOW_BACKGROUND_TASKS_NUM			8
#define DIM_ALLOW_BACKGROUND_TASKS_USERITEM	9
#define SHOW_STATUS_WINDOW_CHECKBOX			10
#define HIDE_OPTIONS_WINDOW_CHECKBOX		11
#define DIM_SHOW_IMAGE_RENDERING_USERITEM	12
#define DIM_KEEP_IMAGE_IN_MEMORY_USERITEM	13
#define SHOW_ABOUT_WINDOW_CHECKBOX			14
#define DUMMY_EDITTEXT						15
#define ALLOW_BACKGROUND_TASKS_LABEL		16

/* Dialog items in status dialog */
#define STATUS_TEXT					1
#define PROGRESS_BAR_USERITEM		3
#define FREE_MEMORY_NUM				7
#define IMAGE_MEMORY_NUM			8
#define NUM_OBJECTS_NUM				9
#define SUB_PROGRESS_BAR_USERITEM	10

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
#define FOCAL_APERATURE_NUM				8
#define FOCAL_APERATURE_LABEL			9
#define CLUSTER_SIZE_NUM				10
#define CLUSTER_SIZE_LABEL				11
#define SHADING_LEVELS_NUM				12
#define SHADING_LEVELS_LABEL			13
#define SHADING_THRESHOLD_NUM			14
#define SHADING_THRESHOLD_LABEL			15
#define AMBIENT_SAMPLES_NUM				16
#define AMBIENT_SAMPLES_LABEL			17
#define AMBIENT_LEVELS_NUM				18
#define AMBIENT_LEVELS_LABEL			19
#define AMBIENT_THRESHOLD_NUM			20
#define AMBIENT_THRESHOLD_LABEL			21
#define ALIASING_THRESHOLD_NUM			22
#define ALIASING_THRESHOLD_LABEL		23
#define DIM_ALIASING_THRESHOLD_USERITEM	24
#define STEREO_SEPARATION_NUM			25
#define STEREO_SEPARATION_LABEL			26
#define STEREO_SEPARATION_POPUP			27
#define DIM_STEREO_SEPARATION_USERITEM	28
#define FOCAL_DISTANCE_NUM				29
#define DIM_FOCAL_DISTANCE_USERITEM		30
#define SAMPLING_LEVEL_POPUP			31
#define SAMPLING_LEVEL_TEXT				32
#define SPECIFY_FOCAL_DISTANCE_CHECKBOX	33
#define ANTIALIASING_POPUP				34
#define ANTIALIASING_POPUP_TEXT			35
#define LIGHTING_POPUP					36
#define LIGHTING_POPUP_TEXT				37
#define NORMAL_POPUP					38
#define NORMAL_POPUP_TEXT				39
#define TEXTURE_POPUP					40
#define TEXTURE_POPUP_TEXT				41
#define VIEW_POPUP						42
#define VIEW_POPUP_TEXT					43
#define INTERSECT_POPUP					44
#define INTERSECT_POPUP_TEXT			45
#define SHADING_POPUP					46
#define SHADING_POPUP_TEXT				47
#define CORRECT_TEXTURE_NORMAL_CHECKBOX	48
#define INTERSECT_ADJUST_CHECKBOX		49
#define USE_JITTERED_SAMPLE_CHECKBOX	50
#define ANIMATE_CHECKBOX				51
#define ANIMATION_BUTTON				52
#define FOCAL_DISTANCE_LABEL			53

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
#define LIGHTING_MENU				130
#define NORMAL_MENU					131
#define TEXTURE_MENU				132
#define VIEW_MENU					133
#define INTERSECT_MENU				134
#define SHADING_MENU				135
#define SAVE_FILE_TYPE_MENU			136
#define SAMPLING_LEVEL_MENU			137
#define SAVE_ANIM_FILE_TYPE_MENU	138
 
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
