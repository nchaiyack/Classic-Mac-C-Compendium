// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadApple.cp

#include "UThreadApple.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#include <GestaltEqu.h>

#pragma segment MyThread

#define qDebugThreadCommands qDebug & 0

#define qDebugSchedule qDebug & 0

const long kInitialStackSize = 24 * 1024;

//==============================================================================

TThreadApple::TThreadApple()
{
}

pascal void TThreadApple::Initialize()
{
	inherited::Initialize();
	fThreadID = kNoThreadID;
}

void TThreadApple::IThreadApple(Boolean isMainThread, const char *debugDoingWhat)
{
	inherited::IThread(isMainThread, debugDoingWhat);
}

pascal void TThreadApple::Free()
{
	if (fIsMainThread == false && fThreadID != kNoThreadID)
	{
		OSErr err = DisposeThread(fThreadID, 0, false);
		if (err != noErr)
		{
			sprintf(gPanicBuffer, "Got OSErr %ld from DisposeThread", long(err));
			PanicExitToShell(gPanicBuffer);
		}
	}
	inherited::Free();
}

void TThreadApple::Die()
{
#if qDebugSchedule
	fprintf(stderr, "ThreadDie, id = %ld, %s\n", fThreadID, fDebugDoingWhat);
#endif
	ThreadID id = fThreadID; // we're going to shoot ourself!
	fThreadID = kNoThreadID;
	inherited::Die();
#if qDebugSchedule
	fprintf(stderr, "ThreadDie, Goes disposing of: id = %ld\n", id);
#endif
	OSErr err = DisposeThread(id, 0, false);
	sprintf(gPanicBuffer, "In TThreadApple::Die, got past end of world with OSErr %ld", long(err));
	PanicExitToShell(gPanicBuffer);
}

//-----------------------------------------------------------------------------
void TThreadApple::CallSaveState()
{
	SaveState();
}

void TThreadApple::CallRestoreState()
{
	RestoreState();
}

pascal void ThreadAppleSwitchIn(ThreadID threadID, void *param)
{
	long oldA5 = SetCurrentA5();
	TThreadApple *thread = (TThreadApple*)param;
	if (VerboseIsObject(thread))
	{
		if (qDebugSchedule)
			fprintf(stderr, "SwitchIn,  id = %ld, param = $%lx, %s\n", threadID, param, thread->PeekShortDescription());
		thread->CallRestoreState();
	}
	else
		if (qDebug)
			DebugStr("\pUps, its not an object!");
	SetA5(oldA5);
}

pascal void ThreadAppleSwitchOut(ThreadID threadID, void *param)
{
	long oldA5 = SetCurrentA5();
	TThreadApple *thread = (TThreadApple*)param;
	if (VerboseIsObject(thread))
	{
		if (qDebugSchedule)
			fprintf(stderr, "SwitchOut, id = %ld, param = $%lx, %s\n", threadID, param, thread->PeekShortDescription());
		thread->CallSaveState();
	}
	else
		if (qDebug)
			DebugStr("\pUps, its not an object!");
	SetA5(oldA5);
}

void TThreadApple::InstallSwitchers()
{
	OSErr err = SetThreadSwitcher(fThreadID, ThreadAppleSwitchIn, this, true);
#if qDebug
	if (err)
		fprintf(stderr, "OSErr from SetThreadSwitcher.In = %ld\n", long(err));
#endif
	err = SetThreadSwitcher(fThreadID, ThreadAppleSwitchOut, this, false);
#if qDebug
	if (err)
		fprintf(stderr, "OSErr from SetThreadSwitcher.Out = %ld\n", long(err));
#endif
}
//==============================================================================

void TThreadApple::StartThread(ThreadID threadID)
{
	if (fThreadID != kNoThreadID)
		PanicExitToShell("Has already a threadID");
	fThreadID = threadID;
	InstallSwitchers();
	gAllThreads->InsertLast(this);
	fIsStarted = true;
#if qDebugSchedule
	fprintf(stderr, "Starts AppleThread id %ld at $%lx (%s)\n", threadID, this, fDebugDoingWhat);
#endif
	FailOSErr(SetThreadState(fThreadID, kReadyThreadState, fThreadID));
}

void TThreadApple::SwapThread()
{
	TThreadApple *nextThread = (TThreadApple*)MySchedule();
	if (nextThread == this)
	{
#if qDebug
		fprintf(stderr, "Yeild (MySchedule) to current thread\n");
#endif
		return;
	}
	OSErr err = YieldToThread(nextThread->fThreadID);
#if qDebug
	if (err != noErr)
		fprintf(stderr, "Got error from YieldToThread: %ld\n", long(err));
#endif
}

void TThreadApple::DumpDebugDescription()
{
	inherited::DumpDebugDescription();
	unsigned long stackFree;
	FailOSErr(ThreadCurrentStackSpace(fThreadID, &stackFree));
	fprintf(stderr, "       stack-free = %ld\n", stackFree);
}
//===============================================================================
pascal void *ThreadAppleEntry(void *param)
{
	TCommand *cmd = (TCommand*)param;
#if qDebug
	if (!VerboseIsObject(cmd))
		ProgramBreak("param is bad!");
#endif
	ThreadExecuteCommand(cmd);
	return 0;
}

TThread *ExecuteInNewThreadApple(TCommand *command, const char *debugDoingWhat)
{
#if qDebugSchedule
	fprintf(stderr, "ExecuteInNewThreadApple, cmd = $%lx, doing: %s\n", command, debugDoingWhat);
#endif
	TThreadApple *thread = nil;
	VOLATILE(thread);
	//
	ThreadID threadID = kNoThreadID;
	VOLATILE(threadID);
	//
	FailInfo fi;
	if (fi.Try())
	{
		TThreadApple *ta = new TThreadApple();
		ta->IThreadApple(false, debugDoingWhat);
		thread = ta;
		//
		ThreadID threadID;
		FailOSErr(NewThread(kCooperativeThread, ThreadAppleEntry, command,
				kInitialStackSize, kNewSuspend|kCreateIfNeeded|kFPUNotNeeded, nil, &threadID));
		//
		FailSpaceIsLow();
		thread->StartThread(threadID);
		//
		fi.Success();
		return thread;
	}
	else // fail
	{
		FreeIfObject(command); command = nil;
		FreeIfObject(thread); thread = nil;
		if (threadID != kNoThreadID)
		{
			OSErr err = DisposeThread(kNoThreadID, 0, false);
			if (err != noErr)
			{
				sprintf(gPanicBuffer, "Got OSErr %ld from DisposeThread", long(err));
				PanicExitToShell(gPanicBuffer);
			}
		}
		fi.ReSignal();
	}
}
//===========================================================================
Boolean HasThreadApple()
{
#if qDebug
	return false; // want to test my own
#else
	long resp;
	if (Gestalt(gestaltThreadMgrAttr, resp) != noErr)
		return false;
	// not 100% compatible with MacPlus:
	if (Gestalt(gestaltMachineType, resp) != noErr)
		return false;
	if (resp == gestaltMacPlus)
		return false;
	return true;
#endif
}

TThread *CreateMainThreadApple()
{
	TThreadApple *thread = new TThreadApple();
	thread->IThreadApple(true, "main thread");
	ThreadID threadID;
	if (GetCurrentThread(&threadID) != noErr)
		PanicExitToShell("GetCurrentThread won't play");
	thread->StartThread(threadID);
	return thread;
}
