// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// MNuntius.cp

#include "UNewsAppl.h"
#include "UMacTcp.h"
#include "UNntp.h"
#include "FileTools.h"
#include "FastBits.h"
#include "MyDontDeadStrip.h"
#include "Tools.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>
#include <Memory.h>
#include <UGridView.h>
#include <UDialog.h>
#include <UTEView.h>
#include <UPrinting.h>
#include <Resources.h>
#include <Packages.h>
#include <UFloatWindow.h>

#pragma segment Main

void MakeAllSegmentsResident()
{
#if 0
	gUnloadAllSegs = false;
	short maxNo = CountResources('CODE');
	for (short i = 1; i <= maxNo; i++)
	{
		Handle h = GetIndResource('CODE', i);
		FailNILResource(h);
		CStr255 name;
		short resID;
		ResType theType;
		GetResInfo(h, resID, theType, name);
		if (resID > 0)
		{
			HLockHi(h);
			SetResidentSegment(resID, true);
		}
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

void main()
{
	if (qDebug && IsControlKeyDown())
		DebugStr("At real start...");
 	InitToolBox();
	if (gConfiguration.systemVersion < 0x0700)
	{
		// ugly to check for version
		SetCursor(qd.arrow);
		Alert(phNeedSystem7, nil);
		ExitToShell();
	}
	ShowSplashScreen();
	InitUMacApp(50);
	InitUFatalError();
	InitUPtrObject();
	InitUTools();
	CreateStdErr();
	FlushVols();
	MakeAllSegmentsResident();
	InitUGridView();
	InitUTEView();
	InitUDialog();
	InitUPrinting();
//@	InitUFloatWindow();
	InitUFastBits();
	MyDontDeadStrip();
	if (qDebug && IsControlKeyDown())
		DebugStr("Jumping off...");
	OpenConnectionToTheWorld();

	gNewsAppl = new TNewsAppl;
	gNewsAppl->INewsAppl();
	gNewsAppl->Run();
	gNewsAppl = nil; // CleanupMacApp frees gApplication
}
