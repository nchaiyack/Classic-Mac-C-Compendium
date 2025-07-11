/* ExportAIFFSample.c */
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

#include "ExportAIFFSample.h"
#include "Memory.h"
#include "SampleConsts.h"
#include "Files.h"
#include "SampleStorageActual.h"
#include "BufferedFileOutput.h"
#include "Alert.h"


/* prototype for the function that actually does the work. */
static void					TryToExportAIFF(SampleStorageActualRec* Sample,
											BufferedOutputRec* File, long SamplingRate);


/* this routine saves the data in the provided sample storage object as an AIFF-C */
/* formatted file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportAIFFSample(struct SampleStorageActualRec* TheSample,
											long SamplingRate)
	{
		FileSpec*					WhereToPut;

		CheckPtrExistence(TheSample);
		WhereToPut = PutFile("Untitled.AIFF");
		if (WhereToPut != NIL)
			{
				if (CreateFile(WhereToPut,ApplicationCreator,CODE4BYTES('A','I','F','F')))
					{
						FileType*					FileDesc;

						if (OpenFile(WhereToPut,&FileDesc,eReadAndWrite))
							{
								BufferedOutputRec*	File;

								File = NewBufferedOutput(FileDesc);
								if (File != NIL)
									{
										TryToExportAIFF(TheSample,File,SamplingRate);
										EndBufferedOutput(File);
									}
								 else
									{
										AlertHalt("There is not enough memory available to export the sample.",NIL);
									}
								CloseFile(FileDesc);
							}
						 else
							{
								AlertHalt("Unable to open the file for writing.",NIL);
							}
					}
				 else
					{
						AlertHalt("Unable to create the file.",NIL);
					}
				DisposeFileSpec(WhereToPut);
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
static void					TryToExportAIFF(SampleStorageActualRec* Sample,
											BufferedOutputRec* File, long SamplingRate)
	{
		long							NumSampleFrames;
		long							BytesPerFrame;
		long							ActualBytesInTheSampleChunk;
		long							BytesInTheFormChunk;
		long							NumberOfChannels;
		long							BitDepthOfSamples;
		unsigned long			Mantissa;
		unsigned long			Exponent;
		char							StupidExtendedThing[10];

		CheckPtrExistence(Sample);
		CheckPtrExistence(File);

		NumSampleFrames = GetSampleStorageActualNumFrames(Sample);

		BytesPerFrame = 1;
		switch (GetSampleStorageActualNumChannels(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportAIFF:  bad value from "
						"GetSampleStorageActualNumChannels"));
					break;
				case eSampleStereo:
					NumberOfChannels = 2;
					break;
				case eSampleMono:
					NumberOfChannels = 1;
					break;
			}
		BytesPerFrame *= NumberOfChannels;

		switch (GetSampleStorageActualNumBits(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportAIFF:  bad value from "
						"GetSampleStorageActualNumBits"));
					break;
				case eSample16bit:
					BitDepthOfSamples = 16;
					break;
				case eSample8bit:
					BitDepthOfSamples = 8;
					break;
			}
		BytesPerFrame *= (BitDepthOfSamples / 8);

		/* extended 22050 = 400D AC44000000000000 */
		/* extended 22051 = 400D AC46000000000000 */
		/* extended 44100 = 400E AC44000000000000 */
		/* extended 44101 = 400E AC45000000000000 */
		Exponent = 0x401e;
		Mantissa = SamplingRate;
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

		ActualBytesInTheSampleChunk = BytesPerFrame * NumSampleFrames;
		if ((ActualBytesInTheSampleChunk % 2) != 0)
			{
				ActualBytesInTheSampleChunk += 1;
			}

		BytesInTheFormChunk = (ActualBytesInTheSampleChunk + 8) + (38 + 8) + (12);

		/*     "FORM" */
		if (!WriteBufferedOutput(File,4,"FORM"))
			{
			 DiskErrorPoint:
				AlertHalt("Unable to write to the file.",NIL);
				return;
			}

		/*     4-byte big endian form chunk length descriptor (minus 8 for "FORM" & this) */
		if (!WriteBufferedUnsignedLongBigEndian(File,BytesInTheFormChunk))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte type */
		/*        "AIFF" = AIFF format file */
		/*        "AIFC" = AIFF-C format file */
		if (!WriteBufferedOutput(File,4,"AIFC"))
			{
				goto DiskErrorPoint;
			}

		/*   Version Chunk (this only occurs in AIFF-C files) */

		/*     "FVER" */
		if (!WriteBufferedOutput(File,4,"FVER"))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian length, which should always be the value 4 (four) */
		if (!WriteBufferedUnsignedLongBigEndian(File,4))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte date code.  this is probably 0xA2805140 (stored big endian), but it */
		/*          probably doesn't matter. */
		if (!WriteBufferedOutput(File,4,"\xa2\x80\x51\x40"))
			{
				goto DiskErrorPoint;
			}

		/*   Common Chunk for AIFF-C files */

		/*     "COMM" */
		if (!WriteBufferedOutput(File,4,"COMM"))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian length. */
		/*        22 + compression method string length for AIFF-C files */
		if (!WriteBufferedUnsignedLongBigEndian(File,38))
			{
				goto DiskErrorPoint;
			}

		/*     2-byte big endian number of channels */
		if (!WriteBufferedUnsignedShortBigEndian(File,NumberOfChannels))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian number of sample frames */
		if (!WriteBufferedUnsignedLongBigEndian(File,NumSampleFrames))
			{
				goto DiskErrorPoint;
			}

		/*     2-byte big endian number of bits per sample */
		/*        a value in the domain 1..32 */
		if (!WriteBufferedUnsignedShortBigEndian(File,BitDepthOfSamples))
			{
				goto DiskErrorPoint;
			}

		/*     10-byte extended precision number of frames per second */
		if (!WriteBufferedOutput(File,10,StupidExtendedThing))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte character code ID for the compression method */
		/*        "NONE" means there is no compression method used */
		if (!WriteBufferedOutput(File,4,"NONE"))
			{
				goto DiskErrorPoint;
			}

		/*     some characters in a string identifying the compression method */
		/*        this must be padded to an even number of bytes, but the pad is */
		/*        NOT included in the length descriptor for the chunk. */
		/*        for uncompressed data, the string should be */
		/*        "\x0enot compressed\x00", including the null, for 16 bytes. */
		/*        the total chunk length is thus 38 bytes. */
		if (!WriteBufferedOutput(File,16,"\x0enot compressed\x00"))
			{
				goto DiskErrorPoint;
			}

		/*   Sound Data Chunk */

		/*     "SSND" */
		if (!WriteBufferedOutput(File,4,"SSND"))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian number of bytes in sample data array */
		if (!WriteBufferedUnsignedLongBigEndian(File,BytesPerFrame * NumSampleFrames))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian offset to the first byte of sample data in the array */
		if (!WriteBufferedUnsignedLongBigEndian(File,0))
			{
				goto DiskErrorPoint;
			}

		/*     4-byte big endian number of bytes to which the sound data is aligned. */
		if (!WriteBufferedUnsignedLongBigEndian(File,0))
			{
				goto DiskErrorPoint;
			}

		/*     any length vector of raw sound data. */
		/*        this must be padded to an even number of bytes, but the pad is */
		/*        NOT included in the length descriptor for the chunk. */
		/*        Samples are stored in an integral number of bytes, the smallest that */
		/*        is required for the specified number of bits.  If this is not an even */
		/*        multiple of 8, then the data is shifted left and the low bits are zeroed */
		/*        Multichannel sound is interleaved with the left channel first. */
		switch (GetSampleStorageActualNumChannels(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportAIFF:  bad value from "
						"GetSampleStorageActualNumChannels"));
					break;
				case eSampleStereo:
					switch (GetSampleStorageActualNumBits(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportAIFF:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSample16bit:
								{
									long						Scan;

									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eLeftChannel);
											if (!WriteBufferedSignedShortBigEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}
											SampleWord = GetSampleStorageActualValue(Sample,Scan,eRightChannel);
											if (!WriteBufferedSignedShortBigEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}
										}
									/* 16-bit is always even aligned */
								}
								break;
							case eSample8bit:
								{
									long						Scan;

									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eLeftChannel);
											if (!WriteBufferedSignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT)))
												{
													goto DiskErrorPoint;
												}
											SampleWord = GetSampleStorageActualValue(Sample,Scan,eRightChannel);
											if (!WriteBufferedSignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT)))
												{
													goto DiskErrorPoint;
												}
										}
									/* stereo is always even aligned */
								}
								break;
						}
					break;
				case eSampleMono:
					switch (GetSampleStorageActualNumBits(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportAIFF:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSample16bit:
								{
									long						Scan;

									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eMonoChannel);
											if (!WriteBufferedSignedShortBigEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}
										}
									/* 16-bit is always even aligned */
								}
								break;
							case eSample8bit:
								{
									long						Scan;

									for (Scan = 0; Scan < NumSampleFrames; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eMonoChannel);
											if (!WriteBufferedSignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT)))
												{
													goto DiskErrorPoint;
												}
										}
									/* 8-bit mono might be unaligned */
									if ((NumSampleFrames % 2) != 0)
										{
											/* write an extra pad byte */
											if (!WriteBufferedUnsignedChar(File,0))
												{
													goto DiskErrorPoint;
												}
										}
								}
								break;
						}
					break;
			}

		/* all done */
	}
