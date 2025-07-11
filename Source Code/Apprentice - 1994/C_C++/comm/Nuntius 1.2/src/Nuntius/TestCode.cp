// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// TestCode.cp

#include "TestCode.h"
#include <RsrcGlobals.h>
#include <Resources.h>

#pragma segment MyMisc

//void Test1() {}
void Test2() {}
void Test3() {}
//void Test4() {}


#include "UThread.h"
#include <stdio.h>

void Test1()
{
	unsigned long st =TickCount();
	long num = 00;
	while (TickCount() - st < 60)
	{
		gCurThread->YieldTime();
		num++;
	}
	fprintf(stderr, "Yields = %ld\n", num);
}