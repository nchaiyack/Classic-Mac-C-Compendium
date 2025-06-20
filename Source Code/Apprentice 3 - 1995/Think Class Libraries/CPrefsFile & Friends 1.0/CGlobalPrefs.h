/*
 * CGlobalPrefs.h
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
} globalPrefsStruct, *globalPrefsPtr, **globalPrefsHndl;

/********************************/



class CGlobalPrefs : public CPrefsFile {
	
public:
	
	const static OSType		itsFileType;
	
	void			IGlobalPrefs(void);
	
protected:
	
	void			assignToPrefsHndl(Handle theHandle);
	Handle		valueOfPrefsHndl(void);
	short			sizeOfPrefs(void);
	
private:
	
} ;



/********************************/

extern globalPrefsHndl gPrefs;