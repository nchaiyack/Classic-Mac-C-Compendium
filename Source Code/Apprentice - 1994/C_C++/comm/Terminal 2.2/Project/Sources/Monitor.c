/*
	Terminal 1.9
	"Monitor.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment ZModem
#endif

#include "Monitor.h"
#include "FormatStr.h"

static short Ref = 0;
static long Start;

void MonitorOpen(
	register Byte *name,
	register short vol,
	register long dir)
{
	HParamBlockRec p;

	if (Ref)
		return;

	p.ioParam.ioCompletion = 0;
	p.ioParam.ioNamePtr = (StringPtr)name;
	p.ioParam.ioVRefNum = vol;
	p.fileParam.ioDirID = dir;
	PBHDelete(&p, FALSE);

	p.ioParam.ioCompletion = 0;
	p.ioParam.ioNamePtr = (StringPtr)name;
	p.ioParam.ioVRefNum = vol;
	p.fileParam.ioDirID = dir;
	p.fileParam.ioFVersNum = 0;
	if (PBHCreate(&p, FALSE))
		return;

	p.ioParam.ioCompletion = 0;
	p.ioParam.ioNamePtr = (StringPtr)name;
	p.ioParam.ioVRefNum = vol;
	p.fileParam.ioFDirIndex = 0;
	p.fileParam.ioDirID = dir;
	if (PBHGetFInfo(&p, FALSE))
		return;

	p.ioParam.ioCompletion = 0;
	p.ioParam.ioNamePtr = (StringPtr)name;
	p.ioParam.ioVRefNum = vol;
	p.fileParam.ioFlFndrInfo.fdType = 'TEXT';
	p.fileParam.ioFlFndrInfo.fdCreator = 'PEDT';
	p.fileParam.ioDirID = dir;
	if (PBHSetFInfo(&p, FALSE))
		return;
	
	p.ioParam.ioCompletion = 0;
	p.ioParam.ioNamePtr = (StringPtr)name;
	p.ioParam.ioVRefNum = vol;
	p.ioParam.ioPermssn = 0;
	p.ioParam.ioMisc = 0;
	p.fileParam.ioFVersNum = 0;
	p.fileParam.ioDirID = dir;
	if (PBHOpen(&p, FALSE))
		return;
	Ref = p.fileParam.ioFRefNum;
	Start = Ticks;
}

void MonitorClose(void)
{
	if (Ref) {
		FSClose(Ref);
		Ref = 0;
	}
}

void MonitorDump(
	register Byte *buffer,
	long count)
{
	register Byte s[255];
	register Byte b;
	register short n;
	register Byte *max = buffer + count;
	register Byte *p;
	static Byte hex[] = "0123456789ABCDEF";

	if (!Ref)
		return;

	n = 0;
	p = s;
	while (buffer < max) {
		if (n >= 26) {
			*p++ = '\r';
			count = p - s;
			if (FSWrite(Ref, &count, s)) {
				MonitorClose();
				return;
			}
			p = s;
			n = 0;
		}
		b = *buffer++;
		*p++ = ' ';
		*p++ = hex[(b >> 4) & 0x0F];
		*p++ = hex[b & 0x0F];
		++n;
	}
	*p++ = '\r';
	count = p - s;
	if (FSWrite(Ref, &count, s))
		MonitorClose();
}

void MonitorText(Byte *t)
{
	register Byte s[255];
	long count;

	if (!Ref)
		return;
	FormatStr(s, (Byte *)"\p%06l: %s\r", Ticks - Start, t);
	count = *s;
	if (FSWrite(Ref, &count, s + 1))
		MonitorClose();
}
