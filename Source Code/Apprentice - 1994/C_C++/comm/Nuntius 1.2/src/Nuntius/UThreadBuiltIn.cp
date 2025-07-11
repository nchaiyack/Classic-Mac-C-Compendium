// Copyright � 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadBuiltIn.cp

#include "UThreadBuiltIn.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#pragma segment MyThread

#define qDebugThreadCommands qDebug & 0

#define qDebugSchedule qDebug & 0
#define qDebugScheduleVerbose qDebugSchedule & 0

#define qDebugStackSize qDebug & 1

const long kInitialStackSize = 24 * 1024;

long gLastMaxUsedStackSize = 4000;

extern "C" MySwapStacks(Handle fromStackH, Handle toStackH);
// Used in asm code: (UThread.a)

Handle gFromStackH, gToStackH;
typedef struct
{
	long fAllocSize;
	long fStartAddr;
	long fSize;
	long fSP;
	long fDataOffset;
} *StackSaveHandleDataPtr;


//==============================================================================

TThreadBuiltIn::TThreadBuiltIn()
{
}

pascal void TThreadBuiltIn::Initialize()
{
	inherited::Initialize();
	fStackH = nil;
}

void TThreadBuiltIn::IThreadBuiltIn(Boolean isMainThread, const char *debugDoingWhat)
{
	inherited::IThread(isMainThread, debugDoingWhat);
	FailInfo fi;
	if (fi.Try())
	{
		fStackH = NewPermHandle(kInitialStackSize);
		// see UThread.a: initial values
		StackSaveHandleDataPtr sshdP = StackSaveHandleDataPtr(*fStackH);
		sshdP->fAllocSize = kInitialStackSize;
		sshdP->fStartAddr = 0;
		sshdP->fSize = 0;
		sshdP->fSP = 0;
		sshdP->fDataOffset = 0;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

pascal void TThreadBuiltIn::Free()
{
	DisposeIfHandle(fStackH); fStackH = nil;
	inherited::Free();
}

void TThreadBuiltIn::Die()
{
	inherited::Die();
	TThreadBuiltIn *thread = (TThreadBuiltIn*)MySchedule();
	thread->OverwriteWithThisThread();
	PanicExitToShell("In TThreadBuiltIn::Die, got past end of world.");
}
//==============================================================================

void TThreadBuiltIn::StartThread()
{
	// NO LOCALS ARE ALLOWED as SwapStacks are called
	//
	if (gAllThreads->GetEqualItemNo(this) != kEmptyIndex)
		PanicExitToShell("Trying to StartThread() a running thread");
	MySwapStacks(fStackH, nil);
#if qDebugThreadsCreate
	fprintf(stderr, "StartThread() at $%lx after MySwapStacks(), gAllThreads index = %ld\n", long(this), gAllThreads->GetEqualItemNo(this));
#endif
	if (gAllThreads->GetEqualItemNo(this) != kEmptyIndex)
	{
		// the code below have been executed and then we are the new thread
		RestoreState();
		return;
	}
	gAllThreads->InsertLast(this);
	fIsStarted = true;
}

void TThreadBuiltIn::SwapThread()
{
	TThreadBuiltIn *nextThread = (TThreadBuiltIn*)MySchedule();
	if (nextThread == this)
	{
#if qDebug
		fprintf(stderr, "Yeild (MySchedule) to current thread\n");
#endif
		return;
	}
	SaveState();
	MySwapStacks(fStackH, nextThread->fStackH);
	RestoreState();
#if qDebugStackSize
	if (StackSaveHandleDataPtr(*fStackH)->fSize > gLastMaxUsedStackSize)
	{
		fprintf(stderr, "New max stack size: %ld (old = %ld)\n", StackSaveHandleDataPtr(*fStackH)->fSize, gLastMaxUsedStackSize);
		gLastMaxUsedStackSize = StackSaveHandleDataPtr(*fStackH)->fSize;
	}
#endif
}

void TThreadBuiltIn::OverwriteWithThisThread()
{
	FailNonObject(this);
#if qDebugThreadsCreate
	fprintf(stderr, "OverwriteWithThisThread for thread at %lx\n", long(this));
#endif
	MySwapStacks(nil, fStackH);
#if qDebug
	FailNonObject(this);
#endif
	RestoreState();
	if (gCurThread != this)
		PanicExitToShell("TThreadBuiltIn::OverwriteWithThisThread: 'this' is not gCurThread");
	fLastYieldTick = TickCount();
#if qDebugScheduleVerbose
	fprintf(stderr, "� Thread at $%lx\n", long(this));
#endif
}

void TThreadBuiltIn::DumpDebugDescription()
{
	inherited::DumpDebugDescription();
	fprintf(stderr, "       used-stack = %ld, stack-alloc = %ld\n", StackSaveHandleDataPtr(*fStackH)->fSize, StackSaveHandleDataPtr(*fStackH)->fAllocSize);
}
//===============================================================================
class TExecuteCommandInNewThreadCommand : public TCommand
{
	public:
		pascal void DoIt();

		pascal void Initialize();
		void IExecuteCommandInNewThreadCommand(TCommand *command, TThread *thread); // owns the thread
		pascal void Free();
	protected:
		TCommand *fThreadCommand;
		TThread *fThread;
};


pascal void TExecuteCommandInNewThreadCommand::Initialize()
{
	inherited::Initialize();
	fThreadCommand = nil;
	fThread = nil;
}

void TExecuteCommandInNewThreadCommand::IExecuteCommandInNewThreadCommand(TCommand *command, TThread *thread)
{
	inherited::ICommand(cExecuteCommandInNewThreadCommand, nil, false, false, nil);
	fThreadCommand = command;
	fThread = thread;
}

pascal void TExecuteCommandInNewThreadCommand::Free()
{
	FreeIfObject(fThreadCommand); fThreadCommand = nil;
	if (fThread)
		fThread->Die();
	fThread = nil;
	inherited::Free();
}

pascal void TExecuteCommandInNewThreadCommand::DoIt()
{
	TCommand *cmd = fThreadCommand;
	VOLATILE(cmd);
	fThreadCommand = nil;

	TThread *thread = fThread;
	VOLATILE(thread);
	fThread = nil;

	( (TThreadBuiltIn*)thread)->StartThread();
	// 'this' is invalid from now on, as the command object is disposed
	if (gCurThread != thread)
		return;
	ThreadExecuteCommand(cmd);
}

TThread *ExecuteInNewThreadBuiltIn(TCommand *command, const char *debugDoingWhat)
{
	TThread *thread = nil;
	VOLATILE(thread);
	TExecuteCommandInNewThreadCommand *cmd = nil;
	VOLATILE(cmd);
	FailInfo fi;
	if (fi.Try())
	{
		TThreadBuiltIn *t = new TThreadBuiltIn();
		t->IThreadBuiltIn(false, debugDoingWhat);
		thread = t;
		FailSpaceIsLow();
		
		TExecuteCommandInNewThreadCommand *c = new TExecuteCommandInNewThreadCommand();
		c->IExecuteCommandInNewThreadCommand(command, thread);
		cmd = c;
		
		FailSpaceIsLow();
		gApplication->PostCommand(cmd);
		fi.Success();
		return thread;
	}
	else // fail
	{
		FreeIfObject(cmd); cmd = nil;
		FreeIfObject(thread); thread = nil;
		FreeIfObject(command); command = nil;
		fi.ReSignal();
	}
}
//===========================================================================
TThread *CreateMainThreadBuiltIn()
{
	TThreadBuiltIn *thread = new TThreadBuiltIn();
	thread->IThreadBuiltIn(true, "main thread");
	thread->StartThread();
	return thread;
}
