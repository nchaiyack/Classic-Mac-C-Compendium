/* EDStuff - Implementation of the ed-based file-transfer routines            */

#include "EDStuff.h"


		// ¥ Toolbox
#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif

		// ¥ Implementation use
#ifndef __FILTERS__
#include "Filters.h"
#endif

#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MTPSTUFF__
#include "MTPStuff.h"
#endif


//------------------------------------------------------------------------------

const int kTmpBufferSize = 2 * 1024;
const int kBlockSize = 1024;
const int kNumLines = 20;

static short dRef, rRef;
static Boolean done;
static TMUDDoc *doc;
static TFilter *filter;

//------------------------------------------------------------------------------

typedef enum {kerReturn, kerDone, kerSkip} ReceiveEchoState;

typedef enum {ksrStart, ksrReceive, ksrEnd, ksrSkip} ReceiveState;

class TReceiveFilter: public TFilter {
public:
	ReceiveEchoState fEcho;
	ReceiveState fState;
	unsigned char *fBuf, fEdPrompt, fPrompt;
	long fCurLine, fLinesRcv, fOut, fFilePos, fLastPos, fMatched;
	Boolean fPass, fBlocks;
	Str255 fUnrecognized;

	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void FlushBuffer (void);
	virtual pascal void Free (void);
	virtual pascal void IFilter (TDocument *itsDoc);
};

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal long TReceiveFilter::Filter (unsigned char *theChars, long count)
{
	unsigned char ch, c, *cp, buf [kTmpBufferSize], echo [kTmpBufferSize];
	long act, out, length, cnt, pos, lines, expected;
	Str255 tmp;
	
	act = 0;
	out = 0;
	cp = buf;
	while ((count > act) && (out < kTmpBufferSize)) {
		ch = *theChars++;
		++act;
		switch (fEcho) {
		case kerReturn:
			if ((ch == fEdPrompt) && (fState != ksrSkip) &&
					((fState != ksrReceive) || (fLinesRcv > 0))) {
				switch (fState) {
				case ksrStart:
					fPass = FALSE;
					if (fBlocks)
						GetIndString (tmp, kMUDStrings, kmsFirstBlk);
					else
						GetIndString (tmp, kMUDStrings, kmsFirst);
					((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
					fLastPos = fFilePos;
					fLinesRcv = 0;
					if (((TMUDDoc *) fDocument)->fDoEcho)
						fState = ksrReceive;
					else
						fState = ksrSkip;
					break;
				case ksrReceive:
					FlushBuffer ();
					if (fBlocks && (fLinesRcv >= 20))
						expected = 20;
					else
						expected = 1;
					if ((fLinesRcv > expected) &&
							((TMUDDoc *) fDocument)->fDoEcho) {
						length = fFilePos - fLastPos;
						lines = expected;
						FailOSErr (SetFPos (dRef, fsFromStart, fLastPos));
						while ((length > 0) && (lines < fLinesRcv)) {
							if (length > kTmpBufferSize)
								cnt = kTmpBufferSize;
							else
								cnt = length;
							FailOSErr (FSRead (dRef, &cnt, (Ptr) echo));
							pos = 0;
							while ((pos < cnt) && (lines < fLinesRcv)) {
								c = echo [pos];
								pos += 1;
								*cp++ = c;
								++out;
								if (out >= kTmpBufferSize) {
									inherited::Filter (buf, out);
									out = 0;
									cp = buf;
								}
								if (c == chReturn) lines += 1;
							}
						}
					}
					if (fMatched != fUnrecognized [0]) {
						if (fLinesRcv > expected) {
							FailOSErr (SetFPos (dRef, fsFromStart, fLastPos));
							if (fBlocks)
								GetIndString (tmp, kMUDStrings, kmsAgainBlk);
							else
								GetIndString (tmp, kMUDStrings, kmsAgain);
						} else {
							fLastPos = fFilePos;
							if (fBlocks)
								GetIndString (tmp, kMUDStrings, kmsNextBlk);
							else
								GetIndString (tmp, kMUDStrings, kmsNext);
						}
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						fLinesRcv = 0;
						if (!((TMUDDoc *) fDocument)->fDoEcho) fState = ksrSkip;
					} else if (fBlocks) {
						fBlocks = FALSE;
						FailOSErr (SetFPos (dRef, fsFromStart, fLastPos));
						if (fLastPos == 0)
							GetIndString (tmp, kMUDStrings, kmsAgain);
						else
							GetIndString (tmp, kMUDStrings, kmsNext);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						fLinesRcv = 0;
						if (!((TMUDDoc *) fDocument)->fDoEcho) fState = ksrSkip;
					} else {
						FailOSErr (SetEOF (dRef, fLastPos));
						GetIndString (tmp, kMUDStrings, kmsQuit);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						fState = ksrEnd;
					}
					break;
				default:
					if (ch != chReturn) fEcho = kerSkip;
					if (fPass) {
						*cp++ = ch;
						++out;
					}
					break;
				}
			} else if ((ch == fPrompt) && (fState == ksrEnd)) {
				fEcho = kerDone;
				done = TRUE;
				*cp++ = ch;
				++out;
			} else {
				if (fState == ksrReceive) {
					if (ch == fUnrecognized [1])
						fMatched = 1;
					else
						fMatched = -1;
					fLinesRcv += 1;
					*(fBuf + fOut) = ch;
					fOut += 1;
					if (fOut >= kTmpBufferSize) FlushBuffer ();
				}
				if (fPass) {
					*cp++ = ch;
					++out;
				}
				if (ch != chReturn)
					fEcho = kerSkip;
				else if (fState == ksrSkip)
					fState = ksrReceive;
			}
			break;
		case kerSkip:
			if (fState == ksrReceive) {
				if (fMatched > 0) {
					fMatched += 1;
					if (ch != fUnrecognized [fMatched]) fMatched = -1;
				}
				*(fBuf + fOut) = ch;
				fOut += 1;
				if (fOut >= kTmpBufferSize) FlushBuffer ();
			}
			if (fPass) {
				*cp++ = ch;
				++out;
			}
			if (ch == chReturn) {
				fEcho = kerReturn;
				if (fState == ksrSkip) fState = ksrReceive;
			}
			break;
		default: /* kerDone */
			*cp++ = ch;
			++out;
			break;
		}
	}
	inherited::Filter (buf, out);
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TReceiveFilter::FlushBuffer ()
{
	long tmp;
	
	if (fOut > 0) {
		tmp = fOut;
		FailOSErr (FSWrite (dRef, &tmp, (Ptr) fBuf));
		FailOSErr (GetFPos (dRef, &tmp));
		fFilePos = tmp;
		fOut = 0;
	}
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TReceiveFilter::Free ()
{
	if (fBuf != NULL) DisposIfPtr (fBuf);
	inherited::Free ();
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TReceiveFilter::IFilter (TDocument *itsDoc)
{
	Str255 tmp;
	
	fBuf = NULL;
	inherited::IFilter (itsDoc);
	fBuf = (unsigned char *) NewPermPtr (kTmpBufferSize);
	fEcho = kerReturn;
	fState = ksrStart;
	GetIndString (tmp, kMUDStrings, kmsEdPrompt);
	fEdPrompt = tmp [1];
	GetIndString (tmp, kMUDStrings, kmsPrompt);
	fPrompt = tmp [1];
	fCurLine = 1;
	fOut = 0;
	fFilePos = 0;
	fPass = TRUE;
	fBlocks = TRUE;
	GetIndString (tmp, kMUDStrings, kmsUnrecognized);
	CopyStr255 (tmp, fUnrecognized);
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void EDReceiveHandler (short , long , void * )
{
	TFilter *aFilter;
	
	gDisableMenus = FALSE;
	InvalidateMenuBar ();
	aFilter = doc->fInputFilter;
	while ((aFilter != NULL) && (aFilter->fNext != filter))
		aFilter = aFilter->fNext;
	if (aFilter != NULL) aFilter->fNext = NULL;
	if (filter != NULL) filter->Free ();
	CloseFile (dRef, rRef);
}

pascal void EDReceive (TMUDDoc *theDoc, Str255 path, Str255 fileName,
		short vRefNum)
{
	FailInfo fi;
	TReceiveFilter *aReceiveFilter;
	TFilter *aFilter;
	Str255 aStr;
	
	FailOSErr (Create (fileName, vRefNum, theDoc->fTextSig, 'TEXT'));
	FailOSErr (MAOpenFile (fileName, vRefNum, TRUE, FALSE, fsRdWrPerm, fsRdPerm,
		&dRef, &rRef));
	doc = theDoc;
	filter = NULL;
	CatchFailures (&fi, EDReceiveHandler, kDummyLink);
	gDisableMenus = TRUE;
	InvalidateMenuBar ();
	done = FALSE;
	aReceiveFilter = new TReceiveFilter;
	FailNIL (aReceiveFilter);
	aReceiveFilter->IFilter (theDoc);
	aFilter = doc->fInputFilter;
	while (aFilter->fNext != NULL) aFilter = aFilter->fNext;
	aFilter->fNext = aReceiveFilter;
	GetIndString (aStr, kMUDStrings, kmsEd);
	theDoc->Send (aStr + 1, aStr [0]);
	theDoc->Send (path + 1, path [0]);
	aStr [0] = chReturn;
	theDoc->Send (aStr, 1);
	while (!done) {
		gApplication->PollEvent (kAllowApplicationToSleep);
		if (gStop) Failure (noErr, msgCancelled);
	}
	Success (&fi);
	gDisableMenus = FALSE;
	InvalidateMenuBar ();
	gApplication->SetupTheMenus ();
	aFilter = doc->fInputFilter;
	while ((aFilter != NULL) && (aFilter->fNext != filter))
		aFilter = aFilter->fNext;
	if (aFilter != NULL) aFilter->fNext = NULL;
	if (filter != NULL) filter->Free ();
	FailOSErr (CloseFile (dRef, rRef));
	FailOSErr (FlushVol (NULL, vRefNum));
}

//------------------------------------------------------------------------------

typedef enum {kesReturn, kesDone, kesSkip} SendEchoState;

typedef enum {kssStart, kssSend, kssWrite, kssQuit, kssEnd} SendState;

class TSendFilter: public TFilter {
public:
	SendEchoState fEcho;
	SendState fState;
	long fRemaining, fPos, fCount, fSkipColon;
	unsigned char *fBuf, fEdPrompt, fPrompt;
	Str255 fPath;
	Boolean fPass, fWasColon;
	
	virtual pascal void IFilter (TDocument *itsDoc);
	virtual pascal long Filter (unsigned char *theChars, long count);
	virtual pascal void Free (void);
	virtual pascal void GetBlock (void);
	virtual pascal void SendBlock (void);
};

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal long TSendFilter::Filter (unsigned char *theChars, long count)
{
	unsigned char ch, *cp, buf [kTmpBufferSize];
	long act, out;
	Str255 tmp;
	
	act = 0;
	out = 0;
	cp = buf;
	while ((count > act) && (out < kTmpBufferSize)) {
		ch = *theChars++;
		++act;
		switch (fEcho) {
		case kesReturn:
			if (ch == fEdPrompt) {
				if (fSkipColon > 0)
					fSkipColon -= 1;
				else {
					switch (fState) {
					case kssStart:
						fPass = FALSE;
						GetIndString (tmp, kMUDStrings, kmsDelete);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						fState = kssSend;
						break;
					case kssSend:
						fPass = ((TMUDDoc *) fDocument)->fDoEcho;
						SendBlock ();
						if ((fRemaining == 0) && (fPos >= fCount))
							fState = kssWrite;
						break;
					case kssWrite:
						fPass = TRUE;
						GetIndString (tmp, kMUDStrings, kmsWrite);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						CopyStr255 (fPath, tmp);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						tmp [0] = chReturn;
						((TMUDDoc *) fDocument)->SendNoEcho (tmp, 1);
						fState = kssQuit;
						break;
					case kssQuit:
						fPass = FALSE;
						GetIndString (tmp, kMUDStrings, kmsQuit);
						((TMUDDoc *) fDocument)->SendNoEcho (tmp + 1, tmp [0]);
						fState = kssEnd;
						break;
					default:
						if (ch != chReturn) fEcho = kesSkip;
						if (fPass) {
							*cp++ = ch;
							++out;
						}
						break;
					}
				}
			} else if ((ch == fPrompt) && (fState == kssEnd)) {
				fPass = TRUE;
				fEcho = kesDone;
				done = TRUE;
				*cp++ = ch;
				++out;
			} else if (ch != '*') {
				if (ch != chReturn) fEcho = kesSkip;
				if (fPass) {
					*cp++ = ch;
					++out;
				}
			}
			break;
		case kesSkip:
			if (ch == chReturn) fEcho = kesReturn;
			if (fPass) {
				*cp++ = ch;
				++out;
			}
			break;
		default: /* kesDone */
			*cp++ = ch;
			++out;
			break;
		}
	}
	inherited::Filter (buf, out);
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TSendFilter::Free ()
{
	if (fBuf != NULL) DisposIfPtr (fBuf);
	inherited::Free ();
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TSendFilter::GetBlock ()
{
	long count;
	
	if (fRemaining > kTmpBufferSize)
		count = kTmpBufferSize;
	else
		count = fRemaining;
	FailOSErr (FSRead (dRef, &count, (Ptr) fBuf));
	fRemaining -= count;
	fCount = count;
	fPos = 0;
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TSendFilter::IFilter (TDocument *itsDoc)
{
	long length;
	Str255 tmp;
	
	fBuf = NULL;
	inherited::IFilter (itsDoc);
	fBuf = (unsigned char *) NewPermPtr (kTmpBufferSize);
	fEcho = kesReturn;
	fState = kssStart;
	FailOSErr (GetEOF (dRef, &length));
	fRemaining = length;
	fPos = 0;
	fCount = 0;
	fSkipColon = 0;
	fPass = TRUE;
	fWasColon = FALSE;
	GetIndString (tmp, kMUDStrings, kmsEdPrompt);
	fEdPrompt = tmp [1];
	GetIndString (tmp, kMUDStrings, kmsPrompt);
	fPrompt = tmp [1];
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void TSendFilter::SendBlock ()
{
	unsigned char ch, buf [kTmpBufferSize];
	long pos, out, outSent;
	Str255 aStr;
	
	GetIndString (aStr, kMUDStrings, kmsAppend);
	((TMUDDoc *) fDocument)->SendNoEcho (aStr + 1, aStr [0]);
	pos = 0;
	out = 0;
	outSent = 0;
	while (((fRemaining > 0) || (fPos < fCount)) &&
			((pos != 0) || (outSent + out < kBlockSize))) {
		if (fPos >= fCount) GetBlock ();
		ch = *(fBuf + fPos);
		if ((ch == chTab) && ((TMUDDoc *) fDocument)->fCvtTab) {
			buf [out] = ' ';
			out += 1;
			pos += 1;
			if ((pos % ((TMUDDoc *) fDocument)->fSTab) == 0) fPos += 1;
		} else if ((ch == chReturn) || (ch == chLineFeed)) {
			buf [out] = ch;
			out += 1;
			pos = 0;
			fPos += 1;
		} else {
			if (((pos == 0) || fWasColon) && (ch == fEdPrompt) &&
					!((TMUDDoc *) fDocument)->fDoEcho) {
				fSkipColon += 1;
				fWasColon = TRUE;
			} else
				fWasColon = FALSE;
			buf [out] = ch;
			out += 1;
			pos += 1;
			fPos += 1;
		}
		if (out >= kTmpBufferSize) {
			((TMUDDoc *) fDocument)->SendNoEcho (buf, out);
			outSent += out;
			out = 0;
		}
	}
	if (out > 0) ((TMUDDoc *) fDocument)->SendNoEcho (buf, out);
	if (pos != 0) {
		buf [0] = chReturn;
		((TMUDDoc *) fDocument)->SendNoEcho (buf, 1);
	}
	GetIndString (aStr, kMUDStrings, kmsStop);
	((TMUDDoc *) fDocument)->SendNoEcho (aStr + 1, aStr [0]);
}

//------------------------------------------------------------------------------

#pragma segment SEDStuff

pascal void EDSendHandler (short , long , void * )
{
	TFilter *aFilter;
	
	gDisableMenus = FALSE;
	InvalidateMenuBar ();
	aFilter = doc->fInputFilter;
	while ((aFilter != NULL) && (aFilter->fNext != filter))
		aFilter = aFilter->fNext;
	if (aFilter != NULL) aFilter->fNext = NULL;
	if (filter != NULL) filter->Free ();
	CloseFile (dRef, rRef);
}

pascal void EDSend (TMUDDoc *theDoc, AppFile *theFile, Str255 path)
{
	TSendFilter *aSendFilter;
	TFilter *aFilter;
	FailInfo fi;
	Str255 aStr;
	
	FailOSErr (MAOpenFile (theFile->fName, theFile->vRefNum, TRUE, FALSE,
		fsRdPerm, fsRdPerm, &dRef, &rRef));
	doc = theDoc;
	filter = NULL;
	CatchFailures (&fi, EDSendHandler, kDummyLink);
	gDisableMenus = TRUE;
	InvalidateMenuBar ();
	done = FALSE;
	aSendFilter = new TSendFilter;
	FailNIL (aSendFilter);
	aSendFilter->IFilter (theDoc);
	aFilter = doc->fInputFilter;
	while (aFilter->fNext != NULL) aFilter = aFilter->fNext;
	aFilter->fNext = aSendFilter;
	CopyStr255 (path, aSendFilter->fPath);
	GetIndString (aStr, kMUDStrings, kmsEd);
	theDoc->Send (aStr + 1, aStr [0]);
	theDoc->Send (path + 1, path [0]);
	aStr [0] = chReturn;
	theDoc->Send (aStr, 1);
	while (!done) {
		gApplication->PollEvent (kAllowApplicationToSleep);
		if (gStop) Failure (noErr, msgCancelled);
	}
	Success (&fi);
	gDisableMenus = FALSE;
	InvalidateMenuBar ();
	gApplication->SetupTheMenus ();
	aFilter = doc->fInputFilter;
	while ((aFilter != NULL) && (aFilter->fNext != filter))
		aFilter = aFilter->fNext;
	if (aFilter != NULL) aFilter->fNext = NULL;
	if (filter != NULL) filter->Free ();
	FailOSErr (CloseFile (dRef, rRef));
	FailOSErr (FlushVol (NULL, theFile->vRefNum));
}

//------------------------------------------------------------------------------
