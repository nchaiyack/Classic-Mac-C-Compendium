// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UFatalError.cp

#include "UFatalError.h"
#include "Tools.h"
#include "FileTools.h"
#include "UThread.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <ToolUtils.h>
#include <OSUtils.h>
#include <Packages.h>
#include <Resources.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <Unmangler.h>
#include <DisAsmLookup.h>
#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyTools

Handle gFatalErrorReserveH = nil;

char gPanicBuffer[kPanicBufferSize + 100];

void ReallocFatalErrorReserve()
{
	if (gFatalErrorReserveH)
		return;
	long size = 5 * 1024;
	while (size)
	{
		gFatalErrorReserveH = NewHandle(size);
		if (gFatalErrorReserveH)
			return;
		size -= 1 * 1024;
	}
}

void InitUFatalError()
{
	gFatalErrorReserveH = NewPermHandle(5 * 1024);
}

void AppendLineToErrorFile(const CStr255 &msg)
{
	CStr255 s(msg);
	if (!s.Length() || s[s.Length()] != 13)
		s += 13;
	fprintf(stderr, "%s", (char*)s);
}

void MyGetFunctionName(void* pc, CStr255 &name)
{
	if (pc != NULL && !odd(long(pc)))
	{
		void* nextPC;
		CStr255 localname;
		Ptr limit = Ptr(ptrdiff_t(pc) + 32767);
		while (!endOfModule(pc, limit, (char *) &localname, &nextPC))
		{
			if (pc >= limit)
			{
				name = "";
				localname = "";
				return;
			}
			else
				pc = (void *) (ptrdiff_t(pc) + 2);
		}

		char cname[256];

		if (unmangle(cname, (char *) localname, 255) < 1)
			name = localname;
		else
			name = cname;
		short index = name.Pos(".");
	}
	else
	{
		name = "";
	}
}

void DebugFrameFindReturnOffset(void *pc)
{
	long numIter = 0;
	short *wp = (short*)pc;
	if (long(wp) & 1) // odd address
		return;
	while (true)
	{
		if (++numIter > 1000)
			return;
		// avoid the constant 0x4E56 (LINK A6,#) in code as Macsbug doesn't like it,
		// just like this little function
		if (*wp - 6 == 0x4E50) 
			break;
		wp--;
	}
	fprintf(stderr, " + $%lx", long(pc) - long(wp));
}

void *gNextA6;
CStr255 procName;
void DumpFrame()
{
	if (odd(long(gNextA6)) ||
				gNextA6 > GetCurStackBase() ||
				gNextA6 < GetCurStackTop())
		return; // CStackFrame::LooksValid() inverted
	void *pc = *(void **) (long(gNextA6) + 4); // CStackFrame::GetReturnAddress
	if (odd(long(pc)))
		return;
	gNextA6 = *((void **) gNextA6);
	DumpFrame();
	MyGetFunctionName(pc, procName);
	fprintf(stderr, "      %s", (char*)procName);
	DebugFrameFindReturnOffset(pc);
	fprintf(stderr, "\n");
}

void DumpStack()
{
	fprintf(stderr, "Dumping stack\n");
	gNextA6 = GetCurStackFramePtr();
	DumpFrame();
	fprintf(stderr, "End of stack dump, size ~ %ld\n", long(GetCurStackBase()) - long(GetCurStackFramePtr()));	
}

void DoFatalError(const char *fatalMsg, Boolean abort)
{
	gFatalErrorReserveH = DisposeIfHandle(gFatalErrorReserveH);
#if !qDebug & 0
	if (logf)
		fclose(logf);
#endif
#if qDebug
	fprintf(stderr, "\n");
	fprintf(stderr, "Fatal error:\n");
	static char buffer[3000];
	strcpy(buffer, fatalMsg);
	Ptr start = buffer;
	while (*start)
	{
		Ptr p = start;
		while (*p && *p != 13) ++p;
		Boolean atEnd = (*p == 0);
		*p = 0;
		fprintf(stderr, "%s\n", start);
		if (atEnd)
			break;
		start = p + 1;
	}
	ProgramBreak(gEmptyString);
#endif
	unsigned long tc = TickCount();
	unsigned long dt;
	GetDateTime(dt);

	FlushVols();
	if (abort)
	{
		FreeIfObject(gBusyCursor); gBusyCursor = nil; // stop spinning cursor
		UnpatchAll();
	}
	UseResFile(gApplicationRefNum);

	FailInfo fi;
	if (fi.Try())
	{
		CStr255 s, msg, repl;
		long resp, li;
	
// alert
		MAInteractWithUser(kNoTimeOut, gNotificationPtr, nil);
		SetCursor(qd.arrow);
		if (abort)
			Alert(phPanicExitAlert, nil);
		else
			Alert(phFatalReportAlert, nil);

//	create bug file
		FSSpec spec;
		FailOSErr(FindFolder(kOnSystemDisk, kDesktopFolderType, kCreateFolder, spec.vRefNum, spec.parID));
		MyGetIndString(s, kErrorFileName);
		CopyCString2String(s, spec.name);
		MakeFilenameUnique(spec);
		GetPathNameFromDirID(spec.vRefNum, spec.parID, msg);
		msg += CStr255(spec.name);
#if qDebug
		fprintf(stderr, "Filename for FatalErrorMsg: '%s'\n", (char*)msg);
#endif
		FILE oldStdErr = *stderr;
		FILE *file = fopen(msg, "w+");
		if (!file)
		{
			for (li = 1; li <= 6; li++)
				SysBeep(1);
			FlushVols();
			ExitToShell();
		}
		fsetfileinfo(msg, 'ttxt', 'TEXT');
//		file = fopen(msg, "a");
		_iob[2] = *file;  // _iob[2] = stderr

// initial message
		MyGetIndString(msg, kFEInitialMessage);
		AppendLineToErrorFile(msg);
		
// Error description
		MyGetIndString(msg, kFEErrorMessage);
		AppendLineToErrorFile(msg);
		fputs(fatalMsg, stderr);
		fprintf(stderr, "\n\n");

// Nuntius version
		VersRecHndl vH = nil;
		vH = VersRecHndl(GetResource('vers', 1));
		if (vH != nil)
		{
			MyGetIndString(msg, kFEVersion1);
			repl = (*vH)->shortVersion;
			SubstituteStringItems(msg, "«version»", repl);
			if ((*vH)->countryCode == 0)
				MyGetIndString(repl, kFEUSLangName);
			else
				NumToString((*vH)->countryCode, repl);
			SubstituteStringItems(msg, "«language»", repl);
			AppendLineToErrorFile(msg);
		}	
		vH = VersRecHndl(GetResource('vers', 2));
		if (vH != nil)
		{
			MyGetIndString(msg, kFEVersion2);
			repl = (*vH)->shortVersion;
			SubstituteStringItems(msg, "«version»", repl);
			if ((*vH)->countryCode == 0)
				MyGetIndString(repl, kFEUSLangName);
			else
				NumToString((*vH)->countryCode, repl);
			SubstituteStringItems(msg, "«language»", repl);
			AppendLineToErrorFile(msg);
		}
		MyGetIndString(msg, kFECompileDateString);
		AppendLineToErrorFile(msg);
	
// Time for error
		fprintf(stderr, "Tidspunkt:\n");
		fprintf(stderr, "  TickCount() = %ld\n", tc);
		IUDateString(dt, longDate, s);
		IUTimeString(dt, true, msg);
		fprintf(stderr, "   %s  ", (char*)s);
		fprintf(stderr, "   %s\n", (char*)msg);

// HeaderBodySeparator
		MyGetIndString(msg, kFEHeaderBodySeparator);
		AppendLineToErrorFile(msg);

// Free memory
		long total, contig;
		PurgeSpace(total, contig);
		fprintf(stderr, "Free memory: %ld  (largest block: %ld)\n", total, contig);
		Size szCodeReserve, szMemReserve;
		GetReserveSize(szCodeReserve, szMemReserve);
		li = (pCodeReserve && *pCodeReserve) ? GetHandleSize(pCodeReserve) : 0;
		fprintf(stderr, "  - pSzCodeReserve = %ld, pCodeReserveSize = %ld\n", szCodeReserve, li);
		li = (pMemReserve && *pMemReserve) ? GetHandleSize(pMemReserve) : 0;
		fprintf(stderr, "  - pSzMemReserve  = %ld, pMemReserveSize = %ld\n", szMemReserve, li);
		li = (pMemReserve && *pMemReserve) ? GetHandleSize(pMemReserve) : 0;
		fprintf(stderr, "  - pOKCodeReserve = %ld, pReserveExists = %ld, MemSpaceIsLow() = %ld\n", long(pOKCodeReserve), long(pReserveExists), long(MemSpaceIsLow()));

// Machine type
		Gestalt(gestaltMachineType, resp);
		GetIndString(repl, kMachineNameStrID, short(resp));
		MyGetIndString(msg, kFEMacModel);
		SubstituteStringItems(msg, "«macmodel»", repl);
		AppendLineToErrorFile(msg);

// system version
		Gestalt(gestaltSystemVersion, resp);
		VersionToString(resp, repl);
		MyGetIndString(msg, kFESystemVersion);
		SubstituteStringItems(msg, "«sysversion»", repl);
		AppendLineToErrorFile(msg);

// Threads
		DumpDebugThreadDescription();
		
// Stack crawl
		DumpStack();
		
// done
		MyGetIndString(msg, kFEPostMessage);
		AppendLineToErrorFile(msg);
		fclose(stderr);
		*stderr = oldStdErr;
		FlushVols();
		if (abort)
		{
			ExitToShell();
			// report a fatal error here?
		}
		ReallocFatalErrorReserve();
		fi.Success();
	}
	else // fail, ups
	{
		for (short i = 1; i <= 10; i++)
			SysBeep(1);
		ExitToShell();
		fi.ReSignal(); // what else?
	}
}

void PanicExitToShell(const char *fatalMsg)
{
	DoFatalError(fatalMsg, true);
}

void ReportFatalError(const char *fatalMsg)
{
	DoFatalError(fatalMsg, false);
}
