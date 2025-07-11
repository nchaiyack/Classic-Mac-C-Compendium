/*----------------------------------------------------------------------------

	filebuff.c

	This module handles buffered Macintosh file I/O.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <script.h>

#include "filebuff.h"
#include "util.h"

#define kBufferSize 8000

/*----------------------------------------------------------------------------
 * Prototypes for internal functions
 */
OSErr BufFill(BufferedFile *file);

/*----------------------------------------------------------------------------
 * BufOpen
 */
OSErr BufOpen(
	BufferedFile*	file,
	FSSpec			*theFile,
	SignedByte		perm,
	OSType			creator,
	OSType			fileType)
{
	OSErr err = noErr;
	short refNum;

	/* Set everything to safe default values */
	file->buffer = file->bufPtr = NULL;
	file->reading = file->writing = FALSE;
	file->count = file->bufSize = 0;
	file->fRefNum = 0;
	file->vRefNum = theFile->vRefNum;

	/* Allocate a buffer */
	if ((file->buffer = MyNewPtr(kBufferSize)) == NULL)
		err = MyMemErr();
	if (err == noErr) {
		file->bufPtr = file->buffer;
		file->bufSize = kBufferSize;
	}

	/* Try to open the file */
	if (err == noErr) {
		err = FSpOpenDF(theFile, perm, &refNum);
		if (err == fnfErr && (perm == fsWrPerm || perm == fsRdWrPerm
								|| perm == fsRdWrShPerm)) {
			/* If the file doesn't exist and we're opening for writing, create it */
			FSpCreate(theFile, creator, fileType, smSystemScript);
			err = FSpOpenDF(theFile, perm, &refNum);
		}
	}
	
	if (err == noErr) {
		file->fRefNum = refNum;
	}
	
	if (err != noErr)
		BufClose(file);

	return err;
}

/*----------------------------------------------------------------------------
 * BufClose
 */
OSErr BufClose(BufferedFile *file)
{
	OSErr	err = noErr;
	long	pos;

	if (file->writing) {
		/* Flush the buffer */
		err = BufFlush(file);
		if (err == noErr)
			err = GetFPos(file->fRefNum, &pos);
		if (err == noErr)
			err = SetEOF(file->fRefNum, pos);
	}

	if (file->buffer) {
		DisposPtr(file->buffer);
		file->buffer = file->bufPtr = NULL;
	}
	
	if (file->fRefNum) {
		err = FSClose(file->fRefNum);
		file->fRefNum = 0;
		if (err == noErr) {
			err = FlushVol(0, file->vRefNum);
		}
	}
	
	return err;
}

/*----------------------------------------------------------------------------
 * BufFlush
 */
OSErr BufFlush(BufferedFile *file)
{
	OSErr	err = noErr;
	long	byteCount;

	if (file->writing) {
		if (file->bufPtr > file->buffer) {
			long byteCount = file->count;
			err = FSWrite(file->fRefNum, &byteCount, file->buffer);
			if (err == noErr) {
				file->bufPtr = file->buffer;
				file->count = 0;
			}
		}
	} else if (file->reading) {
		err = paramErr;
	}
	return err;
}


/*----------------------------------------------------------------------------
 * BufFill
 * Make sure there are some characters in the buffer for reading.
 * On successful return there will be at least one character there.
 * At end of file returns eofErr.
 * Returns paramErr if file is being used for writing
 */
OSErr BufFill(BufferedFile *file)
{
	OSErr err = noErr;
	long byteCount;

	if (file->writing || !file->fRefNum) {
		err = paramErr;
	} else if (!file->count) {
		file->reading = TRUE;
		byteCount = file->bufSize;
		err = FSRead(file->fRefNum, &byteCount, file->buffer);
		if (err == eofErr && byteCount > 0) {
			/* End of File is OK as long as we read something */
			err = noErr;
		}
		if (err == noErr) {
			file->bufPtr = file->buffer;
			file->count = byteCount;
		}
	}

	return err;
}

/*----------------------------------------------------------------------------
 * BufWrite
 */
OSErr BufWrite(
	BufferedFile	*file,
	const void		*data,
	long			len)
{
	OSErr		err = noErr;
	long		toMove;
	const char*	dataPtr;

	dataPtr = data;

	if (!file->reading) {
		file->writing = TRUE;
		while (err == noErr && len > 0) {
			/* Figure out how many bytes to copy to the buffer */
			toMove = file->bufSize - file->count;
			if (toMove > len)
				toMove = len;
			/* Copy them */
			BlockMove(dataPtr, file->bufPtr, toMove);
			dataPtr += toMove;
			file->bufPtr += toMove;
			file->count += toMove;
			len -= toMove;
			/* Flush the buffer if it's full */
			if (file->count == file->bufSize) {
				err = BufFlush(file);
			}
		}
	} else {
		err = paramErr;		/* Error: writing to a file being used for reading */
	}
	return err;
}

/*----------------------------------------------------------------------------
 * BufRead
 */
OSErr BufRead(
	BufferedFile	*file,
	void			*data,
	long			len)
{
	OSErr	err = noErr;
	long	toMove;
	char	*dataPtr = data;

	while (len > 0 && err == noErr) {
		if ((err = BufFill(file)) != noErr)
			break;
		toMove = file->count;
		if (toMove > len)
			toMove = len;
		BlockMove(file->bufPtr, dataPtr, toMove);
		file->bufPtr += toMove;
		dataPtr += toMove;
		file->count -= toMove;
		len -= toMove;
	}

	return err;
}

/*----------------------------------------------------------------------------
 * BufWriteStr
 */
OSErr BufWriteStr(
	BufferedFile	*file,
	const char		*str)
{
	return BufWrite(file, str, strlen(str)+1);
}

/*----------------------------------------------------------------------------
 * BufWriteStrHdl
 */
OSErr BufWriteStrHdl(
	BufferedFile	*file,
	Handle			str)
{
	OSErr err;
	SignedByte oldState = HGetState(str);
	HLock(str);
	err = BufWriteStr(file, *str);
	HSetState(str, oldState);
	return err;
}

/*----------------------------------------------------------------------------
 * BufReadStr
 */
OSErr BufReadStr(BufferedFile *file, char *str, long maxLen)
{
	OSErr err = noErr;
	Boolean done = FALSE;

	maxLen--;	/* Leave room for NULL at end; */

	while (maxLen > 0 && !done) {
		if ((err = BufFill(file)) != noErr)
			break;
		while (maxLen > 0 && file->count > 0 && !done) {
			done = (*file->bufPtr == 0);	/* If this is a null char, we're done */
			*str++ = *file->bufPtr++;
			maxLen--;
			file->count--;
		}
	}
	*str = '\0';

	return err;
}

/*----------------------------------------------------------------------------
 * BufWriteChar
 */
OSErr BufWriteChar(BufferedFile *file, char c)
{
	OSErr err = noErr;
	
	if (file->reading) {
		err = paramErr;
	} else if (file->count >= file->bufSize) {
		/* Make room in the buffer */
		err = BufFlush(file);
	}
	
	if (err == noErr) {
		file->writing = TRUE;
		*file->bufPtr++ = c;
		file->count++;
	}
	
	return err;
}

/*----------------------------------------------------------------------------
 * BufReadChar
 */
OSErr BufReadChar(BufferedFile *file, char *c)
{
	OSErr err = BufFill(file);

	if (err == noErr) {
		*c = *file->bufPtr++;
		file->count--;
	}

	return err;
}

/*----------------------------------------------------------------------------
 * BufWriteShort
 */
OSErr BufWriteShort(BufferedFile *file, short s)
{
	return BufWrite(file, &s, sizeof(s));
}

/*----------------------------------------------------------------------------
 * BufReadShort
 */
OSErr BufReadShort(BufferedFile *file, short *s)
{
	return BufRead(file, s, sizeof(*s));
}

/*----------------------------------------------------------------------------
 * BufWriteULong
 */
OSErr BufWriteULong(BufferedFile *file, unsigned long l)
{
	return BufWrite(file, &l, sizeof(l));
}

/*----------------------------------------------------------------------------
 * BufReadULong
 */
OSErr BufReadULong(BufferedFile *file, unsigned long *l)
{
	return BufRead(file, l, sizeof(*l));
}

