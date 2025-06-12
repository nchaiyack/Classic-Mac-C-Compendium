// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadApple.h

#ifndef __UTHREAD__
#include "UThread.h"
#endif

#ifndef __THREADS__
#include "Threads.h"
#endif

class TThreadApple : public TThread
{
	public:
		void StartThread(ThreadID threadID);
		void CallSaveState();
		void CallRestoreState();
		virtual void DumpDebugDescription();

		TThreadApple();
		virtual pascal void Initialize();
		void IThreadApple(Boolean isMainThread, const char *debugDoingWhat);
		virtual void Die();
		
	protected:
		virtual pascal void Free(); // virtual ~TThreadApple();

		virtual void SwapThread();

	private:
		ThreadID fThreadID;

		void InstallSwitchers();
};

Boolean HasThreadApple();
TThread *CreateMainThreadApple();

TThread *ExecuteInNewThreadApple(TCommand *command, const char *debugDoingWhat);
