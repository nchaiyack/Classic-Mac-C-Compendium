// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPeriodic.h

#define __UPERIODIC__

class TDoTPeriodicActionCommand;
class TThreadList;

class TPeriodicAction : public TEventHandler
{
	public:
		virtual pascal Boolean DoIdle(IdlePhase phase); // override DoPeriodic

		virtual void WakeUp();
		virtual void GoSleep();
		virtual void SetSleep(unsigned long sleepTicks);
		virtual Boolean IsBusy(); // true if having thread

		virtual void DumpDebugDescription();
		
		TPeriodicAction();
		virtual pascal void Initialize();
		void IPeriodicAction(Boolean useThread);
		virtual void Close();
		virtual pascal void Free();

	protected:
		virtual void DoPeriodic(); // override to do your stuff		
		virtual const char *GetDebugDescription(); // eg classname

		virtual Boolean NeedTime(); // default is true
		virtual Boolean SkipFirstCheckAfterWakeUp(); // default is false

	private:
		friend TDoPeriodicActionCommand;
		
		TThreadList *fThreadList;
		unsigned long fLastCheckTick;
		unsigned long fSleepTicks;
		Boolean fCoHandlerInstalled;
};
