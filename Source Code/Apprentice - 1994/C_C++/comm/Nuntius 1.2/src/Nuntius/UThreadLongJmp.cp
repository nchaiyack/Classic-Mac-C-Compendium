// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadLongJmp.cp

#include "UThreadLongJmp.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#include <GestaltEqu.h>
#include <SysEqu.h>

#pragma segment MyThread

#define qDebugSchedule qDebug & 0

const long kInitialStackSize = 24 * 1024;
long gThreadLongJmpID = 99;

//==============================================================================

TThreadLongJmp::TThreadLongJmp()
{
}

pascal void TThreadLongJmp::Initialize()
{
	inherited::Initialize();
	BlockSet(Ptr(&fLongJmpBuf), sizeof(fLongJmpBuf), 0);
	fLongJmpBuf.fStack = nil;
}

void TThreadLongJmp::IThreadLongJmp(Boolean isMainThread, const char *debugDoingWhat)
{
	inherited::IThread(isMainThread, debugDoingWhat);
	FailInfo fi;
	if (fi.Try())
	{
		fLongJmpBuf.fID = ++gThreadLongJmpID;
		if (isMainThread == false)
		{
			*( (void**)StkLowPt ) = 0;
			fLongJmpBuf.fStack = NewPermPtr(kInitialStackSize);
			Ptr stackEndP = fLongJmpBuf.fStack + kInitialStackSize;
			fLongJmpBuf.fStackTop = stackEndP - 1;
			fLongJmpBuf.SP = (long*)stackEndP - 64;
			fLongJmpBuf.fHeapEnd = (long*)fLongJmpBuf.fStack;
			fLongJmpBuf.fApplLimit = (long*)fLongJmpBuf.fStack;
			fLongJmpBuf.fHiHeapMark = (long*)fLongJmpBuf.fStack;
		}
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

pascal void TThreadLongJmp::Free()
{
	DisposeIfPtr(fLongJmpBuf.fStack); fLongJmpBuf.fStack = nil;
	inherited::Free();
}

void TThreadLongJmp::Die()
{
#if qDebugSchedule
	fprintf(stderr, "ThreadDie, id = %ld, %s\n", fLongJmpBuf.fID, fDebugDoingWhat);
#endif
	Ptr stack = fLongJmpBuf.fStack; // we're going to shoot ourself!
	fLongJmpBuf.fStack = nil;
	inherited::Die();
	TThreadLongJmp *nextThread = (TThreadLongJmp*)MySchedule();
	nextThread->SwapIn(stack);
	PanicExitToShell("In TThreadLongJmp::Die, got past end of world");
}

//-----------------------------------------------------------------------------
void TThreadLongJmp::SwapIn(Ptr ptrToDispose)
{
	MyLongJmp68000(fLongJmpBuf, ptrToDispose);
}

//==============================================================================

void TThreadLongJmp::ExecCmd(TCommand *cmd)
{
	FailNonObject(this);
	RestoreState();
	if (qDebugSchedule)
		fprintf(stderr, "Switch++,  id = %ld, cmd = $%lx, %s\n", fLongJmpBuf.fID, cmd, fDebugDoingWhat);
	ThreadExecuteCommand(cmd);
	PanicExitToShell("In TThreadLongJmp::ExecCmd, returned from ThreadExecuteCommand");
}

void TThreadLongJmp::StartThread(TCommand *cmd)
{
	gAllThreads->InsertLast(this);
	fIsStarted = true;
#if qDebugSchedule
	fprintf(stderr, "Starts LongJmpThread id = %ld (%s)\n", fLongJmpBuf.fID, fDebugDoingWhat);
#endif
	if (fIsMainThread)
		return;
	fLongJmpBuf.PC = (const void*)&TThreadLongJmp::ExecCmd;
	fLongJmpBuf.A6 = 0;
	long *sp = fLongJmpBuf.SP;
	// when called, SP points to return address (non-existing)
	sp[0] = 0xF1F1F1F1;
	sp[1] = long(this);
	sp[2] = long(cmd);
}

void TThreadLongJmp::SwapThread()
{
	TThreadLongJmp *nextThread = (TThreadLongJmp*)MySchedule();
	VOLATILE(nextThread);
	if (nextThread == this)
	{
#if qDebug
		fprintf(stderr, "MySchedule to current thread, id = %ld, %s\n", fLongJmpBuf.fID, fDebugDoingWhat);
#endif
		return;
	}
	SaveState();
	if (qDebugSchedule)
		fprintf(stderr, "SwitchOut, id = %ld, %s\n", fLongJmpBuf.fID, fDebugDoingWhat);
	if (MySetJmp68000(fLongJmpBuf) == 0) // zero after set, nonzero after return
		nextThread->SwapIn(nil);
	if (qDebugSchedule)
		fprintf(stderr, "SwitchIn,  id = %ld, %s\n", fLongJmpBuf.fID, fDebugDoingWhat);
	RestoreState();
}

void TThreadLongJmp::DumpDebugDescription()
{
	inherited::DumpDebugDescription();
	long stackFree = long(fLongJmpBuf.SP) - long(fLongJmpBuf.fStack);
	fprintf(stderr, "       id = %ld, stack-free = %ld\n", fLongJmpBuf.fID, stackFree);
}
//===============================================================================
TThread *ExecuteInNewThreadLongJmp(TCommand *command, const char *debugDoingWhat)
{
#if qDebugSchedule
	fprintf(stderr, "ExecuteInNewThreadLongJmp, cmd = $%lx, doing: %s\n", command, debugDoingWhat);
#endif
	TThreadLongJmp *thread = nil;
	VOLATILE(thread);
	//
	FailInfo fi;
	if (fi.Try())
	{
		TThreadLongJmp *ta = new TThreadLongJmp();
		ta->IThreadLongJmp(false, debugDoingWhat);
		thread = ta;
		//
		FailSpaceIsLow();
		thread->StartThread(command);
		//
		fi.Success();
		return thread;
	}
	else // fail
	{
		FreeIfObject(command); command = nil;
		FreeIfObject(thread); thread = nil;
		fi.ReSignal();
	}
}
//===========================================================================
Boolean HasThreadLongJmp()
{
	long resp;
	// not 100% compatible with MacPlus, just like Apples Thread Manager:
	if (Gestalt(gestaltMachineType, resp) != noErr)
		return false;
	if (resp == gestaltMacPlus)
		return false;
	return true;
}

TThread *CreateMainThreadLongJmp()
{
	TThreadLongJmp *thread = new TThreadLongJmp();
	thread->IThreadLongJmp(true, "main thread");
	thread->StartThread(nil);
	return thread;
}
