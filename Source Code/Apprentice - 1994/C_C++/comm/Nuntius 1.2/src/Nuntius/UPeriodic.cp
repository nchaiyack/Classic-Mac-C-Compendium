// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPeriodic.cp

#include "UPeriodic.h"
#include "UThread.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#pragma segment MyTools

#define qDebugThreadsPeriodic qDebug & 1

class TDoPeriodicActionCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		pascal void Initialize();
		void IDoPeriodicActionCommand(TPeriodicAction *pt);
		pascal void Free();
	private:
		TPeriodicAction *fPeriodicAction;
};

pascal void TDoPeriodicActionCommand::Initialize()
{
	inherited::Initialize();
	fPeriodicAction = nil;
}

void TDoPeriodicActionCommand::IDoPeriodicActionCommand(TPeriodicAction *pt)
{
	inherited::ICommand(cDoPeriodicActionCommand, nil, false, false, nil);
	fPeriodicAction = pt;
#if qDebugThreadsPeriodic
	fprintf(stderr, "Created TDoPeriodicActionCommand at $%lx\n", long(this));
#endif
}

pascal void TDoPeriodicActionCommand::Free()
{
#if qDebugThreadsPeriodic
	fprintf(stderr, "TDoPeriodicActionCommand::Free at $%lx, fPeriodicAction = $%lx\n", long(this), long(fPeriodicAction));
#endif
	fPeriodicAction = nil;
	inherited::Free();
}

pascal void TDoPeriodicActionCommand::DoIt()
{
#if qDebugThreadsPeriodic
	fprintf(stderr, "Executing TPeriodicAction at $%lx in thread at $%lx\n", long(fPeriodicAction), long(gCurThread));
	fprintf(stderr, " for service: %s\n", fPeriodicAction->GetDebugDescription());
#endif
	fPeriodicAction->DoPeriodic();
}

//----------

TPeriodicAction::TPeriodicAction()
{
}

pascal void TPeriodicAction::Initialize()
{
	inherited::Initialize();
	fThreadList = nil;
	fLastCheckTick = 0;
	fSleepTicks = 600000;
	fCoHandlerInstalled = false;
}

void TPeriodicAction::IPeriodicAction(Boolean useThread)
{
	inherited::IEventHandler(nil);
	FailInfo fi;
	if (fi.Try())
	{
#if qDebugThreadsPeriodic
		fprintf(stderr, "TPeriodicAction created at $%lx (%s)\n", long(this), GetDebugDescription());
#endif
		if (useThread)
			fThreadList = NewThreadList(GetDebugDescription());		
		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}
}	
	
pascal void TPeriodicAction::Free()
{
	if (fCoHandlerInstalled)
		PanicExitToShell("In TPeriodicAction::Free, fCoHandlerInstalled == true");
	if (fThreadList)
	{
		if (fThreadList->fSize)
			PanicExitToShell("In TPeriodicAction::Free, fThreadList->fSize > 0");
		FreeIfObject(fThreadList); fThreadList = nil;
	}
#if qDebugThreadsPeriodic
	fprintf(stderr, "TPeriodicAction at $%lx (%s) free'd\n", long(this), GetDebugDescription());
#endif
	inherited::Free();
}

void TPeriodicAction::Close()
{
#if qDebugThreadsPeriodic
	fprintf(stderr, "TPeriodicAction::Close at $%lx (%s)\n", long(this), GetDebugDescription());
#endif
	SetSleep(kMaxLong);
}

Boolean TPeriodicAction::IsBusy()
{
	return fThreadList->fSize > 0;
}

void TPeriodicAction::SetSleep(unsigned long sleepTicks)
{
#if qDebugThreadsPeriodic
	fprintf(stderr, "TPeriodicAction::SetSleep(%ld) at $%lx (%s)\n", sleepTicks, long(this), GetDebugDescription());
#endif
	fSleepTicks = sleepTicks;
	if (sleepTicks == kMaxLong)
	{
		if (fCoHandlerInstalled)
		{
			gApplication->InstallCohandler(this, false);
			fCoHandlerInstalled = false;
		}
		SetIdleFreq(kMaxLong);
		GoSleep();
		return;
	}
	if (!fCoHandlerInstalled)
	{
		gApplication->InstallCohandler(this, true);
		fCoHandlerInstalled = true;
	}
	SetIdleFreq(sleepTicks);
	if (SkipFirstCheckAfterWakeUp())
		fLastCheckTick = TickCount();
	else
		if (TickCount() - fLastCheckTick >= sleepTicks)
			WakeUp();
}

void TPeriodicAction::GoSleep()
{
	fThreadList->KillAll();
	if (fThreadList->fSize)
		PanicExitToShell("In TPeriodicAction::GoSleep, still got threads");
	fLastCheckTick = TickCount();
	fLastIdle = TickCount();
}

void TPeriodicAction::WakeUp()
{
	if (fThreadList->fSize == 0 && NeedTime())
	{
#if qDebugThreadsPeriodic
		fprintf(stderr, "TPeriodicAction at $%lx (%s) got nightmare\n", long(this), GetDebugDescription());
#endif
		fLastCheckTick = 0;
		fLastIdle = 0;
		TDoPeriodicActionCommand *cmd = new TDoPeriodicActionCommand();
		cmd->IDoPeriodicActionCommand(this);
		fThreadList->ExecuteCommand(cmd, GetDebugDescription());
	}
	else
	{
#if qDebugThreadsPeriodic
		if (fThreadList->fSize)
			fprintf(stderr, "TPeriodicAction at $%lx (%s) didn't want time\n", long(this), GetDebugDescription());
		else
			fprintf(stderr, "TPeriodicAction at $%lx (%s) is already getting time\n", long(this), GetDebugDescription());
#endif
	}
	fLastCheckTick = TickCount();
}

pascal Boolean TPeriodicAction::DoIdle(IdlePhase phase)
{
	if (phase != idleEnd && fThreadList->fSize == 0)
	{
		if (TickCount() - fLastCheckTick >= fSleepTicks * 9 / 8)
		{
			if (NeedTime())
				WakeUp();
			else
				fLastCheckTick = TickCount(); // skip this time
		}
	}
	return inherited::DoIdle(phase);
}

Boolean TPeriodicAction::NeedTime()
{
	return true;
}

Boolean TPeriodicAction::SkipFirstCheckAfterWakeUp()
{
	return false;
}

void TPeriodicAction::DoPeriodic()
{
}

const char *TPeriodicAction::GetDebugDescription()
{
	return "(unspecified)";
}

void TPeriodicAction::DumpDebugDescription()
{
	fprintf(stderr, "  %s at $%lx\n", GetDebugDescription(), long(this));
	fprintf(stderr, "              fCoHandInst = %ld, fSleepTicks = %ld\n", long(fCoHandlerInstalled), fSleepTicks);
	fprintf(stderr, "              fLastCheckTick = %ld, fThreadList->fSize = $%lx\n", fLastCheckTick, fThreadList->fSize);
}
