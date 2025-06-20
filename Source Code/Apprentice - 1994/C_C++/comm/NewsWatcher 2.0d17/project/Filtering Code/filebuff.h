/*----------------------------------------------------------
#
#	NewsWatcher	- Macintosh NNTP Client Application
#
#-----------------------------------------------------------
#
#	filebuff.h
#
#	This module handles buffered Macintosh file I/O.
#
#	John Werner - 11/92
#
#
#-----------------------------------------------------------*/

typedef struct {
	char *	buffer;
	char *	bufPtr;
	long	bufSize;
	long	count;
	short	fRefNum;
	short	vRefNum;
	Boolean	reading;
	Boolean	writing;
} BufferedFile;

OSErr BufOpen(BufferedFile* bFile, FSSpec *theFile,
				SignedByte perm, OSType creator, OSType fileType);
OSErr BufClose(BufferedFile *bFile);
OSErr BufFlush(BufferedFile *bFile);

OSErr BufWrite(BufferedFile *bFile, const void *data, long len);
OSErr BufRead(BufferedFile *bFile, void *data, long len);

OSErr BufWriteStr(BufferedFile *bFile, const char *str);
OSErr BufWriteStrHdl(BufferedFile *bFile, Handle str);
OSErr BufReadStr(BufferedFile *bFile, char *str, long maxLen);

OSErr BufWriteShort(BufferedFile *bFile, short s);
OSErr BufReadShort(BufferedFile *bFile, short *s);

OSErr BufWriteULong(BufferedFile *bFile, unsigned long s);
OSErr BufReadULong(BufferedFile *bFile, unsigned long *s);

OSErr BufWriteChar(BufferedFile *bFile, char c);
OSErr BufReadChar(BufferedFile *bFile, char *c);
