/* ImportWAVSample.c */
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

#include "ImportWAVSample.h"
#include "MainWindowStuff.h"
#include "Files.h"
#include "Alert.h"
#include "BufferedFileInput.h"
#include "SampleConsts.h"
#include "Memory.h"
#include "DataMunging.h"
#include "SampleObject.h"


/* prototype for the function that actually does the work. */
static MyBoolean		TryToImportWAVFile(BufferedInputRec* File, NumBitsType* NumBitsOut,
											NumChannelsType* NumChannelsOut, char** RawDataPtrOut,
											long* NumSampleFramesOut, long* SamplingRateOut);


/* this routine asks for a file and tries to import the contents of that */
/* file as a WAV sample.  it reports any errors to the user. */
void								ImportWAVSample(struct MainWindowRec* MainWindow)
	{
		FileSpec*					FileLocation;

		CheckPtrExistence(MainWindow);

		/* get the file to be imported */
		FileLocation = GetFileAny();
		if (FileLocation != NIL)
			{
				FileType*					FileDescriptor;

				if (OpenFile(FileLocation,&FileDescriptor,eReadOnly))
					{
						BufferedInputRec*	BufferedInputDescriptor;

						BufferedInputDescriptor = NewBufferedInput(FileDescriptor);
						if (BufferedInputDescriptor != NIL)
							{
								NumBitsType		NumBits;
								NumChannelsType	NumChannels;
								char*					SampleData;
								long					NumFrames;
								long					SamplingRate;

								if (TryToImportWAVFile(BufferedInputDescriptor,&NumBits,&NumChannels,
									&SampleData,&NumFrames,&SamplingRate))
									{
										SampleObjectRec*		ReturnedSampleObject;

										ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
											SampleData,NumBits,NumChannels,0,0,0,0,0,0,0,SamplingRate,
											261.625565300598635);
										if (ReturnedSampleObject != NIL)
											{
												char*					Filename;

												Filename = ExtractFileName(FileLocation);
												if (Filename != NIL)
													{
														/* we don't need to release the name */
														SampleObjectNewName(ReturnedSampleObject,Filename);
													}
											}
										ReleasePtr(SampleData);
									}
								EndBufferedInput(BufferedInputDescriptor);
							}
						 else
							{
								AlertHalt("There is not enough memory available to open the "
									"requested file.",NIL);
							}
						CloseFile(FileDescriptor);
					}
				 else
					{
						AlertHalt("The requested file could not be opened.",NIL);
					}
				DisposeFileSpec(FileLocation);
			}
	}


typedef enum
	{
		eImportUnrecognizedFileFormat EXECUTE(= -5125),
		eImportBadNumberOfChannels,
		eImportNotAPCMFile,
		eImportBadNumberOfBits
	} ImportErrors;

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
static MyBoolean		TryToImportWAVFile(BufferedInputRec* File, NumBitsType* NumBitsOut,
											NumChannelsType* NumChannelsOut, char** RawDataPtrOut,
											long* NumSampleFramesOut, long* SamplingRateOut)
	{
		char							StringBuffer[4];
		unsigned long			TotalFileLength;
		unsigned long			HeaderLength;
		unsigned short		DataTypeDescriptor;
		unsigned short		NumberOfChannels;
		unsigned long			SamplingRate;
		unsigned long			AverageBytesPerSecond;
		unsigned short		BlockAlignment;
		unsigned short		NumberOfBitsRaw;
		NumBitsType				NumBits;
		NumChannelsType		NumChannels;
		unsigned long			SampledNumberOfBytes;
		unsigned long			NumberOfSampleFrames;
		ImportErrors			Error EXECUTE(= (ImportErrors)0);

		CheckPtrExistence(File);

		/*  'RIFF' */
		if (!ReadBufferedInput(File,4,StringBuffer))
			{
			 FileErrorPoint:
				AlertHalt("Unable to read data from the file.",NIL);
				return False;
			}
		if (!MemEqu(StringBuffer,"RIFF",4))
			{
				Error = eImportUnrecognizedFileFormat;
			 DoesntSeemToBeGoodFile1:
				switch (Error)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  "
								"unknown error code"));
							break;
						case eImportUnrecognizedFileFormat:
							AlertHalt("The file does not appear to be a WAV file.  Try importing "
								"it as a RAW file.",NIL);
							break;
						case eImportBadNumberOfChannels:
							AlertHalt("Only files with 1 or 2 channels can be imported.",NIL);
							break;
						case eImportNotAPCMFile:
							AlertHalt("The file is not a PCM file.",NIL);
							break;
						case eImportBadNumberOfBits:
							AlertHalt("Only 8-bit or 16-bit files can be imported.",NIL);
							break;
					}
				return False;
			}

		/*  4-byte little endian length descriptor (minus 8 bytes for these 2 fields) */
		if (!ReadBufferedUnsignedLongLittleEndian(File,&TotalFileLength))
			{
				goto FileErrorPoint;
			}

		/*  'WAVE' */
		if (!ReadBufferedInput(File,4,StringBuffer))
			{
				goto FileErrorPoint;
			}
		if (!MemEqu(StringBuffer,"WAVE",4))
			{
				Error = eImportUnrecognizedFileFormat;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  'fmt ' */
		if (!ReadBufferedInput(File,4,StringBuffer))
			{
				goto FileErrorPoint;
			}
		if (!MemEqu(StringBuffer,"fmt ",4))
			{
				Error = eImportUnrecognizedFileFormat;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  4-byte little endian length descriptor for the 'fmt ' header block */
		/*      - this should be 16.  if not, then it's some other kind of WAV file */
		if (!ReadBufferedUnsignedLongLittleEndian(File,&HeaderLength))
			{
				goto FileErrorPoint;
			}
		if (HeaderLength != 16)
			{
				Error = eImportUnrecognizedFileFormat;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  2-byte little endian format descriptor.  this is always here. */
		/*      - 1 = PCM */
		if (!ReadBufferedUnsignedShortLittleEndian(File,&DataTypeDescriptor))
			{
				goto FileErrorPoint;
			}
		if (DataTypeDescriptor != 1)
			{
				Error = eImportNotAPCMFile;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  2-byte little endian number of channels. */
		if (!ReadBufferedUnsignedShortLittleEndian(File,&NumberOfChannels))
			{
				goto FileErrorPoint;
			}
		if ((NumberOfChannels != 1) && (NumberOfChannels != 2))
			{
				Error = eImportBadNumberOfChannels;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  4-byte little endian sampling rate integer. */
		if (!ReadBufferedUnsignedLongLittleEndian(File,&SamplingRate))
			{
				goto FileErrorPoint;
			}

		/*  4-byte little endian average bytes per second. */
		if (!ReadBufferedUnsignedLongLittleEndian(File,&AverageBytesPerSecond))
			{
				goto FileErrorPoint;
			}

		/*  2-byte little endian block align.  for 8-bit mono, this is 1; for 16-bit */
		/*    stereo, this is 4. */
		if (!ReadBufferedUnsignedShortLittleEndian(File,&BlockAlignment))
			{
				goto FileErrorPoint;
			}

		/*  2-byte little endian number of bits. */
		/*      - 8 = 8-bit */
		/*      - 16 = 16-bit */
		if (!ReadBufferedUnsignedShortLittleEndian(File,&NumberOfBitsRaw))
			{
				goto FileErrorPoint;
			}
		switch (NumberOfBitsRaw)
			{
				default:
					Error = eImportBadNumberOfBits;
					goto DoesntSeemToBeGoodFile1;
				case 8:
					NumBits = eSample8bit;
					break;
				case 16:
					NumBits = eSample16bit;
					break;
			}

		/*  'data' */
		if (!ReadBufferedInput(File,4,StringBuffer))
			{
				goto FileErrorPoint;
			}
		if (!MemEqu(StringBuffer,"data",4))
			{
				Error = eImportUnrecognizedFileFormat;
				goto DoesntSeemToBeGoodFile1;
			}

		/*  4-byte little endian length of sample data descriptor */
		if (!ReadBufferedUnsignedLongLittleEndian(File,&SampledNumberOfBytes))
			{
				goto FileErrorPoint;
			}

		/* calculate number of sample frames */
		switch (NumBits)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
						"number of bits error"));
					break;
				case eSample8bit:
					switch (NumberOfChannels)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
									"number of channels error"));
								break;
							case 1:
								NumberOfSampleFrames = SampledNumberOfBytes / 1;
								break;
							case 2:
								NumberOfSampleFrames = SampledNumberOfBytes / 2;
								break;
							}
					break;
				case eSample16bit:
					switch (NumberOfChannels)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
									"number of channels error"));
								break;
							case 1:
								NumberOfSampleFrames = SampledNumberOfBytes / 2;
								break;
							case 2:
								NumberOfSampleFrames = SampledNumberOfBytes / 4;
								break;
							}
					break;
			}

		/*  any length data.  8-bit data goes from 0..255, but 16-bit data goes */
		/*    from -32768 to 32767. */
		switch (NumberOfChannels)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
						"number of channels error"));
					break;
				case 1:
					NumChannels = eSampleMono;
					break;
				case 2:
					NumChannels = eSampleStereo;
					break;
				}
		switch (NumBits)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
						"number of bits error"));
					break;
				case eSample8bit:
					switch (NumChannels)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
									"number of channels error"));
								break;
							case eSampleMono:
								{
									signed char*		Buffer;
									long						Scan;

									Buffer = (signed char*)AllocPtrCanFail(NumberOfSampleFrames,
										"TryToImportWAVFile data block");
									if (Buffer == NIL)
										{
											AlertHalt("There is not enough memory available to import "
												"the sample.",NIL);
											return False;
										}
									for (Scan = 0; Scan < NumberOfSampleFrames; Scan += 1)
										{
											PRNGCHK(Buffer,&(Buffer[Scan]),sizeof(Buffer[Scan]));
											if (!ReadBufferedSignedChar(File,&(Buffer[Scan])))
												{
													ReleasePtr((char*)Buffer);
													goto FileErrorPoint;
												}
											Buffer[Scan] -= 128; /* change from unsigned to signed */
										}
									*RawDataPtrOut = (char*)Buffer;
								}
								break;
							case eSampleStereo:
								{
									signed char*		Buffer;
									long						Scan;

									Buffer = (signed char*)AllocPtrCanFail(NumberOfSampleFrames * 2,
										"TryToImportWAVFile data block");
									if (Buffer == NIL)
										{
											AlertHalt("There is not enough memory available to import "
												"the sample.",NIL);
											return False;
										}
									for (Scan = 0; Scan < NumberOfSampleFrames * 2; Scan += 1)
										{
											/* alternates frames */
											PRNGCHK(Buffer,&(Buffer[Scan]),sizeof(Buffer[Scan]));
											if (!ReadBufferedSignedChar(File,&(Buffer[Scan])))
												{
													ReleasePtr((char*)Buffer);
													goto FileErrorPoint;
												}
											Buffer[Scan] -= 128; /* change from unsigned to signed */
										}
									*RawDataPtrOut = (char*)Buffer;
								}
								break;
							}
					break;
				case eSample16bit:
					switch (NumChannels)
						{
							default:
								EXECUTE(PRERR(ForceAbort,"TryToImportWAVFile:  internal "
									"number of channels error"));
								break;
							case eSampleMono:
								{
									signed short*		Buffer;
									long						Scan;

									Buffer = (signed short*)AllocPtrCanFail(NumberOfSampleFrames
										* sizeof(short),"TryToImportWAVFile data block");
									if (Buffer == NIL)
										{
											AlertHalt("There is not enough memory available to import "
												"the sample.",NIL);
											return False;
										}
									for (Scan = 0; Scan < NumberOfSampleFrames; Scan += 1)
										{
											PRNGCHK(Buffer,&(Buffer[Scan]),sizeof(Buffer[Scan]));
											if (!ReadBufferedSignedShortLittleEndian(File,&(Buffer[Scan])))
												{
													ReleasePtr((char*)Buffer);
													goto FileErrorPoint;
												}
										}
									*RawDataPtrOut = (char*)Buffer;
								}
								break;
							case eSampleStereo:
								{
									signed short*		Buffer;
									long						Scan;

									Buffer = (signed short*)AllocPtrCanFail(NumberOfSampleFrames
										* sizeof(short) * 2,"TryToImportWAVFile data block");
									if (Buffer == NIL)
										{
											AlertHalt("There is not enough memory available to import "
												"the sample.",NIL);
											return False;
										}
									for (Scan = 0; Scan < NumberOfSampleFrames * 2; Scan += 1)
										{
											/* alternates frames */
											PRNGCHK(Buffer,&(Buffer[Scan]),sizeof(Buffer[Scan]));
											if (!ReadBufferedSignedShortLittleEndian(File,&(Buffer[Scan])))
												{
													ReleasePtr((char*)Buffer);
													goto FileErrorPoint;
												}
										}
									*RawDataPtrOut = (char*)Buffer;
								}
								break;
							}
					break;
			}

		/* output parameters */
		*NumBitsOut = NumBits;
		*NumChannelsOut = NumChannels;
		CheckPtrExistence(*RawDataPtrOut);
		*NumSampleFramesOut = NumberOfSampleFrames;
		*SamplingRateOut = SamplingRate;

		return True;
	}
