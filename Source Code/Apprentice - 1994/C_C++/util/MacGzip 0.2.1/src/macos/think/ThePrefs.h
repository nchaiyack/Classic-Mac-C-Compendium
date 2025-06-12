/*********************************************************/
/* Preferences */
#define kCurrentPrefsTypeVers 3

/* this structure will contain the prefs for the application */
typedef struct {
	long prefsTypeVers;  /* version number of prefs struct                */
	Boolean compress;
	Boolean	ascii;
	Boolean	force;
	int		level;
	Str255	suffix;
	Str255	textcreator;
	Boolean	QuitWhenDone;
	Boolean BeepWhenDone;
	Boolean	SavedPos;
	Point	SavedPoint;

} PrefsType;

extern PrefsType currPrefs;
extern Boolean prefsChangedFlag;

extern short OpenPrefsResFile(SignedByte prefsPerm, Boolean createFlag);
extern Boolean DoPrefsDialog(PrefsType * thePrefsType );
extern Boolean GetOrGeneratePrefs(PrefsType * thePrefsTypePtr, long versWantedLong );
extern OSErr SavePrefs( PrefsType * thePrefsTypePtr );

