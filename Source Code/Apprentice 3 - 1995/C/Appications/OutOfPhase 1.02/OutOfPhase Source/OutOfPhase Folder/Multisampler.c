/* Multisampler.c */
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

#include "Multisampler.h"
#include "Memory.h"
#include "SampleSelector.h"
#include "SampleObject.h"
#include "AlgoSampObject.h"
#include "WaveTableObject.h"
#include "AlgoWaveTableObject.h"


typedef struct OneSampleRec
	{
		/* this is the data reference.  for wave tables, it is an array of waves, so */
		/* we have to dispose it.  for samples, it is the actual wave, so we don't */
		/* dispose it. */
		void*									DataReference;
		/* if this is a wave table, then this number contains the number of */
		/* frames per table */
		long									NumTables;
		long									FramesPerTable;
		/* if this is a sample, then this contains special information about it */
		NumBitsType						NumBits;
		NumChannelsType				NumChannelsForSample;
		long									NumSampleFrames;
		long									Loop1Start;
		long									Loop1End;
		long									Loop2Start;
		long									Loop2End;
		long									Loop3Start;
		long									Loop3End;
		long									Origin;
		double								NaturalFreq;
		long									SamplingRate;

		/* this is the lowest frequency that the sample should be played at */
		float									MinFrequency;
		/* this is the frequency just above the highest frequency that this */
		/* sample should be played at. */
		float									MaxFrequency;

		/* reference to the next one */
		struct OneSampleRec*	Next;
	} OneSampleRec;


struct MultiSampleRec
	{
		/* this is the list of samples */
		OneSampleRec*					SampleList;

		/* this flag is True if it was a wave table (i.e. dispose data references) or */
		/* False if it was a sample */
		MyBoolean							DisposeReferencesFlag;
	};


/* create a new multisampling object based on a list of wave tables */
/* the named references in the Selector should have been resolved, but it is */
/* the parser's job to do that. */
MultiSampleRec*				NewMultisampleWaveTable(struct SampleSelectorRec* Selector)
	{
		MultiSampleRec*			Multisample;
		OneSampleRec*				ListTail;
		long								Limit;
		long								Scan;

		CheckPtrExistence(Selector);

		Multisample = (MultiSampleRec*)AllocPtrCanFail(sizeof(MultiSampleRec),
			"MultiSampleRec:  wave table variant");
		if (Multisample == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Multisample->SampleList = NIL;
		Multisample->DisposeReferencesFlag = True; /* since it's a wave table one */

		ListTail = NIL;
		Limit = GetSampleSelectorListLength(Selector);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				OneSampleRec*				NewNode;

				NewNode = (OneSampleRec*)AllocPtrCanFail(sizeof(OneSampleRec),"OneSampleRec");
				if (NewNode == NIL)
					{
					 FailurePoint2:
						while (Multisample->SampleList != NIL)
							{
								NewNode = Multisample->SampleList;
								Multisample->SampleList = Multisample->SampleList->Next;
								ReleasePtr((char*)NewNode->DataReference);
								ReleasePtr((char*)NewNode);
							}
						ReleasePtr((char*)Multisample);
						goto FailurePoint1;
					}

				NewNode->MinFrequency = GetSampleListEntryLowFreqBound(Selector,Scan);
				NewNode->MaxFrequency = GetSampleListEntryHighFreqBound(Selector,Scan);

				switch (GetSampleListWaveType(Selector,Scan))
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"NewMultisampleWaveTable:  bad value from GetSampleListWaveType"));
							break;
						case eDataWaveTable:
							{
								WaveTableObjectRec*			Table;
								long										Index;
								long										NumTables;

								Table = (WaveTableObjectRec*)GetSampleListWaveReference(Selector,Scan);
								CheckPtrExistence(Table);
								NumTables = WaveTableObjectGetNumTables(Table);
								NewNode->DataReference = AllocPtrCanFail(sizeof(void*) * NumTables,
									"Multisample wave table list");
								if (NewNode->DataReference == NIL)
									{
									 FailurePoint2a:
										ReleasePtr((char*)NewNode);
										goto FailurePoint2;
									}
								for (Index = 0; Index < NumTables; Index += 1)
									{
										PRNGCHK(NewNode->DataReference,&(((void**)(NewNode->DataReference))[
											Index]),sizeof(((void**)(NewNode->DataReference))[Index]));
										((void**)(NewNode->DataReference))[Index]
											= WaveTableObjectGetRawSlice(Table,Index);
									}
								NewNode->FramesPerTable = WaveTableObjectEntriesPerTable(Table);
								NewNode->NumTables = NumTables;
								NewNode->NumBits = WaveTableObjectGetNumBits(Table);
							}
							break;
						case eAlgoWaveTable:
							{
								AlgoWaveTableObjectRec*	Table;
								long										Index;
								long										NumTables;

								Table = (AlgoWaveTableObjectRec*)GetSampleListWaveReference(
									Selector,Scan);
								CheckPtrExistence(Table);
								NumTables = AlgoWaveTableObjectGetNumTables(Table);
								NewNode->DataReference = AllocPtrCanFail(sizeof(void*) * NumTables,
									"Multisample wave table list");
								if (NewNode->DataReference == NIL)
									{
										goto FailurePoint2a;
									}
								for (Index = 0; Index < NumTables; Index += 1)
									{
										PRNGCHK(NewNode->DataReference,&(((void**)(NewNode->DataReference))[
											Index]),sizeof(((void**)(NewNode->DataReference))[Index]));
										((void**)(NewNode->DataReference))[Index]
											= AlgoWaveTableObjectGetRawSlice(Table,Index);
									}
								NewNode->FramesPerTable = AlgoWaveTableObjectGetNumFrames(Table);
								NewNode->NumTables = NumTables;
								NewNode->NumBits = AlgoWaveTableObjectGetNumBits(Table);
							}
							break;
					}

				NewNode->Next = NIL;
				if (ListTail != NIL)
					{
						ListTail->Next = NewNode;
					}
				 else
					{
						Multisample->SampleList = NewNode;
					}
				ListTail = NewNode;
			}

		return Multisample;
	}


/* create a new multisampling object based on a list of samples */
/* the named references in the Selector should have been resolved, but it is */
/* the parser's job to do that. */
MultiSampleRec*				NewMultisampleSample(struct SampleSelectorRec* Selector)
	{
		MultiSampleRec*			Multisample;
		OneSampleRec*				ListTail;
		long								Limit;
		long								Scan;

		CheckPtrExistence(Selector);

		Multisample = (MultiSampleRec*)AllocPtrCanFail(sizeof(MultiSampleRec),
			"MultiSampleRec:  sample variant");
		if (Multisample == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Multisample->SampleList = NIL;
		Multisample->DisposeReferencesFlag = False; /* since it's a sample one */

		ListTail = NIL;
		Limit = GetSampleSelectorListLength(Selector);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				OneSampleRec*				NewNode;

				NewNode = (OneSampleRec*)AllocPtrCanFail(sizeof(OneSampleRec),"OneSampleRec");
				if (NewNode == NIL)
					{
					 FailurePoint2:
						while (Multisample->SampleList != NIL)
							{
								NewNode = Multisample->SampleList;
								Multisample->SampleList = Multisample->SampleList->Next;
								ReleasePtr((char*)NewNode->DataReference);
								ReleasePtr((char*)NewNode);
							}
						ReleasePtr((char*)Multisample);
						goto FailurePoint1;
					}

				NewNode->MinFrequency = GetSampleListEntryLowFreqBound(Selector,Scan);
				NewNode->MaxFrequency = GetSampleListEntryHighFreqBound(Selector,Scan);

				switch (GetSampleListWaveType(Selector,Scan))
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"NewMultisampleSample:  bad value from GetSampleListWaveType"));
							break;
						case eDataSample:
							{
								SampleObjectRec*				Sample;

								Sample = (SampleObjectRec*)GetSampleListWaveReference(Selector,Scan);
								CheckPtrExistence(Sample);
								NewNode->DataReference = SampleObjectGetRawData(Sample);
								NewNode->NumBits = SampleObjectGetNumBits(Sample);
								NewNode->NumChannelsForSample = SampleObjectGetNumChannels(Sample);
								NewNode->NumSampleFrames = SampleObjectGetNumSampleFrames(Sample);
								NewNode->Loop1Start = SampleObjectGetLoopStart1(Sample);
								NewNode->Loop1End = SampleObjectGetLoopEnd1(Sample);
								NewNode->Loop2Start = SampleObjectGetLoopStart2(Sample);
								NewNode->Loop2End = SampleObjectGetLoopEnd2(Sample);
								NewNode->Loop3Start = SampleObjectGetLoopStart3(Sample);
								NewNode->Loop3End = SampleObjectGetLoopEnd3(Sample);
								NewNode->Origin = SampleObjectGetOrigin(Sample);
								NewNode->NaturalFreq = SampleObjectGetNaturalFrequency(Sample);
								NewNode->SamplingRate = SampleObjectGetSamplingRate(Sample);
							}
							break;
						case eAlgoSample:
							{
								AlgoSampObjectRec*			Sample;

								Sample = (AlgoSampObjectRec*)GetSampleListWaveReference(Selector,Scan);
								CheckPtrExistence(Sample);
								NewNode->DataReference = AlgoSampObjectGetRawData(Sample);
								NewNode->NumBits = AlgoSampObjectGetNumBits(Sample);
								NewNode->NumChannelsForSample = AlgoSampObjectGetNumChannels(Sample);
								NewNode->NumSampleFrames = AlgoSampObjetGetNumFrames(Sample);
								NewNode->Loop1Start = AlgoSampObjectGetLoopStart1(Sample);
								NewNode->Loop1End = AlgoSampObjectGetLoopEnd1(Sample);
								NewNode->Loop2Start = AlgoSampObjectGetLoopStart2(Sample);
								NewNode->Loop2End = AlgoSampObjectGetLoopEnd2(Sample);
								NewNode->Loop3Start = AlgoSampObjectGetLoopStart3(Sample);
								NewNode->Loop3End = AlgoSampObjectGetLoopEnd3(Sample);
								NewNode->Origin = AlgoSampObjectGetOrigin(Sample);
								NewNode->NaturalFreq = AlgoSampObjectGetNaturalFrequency(Sample);
								NewNode->SamplingRate = AlgoSampObjectGetSamplingRate(Sample);
							}
							break;
					}

				NewNode->Next = NIL;
				if (ListTail != NIL)
					{
						ListTail->Next = NewNode;
					}
				 else
					{
						Multisample->SampleList = NewNode;
					}
				ListTail = NewNode;
			}

		return Multisample;
	}


/* dispose of a multisampling object */
void									DisposeMultisample(MultiSampleRec* Multisample)
	{
		CheckPtrExistence(Multisample);
		while (Multisample->SampleList != NIL)
			{
				OneSampleRec*				Temp;

				Temp = Multisample->SampleList;
				Multisample->SampleList = Multisample->SampleList->Next;
				if (Multisample->DisposeReferencesFlag)
					{
						ReleasePtr((char*)Temp->DataReference);
					}
				ReleasePtr((char*)Temp);
			}
		ReleasePtr((char*)Multisample);
	}


/* obtain a data reference & info for a wave table.  returns False if there is */
/* no wave table corresponding to the supplied frequency */
MyBoolean							GetMultisampleReferenceWaveTable(MultiSampleRec* Multisample,
												float FrequencyHertz, void*** TwoDimensionalVecOut,
												long* NumFramesOut, long* NumTablesOut,
												NumBitsType* NumBitsOut)
	{
		OneSampleRec*				Scan;

		CheckPtrExistence(Multisample);
		ERROR(!Multisample->DisposeReferencesFlag,PRERR(ForceAbort,
			"GetMultisampleReferenceWaveTable:  list is for samples, not wave tables"));
		Scan = Multisample->SampleList;
		while (Scan != NIL)
			{
				if ((FrequencyHertz >= Scan->MinFrequency)
					&& (FrequencyHertz < Scan->MaxFrequency))
					{
						*TwoDimensionalVecOut = (void**)(Scan->DataReference);
						*NumFramesOut = Scan->FramesPerTable;
						*NumTablesOut = Scan->NumTables;
						*NumBitsOut = Scan->NumBits;
						return True;
					}
				Scan = Scan->Next;
			}
		return False;
	}


/* obtain a data reference & info for a sample.  returns False if there is no */
/* sample corresponding to the supplied frequency. */
MyBoolean							GetMultisampleReferenceSample(MultiSampleRec* Multisample,
												float FrequencyHertz, void** DataOut, long* NumFramesOut,
												NumBitsType* NumBitsOut, NumChannelsType* NumChannelsOut,
												long* Loop1StartOut, long* Loop1EndOut, long* Loop2StartOut,
												long* Loop2EndOut, long* Loop3StartOut, long* Loop3EndOut,
												long* OriginOut, double* NaturalFreqOut, long* SamplingRateOut)
	{
		OneSampleRec*				Scan;

		CheckPtrExistence(Multisample);
		ERROR(Multisample->DisposeReferencesFlag,PRERR(ForceAbort,
			"GetMultisampleReferenceSample:  list is for wave tables, not samples"));
		Scan = Multisample->SampleList;
		while (Scan != NIL)
			{
				if ((FrequencyHertz >= Scan->MinFrequency)
					&& (FrequencyHertz < Scan->MaxFrequency))
					{
						*DataOut = Scan->DataReference;
						*NumFramesOut = Scan->NumSampleFrames;
						*NumBitsOut = Scan->NumBits;
						*NumChannelsOut = Scan->NumChannelsForSample;
						*Loop1StartOut = Scan->Loop1Start;
						*Loop1EndOut = Scan->Loop1End;
						*Loop2StartOut = Scan->Loop2Start;
						*Loop2EndOut = Scan->Loop2End;
						*Loop3StartOut = Scan->Loop3Start;
						*Loop3EndOut = Scan->Loop3End;
						*OriginOut = Scan->Origin;
						*NaturalFreqOut = Scan->NaturalFreq;
						*SamplingRateOut = Scan->SamplingRate;
						return True;
					}
				Scan = Scan->Next;
			}
		return False;
	}
