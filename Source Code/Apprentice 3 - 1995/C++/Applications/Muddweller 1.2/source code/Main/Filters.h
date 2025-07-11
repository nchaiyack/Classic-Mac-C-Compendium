/* Filters - Implementation of the Telnet and VT100 filters                   */

#ifndef __FILTERS__
#define __FILTERS__

		// � MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif


//------------------------------------------------------------------------------

typedef enum {kPrimDA, kSecDA, kStatus, kPosition} VT100Report;

Boolean gPrompt;

//------------------------------------------------------------------------------

class TFilter: public TObject {
public:
	TDocument *fDocument;
	TFilter *fNext;
	
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
};

//------------------------------------------------------------------------------

class TTelnetFilter: public TFilter {
public:
	unsigned char fState;
	
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
	virtual pascal void SendOption (unsigned char kind, unsigned char parm);
};

//------------------------------------------------------------------------------

class TVT100Filter: public TFilter {
public:
	int fPos;
	unsigned char fState, fPrompt;
	
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
	virtual pascal void SendReport (VT100Report report);
};

//------------------------------------------------------------------------------

class TMacroFilter: public TFilter {
public:
	Str255 fStr;
	int fPos;
	Boolean fScan;
	
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
};

//------------------------------------------------------------------------------

class TEchoFilter: public TFilter {
public:
	int fPos;

	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
};

//------------------------------------------------------------------------------

class TLogFilter: public TFilter {
public:
	Boolean fLogging;
	short fFileRef, fVRefNum;

	virtual pascal void CloseLog (void);
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void IFilter (TDocument *itsDoc);
	virtual pascal void OpenLog (void);
	virtual pascal void RequestName (Str255 name, short *vRefNum,
				short *vol, Boolean *closeWD, Boolean *save);
};

//------------------------------------------------------------------------------

#endif
