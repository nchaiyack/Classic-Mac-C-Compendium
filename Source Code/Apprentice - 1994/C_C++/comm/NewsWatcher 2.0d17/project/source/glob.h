/* Constants. */

#define CR			'\r'
#define LF			'\n'
#define	CRSTR		"\r"
#define	LFSTR		"\n"
#define CRLF		"\r\n"
#define CRCR 		"\r\r"

#define homeKey				0x01		/* ascii code for home key */
#define enterKey			0x03		/* ascii code for enter key */
#define endKey				0x04		/* ascii code for end key */
#define helpKey				0x05		/* ascii code for help key */
#define deleteKey			0x08		/* ascii code for delete/backspace */
#define tabKey				0x09		/* ascii code for tab key */
#define pageUpKey			0x0B		/* ascii code for page up key */
#define pageDownKey			0x0C		/* ascii code for page down key */
#define returnKey			0x0D		/* ascii code for return key */
#define leftArrow			0x1C		/* ascii code for left arrow key */
#define rightArrow			0x1D		/* ascii code for right arrow key */
#define upArrow				0x1E		/* ascii code for up arrow key */
#define downArrow			0x1F		/* ascii code for down arrow key */
#define forwardDelKey		0x7F		/* ascii code for forward delete key */

#define escapeKeyCode		0x35		/* key code for escape key */
#define clearKeyCode		0x47		/* key code for clear key */

#define kVScroll			0			/* refcon for vertical scroll bars */
#define kHScroll			1			/* refcon for horizontal scroll bars */
#define kSendButton			2			/* refcon for send button control */

#define kLDEFProc			128

#define kNNTPPort			119
#define kSMTPPort			25
#define kFTPPort			21

#define kBufLen				0x7fff		/* buffer size for TCP stream send/receives (32K) */
#define kMaxLength			32000		/* max length of article section */

#define kTextMargin			4
#define kSectionMargin		120

#define kOffLeft			5
#define kOffTop				41
#define kDefaultWindHeight	230
#define kMinWindHeight		64
#define kMinWindWidth		200

#define kLifeBoatSize		40000L

#define kFCreator			'NNTP'
#define kFType				'NEWS'
#define kPrefType			'PREF'

#define kPrefName			"\pNewsWatcher Preferences"
#define kOldPrefName		"\pNews Prefs"
#define kFilterName			"\pNewsWatcher Filters"
#define kShowText			"\pShow Full Group List"
#define kHideText			"\pHide Full Group List"

#define kTPrefRecSize		7960


/* Types. */

typedef char CStr255[256];		/* like Str255, except for C-format strings. */

typedef enum EWindowKind {
	kFullGroup,					/* full group list window */
	kNewGroup,					/* new groups list window */
	kUserGroup,					/* user group list window */
	kSubject,					/* subject list window */
	kArticle,					/* article window */
	kMiscArticle,				/* misc article window (no parent subject window) */
	kMailMessage,				/* mail message window */
	kPostMessage,				/* posting message window */
	kStatus						/* status window */
} EWindowKind;

typedef enum EMatchType { 
	kContains, 					/* string contains another string */
	kStartsWith, 				/* string starts with another string */
	kEquals 					/* string equals another string */
} EMatchType;

typedef struct THeader {
	long number;				/* article number */
	long offset;				/* offset in strings of header text */
} THeader;

typedef struct TUnread {
	long firstUnread;			/* first unread message */
	long lastUnread;			/* last unread message */
	struct TUnread **next;		/* handle to next entry in list */
} TUnread;

typedef struct TGroup {
	long nameOffset;			/* offset in gGroupNames of group name string */
	long firstMess;				/* # of first msg in group */
	long lastMess;				/* # of last msg in group */
	TUnread **unread;			/* handle to list of unread messages for group */
	long numUnread;				/* number of unread articles in group */
	char status;				/* a general purpose status "marker" used in
	                               several contexts */
	Boolean onlyRedrawCount;	/* true to tell LDEF to only redraw the unread
								   message count */
} TGroup;

typedef struct TSubject {
	long subjectOffset;			/* offset in strings block of subject string */
	long authorOffset;			/* offset in strings block of author string */
	long number;				/* article number */
	long threadHeadNumber;		/* article number of first article in thread  */
	short myIndex;				/* index of self in subject array */
	short threadHeadIndex;		/* index of first article in thread in
								   subject array. */
	short threadOrdinal;		/* article ordinal in thread (1,2,3,...) */
	short threadLength;			/* total number of articles in thread */
	short nextInThread;			/* index in subject array of next subject 
								   in this thread, or -1 if none. */
	Boolean collapsed;			/* true if thread collapsed */
	Boolean read;				/* true if article has been read */
	Boolean drawTriangleFilled;	/* true to tell LDEF to draw triangle filled with 
								   black. */
	Boolean onlyRedrawTriangle;	/* true to tell LDEF to only redraw the
								   triangle */
	Boolean onlyRedrawCheck;	/* true to tell LDEF to only redraw the Ã */
	short highlight;			/* nonzero: index of highlight color */
} TSubject;

typedef struct TChild {
	WindowPtr childWindow;		/* pointer to child window */
	struct TChild **next;		/* handle to next entry in list */
} TChild;

typedef struct TWindow {
	EWindowKind kind;			/* kind of window */
	ListHandle theList;			/* list handle */
	TEHandle theTE;				/* TextEdit handle */
	TGroup **groupArray;		/* handle to group array */
	short numGroups;			/* number of groups */
	Handle unsubscribed;		/* handle to list of unsubscribed groups if user
								   group list, or nil if none */
	TSubject **subjectArray;	/* handle to subject array  */
	short numSubjects;			/* number of subjects */
	long firstFetched;			/* article number of first article fetched if
								   subject window */
	long groupNameOffset;		/* offset in gGroupNames of group name if subject
								   window */
	Handle strings;				/* handle to strings */
	Handle fullText;			/* handle to full article text */
	Handle headerText;			/* handle to header text for message window if
								   header not shown */
	short numSections;			/* number of sections */
	short curSection;			/* current section */
	long **sectionBreaks;		/* handle to array of section breaks */
	Handle msgId;				/* handle to msgId if misc article window */
	FSSpec theFile;				/* saved disk file */
	ScriptCode scriptTag;		/* script code for saved disk file */
	WindowPtr parentWindow;		/* pointer to parent window */
	short parentGroup;			/* index in groupArray of parent window for 
								   this subject window */
	short parentSubject;		/* index in subjectArray of parent window for
								   this article window */
	TChild **childList;			/* handle to linked list of child windows */
	Boolean changed;			/* true if contents changed */
	Boolean okToCloseIfChanged;	/* true if ok to close window with changes without saving */
	Boolean saved;				/* true if contents saved */
	Boolean autoFetched;		/* true if autofetched from host */
	Boolean headerShown;		/* true if header shown in article or 
								   message window */
	Boolean authorsShown;		/* true if authors shown in subject window */
	short panelHeight;			/* height of panel area below window title bar
								   in article and message windows */
	short groupNameHCoord;		/* h coord of group name in user group window */
	short numUnreadHCoord;		/* h coord of num unread articles in
								   user group window */
	short minusSignHCoord;		/* h coord of minus sign in subject window */
	short threadCountHCoord;	/* h coord of thread count in subject window */
	short checkHCoord;			/* h coord of Ã in subject window */
	short authorHCoord;			/* h coord of author in subject window */
	short authorWidth;			/* width of author field in subject window */
	short subjectHCoord;		/* h coord of subject in subject window */
	PolyHandle expandTriangle;	/* handle to expanded triangle polygon */
	PolyHandle collapseTriangle;/* handle to collapsed triangle polygon */
} TWindow;

typedef struct TPrefRec {

	Str255	newsServerName;		/* name of news server */
	Str255	textFont;			/* name of textedit font */
	short	textSize;			/* size of textedit font */
	Str255	listFont;			/* name of list font */
	short	listSize;			/* size of list font */
	Rect	groupWindowRect;	/* saved full group list window rect */
	Point	unused1;			/* unused (used to be stagerring offset) */
	Point	statusWindowLocn;	/* origin of status window */
	short	maxFetch;			/* max. number of articles to fetch */
	CStr255	name;				/* username */
	CStr255	host;				/* user's host */
	CStr255	fullName;			/* user's full name */
	CStr255	organization;		/* user's organization */
	char	signature[4096];	/* user's signature */
	Boolean	groupWindowVisible;	/* true if group window is visible */
	Boolean	unused4;			/* unused (used to be auto zoom windows) */
	Boolean	unused2;			/* unused (used to be offset winds from parent) */
	Boolean unused3;			/* unused (used to be most recent first) */
	Str255	mailServerName;		/* mail server name */
	CStr255	address;			/* user's e-mail address */
	
	/* 	Added in 1.2(NU) */
	char	magicCookie[12];	/* magic cookie */
	char	version[12];		/* version string */
	Boolean	areYouSureAlert;	/* true to present are you sure alert on 
								   postings */
								   
	/*	Added in 1.3d1 */
	Boolean	autoFetchnewsrc;	/* auto get/put .newsrc file */
	Boolean replyCC;			/* replies CC sender */
	
	/* 	Added in 1.3d4 */
	Boolean showHeaders;		/* true to show headers by default */
	Boolean showAuthors;		/* true to show authors in subject windows */
	Boolean showThreadsCollapsed;	/* true to show threads collapsed by default */

	/* Added in 2.0d9 */
	Boolean checkForNewGroups;	/* Check for new newsgroups at startup */
	Boolean showKilledArticles;	/* Killed articles are shown but marked as read */
	unsigned long groupCheckTime;	/* The last time we checked for new groups */
	char	remotePassword[32];	/* Password for remote .newsrc put/get */
	Boolean	savePassword;		/* Save password between sessions? */

	Boolean	expandHilited; 		/* Expand threads with highlighted articles */

	OSType	textCreator;		/* Creator code for saved text files */
	Str31	textCreatorName;	/* And the name of the app */
	Str31	textVolName;		/* Volume to store saved text files */
	long	textDirID;			/* and dirID */
	short	maxGroupNameWidth;	/* max group name width in current list font
								   and size, or 0 if needs to be recomputed */
	Boolean	useXPAT;			/* use XPAT for searching if available */
	
	/* Added in 2.0d10 */
	Boolean textDefaultDir;		/* true if using default vol/dirID for saved text files */
	char	remotePath[32];		/* .newsrc pathname for remote put/get */
	
	/* Added in 2.0d11 */
	
	Boolean	addSigSeparatorLine;/* true to add "-- " separator line before sig */
	
	/* Added in 2.0d12 */
	
	Boolean keypadShortcuts;	/* true if keypad shortcuts enabled */
	Boolean logActionsToFile;	/* true if logging enabled */
	
	/* Added in 2.0d14 */
	
	Boolean batchedGroupCmds;	/* use batched GROUP commands */
	Boolean noNewConnection;	/* don't establish new connection when checking for new articles */
	Boolean noModeReader;		/* don't send MODE READER command */
	
	/* Added in 2.0d16 */
	
	Boolean zoomWindows;		/* zoom windows */
	Boolean autoSaveOnQuit;		/* auto save on quit */

	char	fill[1347];			/* for future expansion */
	
} TPrefRec, *TPrefPtr;

/* Global variables. */

extern short		gNumGroups;			/* number of newsgroups */
extern TGroup		**gGroupArray;		/* handle to full group array */
extern Handle		gGroupNames;		/* handle to group name strings */
extern WindowPtr	gFullGroupWindow;	/* pointer to full group list window */
extern Boolean		gMustDoZoomOnShowFullGroupList;
										/* true if full group list needs zooming */

extern TPrefRec 	gPrefs;				/* preferences */

extern Boolean		gDone;				/* flag set true upon program termination */
extern Boolean 		gCancel;			/* flag set when user cancels an action */

extern Handle		gLifeBoat;			/* lifeboat memory -- de-allocated when 
										   memory gets low */
extern Boolean		gSinking;			/* flag set after lifeboat has been 
										   jettisoned */
extern Boolean 		gOutOfMemory;		/* flag set when out of memory - and luck */

extern Boolean		gInBackground;		/* background/foreground flag */
extern Rect			gDesktopExtent;		/* desktop extent */

extern Cursor		gIBeamCurs;			/* ibeam cursor */
extern Boolean		gHasColorQD;		/* true if we have color QuickDraw */
extern Rect			gWindLimits;		/* window grow limits */

extern CStr255		gAutoFetchHost;		/* autofetch host */
extern CStr255		gAutoFetchName;		/* autofetch username */
extern char			gAutoFetchPass[32];	/* autofetch password */
extern char			gAutoFetchPath[32];	/* autofetch path */

extern Rect			gDragRect;			/* drag limit rectangle */

extern Boolean		gStartingUp;		/* true during initialization */
extern Boolean		gStartupOK;			/* true if no errors during startup */

extern Boolean		gFullGroupListDirty;	/* true if full group list dirty */

extern FSSpec 		gPrefsFile;			/* preferences file location */
extern FSSpec		gFilterFile;		/* filter file location */
