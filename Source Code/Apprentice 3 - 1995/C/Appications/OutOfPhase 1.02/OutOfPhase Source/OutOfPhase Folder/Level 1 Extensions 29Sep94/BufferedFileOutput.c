/* BufferedFileOutput.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "BufferedFileOutput.h"
#include "Files.h"
#include "Memory.h"


#define BUFFERSIZE (4096) /* must be a power of two! */
#define BUFFERMASK (BUFFERSIZE - 1)


struct BufferedOutputRec
	{
		FileType*				FileDescriptor;
		long						IndexIntoBuffer;
		long						IndexIntoFileOfStartOfBuffer;
		MyBoolean				ErrorOcurredCantBeUsed;
		char						Buffer[BUFFERSIZE];
	};


/* create a new buffered output object around a file.  the file is not allowed */
/* to be tampered with after it has been registered with this since I am too */
/* lazy to implement proper buffering in the Level 0 library like I should. */
BufferedOutputRec*	NewBufferedOutput(struct FileType* TheFileDescriptor)
	{
		long								FileIndex;
		BufferedOutputRec*	BufferedThang;

		CheckPtrExistence(TheFileDescriptor);

		BufferedThang = (BufferedOutputRec*)AllocPtrCanFail(sizeof(BufferedOutputRec),
			"BufferedOutputRec");
		if (BufferedThang == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		BufferedThang->FileDescriptor = TheFileDescriptor;

		FileIndex = GetFilePosition(TheFileDescriptor);
		BufferedThang->IndexIntoBuffer = FileIndex & BUFFERMASK;
		BufferedThang->IndexIntoFileOfStartOfBuffer = FileIndex & (~BUFFERMASK);

		ERROR(GetFileLength(TheFileDescriptor) > FileIndex,PRERR(ForceAbort,
			"BufferedOutputRec:  can't be used for writing into the middle of a file"));

		BufferedThang->ErrorOcurredCantBeUsed = False;

		/* preload the buffer */
		if (!SetFilePosition(TheFileDescriptor,BufferedThang->IndexIntoFileOfStartOfBuffer))
			{
			 FailurePoint2:
				ReleasePtr((char*)BufferedThang);
				goto FailurePoint1;
			}
		if (0 != ReadFromFile(TheFileDescriptor,&(BufferedThang->Buffer[0]),
			BufferedThang->IndexIntoBuffer))
			{
			 FailurePoint3:
				goto FailurePoint2;
			}
		if (!SetFilePosition(TheFileDescriptor,BufferedThang->IndexIntoFileOfStartOfBuffer))
			{
			 FailurePoint4:
				ReleasePtr((char*)BufferedThang);
				goto FailurePoint3;
			}

		return BufferedThang;
	}


static MyBoolean		FlushBuffers(BufferedOutputRec* BufferedThang)
	{
		MyBoolean					Successful;

		CheckPtrExistence(BufferedThang);
		ERROR(BufferedThang->ErrorOcurredCantBeUsed,PRERR(ForceAbort,
			"FlushBuffers:  error flag is set"));
		ERROR(GetFilePosition(BufferedThang->FileDescriptor)
			!= BufferedThang->IndexIntoFileOfStartOfBuffer,PRERR(ForceAbort,
			"FlushBuffers:  file and buffer pointers are out of sync"));
		Successful = (0 == WriteToFile(BufferedThang->FileDescriptor,
			&(BufferedThang->Buffer[0]),BufferedThang->IndexIntoBuffer));
		if (!Successful)
			{
				BufferedThang->ErrorOcurredCantBeUsed = True;
			}
		return Successful;
	}


/* clean up a buffered output object from around a file.  the file may be */
/* used normally after this has been called.  if it failed to write the */
/* data out to disk, then it returns False. */
MyBoolean						EndBufferedOutput(BufferedOutputRec* BufferedThang)
	{
		MyBoolean					SuccessFlag;

		CheckPtrExistence(BufferedThang);

		if (!BufferedThang->ErrorOcurredCantBeUsed)
			{
				SuccessFlag = FlushBuffers(BufferedThang);
			}
		 else
			{
				SuccessFlag = True;
			}

		ReleasePtr((char*)BufferedThang);

		return SuccessFlag;
	}


/* write a raw block of data to the file.  it returns 0 if it wrote all of the */
/* data in or however many bytes were not able to be written. */
MyBoolean						WriteBufferedOutput(BufferedOutputRec* BufferedThang,
											long RequestedBytes, char* PlaceToGetFrom)
	{
		CheckPtrExistence(BufferedThang);
		ERROR(RequestedBytes < 0,PRERR(ForceAbort,
			"WriteBufferedOutput:  bad requested bytes"));
		ERROR(BufferedThang->ErrorOcurredCantBeUsed,PRERR(ForceAbort,
			"WriteBufferedOutput:  error flag is set"));

		/* output into the current buffer */
		while ((RequestedBytes > 0) && (BufferedThang->IndexIntoBuffer < BUFFERSIZE))
			{
				BufferedThang->Buffer[BufferedThang->IndexIntoBuffer] = *PlaceToGetFrom;
				PlaceToGetFrom += 1;
				BufferedThang->IndexIntoBuffer += 1;
				RequestedBytes -= 1;
			}
		/* flush the buffer */
		if (BufferedThang->IndexIntoBuffer == BUFFERSIZE)
			{
				if (!FlushBuffers(BufferedThang))
					{
						BufferedThang->ErrorOcurredCantBeUsed = True;
						return False;
					}
				BufferedThang->IndexIntoBuffer = 0;
				BufferedThang->IndexIntoFileOfStartOfBuffer += BUFFERSIZE;
			}
		if (RequestedBytes == 0)
			{
				/* all done! */
				return True;
			}

		/* write out whole blocks while there's enough to do */
		while (RequestedBytes >= BUFFERSIZE)
			{
				if (0 != WriteToFile(BufferedThang->FileDescriptor,PlaceToGetFrom,BUFFERSIZE))
					{
						BufferedThang->ErrorOcurredCantBeUsed = True;
						return False;
					}
				BufferedThang->IndexIntoFileOfStartOfBuffer += BUFFERSIZE;
				RequestedBytes -= BUFFERSIZE;
				PlaceToGetFrom += BUFFERSIZE;
			}
		if (RequestedBytes == 0)
			{
				/* hmm done */
				return True;
			}

		/* write out bytes into the final block */
		while (RequestedBytes > 0)
			{
				BufferedThang->Buffer[BufferedThang->IndexIntoBuffer] = *PlaceToGetFrom;
				PlaceToGetFrom += 1;
				BufferedThang->IndexIntoBuffer += 1;
				RequestedBytes -= 1;
			}

		return True;
	}


/* write a signed character to the file.  returns True if successful. */
MyBoolean						WriteBufferedSignedChar(BufferedOutputRec* BufferedThang,
											signed char SignedChar)
	{
		char							Buff[1];

		CheckPtrExistence(BufferedThang);
		Buff[0] = SignedChar;
		return WriteBufferedOutput(BufferedThang,1,Buff);
	}


/* write an unsigned character to the file.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedChar(BufferedOutputRec* BufferedThang,
											unsigned char UnsignedChar)
	{
		char							Buff[1];

		CheckPtrExistence(BufferedThang);
		Buff[0] = UnsignedChar;
		return WriteBufferedOutput(BufferedThang,1,Buff);
	}


/* write a signed 2's complement 16-bit short little endian.  returns True if successful */
MyBoolean						WriteBufferedSignedShortLittleEndian(BufferedOutputRec* BufferedThang,
											signed short SignedShort)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		Buff[0] = SignedShort & 0xff;
		Buff[1] = (SignedShort >> 8) & 0xff;
		return WriteBufferedOutput(BufferedThang,2,Buff);
	}


/* write a signed 2's complement 16-bit short big endian.  returns True if successful */
MyBoolean						WriteBufferedSignedShortBigEndian(BufferedOutputRec* BufferedThang,
											signed short SignedShort)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		Buff[1] = SignedShort & 0xff;
		Buff[0] = (SignedShort >> 8) & 0xff;
		return WriteBufferedOutput(BufferedThang,2,Buff);
	}


/* write an unsigned 16-bit short little endian.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedShortLittleEndian(BufferedOutputRec* BufferedThang,
											unsigned short UnsignedShort)
	{
		return WriteBufferedSignedShortLittleEndian(BufferedThang,UnsignedShort);
	}


/* write an unsigned 16-bit short big endian.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedShortBigEndian(BufferedOutputRec* BufferedThang,
											unsigned short UnsignedShort)
	{
		return WriteBufferedSignedShortBigEndian(BufferedThang,UnsignedShort);
	}


/* write a signed 2's complement 32-bit long little endian.  returns True if successful */
MyBoolean						WriteBufferedSignedLongLittleEndian(BufferedOutputRec* BufferedThang,
											signed long SignedLong)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		Buff[0] = SignedLong & 0xff;
		Buff[1] = (SignedLong >> 8) & 0xff;
		Buff[2] = (SignedLong >> 16) & 0xff;
		Buff[3] = (SignedLong >> 24) & 0xff;
		return WriteBufferedOutput(BufferedThang,4,Buff);
	}


/* write a signed 2's complement 32-bit long big endian.  returns True if successful */
MyBoolean						WriteBufferedSignedLongBigEndian(BufferedOutputRec* BufferedThang,
											signed long SignedLong)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		Buff[3] = SignedLong & 0xff;
		Buff[2] = (SignedLong >> 8) & 0xff;
		Buff[1] = (SignedLong >> 16) & 0xff;
		Buff[0] = (SignedLong >> 24) & 0xff;
		return WriteBufferedOutput(BufferedThang,4,Buff);
	}


/* write an unsigned 32-bit long little endian.  returns True if successful */
MyBoolean						WriteBufferedUnsignedLongLittleEndian(BufferedOutputRec* BufferedThang,
											unsigned long UnsignedLong)
	{
		return WriteBufferedSignedLongLittleEndian(BufferedThang,UnsignedLong);
	}


/* write an unsigned 32-bit long big endian.  returns True if successful */
MyBoolean						WriteBufferedUnsignedLongBigEndian(BufferedOutputRec* BufferedThang,
											unsigned long UnsignedLong)
	{
		return WriteBufferedSignedLongBigEndian(BufferedThang,UnsignedLong);
	}
