typedef struct
	{
	char_ptr    parameters[50];
	char		storage[400];
	short		current_param;
	short		current_loc;
	} params_struct;

#if __powerc
#pragma options align=mac68k
#endif

/* types */
typedef
struct
	{
	
	short	prefs_version;
	Rect	options_window_position;
	Rect	image_window_position;
	Rect	log_window_position;
	Rect	status_window_position;
	Rect	animation_window_position;
	Boolean show_image_rendering;
	Boolean allow_background_tasks;
	Boolean show_status_window_flag;
	Boolean show_about_window_flag;
	Boolean hide_options_window_flag;
	char    texture_mode;
	long	time_between_events;
	long	max_objects;
	long	max_lights;
	long	max_surfaces;
	long	max_csg_levels;
	
	} preferences_struct, *preferences_ptr, **preferences_handle;

#if __powerc
#pragma options align=reset
#endif
