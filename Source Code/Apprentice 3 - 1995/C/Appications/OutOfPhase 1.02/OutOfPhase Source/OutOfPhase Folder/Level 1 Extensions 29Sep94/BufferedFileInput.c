/* BufferedFileInput.c */
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

#include "BufferedFileInput.h"
#include "Files.h"
#include "Memory.h"


#define BUFFERSIZE (4096) /* must be a power of two! */
#define BUFFERMASK (BUFFERSIZE - 1)


/* this structure is simple because we only handle sequential reading. */
struct BufferedInputRec
	{
		FileType*				FileDescriptor;
		long						IndexIntoBuffer;
		long						IndexIntoFileOfStartOfBuffer;
		long						LengthOfFile;
		MyBoolean				ErrorOcurredCantBeUsed;
		char						Buffer[BUFFERSIZE];
	};


static MyBoolean		FillBuffer(BufferedInputRec* BufferedThang)
	{
		long							NumBytesToRead;

		CheckPtrExistence(BufferedThang);
		ERROR(BufferedThang->ErrorOcurredCantBeUsed,PRERR(ForceAbort,
			"FillBuffer:  error occurred and buffer can not be used"));
		if (BufferedThang->IndexIntoFileOfStartOfBuffer + BUFFERSIZE
			> BufferedThang->LengthOfFile)
			{
				NumBytesToRead = BufferedThang->LengthOfFile
					- BufferedThang->IndexIntoFileOfStartOfBuffer;
			}
		 else
			{
				NumBytesToRead = BUFFERSIZE;
			}
		ERROR(GetFilePosition(BufferedThang->FileDescriptor)
			!= BufferedThang->IndexIntoFileOfStartOfBuffer,PRERR(ForceAbort,
			"FillBuffer:  buffer index is out of sync with the file"));
		if (0 != ReadFromFile(BufferedThang->FileDescriptor,&(BufferedThang->Buffer[0]),
			NumBytesToRead))
			{
				BufferedThang->ErrorOcurredCantBeUsed = True;
				return False;
			}
		return True;
	}


/* create a new buffered input object around a file.  the file is not allowed */
/* to be tampered with after it has been registered with this since I am too */
/* lazy to implement proper buffering in the Level 0 library like I should. */
BufferedInputRec*		NewBufferedInput(struct FileType* TheFileDescriptor)
	{
		BufferedInputRec*	BufferedThang;
		long							FilePosition;

		CheckPtrExistence(TheFileDescriptor);

		BufferedThang = (BufferedInputRec*)AllocPtrCanFail(sizeof(BufferedInputRec),
			"BufferedInputRec");
		if (BufferedThang == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		BufferedThang->FileDescriptor = TheFileDescriptor;

		BufferedThang->LengthOfFile = GetFileLength(TheFileDescriptor);

		FilePosition = GetFilePosition(TheFileDescriptor);
		BufferedThang->IndexIntoBuffer = FilePosition & BUFFERMASK;
		BufferedThang->IndexIntoFileOfStartOfBuffer = FilePosition & (~BUFFERMASK);

		BufferedThang->ErrorOcurredCantBeUsed = False;

		if (!SetFilePosition(TheFileDescriptor,BufferedThang->IndexIntoFileOfStartOfBuffer))
			{
			 FailurePoint2:
				ReleasePtr((char*)BufferedThang);
				goto FailurePoint1;
			}
		if (!FillBuffer(BufferedThang))
			{
			 FailurePoint3:
				goto FailurePoint2;
			}

		return BufferedThang;
	}


/* clean up a buffered input object from around a file.  the file may be */
/* used normally after this has been called. */
void								EndBufferedInput(BufferedInputRec* BufferedThang)
	{
		CheckPtrExistence(BufferedThang);
		if (!BufferedThang->ErrorOcurredCantBeUsed)
			{
				(void)SetFilePosition(BufferedThang->FileDescriptor,
					BufferedThang->IndexIntoBuffer + BufferedThang->IndexIntoFileOfStartOfBuffer);
			}
		ReleasePtr((char*)BufferedThang);
	}


/* read a raw block of data from the file.  returns True if all went well or */
/* False if some data could not be written to the file. */
MyBoolean						ReadBufferedInput(BufferedInputRec* BufferedThang,
											long RequestedBytes, char* PlaceToPut)
	{
		CheckPtrExistence(BufferedThang);
		ERROR(BufferedThang->ErrorOcurredCantBeUsed,PRERR(ForceAbort,
			"ReadBufferedInput:  error ocurred, so buffer is no longer valid"));
		ERROR(RequestedBytes < 0,PRERR(ForceAbort,
			"ReadBufferedInput:  bad requested bytes"));

		/* do the read loop */
		while (RequestedBytes > 0)
			{
				if (BufferedThang->IndexIntoBuffer + BufferedThang->IndexIntoFileOfStartOfBuffer
					>= BufferedThang->LengthOfFile)
					{
						/* ran out of bytes, so we'll just tell them */
						BufferedThang->ErrorOcurredCantBeUsed = True;
						return False;
					}
				*PlaceToPut = BufferedThang->Buffer[BufferedThang->IndexIntoBuffer];
				BufferedThang->IndexIntoBuffer += 1;
				PlaceToPut += 1;
				RequestedBytes -= 1;
				if (BufferedThang->IndexIntoBuffer == BUFFERSIZE)
					{
						/* if we ran off the end of the file, then load increment internal indices */
						BufferedThang->IndexIntoBuffer = 0;
						BufferedThang->IndexIntoFileOfStartOfBuffer += BUFFERSIZE;
						/* load the next block from the disk */
						if (!FillBuffer(BufferedThang))
							{
								BufferedThang->ErrorOcurredCantBeUsed = True;
								return False;
							}
					}
			}

		return True;
	}


/* read in a signed (2's complement) character.  returns True if successful. */
MyBoolean						ReadBufferedSignedChar(BufferedInputRec* BufferedThang,
											signed char* SignedCharOut)
	{
		char							Buff[1];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,1,Buff))
			{
				return False;
			}
		if ((Buff[0] & 0x80) != 0)
			{
				*SignedCharOut = ~0x7f | (Buff[0] & 0x7f);
			}
		 else
			{
				*SignedCharOut = Buff[0] & 0x7f;
			}
		return True;
	}


/* read in an unsigned character.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedChar(BufferedInputRec* BufferedThang,
											unsigned char* UnsignedCharOut)
	{
		char							Buff[1];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,1,Buff))
			{
				return False;
			}
		*UnsignedCharOut = Buff[0] & 0xff;
		return True;
	}


/* read in a signed (2's complement) 16-bit short little endian.  returns True if sucessful. */
MyBoolean						ReadBufferedSignedShortLittleEndian(BufferedInputRec* BufferedThang,
											signed short* SignedShortOut)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,2,Buff))
			{
				return False;
			}
		if ((Buff[1] & 0x80) != 0)
			{
				*SignedShortOut
					= (0xff & Buff[0])
					| ((~0x7f | ((short)Buff[1] & 0x7f)) << 8);
			}
		 else
			{
				*SignedShortOut
					= (0xff & Buff[0])
					| (((short)Buff[1] & 0x7f) << 8);
			}
		return True;
	}


/* read in a signed (2's complement) 16-bit short big endian.  returns True if sucessful. */
MyBoolean						ReadBufferedSignedShortBigEndian(BufferedInputRec* BufferedThang,
											signed short* SignedShortOut)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,2,Buff))
			{
				return False;
			}
		if ((Buff[0] & 0x80) != 0)
			{
				*SignedShortOut
					= (0xff & Buff[1])
					| ((~0x7f | ((short)Buff[0] & 0x7f)) << 8);
			}
		 else
			{
				*SignedShortOut
					= (0xff & Buff[1])
					| (((short)Buff[0] & 0x7f) << 8);
			}
		return True;
	}


/* read in an unsigned 16-bit short little endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedShortLittleEndian(BufferedInputRec* BufferedThang,
											unsigned short* UnsignedShortOut)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,2,Buff))
			{
				return False;
			}
		*UnsignedShortOut
			= (0xff & Buff[0])
			| (((short)Buff[1] & 0xff) << 8);
		return True;
	}


/* read in an unsigned 16-bit short big endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedShortBigEndian(BufferedInputRec* BufferedThang,
											unsigned short* UnsignedShortOut)
	{
		char							Buff[2];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,2,Buff))
			{
				return False;
			}
		*UnsignedShortOut
			= (0xff & Buff[1])
			| (((short)Buff[0] & 0xff) << 8);
		return True;
	}


/* read in a signed (2's complement) 32-bit long little endian.  returns True if successful. */
MyBoolean						ReadBufferedSignedLongLittleEndian(BufferedInputRec* BufferedThang,
											signed long* SignedLongOut)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,4,Buff))
			{
				return False;
			}
		if ((Buff[3] & 0x80) != 0)
			{
				*SignedLongOut
					= (0xff & Buff[0])
					| ((0xff & (long)Buff[1]) << 8)
					| ((0xff & (long)Buff[2]) << 16)
					| ((~0x7f | ((long)Buff[3] & 0x7f)) << 24);
			}
		 else
			{
				*SignedLongOut
					= (0xff & Buff[0])
					| ((0xff & (long)Buff[1]) << 8)
					| ((0xff & (long)Buff[2]) << 16)
					| (((long)Buff[3] & 0x7f) << 24);
			}
		return True;
	}


/* read in a signed (2's complement) 32-bit long big endian.  returns True if successful. */
MyBoolean						ReadBufferedSignedLongBigEndian(BufferedInputRec* BufferedThang,
											signed long* SignedLongOut)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,4,Buff))
			{
				return False;
			}
		if ((Buff[0] & 0x80) != 0)
			{
				*SignedLongOut
					= (0xff & Buff[3])
					| ((0xff & (long)Buff[2]) << 8)
					| ((0xff & (long)Buff[1]) << 16)
					| ((~0x7f | ((long)Buff[0] & 0x7f)) << 24);
			}
		 else
			{
				*SignedLongOut
					= (0xff & Buff[3])
					| ((0xff & (long)Buff[2]) << 8)
					| ((0xff & (long)Buff[1]) << 16)
					| (((long)Buff[0] & 0x7f) << 24);
			}
		return True;
	}


/* read in an unsigned 32-bit long little endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedLongLittleEndian(BufferedInputRec* BufferedThang,
											unsigned long* UnsignedLongOut)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,4,Buff))
			{
				return False;
			}
		*UnsignedLongOut
			= (0xff & Buff[0])
			| ((0xff & (long)Buff[1]) << 8)
			| ((0xff & (long)Buff[2]) << 16)
			| (((long)Buff[3] & 0xff) << 24);
		return True;
	}


/* read in an unsigned 32-bit long big endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedLongBigEndian(BufferedInputRec* BufferedThang,
											unsigned long* UnsignedLongOut)
	{
		char							Buff[4];

		CheckPtrExistence(BufferedThang);
		if (!ReadBufferedInput(BufferedThang,4,Buff))
			{
				return False;
			}
		*UnsignedLongOut
			= (0xff & Buff[3])
			| ((0xff & (long)Buff[2]) << 8)
			| ((0xff & (long)Buff[1]) << 16)
			| (((long)Buff[0] & 0xff) << 24);
		return True;
	}
