/*

BufferedFile.c

*/

struct BufferedFile_S {
    long                            errval;
    short                           FileNum;
    EString_t                       buffer;
    unsigned long                   buffersize;
    unsigned long                   bufferindex;
    unsigned long                   Fullness;
};
typedef struct BufferedFile_S   BufferedFile_t;
typedef BufferedFile_t P__H    *BufferedFileVia_t;

long                            FillFileBuffer(BufferedFileVia_t thef);
void                            FlushFileBuffer(BufferedFileVia_t thef);
 char                   GetBufChar(MemBufVia_t theb);
 short                  GetBufWord(MemBufVia_t theb);
 long                   GetBufLong(MemBufVia_t theb);
void                            PutBufChar(MemBufVia_t theb,  char c);
void                            PutBufWord(MemBufVia_t theb,  short wd);
void                            PutBufLong(MemBufVia_t theb,  long lg);
 char                   GetFileChar(BufferedFileVia_t thef);
void                            PutFileChar(BufferedFileVia_t thef,  char c);
void                            PutFileWord(BufferedFileVia_t thef,  short wd);
void                            PutFileLong(BufferedFileVia_t thef,  long wd);
BufferedFileVia_t               OpenFile(char *name, short volrefnum, long dirID, char *mode);
void                            CloseFile(BufferedFileVia_t thef);
 short                  GetFileWord(BufferedFileVia_t thef);
 long                   GetFileLong(BufferedFileVia_t thef);
long
FillFileBuffer(BufferedFileVia_t thef)
/* For input files, this fills the buffer by reading the file */
{
    long                            count;
    #ifdef OLDMEM
    HLock((Handle) Via(thef)->buffer);
    #endif
    count = Via(thef)->buffersize;
    Via(thef)->errval = FSRead(Via(thef)->FileNum, &count, (char *) Via(Via(thef)->buffer));
    Via(thef)->Fullness = count;
    Via(thef)->bufferindex = 0;
    #ifdef OLDMEM
    HUnlock((Handle) Via(thef)->buffer);
    #endif
    return count;
}

void
FlushFileBuffer(BufferedFileVia_t thef)
/* For output files, this dumps the contents of the buffer */
{
    long                            count;
    #ifdef OLDMEM
    HLock((Handle) Via(thef)->buffer);
    #endif
    count = Via(thef)->bufferindex;
    Via(thef)->errval = FSWrite(Via(thef)->FileNum, &count, (char *) Via(Via(thef)->buffer));
    Via(thef)->bufferindex = 0;
    #ifdef OLDMEM
    HUnlock((Handle) Via(thef)->buffer);
    #endif
}

 char
GetBufChar(MemBufVia_t theb)
{
    return Via(Via(theb)->buf)[Via(theb)->ndx++];
}

 short
GetBufWord(MemBufVia_t theb)
{
     short                  wd;
    wd = GetBufChar(theb) << 8;
    wd |= GetBufChar(theb);
    return wd;
}

 long
GetBufLong(MemBufVia_t theb)
{
     long                   lg;
    lg = GetBufWord(theb) << 16;
    lg |= GetBufWord(theb);
    return lg;
}

void
PutBufChar(MemBufVia_t theb,  char c)
{
    Via(Via(theb)->buf)[Via(theb)->ndx++] = c;
}

void
PutBufWord(MemBufVia_t theb,  short wd)
{
    PutBufChar(theb, wd >> 8);
    PutBufChar(theb, wd);
}

void
PutBufLong(MemBufVia_t theb,  long lg)
{
    PutBufWord(theb, lg >> 16);
    PutBufWord(theb, lg);
}

 char
GetFileChar(BufferedFileVia_t thef)
{
    if (Via(thef)->bufferindex >= Via(thef)->Fullness)
	FillFileBuffer(thef);
    if (!Via(thef)->Fullness)
	Via(thef)->errval = -1;
    return Via(Via(thef)->buffer)[Via(thef)->bufferindex++];
}

void
PutFileChar(BufferedFileVia_t thef,  char c)
{
    Via(Via(thef)->buffer)[Via(thef)->bufferindex++] = c;
    if (Via(thef)->bufferindex >= Via(thef)->buffersize)
	FlushFileBuffer(thef);
}

void
PutFileWord(BufferedFileVia_t thef,  short wd)
{
    PutFileChar(thef, wd >> 8);
    PutFileChar(thef, wd);
}

void
PutFileLong(BufferedFileVia_t thef,  long wd)
{
    PutFileWord(thef, wd >> 16);
    PutFileWord(thef, wd);
}

BufferedFileVia_t
OpenFile(char *name, short volrefnum, long dirID, char *mode)
{
    /* name coming in must be a C string */
    BufferedFileVia_t               raw;
    Str255                          pname;

    raw = Ealloc(sizeof(BufferedFile_t));
    strcpy((char *) pname, name);
    c2pstr(pname);
    if (!strcmp(mode, "w")) {
	HDelete(volrefnum, dirID, pname);
	HCreate(volrefnum, dirID, pname, MakeOSType("Jn15"), MakeOSType("TEXT"));
	Via(raw)->errval = HOpen(volrefnum, dirID, pname, 2, &(Via(raw)->FileNum));
    } else {
	Via(raw)->errval = HOpen(volrefnum, dirID, pname, 1, &(Via(raw)->FileNum));
    }
    if (Via(raw)->errval)
	return NULL;
    Via(raw)->buffersize = 10000;
    Via(raw)->buffer = Ealloc(Via(raw)->buffersize);
    Via(raw)->bufferindex = 0;
    Via(raw)->errval = 0;
    FillFileBuffer(raw);
    return raw;
}

void
CloseFile(BufferedFileVia_t thef)
{
    FlushFileBuffer(thef);
    FSClose(Via(thef)->FileNum);
    Efree(Via(thef)->buffer);
    Efree(thef);
}

 short
GetFileWord(BufferedFileVia_t thef)
{
    int                             wd;
    wd = GetFileChar(thef);
    wd = (wd << 8) | GetFileChar(thef);
    return wd;
}

 long
GetFileLong(BufferedFileVia_t thef)
{
     long                   lg;
    lg = GetFileWord(thef);
    lg = (lg << 16) | GetFileWord(thef);
    return lg;
}

