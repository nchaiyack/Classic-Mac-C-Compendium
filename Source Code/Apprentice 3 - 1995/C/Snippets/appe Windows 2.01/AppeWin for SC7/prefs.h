// File "prefs.h" - 

#ifndef ____PREFS_HEADER____
#define ____PREFS_HEADER____

// * **************************************************************************** * //
// * **************************************************************************** * //

#define kCreatorType	'CWnd'
#define kPrefsFileType	'PREF'
#define kPrefsFileName	"\pAppeWin Prefs"

// * **************************************************************************** * //
// * **************************************************************************** * //
// Function Prototypes

Handle GetPrefs(long type, short version);
void WritePrefs(Handle prefs, long type, short version);
short OpenPrefsFile(void);
short CreatePrefsFile(FSSpecPtr prefsFile);

#endif  ____PREFS_HEADER____
