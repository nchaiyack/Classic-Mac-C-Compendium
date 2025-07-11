/* MUDDoc - Implementation of TMUDDoc                                         */

#include "MUDDoc.h"


		// � Toolbox
#ifndef NUM_ALT_ADDRS
#include "AddressXlation.h"
#endif

#ifndef __CMINTF__
#include "CMIntf.h"
#endif

#ifndef __CRMINTF__
#include "CRMIntf.h"
#endif

#ifndef __ERRORS__
#include "Errors.h"
#endif

#ifndef __FONTS__
#include "Fonts.h"
#endif

#ifndef __OSEVENTS__
#include "OSEvents.h"
#endif

#ifndef __RESOURCES__
#include "Resources.h"
#endif

#ifndef __STRINGS__
#include "Strings.h"
#endif

#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif


		// � MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif

#ifndef __UMenuSetup__
#include "UMenuSetup.h"
#endif


		// � Implementation use
#ifndef __DOCDIALOGS__
#include "DocDialogs.h"
#endif

#ifndef __EDSTUFF__
#include "EDStuff.h"
#endif

#ifndef __FILTERS__
#include "Filters.h"
#endif

#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MTPSTUFF__
#include "MTPStuff.h"
#endif

#ifndef __NOTRACE__
#include "NoTrace.h"
#endif

#ifndef __USizerView__
#include "USizerView.h"
#endif


//------------------------------------------------------------------------------

struct Header {
	unsigned long creator;
	long version;
};

struct DocInfo1 {
	Boolean doConnect;
	unsigned char endLine;
	Boolean skip;
	Boolean echo;
	Boolean update;
	char filler1;
	char rtab;
	char stab;
	unsigned char intChar;
	char useCTB;
	long tcpAddr;
	short tcpPort;
	short mtpPort;
	short logSize;
	short histSize;
	long mtpDirID;
	VPoint wLoc;
	VPoint wSize;
	Size configSize;
};

struct DocInfo2 {
	Boolean useCTB, doConnect, skip, echo;
	Boolean update, useTelnet, parseVT100, cvtTab;
	unsigned char endLine, intChar, protocol, filler1;
	short rtab, stab, fontSize, tileHeight;
	short tcpPort, mtpPort;
	short logSize, histSize;
	long mtpDirID;
	unsigned long textSig;
	VPoint wLoc, wSize;
	Size configSize;
};

struct DocInfo3 {
	Boolean autoScroll, filler1;
	short maxCmdSize;
	short numMacros;
};

const Header currentHeader = {kSignature, 3};
const int kCMBufferSize = 4 * 1024;
const int kTCPBufferSize = 16 * 1024;
const Rect kMaxChooseSize = {0, 0, 308, 498};
const int kTmpBufSize = 512;
const unsigned long kTextSig = 'MPS ';
const unsigned char *kCreatorApp = "\pMPW Shell";
const unsigned char *kHostName = "\p0.0.0.0";

//------------------------------------------------------------------------------

#pragma segment MACommandRes

class TMUDCmd: public TCommand {
public:
	virtual pascal void DoIt (void);
	virtual pascal void IMUDCmd (TMUDDoc *itsDocument, CmdNumber itsCmdNumber);
};


pascal void TMUDCmd::DoIt (void)
{
	switch (fCmdNumber) {
	case cConfigure:
		if (((TMUDDoc *) fChangedDocument)->fUseCTB)
			((TMUDDoc *) fChangedDocument)->Configure ();
		else
			ConfigTCP ((TMUDDoc *) fChangedDocument);
		break;
	case cUseCTB:
		((TMUDDoc *) fChangedDocument)->UseCTB ();
		break;
	case cPrefs:
		Preferences ((TMUDDoc *) fChangedDocument);
		break;
	case cComm:
		Communication ((TMUDDoc *) fChangedDocument);
		break;
	case cLogFile:
		if (((TMUDDoc *) fChangedDocument)->fLogFilter->fLogging)
			((TMUDDoc *) fChangedDocument)->fLogFilter->CloseLog ();
		else
			((TMUDDoc *) fChangedDocument)->fLogFilter->OpenLog ();
		break;
	case cConnect:
		if (((TMUDDoc *) fChangedDocument)->fConnected)
			((TMUDDoc *) fChangedDocument)->Disconnect ();
		else
			((TMUDDoc *) fChangedDocument)->Connect ();
		break;
	case cUpdate:
		((TMUDDoc *) fChangedDocument)->Update ();
		break;
	case cUpdateAll:
		((TMUDDoc *) fChangedDocument)->UpdateAll ();
		break;
	case cUpload:
		((TMUDDoc *) fChangedDocument)->Upload ();
		break;
	case cDownload:
		((TMUDDoc *) fChangedDocument)->Download ();
		break;
	case cSetup:
		Setup ((TMUDDoc *) fChangedDocument);
		break;
	case cMacros:
		EditMacros (fChangedDocument);
		break;
	case cBreak:
		((TMUDDoc *) fChangedDocument)->Break ();
		break;
	case cInterrupt:
		((TMUDDoc *) fChangedDocument)->Interrupt ();
		break;
	case cSendFile:
		((TMUDDoc *) fChangedDocument)->SendFile ();
		break;
	}
}


pascal void TMUDCmd::IMUDCmd (TMUDDoc *itsDocument, CmdNumber itsCmdNumber)
{
	ICommand (itsCmdNumber, itsDocument, NULL, NULL);
	fCanUndo = FALSE;
	fCausesChange = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

class TMUDIdler: public TEvtHandler {
public:
	TMUDDoc *fDocument;
	
	virtual pascal Boolean DoIdle (IdlePhase phase);
	virtual pascal void IMUDIdler (TMUDDoc *itsDocument);
};


pascal Boolean TMUDIdler::DoIdle (IdlePhase phase)
{
	fDocument->DoIdle (phase);
	return FALSE;
}


pascal void TMUDIdler::IMUDIdler (TMUDDoc *itsDocument)
{
	IEvtHandler (NULL);
	fIdleFreq = 0;
	fDocument = itsDocument;
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

class TMUDUpdater: public TWindow {
public:
	virtual pascal void DoChoice (TView *origView, short itsChoice);
};


pascal void TMUDUpdater::DoChoice (TView *, short itsChoice)
{
	if (itsChoice == mButtonHit) gStop = TRUE;
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void ASRDummy (StreamPtr , unsigned short , Ptr , unsigned short ,
		struct ICMPReport *)
{
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::Break (void)
{
	if (fUseCTB && fConnected) CMBreak (fConn, 2, FALSE, NULL);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::CheckConnection (void)
{
	CMBufferSizes sizes;
	CMStatFlags status;
	TCPiopb pb;
	OSErr error;
	int state;

	if (fConnected) {
		if (fUseCTB) {
			fConnected = (fConn != NULL) &&
				(CMStatus (fConn, sizes, &status) == noErr) &&
				((status & (cmStatusOpen + cmStatusOpening)) != 0);
		} else {
			pb.ioNamePtr = NULL;
			pb.ioCRefNum = gTCPRef;
			pb.tcpStream = fTCPStream;
			pb.csCode = TCPStatus;
			pb.csParam.status.userDataPtr = (Ptr) fBlock;
			PBControl ((ParamBlockRec *) &pb, FALSE);
			state = (pb.csParam.status.connectionState & -2);
			fConnected = (pb.ioResult == noErr) && (state != 0) &&
				(state != 2) && (state != 20) && (state != 14);
			if (!fConnected && (pb.ioResult == noErr) && (state != 0)) {
				pb.ioNamePtr = NULL;
				pb.ioCRefNum = gTCPRef;
				pb.tcpStream = fTCPStream;
				pb.csCode = TCPAbort;
				PBControl ((ParamBlockRec *) &pb, FALSE);
			}
		}
		if (!fConnected) {
			fBlock->wtBufIn = 0;
			fBlock->wtBufOut = 0;
			fBlock->wtBufChars = 0;
			InvalidateMenuBar ();
			gApplication->SetupTheMenus ();
		}
		if (fBlock->ioError != noErr) {
			error = fBlock->ioError;
			fBlock->ioError = noErr;
			gApplication->ShowError (error, 0);
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TMUDDoc::Close (void)
{
	TLogoutMacro *logout;
	
	if (fClosing) return;
	if (fConnected) {
		logout = new TLogoutMacro;
		FailNIL (logout);
		logout->IMacro (this, fLogout);
		logout->fDoneCmd = cClose;
		return;
	}
	if (fLogFilter->fLogging) fLogFilter->CloseLog ();
	InvalidateMenuBar ();
	gIdlePhase = idleBegin;
	inherited::Close();
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TMUDDoc::CloseConnection (void)
{
	TCPiopb pb;

	Disconnect ();
	if (fUseCTB) {
		CMActivate (fConn, FALSE);
		if (fConn) {
			HUnlock ((Handle) fConn);
			CMDispose (fConn);
			fConn = NULL;
		}
	} else {
		pb.ioNamePtr = NULL;
		pb.ioCRefNum = gTCPRef;
		pb.tcpStream = fTCPStream;
		pb.csCode = TCPRelease;
		PBControl ((ParamBlockRec *) &pb, FALSE);
	}
	if (fBuffer) {
		DisposIfPtr (fBuffer);
		fBuffer = NULL;
	}
	if (fIdler) {
		gApplication->InstallCohandler (fIdler, FALSE);
		DisposIfHandle (fIdler);
		fIdler = NULL;
	}
	fDoConnect = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::Configure ()
{
	short result;
	ConnHandle tmp;
	Str255 name;
	Rect r;
	Point pt;
	Boolean wasPermanent;
	
	r = kMaxChooseSize;
	CenterRectOnScreen (&r, TRUE, TRUE, TRUE);
	pt.v = r.top;
	pt.h = r.left;
	CMActivate (fConn, FALSE);
	wasPermanent = PermAllocation (TRUE);
	HUnlock ((Handle) fConn);
	tmp = fConn;
	result = CMChoose (&tmp, pt, NULL);
	fConn = tmp;
	MoveHHi ((Handle) fConn);
	HLock ((Handle) fConn);
	wasPermanent = PermAllocation (wasPermanent);
	CMActivate (fConn, TRUE);
	switch (result) {
	case chooseDisaster:
	case chooseFailed:
		Failure (0, msgOpenFailed);
		break;
	case chooseOKMinor:
		++fChangeCount;
		break;
	case chooseOKMajor:
		++fChangeCount;
		CMGetToolName ((*fConn)->procID, name);
		CopyStr255 (name, fToolName);
		break;
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void ResultProc (struct hostInfo *, char *)
{
}

pascal void ResolverHandler (short , long , void * )
{
	CloseResolver ();
}

pascal void TMUDDoc::Connect (void)
{
	TCPiopb *pbp;
	FailInfo fi;
	OSErr err;
	Str255 tmp;
	hostInfo info;
	TLoginMacro *login;

	if (!fConnected) {
		fBlock->rdBufIn = 0;
		fBlock->rdBufOut = 0;
		fBlock->rdBufChars = 0;
		fBlock->wtBufIn = 0;
		fBlock->wtBufOut = 0;
		fBlock->wtBufChars = 0;
		fBlock->wtCMChars = 0;
		fBlock->ioError = noErr;
		if (fUseCTB) {
			FailCMErr (CMOpen (fConn, FALSE, NULL, -1));
			CMSetUserData (fConn, (long) fBlock);
			fConnected = TRUE;
			fDoConnect = TRUE;
		} else {
			CopyStr255 (fHostName, tmp);
			P2CStr (tmp);
			FailOSErr (OpenResolver (NULL));
			CatchFailures (&fi, ResolverHandler, kDummyLink);
			err = StrToAddr ((char *) tmp, &info, ResultProc, NULL);
			while (err == cacheFault) err = info.rtnCode;
			FailOSErr (err);
			Success (&fi);
			FailOSErr (CloseResolver ());
			login = new TLoginMacro;
			FailNIL (login);
			login->IMacro (this, fLogin);
			login->fStartTicks = TickCount ();
			fTCPAddr = info.addr [0];
			pbp = &fBlock->pb;
			pbp->ioCompletion = NULL;
			pbp->ioNamePtr = NULL;
			pbp->ioCRefNum = gTCPRef;
			pbp->tcpStream = fTCPStream;
			pbp->csCode = TCPActiveOpen;
			pbp->csParam.open.ulpTimeoutValue = kTCPTimeout;
			pbp->csParam.open.ulpTimeoutAction = 1;
			pbp->csParam.open.validityFlags = timeoutValue + timeoutAction;
			pbp->csParam.open.commandTimeoutValue = kTCPTimeout;
			pbp->csParam.open.remoteHost = fTCPAddr;
			pbp->csParam.open.remotePort = fTCPPort;
			pbp->csParam.open.localPort = 0;
			pbp->csParam.open.tosFlags = 0;
			pbp->csParam.open.precedence = 0;
			pbp->csParam.open.dontFrag = FALSE;
			pbp->csParam.open.timeToLive = 0;
			pbp->csParam.open.security = 0;
			pbp->csParam.open.optionCnt = 0;
			pbp->csParam.open.userDataPtr = (Ptr) fBlock;
			PBControl ((ParamBlockRec *) pbp, TRUE);
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TMUDDoc::Disconnect (void)
{
	TCPiopb pb;

	CheckConnection ();
	if (fConnected) {
		fConnected = FALSE;
		if (fUseCTB) {
			CMIOKill (fConn, cmDataIn);
			CMIOKill (fConn, cmDataOut);
			fBlock->wtCMChars = 0;
			CMClose (fConn, FALSE, NULL, 0, TRUE);
		} else {
			pb.ioNamePtr = NULL;
			pb.ioCRefNum = gTCPRef;
			pb.tcpStream = fTCPStream;
			pb.csCode = TCPAbort;
			PBControl ((ParamBlockRec *) &pb, FALSE);
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TMUDDoc::DoCMEvent (ConnHandle hConn, EventInfo *evtInfo)
{
	if (fUseCTB && (fConn != NULL) && (fConn == hConn)) {
		CMEvent (fConn, evtInfo->thePEvent);
		return TRUE;
	} else
		return FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TMUDDoc::DoCMMenu (short theMenu, short theItem)
{
	if (fUseCTB && (fConn != NULL))
		return CMMenu (fConn, theMenu, theItem);
	else
		return FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::DoCMResume (Boolean switchIn)
{
	if (fUseCTB && (fConn != NULL)) CMResume (fConn, switchIn);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes
// WtCompletor must be in same segment as CMtoOSErr!!!

static Boolean gIdleEntered = FALSE;

pascal void WtCompletor (ConnHandle hConn)
{
	IOBlock *blockPtr;
	OSErr err;
	
	blockPtr = (IOBlock *) (**hConn).userData;
		// Don't use CMGetUserData unless it is in the same segment!
	blockPtr->wtBufOut += blockPtr->wtCMChars;
	if (blockPtr->wtBufOut >= kRWBufferSize) blockPtr->wtBufOut = 0;
	blockPtr->wtBufChars -= blockPtr->wtCMChars;
	blockPtr->wtCMChars = 0;
	err = CMtoOSErr ((**hConn).errCode);
	if ((err != noErr) && (blockPtr->ioError == noErr))
		blockPtr->ioError = err;
}

void IOCompletion (struct TCPiopb *iopb)
{
	IOBlock *blockPtr;
	
	blockPtr = (IOBlock *) iopb->csParam.send.userDataPtr;
	blockPtr->wtBufOut += blockPtr->wtCMChars;
	if (blockPtr->wtBufOut >= kRWBufferSize) blockPtr->wtBufOut = 0;
	blockPtr->wtBufChars -= blockPtr->wtCMChars;
	blockPtr->wtCMChars = 0;
	if ((iopb->ioResult < 0) && (blockPtr->ioError == noErr))
		blockPtr->ioError = iopb->ioResult;
}

pascal void IdleEnteredHandler (short , long , void *thisDocument)
{
	long count;
	IOBlock *blockPtr;
	
	gIdleEntered = FALSE;
	blockPtr = ((TMUDDoc *) thisDocument)->fBlock;
	count = blockPtr->rdBufChars;
	if (blockPtr->rdBufOut + count > kRWBufferSize)
		count = kRWBufferSize - blockPtr->rdBufOut;
	blockPtr->rdBufOut += count;
	if (blockPtr->rdBufOut >= kRWBufferSize) blockPtr->rdBufOut = 0;
	blockPtr->rdBufChars -= count;
}

pascal Boolean TMUDDoc::DoIdle (IdlePhase phase)
{
	long count;
	CMFlags flags;
	CMStatFlags sflags;
	CMBufferSizes bsizes;
	TCPiopb pb;
	FailInfo fi;
	TMacro *oldmac;
	
	while (fRunList) {
		oldmac = fRunList;
		fRunList->DoIdle ();
		if (oldmac == fRunList) break; // loop only if the macro freed itself
	}
	if ((gEventLevel <= 1) && (phase == idleContinue) && !fRunList &&
			fLogWindow->fIsActive)
		gStop = FALSE;
	if (fUseCTB && (fConn != NULL)) CMIdle (fConn);
	CheckConnection ();
	if (fConnected) {
		if (fBlock->wtBufChars > 0) {
			if (fBlock->wtCMChars == 0) {
				count = fBlock->wtBufChars;
				if (fBlock->wtBufOut + count > kRWBufferSize)
					count = kRWBufferSize - fBlock->wtBufOut;
				if (fUseCTB) {
					fBlock->wtCMChars = count;
					FailCMErr (CMWrite (fConn, (Ptr) (fBlock->wtBuf +
						fBlock->wtBufOut), &count, cmData, TRUE,
						(ProcPtr) WtCompletor, -1, 0));
				} else {
					fBlock->pb.ioCompletion = IOCompletion;
					fBlock->pb.ioNamePtr = NULL;
					fBlock->pb.ioCRefNum = gTCPRef;
					fBlock->pb.tcpStream = fTCPStream;
					fBlock->pb.csCode = TCPSend;
					fBlock->pb.csParam.send.ulpTimeoutValue = kTCPTimeout;
					fBlock->pb.csParam.send.ulpTimeoutAction = 1;
					fBlock->pb.csParam.send.validityFlags = timeoutValue +
						timeoutAction;
					fBlock->pb.csParam.send.pushFlag = TRUE;
					fBlock->pb.csParam.send.urgentFlag = FALSE;
					fBlock->pb.csParam.send.wdsPtr =
						(Ptr) &(fBlock->wdsLength);
					fBlock->pb.csParam.send.sendFree = 0;
					fBlock->pb.csParam.send.sendLength = 0;
					fBlock->pb.csParam.send.userDataPtr = (Ptr) fBlock;
					fBlock->wdsLength = (unsigned short) count;
					fBlock->wdsPtr = (Ptr) (fBlock->wtBuf +
						fBlock->wtBufOut);
					fBlock->wdsEndmarker = 0;
					pb.csParam.send.userDataPtr = (Ptr) fBlock;
					fBlock->wtCMChars = count;
					PBControl ((ParamBlockRec *) &fBlock->pb, TRUE);
					if ((fBlock->pb.ioResult != 1) &&
							(fBlock->ioError == noErr))
						FailOSErr (fBlock->pb.ioResult);
				}
			}
		}
		if (fBlock->rdBufChars < kRWBufferSize) {
			count = kRWBufferSize - fBlock->rdBufChars;
			if (fBlock->rdBufIn + count > kRWBufferSize)
				count = kRWBufferSize - fBlock->rdBufIn;
			if (fUseCTB) {
				FailCMErr (CMStatus (fConn, bsizes, &sflags));
				if (((sflags & cmStatusClosing) == 0) &&
						(bsizes [cmDataIn] > 0)) {
					FailCMErr (CMRead (fConn, (Ptr) (fBlock->rdBuf +
						fBlock->rdBufIn), &count, cmData, FALSE, NULL, 0,
						&flags));
				} else
					count = 0;
			} else {
				pb.ioNamePtr = NULL;
				pb.ioCRefNum = gTCPRef;
				pb.tcpStream = fTCPStream;
				pb.csCode = TCPStatus;
				pb.csParam.status.userDataPtr = (Ptr) fBlock;
				FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
				if (pb.csParam.status.amtUnreadData < count)
					count = pb.csParam.status.amtUnreadData;
				if ((count > 0) && (pb.csParam.status.connectionState == 8)) {
					pb.ioNamePtr = NULL;
					pb.ioCRefNum = gTCPRef;
					pb.tcpStream = fTCPStream;
					pb.csCode = TCPRcv;
					pb.csParam.receive.commandTimeoutValue = 1;
					pb.csParam.receive.rcvBuff = (Ptr) (fBlock->rdBuf +
						fBlock->rdBufIn);
					pb.csParam.receive.rcvBuffLen = (unsigned short) count;
					pb.csParam.receive.rdsPtr = NULL;
					pb.csParam.receive.rdsLength = 0;
					pb.csParam.receive.userDataPtr = (Ptr) fBlock;
					PBControl ((ParamBlockRec *) &pb, FALSE);
					if (pb.ioResult != noErr) {
						if (fBlock->ioError == noErr)
							FailOSErr (pb.ioResult);
						else
							count = 0; // CheckConnection will display the error
					} else
						count = pb.csParam.receive.rcvBuffLen;
				}
			}
			if (count > 0) {
				fBlock->rdBufIn += count;
				if (fBlock->rdBufIn >= kRWBufferSize) fBlock->rdBufIn = 0;
				fBlock->rdBufChars += count;
			}
		}
	} else if (fBlock->wtBufChars > 0) {
		fBlock->wtBufChars = 0;
		fBlock->wtBufOut = fBlock->wtBufIn;
	}
	if (!gIdleEntered && (fBlock->rdBufChars > 0)) {
		CatchFailures (&fi, IdleEnteredHandler, (void *) this);
		gIdleEntered = TRUE;
		count = fBlock->rdBufChars;
		if (fBlock->rdBufOut + count > kRWBufferSize)
			count = kRWBufferSize - fBlock->rdBufOut;
		count = fInputFilter->Filter (fBlock->rdBuf + fBlock->rdBufOut, count);
		if (count > 0) {
			fBlock->rdBufOut += count;
			if (fBlock->rdBufOut >= kRWBufferSize) fBlock->rdBufOut = 0;
			fBlock->rdBufChars -= count;
		}
		Success (&fi);
		gIdleEntered = FALSE;
	}
	return FALSE;
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TMUDDoc::DoInitialState (void)
{
	short fontSize;
	Str255 tmp;
	
	fToolName [0] = 0;
	fUseCTB = !gTCPAvailable;
	fDoConnect = FALSE;
	fSkipCR = TRUE;
	fEcho = TRUE;
	fDoEcho = TRUE;
	fUpdate = TRUE;
	fUseTelnet = TRUE;
	fParseVT100 = TRUE;
	fCvtTab = TRUE;
	fEndLine = 1;
	fInterrupt = 3;
	fProtocol = 0;
	fTabSize = 8;
	fSTab = 4;
	fFontSize = 9;
	fTileHeight = 0;
	fTCPPort = 0;
	fMTPPort = 0;
	fLogSize = kStdLogSize;
	fHistSize = kStdHistSize;
	fTextSig = kTextSig;
	fLoc = gZeroVPt;
	fSize = gZeroVPt;
	MTPDirSetup (this, gDefVRefNum, gDefDirID);
	fPlayer [0] = 0;
	fPasswd [0] = 0;
	fFontNum = monaco;
	fontSize = fFontSize;
	GetPortFontInfo (fFontNum, tmp, &fontSize);
	CopyStr255 (tmp, fFontName);
	GetIndString (tmp, kMUDStrings, kmsHost);
	CopyStr255 (tmp, fHostName);
	GetIndString (tmp, kMUDStrings, kmsCreator);
	CopyStr255 (tmp, fCreatorApp);
	fMaxCmdSize = kMaxCmdSize;
	fAutoScroll = TRUE;
	fLogin = NewMacroRec ();
	fLogout = NewMacroRec ();
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TMUDDoc::DoMakeViews (Boolean forPrinting)
{
	TWindow *aWindow;

	if (forPrinting)
		Failure (0, msgPrintFailed);
	else
	{
		aWindow = NewTemplateWindow (kMUDWindID, this);
		FailNIL (aWindow);
		if ((fSize.h != 0) && (fSize.v != 0)) {
			aWindow->Locate (fLoc.h, fLoc.v, kDontInvalidate);
			aWindow->Resize (fSize.h, fSize.v, kDontInvalidate);
			aWindow->fStaggered = TRUE;
		}
		fLogWindow = (TLogWindow *) aWindow;
		OpenConnection ();
	}
}

//------------------------------------------------------------------------------

#pragma segment MASelCommand

pascal TCommand *TMUDDoc::DoMenuCommand (CmdNumber aCmdNumber)
{
	TMUDCmd *aMUDCmd;

	switch (aCmdNumber) {
	case cConfigure:
	case cUseCTB:
	case cPrefs:
	case cComm:
	case cLogFile:
	case cConnect:
	case cUpdate:
	case cUpdateAll:
	case cUpload:
	case cDownload:
	case cSetup:
	case cBreak:
	case cInterrupt:
	case cSendFile:
	case cMacros:
		aMUDCmd = new TMUDCmd;
		FailNIL (aMUDCmd);
		aMUDCmd->IMUDCmd (this, aCmdNumber);
		return aMUDCmd;
	default:
		return inherited::DoMenuCommand (aCmdNumber);
	}
}

//------------------------------------------------------------------------------

#pragma segment MAReadFile

pascal void TMUDDoc::DoNeedDiskSpace (long *dataForkBytes, long *)
{
	MHandle mh;
	
	inherited::DoNeedDiskSpace (dataForkBytes, dataForkBytes);
	*dataForkBytes += sizeof (Header) + fToolName [0] + 1 + sizeof (DocInfo2) +
		fMTPVol [0] + 1 + fPlayer [0] + 1 + fPasswd [0] + 1 + sizeof (long) +
		GetHandleSize ((Handle) fUpdateList);
	if (fUseCTB) {
		if (fConfig) DisposIfPtr (fConfig);
		fConfig = CMGetConfig (fConn);
		if (fConfig != NULL) {
			*dataForkBytes += GetPtrSize (fConfig);
			DisposIfPtr (fConfig);
			fConfig = NULL;
		}
	}
	*dataForkBytes += fFontName[0] + 1 + fHostName[0] + 1 + fCreatorApp[0] + 1;
	*dataForkBytes += sizeof (DocInfo3) + MacroRecSize (fLogin) +
		 MacroRecSize (fLogout);
	mh = fMacroList;
	while (mh) {
		*dataForkBytes += MacroRecSize (mh);
		mh = (**mh).mNext;
	}
}

//------------------------------------------------------------------------------

#pragma segment MAReadFile

pascal void DoReadHandler (short , long , void * )
{
	CloseResolver ();
}

pascal void TMUDDoc::DoRead (short aRefNum, Boolean rsrcExists,
		Boolean forPrinting)
{
	Header head;
	short fontSize;
	long count, len, configSize;
	Str255 tmp;
	DocInfo1 dInfo1;
	DocInfo2 dInfo2;
	DocInfo3 dInfo3;
	SignedByte oldState;
	EventRecord theEvent;
	FailInfo fi;
	MHandle mh;
	int i;
	
	OSEventAvail (everyEvent, &theEvent);
	count = sizeof (currentHeader);
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &head));
	if ((head.creator != kSignature) || (head.version < 1) ||
			(head.version > 3))
		Failure (errDocFormat, 0);
	if (head.version < 3) {
		CopyStr255 (*fTitle, tmp);
		ParamText (tmp, "\p", "\p", "\p");
		MacAppAlert (phConvertID, NULL);
		++fChangeCount;
	}
	inherited::DoRead (aRefNum, rsrcExists, forPrinting);
	count = 1;
	FailOSErr (FSRead (aRefNum, &count, (Ptr) tmp));
	count = tmp [0];
	FailOSErr (FSRead (aRefNum, &count, (Ptr) (tmp + 1)));
	CopyStr255 (tmp, fToolName);
	if (head.version == 1) {
		++fChangeCount;
		count = sizeof (dInfo1);
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &dInfo1));
		fDoConnect = dInfo1.doConnect;
		fEndLine = dInfo1.endLine;
		fSkipCR = dInfo1.skip;
		fEcho = dInfo1.echo;
		fUpdate = dInfo1.update;
		fTabSize = dInfo1.rtab;
		fSTab = dInfo1.stab;
		fInterrupt = dInfo1.intChar;
		fUseCTB = dInfo1.useCTB;
		fTCPAddr = dInfo1.tcpAddr;
		fTCPPort = dInfo1.tcpPort;
		fMTPPort = dInfo1.mtpPort;
		fLogSize = dInfo1.logSize / 12;
		if (fLogSize < 10) fLogSize = 10;
		fHistSize = dInfo1.histSize;
		fMTPDirID = dInfo1.mtpDirID;
		fLoc = dInfo1.wLoc;
		fSize = dInfo1.wSize;
		configSize = dInfo1.configSize;
		fUseTelnet = TRUE;
		fParseVT100 = TRUE;
		fCvtTab = TRUE;
		fProtocol = 0;
		fFontSize = 9;
		fTileHeight = (int) (fSize.v - 17);
		fTextSig = kTextSig;
	} else {
		count = sizeof (dInfo2);
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &dInfo2));
		fUseCTB = dInfo2.useCTB;
		fDoConnect = dInfo2.doConnect;
		fSkipCR = dInfo2.skip;
		fEcho = dInfo2.echo;
		fUpdate = dInfo2.update;
		fUseTelnet = dInfo2.useTelnet;
		fParseVT100 = dInfo2.parseVT100;
		fCvtTab = dInfo2.cvtTab;
		fEndLine = dInfo2.endLine;
		fInterrupt = dInfo2.intChar;
		fProtocol = dInfo2.protocol;
		fTabSize = dInfo2.rtab;
		fSTab = dInfo2.stab;
		fFontSize = dInfo2.fontSize;
		fTileHeight = dInfo2.tileHeight;
		fTCPPort = dInfo2.tcpPort;
		fMTPPort = dInfo2.mtpPort;
		fLogSize = dInfo2.logSize;
		fHistSize = dInfo2.histSize;
		fMTPDirID = dInfo2.mtpDirID;
		fTextSig = dInfo2.textSig;
		fLoc = dInfo2.wLoc;
		fSize = dInfo2.wSize;
		configSize = dInfo2.configSize;
	}
	if ((theEvent.modifiers & optionKey) != 0) fDoConnect = FALSE;
	fDoEcho = fEcho;
	if (fUseCTB && !gCTBAvailable) {
		fUseCTB = FALSE;
		fDoConnect = FALSE;
		StdAlert (phUnavailID);
		++fChangeCount;
	} else if (!fUseCTB && !gTCPAvailable) {
		fUseCTB = TRUE;
		fDoConnect = FALSE;
		StdAlert (phUnavailID);
		++fChangeCount;
	}
	count = 1;
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
	count = tmp [0];
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
	CopyStr255 (tmp, fMTPVol);
	count = 1;
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
	count = tmp [0];
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
	CopyStr255 (tmp, fPlayer);
	count = 1;
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
	count = tmp [0];
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
	CopyStr255 (tmp, fPasswd);
	count = sizeof (long);
	FailOSErr (FSRead (aRefNum, &count, (Ptr) &len));
	SetPermHandleSize ((Handle) fUpdateList, len);
	oldState = HGetState ((Handle) fUpdateList);
	HLock ((Handle) fUpdateList);
	count = len;
	FailOSErr (FSRead (aRefNum, &count, (Ptr) *fUpdateList));
	HSetState ((Handle) fUpdateList, oldState);
	if (fConfig) DisposIfPtr (fConfig);
	if (configSize != 0) {
		fConfig = NewPermPtr (configSize);
		count = configSize;
		FailOSErr (FSRead (aRefNum, &count, fConfig));
	}
	MTPDirCheck (this);
	if (head.version == 1) {
		fFontNum = monaco;
		fontSize = fFontSize;
		GetPortFontInfo (fFontNum, tmp, &fontSize);
		CopyStr255 (tmp, fFontName);
		FailOSErr (OpenResolver (NULL));
		CatchFailures (&fi, DoReadHandler, kDummyLink);
		FailOSErr (AddrToStr (fTCPAddr, (char *) tmp));
		Success (&fi);
		FailOSErr (CloseResolver ());
		C2PStr ((Ptr) tmp);
		CopyStr255 (tmp, fHostName);
		CopyStr255 ((StringPtr) kCreatorApp, fCreatorApp);
	} else {
		count = 1;
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
		count = tmp [0];
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
		CopyStr255 (tmp, fFontName);
		fFontNum = GetFontNum (tmp);
		count = 1;
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
		count = tmp [0];
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
		CopyStr255 (tmp, fHostName);
		count = 1;
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &tmp));
		count = tmp [0];
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &(tmp [1])));
		CopyStr255 (tmp, fCreatorApp);
	}
	if (head.version < 3) {
		fMaxCmdSize = kMaxCmdSize;
		fAutoScroll = TRUE;
		fLogin = NewMacroRec ();
		fLogout = NewMacroRec ();
	} else {
		count = sizeof (dInfo3);
		FailOSErr (FSRead (aRefNum, &count, (Ptr) &dInfo3));
		fMaxCmdSize = dInfo3.maxCmdSize;
		fAutoScroll = dInfo3.autoScroll;
		fLogin = ReadMacroRec (aRefNum);
		fLogout = ReadMacroRec (aRefNum);
		if (dInfo3.numMacros > 0) fMacroList = ReadMacroRec (aRefNum);
		mh = fMacroList;
		for (i = 1; i < dInfo3.numMacros; i++) {
			(**mh).mNext = ReadMacroRec (aRefNum);
			mh = (**mh).mNext;
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::DoSetupMenus (void)
{
	Boolean okTrans;
	
	if (gDisableMenus)
		TEvtHandler::DoSetupMenus ();
	else {
		inherited::DoSetupMenus ();
		CheckConnection ();
		Enable (cConfigure, TRUE);
		SetMenuState (cConfigure, kMUDStrings, kmsToolSetup, kmsTCPSetup,
			fUseCTB);
		EnableCheck (cUseCTB, gCTBAvailable && gTCPAvailable, fUseCTB);
		Enable (cPrefs, TRUE);
		Enable (cComm, TRUE);
		Enable (cSetup, TRUE);
		Enable (cMacros, !fRunning);
		Enable (cLogFile, TRUE);
		SetMenuState (cLogFile, kMUDStrings, kmsOpenLog, kmsCloseLog,
			fLogFilter->fLogging);
		Enable (cConnect, (fBuffer != NULL) && !fRunning && !fClosing);
		SetMenuState (cConnect, kMUDStrings, kmsConnect, kmsDisconnect,
			fConnected);
		okTrans = fConnected && !fRunning && !IsHandlePurged (pMemReserve);
		if (okTrans && (fProtocol == 0)) okTrans = !fUseCTB && (fMTPPort != 0)
			&& (fPlayer [0] != 0) && (fPasswd [0] != 0);
		Enable (cUpdate, okTrans);
		Enable (cUpdateAll, okTrans);
		Enable (cUpload, okTrans);
		Enable (cDownload, okTrans);
		Enable (cBreak, fUseCTB && fConnected && !fRunning);
		Enable (cInterrupt, fConnected && !fRunning);
		Enable (cSendFile, fConnected && !fRunning);
	}
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

pascal void TMUDDoc::Download (void)
{
	Str255 path, fileName;
	short vRefNum;
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	TEditText *aEditText;
	
	if (fConnected) {
		aWindow = NewTemplateWindow (kDownloadID, NULL);
		FailNIL (aWindow);
		aEditText = (TEditText *) aWindow->FindSubView ('path');
		CopyStr255 (fLastPath, path);
		aEditText->SetText (path, kDontRedraw);
		aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
		okPressed = aDialogView->PoseModally () == 'OK  ';
		if (okPressed) aEditText->GetText (path);
		aWindow->Close ();
		if (!okPressed) Failure (noErr, msgCancelled);
		CopyStr255 (path, fLastPath);
		if (path [0] == 0) Failure (noErr, msgCancelled);
		RequestName (path, fileName, &vRefNum);
		if (fProtocol == 0)
			MTPReceive (this, path, fileName, vRefNum);
		else
			EDReceive (this, path, fileName, vRefNum);
	}
}

//------------------------------------------------------------------------------

#pragma segment MAWriteFile

pascal void TMUDDoc::DoWrite (short aRefNum, Boolean makingCopy)
{
	long count, len;
	Str255 tmp;
	DocInfo2 dInfo;
	DocInfo3 dInfo3;
	TSizerView *aSizerView;
	SignedByte oldState;
	VRect sizerRect;
	MHandle mh;
	short i;
	
	fLoc = fLogWindow->fLocation;
	fSize = fLogWindow->fSize;
	aSizerView = (TSizerView *) fLogWindow->FindSubView ('hors');
	sizerRect = aSizerView->GetSizerRect (1);
	fTileHeight = (int) sizerRect.top;
	count = sizeof (currentHeader);
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &currentHeader));
	inherited::DoWrite (aRefNum, makingCopy);
	CopyStr255 (fToolName, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	dInfo.useCTB = fUseCTB;
	dInfo.doConnect = fDoConnect;
	dInfo.skip = fSkipCR;
	dInfo.echo = fEcho;
	dInfo.update = fUpdate;
	dInfo.useTelnet = fUseTelnet;
	dInfo.parseVT100 = fParseVT100;
	dInfo.cvtTab = fCvtTab;
	dInfo.endLine = (unsigned char) fEndLine;
	dInfo.intChar = fInterrupt;
	dInfo.protocol = fProtocol;
	dInfo.filler1 = 0;
	dInfo.rtab = fTabSize;
	dInfo.stab = fSTab;
	dInfo.fontSize = fFontSize;
	aSizerView = (TSizerView *) fLogWindow->FindSubView ('hors');
	sizerRect = aSizerView->GetSizerRect (1);
	dInfo.tileHeight = (int) sizerRect.top;
	dInfo.tcpPort = fTCPPort;
	dInfo.mtpPort = fMTPPort;
	dInfo.logSize = (short) fLogSize;
	dInfo.histSize = (short) fHistSize;
	dInfo.mtpDirID = fMTPDirID;
	dInfo.textSig = fTextSig;
	dInfo.wLoc = fLoc;
	dInfo.wSize = fSize;
	if (fConfig) {
		DisposIfPtr (fConfig);
		fConfig = NULL;
	}
	if (fUseCTB) fConfig = CMGetConfig (fConn);
	if (!fUseCTB || !fConfig)
		dInfo.configSize = 0;
	else
		dInfo.configSize = GetPtrSize (fConfig);
	count = sizeof (dInfo);
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &dInfo));
	CopyStr255 ((StringPtr) fMTPVol, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	CopyStr255 ((StringPtr) fPlayer, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	CopyStr255 ((StringPtr) fPasswd, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	len = GetHandleSize ((Handle) fUpdateList);
	count = sizeof (long);
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &len));
	oldState = HGetState ((Handle) fUpdateList);
	HLock ((Handle) fUpdateList);
	count = len;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) *fUpdateList));
	HSetState ((Handle) fUpdateList, oldState);
	if (fConfig) {
		count = dInfo.configSize;
		FailOSErr (FSWrite (aRefNum, &count, fConfig));
		DisposIfPtr (fConfig);
		fConfig = NULL;
	}
	CopyStr255 ((StringPtr) fFontName, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	CopyStr255 ((StringPtr) fHostName, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	CopyStr255 ((StringPtr) fCreatorApp, tmp);
	count = tmp [0] + 1;
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &tmp));
	dInfo3.autoScroll = fAutoScroll;
	dInfo3.filler1 = FALSE;
	dInfo3.maxCmdSize = fMaxCmdSize;
	mh = fMacroList;
	i = 0;
	while (mh) {
		i += 1;
		mh = (**mh).mNext;
	}
	dInfo3.numMacros = i;
	count = sizeof (dInfo3);
	FailOSErr (FSWrite (aRefNum, &count, (Ptr) &dInfo3));
	WriteMacroRec (aRefNum, fLogin);
	WriteMacroRec (aRefNum, fLogout);
	mh = fMacroList;
	while (mh) {
		WriteMacroRec (aRefNum, mh);
		mh = (**mh).mNext;
	}
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TMUDDoc::Free (void)
{
	TFilter *aFilter;
	MHandle mh, mold;
	
	RemHandle ((Handle) this, gMyDocList);
	CloseConnection ();
	if (fUpdateList) DisposIfHandle (fUpdateList);
	if (fBlock) DisposIfPtr (fBlock);
	if (fConfig) DisposIfPtr (fConfig);
	while (fInputFilter) {
		aFilter = fInputFilter->fNext;
		fInputFilter->Free ();
		fInputFilter = aFilter;
	}
	if (fEchoFilter) fEchoFilter->Free ();
	if (fLogFilter) fLogFilter->Free ();
	while (fRunList) fRunList->Free ();
	DisposMacroRec (fLogin);
	DisposMacroRec (fLogout);
	mh = fMacroList;
	while (mh) {
		mold = mh;
		mh = (**mh).mNext;
		DisposMacroRec (mold);
	}
	inherited::Free ();
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TMUDDoc::IMUDDoc (void)
{
	fConn = NULL;
	fConfig = NULL;
	fBuffer = NULL;
	fIdler = NULL;
	fBlock = NULL;
	fUpdateList = NULL;
	fLogWindow = NULL;
	fInputFilter = NULL;
	fEchoFilter = NULL;
	fLogFilter = NULL;
	fRunList = NULL;
	fLogin = NULL;
	fLogout = NULL;
	fMacroList = NULL;
	IDocument (kFileType, kSignature, kUsesDataFork, ! kUsesRsrcFork,
		! kDataOpen, ! kRsrcOpen);
	fBlock = (IOBlock *) NewPermPtr (sizeof (IOBlock));
	FailNIL (fBlock);
	fUpdateList = (unsigned char **) NewPermHandle (0);
	FailNIL (fUpdateList);
	fBlock->rdBufIn = 0;
	fBlock->rdBufOut = 0;
	fBlock->rdBufChars = 0;
	fBlock->wtBufIn = 0;
	fBlock->wtBufOut = 0;
	fBlock->wtBufChars = 0;
	fBlock->wtCMChars = 0;
	fConnected = FALSE;
	fRunning = FALSE;
	fClosing = FALSE;
	fTCPAddr = 0;
	fCurCol = 0;
	fLastPath [0] = 0;
	AddHandle ((Handle) this, gMyDocList);
	fInputFilter = new TTelnetFilter;
	FailNIL (fInputFilter);
	fInputFilter->IFilter (this);
	fInputFilter->fNext = new TVT100Filter;
	FailNIL (fInputFilter->fNext);
	fInputFilter->fNext->IFilter (this);
	fMacroFilter = new TMacroFilter;
	FailNIL (fMacroFilter);
	fInputFilter->fNext->fNext = fMacroFilter;
	fMacroFilter->IFilter (this);
	fLogFilter = new TLogFilter;
	FailNIL (fLogFilter);
	fLogFilter->IFilter (this);
	fEchoFilter = new TEchoFilter;
	FailNIL (fEchoFilter);
	fEchoFilter->IFilter (this);
	fLastMacItem = 1;
	fQuiet = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::Interrupt (void)
{
	unsigned char ch;
	
	if (fConnected) {
		ch = fInterrupt;
		Send (&ch, 1);
	}
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TMUDDoc::OpenConnection (void)
{
	TCPiopb pb;
	Str255 tmp;
	int procID;
	CMBufferSizes sizes = {kCMBufferSize, kCMBufferSize};
	Boolean toolFound, wasPermanent;

	if (fUseCTB) {
		CopyStr255 (fToolName, tmp);
		procID = CMGetProcID (tmp);
		if (procID == -1) {
			toolFound = (tmp [0] == 0);
			fDoConnect = FALSE;
			FailOSErr (CRMGetIndToolName (classCM, 1, tmp));
			procID = CMGetProcID (tmp);
			CopyStr255 (tmp, fToolName);
		} else
			toolFound = TRUE;
		if (procID == -1) Failure (errNoTools, msgOpenFailed);
		wasPermanent = PermAllocation (TRUE);
		fConn = CMNew (procID, cmData, sizes, 0, 0);
		wasPermanent = PermAllocation (wasPermanent);
		FailNIL (fConn);
		MoveHHi ((Handle) fConn);
		HLock ((Handle) fConn);
		if (fBuffer) DisposIfPtr (fBuffer);
		fBuffer = NewPermPtr (sizes [cmDataIn]);
		FailNIL (fBuffer);
		if (fConfig != NULL) {
			if (toolFound) CMSetConfig (fConn, fConfig);
			DisposIfPtr (fConfig);
			fConfig = NULL;
		}
		CMActivate (fConn, TRUE);
		if (!toolFound) {
			StdAlert (phUnavailID);
			++fChangeCount;
		}
	} else {
		if (fBuffer) DisposIfPtr (fBuffer);
		fBuffer = NewPermPtr (kTCPBufferSize);
		FailNIL (fBuffer);
		pb.ioNamePtr = NULL;
		pb.ioCRefNum = gTCPRef;
		pb.csCode = TCPCreate;
		pb.csParam.create.rcvBuff = fBuffer;
		pb.csParam.create.rcvBuffLen = kTCPBufferSize;
		pb.csParam.create.notifyProc = ASRDummy;
		pb.csParam.create.userDataPtr = (Ptr) fBlock;
		FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
		fTCPStream = pb.tcpStream;
	}
	if (fDoConnect) Connect ();
	fIdler = new TMUDIdler;
	FailNIL (fIdler);
	((TMUDIdler *) fIdler)->IMUDIdler (this);
	gApplication->InstallCohandler (fIdler, TRUE);
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

pascal void TMUDDoc::RequestName (Str255 path, Str255 name, short *vRefNum)
{
	SFReply reply;
	short dlgID, pos;
	Str255 tmp;
	Point dlgLoc;
	DlgHookProcPtr dlgHook;
	ModalFilterProcPtr filterProc;
	OSErr err;
	
	pos = path [0];
	for (pos = path [0]; (pos > 0) && (path [pos] != '/'); pos--);
	name [0] = path [0] - pos;
	for (int i = 1; i <= name [0]; i++) name [i] = path [pos + i];
	SFPutParms (cDownload, &dlgID, &dlgLoc, name, tmp, (Ptr *) &dlgHook,
		(Ptr *) &filterProc);
	gApplication->UpdateAllWindows ();
	SFPPutFile (dlgLoc, tmp, name, dlgHook, &reply, dlgID, filterProc);
	if (reply.good) {
		CopyStr255 (reply.fName, name);
		*vRefNum = reply.vRefNum;
		err = DeleteFile (name, *vRefNum);
		if (err != noErr && err != fnfErr) Failure (err, 0);
	} else
		Failure (noErr, msgCancelled);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::Send (unsigned char *theChars, long count)
{
	long act;
	
	if (fConnected && fDoEcho && !fQuiet) {
		act = 0;
		do {
			act += fEchoFilter->Filter (theChars + act, count - act);
		} while (act < count);
	}
	SendNoEcho (theChars, count);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

static short dRef, rRef;

pascal void SendFileHandler (short , long , void * )
{
	CloseFile (dRef, rRef);
}

pascal void TMUDDoc::SendFile (void)
{
	AppFile aFile;
	FailInfo fi;
	unsigned char buf[kTmpBufSize + 1];
	long count;
	OSErr err;
	
	if (fConnected && gApplication->ChooseDocument (cSendFile, &aFile)) {
		FailOSErr (MAOpenFile (aFile.fName, aFile.vRefNum, TRUE, FALSE,
			fsRdPerm, fsRdPerm, &dRef, &rRef));
		CatchFailures (&fi, SendFileHandler, kDummyLink);
		do {
			count = kTmpBufSize;
			err = FSRead (dRef, &count, (Ptr) &buf);
			if ((err != noErr) && (err != eofErr)) FailOSErr (err);
			Send ((unsigned char *) &buf, count);
		} while (count == kTmpBufSize);
		Success (&fi);
		FailOSErr (CloseFile (dRef, rRef));
		FailOSErr (FlushVol (NULL, aFile.vRefNum));
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

static Boolean wasDisabled;

pascal void SendNoEchoHandler (short , long , void * )
{
	gDisableMenus = FALSE;
}

pascal void TMUDDoc::SendNoEcho (unsigned char *theChars, long count)
{
	long act;
	unsigned char ch;
	FailInfo fi;
	
	wasDisabled = gDisableMenus;
	act = 0;
	while (count > act) {
		while ((count > act) && (fBlock->wtBufChars < kRWBufferSize - 1)) {
			ch = *theChars++;
			if (ch == chReturn) {
				switch (fEndLine) {
				case 0:
					fBlock->wtBuf [fBlock->wtBufIn] = chReturn;
					break;
				case 1:
					fBlock->wtBuf [fBlock->wtBufIn] = chReturn;
					++(fBlock->wtBufIn);
					if (fBlock->wtBufIn >= kRWBufferSize) fBlock->wtBufIn = 0;
					++(fBlock->wtBufChars);
					fBlock->wtBuf [fBlock->wtBufIn] = chLineFeed;
					break;
				case 2:
					fBlock->wtBuf [fBlock->wtBufIn] = chLineFeed;
					break;
				}
				fCurCol = 0;
			} else {
				fBlock->wtBuf [fBlock->wtBufIn] = ch;
				++fCurCol;
			}
			++(fBlock->wtBufIn);
			if (fBlock->wtBufIn >= kRWBufferSize) fBlock->wtBufIn = 0;
			++(fBlock->wtBufChars);
			++act;
		}
		if (count > act) {
			if (!gDisableMenus) {
				CatchFailures (&fi, SendNoEchoHandler, kDummyLink);
				gDisableMenus = TRUE;
				InvalidateMenuBar ();
			}
			gApplication->PollEvent (kAllowApplicationToSleep);
			if (gStop) Failure (noErr, msgCancelled);
		}
	}
	if (gDisableMenus && !wasDisabled) {
		Success (&fi);
		gDisableMenus = FALSE;
		InvalidateMenuBar ();
	}
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

static TWindow *anUpdater;

pascal void UpdateHandler (short , long , void * )
{
	anUpdater->Close();
}

pascal void TMUDDoc::Update (void)
{
	Str255 path;
	FailInfo fi;
	
	anUpdater = NewTemplateWindow (kUpdateID, NULL);
	FailNIL (anUpdater);
	CatchFailures (&fi, UpdateHandler, kDummyLink);
	gApplication->CommitLastCommand ();
	anUpdater->Open();
	anUpdater->Select();
	do {
		gApplication->PollEvent (kAllowApplicationToSleep);
		if (gStop) Failure (noErr, msgCancelled);
	} while (gIdlePhase != idleContinue);
	path [0] = 0;
	UpdateFrom (anUpdater, fMTPDirID, path);
	Success (&fi);
	anUpdater->Close();
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

pascal void TMUDDoc::UpdateAll (void)
{
	Str255 path;
	FailInfo fi;
	
	anUpdater = NewTemplateWindow (kUpdateID, NULL);
	FailNIL (anUpdater);
	CatchFailures (&fi, UpdateHandler, kDummyLink);
	gApplication->CommitLastCommand ();
	anUpdater->Open();
	anUpdater->Select();
	do {
		gApplication->PollEvent (kAllowApplicationToSleep);
		if (gStop) Failure (noErr, msgCancelled);
	} while (gIdlePhase != idleContinue);
	SetPermHandleSize ((Handle) fUpdateList, 0);
	path [0] = 0;
	UpdateFrom (anUpdater, fMTPDirID, path);
	Success (&fi);
	anUpdater->Close();
	++fChangeCount;
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

static short wdRef;

pascal void UpdateFileHandler (short , long , void * )
{
	WDPBRec pb;

	pb.ioNamePtr = NULL;
	pb.ioVRefNum = wdRef;
	PBCloseWD (&pb, FALSE);
}

pascal void TMUDDoc::UpdateFile (TWindow *anUpdater, long dirID, Str255 fName,
		unsigned long mDate, Str255 path)
{
	long listLen, offset, i, len, date;
	unsigned char *name, *tmp;
	AppFile theFile;
	WDPBRec pb;
	FailInfo fi;
	Str255 aStr;
	TStaticText *aStaticText;

	offset = 0;
	listLen = GetHandleSize ((Handle) fUpdateList);
	while (offset < listLen) {
		name = (unsigned char *) *fUpdateList + offset + sizeof (long);
		len = *name++;
		if (len == path [0]) {
			tmp = path + 1;
			for (i = len; i > 0; --i)
				if (*name++ != *tmp++) break;
			if (i == 0) break;
		}
		offset += len + 1 + sizeof (long);
	}
	if (offset >= listLen) {
		SetPermHandleSize ((Handle) fUpdateList, offset + sizeof (long) +
			path [0] + 1);
		date = 0;
		name = (unsigned char *) *fUpdateList + offset + sizeof (long);
		tmp = path;
		for (i = path [0]; i >= 0; --i) *name++ = *tmp++;
	} else {
		tmp = (unsigned char *) *fUpdateList + offset;
		date = *tmp++ << 24;
		date += *tmp++ << 16;
		date += *tmp++ << 8;
		date += *tmp;
	}
	if (date < mDate) {
		tmp = (unsigned char *) *fUpdateList + offset;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp = 0;
		aStaticText = (TStaticText *) ((TMUDUpdater *) anUpdater)->
			FindSubView ('name');
		aStaticText->SetText (path, kRedraw);
		pb.ioNamePtr = NULL;
		pb.ioVRefNum = fMTPVRefNum;
		pb.ioWDProcID = kSignature;
		pb.ioWDDirID = dirID;
		FailOSErr (PBOpenWD (&pb, FALSE));
		wdRef = pb.ioVRefNum;
		CatchFailures (&fi, UpdateFileHandler, kDummyLink);
		do {
			gApplication->PollEvent (kAllowApplicationToSleep);
			if (gStop) Failure (noErr, msgCancelled);
		} while (gIdlePhase != idleContinue);
		theFile.vRefNum = wdRef;
		CopyStr255 (fName, theFile.fName);
		if (fProtocol == 0)
			MTPSend (this, &theFile, path);
		else
			EDSend (this, &theFile, path);
		Success (&fi);
		pb.ioNamePtr = NULL;
		pb.ioVRefNum = wdRef;
		FailOSErr (PBCloseWD (&pb, FALSE));
		tmp = (unsigned char *) *fUpdateList + offset;
		*tmp++ = (unsigned char) ((mDate >> 24) & 0xFF);
		*tmp++ = (unsigned char) ((mDate >> 16) & 0xFF);
		*tmp++ = (unsigned char) ((mDate >> 8) & 0xFF);
		*tmp = (unsigned char) (mDate & 0xFF);
		if (fUpdate) {
			len = path [0];
			if ((len >= 2) && (path [len - 1] == '.') && (path [len] == 'c')) {
				path [0] = (unsigned char) (len - 2);
				GetIndString (aStr, kMUDStrings, kmsUpdate);
				Send (aStr + 1, aStr [0]);
				Send (path + 1, path [0]);
				gPrompt = FALSE;
				aStr [0] = chReturn;
				Send (aStr, 1);
				while (!gPrompt) {
					gApplication->PollEvent (kAllowApplicationToSleep);
					if (gStop) Failure (noErr, msgCancelled);
				}
			}
		}
		++fChangeCount;
	}
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

pascal void TMUDDoc::UpdateFrom (TWindow *anUpdater, long dirID, Str255 path)
{
	int index, len;
	OSErr err;
	CInfoPBRec pb;
	Str255 fName;
	
	len = path [0];
	pb.hFileInfo.ioNamePtr = fName;
	pb.hFileInfo.ioVRefNum = fMTPVRefNum;
	index = 1;
	do {
		pb.hFileInfo.ioFDirIndex = index;
		pb.hFileInfo.ioDirID = dirID;
			/* set to ioFlNum by GetCatInfo */
		err = PBGetCatInfo (&pb, FALSE);
		if (err != fnfErr) FailOSErr (err);
		if (err == noErr) {
			if (len > 0) PStrCat (path, "\p/");
			PStrCat (path, fName);
			if ((pb.hFileInfo.ioFlAttrib & ioDirMask) != 0)
				UpdateFrom (anUpdater, pb.hFileInfo.ioDirID, path);
			else
				UpdateFile (anUpdater, dirID, fName, pb.hFileInfo.ioFlMdDat,
					path);
			path [0] = len;
		}
		++index;
	} while (err == noErr);
}

//------------------------------------------------------------------------------

#pragma segment SFileTrans

pascal void TMUDDoc::Upload (void)
{
	AppFile aFile;
	Str255 path, aStr;
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	TEditText *aEditText;
	int len;
	
	if (fConnected && gApplication->ChooseDocument (cUpload, &aFile)) {
		MTPBuildPath (this, &aFile, path);
		aWindow = NewTemplateWindow (kUploadID, NULL);
		FailNIL (aWindow);
		aEditText = (TEditText *) aWindow->FindSubView ('path');
		aEditText->SetText (path, kDontRedraw);
		aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
		okPressed = aDialogView->PoseModally () == 'OK  ';
		if (okPressed) aEditText->GetText (path);
		aWindow->Close ();
		if (!okPressed) Failure (noErr, msgCancelled);
		if (fProtocol == 0)
			MTPSend (this, &aFile, path);
		else
			EDSend (this, &aFile, path);
		if (fUpdate) {
			len = path [0];
			if ((len >= 2) && (path [len - 1] == '.') && (path [len] == 'c')) {
				path [0] = len - 2;
				GetIndString (aStr, kMUDStrings, kmsUpdate);
				Send (aStr + 1, aStr [0]);
				Send (path + 1, path [0]);
				aStr [0] = chReturn;
				Send (aStr, 1);
			}
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMUDDoc::UseCTB (void)
{
	CloseConnection ();
	fUseCTB = !fUseCTB;
	fDoConnect = FALSE;
	OpenConnection ();
	++fChangeCount;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes
// CMtoOSErr must be in same segment as WtCompletor!!!

pascal OSErr CMtoOSErr (CMErr err)
{
	OSErr oe;
	
	oe = noErr;
	switch (err) {
	case cmGenericError:
		oe = errCMGeneric;
		break;
	case cmRejected:
		oe = errCMRejected;
		break;
	case cmFailed:
		oe = errCMFailed;
		break;
	case cmTimeOut:
		oe = errCMTimeOut;
		break;
	case cmNotOpen:
		oe = errCMNotOpen;
		break;
	case cmNotClosed:
		oe = errCMNotClosed;
		break;
	case cmNoRequestPending:
		oe = errCMNoRequest;
		break;
	case cmNotSupported:
		oe = errCMNotSup;
		break;
	case cmNoTools:
		oe = errCMNoTools;
		break;
	default:
		if (err > 0)
			oe = errCMGeneric;
		else
			oe = err;
		break;
	}
	return oe;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void FailCMErr (CMErr err)
{
	OSErr oe;
	
	oe = CMtoOSErr (err);
	if (oe != noErr) Failure (oe, 0);
}

//------------------------------------------------------------------------------

#pragma segment MAInit

pascal void InitMUDDoc (void)
{
	if (gDeadStripSuppression) {
		TMUDUpdater *aMUDUpdater = new TMUDUpdater;
	}
}

//------------------------------------------------------------------------------
