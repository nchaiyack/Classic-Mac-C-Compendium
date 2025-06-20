/*********************************************************/
/* Preferences */

#include <Types.h>

/* 06jun95,ejo: version++ */ 
#define kCurrentPrefsTypeVers 5
/*#define kCurrentPrefsTypeVers 4 */

#define	OnlyText	1
#define	Fetch		1
#define	Never		2
#define	UseMenu		3
#define	UseICinComp	4
#define	UseICinExp	4

#define	NonMacGzip	1
#define	G_UseMenu	2


#define	UseHQX		1
#define	Discard		2
#define	Skip		3
#define	Ask			4

/* 23may94,ejo: added this to get 68k aligned prefs */
#if GENERATINGPOWERPC
#pragma options align=mac68k
#endif

/* this structure will contain the prefs for the application */
typedef struct {
	long prefsTypeVers;  /* version number of prefs struct   */
	Boolean compress;
	Boolean	ascii;
	Boolean	force;
	int		level;
	Str255	suffix;			
	Str255	textcreator;	/* this should be fixed... */
	Str255	bincreator;
	Str255	bintype;
	Boolean	KeepOriginals;
	Boolean	QuitWhenDone;
	Boolean BeepWhenDone;
	short	AsciiCompress;
	short	AsciiUnCompress;
	short	WhenCompress;
	short	ResourceCompress;
	short	S_Unused[4];
	Boolean	DecodeHQX;
	Boolean	StKeysComp;
	Boolean	StKeysUnComp;
	Boolean	StKeysAlt;
	Boolean	B_Unused[4];
	Boolean	SavedPos;
	Point	SavedPoint;

} PrefsType;

extern PrefsType	currPrefs;
extern Boolean		prefsChangedFlag;

extern short	OpenPrefsResFile(SignedByte prefsPerm, Boolean createFlag, short StrID);
extern Boolean	DoPrefsDialog(PrefsType * thePrefsType );
extern Boolean	GetOrGeneratePrefs(PrefsType * thePrefsTypePtr, long versWantedLong );
extern OSErr	SavePrefs( PrefsType * thePrefsTypePtr );

extern Handle	ICmappings;

/* 23may94,ejo: reset alignment setting */
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif
