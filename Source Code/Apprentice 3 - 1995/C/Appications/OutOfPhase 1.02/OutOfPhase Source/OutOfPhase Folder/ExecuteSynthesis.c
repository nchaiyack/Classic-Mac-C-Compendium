/* ExecuteSynthesis.c */
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

#include "ExecuteSynthesis.h"
#include "Memory.h"
#include "MainWindowStuff.h"
#include "LinearTransition.h"
#include "PlayTrackInfoThang.h"
#include "Array.h"
#include "WaveTableOscControl.h"
#include "EnvelopeState.h"
#include "LFOGenerator.h"
#include "InstrList.h"
#include "InstrObject.h"
#include "TrackObject.h"
#include "SampleOscControl.h"
#include "ModulationOscControl.h"
#include "Fractions.h"
#include "FrameObject.h"
#include "DeterminedNoteStructure.h"
#include "TempoController.h"
#include "OscBankPlayer.h"
#include "DataMunging.h"
#include "Alert.h"
#include "CheckNameUniqueness.h"
#include "ErrorDaemon.h"


typedef struct PlayListNodeRec
	{
		struct PlayListNodeRec*	Next;
		PlayTrackInfoRec*				ThisTrack;
	} PlayListNodeRec;


/* build the list of objects involved in playing. */
static SynthErrorCodes	BuildPlayList(PlayListNodeRec** ListOut,
												ArrayRec* TrackObjectList, MyBoolean InStereoFlag,
												MainWindowRec* MainWindow,
												LargeBCDType OverallVolumeScalingReciprocal,
												long SamplingRate, long EnvelopeRate, MyBoolean TimeInterp,
												MyBoolean WaveInterp, TempoControlRec* TempoControl,
												long ScanningGapWidthInEnvelopeTicks,
												ErrorDaemonRec* ErrorDaemon)
	{
		PlayListNodeRec*		TrackPlayList;
		long								Scan;
		long								Limit;
		InstrListRec*				InstrList;
		SynthErrorCodes			Error;

		/* build list of tracks */
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(TrackObjectList);
		InstrList = MainWindowGetInstrList(MainWindow);
		TrackPlayList = NIL;
		Limit = ArrayGetLength(TrackObjectList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				TrackObjectRec*			PossibleTrack;
				PlayListNodeRec*		NewListNode;
				PlayTrackInfoRec*		TrackPlayObject;
				InstrObjectRec*			BaseInstrument;
				char*								InstrumentNameCopy;

				/* get the track */
				PossibleTrack = (TrackObjectRec*)ArrayGetElement(TrackObjectList,Scan);
				CheckPtrExistence(PossibleTrack);
				/* get the instrument to be used */
				InstrumentNameCopy = TrackObjectGetInstrName(PossibleTrack);
				if (InstrumentNameCopy == NIL)
					{
						Error = eSynthNoMemory;
						goto TrackListBuildFailure1;
					}
				BaseInstrument = InstrListLookupNamedInstr(InstrList,InstrumentNameCopy);
				ReleasePtr(InstrumentNameCopy);
				if (BaseInstrument == NIL)
					{
						char*								TrackName;
						MyBoolean						MessageSuccess = False;

						TrackName = TrackObjectGetNameCopy(PossibleTrack);
						if (TrackName != NIL)
							{
								char*								NullTerminated;

								NullTerminated = BlockToStringCopy(TrackName);
								if (NullTerminated != NIL)
									{
										AlertHalt("The track '_' uses a nonexistent instrument.",
											NullTerminated);
										MessageSuccess = True;
										ReleasePtr(NullTerminated);
									}
								ReleasePtr(TrackName);
							}
						if (!MessageSuccess)
							{
								AlertHalt("A track uses a nonexistent instrument.",NIL);
							}
						Error = eSynthUndefinedInstrumentError;
						goto TrackListBuildFailure1;
					}
				/* build the track */
				TrackPlayObject = NewPlayTrackInfo(PossibleTrack,
					GetInstrObjectRawData(BaseInstrument),InStereoFlag,
					OverallVolumeScalingReciprocal,SamplingRate,EnvelopeRate,TimeInterp,
					WaveInterp,TempoControl,ScanningGapWidthInEnvelopeTicks,ErrorDaemon);
				if (TrackPlayObject == NIL)
					{
						Error = eSynthNoMemory;
					 TrackListBuildFailure1:
						while (TrackPlayList != NIL)
							{
								PlayListNodeRec*		Temp;

								DisposePlayTrackInfo(TrackPlayList->ThisTrack);
								Temp = TrackPlayList;
								TrackPlayList = TrackPlayList->Next;
								ReleasePtr((char*)Temp);
							}
						return Error;
					}
				/* add this one to the list */
				NewListNode = (PlayListNodeRec*)AllocPtrCanFail(sizeof(PlayListNodeRec),
					"PlayListNodeRec");
				if (NewListNode == NIL)
					{
						Error = eSynthNoMemory;
					 TrackListBuildFailure2:
						DisposePlayTrackInfo(TrackPlayObject);
						goto TrackListBuildFailure1;
					}
				NewListNode->ThisTrack = TrackPlayObject;
				NewListNode->Next = TrackPlayList;
				TrackPlayList = NewListNode;
			}
		*ListOut = TrackPlayList;
		return eSynthDone;
	}


/* this routine scans through the key playlist and determines the exact point */
/* at which playback should begin. */
static void						FindStartPoint(TrackObjectRec* KeyTrack, long FrameToStartAt,
												FractionRec* StartTimeOut)
	{
		long								Scan;
		FractionRec					Counter;

		CheckPtrExistence(KeyTrack);
		Counter.Integer = 0;
		Counter.Fraction = 0;
		Counter.Denominator = (64*3*5*7*2);
		ERROR(FrameToStartAt > TrackObjectGetNumFrames(KeyTrack),PRERR(ForceAbort,
			"FindStartPoint:  start frame is beyond end of track"));
		for (Scan = 0; Scan < FrameToStartAt; Scan += 1)
			{
				FrameObjectRec*			Frame;
				FractionRec					TempDuration;

				Frame = TrackObjectGetFrame(KeyTrack,Scan);
				CheckPtrExistence(Frame);
				DurationOfFrame(Frame,&TempDuration);
				AddFractions(&TempDuration,&Counter,&Counter);
			}
		*StartTimeOut = Counter;
	}


/* This routine does all of the work. */
/* The DataOutCallback is called every time a block of data is */
/* ready to be sent to the target device; this is provided so that data can be */
/* redirected to a file or postprocessed in some way before playback. */
/* the KeyTrack and FrameToStartAt provide a reference point indicating where */
/* playback should occur.  if KeyTrack is NIL, then playback begins at the beginning. */
/* the rate parameters are in operations per second. */
SynthErrorCodes				Synthesizer(struct MainWindowRec* MainWindow,
												MyBoolean (*DataOutCallback)(void* Refcon,
													largefixedsigned* DataBlock, long NumFrames,
													MyBoolean* AbortPlaybackFlagOut),
												void* DataOutRefcon, struct ArrayRec* ListOfTracks,
												struct TrackObjectRec* KeyTrack, long FrameToStartAt,
												long SamplingRate, long EnvelopeRate, MyBoolean UseStereo,
												LargeBCDType DefaultBeatsPerMinute,
												LargeBCDType OverallVolumeScalingReciprocal,
												MyBoolean InterpOverTime, MyBoolean InterpAcrossWaves,
												LargeBCDType ScanningGap, ErrorDaemonRec* ErrorDaemon)
	{
		SynthErrorCodes			Error;
		largefixedsigned*		SampleArray;
		long								SampleArrayLength;


		/* error checking */
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(ListOfTracks);
		CheckPtrExistence(ErrorDaemon);
		ERROR(DataOutCallback == NIL,PRERR(ForceAbort,"Synthesizer: bad DataOutCallback"));

		/* check to see that there aren't any naming ambiguities */
		if (!CheckNameUniqueness(MainWindow))
			{
				return eSynthDuplicateNames;
			}

		/* make sure all objects are up to date */
		if (MainWindowMakeEverythingUpToDate(MainWindow))
			{
				PlayListNodeRec*		PlayTrackList;

				SampleArrayLength = 1000;
				SampleArray = (largefixedsigned*)AllocPtrCanFail(SampleArrayLength
					* sizeof(largefixedsigned),"SampleArray");
				if (SampleArray != NIL)
					{
						TempoControlRec*		TempoControl;
						long								ScanningGapWidthInEnvelopeTicks;

						/* figure out how large the scanning gap is */
						ScanningGapWidthInEnvelopeTicks = LargeBCD2Double(ScanningGap) * EnvelopeRate;

						TempoControl = NewTempoControl(DefaultBeatsPerMinute);
						if (TempoControl != NIL)
							{
								Error = BuildPlayList(&PlayTrackList,ListOfTracks,UseStereo,MainWindow,
									OverallVolumeScalingReciprocal,SamplingRate,EnvelopeRate,
									InterpOverTime,InterpAcrossWaves,TempoControl,
									ScanningGapWidthInEnvelopeTicks,ErrorDaemon);
								if (Error == eSynthDone)
									{
										PlayListNodeRec*		Scan;
										PlayListNodeRec*		Lag;
										MyBoolean						AbortPlaybackFlag;
										FractionRec					MomentOfStarting;
										LargeBCDType				CurrentBeatsPerMinute;
										long								ScanningGapFrontInEnvelopeTicks;
										double							EnvelopeClockAccumulatorFraction;
										double							NoteDurationClockAccumulatorFraction;
										double							SamplesPerEnvelopeClock;
										double							DurationTicksPerEnvelopeClock;

										/* calculate the moment of starting for tracks */
										FindStartPoint(KeyTrack,FrameToStartAt,&MomentOfStarting);
										/* cue all tracks up to this point */
										Scan = PlayTrackList;
										while (Scan != NIL)
											{
												if (!CuePlayTrackInfoToPoint(Scan->ThisTrack,&MomentOfStarting))
													{
														Error = eSynthNoMemory;
														goto PlaybackFailurePoint;
													}
												Scan = Scan->Next;
											}

										/* this value is for determining when in REAL time (not score time) */
										/* each note begins */
										ScanningGapFrontInEnvelopeTicks = 0;

										/* initialize tempo thing */
										CurrentBeatsPerMinute = DefaultBeatsPerMinute;

										/* initialize accumulators */
										EnvelopeClockAccumulatorFraction = 0;
										NoteDurationClockAccumulatorFraction = 0;
										/* calculate increment factors */
										SamplesPerEnvelopeClock = (double)SamplingRate / EnvelopeRate;
										DurationTicksPerEnvelopeClock = ((LargeBCD2Double(DefaultBeatsPerMinute)
											/ (4/*beats per whole note*/ * 60/*seconds per minute*/))
											/ EnvelopeRate) * DURATIONUPDATECLOCKRESOLUTION;

										/* play */
										AbortPlaybackFlag = False;
										while ((PlayTrackList != NIL) && !AbortPlaybackFlag)
											{
												long								NumSampleFrames;
												long								NumNoteDurationTicks;
												LargeBCDType				OldBeatsPerMinute;

												/* figure out how many note duration ticks to generate */
												/* increment counter */
												NoteDurationClockAccumulatorFraction += DurationTicksPerEnvelopeClock;
												/* round down */
												NumNoteDurationTicks = (long)NoteDurationClockAccumulatorFraction;
												/* subtract off what we're taking out this time around, */
												/* leaving the extra little bit in there */
												NoteDurationClockAccumulatorFraction -= NumNoteDurationTicks;

												/* figure out how many samples to do before the next envelope */
												if (ScanningGapFrontInEnvelopeTicks >= ScanningGapWidthInEnvelopeTicks)
													{
														/* we're really sampling stuff */
														EnvelopeClockAccumulatorFraction += SamplesPerEnvelopeClock;
														NumSampleFrames = (long)EnvelopeClockAccumulatorFraction;
														EnvelopeClockAccumulatorFraction -= NumSampleFrames;
													}
												 else
													{
														/* scanning gap is still opening, so we're not sampling */
														NumSampleFrames = 0;
													}

												/* see if we need to resize the output workspace array */
												if (UseStereo)
													{
														long								Scan;

														/* stereo array sizing */
														if (NumSampleFrames * 2 > SampleArrayLength)
															{
																largefixedsigned*		TempArray;

																TempArray = (largefixedsigned*)ResizePtr(
																	(char*)SampleArray,NumSampleFrames * 2
																	* sizeof(largefixedsigned));
																if (TempArray == NIL)
																	{
																		Error = eSynthNoMemory;
																		goto PlaybackFailurePoint;
																	}
																SampleArray = TempArray;
																SampleArrayLength = NumSampleFrames * 2;
															}
														/* initialize the array */
														for (Scan = 0; Scan < NumSampleFrames * 2; Scan += 1)
															{
																PRNGCHK(SampleArray,&(SampleArray[Scan]),
																	sizeof(SampleArray[Scan]));
																SampleArray[Scan] = 0;
															}
													}
												 else
													{
														long								Scan;

														/* mono array sizing */
														if (NumSampleFrames > SampleArrayLength)
															{
																largefixedsigned*		TempArray;

																TempArray = (largefixedsigned*)ResizePtr(
																	(char*)SampleArray,NumSampleFrames
																	* sizeof(largefixedsigned));
																if (TempArray == NIL)
																	{
																		Error = eSynthNoMemory;
																		goto PlaybackFailurePoint;
																	}
																SampleArray = TempArray;
																SampleArrayLength = NumSampleFrames;
															}
														/* initialize the array */
														for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
															{
																PRNGCHK(SampleArray,&(SampleArray[Scan]),
																	sizeof(SampleArray[Scan]));
																SampleArray[Scan] = 0;
															}
													}

												/* perform execution cyle */
												Scan = PlayTrackList;
												Lag = NIL;
												while (Scan != NIL)
													{
														if (!PlayTrackUpdate(Scan->ThisTrack,
															/* this condition is responsible for opening the scanning gap */
															(ScanningGapFrontInEnvelopeTicks >= ScanningGapWidthInEnvelopeTicks),
															NumNoteDurationTicks,NumSampleFrames,SampleArray,
															/* envelope ticks per duration tick */
															(float)(1 / DurationTicksPerEnvelopeClock),
															ScanningGapFrontInEnvelopeTicks))
															{
																Error = eSynthNoMemory;
																goto PlaybackFailurePoint;
															}
														if (PlayTrackIsItStillActive(Scan->ThisTrack))
															{
																Lag = Scan;
																Scan = Scan->Next;
															}
														 else
															{
																PlayListNodeRec*		Temp;

																if (Lag == NIL)
																	{
																		PlayTrackList = Scan->Next;
																	}
																 else
																	{
																		Lag->Next = Scan->Next;
																	}
																Temp = Scan;
																Scan = Scan->Next;
																DisposePlayTrackInfo(Temp->ThisTrack);
																ReleasePtr((char*)Temp);
															}
													}

												/* keep track of what time it is */
												ScanningGapFrontInEnvelopeTicks += 1;

												/* submit the data */
												AbortPlaybackFlag = False;
												if (!(*DataOutCallback)(DataOutRefcon,SampleArray,NumSampleFrames,
													&AbortPlaybackFlag)) /* DataOutRefcon knows about stereo status */
													{
														Error = eSynthDataSubmitError;
														goto PlaybackFailurePoint;
													}

												/* and also do the tempo generator */
												OldBeatsPerMinute = CurrentBeatsPerMinute;
												CurrentBeatsPerMinute = TempoControlUpdate(TempoControl,
													NumNoteDurationTicks);
												if (OldBeatsPerMinute != CurrentBeatsPerMinute)
													{
														/* tempo changed, so we have to adjust the clock */
														DurationTicksPerEnvelopeClock = ((LargeBCD2Double(CurrentBeatsPerMinute)
															/ (4/*beats per whole note*/ * 60/*seconds per minute*/))
															/ EnvelopeRate) * DURATIONUPDATECLOCKRESOLUTION;
													}
											}
										if (AbortPlaybackFlag)
											{
												Error = eSynthUserCancelled;
											}

									 PlaybackFailurePoint:
										;

										/* clean up */
										while (PlayTrackList != NIL)
											{
												PlayListNodeRec*		Temp;

												DisposePlayTrackInfo(PlayTrackList->ThisTrack);
												Temp = PlayTrackList;
												PlayTrackList = PlayTrackList->Next;
												ReleasePtr((char*)Temp);
											}
									}

								DisposeTempoControl(TempoControl);
							}

						/* clean up */
						ReleasePtr((char*)SampleArray);
					}
				 else
					{
						Error = eSynthNoMemory;
					}

			 SkipBuildLoop:
				;
			}
		 else
			{
				Error = eSynthPrereqError;
			}

		/* clean up */
		FlushLinearTransitionRecords();
		FlushWaveTableOscControl();
		FlushSampleOscControl();
		FlushEvalEnvelopeStateRecords();
		FlushLFOGeneratorRecords();
		FlushModOscControl();
		FlushPlayTrackInfo();
		FlushFrozenNoteStructures();
		FlushCachedOscStateBankRecords();

		return Error;
	}
