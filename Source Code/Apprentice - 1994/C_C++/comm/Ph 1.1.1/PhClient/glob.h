/*______________________________________________________________________

	glob.h - Global Definitions.
_____________________________________________________________________*/

#ifndef __glob__
#define __glob__

#include "rez.h"
#include "oop.h"

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define longSleep			0x7fffffff	/* background idle sleep time in ticks */
#define shortSleep		15				/* foreground idle sleep time in ticks */
#define maxFieldName		18				/* max length of field name */
#define font				"\pMonaco"	/* font */
#define fontSize			9				/* font size */
#define staggerInitial	3				/* initial window staggering offset */
#define staggerOffset	20				/* subsequent staggering offset */
#define zoomSlop			3				/* pixels to leave on edges on zoom */
#define dragSlop			4				/* window drag slop in pixels */
#define intervalBB		10				/* tick interval between beachball spins */
#define numPosSave		10				/* number of window positions to save
													for the query, edit, and help windows */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

/* Window State. */

typedef struct WindState {
	Rect		userState;			/* user state rectangle */
	Boolean	zoomed;				/* true if in zoomed (standard) state */
	Boolean	moved;				/* true if moved or grown */
} WindState;

/* FieldInfo.

	The three fields name, attributes, and description are stored in
	both Pascal and C formats.  The first byte is the Pascal length byte,
	and a C zero terminator byte also follows the string.
	
*/

typedef struct FieldInfo {
	char			name[maxFieldName+2];	/* field name */
	Boolean		hasChangeAttr;				/* true if field has change attribute */
	Boolean		canEdit;						/* true if field can be edited */
	Boolean		dirty;						/* true if field has been changed */
	Boolean		putOK;						/* true if field put ok */
	Handle		servErrMsg;					/* handle to server error message */
	short			maxSize;						/* max size */
	char			**attributes;				/* handle to attributes */
	char			**description;				/* handle to description */
	char			**original;					/* handle to original text */
	short			origSize;					/* size of original text */
	short			nLines;						/* number of lines in field */
	TEHandle		edit;							/* handle to TextEdit record */
} FieldInfo;
																
typedef enum WindKind {
	editWind = firstUserWindKind,		/* edit window */
   queryWind,								/* query window */
	helpWind,								/* help window */
	firstModal,								/* place holder - all subsequent
													window kinds are for movable modal
													window */
	openModal,								/* open ph record */
	loginModal,								/* login */
	pswdModal,								/* change password */
	newModal,								/* new ph record */
	tranModal,								/* server transaction */
	siteModal,								/* default site */
} WindKind;

/*______________________________________________________________________

	Variables.
_____________________________________________________________________*/

extern short			NumMenus;			/* number of menus in menu bar */
extern MenuHandle		AppleMenu;			/* handle to apple menu */
extern MenuHandle		FileMenu;			/* handle to file menu */
extern MenuHandle		EditMenu;			/* handle to edit menu */
extern MenuHandle		ProxyMenu;			/* handle to proxy menu */
extern MenuHandle		WindowsMenu;		/* handle to windows menu */
extern MenuHandle		HelpMenu;			/* handle to help menu */
extern CursHandle		IBeamHandle;		/* handle to ibeam cursor */
extern CursHandle		WatchHandle;		/* handle to watch cursor */
extern CursHandle		BBArray[numBB];	/* array of handles to beachball cursors */
extern RgnHandle		BigClipRgn;			/* infinite clip region */
extern Boolean			Done;					/* true when time to quit */
extern short			PhPort;				/* Ph server port number */
extern short			FontNum;				/* monaco font number */
extern short			Ascent;				/* font ascent */
extern short			Descent;				/* font descent */
extern short			Leading;				/* font leading */
extern short			LineHeight;			/* font line height */
extern MenuHandle		SiteMenu;			/* handle to site popup menu */
extern short			NumSites;			/* number of sites in menu */
extern Handle			Servers;				/* handle to list of server 
														domain names */
extern Handle			Domains;				/* handle to list of email
														domain names */
extern Boolean			HaveBalloons;		/* true if balloon help available */
extern Handle			HelpTopics;			/* handle to help topic list, or nil if
														help not available */
extern Boolean			HelpNative;			/* true if only native help text avail */
extern Str255			HelpServer;			/* current help server */
extern short			HelpMenuLength;	/* length of initial part of help menu */
extern unsigned long	LastSiteUpdate;	/* time of last site list update */
extern unsigned long	LastHelpUpdate;	/* time of last help menu update */
extern WindState		TranState;			/* saved transaction window state */
extern WindState		LoginState;			/* saved login window state */
extern WindState		NewState;			/* saved new window state */
extern WindState		OpenState;			/* saved open window state */
extern WindState		PswdState;			/* saved pswd window state */
extern WindState		SiteState;			/* saved site window state */
extern WindState		QueryStates[numPosSave];	/* saved query window states */
extern WindState		EditStates[numPosSave];		/* saved edit window states */
extern WindState		HelpStates[numPosSave];		/* saved help window states */
extern Str255			DefaultServer;		/* default server domain name */
extern Str255			DbOffMsg;			/* database turned off error message */

/*______________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void glob_Error (short rezID, short index, Str255 p0);
extern void glob_ErrorCode (OSErr rCode);
extern void glob_ServErrorCode (short sCode);
extern void glob_CheckPswdSel (DialogPtr theDialog);
extern Boolean glob_FilterPswdChar (DialogPtr theDialog, char key, 
	short modifiers, Str255 pswd);
extern void glob_FilterPswdEditCmd (DialogPtr theDialog, short theItem, 
	Str255 pswd);
extern Boolean glob_FilterAsciiChar (DialogPtr theDialog, 
	char key, short modifiers);
extern Boolean glob_FilterPaste (void);
extern void glob_DrawSitePopup (Rect *rect, short curSel);
extern short glob_GetSiteIndex (Str255 server);
extern void glob_PopupSiteMenu (Rect *rect, short oldSel, short *newSel,
	Str255 newDomain);
extern void glob_BringToFront (void);

#endif