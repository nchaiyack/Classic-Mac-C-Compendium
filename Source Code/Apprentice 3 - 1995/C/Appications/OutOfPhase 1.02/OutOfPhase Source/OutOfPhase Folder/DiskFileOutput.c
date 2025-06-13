/* DiskFileOutput.c */
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

#include "DiskFileOutput.h"
#include "Files.h"
#include "Memory.h"
#include "Alert.h"
#include "ExecuteSynthesis.h"
#include "ClipWarnDialog.h"
#include "SynthProgressWindow.h"
#include "BufferedFileOutput.h"
#include "ErrorDaemon.h"


#define CANCELCHECKTIMER (50)


typedef struct
	{
		MyBoolean							UseStereo;
		long									SamplingRate;
		OutputNumBitsType			NumBitsOut;
		FileType*							OutputFile;

		long									TotalSampleCount;
		long									ClippedSampleCount;
		largefixedsigned			MaxClipExtent;

		long									CancelCheck;

		SynthWinRec*					Window;
	} StateRecord;


/* AIFF/AIFF-C File Format: */
/*     "FORM" */
/*     4-byte big endian form chunk length descriptor (minus 8 for "FORM" & this) */
/*     4-byte type */
/*        "AIFF" = AIFF format file */
/*        "AIFC" = AIFF-C format file */
/* in any order, these chunks can occur: */
/*   Version Chunk (this only occurs in AIFF-C files) */
/*     "FVER" */
/*     4-byte big endian length, which should always be the value 4 (four) */
/*     4-byte date code.  this is probably 0xA2805140 (stored big endian), but it */
/*          probably doesn't matter. */
/*   Common Chunk for AIFF files */
/*     "COMM" */
/*     4-byte big endian length. */
/*        always 18 for AIFF files */
/*     2-byte big endian number of channels */
/*     4-byte big endian number of sample frames */
/*     2-byte big endian number of bits per sample */
/*        a value in the domain 1..32 */
/*     10-byte extended precision number of frames per second */
/*   Common Chunk for AIFF-C files */
/*     "COMM" */
/*     4-byte big endian length. */
/*        22 + compression method string length for AIFF-C files */
/*     2-byte big endian number of channels */
/*     4-byte big endian number of sample frames */
/*     2-byte big endian number of bits per sample */
/*        a value in the domain 1..32 */
/*     10-byte extended precision number of frames per second */
/*     4-byte character code ID for the compression method */
/*        "NONE" means there is no compression method used */
/*     some characters in a string identifying the compression method */
/*        this must be padded to an even number of bytes, but the pad is */
/*        NOT included in the length descriptor for the chunk. */
/*        for uncompressed data, the string should be */
/*        "\x0enot compressed\x00", including the null, for 16 bytes. */
/*        the total chunk length is thus 38 bytes. */
/*   Sound Data Chunk */
/*     "SSND" */
/*     4-byte big endian number of bytes in sample data array */
/*     4-byte big endian offset to the first byte of sample data in the array */
/*     4-byte big endian number of bytes to which the sound data is aligned. */
/*     any length vector of raw sound data. */
/*        this must be padded to an even number of bytes, but the pad is */
/*        NOT included in the length descriptor for the chunk. */
/*        Samples are stored in an integral number of bytes, the smallest that */
/*        is required for the specified number of bits.  If this is not an even */
/*        multiple of 8, then the data is shifted left and the low bits are zeroed */
/*        Multichannel sound is interleaved with the left channel first. */
static MyBoolean			SetUpAIFFHeader(FileType* OutputFile, StateRecord* Info)
	{
		BufferedOutputRec*	Output;
		unsigned long				Mantissa;
		unsigned long				Exponent;
		char								StupidExtendedThing[10];

		Output = NewBufferedOutput(OutputFile);
		if (Output == NIL)
			{
			 FailurePoint1:
				return False;
			}

		/* 0..3 */
		/*     "FORM" */
		if (!WriteBufferedOutput(Output,4,"FORM"))
			{
			 FailurePoint2:
				EndBufferedOutput(Output);
				goto FailurePoint1;
			}

		/* 4..7 */
		/*     4-byte big endian form chunk length descriptor (minus 8 for "FORM" & this) */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0)) /* RESOLVED LATER */
			{
				goto FailurePoint2;
			}

		/* 8..11 */
		/*     4-byte type */
		/*        "AIFC" = AIFF-C format file */
		if (!WriteBufferedOutput(Output,4,"AIFC"))
			{
				goto FailurePoint2;
			}

		/* 12..15 */
		/*     "FVER" */
		if (!WriteBufferedOutput(Output,4,"FVER"))
			{
				goto FailurePoint2;
			}

		/* 16..19 */
		/*     4-byte big endian length, which should always be the value 4 (four) */
		if (!WriteBufferedUnsignedLongBigEndian(Output,4))
			{
				goto FailurePoint2;
			}

		/* 20..23 */
		/*     4-byte date code.  this is probably 0xA2805140 (stored big endian), but it */
		/*          probably doesn't matter. */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0xA2805140))
			{
				goto FailurePoint2;
			}

		/* 24..27 */
		/*     "COMM" */
		if (!WriteBufferedOutput(Output,4,"COMM"))
			{
				goto FailurePoint2;
			}

		/* 28..31 */
		/*     4-byte big endian length. */
		/*        22 + compression method string length for AIFF-C files */
		if (!WriteBufferedUnsignedLongBigEndian(Output,38))
			{
				goto FailurePoint2;
			}

		/* 32..33 */
		/*     2-byte big endian number of channels */
		if (Info->UseStereo)
			{
				if (!WriteBufferedUnsignedShortBigEndian(Output,2))
					{
						goto FailurePoint2;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedShortBigEndian(Output,1))
					{
						goto FailurePoint2;
					}
			}

		/* 34..37 */
		/*     4-byte big endian number of sample frames */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0)) /* RESOLVED LATER */
			{
				goto FailurePoint2;
			}

		/* 38..39 */
		/*     2-byte big endian number of bits per sample */
		/*        a value in the domain 1..32 */
		switch (Info->NumBitsOut)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"SetUpAIFFHeader:  bad number of bits"));
					break;
				case eOutput8Bits:
					if (!WriteBufferedUnsignedShortBigEndian(Output,8))
						{
							goto FailurePoint2;
						}
					break;
				case eOutput16Bits:
					if (!WriteBufferedUnsignedShortBigEndian(Output,16))
						{
							goto FailurePoint2;
						}
					break;
				case eOutput24Bits:
					if (!WriteBufferedUnsignedShortBigEndian(Output,24))
						{
							goto FailurePoint2;
						}
					break;
				case eOutput32Bits:
					if (!WriteBufferedUnsignedShortBigEndian(Output,32))
						{
							goto FailurePoint2;
						}
					break;
			}

		/* 40..49 */
		/*     10-byte extended precision number of frames per second */
		/* extended 22050 = 400D AC44000000000000 */
		/* extended 22051 = 400D AC46000000000000 */
		/* extended 44100 = 400E AC44000000000000 */
		/* extended 44101 = 400E AC45000000000000 */
		Exponent = 0x401e;
		Mantissa = Info->SamplingRate;
		while ((Mantissa & 0x80000000) == 0)
			{
				Mantissa = Mantissa << 1;
				Exponent -= 1;
			}
		StupidExtendedThing[0] = (Exponent >> 8) & 0xff;
		StupidExtendedThing[1] = Exponent & 0xff;
		StupidExtendedThing[2] = (Mantissa >> 24) & 0xff;
		StupidExtendedThing[3] = (Mantissa >> 16) & 0xff;
		StupidExtendedThing[4] = (Mantissa >> 8) & 0xff;
		StupidExtendedThing[5] = Mantissa & 0xff;
		StupidExtendedThing[6] = 0;
		StupidExtendedThing[7] = 0;
		StupidExtendedThing[8] = 0;
		StupidExtendedThing[9] = 0;
		if (!WriteBufferedOutput(Output,10,StupidExtendedThing))
			{
				goto FailurePoint2;
			}

		/* 50..53 */
		/*     4-byte character code ID for the compression method */
		/*        "NONE" means there is no compression method used */
		if (!WriteBufferedOutput(Output,4,"NONE"))
			{
				goto FailurePoint2;
			}

		/* 54..69 */
		/*     some characters in a string identifying the compression method */
		/*        this must be padded to an even number of bytes, but the pad is */
		/*        NOT included in the length descriptor for the chunk. */
		/*        for uncompressed data, the string should be */
		/*        "\x0enot compressed\x00", including the null, for 16 bytes. */
		/*        the total chunk length is thus 38 bytes. */
		if (!WriteBufferedOutput(Output,16,"\x0enot compressed\x00"))
			{
				goto FailurePoint2;
			}

		/* 70..73 */
		/*     "SSND" */
		if (!WriteBufferedOutput(Output,4,"SSND"))
			{
				goto FailurePoint2;
			}

		/* 74..77 */
		/*     4-byte big endian number of bytes in sample data array */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0)) /* RESOLVED LATER */
			{
				goto FailurePoint2;
			}

		/* 78..81 */
		/*     4-byte big endian offset to the first byte of sample data in the array */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0))
			{
				goto FailurePoint2;
			}

		/* 82..85 */
		/*     4-byte big endian number of bytes to which the sound data is aligned. */
		if (!WriteBufferedUnsignedLongBigEndian(Output,0))
			{
				goto FailurePoint2;
			}

		/*     any length vector of raw sound data. */
		/*        this must be padded to an even number of bytes, but the pad is */
		/*        NOT included in the length descriptor for the chunk. */
		/*        Samples are stored in an integral number of bytes, the smallest that */
		/*        is required for the specified number of bits.  If this is not an even */
		/*        multiple of 8, then the data is shifted left and the low bits are zeroed */
		/*        Multichannel sound is interleaved with the left channel first. */

		if (!EndBufferedOutput(Output))
			{
				return False;
			}

		return True;
	}


/* update various size fields in the file */
static MyBoolean			ResolveAIFFHeader(FileType* OutputFile, StateRecord* Info)
	{
		long								TotalBytesOfSamples;
		MyBoolean						FileSuccess;
		char								Buffer[4];

		TotalBytesOfSamples = Info->TotalSampleCount;
		if (Info->UseStereo)
			{
				TotalBytesOfSamples *= 2;
			}
		switch (Info->NumBitsOut)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"ResolveAIFFHeader:  bad number of bits"));
					break;
				case eOutput8Bits:
					break;
				case eOutput16Bits:
					TotalBytesOfSamples *= 2;
					break;
				case eOutput24Bits:
					TotalBytesOfSamples *= 3;
					break;
				case eOutput32Bits:
					TotalBytesOfSamples *= 4;
					break;
			}

		FileSuccess = True;

		/* make sure file is an even number of bytes */
		if ((GetFilePosition(OutputFile) & 1) != 0)
			{
				char						Stupid[1] = {0};

				if (0 != WriteToFile(OutputFile,Stupid,1))
					{
						FileSuccess = False;
					}
			}

		/* chop off any crud from the end */
		FileSuccess = SetFileLength(OutputFile,GetFilePosition(OutputFile)) && FileSuccess;

		/* 4..7 */
		/*     4-byte big endian form chunk length descriptor (minus 8 for "FORM" & this) */
		if (SetFilePosition(OutputFile,4))
			{
				Buffer[0] = ((TotalBytesOfSamples + 78 - 8) >> 24) & 0xff;
				Buffer[1] = ((TotalBytesOfSamples + 78 - 8) >> 16) & 0xff;
				Buffer[2] = ((TotalBytesOfSamples + 78 - 8) >> 8) & 0xff;
				Buffer[3] = (TotalBytesOfSamples + 78 - 8) & 0xff;
				if (0 != WriteToFile(OutputFile,Buffer,4))
					{
						FileSuccess = False;
					}
			}
		 else
			{
				FileSuccess = False;
			}

		/* 34..37 */
		/*     4-byte big endian number of sample frames */
		if (SetFilePosition(OutputFile,34))
			{
				Buffer[0] = (Info->TotalSampleCount >> 24) & 0xff;
				Buffer[1] = (Info->TotalSampleCount >> 16) & 0xff;
				Buffer[2] = (Info->TotalSampleCount >> 8) & 0xff;
				Buffer[3] = Info->TotalSampleCount & 0xff;
				if (0 != WriteToFile(OutputFile,Buffer,4))
					{
						FileSuccess = False;
					}
			}
		 else
			{
				FileSuccess = False;
			}

		/* 74..77 */
		/*     4-byte big endian number of bytes in sample data array */
		if (SetFilePosition(OutputFile,74))
			{
				Buffer[0] = (TotalBytesOfSamples >> 24) & 0xff;
				Buffer[1] = (TotalBytesOfSamples >> 16) & 0xff;
				Buffer[2] = (TotalBytesOfSamples >> 8) & 0xff;
				Buffer[3] = TotalBytesOfSamples & 0xff;
				if (0 != WriteToFile(OutputFile,Buffer,4))
					{
						FileSuccess = False;
					}
			}
		 else
			{
				FileSuccess = False;
			}

		return FileSuccess;
	}


static MyBoolean			CallbackRoutine(StateRecord* Info, largefixedsigned* DataBlock,
												long NumFrames, MyBoolean* AbortPlaybackFlagOut)
	{
		long								Limit;
		void*								Buffer;
		long								BufferSizeInBytes;
		char*								DataOutThing;
		long								DataOutInBytes;
		long								Scan;
		largefixedsigned		TempValue;
		long								UnwritableBytes;

		Info->CancelCheck -= 1;
		if (Info->CancelCheck < 0)
			{
				Info->CancelCheck = CANCELCHECKTIMER;
				if (RelinquishCPUJudiciouslyCheckCancel())
					{
						switch (AskYesNoCancel("Are you sure you want to abort synthesis?",NIL,
							"Abort","Resume",NIL))
							{
								default:
									EXECUTE(PRERR(ForceAbort,"DiskFileOutput:CallbackRoutine:  bad value "
										"from AskYesNoCancel"));
									break;
								case eYes:
									*AbortPlaybackFlagOut = True;
									return True;
								case eNo:
									break;
							}
						/* at first glance, we might need to somehow reset the RelinquishCPU */
						/* sticky cancel flag, but AskYesNoCancel calls the event loop, so */
						/* it gets automatically reset. */
					}
			}

		/* calculate required buffer size */
		Limit = NumFrames;
		BufferSizeInBytes = NumFrames;
		DataOutInBytes = NumFrames;
		if (Info->UseStereo)
			{
				Limit *= 2; /* twice as many frames for stereo */
				BufferSizeInBytes *= 2;
				DataOutInBytes *= 2;
			}
		switch (Info->NumBitsOut)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CallbackRoutine:  bad number of bits"));
					break;
				case eOutput8Bits:
					break;
				case eOutput16Bits:
					BufferSizeInBytes *= sizeof(short);
					DataOutInBytes *= 2;
					break;
				case eOutput24Bits:
					BufferSizeInBytes *= sizeof(largefixedsigned);
					DataOutInBytes *= 3;
					break;
				case eOutput32Bits:
					BufferSizeInBytes *= sizeof(largefixedsigned);
					DataOutInBytes *= 4;
					break;
			}
		Buffer = AllocPtrCanFail(BufferSizeInBytes,"AudioFileOutputBuffer");
		if (Buffer == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to continue synthesis.",NIL);
				return False;
			}
		DataOutThing = AllocPtrCanFail(DataOutInBytes,"AudioFileOutputBuffer2");
		if (DataOutThing == NIL)
			{
			 FailurePoint2:
				ReleasePtr(DataOutThing);
				goto FailurePoint1;
			}

		/* generate the data into the buffer */
		switch (Info->NumBitsOut)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"SynthToAIFFFile:  bad number of bits"));
					break;
				case eOutput8Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							PRNGCHK(DataBlock,&(DataBlock[Scan]),sizeof(DataBlock[Scan]));
							TempValue = DataBlock[Scan] + double2largefixed((double)1 / 65536);
							if (TempValue > int2largefixed(1) - 1)
								{
									Info->ClippedSampleCount += 1;
									if (TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = TempValue;
										}
									TempValue = int2largefixed(1) - 1;
								}
							else if (TempValue < - (int2largefixed(1) - 1))
								{
									Info->ClippedSampleCount += 1;
									if (- TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = - TempValue;
										}
									TempValue = - (int2largefixed(1) - 1);
								}
							PRNGCHK(Buffer,&(((signed char*)Buffer)[Scan]),
								sizeof(((signed char*)Buffer)[Scan]));
							((signed char*)Buffer)[Scan] = TempValue >> (largefixed_precision - 8 + 1);
						}
					break;
				case eOutput16Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							PRNGCHK(DataBlock,&(DataBlock[Scan]),sizeof(DataBlock[Scan]));
							TempValue = DataBlock[Scan] + double2largefixed((double)1 / 65536);
							if (TempValue > int2largefixed(1) - 1)
								{
									Info->ClippedSampleCount += 1;
									if (TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = TempValue;
										}
									TempValue = int2largefixed(1) - 1;
								}
							else if (TempValue < - (int2largefixed(1) - 1))
								{
									Info->ClippedSampleCount += 1;
									if (- TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = - TempValue;
										}
									TempValue = - (int2largefixed(1) - 1);
								}
							PRNGCHK(Buffer,&(((signed short*)Buffer)[Scan]),
								sizeof(((signed short*)Buffer)[Scan]));
							((signed short*)Buffer)[Scan] = TempValue >> (largefixed_precision - 16 + 1);
						}
					break;
				case eOutput24Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							PRNGCHK(DataBlock,&(DataBlock[Scan]),sizeof(DataBlock[Scan]));
							TempValue = DataBlock[Scan];
							if (TempValue > int2largefixed(1) - 1)
								{
									Info->ClippedSampleCount += 1;
									if (TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = TempValue;
										}
									TempValue = int2largefixed(1) - 1;
								}
							else if (TempValue < - (int2largefixed(1) - 1))
								{
									Info->ClippedSampleCount += 1;
									if (- TempValue > Info->MaxClipExtent)
										{
											Info->MaxClipExtent = - TempValue;
										}
									TempValue = - (int2largefixed(1) - 1);
								}
							PRNGCHK(Buffer,&(((signed short*)Buffer)[Scan]),
								sizeof(((signed short*)Buffer)[Scan]));
							((largefixedsigned*)Buffer)[Scan] = TempValue >> (largefixed_precision - 24 + 1);
						}
					break;
				case eOutput32Bits:
					/* 32-bits doesn't clip. */
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							PRNGCHK(DataBlock,&(DataBlock[Scan]),sizeof(DataBlock[Scan]));
							TempValue = DataBlock[Scan];
							PRNGCHK(Buffer,&(((signed short*)Buffer)[Scan]),
								sizeof(((signed short*)Buffer)[Scan]));
							((largefixedsigned*)Buffer)[Scan] = TempValue;
						}
					break;
			}

		/* reorder the data to be big-endian */
		switch (Info->NumBitsOut)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"SynthToAIFFFile:  bad number of bits"));
					break;
				case eOutput8Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							signed short				TempVal;

							PRNGCHK(Buffer,&(((signed char*)Buffer)[Scan]),
								sizeof(((signed char*)Buffer)[Scan]));
							TempVal = ((signed char*)Buffer)[Scan];
							PRNGCHK(DataOutThing,&(DataOutThing[Scan]),sizeof(DataOutThing[Scan]));
							DataOutThing[Scan] = TempVal;
						}
					break;
				case eOutput16Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							signed short				TempVal;

							PRNGCHK(Buffer,&(((signed short*)Buffer)[Scan]),
								sizeof(((signed short*)Buffer)[Scan]));
							TempVal = ((signed short*)Buffer)[Scan];
							PRNGCHK(DataOutThing,&(DataOutThing[2 * Scan + 0]),
								sizeof(DataOutThing[2 * Scan + 0]) * 2);
							DataOutThing[2 * Scan + 1] = TempVal & 0xff;
							DataOutThing[2 * Scan + 0] = (TempVal >> 8) & 0xff;
						}
					break;
				case eOutput24Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							largefixedsigned		TempVal;

							PRNGCHK(Buffer,&(((largefixedsigned*)Buffer)[Scan]),
								sizeof(((largefixedsigned*)Buffer)[Scan]));
							TempVal = ((largefixedsigned*)Buffer)[Scan];
							PRNGCHK(DataOutThing,&(DataOutThing[3 * Scan + 0]),
								sizeof(DataOutThing[3 * Scan + 0]) * 3);
							DataOutThing[3 * Scan + 2] = TempVal & 0xff;
							DataOutThing[3 * Scan + 1] = (TempVal >> 8) & 0xff;
							DataOutThing[3 * Scan + 0] = (TempVal >> 16) & 0xff;
						}
					break;
				case eOutput32Bits:
					for (Scan = 0; Scan < Limit; Scan += 1)
						{
							largefixedsigned		TempVal;

							PRNGCHK(Buffer,&(((largefixedsigned*)Buffer)[Scan]),
								sizeof(((largefixedsigned*)Buffer)[Scan]));
							TempVal = ((largefixedsigned*)Buffer)[Scan];
							PRNGCHK(DataOutThing,&(DataOutThing[4 * Scan + 0]),
								sizeof(DataOutThing[4 * Scan + 0]) * 4);
							DataOutThing[4 * Scan + 3] = TempVal & 0xff;
							DataOutThing[4 * Scan + 2] = (TempVal >> 8) & 0xff;
							DataOutThing[4 * Scan + 1] = (TempVal >> 16) & 0xff;
							DataOutThing[4 * Scan + 0] = (TempVal >> 24) & 0xff;
						}
					break;
			}

		/* write data to the file */
	 RetryPoint:
		UnwritableBytes = WriteToFile(Info->OutputFile,DataOutThing,DataOutInBytes);
		if (UnwritableBytes != 0)
			{
				switch (AskYesNoCancel("An error occurred writing to the file.  The disk "
					"may be full.  Do you want to try writing again?",NIL,"Retry","Abort",NIL))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"SynthToAIFFFile:  bad value from AskYesNoCancel"));
							break;
						case eYes:
							/* back up */
							if (!SetFilePosition(Info->OutputFile,GetFilePosition(Info->OutputFile)
								- (UnwritableBytes - DataOutInBytes)))
								{
									AlertHalt("File position could not be reset.  Aborting synthesis.",NIL);
								 FailurePoint3:
									ReleasePtr(DataOutThing);
									goto FailurePoint2;
									return False;
								}
							goto RetryPoint;
						case eNo:
							/* clean up */
							SetFilePosition(Info->OutputFile,GetFilePosition(Info->OutputFile)
								- (UnwritableBytes - DataOutInBytes));
							goto FailurePoint3;
					}
			}

		Info->TotalSampleCount += NumFrames;

		UpdateSynthWindow(Info->Window,Info->SamplingRate,Info->TotalSampleCount,
			Info->ClippedSampleCount,False);

		ReleasePtr((char*)Buffer);
		ReleasePtr(DataOutThing);

		return True;
	}


/* this routine opens a file and dumps the data to it. */
void									SynthToAIFFFile(struct MainWindowRec* MainWindow,
												struct ArrayRec* ListOfTracks, struct TrackObjectRec* KeyTrack,
												long FrameToStartAt, long SamplingRate, long EnvelopeRate,
												MyBoolean UseStereo, LargeBCDType DefaultBeatsPerMinute,
												LargeBCDType OverallVolumeScalingReciprocal,
												MyBoolean InterpOverTime, MyBoolean InterpAcrossWaves,
												LargeBCDType ScanningGap, OutputNumBitsType NumBitsOut,
												MyBoolean ClipWarn)
	{
		StateRecord					StateInfo;
		SynthErrorCodes			SynthErrorReturnCode;
		FileSpec*						WhereToSaveFile;
		ErrorDaemonRec*			ErrorDaemon;

		CheckPtrExistence(MainWindow);
		CheckPtrExistence(ListOfTracks);
		CheckPtrExistence(KeyTrack);

		StateInfo.UseStereo = UseStereo;
		StateInfo.SamplingRate = SamplingRate;
		StateInfo.NumBitsOut = NumBitsOut;

		StateInfo.TotalSampleCount = 0;
		StateInfo.ClippedSampleCount = 0;
		StateInfo.MaxClipExtent = 0;

		StateInfo.CancelCheck = 0;

		StateInfo.Window = NewSynthWindow(EnvelopeRate / 2,True/*show clipping*/);
		if (StateInfo.Window == NIL)
			{
				AlertHalt("There is not enough memory available to perform synthesis.",NIL);
			 SetupFailurePoint1:
				return;
			}

		WhereToSaveFile = PutFile("AIFF Output File");
		if (WhereToSaveFile == NIL)
			{
			 SetupFailurePoint2:
				DisposeSynthWindow(StateInfo.Window);
				goto SetupFailurePoint1;
			}

		if (!CreateFile(WhereToSaveFile,ApplicationCreator,CODE4BYTES('A','I','F','F')))
			{
				AlertHalt("Unable to create the file.",NIL);
			 SetupFailurePoint3:
				DisposeFileSpec(WhereToSaveFile);
				goto SetupFailurePoint2;
			}

		if (!OpenFile(WhereToSaveFile,&(StateInfo.OutputFile),eReadAndWrite))
			{
				AlertHalt("Unable to open the file for writing.",NIL);
			 SetupFailurePoint4:
				goto SetupFailurePoint3;
			}

		if (!SetUpAIFFHeader(StateInfo.OutputFile,&StateInfo))
			{
				AlertHalt("Unable to write data to the file.",NIL);
			 SetupFailurePoint5:
				CloseFile(StateInfo.OutputFile);
				goto SetupFailurePoint4;
			}

		ErrorDaemon = NewErrorDaemon();
		if (ErrorDaemon == NIL)
			{
				AlertHalt("There is not enough memory available to perform synthesis.",NIL);
			 SetupFailurePoint6:
				goto SetupFailurePoint5;
			}

		SynthErrorReturnCode = Synthesizer(MainWindow,
			(MyBoolean (*)(void*,largefixedsigned*,long,MyBoolean*))&CallbackRoutine,
			&StateInfo,ListOfTracks,KeyTrack,FrameToStartAt,SamplingRate,EnvelopeRate,
			UseStereo,DefaultBeatsPerMinute,OverallVolumeScalingReciprocal,InterpOverTime,
			InterpAcrossWaves,ScanningGap,ErrorDaemon);

		UpdateSynthWindow(StateInfo.Window,StateInfo.SamplingRate,StateInfo.TotalSampleCount,
			StateInfo.ClippedSampleCount,True);

		if (!ResolveAIFFHeader(StateInfo.OutputFile,&StateInfo))
			{
				AlertHalt("Unable to properly finalize the data in the file.  It will "
					"probably be unusable without manual repair.",NIL);
			}

		switch (SynthErrorReturnCode)
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"SynthToSoundDevice:  bad return code from Synthesizer()"));
					break;
				case eSynthDone:
					break;
				case eSynthNoMemory:
					AlertHalt("There is not enough memory available to continue synthesis.",NIL);
					break;
				case eSynthUserCancelled:
					break;
				/* case eSynthProgramError: */
					break;
				case eSynthPrereqError:
					break;
				case eSynthUndefinedInstrumentError:
					break;
				case eSynthDataSubmitError:
					break;
				case eSynthDuplicateNames:
					break;
			}

		CloseFile(StateInfo.OutputFile);
		DisposeFileSpec(WhereToSaveFile);
		DisposeSynthWindow(StateInfo.Window);

		if (ErrorDaemonDidClampingOccur(ErrorDaemon))
			{
				ClampWarnDialog(ErrorDaemonGetMaxClamping(ErrorDaemon),
					ErrorDaemonGetMaxClamping(ErrorDaemon)
					* LargeBCD2Double(OverallVolumeScalingReciprocal));
			}
		 else
			{
				if (ClipWarn && (StateInfo.ClippedSampleCount != 0))
					{
						ClipWarnDialog(StateInfo.ClippedSampleCount,StateInfo.TotalSampleCount,
							largefixed2double(StateInfo.MaxClipExtent),
							largefixed2double(StateInfo.MaxClipExtent)
							* LargeBCD2Double(OverallVolumeScalingReciprocal),False);
					}
			}

		DisposeErrorDaemon(ErrorDaemon);
	}
