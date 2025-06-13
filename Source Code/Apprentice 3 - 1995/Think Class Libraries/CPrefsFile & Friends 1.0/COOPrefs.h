/*
 * COOPrefs.h
 *
 */



/********************************/

#pragma once

/********************************/

#include "CPrefsFile.h"

/********************************/

typedef struct {
	short defaultOverwriteExistingFiles;
	short sleepTime;
	short updateTicks;
	short fontSize;
	Str63 fontName;
} laconicPrefs, *laconicPrefsPtr, **laconicPrefsHndl;

/********************************/



class COOPrefs : public CPrefsFile {
	
public:
	
	const static OSType		itsFileType;
	
	void			ILaconicPrefs(void);
	
	Boolean		getDefaultOverwriteExistingFiles(void);
	
	short			getSleepTime(void);
	short			getUpdateTicks(void);
	
	void			getFontSize(short *theFontSize);
	void			getFontName(Str63 theFontName);
	
protected:
	
	laconicPrefsHndl		itsPrefsHndl;
	
	void			assignToPrefsHndl(Handle theHandle);
	Handle		valueOfPrefsHndl(void);
	short			sizeOfPrefs(void);
	
private:
	
} ;



/********************************/

extern COOPrefs *gOOPrefs;