/* STR#s */

#define EXTENSIONS_STR	128

#define ERROR_MSGS		1000
#define ERR_VERSION		2
#define ERR_EJECT			3
#define ERR_BADMOUNT		4
#define ERR_NOCDDIR		5
#define ERR_CREATEWD		6
#define ERR_NOTFOUND		7
#define ERR_OPENFILE		8
#define ERR_READFILE 	9
#define ERR_CREATEFILE	10
#define ERR_WRITEFILE	11
#define ERR_FILEBUF		12
#define ERR_HITLISTCREATE	13
#define ERR_HITLISTEXT	14
#define ERR_INVRECSIZE	15
#define ERR_DIVCODEORDER	16
#define ERR_HEADINC		17
#define ERR_NOCD			18
#define ERR_MEMORY		19
#define ERR_WRONGNREC	20
#define ERR_CREATERES	21
#define ERR_OPENRES		22
#define ERR_WRITERES		23
#define ERR_READRES		24
#define ERR_PRINTABORT	25
#define ERR_PRINT			26
#define ERR_MAXHITSEXC	27
#define ERR_INCVERSION	28
#define ERR_MAXWIN		29
#define ERR_NOPREF		30
#define ERR_OPENPREF		31
#define ERR_READPREF		32
#define ERR_CREATPREF	33
#define ERR_WRITEPREF	34
#define ERR_STADENONE	35
#define ERR_INVENAME		36
#define ERR_INVACCNUM	37
#define ERR_INVKEYWORD	38
#define ERR_TOOOLD		39
#define ERR_AEINSTALL	40
#define ERR_AEOPEN		41
#define ERR_PREFCHGWARN	42
#define ERR_REINITFAIL	43
#define ERR_PNF			44
#define ERR_INCQUERY		45
#define ERR_INCPREF		46

#define OTHERS				200
#define VOL_LABEL			1
#define REL_TXT			2
#define RESTITLE			3
#define PAGESTR			4
#define PROMPTSTR			5
#define RESHEADER			6
#define PREFSNAME			7
#define CDROMPATH			8
#define QUERYTITLE		9
#define OTHERCREATOR		10
#define ITEMSINLIST		11
#define HELPSTR			12

#define EMBLDATA			300
#define SWISSDATA			400
#define DATA_DIR			1
#define INDEX_DIR			2

#define INDEX_NAMES		500
#define DIV_TABLE			1
#define ENAME_IDX			2
#define BRIEF_IDX			3
#define ACNUM_TRG			4
#define ACNUM_HIT			5
#define KEYWORD_TRG		6
#define KEYWORD_HIT		7
#define FREETEXT_TRG		8
#define FREETEXT_HIT		9
#define AUTHOR_TRG		10
#define AUTHOR_HIT		11
#define TAXON_TRG			12
#define TAXON_HIT			13

#define FORMAT_STRINGS	700

#define LOCATE_PROMPTS	800
#define EMBLINDEX_PROMPT	1
#define SWISSINDEX_PROMPT	2


/* ALRTs */

#define ERROR_ALRT			128
#define INVENTRYNAME_ALRT	129
#define INVACNUM_ALRT		130
#define CLOSE_ALRT			131
#define aaSave				1
#define aaDiscard			2
#define aaCancel			3


/* DLOGs */

#define SEARCHWAIT_DLG		150

#define FETCHTITLES_DLG		152

#define EXPORT_DLG			154

#define PICKCREATOR_DLG		156

#define ABOUT_DLG				200
#define ABOUT_VERSION_STR	9

#define OPTIONS_DLG			300
#define CONFIRM_BOX			3
#define STARTQ_BOX			4
#define OPTIONS_USRITEM1	5
#define OPTIONS_USRITEM2	7

#define INFO_DLG				400
#define INFO_EMBL_REL		8
#define INFO_EMBL_ENTRIES	9
#define INFO_SWISS_REL		10
#define INFO_SWISS_ENTRIES	11
#define INFO_USRITEM1		12
#define INFO_USRITEM2		14

#define INITWAIT_DLG		500
#define PRINTINFO_DLG	600
#define SPOOLINFO_DLG	610

#define LOCATE_DLG				700
#define LOCATE_EMBLCHG_BTN		4
#define LOCATE_EMBLINX_TXT		5
#define LOCATE_SWISSCHG_BTN	7
#define LOCATE_SWISSINX_TXT	8
#define LOCATE_USRITEM1			9
#define LOCATE_USRITEM2			11

#define BROWSER_DLG			800
#define BROWSER_USRITEM1	3
#define BROWSER_EDITFLD		4
#define BROWSER_FINDBTN		5
#define BROWSER_USRITEM2	6
#define BROWSER_SCROLLBAR	8
#define BROWSER_USRITEM3	9

#define SAVEENTRY_DLG	1001
#define SKIP_BUTTON		9
#define SAVEALL_BUTTON	10
#define FORMAT_STR		11

#define DIRSELECT_DLG	1002

#define HELP_DLG			1280

#define QUERY_DLG			4000
#define QRY_SUBQRY1		2
#define QRY_BOOLSTR1		7
#define QRY_IN1			12
#define QRY_POPUP1		17
#define QRY_EMBL_BTN		23
#define QRY_SWISS_BTN	24
#define QRY_AND_BTN		26
#define QRY_OR_BTN		27
#define QRY_USRITEM		28
#define QRY_BROWSEBTN	29
#define QRY_CLEARBTN		30

/* MENUs */

#define APPLE_M		128
#define ABOUT_I		1
#define HELP_I			2

#define FILE_M			129
#define NEW_I			1
#define OPEN_I			2
#define LOAD_I			3
#define CLOSE_I		4
#define SAVE_I			5
#define SAVEAS_I		6
#define EXPORTSEL_I	7
#define PAGESETUP_I	9
#define PRINT_I		10
#define QUIT_I			12

#define EDIT_M			130
#define UNDO_I			1
#define CUT_I			3
#define COPY_I			4
#define PASTE_I		5
#define CLEAR_I		6
#define SELALL_I		8
#define SELNONE_I		9
#define DUPLICATE_I	11

#define OSTUFF_M		131
#define INFO_I			1
#define XREF_I			2

#define PREFS_M		132
#define FORMAT_I		1
#define CREATOR_I		2
#define LOCATE_I		3
#define GENERAL_I		4
#define SAVEPREFS_I	6

#define WINDOWS_M		133
#define ROTATE_I		1
#define FIRSTWIN_I	3

#define CREATOR_M	170
#define WORD_I		1
#define WRITE_I	2
#define TTEXT_I	3
#define OTHERS_I	5

#define FORMAT_M	171

#define XREF_M		172
#define ALLXREFS_I	1

#define POPUP_M	200