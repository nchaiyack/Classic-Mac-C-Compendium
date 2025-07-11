/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * This file contains resource definitions for Pop mail
 **********************************************************************/
#define CREATOR 'CSOm'
#define SETTINGS_TYPE 'PREF'
#define MAILBOX_TYPE	'TEXT'
#define TOC_TYPE			'TOCF'
#define CTB_CFIG_TYPE 'CTfg'
#define CTB_CFIG_ID 1001
#define SAVE_POS_TYPE 'S_WP'
#define PLUG_TYPE			'rsrc'
typedef struct
{
	Rect r;
	Boolean zoomed;
} PositionType, *PositionPtr, **PositionHandle;

/**********************************************************************
 * for holding page setup stuff
 **********************************************************************/
#define PRINT_RTYPE 'CSOp'
#define PRINT_CSOp	1001

/**********************************************************************
 * Strings - these are STR# resources for use for various things
 * They are organized as an enum for the STR#'s resource ids, then
 * enums (in the form of typedefs) for the individual strings
 **********************************************************************/
typedef enum {
	PRIOR_STRN=600,						/* priority strings */
	PW_STRN=800,							/* strn for password change protocol */
	PREF_STRN=1000, 					/* strn for user preferences and settings */
	BOX_LINES_STRN=1200,			/* places to draw lines in mailbox windows */
	FILE_STRN=1400, 					/* file system errors */
	HEADER_STRN=1600, 				/* headers for outgoing messages */
	UNDO_STRN=1800, 					/* menu items for undo */
	MACTCP_ERR_STRN=2000, 		/* mactcp error list */
	ICMP_STRN=2200, 					/* icmp message list */
	CTB_NAV_IN_STRN=2400,
	CTB_NAV_OUT_STRN=2600,
	SMTP_STRN=2800, 					/* STMP commands */
	POP_STRN=3000,						/* POP commands */
	BADHEAD_STRN=3200,				/* headers to delete */
	FROM_STRN=3400, 					/* "from " header is toast, baby */
	CTB_ERR_STRN=3600,				/* CTB errors */
	MONTH_STRN=3800,
	WEEKDAY_STRN=4000,
	CTB_NAV_MID_STRN=4200,
	EX_HEADERS_STRN=4400,
	COMP_HELP_STRN=4600,
	MBWIN_HELP_STRN=4800,
	NICK_HELP_STRN=5000,
	MBOX_HELP_STRN=5200,
	REPLY_STRN=5400,
	PHWIN_HELP_STRN=24000,
	HEADER_LABEL_STRN=24200,
	STRN_LIMIT								/* placeholder */
} StrnEnum;

typedef enum {
	WID_STAT=0,
	WID_FROM,
	WID_DATE,
	WID_SIZE,
	WID_LIMIT
} BoxLinesStrnEnum;

typedef enum {
	TO_HEAD=1,
	FROM_HEAD,
	SUBJ_HEAD,
	CC_HEAD,
	BCC_HEAD,
	ATTACH_HEAD,
	REPLYTO_HEAD,
	PRIORITY_HEAD
} HeaderStrnEnum;
#define HEAD_LIMIT REPLYTO_HEAD
#define BODY (HEAD_LIMIT-1)

typedef enum {
	CTB_TOOL_STR=1001,
	STR_LIMIT
} StrEnum;

typedef enum {
	TRANS_IN_TABL=1001,
	TRANS_OUT_TABL,
	TRANS_LIMIT
} TransEnum;

/**********************************************************************
 * Dialog Item Lists
 **********************************************************************/
typedef enum {
	OK_ALRT_DITL=1001,				/* DITL for OK Alert */
	ABOUT_ALRT_DITL,
	ERR_ALRT_DITL,
	NPREF_DLOG_DITL,
	BIG_ALRT_DITL,
	DUMP_ALRT_DITL,
	NEW_TOC_ALRT_DITL,
	NEW_MAILBOX_DITL,
	NOT_HOME_DITL,
	WANNA_SAVE_DITL,
	PROTO_ERR_ALRT_DITL,
	WANNA_SEND_DITL,
	PASSWORD_DITL,
	ADDR_ALRT_DITL,
	FIND_DITL,
	NOT_FOUND_DITL,
	DELETE_NON_EMPTY_ALRT_DITL,
	DELETE_EMPTY_ALRT_DITL,
	RENAME_DLOG_DITL,
	ICMP_ALRT_DITL,
	NEW_MAIL_ALRT_DITL,
	TIMEOUT_ALRT_DITL,
	QUIT_QUEUE_DITL,
	UPREF_DLOG_DITL,
	NEW_TOC_ALRT_DITL,
	TOC_SALV_DITL,
	DITL_LIMIT								/* placeholder */
} DitlEnum;

#define DEBUG_BUTTON	5
#define EXIT_BUTTON 7

typedef enum {
	TOC_CREATE_NEW = 1,
	TOC_CANCEL,
	TOC_USE_OLD
} RebTOCDitlEnum;
	

typedef enum {
	NEW_MAILBOX_OK = 1,
	NEW_MAILBOX_NAME,
	NEW_MAILBOX_CANCEL,
	NEW_MAILBOX_ICON,
	NEW_MAILBOX_TITLE,
	NEW_MAILBOX_FOLDER,
	NEW_MAILBOX_NOXF,
	NEW_MAILBOX_LIMIT
} NewMDitlEnum; 						/* item list for new mailbox DITL */

typedef enum {
	NEW_NICK_OK = 1,
	NEW_NICK_NAME,
	NEW_NICK_CANCEL,
	NEW_NICK_ICON,
	NEW_NICK_TITLE,
	NEW_NICK_RECIP,
	NEW_NICK_LIMIT
} NewNickDitlEnum; 					/* item list for new/rename nickname */

typedef enum {
	WANNA_SAVE_SAVE = 1,
	WANNA_SAVE_DISCARD,
	WANNA_SAVE_CANCEL,
	WANNA_SAVE_LIMIT
} WannaSaveDitlEnum;

typedef enum {
	PASSWORD_OK = 1,
	PASSWORD_CANCEL,
	PASSWORD_WORD,
	PASSWORD_WARNING,
	PASSWORD_LIMIT
} PasswordDitlEnum;

typedef enum {
	FDL_NEXT=1,
	FDL_CANCEL,
	FDL_FIND,
	FDL_NEXT_MESS,
	FDL_NEXT_BOX,
	FDL_TEXT,
	FDL_SENSITIVE,
	FDL_SUM_ONLY,
	FDL_LIMIT
} FindDitlEnum;

typedef enum {
	SADL_PARAGRAPHS=9,
	SADL_EXCLUDE_HEADERS,
	SADL_LIMIT
} SaveAsDitlEnum;

typedef enum {
	QQL_SEND=1,
	QQL_QUIT,
	QQL_CANCEL,
	QQL_LIMIT
} QuitQueueDitlEnum;

typedef enum {
  MQDL_OK=1,
	MQDL_CANCEL,
	MQDL_NOW,
	MQDL_QUEUE,
	MQDL_LATER,
	MQDL_UNQUEUE,
	MQDL_TIME,
	MQDL_DATE,
	MQDL_LIMIT
} ModQueueItems;

typedef enum {
  QMQ_SEND=1,
	QMQ_DONT,
	QMQ_SEND_ALL,
	QMQ_CANCEL,
	QMQ_LIMIT
} QuitModQueueItems;

typedef enum {
	MEMORY_QUIT=1,
	MEMORY_CONTINUE,
	MEMORY_LIMIT
} MemoryDitlEnum;

/************************************************************************
 * cursors
 ************************************************************************/
#define PENDULUM_CURS 1001
#define MENU_CURS 1005
#define DIVIDER_CURS 1006
#define BALL_CURS 128
/**********************************************************************
 * Icons
 **********************************************************************/
typedef enum {
	APP_ICON=1001,						/* application icon */
	ICON_LIMIT								/* placeholder */
} IconEnum;

typedef enum {
	FLAG_SICN=1001, 							/* a mailbox "flag" */
	EUDORA_SICN, 									/* our SICN */
	PRIOR_SICN_BASE=257,					/* consumer reports symbols */
	ICON_BAR_BASE_SICN=2000,
	SICN_LIMIT
} SIconEnum;
#define ICON_BAR_NUM 5

/**********************************************************************
 * Alerts and dialogs
 **********************************************************************/
typedef enum {
	OK_ALRT=1001, 				/* an alert with just an ok button */
	ABOUT_ALRT, 					/* About UIUCmail */
	ERR_ALRT, 						/* alert for fatal errors */
	NPREF_DLOG, 					/* dialog for preferences */
	BIG_OK_ALRT,					/* like the OK_ALRT, but bigger */
	DUMP_ALRT,						/* for debugging */
	NEW_TOC_ALRT, 				/* create a new toc? */
	NEW_MAILBOX_DLOG, 		/* ask for the name of a mailbox */
	NOT_HOME_ALRT,				/* nobody is listening to keystrokes */
	WANNA_SAVE_ALRT,			/* do we want to save changes? */
	PROTO_ERR_ALRT, 			/* error with SMTP */
	WANNA_SEND_ALRT,			/* do we want to save changes before sending? */
	PASSWORD_DLOG,				/* used to get someone's password */
	BAD_ADDR_ALRT,				/* to report a bad address */
	FIND_DLOG,						/* implement Find */
	NOT_FOUND_ALRT, 			/* report failure of find */
	DELETE_NON_EMPTY_ALRT,				/* confirm delete of non-empty mailbox */
	DELETE_EMPTY_ALRT,		/* confirm delete of empty mailbox */
	RENAME_BOX_DLOG,			/* rename mailbox */
	ICMP_ALRT,						/* ICMP message alert */
	NEW_MAIL_ALRT,				/* new mail has arrived */
	TIMEOUT_ALRT, 				/* does the user want to keep waiting, or cancel? */
	QUIT_QUEUE_ALRT,			/* you have queued messages. quit? */
	TRASH_UNREAD_ALRT,		/* trash unread messages? */
	UPREF_DLOG, 					/* the other half of the preferences */
	SAVEAS_DLOG,					/* my hacked SFPUTFILE */
	GETFOLDER_DLOG, 			/* tim maroney's hacked SFGETFILE */
	BAD_HEXBIN_ALRT,			/* a hexbin has failed */
	REB_TOC_ALRT, 				/* create a new toc? */
	CLEAR_DROP_ALRT,			/* errors UUPC'ing.  Clear drop anyway? */
	TOC_SALV_ALRT,				/* can't salvage TOC.  Rebuild? */
	MODQ_DLOG,						/* modify queue dialog */
	QUIT_MQ_ALRT,					/* quitting with modified queue */
	NEW_NICK_DLOG,				/* new nickname dialog */
	MEMORY_ALRT,					/* dialog for memory warning */
	READ_ONLY_ALRT,				/* read only */
	ALRT_LIMIT						/* placeholder */
} AlrtDialEnum;
#define SFGETFILE_ID -4000
#define SFPUTFILE_ID -3999
#define BAD_HEXBIN_IGNORE 2
#define OK 1
/**********************************************************************
 * menus - add 1000 to get resource id's
 **********************************************************************/
typedef enum {
	APPLE_MENU=501,
	FILE_MENU,
	EDIT_MENU,
	MAILBOX_MENU,
	MESSAGE_MENU,
	TRANSFER_MENU,
	SPECIAL_MENU,
	MENU_LIMIT,
	DEBUG_MENU,
	FONT_NAME_MENU,
	PRINT_FONT_MENU,
	FROM_MB_MENU,
	TO_MB_MENU,
	WINDOW_MENU,
	PRIOR_MENU=1515,
	MENU_LIMIT2
} MenuEnum;

typedef enum {
	FIND_HIER_MENU=201,
	NEW_TO_HIER_MENU,
	REPLY_TO_HIER_MENU,
	FORWARD_TO_HIER_MENU,
	REDIST_TO_HIER_MENU,
	REMOVE_TO_HIER_MENU,
	INSERT_TO_HIER_MENU,
	SORT_HIER_MENU,
	HIER_MENU_LIMIT
} HierMenuEnum;
#define MAX_BOX_LEVELS 100
typedef enum {MAILBOX,TRANSFER,MENU_ARRAY_LIMIT} MTypeEnum;

typedef enum {
	APPLE_ABOUT_ITEM=1,
	APPLE_MENU_LIMIT
} AppleMenuEnum;

typedef enum {
	FILE_NEW_ITEM=1,
	FILE_OPEN_ITEM,
	/*------------*/ FILE_BAR1_ITEM,
	FILE_CLOSE_ITEM,
	FILE_SAVE_ITEM,
	FILE_SAVE_AS_ITEM,
	/*------------*/ FILE_BAR2_ITEM,
	FILE_SEND_ITEM,
	FILE_CHECK_ITEM,
	/*------------*/ FILE_BAR3_ITEM,
	FILE_PAGE_ITEM,
	FILE_PRINT_ITEM,
	FILE_PRINT_SELECT_ITEM,
	/*------------*/ FILE_BAR5_ITEM,
	FILE_BEHIND_ITEM,
	/*------------*/ FILE_BAR5_ITEM,
	FILE_QUIT_ITEM,
	FILE_MENU_LIMIT
} FileMenuEnum;

typedef enum {
	EDIT_UNDO_ITEM=1,
	/*------------*/ EDIT_BAR1_ITEM,
	EDIT_CUT_ITEM,
	EDIT_COPY_ITEM,
	EDIT_PASTE_ITEM,
	EDIT_QUOTE_ITEM,
	EDIT_CLEAR_ITEM,
	/*------------*/ EDIT_BAR2_ITEM,
	EDIT_SELECT_ITEM,
	EDIT_SHOW_ITEM,
	EDIT_WRAP_ITEM,
	/*------------*/ EDIT_BAR3_ITEM,
	EDIT_FINISH_ITEM,
	EDIT_INSERT_TO_ITEM,
	/*------------*/ EDIT_BAR4_ITEM,
	EDIT_FIND_ITEM,
	EDIT_SORT_ITEM,
	EDIT_MENU_LIMIT
} EditMenuEnum;

typedef enum {
	MESSAGE_NEW_ITEM=1,
	MESSAGE_REPLY_ITEM,
	MESSAGE_FORWARD_ITEM,
	MESSAGE_REDISTRIBUTE_ITEM,
	MESSAGE_SALVAGE_ITEM,
	/*------------*/ MESSAGE_BAR1_ITEM,
	MESSAGE_NEW_TO_ITEM,
	MESSAGE_REPLY_TO_ITEM,
	MESSAGE_FORWARD_TO_ITEM,
	MESSAGE_REDIST_TO_ITEM,
	/*------------*/ MESSAGE_BAR2_ITEM,
	MESSAGE_QUEUE_ITEM,
	MESSAGE_MOD_Q_ITEM,
	/*------------*/ MESSAGE_BAR3_ITEM,
	MESSAGE_ATTACH_ITEM,
	/*------------*/ MESSAGE_BAR4_ITEM,
	MESSAGE_DELETE_ITEM,
	MESSAGE_MENU_LIMIT
} MessageMenuEnum;

typedef enum {
	MAILBOX_IN_ITEM=1,
	MAILBOX_OUT_ITEM,
	MAILBOX_TRASH_ITEM,
	/*------------*/ MAILBOX_BAR1_ITEM,
	MAILBOX_MENU_LIMIT
}MailboxMenuEnum;

typedef enum {
	TRANSFER_IN_ITEM=1,
	TRANSFER_TRASH_ITEM,
	/*------------*/ TRANSFER_BAR1_ITEM,
	TRANSFER_NEW_ITEM,
	TRANSFER_MENU_LIMIT
} TransferMenuEnum;

typedef enum {
	SPECIAL_PH_ITEM=1,
	/*------------*/ SPECIAL_BAR1_ITEM,
	SPECIAL_ADD_TO_ITEM,
	SPECIAL_REMOVE_TO_ITEM,
	/*------------*/ SPECIAL_BAR2_ITEM,
	SPECIAL_TRASH_ITEM,
	SPECIAL_COMPACT_ITEM,
	SPECIAL_MAILBOX_ITEM,
	/*------------*/ SPECIAL_BAR3_ITEM,
	SPECIAL_NPREFS_ITEM,
	SPECIAL_UPREFS_ITEM,
	SPECIAL_CTB_ITEM,
	SPECIAL_SIG_ITEM,
	SPECIAL_ALIASES_ITEM,
	SPECIAL_MAKE_NICK_ITEM,
	/*------------*/ SPECIAL_BAR4_ITEM,
	SPECIAL_FORGET_ITEM,
	SPECIAL_CHANGE_ITEM,
	SPECIAL_MENU_LIMIT
} SpecialMenuEnum;

typedef enum {
	FIND_FIND_ITEM=1,
	FIND_AGAIN_ITEM,
	FIND_ENTER_ITEM,
	FIND_BAR1_ITEM,
	FIND_NEXT_ITEM,
	FIND_NEXT_MESS_ITEM,
	FIND_NEXT_BOX_ITEM,
	FIND_MENU_LIMIT
} FindMenuEnum;

typedef enum {
	SORT_STATUS_ITEM=1,
	SORT_PRIORITY_ITEM,
	SORT_SENDER_ITEM,
	SORT_TIME_ITEM,
	SORT_SUBJECT_ITEM,
	SORT_LIMIT
} SortMenuEnum;

#define XLATE_BAR_ITEM 6


/**********************************************************************
 * Window templates
 **********************************************************************/
typedef enum {
	MAILBOX_WIND = 1001,
	MESSAGE_WIND,
	PROGRESS_WIND,
	ALIAS_WIND,
	FIND_WIND,
	PH_WIND,
	MBWIN_WIND,
	WIND_LIMIT
} WindEnum;


/************************************************************************
 * 
 ************************************************************************/
typedef enum {
	TZ_NAMES=1001,
	TZ_LIMIT
} TZEnum;

/**********************************************************************
 * controls
 **********************************************************************/
typedef enum {
	SCROLL_CNTL=1001,
	NEW_ALIAS_CNTL,
	REMOVE_ALIAS_CNTL,
	PH_CNTL,
	MAKE_TO_CNTL,
	INSERT_CNTL,
	FONT_NAME_CNTL,
	PRINT_FONT_CNTL,
	SEND_NOW_CNTL,
	FROM_MB_CNTL,
	TO_MB_CNTL,
	FINGER_CNTL,
	CNTL_LIMIT
} CntlEnum;

/************************************************************************
 * sounds
 ************************************************************************/
typedef enum {
	NEW_MAIL_SND=1001,
	ATTENTION_SND,
	SND_LIMIT
} SndEnum;

/************************************************************************
 * list definitions
 ************************************************************************/
typedef enum {
	MBWIN_LDEF=1001,
	LDEF_LIMIT
} LDEFEnum;

/************************************************************************
 * PICT resources
 ************************************************************************/
typedef enum {
	ROOSTER_PICT=1001,
	UIUC_PICT,
	COPYRIGHT_PICT,
	EUDORA_PICT,
	LINE_PICT,
	QUALCOMM_PICT,
	STATUS_HELP_PICT,
	PICT_LIMIT
} PICTEnum;


