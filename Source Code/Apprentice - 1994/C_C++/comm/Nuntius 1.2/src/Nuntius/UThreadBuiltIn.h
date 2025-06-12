// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UThreadBuiltIn.h

#ifndef __UTHREAD__
#include "UThread.h"
#endif

class TThreadBuiltIn : public TThread
{
	public:
		void StartThread();

		virtual void DumpDebugDescription();

		TThreadBuiltIn();
		virtual pascal void Initialize();
		void IThreadBuiltIn(Boolean isMainThread, const char *debugDoingWhat);
		virtual void Die();
		
	protected:
		virtual pascal void Free(); // virtual ~PThreadBuiltIn();

		virtual void SwapThread();
		void OverwriteWithThisThread();

	private:
		Handle fStackH;		
};

TThread *CreateMainThreadBuiltIn();

TThread *ExecuteInNewThreadBuiltIn(TCommand *command, const char *debugDoingWhat);
