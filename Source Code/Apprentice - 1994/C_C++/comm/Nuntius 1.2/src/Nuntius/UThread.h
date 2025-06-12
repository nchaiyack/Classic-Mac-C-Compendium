// Copyright © 1992-1993 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UThread.h

#define __UTHREAD__

class TProgress;

class TThread : public TObject
{
	public:
		TProgress *GetProgress();
		
		void YieldTime(); // always yield
		void CheckYield(); // yield only if enough time has passed

		void Abort();
		// TThread::Abort(): This only marks the thread is having to be aborted.
		// It does not call TThread::YieldTime.
		// The last aborted or killed thread will be the first swapped in.
		// When the thread is swapped in, it's aborted by Failure(0, 0) and the
		// abort flag is cleared
		
		void Kill();
		// TThread::Kill(): The thread is killed by marking it with "permanent abort",
		// calls TThread::Yieldtime. No other threads gets time until the thread is
		// gone, unless this threads marks another thread as being aborted/killed.
		// (A killed thread is too an aborted thread)

		Boolean IsAborted();
		Boolean IsKilled();
		Boolean IsStarted();
		
		Boolean SetYieldDisable(Boolean disable);
		
		virtual const char *PeekShortDescription();
		virtual void DumpDebugDescription();
		
		TThread();
		pascal void Initialize();
		void IThread(Boolean isMainThread, const char *debugDoingWhat);
		virtual void Die();

	protected:
		TProgress *fProgress;
		unsigned long fLastYieldTick;
		const char *fDebugDoingWhat;
		Boolean fIsMainThread;
		Boolean fIsStarted;

		void SaveState();	// called when swapping out
		void RestoreState(); // called when swapping in
		void DoAbortThread();

		virtual pascal void Free(); //virtual ~TThread(); // keep safe, may only be called by Die()

		virtual void SwapThread();
		
	private:
		friend void InitUThread();
		friend void CloseDownUThread();

		FailInfoPtr fSavedTopHandler;
		Boolean fIsAborting;
		Boolean fIsKilling;
		Boolean fYieldIsDisabled;
};

class TThreadList : public TList
{
	public:
		TThread *ExecuteCommand(TCommand *command, const char *debugDoingWhat);
		pascal void DeleteAll(); // do call KillAll()
		void KillAll();
		TThread *ThreadAt(ArrayIndex index);

		void DebugDump();
		
		virtual pascal void Initialize();
		void IThreadList(const char *debugListDescription);
		virtual pascal void Free(); // screams if any thread in list
	private:
		const char *fDescription;
};

// For your service:
extern TThread *gCurThread;
extern TThreadList *gApplWideThreads;
TThreadList *NewThreadList(const char *debugListDescription);


// Management stuff
void InitUThread();
void CloseDownUThread();
void DumpDebugThreadDescription();

// Only for subclasses
TThread *MySchedule();
void ThreadExecuteCommand(TCommand *command);

short GetNumThreads();
extern TList *gAllThreads;
