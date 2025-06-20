// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UFakeNntp.cp

#include "UFakeNntp.h"
#include "UProgress.h"
#include "UPrefsDatabase.h"
#include "FileTools.h"
#include "Tools.h"
#include "NetAsciiTools.h"
#include "UPassword.h"
#include "UThread.h"

#include <ErrorGlobals.h>

#include <Packages.h>
#include <Errors.h>

#pragma segment MyComm

#define qDebugConstruct qDebug & 0

#define macroPATH "scratch-1:csm:"

PFakeNntp::PFakeNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PFakeNntp::PFakeNntp() at $%lx called\n", long(this));
#endif
	fPath = "";
	fRealFirstArticleID = 0;
	fRealLastArticleID = 0;
}

void PFakeNntp::IFakeNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PFakeNntp::IFakeNntp() at $%lx called \n", long(this));
#endif
	INntp();
}

PFakeNntp::~PFakeNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PFakeNntp::~PFakeNntp() at $%lx called\n", long(this));
#endif
}

Handle PFakeNntp::ReadFile(const CStr255 &name)
{
	FailInfo fi;
	TFile *theFile;
	VOLATILE(theFile);
	Handle h = nil;
	VOLATILE(h);
	if (fi.Try())
	{
		theFile = new TFile();
		theFile->IFile('TEXT', 'MPS ', kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);
		theFile->SpecifyWithTrio(0, 0, name);
		FailOSErr(theFile->OpenFile());
		long len = 0;
		FailOSErr(theFile->GetDataLength(len));
		h = NewPermHandle(len);
		HLock(h);
		FailOSErr(theFile->SetDataMark(0, fsFromStart));
		Ptr p = *h;
		while (len)
		{
			gCurThread->YieldTime();
			long sublen = Min(2000, len);
			FailOSErr(theFile->ReadData(p, sublen));
			p += sublen;
			len -= sublen;
		}
		HUnlock(h);
		theFile->CloseFile();
		theFile->Free(); theFile = nil;// closes file
		gCurThread->YieldTime();
		MakeLastCharNull(h);
		fi.Success();
		return h;
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		FreeIfObject(theFile); theFile = nil;
		fi.ReSignal();
	}
}

Handle PFakeNntp::GetListOfAllGroups()
{
	return ReadFile(macroPATH"GroupList");
}

Handle PFakeNntp::GetListOfNewGroups(unsigned long /* fromDate */)
{
	return ReadFile(macroPATH"GroupNew");
}

Handle PFakeNntp::GetListOfGroupDesciptions()
{
	return ReadFile(macroPATH"GroupDescriptions");
}

void PFakeNntp::DoSetGroup(const CStr255 &name)
{
	TFile *theFile = nil;
	VOLATILE(theFile);
	FailInfo fi;
	if (fi.Try())
	{
		fPath = macroPATH;
		fPath += name;
		fPath += ':';
		CStr255 s(fPath);
		s += "qRange";
		theFile = new TFile();
		theFile->IFile('TEXT', 'MPS ', kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);
		theFile->SpecifyWithTrio(0, 0, s);
		OSErr err = theFile->OpenFile();
		if (err == fnfErr)
			FailOSErr(errNntpBadGroup);
		else
			FailOSErr(err);
		gCurThread->YieldTime();
		char buffer[400];
		long count = 390;
		theFile->ReadData(buffer, count);
		buffer[count] = 0;
		gCurThread->YieldTime();
		long realFirstArticleID, realLastArticleID, first, last;
		sscanf(buffer, "%ld %ld %ld %ld", &realFirstArticleID, &realLastArticleID, &first, &last);
		fFirstArticleID = first;
		fLastArticleID = last;
		if (realLastArticleID != 0 && realFirstArticleID > realLastArticleID)
		{
			DebugStr("realFirstArticleID > realLastArticleID");
			Failure(9999, 0);
		}
		fRealFirstArticleID = realFirstArticleID;
		fRealLastArticleID = realLastArticleID;
		FreeIfObject(theFile); theFile = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(theFile); theFile = nil;
		fi.ReSignal();
	}
}

Handle PFakeNntp::GetHeaderList(const char *headerName, long firstArticleID, long lastArticleID)
{
	if (fPath == "")
	{
		DebugStr("Group not set!");
		return NewPermHandle(0);
	}
	if (firstArticleID > lastArticleID)
		return NewPermHandle(0); // shortcut: don't fetch anything
	FailInfo fi;
	TFile *theFile = nil;
	VOLATILE(theFile);
	Handle h = nil;
	VOLATILE(h);
	if (fi.Try()) 
	{
		CStr255 s(fPath);
		s += "q";
		s += headerName;
		theFile = new TFile();
		theFile->IFile('TEXT', 'MPS ', kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);
		theFile->SpecifyWithTrio(0, 0, s);
		FailOSErr(theFile->OpenFile());
		long len = 0;
		FailOSErr(theFile->GetDataLength(len));
		h = NewPermHandle(len);
		HLock(h);
		theFile->SetDataMark(0, fsFromStart);
		Ptr p = *h;
		long loadLen = len;
		while (loadLen)
		{
			gCurThread->YieldTime();
			long sublen = Min(2000, loadLen);
			FailOSErr(theFile->ReadData(p, sublen));
			p += sublen;
			loadLen -= sublen;
		}
		HUnlock(h);
		theFile->CloseFile();
		theFile->Free(); theFile = nil; // closes file
		register Ptr startP = *h;
		register long i;
		if (firstArticleID < fFirstArticleID)
			firstArticleID = fFirstArticleID;
		if (lastArticleID > fLastArticleID)
			lastArticleID = fLastArticleID;
		for (i = firstArticleID - fRealFirstArticleID; i > 0; i--)
			while (*startP++ != 13)
				;
		if (*startP == 10) 
			startP++;
		register Ptr endP = startP;
		for (i = lastArticleID - firstArticleID + 1; i > 0; i--)
			while (*endP++ != 13)
				;
		if (endP > *h + len) 
		{
			DebugStr(CStr255("UPS: got past end of handle in parsing!"));
			endP = *h + len;
		}
		BytesMove(startP, *h, endP - startP);
		SetPermHandleSize(h, endP - startP); // downsizing
		MakeLastCharNull(h);
		gCurProgress->Worked(lastArticleID - firstArticleID);
#if qDebug
		long zzsize = GetHandleSize(h);
		if (zzsize && *(*h + zzsize - 1) != 0)
			ProgramBreak("The last char is not null (som jeg lovede!)");
#endif
		fi.Success();
		return h;
	}
	else // fail
	{
		FreeIfObject(theFile); theFile = nil;
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}


Handle PFakeNntp::GetArticle(long articleID)
{
	if (fPath == "")
	{
		DebugStr("Group not set!");
		return NewPermHandle(0);
	}
	if (articleID <= 0 || articleID > fRealLastArticleID)
	{
		char sss[200];
		sprintf(sss, "Crazy article requested: articleID = %ld", articleID);
		DebugStr(sss);
		return nil;
	}
	FailInfo fi;
	TFile *theFile;
	VOLATILE(theFile);
	Handle h = nil;
	if (fi.Try()) 
	{
		CStr255 s;
		long hs, hl, bs, bl, as, al;
		theFile = new TFile();
		theFile->IFile('TEXT', 'MPS ', kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);
		{
			s = fPath;
			s += "article_index";
			theFile->SpecifyWithTrio(0, 0, s);
			FailOSErr(theFile->OpenFile());
			short diffID = short(articleID - fRealFirstArticleID);
			long offset = 77 * diffID;
			FailOSErr(theFile->SetDataMark(offset, fsFromStart));
			char buffer[400];
			long count = 390;
			theFile->ReadData(buffer, count);
			buffer[count] = 0;
			long line_id = 0;
			sscanf(buffer, "%ld %ld %ld %ld %ld %ld %ld", &line_id, &hs, &hl, &bs, &bl, &as, &al);
			if (line_id != articleID) 
			{
				char sss[200];
				sprintf(sss, "WRONG ARTICLE ID (%ld) found in article_index, %ld expected, buffer at $%lx", line_id, articleID, buffer);
				DebugStr(sss);
				fi.Success();
				return NewPermHandle(0);
			}
			FailOSErr(theFile->CloseFile());
		}
		theFile->SpecifyWithTrio(0, 0, CStr255(fPath) + "articles");
		FailOSErr(theFile->OpenFile());
		FailOSErr(theFile->SetDataMark(as, fsFromStart));
		long len;
		FailOSErr(theFile->GetDataLength(len));
		al = Min(al, len - as);
		h = NewPermHandle(al);
		HLock(h);
		FailOSErr(theFile->ReadData(*h, al));
		theFile->Free(); theFile = nil; // closes file
		HUnlock(h);
		MakeLastCharNull(h);
		fi.Success();
#if qDebug
		long zzsize = GetHandleSize(h);
		if (zzsize && *(*h + zzsize - 1) != 0)
			ProgramBreak("The last char is not null (som jeg lovede!)");
#endif
		gCurThread->YieldTime();
		return h;
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		FreeIfObject(theFile); theFile = nil;
		fi.ReSignal();
	}
}

Boolean PFakeNntp::IsPostingAllowed()
{
	return true;
}

void PFakeNntp::PostArticle(Handle h, short /* ackStringID */)
{
#if qDebug
	CStr255 username, password;
	GetUserNameAndPassword(username, password);
	fprintf(stderr, "PFakeNntp::PostArticle, username = '%s', ", (char*)username);
	fprintf(stderr, "password = '%s'\n", (char*)password);
#endif
	TFile *file = nil;
	VOLATILE(file);
	FailInfo fi;
	if (fi.Try())
	{
		file = NewFile('TEXT', 'MPS ',
			kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		file->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		FSSpec spec;
		gPrefs->GetSilentDirAliasPrefs('FEdi', spec);
		short nr = 1;
		while (true)
		{
			CStr255 s;
			NumToString(nr++, s);
			s.Insert("Posted article ", 1);
			CopyCString2String(s, spec.name);
			file->Specify(spec);
			if (!FileExist(file))
				break;
		}
		FailOSErr(file->CreateFile());
		FailOSErr(file->OpenFile());
		HLock(h);
		long size = GetHandleSize(h);
		FailOSErr(file->WriteData(*h, size));
		HUnlock(h);
		file->Free(); file = nil;
		gCurThread->YieldTime();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(file); file = nil;
		fi.ReSignal();
	}
}

void PFakeNntp::ExamineNewsServer()
{
	// who is going to doubt this server?
}
