/* Filters - Implementation of the Telnet and VT100 filters                   */

#include "Filters.h"

		// � Toolbox
#ifndef __ERRORS__
#include "Errors.h"
#endif

#ifndef __GESTALTEQU__
#include "GestaltEqu.h"
#endif

#ifndef __RESOURCES__
#include "Resources.h"
#endif

#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif

#ifndef __STRING__
#include "String.h"
#endif

		// � Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif


//------------------------------------------------------------------------------

/* Telnet protocol constants */

const unsigned char	SE			= 240;
const unsigned char	NOP			= 241;
const unsigned char DM			= 242;
const unsigned char BREAK		= 243;
const unsigned char IP			= 244;
const unsigned char AO			= 245;
const unsigned char	AYT			= 246;
const unsigned char	EC			= 247;
const unsigned char EL			= 248;
const unsigned char GA			= 249;
const unsigned char	SB			= 250;
const unsigned char WILLTEL 	= 251;
const unsigned char WONTTEL 	= 252;
const unsigned char DOTEL		= 253;
const unsigned char DONTTEL 	= 254;
const unsigned char	IAC			= 255;

const unsigned char STNORM		= 0;
const unsigned char	NEGOTIATE	= 1;
const unsigned char ESCFOUND 	= 5;
const unsigned char IACFOUND 	= 6;
const unsigned char TTYPFOUND 	= 7;
const unsigned char SBFOUND 	= 9;

const unsigned char IS			= 0;
const unsigned char SEND 		= 1;

const unsigned char BINARY 		= 0;
const unsigned char ECHO		= 1;
const unsigned char SGA			= 3;
const unsigned char STATUS		= 5;
const unsigned char TIMING		= 6;
const unsigned char	TERMTYP		= 24;
const unsigned char GS			= 29;
const unsigned char NAWS		= 31;
const unsigned char	TERMSPEED	= 32;
const unsigned char REMOTEFLOW	= 33;
const unsigned char LINEMODE 	= 34;


//------------------------------------------------------------------------------

/* VT100 emulation constants */

const unsigned char chNUL = 0x00;
const unsigned char chENQ = 0x05;
const unsigned char chVT = 0x0B;
const unsigned char chFF = 0x0C;
const unsigned char chCAN = 0x18;
const unsigned char chSUB = 0x1A;
const unsigned char chDCS = 0x90;
const unsigned char chCSI = 0x9B;
const unsigned char chST = 0x9C;

const unsigned char vtNormal = 0;		// Normal mode
const unsigned char vtESC = 1;			// ESC received
const unsigned char vtESC2 = 2;			// ESC SP or ESC # received
const unsigned char vtESCn = 3;			// ESC [()*+] received
const unsigned char vtCSI = 4;			// CSI received
const unsigned char vtDCS = 5;			// DCS received
const unsigned char vtCSI0 = 6;			// CSI 0 received
const unsigned char vtCSIG = 7;			// CSI > received
const unsigned char vtCSIG0 = 8;		// CSI > 0 received
const unsigned char vtCSI5 = 9;			// CSI 5 received
const unsigned char vtCSI6 = 10;		// CSI 6 received

//------------------------------------------------------------------------------

const int kFilterBufSize = 2048;
const int kDoWrite = 9;
const int kS7DoWrite = 13;

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long DoFilter (TFilter *aFilter, unsigned char *theChars, long count)
{
	return aFilter->Filter (theChars, count);
}

pascal long TFilter::Filter (unsigned char *theChars, long count)
{
	long act;
	
	act = 0;
	do {
		if (fNext != NULL)
			act += DoFilter (fNext, theChars + act, count - act);
		else
			act += DoFilter (((TMUDDoc *) fDocument)->fLogFilter,
				theChars + act, count - act);
	} while (act < count);
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TFilter::IFilter (TDocument *itsDoc)
{
	fNext = NULL;
	IObject ();
	fDocument = itsDoc;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TTelnetFilter::Filter (unsigned char *theChars, long count)
{
	unsigned char ch, *cp, buf [kFilterBufSize];
	long act, out;
	
	if (((TMUDDoc *) fDocument)->fUseTelnet) {
		act = 0;
		out = 0;
		cp = buf;
		while ((count > act) && (out < kFilterBufSize)) {
			ch = *theChars++;
			++act;
			switch (fState) {
			case WILLTEL:
				switch (ch) {
				case ECHO:
					((TMUDDoc *) fDocument)->fDoEcho = FALSE;
					SendOption (DOTEL, ch);
					break;
				case SGA:
					SendOption (DOTEL, ch);
					break;
				default:
					SendOption (DONTTEL, ch);
					break;
				}
				fState = STNORM;
				break;
			case WONTTEL:
				switch (ch) {
				case ECHO:
					((TMUDDoc *) fDocument)->fDoEcho = TRUE;
					SendOption (DONTTEL, ch);
					break;
				default:
					break;
				}
				fState = STNORM;
				break;
			case DOTEL:
				switch (ch) {
				case ECHO:
					((TMUDDoc *) fDocument)->fDoEcho = TRUE;
					SendOption (WILLTEL, ch);
					break;
				case TERMTYP:
					if (((TMUDDoc *) fDocument)->fParseVT100)
						SendOption (WILLTEL, ch);
					else
						SendOption (WONTTEL, ch);
					break;
				default:
					SendOption (WONTTEL, ch);
					break;
				}
				fState = STNORM;
				break;
			case DONTTEL:
				fState = STNORM;
				break;
			case SB:
				if (ch == TERMTYP)
					fState = TTYPFOUND;
				else
					fState = SBFOUND;
				break;
			case TTYPFOUND:
				if (ch == SEND) {
					SendOption (SB, ch);
				}
				fState = SBFOUND;
				break;
			case SBFOUND:
				if (ch == IAC) fState = IACFOUND;
				break;
			case IACFOUND:
				if (ch == IAC) {
					*cp++ = ch;
					++out;
					fState = STNORM;
				} else
					fState = ch;
				break;
			case STNORM:
				if (ch == IAC)
					fState = IACFOUND;
				else {
					*cp++ = ch;
					++out;
				}
				break;
			default:
				fState = STNORM;
				break;
			}
		}
		inherited::Filter (buf, out);
		return act;
	} else
		return inherited::Filter (theChars, count);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TTelnetFilter::IFilter (TDocument *itsDoc)
{
	inherited::IFilter (itsDoc);
	fState = STNORM;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TTelnetFilter::SendOption (unsigned char kind, unsigned char parm)
{
	unsigned char opt[3];
	char *ttyp = "\377\372\30\0DEC-VT100\377\360"; // RFC 930, 990
	
	if (kind == SB) {
		((TMUDDoc *) fDocument)->SendNoEcho ((unsigned char *) ttyp, 15);
	} else {
		opt [0] = IAC;
		opt [1] = kind;
		opt [2] = parm;
		((TMUDDoc *) fDocument)->SendNoEcho ((unsigned char *) opt, 3);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TVT100Filter::Filter (unsigned char *theChars, long count)
{
	unsigned char ch, *cp, buf [kFilterBufSize];
	long act, out;
	
	act = 0;
	out = 0;
	cp = buf;
	while ((count > act) && (out < kFilterBufSize)) {
		ch = *theChars++;
		++act;
		switch (ch) {
		case chNUL:
		case chFwdDelete:
			break;
		case chCAN:
		case chSUB:
			fState = vtNormal;
			break;
		case chEscape:
			if (((TMUDDoc *) fDocument)->fParseVT100) fState = vtESC;
			break;
		default:
			switch (fState) {
			case vtESC:
				switch (ch) {
				case '[':
					fState = vtCSI;
					break;
				case 'P':
					fState = vtDCS;
					break;
				case 'Z':
					SendReport (kPrimDA);
					fState = vtNormal;
					break;
				case ' ':
				case '#':
					fState = vtESC2;
					break;
				case '(':
				case ')':
				case '*':
				case '+':
					fState = vtESCn;
					break;
				default:
					fState = vtNormal;
					break;
				}
				break;
			case vtESC2:
				fState = vtNormal;
				break;
			case vtESCn:
				if ((ch >= '0') && (ch <= '~')) fState = vtNormal;
				break;
			case vtCSI:
				switch (ch) {
				case '0':
					fState = vtCSI0;
					break;
				case '>':
					fState = vtCSIG;
					break;
				case '5':
					fState = vtCSI5;
					break;
				case '6':
					fState = vtCSI6;
					break;
				case 'c':
					SendReport (kPrimDA);
					fState = vtNormal;
					break;
				default:
					if (ch >= '@') fState = vtNormal;
					break;
				}
				break;
			case vtDCS:
				if (ch == chST) fState = vtNormal;
				break;
			case vtCSI0:
				if (ch == 'c') {
					SendReport (kPrimDA);
					fState = vtNormal;
					break;
				} else if (ch >= '@')
					fState = vtNormal;
				break;
			case vtCSIG:
				switch (ch) {
				case '0':
					fState = vtCSIG0;
					break;
				case 'c':
					SendReport (kSecDA);
					fState = vtNormal;
					break;
				default:
					if (ch >= '@') fState = vtNormal;
					break;
				}
				break;
			case vtCSIG0:
				if (ch == 'c') {
					SendReport (kSecDA);
					fState = vtNormal;
					break;
				} else if (ch >= '@')
					fState = vtNormal;
				break;
			case vtCSI5:
				if (ch == 'n') {
					SendReport (kStatus);
					fState = vtNormal;
					break;
				} else if (ch >= '@')
					fState = vtNormal;
				break;
			case vtCSI6:
				if (ch == 'n') {
					SendReport (kPosition);
					fState = vtNormal;
					break;
				} else if (ch >= '@')
					fState = vtNormal;
				break;
			default:
				switch (ch) {
				case chBell:
					gApplication->Beep (0);
					break;
				case chTab:
					*cp++ = ch;
					++out;
					++fPos;
					break;
				case chLineFeed:
				case chVT:
				case chFF:
					if (((TMUDDoc *) fDocument)->fParseVT100 ||
							((TMUDDoc *) fDocument)->fSkipCR || (fPos != 0)) {
						*cp++ = chReturn;
						++out;
						fPos = 0;
					}
					break;
				case chReturn:
					if (!((TMUDDoc *) fDocument)->fParseVT100 &&
							!((TMUDDoc *) fDocument)->fSkipCR) {
						*cp++ = chReturn;
						++out;
						fPos = 0;
					}
					break;
				case chCSI:
					if (((TMUDDoc *) fDocument)->fParseVT100)
						fState = vtCSI;
					else {
						*cp++ = ch;
						++out;
						++fPos;
					}
					break;
				case chDCS:
					if (((TMUDDoc *) fDocument)->fParseVT100)
						fState = vtDCS;
					else {
						*cp++ = ch;
						++out;
						++fPos;
					}
					break;
				default:
					if (!gPrompt && (ch == fPrompt) && (fPos == 0))
						gPrompt = TRUE;
					if ((ch >= ' ') && ((ch < chFwdDelete) ||
							!((TMUDDoc *) fDocument)->fParseVT100)) {
						*cp++ = ch;
						++out;
						++fPos;
					}
					break;
				}
				break;
			}
			break;
		}
	}
	inherited::Filter (buf, out);
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TVT100Filter::IFilter (TDocument *itsDoc)
{
	Str255 tmp;
	
	inherited::IFilter (itsDoc);
	fPos = 0;
	fState = vtNormal;
	gPrompt = FALSE;
	GetIndString (tmp, kMUDStrings, kmsPrompt);
	fPrompt = tmp [1];
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

static char *rch [4] = {
		"\33[?61;1c",			// VT102
		"\33[>1;10;0c",			// version 1.0, no options
		"\33[0n",				// no malfunction
		"\33[1;1R"				// cursor set at 1,1
	};

pascal void TVT100Filter::SendReport (VT100Report report)
{
	((TMUDDoc *) fDocument)->SendNoEcho ((unsigned char *) rch [report],
		strlen (rch [report]));
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TMacroFilter::Filter (unsigned char *theChars, long count)
{
	long act;
	unsigned char *cp, ch;
	TMacro *mac;
	
	if (fScan) {
		act = 0;
		cp = theChars;
		while (act < count) {
			ch = *cp++;
			act += 1;
			if (ch == fStr [fPos]) {
				fPos += 1;
				if (fPos > fStr [0]) break;
			} else if (ch == fStr [1])
				fPos = 2;
			else
				fPos = 1;
		}
		if (!((TMUDDoc *) fDocument)->fQuiet) inherited::Filter (theChars, act);
		if (fPos > fStr [0]) {
			fScan = FALSE;
			mac = ((TMUDDoc *) fDocument)->fRunList;
			if (mac) {
				mac->fCmdDone = TRUE;
				mac->DoIdle ();
			}
		}
	} else {
		act = count;
		if (!((TMUDDoc *) fDocument)->fQuiet) inherited::Filter (theChars, act);
	}
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TMacroFilter::IFilter (TDocument *itsDoc)
{
	inherited::IFilter (itsDoc);
	fScan = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TEchoFilter::Filter (unsigned char *theChars, long count)
{
	unsigned char ch, *cp, buf [kFilterBufSize];
	long act, out;
	
	act = 0;
	out = 0;
	cp = buf;
	while ((count > act) && (out < kFilterBufSize)) {
		ch = *theChars++;
		++act;
		switch (ch) {
		case chBell:
			gApplication->Beep (0);
			break;
		case chLineFeed:
		case chReturn:
			*cp++ = chReturn;
			++out;
			fPos = 0;
			break;
		default:
			*cp++ = ch;
			++out;
			++fPos;
			break;
		}
	}
	inherited::Filter (buf, out);
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TEchoFilter::IFilter (TDocument *itsDoc)
{
	inherited::IFilter (itsDoc);
	fPos = 0;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogFilter::CloseLog ()
{
	if (fLogging) {
		fLogging = FALSE;
		FailOSErr (CloseFile (fFileRef, kNoFileRefnum));
		FailOSErr (FlushVol (NULL, fVRefNum));
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TLogFilter::Filter (unsigned char *theChars, long count)
{
	long act;
	
	act = ((TMUDDoc *) fDocument)->fLogWindow->fLogView->AddText (theChars,
		count);
	if (fLogging) FailOSErr (FSWrite (fFileRef, &act, (Ptr) theChars));
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogFilter::IFilter (TDocument *itsDoc)
{
	inherited::IFilter (itsDoc);
	fLogging = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogFilter::OpenLog ()
{
	Str255 fileName;
	short vRefNum, vol, dRef, rRef;
	Boolean save, closeWD;
	SignedByte oldState;
	OSErr err;
	WDPBRec pb;
	TBHandle th;
	long count;
	
	RequestName (fileName, &vRefNum,&vol, &closeWD, &save);
	FailOSErr (Create (fileName, vRefNum, ((TMUDDoc *) fDocument)->fTextSig,
		'TEXT'));
	FailOSErr (MAOpenFile (fileName, vRefNum, TRUE, FALSE, fsRdWrPerm, fsRdPerm,
		&dRef, &rRef));
	fFileRef = dRef;
	if (closeWD) {
		pb.ioNamePtr = NULL;
		pb.ioVRefNum = vRefNum;
		PBCloseWD (&pb, FALSE);
		fVRefNum = vol;
	} else
		fVRefNum = vRefNum;
	if (save) {
		th = ((TMUDDoc *) fDocument)->fLogWindow->fLogView->fBT->fText;
		while (th) {
			count = (**th).bLength;
			oldState = HGetState ((Handle) th);
			HLock ((Handle) th);
			err = FSWrite (fFileRef, &count, (**th).bBuf);
			HSetState ((Handle) th, oldState);
			if (err != noErr) {
				CloseFile (fFileRef, kNoFileRefnum);
				FailOSErr (err);
			}
			th = (**th).bNext;
		}
		err = FlushVol (NULL, fVRefNum);
		if (err != noErr) {
			CloseFile (fFileRef, kNoFileRefnum);
			FailOSErr (err);
		}
	}
	fLogging = TRUE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

static Boolean doWrite;

pascal short MyLogFileHook (short item, DialogPtr theDialog)
{
	short type;
	Handle itemHdl;
	Rect box;
	
	if (item == kDoWrite) doWrite = !doWrite;
	if ((item == sfHookFirstCall) || (item == kDoWrite)) {
		GetDItem (theDialog, kDoWrite, &type, &itemHdl, &box);
		SetCtlValue ((ControlHandle) itemHdl, doWrite);
	}
	return item;
}

pascal short MyS7LogFileHook (short item, DialogPtr theDialog, void *)
{
	short type;
	Handle itemHdl;
	Rect box;
	
	if (GetWRefCon ((WindowPtr) theDialog) == sfMainDialogRefCon) {
		if (item == kS7DoWrite) doWrite = !doWrite;
		if ((item == sfHookFirstCall) || (item == kS7DoWrite)) {
			GetDItem (theDialog, kS7DoWrite, &type, &itemHdl, &box);
			SetCtlValue ((ControlHandle) itemHdl, doWrite);
		}
	}
	return item;
}

pascal void TLogFilter::RequestName (Str255 name, short *vRefNum,
				short *vol, Boolean *closeWD, Boolean *save)
{
	SFReply reply;
	StandardFileReply s7reply;
	short dlgID, preDocname, constTitle;
	Str255 tmp;
	Point dlgLoc;
	DlgHookProcPtr dlgHook;
	ModalFilterProcPtr filterProc;
	DialogTHndl dlogTemplate;
	Rect dialogRect;
	OSErr err;
	Boolean sys7;
	long response;
	WDPBRec pb;
	
	err = Gestalt (gestaltStandardFileAttr, &response);
	sys7 = (err == noErr) && (response & (1 << gestaltStandardFile58)) != 0;
	GetIndString (tmp, kMUDStrings, kmsLogName);
	if (ParseTitleTemplate (tmp, &preDocname, &constTitle) &&
			SubstituteInTitle (tmp, *((TMUDDoc *) fDocument)->fTitle,
				preDocname, constTitle))
		CopyStr255 (tmp, name);
	else
		CopyStr255 (*((TMUDDoc *) fDocument)->fTitle, name);
	((TMUDDoc *) fDocument)->SFPutParms (cDownload, &dlgID, &dlgLoc, name, tmp,
		(Ptr *) &dlgHook, (Ptr *) &filterProc);
	if (sys7)
		dlgID = phS7OpenLogID;
	else
		dlgID = phOpenLogID;
	dlogTemplate = (DialogTHndl) GetResource ('DLOG', dlgID);
	if (dlogTemplate != NULL) {
		dialogRect = (**dlogTemplate).boundsRect;
		CenterRectOnScreen (&dialogRect, TRUE, TRUE, TRUE);
		dlgLoc.v = dialogRect.top;
		dlgLoc.h = dialogRect.left;
	}
	GetIndString (tmp, kMUDStrings, kmsSaveLog);
	doWrite = TRUE;
	gApplication->UpdateAllWindows ();
	if (sys7) {
		CustomPutFile (tmp, name, &s7reply, dlgID, dlgLoc, MyS7LogFileHook,
			NULL, NULL, NULL, NULL);
		if (s7reply.sfGood) {
			CopyStr255 (s7reply.sfFile.name, name);
			pb.ioNamePtr = NULL;
			pb.ioVRefNum = s7reply.sfFile.vRefNum;
			pb.ioWDProcID = kSignature;
			pb.ioWDDirID = s7reply.sfFile.parID;
			FailOSErr (PBOpenWD (&pb, FALSE));
			*vRefNum = pb.ioVRefNum;
			*vol = s7reply.sfFile.vRefNum;
			*closeWD = TRUE;
			*save = doWrite;
			err = DeleteFile (name, *vRefNum);
			if ((err != noErr) && (err != fnfErr)) Failure (err, 0);
		} else
			Failure (noErr, msgCancelled);
	} else {
		SFPPutFile (dlgLoc, tmp, name, MyLogFileHook, &reply, dlgID,
			filterProc);
		if (reply.good) {
			CopyStr255 (reply.fName, name);
			*vRefNum = reply.vRefNum;
			*closeWD = FALSE;
			*save = doWrite;
			err = DeleteFile (name, *vRefNum);
			if ((err != noErr) && (err != fnfErr)) Failure (err, 0);
		} else
			Failure (noErr, msgCancelled);
	}
}

//------------------------------------------------------------------------------
