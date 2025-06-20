/* ImportAIFFSample.c */
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

#include "ImportAIFFSample.h"
#include "Memory.h"
#include "MainWindowStuff.h"
#include "Files.h"
#include "Alert.h"
#include "BufferedFileInput.h"
#include "SampleConsts.h"
#include "DataMunging.h"
#include "SampleObject.h"


/* prototype for the function that actually does the work. */
static MyBoolean		TryToImportAIFFFile(BufferedInputRec* File, NumBitsType* NumBitsOut,
											NumChannelsType* NumChannelsOut, char** RawDataPtrOut,
											long* NumSampleFramesOut, long* SamplingRateOut);


/* this routine asks for a file and tries to import the contents of that */
/* file as an AIFF sample.  it reports any errors to the user. */
void								ImportAIFFSample(struct MainWindowRec* MainWindow)
	{
		FileSpec*					WhereIsTheFile;

		CheckPtrExistence(MainWindow);
		WhereIsTheFile = GetFileAny();
		if (WhereIsTheFile != NIL)
			{
				FileType*					FileDesc;

				if (OpenFile(WhereIsTheFile,&FileDesc,eReadOnly))
					{
						BufferedInputRec*		File;

						File = NewBufferedInput(FileDesc);
						if (File != NIL)
							{
								NumBitsType		NumBits;
								NumChannelsType	NumChannels;
								char*					SampleData;
								long					NumFrames;
								long					SamplingRate;

								if (TryToImportAIFFFile(File,&NumBits,&NumChannels,
									&SampleData,&NumFrames,&SamplingRate))
									{
										SampleObjectRec*		ReturnedSampleObject;

										ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
											SampleData,NumBits,NumChannels,0,0,0,0,0,0,0,SamplingRate,
											261.625565300598635);
										if (ReturnedSampleObject != NIL)
											{
												char*					Filename;

												Filename = ExtractFileName(WhereIsTheFile);
												if (Filename != NIL)
													{
														/* we don't need to release the name */
														SampleObjectNewName(ReturnedSampleObject,Filename);
													}
											}
										ReleasePtr(SampleData);
									}
								EndBufferedInput(File);
							}
						 else
							{
								AlertHalt("There is not enough memory available to import the file.",NIL);
							}
						CloseFile(FileDesc);
					}
				 else
					{
						AlertHalt("Unable to open the file for reading.",NIL);
					}
				DisposeFileSpec(WhereIsTheFile);
			}
	}


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
static MyBoolean		TryToImportAIFFFile(BufferedInputRec* File, NumBitsType* NumBitsOut,
											NumChannelsType* NumChannelsOut, char** RawDataPtrOut,
											long* NumSampleFramesOut, long* SamplingRateOut)
	{
		char							CharBuff[4];
		MyBoolean					IsAnAIFFCFile;
		long							FormChunkLength;
		void*							RawDataFromFile;
		MyBoolean					RawDataFromFileIsValid = False;
		NumBitsType				NumBits;
		MyBoolean					NumBitsIsValid = False;
		NumChannelsType		NumChannels;
		MyBoolean					NumChannelsIsValid = False;
		long							SamplingRate;
		MyBoolean					SamplingRateIsValid = False;
		unsigned long			NumSampleFrames;
		MyBoolean					NumSampleFramesIsValid = False;

		CheckPtrExistence(File);

		/*     "FORM" */
		if (!ReadBufferedInput(File,4,CharBuff))
			{
			 DiskErrorPoint1:
				AlertHalt("Unable to read data from the file.",NIL);
			 ExitTheProcedureErrorPoint:
				if (RawDataFromFileIsValid)
					{
						ReleasePtr((char*)RawDataFromFile);
					}
				return False;
			}
		if (!MemEqu(CharBuff,"FORM",4))
			{
			 BadFilePoint1:
				AlertHalt("The file is not an AIFF or AIFF-C file.",NIL);
				goto ExitTheProcedureErrorPoint;
			}

		/*     4-byte big endian form chunk length descriptor (minus 8 for "FORM" & this) */
		if (!ReadBufferedUnsignedLongBigEndian(File,(unsigned long*)&FormChunkLength))
			{
				goto DiskErrorPoint1;
			}

		/*     4-byte type */
		/*        "AIFF" = AIFF format file */
		/*        "AIFC" = AIFF-C format file */
		if (!ReadBufferedInput(File,4,CharBuff))
			{
				goto DiskErrorPoint1;
			}
		if (MemEqu(CharBuff,"AIFF",4))
			{
				IsAnAIFFCFile = False;
			}
		else if (MemEqu(CharBuff,"AIFC",4))
			{
				IsAnAIFFCFile = True;
			}
		else
			{
			 UnknownAIFFFilePoint:
				AlertHalt("The file is not a variant of AIFF or AIFF-C file that I can deal with.",NIL);
				goto ExitTheProcedureErrorPoint;
			}
		FormChunkLength -= 4;

		/* now, read in chunks until we die */
		while (FormChunkLength > 0)
			{
				long						LocalChunkLength;

				/* get the chunk type */
				if (!ReadBufferedInput(File,4,CharBuff))
					{
						goto DiskErrorPoint1;
					}
				/* get the chunk length */
				if (!ReadBufferedUnsignedLongBigEndian(File,(unsigned long*)&LocalChunkLength))
					{
						goto DiskErrorPoint1;
					}
				FormChunkLength -= 8;
				/* adjust for even alignment */
				if ((LocalChunkLength % 2) != 0)
					{
						LocalChunkLength += 1;
					}
				FormChunkLength -= LocalChunkLength;

				/* decode the chunk */
				if (MemEqu(CharBuff,"COMM",4))
					{
						unsigned long			Exponent;
						unsigned long			Mantissa;
						char							StupidExtendedThang[10];

						if (!IsAnAIFFCFile)
							{
								unsigned short		ShortInteger;

								/*   Common Chunk for AIFF files */
								/*     "COMM" */
								/*     4-byte big endian length. */
								/*        always 18 for AIFF files */
								if (LocalChunkLength != 18)
									{
										goto UnknownAIFFFilePoint;
									}

								/*     2-byte big endian number of channels */
								if (!ReadBufferedUnsignedShortBigEndian(File,&ShortInteger))
									{
										goto DiskErrorPoint1;
									}
								switch (ShortInteger)
									{
										default:
											goto UnknownAIFFFilePoint;
										case 1:
											NumChannels = eSampleMono;
											NumChannelsIsValid = True;
											break;
										case 2:
											NumChannels = eSampleStereo;
											NumChannelsIsValid = True;
											break;
									}

								/*     4-byte big endian number of sample frames */
								if (!ReadBufferedUnsignedLongBigEndian(File,&NumSampleFrames))
									{
										goto DiskErrorPoint1;
									}
								NumSampleFramesIsValid = True;

								/*     2-byte big endian number of bits per sample */
								/*        a value in the domain 1..32 */
								if (!ReadBufferedUnsignedShortBigEndian(File,&ShortInteger))
									{
										goto DiskErrorPoint1;
									}
								switch (ShortInteger)
									{
										default:
											goto UnknownAIFFFilePoint;
										case 1: case 2: case 3: case 4:
										case 5: case 6: case 7: case 8:
											NumBits = eSample8bit;
											NumBitsIsValid = True;
											break;
										case 9: case 10: case 11: case 12:
										case 13: case 14: case 15: case 16:
											NumBits = eSample16bit;
											NumBitsIsValid = True;
											break;
									}

								/*     10-byte extended precision number of frames per second */
								if (!ReadBufferedInput(File,10,StupidExtendedThang))
									{
										goto DiskErrorPoint1;
									}
							}
						 else
							{
								unsigned short		ShortInteger;
								long							Dumper;

								/*   Common Chunk for AIFF-C files */
								/*     "COMM" */
								/*     4-byte big endian length. */
								/*        always 18 for AIFF files */
								if (LocalChunkLength < 22)
									{
										goto UnknownAIFFFilePoint;
									}

								/*     2-byte big endian number of channels */
								if (!ReadBufferedUnsignedShortBigEndian(File,&ShortInteger))
									{
										goto DiskErrorPoint1;
									}
								switch (ShortInteger)
									{
										default:
											goto UnknownAIFFFilePoint;
										case 1:
											NumChannels = eSampleMono;
											NumChannelsIsValid = True;
											break;
										case 2:
											NumChannels = eSampleStereo;
											NumChannelsIsValid = True;
											break;
									}

								/*     4-byte big endian number of sample frames */
								if (!ReadBufferedUnsignedLongBigEndian(File,&NumSampleFrames))
									{
										goto DiskErrorPoint1;
									}
								NumSampleFramesIsValid = True;

								/*     2-byte big endian number of bits per sample */
								/*        a value in the domain 1..32 */
								if (!ReadBufferedUnsignedShortBigEndian(File,&ShortInteger))
									{
										goto DiskErrorPoint1;
									}
								switch (ShortInteger)
									{
										default:
											goto UnknownAIFFFilePoint;
										case 1: case 2: case 3: case 4:
										case 5: case 6: case 7: case 8:
											NumBits = eSample8bit;
											NumBitsIsValid = True;
											break;
										case 9: case 10: case 11: case 12:
										case 13: case 14: case 15: case 16:
											NumBits = eSample16bit;
											NumBitsIsValid = True;
											break;
									}

								/*     10-byte extended precision number of frames per second */
								if (!ReadBufferedInput(File,10,StupidExtendedThang))
									{
										goto DiskErrorPoint1;
									}

								/*     4-byte character code ID for the compression method */
								/*        "NONE" means there is no compression method used */
								if (!ReadBufferedInput(File,4,CharBuff))
									{
										goto DiskErrorPoint1;
									}
								if (!MemEqu(CharBuff,"NONE",4))
									{
										goto UnknownAIFFFilePoint;
									}

								/*     some characters in a string identifying the compression method */
								/*        this must be padded to an even number of bytes, but the pad is */
								/*        NOT included in the length descriptor for the chunk. */
								/*        for uncompressed data, the string should be */
								/*        "\x0enot compressed\x00", including the null, for 16 bytes. */
								/*        the total chunk length is thus 38 bytes. */
								for (Dumper = 0; Dumper < LocalChunkLength - 22; Dumper += 1)
									{
										unsigned char			Stupid;

										if (!ReadBufferedUnsignedChar(File,&Stupid))
											{
												goto DiskErrorPoint1;
											}
									}
							}

						/* extended 22050 = 400D AC44000000000000 */
						/* extended 22051 = 400D AC46000000000000 */
						/* extended 44100 = 400E AC44000000000000 */
						/* extended 44101 = 400E AC45000000000000 */
						Exponent = (((long)StupidExtendedThang[0] & 0xff) << 8)
							| ((long)StupidExtendedThang[1] & 0xff);
						Mantissa = (((long)StupidExtendedThang[2] & 0xff) << 24)
							| (((long)StupidExtendedThang[3] & 0xff) << 16)
							| (((long)StupidExtendedThang[4] & 0xff) << 8)
							| ((long)StupidExtendedThang[5] & 0xff);
						SamplingRate = (Mantissa >> (0x401e - Exponent));
						if (SamplingRate < MINSAMPLINGRATE)
							{
								SamplingRate = MINSAMPLINGRATE;
							}
						if (SamplingRate > MAXSAMPLINGRATE)
							{
								SamplingRate = MAXSAMPLINGRATE;
							}
						SamplingRateIsValid = True;
					}
				else if (MemEqu(CharBuff,"SSND",4))
					{
						unsigned long			AlignmentFactor;
						unsigned long			OffsetToFirstByte;

						/*   Sound Data Chunk */
						/*     "SSND" */
						/*     4-byte big endian number of bytes in sample data array */

						/* only one of these is allowed */
						if (RawDataFromFileIsValid)
							{
								goto UnknownAIFFFilePoint;
							}

						/*     4-byte big endian offset to the first byte of sample data in the array */
						if (!ReadBufferedUnsignedLongBigEndian(File,&OffsetToFirstByte))
							{
								goto DiskErrorPoint1;
							}
						if (OffsetToFirstByte != 0)
							{
								goto UnknownAIFFFilePoint;
							}

						/*     4-byte big endian number of bytes to which the sound data is aligned. */
						if (!ReadBufferedUnsignedLongBigEndian(File,&AlignmentFactor))
							{
								goto DiskErrorPoint1;
							}
						if (AlignmentFactor != 0)
							{
								goto UnknownAIFFFilePoint;
							}

						/*     any length vector of raw sound data. */
						/*        this must be padded to an even number of bytes, but the pad is */
						/*        NOT included in the length descriptor for the chunk. */
						/*        Samples are stored in an integral number of bytes, the smallest that */
						/*        is required for the specified number of bits.  If this is not an even */
						/*        multiple of 8, then the data is shifted left and the low bits are zeroed */
						/*        Multichannel sound is interleaved with the left channel first. */
						RawDataFromFile = AllocPtrCanFail(LocalChunkLength - 8,
							"TryToImportAIFFFile:  RawDataBuffer");
						if (RawDataFromFile == NIL)
							{
							 NotEnoughMemoryPoint:
								AlertHalt("There is not enough memory available to import the sample.",NIL);
								goto ExitTheProcedureErrorPoint;
							}
						RawDataFromFileIsValid = True;
						if (!ReadBufferedInput(File,LocalChunkLength - 8,(char*)RawDataFromFile))
							{
								goto DiskErrorPoint1;
							}
					}
				else
					{
						/* just read the data & get rid of it */
						while (LocalChunkLength > 0)
							{
								unsigned char			Stupid;

								if (!ReadBufferedUnsignedChar(File,&Stupid))
									{
										goto DiskErrorPoint1;
									}
								LocalChunkLength -= 1;
							}
					}
			}

		/* now, deal with the stuff we just got */
		if (!RawDataFromFileIsValid || !NumBitsIsValid || !NumChannelsIsValid
			|| !SamplingRateIsValid || !NumSampleFramesIsValid)
			{
				goto UnknownAIFFFilePoint;
			}

		switch (NumChannels)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"ImportAIFFSample:  bad NumChannels"));
					break;
				case eSampleMono:
					switch (NumBits)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"ImportAIFFSample:  bad NumBits"));
								break;
							case eSample8bit:
								{
									long						Scan;
									long						ActualLimit;
									char*						SampleVector;

									SampleVector = AllocPtrCanFail(sizeof(char) * NumSampleFrames,
										"ImportAIFFSample:  finished vector");
									ActualLimit = PtrSize((char*)RawDataFromFile);
									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											PRNGCHK(SampleVector,&(SampleVector[Scan]),
												sizeof(SampleVector[Scan]));
											if (Scan * sizeof(char) < ActualLimit)
												{
													PRNGCHK(RawDataFromFile,&(((signed char*)RawDataFromFile)[Scan]),
														sizeof(((signed char*)RawDataFromFile)[Scan]));
													SampleVector[Scan] = ((signed char*)RawDataFromFile)[Scan];
												}
											 else
												{
													SampleVector[Scan] = 0;
												}
										}
									*RawDataPtrOut = (char*)SampleVector;
								}
								break;
							case eSample16bit:
								{
									long						Scan;
									long						ActualLimit;
									short*					SampleVector;

									SampleVector = (short*)AllocPtrCanFail(sizeof(short) * NumSampleFrames,
										"ImportAIFFSample:  finished vector");
									ActualLimit = PtrSize((char*)RawDataFromFile) / 2;
									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											PRNGCHK(SampleVector,&(SampleVector[Scan]),
												sizeof(SampleVector[Scan]));
											if (Scan * sizeof(char) < ActualLimit)
												{
													PRNGCHK(RawDataFromFile,&(((short*)RawDataFromFile)[Scan * 2 + 1]),
														sizeof(((char*)RawDataFromFile)[Scan * 2 + 1]));
													SampleVector[Scan]
														= (((short)((signed char*)RawDataFromFile)[2
														* Scan] & 0xff) << 8) | (((char*)RawDataFromFile)[2
														* Scan + 1] & 0xff);
												}
											 else
												{
													SampleVector[Scan] = 0;
												}
										}
									*RawDataPtrOut = (char*)SampleVector;
								}
								break;
						}
					break;
				case eSampleStereo:
					switch (NumBits)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"ImportAIFFSample:  bad NumBits"));
								break;
							case eSample8bit:
								{
									long						Scan;
									long						ActualLimit;
									char*						SampleVector;

									SampleVector = AllocPtrCanFail(sizeof(char) * NumSampleFrames * 2,
										"ImportAIFFSample:  finished vector");
									ActualLimit = PtrSize((char*)RawDataFromFile);
									for (Scan = 0; Scan < NumSampleFrames * 2; Scan += 1)
										{
											PRNGCHK(SampleVector,&(SampleVector[Scan]),
												sizeof(SampleVector[Scan]));
											if (Scan * sizeof(char) < ActualLimit)
												{
													PRNGCHK(RawDataFromFile,&(((signed char*)RawDataFromFile)[Scan]),
														sizeof(((signed char*)RawDataFromFile)[Scan]));
													SampleVector[Scan] = ((signed char*)RawDataFromFile)[Scan];
												}
											 else
												{
													SampleVector[Scan] = 0;
												}
										}
									*RawDataPtrOut = (char*)SampleVector;
								}
								break;
							case eSample16bit:
								{
									long						Scan;
									long						ActualLimit;
									short*					SampleVector;

									SampleVector = (short*)AllocPtrCanFail(sizeof(short) * 2
										* NumSampleFrames,"ImportAIFFSample:  finished vector");
									ActualLimit = PtrSize((char*)RawDataFromFile) / 2;
									for (Scan = 0; Scan < NumSampleFrames * 2; Scan += 1)
										{
											PRNGCHK(SampleVector,&(SampleVector[Scan]),
												sizeof(SampleVector[Scan]));
											if (Scan * sizeof(char) < ActualLimit)
												{
													PRNGCHK(RawDataFromFile,&(((char*)RawDataFromFile)[Scan * 2
														+ 1]),sizeof(((char*)RawDataFromFile)[Scan * 2 + 1]));
													SampleVector[Scan]
														= (((short)((signed char*)RawDataFromFile)[2
														* Scan] & 0xff) << 8) | (((char*)RawDataFromFile)[2
														* Scan + 1] & 0xff);
												}
											 else
												{
													SampleVector[Scan] = 0;
												}
										}
									*RawDataPtrOut = (char*)SampleVector;
								}
								break;
						}
					break;
			}

		ReleasePtr((char*)RawDataFromFile);

		*NumBitsOut = NumBits;
		*NumChannelsOut = NumChannels;
		CheckPtrExistence(*RawDataPtrOut);
		*NumSampleFramesOut = NumSampleFrames;
		*SamplingRateOut = SamplingRate;

		return True;
	}
