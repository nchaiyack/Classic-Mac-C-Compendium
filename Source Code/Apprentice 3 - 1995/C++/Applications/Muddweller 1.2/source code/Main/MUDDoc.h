/* MUDDoc - Implementation of TMUDDoc                                         */

#ifndef __MUDDOC__
#define __MUDDOC__

		// � Toolbox
#ifndef __CMINTF__
#include "CMIntf.h"
#endif

#ifndef ipBadLapErr
#include "MacTCPCommonTypes.h"
#endif

#ifndef TCPCreate
#include "TCPPB.h"
#endif


		// � MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif


		// � Implementation use
#ifndef __FILTERS__
#include "Filters.h"
#endif

#ifndef __LOGWINDOW__
#include "LogWindow.h"
#endif

#ifndef __MACROS__
#include "Macros.h"
#endif


const int kStdLogSize = 50;
const int kStdHistSize = 20;
const int kMaxCmdSize = 32000;

const int kRWBufferSize = 2048;
const int kTCPTimeout = 200;

struct IOBlock {
	unsigned short wdsLength;
	Ptr wdsPtr;
	short wdsEndmarker;
	long rdBufIn, rdBufOut, rdBufChars;
	long wtBufIn, wtBufOut, wtBufChars, wtCMChars;
	short ioError;
	TCPiopb pb;
	unsigned char rdBuf [kRWBufferSize], wtBuf [kRWBufferSize];
};

class TMUDDoc: public TDocument {
public:
	Boolean fUseCTB;
	long fTCPStream, fTCPAddr, fMTPDirID;
	short fTCPPort, fMTPPort, fMTPVRefNum, fFontNum, fFontSize;
	int fLogSize, fHistSize, fTabSize, fSTab, fEndLine, fCurCol;
	int fProtocol, fTileHeight;
	unsigned char fLastPath [50];
	unsigned char fInterrupt, **fUpdateList;
	Str255 fToolName, fHostName, fFontName;
	Str31 fMTPVol, fMTPDir, fPlayer, fPasswd, fCreatorApp;
	unsigned long fTextSig;
	Boolean fConnected, fDoConnect, fSkipCR, fEcho, fDoEcho, fUpdate;
	Boolean fRunning, fClosing, fUseTelnet, fParseVT100, fCvtTab;
	Boolean fAutoScroll, fQuiet;
	short fMaxCmdSize, fLastMacItem;
	VPoint fLoc, fSize;
	ConnHandle fConn;
	Ptr fConfig, fBuffer;
	IOBlock *fBlock;
	TEvtHandler *fIdler;
	TLogWindow *fLogWindow;
	TFilter *fInputFilter, *fEchoFilter;
	TMacroFilter *fMacroFilter;
	TLogFilter *fLogFilter;
	TMacro *fRunList;
	MHandle fLogin, fLogout;
	MHandle fMacroList;

	virtual pascal void Break (void);
	virtual pascal void CheckConnection (void);
	virtual pascal void Close (void);
	virtual pascal void CloseConnection (void);
	virtual pascal void Configure (void);
	virtual pascal void Connect (void);
	virtual pascal void Disconnect (void);
	virtual pascal Boolean DoCMEvent (ConnHandle hConn, EventInfo *evtInfo);
	virtual pascal Boolean DoCMMenu (short theMenu, short theItem);
	virtual pascal void DoCMResume (Boolean switchIn);
	virtual pascal Boolean DoIdle (IdlePhase phase);
	virtual pascal void DoInitialState (void);
	virtual pascal void DoMakeViews (Boolean forPrinting);
	virtual pascal TCommand *DoMenuCommand (CmdNumber aCmdNumber);
	virtual pascal void DoNeedDiskSpace (long *dataForkBytes,
		long *rsrcForkBytes);
	virtual pascal void DoRead (short aRefNum, Boolean rsrcExists,
		Boolean forPrinting);
	virtual pascal void DoSetupMenus (void);
	virtual pascal void Download (void);
	virtual pascal void DoWrite (short aRefNum, Boolean makingCopy);
	virtual pascal void Free (void);
	virtual pascal long GrowZone (long needed);
	virtual pascal void IMUDDoc (void);
	virtual pascal void Interrupt (void);
	virtual pascal void OpenConnection (void);
	virtual pascal void RequestName (Str255 path, Str255 name, short *vRefNum);
	virtual pascal void Send (unsigned char *theChars, long count);
	virtual pascal void SendFile (void);
	virtual pascal void SendNoEcho (unsigned char *theChars, long count);
	virtual pascal void Update (void);
	virtual pascal void UpdateAll (void);
	virtual pascal void UpdateFile (TWindow *anUpdater, long dirID,
		Str255 fName, unsigned long mDate, Str255 path);
	virtual pascal void UpdateFrom (TWindow *anUpdater, long dirID,
		Str255 path);
	virtual pascal void Upload (void);
	virtual pascal void UseCTB (void);
};


pascal OSErr CMtoOSErr (CMErr err);
pascal void FailCMErr (CMErr err);

pascal void InitMUDDoc (void);

#endif
