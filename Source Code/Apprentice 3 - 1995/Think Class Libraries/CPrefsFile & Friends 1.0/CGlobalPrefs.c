/*
 * CGlobalPrefs.c
 * A sample subclass of CPrefsFile.
 *
 * by Jamie McCarthy
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 *
 * This subclass of CPrefsFile stores the preferences handle in a global
 * variable.  The rest of the program accesses it directly;  for example,
 * to get the font size, your code could read
 *
 * 		short myFontSize;
 * 		myFontSize = (**gPrefs).fontSize;
 *
 * ...and to get the font name, you might use
 *
 * 		Str63 myFontName;
 * 		CopyPString(& (**gPrefs).fontName, myCopyOfTheFontName);
 *
 * The advantage of not going the "pure OOP" route is that you don't
 * have to type in every access method you're ever going to use.
 *
 * The disadvantage is that you may have to put special accessing code
 * into your program anyway.  For example, the code to get the font name
 * above.  Also note that the "defaultOverwriteExistingFiles" field,
 * while declared as "BOOL" in the ResEdit template, must be a "short"
 * in C!  Types.h defines "Boolean" as "unsigned char", but ResEdit
 * reserves 16 bits for a "BOOL"--you don't want to mix those up.
 * And since ResEdit stores "0x0100" for TRUE, you'll probably want to
 * test with "( (**gPrefs).defaultOverwriteExistingFiles != FALSE )".
 *
 */



/********************************/

#include "CGlobalPrefs.h"

/********************************/

#include <TBUtilities.h>

/********************************/

#define globalStrsID (1024)

/********************************/

globalPrefsHndl gPrefs;

/********************************/



void CGlobalPrefs::IGlobalPrefs(void)
{
	inherited::IPrefsFile(globalStrsID, 'PREF', TRUE);
}



void CGlobalPrefs::assignToPrefsHndl(Handle theHandle)
{
	gPrefs = (globalPrefsHndl) theHandle;
}



Handle CGlobalPrefs::valueOfPrefsHndl(void)
{
	return (Handle) gPrefs;
}



short CGlobalPrefs::sizeOfPrefs(void)
{
	return sizeof(globalPrefsStruct);
}



