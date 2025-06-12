// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UThread.cp

#include "UThread.h"
#include "UThreadBuiltIn.h"
#include "UThreadLongJmp.h"
#include "UThreadApple.h"
#include "UFatalError.h"
#include "UProgress.h"
#include "UProgressCache.h"

#include <RsrcGlobals.h>

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyThread

#define qDebugThreadsCreate qDebug & 0
#define qDebugYieldDisable qDebug & 0
#define qDebugYieldDisableVerbose qDebugYieldDisable & 0

Boolean gUsesThreadApple = false;
Boolean gUsesThreadLongJmp = false;

TThread *gHeadThread = nil;
TThread *gCurThread = nil;
TProgress *gCurProgress = nil;
TList *gAllThreads = nil;
TList *gAbortedThreads = nil;
TList *gScheduleBackThreads = nil;
TList *gThreadListList = nil;
Boolean gThreadUnitInited = false;
ArrayIndex gLastScheduleIndex = 1;
TThreadList *gApplWideThreads = nil;

//=======================================================================================
TThread *PreferedThreadToRunNow() // nil if no prefered
{
	TThread *thread = nil;
	if (gAbortedThreads->fSize)
	{
		thread = (TThread*)gAbortedThreads->Last();
#if qDebugSchedule
		fprintf(stderr, "MySchedule: Has %ld threads to abort/kill, returns TThread at $%lx\n", gAbortedThreads->GetSize(), long(thread));
#endif
	}
	else if (gScheduleBackThreads->fSize)
	{
		thread = (TThread*)gScheduleBackThreads->Pop();
#if qDebugSchedule
		fprintf(stderr, "MySchedule: Had %ld threads to schedule back to, returns TThread at $%lx\n", gScheduleBackThreads->GetSize() + 1, long(thread));
#endif
	}
	else
		return nil;
#if qDebug
	FailNonObject(thread);
#endif
	return thread;
}

TThread *FindOrdinaryThreadToRun()
{
	while (true)
	{
		if (++gLastScheduleIndex > gAllThreads->fSize)
			gLastScheduleIndex = 1;
		TThread *thread = (TThread*)gAllThreads->At(gLastScheduleIndex);
		if (thread->IsStarted() == false)
			continue;
		return thread;
	}
}

TThread *MySchedule()
{
	TThread *thread = PreferedThreadToRunNow();
	if (thread == nil)
	{
		if (MemSpaceIsLow())
		{
#if qDebug
			fprintf(stderr, "MySchedule: MemSpaceIsLow, so we swap to main thread\n");
#endif
			if (gLastScheduleIndex > 1) // if we ran lowspace in non-main thread, warn user now
			{
				InvalidateMenus();
				gApplication->SpaceIsLowAlert();
			}
			gLastScheduleIndex = 1; // user can abort threads to free up memory
		}
		thread = FindOrdinaryThreadToRun();
#if qDebugScheduleVerbose
		fprintf(stderr, "MySchedule: Index = %ld, TThread at $%lx\n", gLastScheduleIndex, long(thread));
#endif
	}
#if qDebug
	FailNonObject(thread);
#endif
	return thread;
}
//=======================================================================================
TThread::TThread()
{
}

pascal void TThread::Initialize()
{
	inherited::Initialize();
#if qDebug
	if (!gThreadUnitInited)
		ProgramBreak("Fatal: InitUThread not called before creating a TThread");
#endif
	fProgress = nil;
	fIsAborting = false;
	fIsKilling = false;
	fYieldIsDisabled = false;
	fSavedTopHandler = nil; // no link back in stack
	fLastYieldTick = 0;
	fIsStarted = false;
}

void TThread::IThread(Boolean isMainThread, const char *debugDoingWhat)
{
	inherited::IObject();
	FailInfo fi;
	if (fi.Try())
	{
		fIsMainThread = isMainThread;
		fDebugDoingWhat = debugDoingWhat;
		
		fProgress = gProgressCache->GetProgress();
		fProgress->SetThread(this);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
#if qDebugThreadsCreate
	CStr255 s;
	GetClassName(s);
	fprintf(stderr, "%s: TThread = $%lx, pgrss = $%lx,   %s\n", (char*)s, this, fProgress, debugDoingWhat);
#endif
}

pascal void TThread::Free()
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("Got a bad TThread!");
#endif
#if qDebugThreadsCreate
	fprintf(stderr, "FreeThread: TThread = $%lx, pgrss = $%lx,    %s\n", long(this), long(fProgress), fDebugDoingWhat);
#endif
	for (ArrayIndex index = 1; index <= gThreadListList->fSize; ++index)
	{
		TThreadList *tl = (TThreadList *)gThreadListList->At(index);
#if qDebug
		if (!VerboseIsObject(tl))
			ProgramBreak("Got a bad TThreadList!");
#endif
		tl->Delete(this);
	}
	gAllThreads->Delete(this);
	gAbortedThreads->Delete(this);
	gScheduleBackThreads->Delete(this);
	if (gAllThreads->GetEqualItemNo(this))
		PanicExitToShell("Twice in gAllThreads");
	if (gAbortedThreads->GetEqualItemNo(this))
		PanicExitToShell("Twice in gAbortedThreads");
	if (gScheduleBackThreads->GetEqualItemNo(this))
		PanicExitToShell("Twice in gScheduleBackThreads");
#if qDebug
	if (fProgress && !VerboseIsObject(fProgress))
		ProgramBreak("Got a bad fProgress!");
#endif
	if (fProgress)
		fProgress->WorkDone();
	gProgressCache->ReturnProgress(fProgress); fProgress = nil;

	Boolean isStarted = fIsStarted;
	inherited::Free();
	if (!isStarted)
		return; // is not in this thread, so don't overwrite current thread with another one
	// 'this' is now invalid, nonexisting
	gCurThread = nil;
}

void TThread::Die()
{
	if (fIsStarted)
	{
		while (gScheduleBackThreads->GetEqualItemNo(this) != kEmptyIndex)
		{
#if qDebug
			fprintf(stderr, "TThread::Free, this thread = $%lx is in gScheduleBackThreads\n", long(this));
#endif
			gScheduleBackThreads->Delete(this);
		}
		if (this == gHeadThread)
			PanicExitToShell("TThread::Free: Tried to Free() main thread (A mac without cpu? No, that wouldn't be nice)");
		if (gCurThread != this)
			PanicExitToShell("TThread::Free: 'this' is not gCurThread (I'm not me. Who? Me, not you!)");
	}
	//delete this;
	FreeIfObject(this);
}
//=========================================================================
void TThread::SwapThread()
{
	PanicExitToShell("TThread::SwapThread called");
}

//=========================================================================
TProgress *TThread::GetProgress()
{
	return fProgress;
}

void TThread::CheckYield()
{
	if (TickCount() - fLastYieldTick <= 4)
		return;
	YieldTime();
}

void TThread::YieldTime()
{
	if (this != gCurThread)
	{
#if qDebug
		ProgramBreak("TThread::YieldTime called for non current thread");
#endif
		return;
	}
	if (gAllThreads->fSize == 1)
	{
		gApplication->UpdateAllWindows();
		fLastYieldTick = TickCount();
		return;
	}
	if (fIsAborting)
		DoAbortThread();
	if (fYieldIsDisabled)
	{
#if qDebugSchedule | qDebugYieldDisable
		fprintf(stderr, "Thread at $%lx avoided yield as fYieldIsDisabled == true\n");
#endif
		return;
	}
#if qDebugScheduleVerbose
	static char prevTxt[100];
	sprintf(prevTxt, "< Thread at $%lx\n", long(this));
#endif
	SwapThread();
	if (fIsAborting)
		DoAbortThread();
}

void TThread::Abort()
{
#if qDebug
	fprintf(stderr, "Aborts thread at $%lx, abort flag %hd -> 1\n", long(this), fIsAborting);
#endif
	fIsAborting = true;
	if (gAbortedThreads->GetEqualItemNo(this) == kEmptyIndex)
		gAbortedThreads->InsertLast(this);
}

void TThread::Kill()
{
#if qDebug
	fprintf(stderr, "Kills thread at $%lx, kill flag %hd -> 1, fYieldIsDisabled = %hd\n", long(this), fIsAborting, fYieldIsDisabled);
#endif
	TThread *oldCurThread = gCurThread;
	gScheduleBackThreads->Push(gCurThread);
	fIsKilling = true;
	fIsAborting = true;
	if (gAbortedThreads->GetEqualItemNo(this) == kEmptyIndex)
		gAbortedThreads->InsertLast(this);
	// go kill it
	Boolean oldDisable = gCurThread->SetYieldDisable(false);
	gCurThread->YieldTime();								 // if kills this thread, doesn't return
	gCurThread->SetYieldDisable(oldDisable); // so no problem with this
	if (gCurThread != oldCurThread)
		PanicExitToShell("TThread::Kill: gCurThread is not oldCurThread");
	// 'this' refers to an unexisting object. Lets see if that's true
	// can't use GetEqualItemNo as it qDebug checks the object
	for (ArrayIndex index = 1; index <= gAllThreads->fSize; ++index)
		if (gAllThreads->TList::At(index) == this)
			PanicExitToShell("TThread::Kill, killed thread is still alive (call Ghostbuster(sp?))");
}

Boolean TThread::SetYieldDisable(Boolean disable)
{
	Boolean oldState = fYieldIsDisabled;
	if (oldState != disable)
	{
#if qDebugYieldDisableVerbose
		fprintf(stderr, "TThread::SetYieldDisable at $%lx, flag %ld -> %ld\n", long(this), long(oldState), long(disable));
#endif
		fYieldIsDisabled = disable;
	}
	return oldState;
}

void TThread::DoAbortThread()
{
#if qDebug
	if (!fIsAborting)
	{
		fprintf(stderr, "DoAbortThread: fIsAborting == %hd, fIsKilling = %hd\n", fIsAborting, fIsKilling);
		ProgramBreak(gEmptyString);
	}
#endif
#if qDebug
		fprintf(stderr, "DoAbortThread: thread at $%lx is being %s with Failure(0, 0)\n", long(this), fIsKilling ? "killed" : "aborted");
#endif
	if (!fIsKilling)
	{
		fIsAborting = false;
		gAbortedThreads->Delete(this);
	}
	Failure(0, 0);
}

Boolean TThread::IsAborted()
{
	return fIsAborting;
}

Boolean TThread::IsKilled()
{
	return fIsKilling;
}

Boolean TThread::IsStarted()
{
	return fIsStarted;
}

void TThread::SaveState()
{
	fSavedTopHandler = gTopHandler;
}

void TThread::RestoreState()
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("Got a bad TThread!");
#endif
// globals
	gCurThread = this;
	gCurProgress = fProgress;
// state
	gTopHandler = fSavedTopHandler;
// updates
	fLastYieldTick = TickCount();
}

const char *TThread::PeekShortDescription()
{
	return fDebugDoingWhat;
}

void TThread::DumpDebugDescription()
{
	CStr255 s;
	GetClassName(s);
	fprintf(stderr, " %s at $%lx: %s\n", (char*)s, long(this), fDebugDoingWhat);
	if (gCurThread == this)
		fprintf(stderr, "   is current thread\n");
	fprintf(stderr, "       fIsAborting = %lx, fIsKilling = %lx, fYieldIsDisabled = %lx\n", long(fIsAborting), long(fIsKilling), long(fYieldIsDisabled));
	fprintf(stderr, "       fSavedTopHandler = $%lx, fLastYieldTick = %ld, fProgress = $%lx\n", long(fSavedTopHandler), fLastYieldTick, long(fProgress));
}

//================== T H R E A D   U T I L I T I E S ===================================

void InitUThread()
{
	macroDontDeadStrip(TThreadList); // or SetEltType could fail
	gThreadListList = NewList();
	gThreadListList->SetEltType("TThreadList");
	gAllThreads = NewList();
	gAllThreads->SetEltType("TThread");
	gAbortedThreads = NewList();
	gAbortedThreads->SetEltType("TThread");
	gScheduleBackThreads = NewList();
	gScheduleBackThreads->SetEltType("TThread");
	gApplWideThreads = NewThreadList("Appl-wide/homeless threads");
	gThreadUnitInited = true;
	gUsesThreadApple = HasThreadApple();
	if (gUsesThreadApple == false)
		gUsesThreadLongJmp = HasThreadLongJmp();
#if qDebug
	if (gUsesThreadApple)
		fprintf(stderr, "Uses Apples Thread Manager\n");
	else if (gUsesThreadLongJmp)
		fprintf(stderr, "Uses builtin ThreadLongJmp\n");
	else
		fprintf(stderr, "Uses builtin thread swapper\n");
#endif
	TThread *t;
	if (gUsesThreadApple)
		t = CreateMainThreadApple();
	else if (gUsesThreadLongJmp)
		t = CreateMainThreadLongJmp();
	else
		t = CreateMainThreadBuiltIn();
	gCurThread = gHeadThread = t;
	gCurProgress = gCurThread->fProgress;
	gCurThread->SetYieldDisable(true);
#if qDebugThreadsCreate
	fprintf(stderr, "HeadThread is at $%lx\n", long(gHeadThread));
#endif
}

void CloseDownUThread()
{
	if (!gThreadUnitInited)
		return;	
	if (gAllThreads->GetSize() != 1)
	{
#if qDebug
		fprintf(stderr, "NoThreads == %ld != 1", gAllThreads->GetSize());
#endif
		PanicExitToShell("In CloseDownUThread(): head thread is not the only thread in this world");
	}
#if qDebug
	if (!VerboseIsObject(gHeadThread))
		ProgramBreak("CloseDownUThread: gHeadThread is not object");
#endif
	if (gHeadThread && gCurThread && gCurThread != gHeadThread)
		PanicExitToShell("In CloseDownUThread(): gCurThread != gHeadThread");
}

short GetNumThreads()
{
	return gAllThreads->fSize;
}

//==============================================================================
pascal void TThreadList::Initialize()
{
	inherited::Initialize();
	fDescription = nil;
}

void TThreadList::IThreadList(const char *debugListDescription)
{
	inherited::IList();	
	FailInfo fi;
	if (fi.Try())
	{
		SetEltType("TThread");
		fDescription = debugListDescription;
		gThreadListList->InsertLast(this);
		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}
}

pascal void TThreadList::Free()
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("I'm a really bad TThread");
#endif
	if (fSize)
	{
		DebugDump();
		strcpy(gPanicBuffer, "In TThreadList::Free, list is not empty. ListType: ");
		strcat(gPanicBuffer, fDescription ? fDescription : "<unknown>");
		strcat(gPanicBuffer, "\n");
		PanicExitToShell(gPanicBuffer);
	}
	gThreadListList->Delete(this);
	inherited::Free();
}

TThread *TThreadList::ThreadAt(ArrayIndex index)
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("I'm a really bad TThreadList");
#endif
	TThread *thread = (TThread *)At(index);
#if qDebug
	if (!VerboseIsObject(thread))
		ProgramBreak("Got a bad TThread!");
#endif
	return thread;
}

pascal void TThreadList::DeleteAll()
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("I'm a really bad TThreadList");
#endif
	if (qDebug)
		ProgramBreak("I would prefer if you called KillAll()");
	KillAll();
}

void TThreadList::KillAll()
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("I'm a really bad TThreadList");
#endif
	while (fSize)
	{
		ArrayIndex size = fSize;
		TThread *thread = ThreadAt(fSize);
		thread->Kill();
		if (gAllThreads->GetIdentityItemNo(thread) == kEmptyIndex && fSize == size)
			PanicExitToShell("In TThreadList::KillAll, I killed a thread, but its ghost is still in my list");
	}
}

TThread *TThreadList::ExecuteCommand(TCommand *command, const char *debugDoingWhat)
{
#if qDebug
	if (!VerboseIsObject(this))
		ProgramBreak("I'm a really bad TThreadList");
#endif
	TThread *thread = nil;
	VOLATILE(thread);
	FailInfo fi;
	if (fi.Try())
	{
		if (gUsesThreadApple)
			thread = ExecuteInNewThreadApple(command, debugDoingWhat);
		else if (gUsesThreadLongJmp)
			thread = ExecuteInNewThreadLongJmp(command, debugDoingWhat);
		else
			thread = ExecuteInNewThreadBuiltIn(command, debugDoingWhat);
		InsertLast(thread);
		fi.Success();
		return thread;
	}
	else // fail
	{
		if (thread)
			Delete(thread);
		FreeIfObject(thread); thread = nil;		
		fi.ReSignal();
	}
}

void TThreadList::DebugDump()
{
	fprintf(stderr, "  Dump of TThreadList at $%lx: %s\n", long(this), fDescription);
	if (fSize == 0)
		fprintf(stderr, "      <empty>\n");
	else
	{
		for (ArrayIndex index = 1; index <= fSize; index++)
		{
			TThread *thread = ThreadAt(index);
			fprintf(stderr, "     %2ld: at $%lx: %s\n", index, long(thread), thread->PeekShortDescription());
		}
	}
}

TThreadList *NewThreadList(const char *debugListDescription)
{
	TThreadList *tl = new TThreadList();
	tl->IThreadList(debugListDescription);
	return tl;
}
//==============================================================================
void ThreadExecuteCommand(TCommand *command)
{
	if (gTopHandler)
		PanicExitToShell("In ThreadExecuteCommand(): gTopHandler != nil");
	FailInfo fi;
	if (fi.Try())
	{
#if qDebugThreadCommands
		fprintf(stderr, "Executing command at $%lx in thread at $%lx\n", long(command), long(gCurThread));
#endif
		if (command)
			command->DoIt();
		FreeIfObject(command); command = nil;
		fi.Success();
	}
	else // fail
	{
		if (fi.error != noErr)
			gApplication->ShowError(fi.error, fi.message);
		FailInfo fi2;
		if (fi2.Try())
		{
			FreeIfObject(command); command = nil;
			fi2.Success();
		}
	}
	if (gTopHandler)
		PanicExitToShell("In ThreadExecuteCommand() post exec: gTopHandler != nil");
	gCurThread->Die();// does not return...
	PanicExitToShell("In ThreadExecuteCommand, returned from gCurThread->Die();");
}
//--------------------------------------------------------------------------------------
void DumpOneThreadList(TList *list)
{
	for (ArrayIndex index = 1; index <= list->GetSize(); index++)
	{
		TThread *thread = (TThread*)list->At(index);
		thread->DumpDebugDescription();
	}
}

void DumpDebugThreadDescription()
{
	if (gAllThreads == nil)
	{
		fprintf(stderr, "Thread Unit not Inited\n");
		return;
	}
	fprintf(stderr, "Debug dump of list of threads:  Uses %s\n",
		gUsesThreadApple ? "Apples Thread Manager" : "builtin thread swapper");
	DumpOneThreadList(gAllThreads);
	fprintf(stderr, "Aborted threads:\n");
	DumpOneThreadList(gAbortedThreads);
	fprintf(stderr, "gScheduleBackThreads:\n");
	DumpOneThreadList(gScheduleBackThreads);
	fprintf(stderr, "Dump of lists of lists of threads:\n");
	for (ArrayIndex index = 1; index <= gThreadListList->GetSize(); index++)
	{
		TThreadList *tl = (TThreadList*)gThreadListList->At(index);
		tl->DebugDump();
	}
	fprintf(stderr, "End of dump\n");
}
