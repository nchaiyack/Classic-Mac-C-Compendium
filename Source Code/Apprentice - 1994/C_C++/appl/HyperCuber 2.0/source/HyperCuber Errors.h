enum									//  General errors
	{
	bitmap_too_large = 1
	};

enum									//  Terminal errors
	{
	missing_fpu = 1,
	missing_gestalt,
	missing_folder_manager,
	missing_68020,
	missing_color_qd
	};


#define	GENERAL_ERROR_ALERT_ID	131
#define	TERMINAL_ERROR_ALERT_ID	130

#define GENERAL_ERRORS_STRN_ID	2000
#define	TERMINAL_ERRORS_STRN_ID	2001