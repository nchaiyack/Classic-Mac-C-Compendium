/* ExportWAVSample.c */
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

#include "ExportWAVSample.h"
#include "SampleConsts.h"
#include "SampleStorageActual.h"
#include "Memory.h"
#include "Files.h"
#include "Alert.h"
#include "BufferedFileOutput.h"


/* prototype for the function that actually does the work. */
static MyBoolean		TryToExportWAVFile(BufferedOutputRec* File,
											SampleStorageActualRec* Sample, long SamplingRate);


/* this routine saves the data in the provided sample storage object as a WAVE */
/* formatted file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportWAVSample(struct SampleStorageActualRec* TheSample,
											long SamplingRate)
	{
		FileSpec*					WhereToSave;

		CheckPtrExistence(TheSample);
		WhereToSave = PutFile("Untitled.WAV");
		if (WhereToSave != NIL)
			{
				if (CreateFile(WhereToSave,ApplicationCreator,CODE4BYTES('?','?','?','?')))
					{
						FileType*					TheFileDescriptor;

						if (OpenFile(WhereToSave,&TheFileDescriptor,eReadAndWrite))
							{
								BufferedOutputRec*	BufferedFileThing;

								BufferedFileThing = NewBufferedOutput(TheFileDescriptor);
								if (BufferedFileThing != NIL)
									{
										(void)TryToExportWAVFile(BufferedFileThing,TheSample,SamplingRate);
										EndBufferedOutput(BufferedFileThing);
									}
								 else
									{
										AlertHalt("There is not enough memory available to export "
											"the file.",NIL);
									}
								CloseFile(TheFileDescriptor);
							}
						 else
							{
								AlertHalt("Unable to open the file for writing.",NIL);
							}
					}
				 else
					{
						AlertHalt("The file could not be created.",NIL);
					}
				DisposeFileSpec(WhereToSave);
			}
	}


/* RIFF file format, with WAVE information */
/*  'RIFF' */
/*  4-byte little endian length descriptor (minus 8 bytes for these 2 fields) */
/*  'WAVE' */
/*  'fmt ' */
/*  4-byte little endian length descriptor for the 'fmt ' header block */
/*      - this should be 16.  if not, then it's some other kind of WAV file */
/*  2-byte little endian format descriptor.  this is always here. */
/*      - 1 = PCM */
/*  2-byte little endian number of channels. */
/*  4-byte little endian sampling rate integer. */
/*  4-byte little endian average bytes per second. */
/*  2-byte little endian block align.  for 8-bit mono, this is 1; for 16-bit */
/*    stereo, this is 4. */
/*  2-byte little endian number of bits. */
/*      - 8 = 8-bit */
/*      - 16 = 16-bit */
/*  'data' */
/*  4-byte little endian length of sample data descriptor */
/*  any length data.  8-bit data goes from 0..255, but 16-bit data goes */
/*    from -32768 to 32767. */
static MyBoolean		TryToExportWAVFile(BufferedOutputRec* File,
											SampleStorageActualRec* Sample, long SamplingRate)
	{
		unsigned long			TotalSampleLength;
		unsigned long			TotalChunkLength;
		unsigned long			BytesPerSecond;
		unsigned short		BlockAlignment;

		CheckPtrExistence(File);
		CheckPtrExistence(Sample);

		/*  'RIFF' */
		if (!WriteBufferedOutput(File,4,"RIFF"))
			{
			 DiskErrorPoint:
				AlertHalt("A file error occurred and the sample couldn't be exported.",NIL);
				return False;
			}

		/* figure out how long chunk will be */
		switch (GetSampleStorageActualNumBits(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
						"GetSampleStorageActualNumBits"));
					break;
				case eSample8bit:
					switch (GetSampleStorageActualNumChannels(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSampleMono:
								TotalSampleLength = GetSampleStorageActualNumFrames(Sample) * 1;
								BytesPerSecond = SamplingRate * 1;
								BlockAlignment = 1;
								break;
							case eSampleStereo:
								TotalSampleLength = GetSampleStorageActualNumFrames(Sample) * 2;
								BytesPerSecond = SamplingRate * 2;
								BlockAlignment = 2;
								break;
						}
					break;
				case eSample16bit:
					switch (GetSampleStorageActualNumChannels(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSampleMono:
								TotalSampleLength = GetSampleStorageActualNumFrames(Sample) * 2;
								BytesPerSecond = SamplingRate * 2;
								BlockAlignment = 2;
								break;
							case eSampleStereo:
								TotalSampleLength = GetSampleStorageActualNumFrames(Sample) * 4;
								BytesPerSecond = SamplingRate * 4;
								BlockAlignment = 4;
								break;
						}
					break;
			}
		TotalChunkLength = TotalSampleLength + 36;

		/*  4-byte little endian length descriptor (minus 8 bytes for these 2 fields) */
		if (!WriteBufferedUnsignedLongLittleEndian(File,TotalChunkLength))
			{
				goto DiskErrorPoint;
			}

		/*  'WAVE' */
		/*  'fmt ' */
		if (!WriteBufferedOutput(File,8,"WAVEfmt "))
			{
				goto DiskErrorPoint;
			}

		/*  4-byte little endian length descriptor for the 'fmt ' header block */
		/*      - this should be 16.  if not, then it's some other kind of WAV file */
		if (!WriteBufferedUnsignedLongLittleEndian(File,16))
			{
				goto DiskErrorPoint;
			}

		/*  2-byte little endian format descriptor.  this is always here. */
		/*      - 1 = PCM */
		if (!WriteBufferedUnsignedShortLittleEndian(File,1))
			{
				goto DiskErrorPoint;
			}

		/*  2-byte little endian number of channels. */
		switch (GetSampleStorageActualNumChannels(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
						"GetSampleStorageActualNumBits"));
					break;
				case eSampleMono:
					if (!WriteBufferedUnsignedShortLittleEndian(File,1))
						{
							goto DiskErrorPoint;
						}
					break;
				case eSampleStereo:
					if (!WriteBufferedUnsignedShortLittleEndian(File,2))
						{
							goto DiskErrorPoint;
						}
					break;
			}

		/*  4-byte little endian sampling rate integer. */
		if (!WriteBufferedUnsignedLongLittleEndian(File,SamplingRate))
			{
				goto DiskErrorPoint;
			}

		/*  4-byte little endian average bytes per second. */
		if (!WriteBufferedUnsignedLongLittleEndian(File,BytesPerSecond))
			{
				goto DiskErrorPoint;
			}

		/*  2-byte little endian block align.  for 8-bit mono, this is 1; for 16-bit */
		/*    stereo, this is 4. */
		if (!WriteBufferedUnsignedShortLittleEndian(File,BlockAlignment))
			{
				goto DiskErrorPoint;
			}

		/*  2-byte little endian number of bits. */
		/*      - 8 = 8-bit */
		/*      - 16 = 16-bit */
		switch (GetSampleStorageActualNumBits(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
						"GetSampleStorageActualNumBits"));
					break;
				case eSample8bit:
					if (!WriteBufferedUnsignedShortLittleEndian(File,8))
						{
							goto DiskErrorPoint;
						}
					break;
				case eSample16bit:
					if (!WriteBufferedUnsignedShortLittleEndian(File,16))
						{
							goto DiskErrorPoint;
						}
					break;
			}

		/*  'data' */
		if (!WriteBufferedOutput(File,4,"data"))
			{
				goto DiskErrorPoint;
			}

		/*  4-byte little endian length of sample data descriptor */
		if (!WriteBufferedUnsignedLongLittleEndian(File,TotalSampleLength))
			{
				goto DiskErrorPoint;
			}

		/*  any length data.  8-bit data goes from 0..255, but 16-bit data goes */
		/*    from -32768 to 32767. */
		switch (GetSampleStorageActualNumBits(Sample))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
						"GetSampleStorageActualNumBits"));
					break;
				case eSample8bit:
					switch (GetSampleStorageActualNumChannels(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSampleMono:
								{
									long						Limit;
									long						Scan;

									Limit = GetSampleStorageActualNumFrames(Sample);
									for (Scan = 0; Scan < Limit; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eMonoChannel);
											if (!WriteBufferedUnsignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT) + 128))
												{
													goto DiskErrorPoint;
												}
										}
								}
								break;
							case eSampleStereo:
								{
									long						Limit;
									long						Scan;

									Limit = GetSampleStorageActualNumFrames(Sample);
									for (Scan = 0; Scan < Limit; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eLeftChannel);
											if (!WriteBufferedUnsignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT) + 128))
												{
													goto DiskErrorPoint;
												}

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eRightChannel);
											if (!WriteBufferedUnsignedChar(File,
												roundtonearest(largefixed2double(SampleWord) * MAX8BIT) + 128))
												{
													goto DiskErrorPoint;
												}
										}
								}
								break;
						}
					break;
				case eSample16bit:
					switch (GetSampleStorageActualNumChannels(Sample))
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToExportWAVFile:  bad value from "
									"GetSampleStorageActualNumBits"));
								break;
							case eSampleMono:
								{
									long						Limit;
									long						Scan;

									Limit = GetSampleStorageActualNumFrames(Sample);
									for (Scan = 0; Scan < Limit; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eMonoChannel);
											if (!WriteBufferedSignedShortLittleEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}
										}
								}
								break;
							case eSampleStereo:
								{
									long						Limit;
									long						Scan;

									Limit = GetSampleStorageActualNumFrames(Sample);
									for (Scan = 0; Scan < Limit; Scan += 1)
										{
											largefixedsigned		SampleWord;

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eLeftChannel);
											if (!WriteBufferedSignedShortLittleEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}

											SampleWord = GetSampleStorageActualValue(Sample,Scan,eRightChannel);
											if (!WriteBufferedSignedShortLittleEndian(File,
												roundtonearest(largefixed2double(SampleWord) * MAX16BIT)))
												{
													goto DiskErrorPoint;
												}
										}
								}
								break;
						}
					break;
			}

		return True;
	}
