/* Macros - Implementation of the login/logout macros and key bindings        */

#ifndef __MACROS__
#define __MACROS__


		// ¥ MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif


//------------------------------------------------------------------------------

const int kCmdNone = 0;
const int kCmdSend = 1;
const int kCmdMatch = 2;
const int kCmdWait = 3;
const int kCmdPasswd = 4;
const int kCmdQuiet = 5;
const int kCmdEcho = 6;
const int kLastCmd = 6;

short gBusyState;
long gLastBusyTick;

//------------------------------------------------------------------------------

typedef struct MacroRec *MPtr, **MHandle;

struct MacroRec {
	MHandle mNext;
	Handle mText;
	Boolean mShift;
	Boolean mOption;
	Boolean mControl;
	unsigned char mChar;
	unsigned char mCode;
};

struct MacroDiskRec {
	Boolean shift;
	Boolean option;
	Boolean control;
	Boolean filler1;
	unsigned char ch;
	unsigned char code;
	short textLen;
};


pascal void DisposMacroRec (MHandle theRec);
pascal long MacroRecSize (MHandle theRec);
pascal MHandle NewMacroRec (void);
pascal MHandle ReadMacroRec (short theRefNum);
pascal void WriteMacroRec (short theRefNum, MHandle theRec);

pascal void EditMacros (TDocument *aDocument);

pascal void InitMacros (void);


//------------------------------------------------------------------------------

class TMacro: public TObject {
public:
	TMacro *fNext;
	TDocument *fDocument;
	MHandle fMRec;
	long fMPos;
	int fCmd;
	Str255 fParm;
	Boolean fCmdDone;
	long fWaitEnd;
	
	virtual pascal void DoEcho (void);
	virtual pascal void DoIdle (void);
	virtual pascal void DoMatch (void);
	virtual pascal void DoPasswd (void);
	virtual pascal void DoQuiet (void);
	virtual pascal void DoSend (void);
	virtual pascal void DoWait (void);
	virtual pascal void Free (void);
	virtual pascal void IMacro (TDocument *itsDoc, MHandle itsRec);
	virtual pascal void MacroDone (void);
};


//------------------------------------------------------------------------------

class TLoginMacro: public TMacro {
public:
	Boolean fOpenDone;
	long fStartTicks;
	
	virtual pascal void DoIdle (void);
	virtual pascal void IMacro (TDocument *itsDoc, MHandle itsRec);
};


//------------------------------------------------------------------------------

class TLogoutMacro: public TMacro {
public:
	Boolean fMacroDone;
	Boolean fClosing;
	CmdNumber fDoneCmd;

	virtual pascal void DoIdle (void);
	virtual pascal void Free (void);
	virtual pascal void IMacro (TDocument *itsDoc, MHandle itsRec);
	virtual pascal void MacroDone (void);
};


//------------------------------------------------------------------------------

#endif
