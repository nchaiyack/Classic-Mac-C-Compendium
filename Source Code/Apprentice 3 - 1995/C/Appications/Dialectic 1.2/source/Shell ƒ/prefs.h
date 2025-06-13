/**********************************************************************\

File:		prefs.h

Purpose:	This is the header file for prefs.c

\**********************************************************************/

extern	Str255			gMyName;
extern	Str255			gMyOrg;

void SaveThePrefs(void);
int PreferencesInit(void);
void PrefsError(int err);
