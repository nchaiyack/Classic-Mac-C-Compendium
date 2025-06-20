/* CheckNameUniqueness.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "CheckNameUniqueness.h"
#include "MainWindowStuff.h"
#include "Memory.h"
#include "SampleList.h"
#include "AlgoSampList.h"
#include "WaveTableList.h"
#include "AlgoWaveTableList.h"
#include "InstrList.h"
#include "SampleObject.h"
#include "AlgoSampObject.h"
#include "WaveTableObject.h"
#include "AlgoWaveTableObject.h"
#include "InstrObject.h"
#include "Array.h"
#include "DataMunging.h"
#include "Alert.h"


static void					CleanUpStringArray(ArrayRec* Array)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(Array);
		Limit = ArrayGetLength(Array);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ReleasePtr((char*)ArrayGetElement(Array,Scan));
			}
		DisposeArray(Array);
	}


static MyBoolean		TestArrays(ArrayRec* Array)
	{
		long							Limit;
		long							FirstScan;
		long							SecondScan;
		char*							Str1;
		char*							Str2;

		CheckPtrExistence(Array);
		Limit = ArrayGetLength(Array);
		FirstScan = 0;
		while (FirstScan < Limit - 1)
			{
				SecondScan = FirstScan + 1;
				while (SecondScan < Limit)
					{
						Str1 = (char*)ArrayGetElement(Array,FirstScan);
						Str2 = (char*)ArrayGetElement(Array,SecondScan);
						if (PtrSize(Str1) == PtrSize(Str2))
							{
								if (MemEqu(Str1,Str2,PtrSize(Str1)))
									{
										return True; /* found a match */
									}
							}
						SecondScan += 1;
					}
				FirstScan += 1;
			}
		return False; /* couldn't find a match */
	}


/* this checks all of the objects to make sure that names are unique */
/* it returns True if they are, or returns False and presents a warning dialog */
/* if they aren't unique */
MyBoolean						CheckNameUniqueness(struct MainWindowRec* MainWindow)
	{
		SampleListRec*				SampleList;
		AlgoSampListRec*			AlgoSampList;
		WaveTableListRec*			WaveTableList;
		AlgoWaveTableListRec*	AlgoWaveTableList;
		InstrListRec*					InstrList;
		ArrayRec*							Array;
		long									Scan;
		long									Limit;
		char*									StringTemp;
		MyBoolean							Result;

		CheckPtrExistence(MainWindow);

		SampleList = MainWindowGetSampleList(MainWindow);
		CheckPtrExistence(SampleList);
		AlgoSampList = MainWindowGetAlgoSampList(MainWindow);
		CheckPtrExistence(AlgoSampList);

		WaveTableList = MainWindowGetWaveTableList(MainWindow);
		CheckPtrExistence(WaveTableList);
		AlgoWaveTableList = MainWindowGetAlgoWaveTableList(MainWindow);
		CheckPtrExistence(AlgoWaveTableList);

		InstrList = MainWindowGetInstrList(MainWindow);
		CheckPtrExistence(InstrList);

		Array = NewArray();
		if (Array == NIL)
			{
			 FailurePointA1:
				return True;
			}
		Limit = SampleListHowMany(SampleList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				StringTemp = SampleObjectGetNameCopy(SampleListGetIndexedSample(SampleList,Scan));
				if (StringTemp == NIL)
					{
					 FailurePointA2a:
						CleanUpStringArray(Array);
						goto FailurePointA1;
					}
				if (!ArrayAppendElement(Array,StringTemp))
					{
					 FailurePointA2b:
						ReleasePtr(StringTemp);
						goto FailurePointA2a;
					}
			}
		Limit = AlgoSampListHowMany(AlgoSampList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				StringTemp = AlgoSampObjectGetNameCopy(AlgoSampListGetIndexedAlgoSamp(
					AlgoSampList,Scan));
				if (StringTemp == NIL)
					{
					 FailurePointA3a:
						goto FailurePointA2a;
					}
				if (!ArrayAppendElement(Array,StringTemp))
					{
					 FailurePointA3b:
						ReleasePtr(StringTemp);
						goto FailurePointA3a;
					}
			}
		Result = TestArrays(Array);
		CleanUpStringArray(Array);
		if (Result)
			{
				AlertHalt("Some samples or algorithmic samples have the same name.",NIL);
				return False; /* found a match */
			}

		Array = NewArray();
		if (Array == NIL)
			{
			 FailurePointB1:
				return True;
			}
		Limit = WaveTableListHowMany(WaveTableList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				StringTemp = WaveTableObjectGetNameCopy(WaveTableListGetIndexedWaveTable(
					WaveTableList,Scan));
				if (StringTemp == NIL)
					{
					 FailurePointB2a:
						CleanUpStringArray(Array);
						goto FailurePointB1;
					}
				if (!ArrayAppendElement(Array,StringTemp))
					{
					 FailurePointB2b:
						ReleasePtr(StringTemp);
						goto FailurePointB2a;
					}
			}
		Limit = AlgoWaveTableListHowMany(AlgoWaveTableList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				StringTemp = AlgoWaveTableObjectGetNameCopy(
					AlgoWaveTableListGetIndexedAlgoWaveTable(AlgoWaveTableList,Scan));
				if (StringTemp == NIL)
					{
					 FailurePointB3a:
						goto FailurePointB2a;
					}
				if (!ArrayAppendElement(Array,StringTemp))
					{
					 FailurePointB3b:
						ReleasePtr(StringTemp);
						goto FailurePointB3a;
					}
			}
		Result = TestArrays(Array);
		CleanUpStringArray(Array);
		if (Result)
			{
				AlertHalt("Some wave tables or algorithmic wave tables have the same name.",NIL);
				return False; /* found a match */
			}

		Array = NewArray();
		if (Array == NIL)
			{
			 FailurePointC1:
				return True;
			}
		Limit = InstrListHowMany(InstrList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				StringTemp = InstrObjectGetNameCopy(InstrListGetIndexedInstr(
					InstrList,Scan));
				if (StringTemp == NIL)
					{
					 FailurePointC2a:
						CleanUpStringArray(Array);
						goto FailurePointC1;
					}
				if (!ArrayAppendElement(Array,StringTemp))
					{
					 FailurePointC2b:
						ReleasePtr(StringTemp);
						goto FailurePointC2a;
					}
			}
		Result = TestArrays(Array);
		CleanUpStringArray(Array);
		if (Result)
			{
				AlertHalt("Some instruments have the same name.",NIL);
				return False; /* found a match */
			}

		return True; /* everything's ok */
	}
