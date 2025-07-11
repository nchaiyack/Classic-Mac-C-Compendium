typedef unsigned 	char Str4[5];

typedef union
	{
	Str4			TEXT;
	OSType			LONG;
	} File_Union;

typedef struct
	{
	Str4			CreatorType;
	Str4			FileType;
	Str32			Description;
	} prefs;
	
#define TRUE				1
#define FALSE				0
#define GOOD				1
#define	BAD					0
#define YES					1
#define NO					0
#define REMOVE_EVENTS		0
#define NIL_PTR				0L
#define IN_FRONT 			(WindowPtr)-1L
#define SLEEP_TICKS			0L
#define MOUSE_REGION		0L
#define DRAG_EDGE			20
#define	DELAY				8
#define	MAIN_SEG			4
#define	INIT_SEG			3
#define	APEV_SEG			2
#define TYPE_LEN 			4
#define	DESC_LEN			32

#define	CHNG_TYPE_DLOG_ID		200
#define 	CHNG_OK				1
#define 	CHNG_CANCEL			2
#define 	CHNG_CREATOR		3
#define 	CHNG_FILE			4
#define 	CHNG_CRNT_CREATOR	5
#define 	CHNG_CRNT_TYPE		6
#define 	CHNG_MKLK 			8

#define	EDIT_LIST_DLOG_ID	201
#define		EDIT_ADD		1
#define		EDIT_CLOSE		2
#define		EDIT_DELETE		3
#define		EDIT_CREATOR	4
#define		EDIT_FILE		5
#define		EDIT_DESCR		6
#define		EDIT_GET_FILE	12

#define	ABOUT_DLOG_ID		202				//	"About Creator Changer..." ID
#define		ABOUT_OK		1

#define ERROR_ALERT_ID		300				//	Error alert ID
#define ERROR_STRING_LIST	128
#define 	TYPES_ERROR_1	1
#define		TYPES_ERROR_2	2
#define		DESC_ERROR		3
#define		SYS7_ERROR_1	4
#define		SYS7_ERROR_2	5
#define		BAD_ITEM_ERROR	6

#define	MAIN_MENU_BAR_ID	128				//	The menubar ID

#define	APPLE_MENU_ID		128				//	The apple menu ID
#define		A_ABOUT_ITEM	1				//	about creator changer menu ID

#define	OPTIONS_MENU_ID		129				//	The options menu ID
#define		O_OPEN_ITEM		1				//	open file menu item ID
#define		O_EDIT_LIST		2				//	edit the types list item ID
#define		O_QUIT_ITEM		4				//	quit menu item ID

#define	POP_UP_MENU_ID		300				//	the pop-up menu ID
#define		POP_UP_MENU		7
