#ifndef __MY_PREFS_H__
#define __MY_PREFS_H__

extern	Str255			gMyName;
extern	Str255			gMyOrg;

enum PrefErrorTypes		/* possible error codes */
{
	prefs_allsWell=0,				/* no error */
	prefs_diskReadErr,				/* error reading prefs file */
	prefs_diskWriteErr,				/* error writing prefs file */
	prefs_cantOpenPrefsErr,			/* error trying to open prefs file */
	prefs_cantCreatePrefsErr,		/* error trying to create new prefs file */
	prefs_noMorePrefsErr,			/* no more prefs found in prefs file */
	prefs_versionNotSupportedErr,	/* prefs file created by later version of program */
	prefs_virginErr,				/* prefs file didn't exist -- first time running */
	prefs_IDNotMatchErr				/* file ID in current prefs doesn't match program's ID */
};

#ifdef __cplusplus
extern "C" {
#endif

extern	void SaveThePrefs(void);
extern	void RememberOptions(void);
extern	Boolean RestoreOptions(void);
extern	enum PrefErrorTypes PreferencesInit(void);
extern	void PrefsError(enum PrefErrorTypes err);

#ifdef __cplusplus
}
#endif

#endif
