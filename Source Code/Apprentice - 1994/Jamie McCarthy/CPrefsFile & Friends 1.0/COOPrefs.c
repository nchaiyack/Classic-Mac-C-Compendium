/*
 * COOPrefs.c
 * A sample subclass of CPrefsFile.
 *
 * by Jamie McCarthy
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 *
 * This subclass of CPrefsFile stores the preferences handle in a
 * protected class variable, only making the class itself available to
 * the rest of the program.  The preferences data can only be accessed
 * through access methods.
 *
 * The disadvantage of this method is that it requires a lot of extra
 * typing and a little extra object code.
 *
 * The advantage of this method is that it provides a consistent
 * interface to the data that won't change if you change how it's
 * stored.  It's just generally cleaner.
 *
 * This file and its header file also demonstrate how to declare a
 * constant class variable:  itsFileType.  It's cleaner than a #define,
 * works on any data type you like, and doesn't make object code.
 *
 */



/********************************/

#include "COOPrefs.h"

/********************************/

#include <TBUtilities.h>

/********************************/

#define ooStrsID (1024)

/********************************/

COOPrefs *gOOPrefs;

/********************************/



const OSType COOPrefs::itsFileType = 'PREF';



void COOPrefs::ILaconicPrefs(void)
{
	inherited::IPrefsFile(ooStrsID, this->itsFileType, TRUE);
}



void COOPrefs::assignToPrefsHndl(Handle theHandle)
{
	itsPrefsHndl = (laconicPrefsHndl) theHandle;
}



Handle COOPrefs::valueOfPrefsHndl(void)
{
	return (Handle) itsPrefsHndl;
}



short COOPrefs::sizeOfPrefs(void)
{
	return sizeof(laconicPrefs);
}



Boolean COOPrefs::getDefaultOverwriteExistingFiles(void)
{
	return (**itsPrefsHndl).defaultOverwriteExistingFiles != 0;
}



short COOPrefs::getSleepTime(void)
{
	return (**itsPrefsHndl).sleepTime;
}



short COOPrefs::getUpdateTicks(void)
{
	return (**itsPrefsHndl).updateTicks;
}



void COOPrefs::getFontSize(short *theFontSize)
{
	*theFontSize = (**itsPrefsHndl).fontSize;
}



void COOPrefs::getFontName(Str63 theFontName)
{
	CopyPString( (**itsPrefsHndl).fontName, theFontName );
}
