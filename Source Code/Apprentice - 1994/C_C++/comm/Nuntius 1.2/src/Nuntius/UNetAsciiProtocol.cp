// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNetAsciiProtocol.cp

#include "UNetAsciiProtocol.h"
#include "UMacTcp.h"
#include "NetAsciiTools.h"
#include "Tools.h"
#include "UThread.h"
#include "UProgress.h"
#include "UPrefsDatabase.h"

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyComm

#define qLogCommands qDebug & 0
#define qDebugDumpReceive qDebug & 0
#define qDebugTransmit qDebug & 0
#define qDebugConstruct qDebug & 0

FILE *myNAPstderr = nil;
#if FALSE
#if qDebug
#define myNAPstderr myNAPstderr
void OpenNAPOut()
{
	char filename[] = "HD2:Desktop Folder:NAP_log";
	myNAPstderr = fopen(filename, "w");
	if (myNAPstderr == nil)
		DebugStr("Could not open NAP logfile");
	fsetfileinfo(filename, 'MPS ', 'TEXT'); 
}
#endif
#else
#endif

const long kInBufferSize = 4 * 1024; // only when using copy-receive from MacTCP

const long kOutBufferSize = 4 * 1024;

const char CRLF[] = {13, 10};

PNetAsciiProtocol::PNetAsciiProtocol()
{
#if qDebugConstruct
	fprintf(stderr, "PNetAsciiProtocol::PNetAsciiProtocol() at $%lx called\n", long(this));
#endif
#ifdef myNAPstderr
	if (myNAPstderr == nil)
		OpenNAPOut();
#else
	myNAPstderr = stderr;
#endif
	fMacTcp = nil;
	fCurrentReceiveP = nil;
	fCurrentReceiveLeft = 0;
	fProgressDelta = 0;
	fReceiveBufferP = nil;
	fTransmitBufferP = nil;
	fCurrentTransmitP = nil;
	fCurrentTransmitChars = 0;
	fRespondLineP = nil;
}

void PNetAsciiProtocol::INetAsciiProtocol(long newsServerAddr, short port)
{
#if qDebugConstruct
	fprintf(stderr, "void PNetAsciiProtocol::INetAsciiProtocol() at $%lx called\n", long(this));
#endif
	FailInfo fi;
	if (fi.Try())
	{
		PTcpStream *mt = new PTcpStream();
		mt->ITcpStream(gPrefs->GetLongPrefs('STio'));
		fMacTcp = mt;
		fMacTcp->OpenConnection(newsServerAddr, port);

		fRespondLineP = NewPermPtr(kReturnLineBufferSize);
		*fRespondLineP = 0;
		fReceiveBufferP = NewPermPtr(kInBufferSize);
		fCurrentReceiveP = fReceiveBufferP;
		fTransmitBufferP = NewPermPtr(kOutBufferSize);
		fCurrentTransmitP = fTransmitBufferP;

		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

PNetAsciiProtocol::~PNetAsciiProtocol()
{
#if qDebugConstruct
	fprintf(stderr, "PNetAsciiProtocol::~PNetAsciiProtocol() at $%lx called\n", long(this));
#endif
	fReceiveBufferP = DisposeIfPtr(fReceiveBufferP);
	fTransmitBufferP = DisposeIfPtr(fTransmitBufferP);
	fRespondLineP = DisposeIfPtr(fRespondLineP);
	FreeIfPtrObject(fMacTcp); fMacTcp = nil;
}	

// ---------------- PTcpStream stuff
void PNetAsciiProtocol::FillReceiveBuffer()
{
	long noBytes = 0;
	while (!noBytes)
	{
		fMacTcp->ReceiveSomeData(fReceiveBufferP, kInBufferSize, noBytes);
		fCurrentReceiveP = fReceiveBufferP;
		fCurrentReceiveLeft = noBytes;
	}
#if qDebugDumpReceive
	char ss[200];
	Ptr p = fCurrentReceiveP;
	fprintf(myNAPstderr, "Got %ld bytes at %lx\n", noBytes, long(p));
	long l = noBytes;
	long part = 0;
	while (l)
	{
		switch (*p) {
			case 13:
				ss[part] = '\\';
				break;
			case 10:
				ss[part] = '@';
				break;
			default:
				ss[part] = *p;
		}
		p++;
		l--;
		part++;
		if (part > 70)
		{
			ss[part] = 0;
			fprintf(myNAPstderr, "Data received: %s\n", ss);
			part = 0;
		}
	}
	ss[part] = 0;
	fprintf(myNAPstderr, "Data received: %s\n", ss);
#endif
}

// ---------------- parsing line / dot-end
void PNetAsciiProtocol::CopyChars(Handle h, long noChars, long &curSize)
{
#if qDebugDumpReceive
	fprintf(stderr, "CopyChars, noChars = %ld, curSize %ld -> %ld, ", noChars, curSize, curSize + noChars);
	fprintf(stderr, "fCurrentReceiveLeft %ld -> %ld\n", fCurrentReceiveLeft, fCurrentReceiveLeft - noChars);
#endif
#if qDebug
	if (noChars < 0)
	{
		ProgramBreak("noChars < 0");
		noChars = 0;
	}
	if (noChars > fCurrentReceiveLeft)
	{
		ProgramBreak("noChars > fCurrentReceiveLeft");
		noChars = fCurrentReceiveLeft;
	}
#endif
	SetPermHandleSize(h, curSize + noChars);
	BytesMove(fCurrentReceiveP, *h + curSize, noChars);
	curSize += noChars;
	fCurrentReceiveP += noChars;
	fCurrentReceiveLeft -= noChars;
	UpdateProgress();
}

void PNetAsciiProtocol::UpdateDotBuffer(Handle h, Ptr &destP)
{
	UpdateProgress();
	long currSize = destP - *h;
	FillReceiveBuffer(); /* updates fCurrentReceiveP and fCurrentReceiveLeft */
	SetPermHandleSize(h, currSize + fCurrentReceiveLeft);
	destP = *h + currSize;
}

Handle PNetAsciiProtocol::GetDotTerminatedText()
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = NewPermHandle(0);
		UpdateProgress();
		long size = DoGetDotTerminatedText(h);
		fProgressDelta = Max(0, fProgressDelta - 1); // subtract '.' line
#if qDebug
		if (size < 0)
			ProgramBreak("size < 0, should never could happend!");
		if (size > 120 * 1024)
			fprintf(stderr, "Got a real large GetDotTerminatedText(): %ld bytes\n", size);
#endif
		SetPermHandleSize(h, size + 1);
		*(*h + size) = 0;
#if qDebug
		long zzsize = GetHandleSize(h);
		if (zzsize && *(*h + zzsize - 1) != 0)
			ProgramBreak("The last char is not null (som jeg lovede!)");
#endif
		UpdateProgress();
		fi.Success();
		return h;
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}

long PNetAsciiProtocol::DoGetDotTerminatedText(Handle h)
// Returns number of received chars. Handle can be larger
{
	SetPermHandleSize(h, fCurrentReceiveLeft);
	Ptr tmpDestP;
	register Ptr destP = *h;
	register Ptr fromP = fCurrentReceiveP;
	register long left = fCurrentReceiveLeft;
	goto TestDot;
	while (true)
	{

#define macroCopyNext()		\
		*destP++ = *fromP;		\
		++fromP;							\
		--left;
// end macro

#define macroCheckLeft()							\
		if (!left)												\
		{																	\
			tmpDestP = destP;								\
			fCurrentReceiveLeft = left;			\
			fCurrentReceiveP = fromP;				\
			UpdateDotBuffer(h, tmpDestP);		\
			destP = tmpDestP;								\
			left = fCurrentReceiveLeft;			\
			fromP = fCurrentReceiveP;				\
		}
// end macro

		macroCopyNext();
		macroCheckLeft();
		if (*fromP != 13)
			continue;
	
		while (*fromP == 13)
		{
			++fProgressDelta;
			macroCopyNext();
			macroCheckLeft();
			if (*fromP != 10)
				break;
			macroCopyNext();
			macroCheckLeft();
		}
TestDot:
		macroCheckLeft();
		if (*fromP != '.')
			continue;
		++fromP; --left; // skip dot, don't copy it
		macroCheckLeft();
		if (*fromP != 13)
			continue;
		++fromP; --left; // skip CR, don't copy it
		macroCheckLeft();
		++fromP; --left; // skip LF, don't copy it
		break; // funny down here!
	}
	fCurrentReceiveLeft = left;
	fCurrentReceiveP = fromP;
	return destP - *h;
}

char PNetAsciiProtocol::GetChar()
{
	if (fCurrentReceiveLeft == 0)
		FillReceiveBuffer();
	--fCurrentReceiveLeft;
	return *fCurrentReceiveP++;
}

void PNetAsciiProtocol::GetLine()
{
	Ptr destP = fRespondLineP;
	long roomLeft = kReturnLineBufferSize - 5;

	char ch = GetChar();
	while (ch != 13 && --roomLeft)
	{
		*destP++ = ch;
		ch = GetChar();
	}
	*destP = 0;
	while (ch != 13)
		ch = GetChar();
	GetChar(); // skip LF
}

// ---------------- handling commands
void PNetAsciiProtocol::SendCommand(char *cmd)
{
	SendChars(cmd, strlen(cmd));
	SendChars(CRLF, 2);
	FlushTransmitBuffer();
}

void PNetAsciiProtocol::GetRespondLine()
{
	while (true)
	{
		GetLine();
		Ptr p = fRespondLineP;
		short num = 0;
		char ch = *p;
		while (ch && ch >= '0' && ch <= '9')
		{
			num *= 10;
			num += ch - '0';
			ch = *++p;
		}
		LogCommands(fRespondLineP);
		LogCommands("\n");
		if (*p == '-') // multi line, if anybody is using that
			continue;
		fRespondCode = num;
		break;
	}
}

// ---------------- transmitting
void PNetAsciiProtocol::FlushTransmitBuffer()
{
	if (!fCurrentTransmitChars)
		return;
#if qDebugTransmit
	char ss[200];
	Ptr p = fTransmitBufferP;
	fprintf(myNAPstderr, "Sending %ld bytes at %lx\n", fCurrentTransmitChars, long(p));
	long l = fCurrentTransmitChars;
	long part = 0;
	while (l)
	{
		if (*p == 13)
				ss[part] = '\\';
		else if (*p == 10)
			ss[part] = '@';
		else if (*p < 32)
		{
			ss[part++] = '�';
			ss[part++] = '^';
			ss[part++] = *p + 64;
			ss[part++] = '�';
		}
		else
			ss[part] = *p;
		p++;
		l--;
		part++;
		if (part > 70)
		{
			ss[part] = 0;
			fprintf(myNAPstderr, "Data sent: %s\n", ss);
			part = 0;
		}
	}
	if (part)
	{
		ss[part] = 0;
		fprintf(myNAPstderr, "Data sent: %s\n", ss);
	}
#endif
	fMacTcp->SendData(fTransmitBufferP, fCurrentTransmitChars);
	fCurrentTransmitP = fTransmitBufferP;
	fCurrentTransmitChars = 0;
}

void PNetAsciiProtocol::SendChars(const void *p, long noChars)
{
	register const unsigned char *fromP = (const unsigned char *)(p);
	while (noChars)
	{
		long roomLeft = kOutBufferSize - fCurrentTransmitChars;
		if (!roomLeft) // filled up
		{
			FlushTransmitBuffer();
			roomLeft = kOutBufferSize;
		}
		register long tmpNoChars = Min(noChars, roomLeft);
		BytesMove(fromP, fCurrentTransmitP, tmpNoChars);
		fromP += tmpNoChars;
		fCurrentTransmitP += tmpNoChars;
		fCurrentTransmitChars += tmpNoChars;
		noChars -= tmpNoChars;
	}
	UpdateProgress();
}

void PNetAsciiProtocol::SendDotTerminatedText(Handle h)
{
	MakeLastCharCR(h);
	HLock(h);
	register unsigned char *p = (unsigned char *) *h;
	unsigned char *endP = p + GetHandleSize(h) - 1;
	long line = 0;
	Boolean inHeader = true;
	Boolean avoidFormatting = gPrefs->GetBooleanPrefs('2022');
	long wrapLen = gPrefs->GetLongPrefs('WrLn');
	while (p <= endP)
	{
		unsigned char *lineStartP = p;
		while (*p != 13)
			++p;
		long lineChars = p - lineStartP + 1; // with CR
		while (!avoidFormatting && !inHeader && lineChars > wrapLen) //  We don't want to break headers
		{
			long subLineChars = wrapLen - 1; //with space->cr char
			p = lineStartP + subLineChars - 1;
			while (subLineChars && *p > 32) // a b c _ d e f
			{
				--p; 
				--subLineChars;
			}
			if (!subLineChars)
			{
				subLineChars = wrapLen - 3;
				p = lineStartP + subLineChars - 1;
			}
			fProgressDelta += subLineChars;
			if (*lineStartP == '.')
				SendChars(".", 1); // double dot on start of line
			SendChars(lineStartP, subLineChars - 1); // sends CR myself
			SendChars(CRLF, 2);
			if (inHeader)
				SendChars("     ", 5); // fold header
			lineStartP += subLineChars;
			lineChars -= subLineChars;
		}
		if (inHeader && lineChars <= 1)
			inHeader = false;
		if (*lineStartP == '.')
			SendChars(".", 1); // double dot on start of line
		if (lineChars > 1)
			SendChars(lineStartP, lineChars - 1); // sends CR myself
		SendChars(CRLF, 2);
		fProgressDelta += lineChars;
		p = lineStartP + lineChars;
		if (p <= endP && *p == 10)
			p++;
	}
	SendChars(".", 1);
	SendChars(CRLF, 2);
	HUnlock(h);
	FlushTransmitBuffer();
	UpdateProgress();
}

// ---------------- progress
void PNetAsciiProtocol::UpdateProgress()
{
	if (!fProgressDelta)
		return;
	gCurProgress->Worked(fProgressDelta);
	fProgressDelta = 0;
}

void PNetAsciiProtocol::LogCommands(char *p)
{
#if qLogCommands
	fprintf(stderr, "%s", p);
#else
	p = p;
#endif
}
