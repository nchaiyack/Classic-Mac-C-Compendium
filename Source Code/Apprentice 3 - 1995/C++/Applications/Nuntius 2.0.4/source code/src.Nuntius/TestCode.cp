// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// TestCode.cp

#include "TestCode.h"
#include "RsrcGlobals.h"

#include <Resources.h>

#include <stdio.h>

#pragma segment MyMisc

//void Test1() {}
//void Test2() {}
void Test3() {}
//void Test4() {}


#include "UThread.h"
#include <stdio.h>

class TTestThreadSwitchSpeedCommand : public TCommand
{
	public:
		void DoIt();
		
		TTestThreadSwitchSpeedCommand();
		void ITestThreadSwitchSpeedCommand();
};

TTestThreadSwitchSpeedCommand::TTestThreadSwitchSpeedCommand()
{
}

void TTestThreadSwitchSpeedCommand::ITestThreadSwitchSpeedCommand()
{
	TCommand::ICommand(cGenericCommand, nil, false, false, nil);
}

void TTestThreadSwitchSpeedCommand::DoIt()
{
	unsigned long startTick = TickCount();
	long num = 0;
	while (TickCount() - startTick < 60)
	{
		gCurThread->YieldTime();
		num++;
	}
	fprintf(stderr, "Yields = %ld\n", num);
}

void Test1()
{
	TTestThreadSwitchSpeedCommand *aCommand = new TTestThreadSwitchSpeedCommand();
	aCommand->ITestThreadSwitchSpeedCommand();
	gApplWideThreads->ExecuteCommand(aCommand, "TTestThreadSwitchSpeedCommand");
}
