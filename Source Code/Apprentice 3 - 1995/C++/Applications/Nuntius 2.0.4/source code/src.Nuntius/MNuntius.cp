// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// MNuntius.cp

#include "UNewsAppl.h"
#include "UMacTcp.h"
#include "UNntp.h"
#include "FileTools.h"
#include "FastBits.h"
#include "MyDontDeadStrip.h"
#include "Tools.h"
#include "UFatalError.h"
#include "ExitToShellPatch.h"

#include "RsrcGlobals.h"
#include "MenuGlobals.h"
#include "ErrorGlobals.h"

#include <Memory.h>
#include <UGridView.h>
#include <UDialog.h>
#include <UTEView.h>
#include <UPrinting.h>
#include <Resources.h>
#include <Packages.h>
#include <UFloatWindow.h>
#include <stdio.h>

#pragma segment A5Ref
void A5RefSegmentLoader()
{
	// To get the segment loaded so _LoadSeg is called on that
	// segment. Otherwise it will be called at interrupt time :-(	
}

#pragma segment Main

#if 0
void MakeAllSegmentsResident()
{
#ifndef powerc
	gUnloadAllSegs = false;
	short maxNo = CountResources('CODE');
	for (short i = 1; i <= maxNo; i++)
	{
		Handle h = GetIndResource('CODE', i);
		FailNILResource(h);
		if (HGetState(h) & 0x0080) // is locked
			continue;
		HNoPurge(h);
		HLockHi(h);
#if 0
		CStr255 name;
		short resID;
		ResType theType;
		GetResInfo(h, &resID, &theType, name);
		if (resID > 0)
			SetResidentSegment(resID, true);
#endif
	}
#endif
}
#endif

void LoadAllSegments()
{
#ifndef powerc
	short maxNo = CountResources('CODE');
	for (short i = 1; i <= maxNo; i++)
	{
		Handle h = GetIndResource('CODE', i);
		if (!h)
			continue;
		if (HGetState(h) & 0x0080) // is locked
			continue;
		HNoPurge(h);
		HLock(h); // heap not expanded, so no need to move hi
	}
#endif
}

void CreateStdErr()
{
#if qDebug & 0
	CStr255 s, num;
	unsigned long uli;
	GetDateTime(uli);
	NumToString(long(uli) & 0xFFFFF, num);
	s = "HD2:Desktop Folder:Nuntius_";
	s += num;
	FILE *file = fopen(s, "w");
	if (file)
	{
		fclose(file);
		fsetfileinfo(s, 'MPS ', 'TEXT');
		file = fopen(s, "w");
		_iob[2] = *file;  // _iob[2] = stderr
		Ptr p = NewPtr(300);
//		setvbuf(file, p, _IOLBF, 256); // flush at newline
	}
#endif
}

void OpenConnectionToTheWorld()
{
	if (GetResource('CMNU', mTest) == nil)
	{
		InitUMacTCP(); // no doubt when q_Final
	}
	else
	{
		KeyMap theKeys;
		GetKeys(theKeys);
		Boolean useNntp = (theKeys[1] & 1) == 0;
		if (gConfiguration.machineType == 26) // 26 == Q950
			useNntp = !useNntp;
		if (!useNntp)
			DoUseFakeNntp();
		else
			InitUMacTCP();
	}
#if qDebug
	fprintf(stderr, "Uses %s nntp\n", UsesFakeNntp() ? "fake" : "real");
#endif
	FlushVols();
}

void DoRealInitToolBox();
void main()
{
	CPoint pt = CPoint(1, 1); // to control which segment it gets to...
//	if (qDebug && IsControlKeyDown())
//		DebugStr("\pAt real start...");
	// Avoid RealInitToolBox as it checks for some named segments which
	// I don't have. All it does is to load them and check for a little
	// space left, and then call DoRealInitToolBox
 	Handle h = NewHandle(64 * 1024);
 	if (h == nil)
 		ExitToShell(); // way to little memory
 	DoRealInitToolBox();
	if (gConfiguration.systemVersion < 0x0700)
	{
		// ugly to check for version
		SetCursor(&qd.arrow);
		Alert(phNeedSystem7, nil);
		ExitToShell();
	}
	ShowSplashScreen();
	LoadAllSegments();
#ifndef powerc
	gUnloadAllSegs = false;
#endif
	InitUMacApp(50);
#ifndef powerc
	gUnloadAllSegs = false;
#endif
	InitUFatalError();
	InitUTools();
	CreateStdErr();
	FlushVols();
	InitUGridView();
	InitUTEView();
	InitUDialog();
	InitUPrinting();
//@	InitUFloatWindow();
	InitUFastBits();
	MyDontDeadStrip();
	A5RefSegmentLoader();
#if qDebug
	fprintf(stderr, "\n\n\n\n\n\n\n\n\n\nNuntius is running...\n");
#endif
	if (qDebug && IsControlKeyDown())
		DebugStr("\pJumping off...");
	OpenConnectionToTheWorld();
	FailInfo fi2;
	Try(fi2)
	{
		gNewsAppl = new TNewsAppl;
		gNewsAppl->INewsAppl();
		gNewsAppl->Run();
	}
	else
	{
		if (fi2.error)
			ErrorAlert(fi2.error, fi2.message);
	}
	if (gApplication)
		CleanupMacApp(); // MacApp only does it in qDebug
	if (gTopHandler)
		gTopHandler->Success();					// Remove the pFi failure handler
	ExecuteExitToShellPatches();
	// CleanupMacApp does it for fi2 (outermost handler), so
	// we have to do it for pFi
	FlushVols();
}
