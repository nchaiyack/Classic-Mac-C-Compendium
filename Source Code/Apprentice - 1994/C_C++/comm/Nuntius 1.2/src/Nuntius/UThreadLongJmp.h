// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadLongJmp.h

#ifndef __UTHREAD__
#include "UThread.h"
#endif

struct LongJmpBuf68000
{
	long D2, D3, D4, D5, D6, D7;
	const void *PC;
	long A2, A3, A4, A6;
	long *SP;
	long *fHeapEnd;
	long *fApplLimit;
	long *fHiHeapMark;
	Ptr fStack, fStackTop;
	long fID;
	// This struct has ofcourse a _close_ relationship 
	// to the asm i UThread.a
};

extern "C" {
	Boolean MySetJmp68000(LongJmpBuf68000 &);
	void    MyLongJmp68000(LongJmpBuf68000 &, Ptr ptrToDispose);
}

class TThreadLongJmp : public TThread
{
	public:
		void StartThread(TCommand *cmd);
		virtual void DumpDebugDescription();

		TThreadLongJmp();
		virtual pascal void Initialize();
		void IThreadLongJmp(Boolean isMainThread, const char *debugDoingWhat);
		virtual void Die();
		
	protected:
		virtual pascal void Free(); // virtual ~TThreadLongJmp();

		virtual void SwapThread();

	private:
		LongJmpBuf68000 fLongJmpBuf;

		void ExecCmd(TCommand *cmd);
		void SwapIn(Ptr ptrToDispose);
};

Boolean HasThreadLongJmp();
TThread *CreateMainThreadLongJmp();

TThread *ExecuteInNewThreadLongJmp(TCommand *command, const char *debugDoingWhat);
