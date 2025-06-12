#define FILE_NUM 20
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Lib
static UHandle Buffer=nil;	 /* where our characters go */
static long BufferSize; 		/* how big our buffer is */
static long BFilled=0;			/* the number of chars in the buffer */
static short RefN=0;					/* the path number of the open file */
static long BSpot;					/* index of next character to transfer */
static long LastSpot; 			/* the file position of beginning of
																the last line read */
static long FSpot;					/* the position in the file of the start
																of the buffer */
																
#define LIKE_BUFFER 4096

short OpenLine(short vRef,long dirId,UPtr name)
{
	int err;
	
	CloseLine();
		
	/*
	 * open the file
	 */
	if (err=FSHOpen(name,vRef,dirId,&RefN,fsRdWrPerm)) goto failure;
	
	/*
	 * allocate a buffer
	 */
	GetEOF(RefN,&BufferSize);
	BufferSize += 2;
	if (BufferSize > LIKE_BUFFER) BufferSize = LIKE_BUFFER;
	if ((Buffer=NuHandle(BufferSize))==nil)
	{
		err=MemError();
		goto failure;
	}
	
	/*
	 * fill the first buffer
	 */
	BFilled = BufferSize-1;
	LDRef(Buffer);
	err=FSRead(RefN,&BFilled,*Buffer);
	UL(Buffer);
	if (err && err!=eofErr) goto failure;
	BSpot = LastSpot = FSpot = 0;
	(*Buffer)[BFilled] = '\n';			/* a marker, to expedite searches */
	return(noErr);

failure:
	CloseLine();
	return(err);
}

/************************************************************************
 * SeekLine - seek the line routines to a given spot
 ************************************************************************/
int SeekLine(long spot)
{
	int err;
	
	if (err = SetFPos(RefN,fsFromStart,spot)) goto failure;
	
	/*
	 * fill the first buffer
	 */
	BFilled = BufferSize-1;
	LDRef(Buffer);
	err=FSRead(RefN,&BFilled,*Buffer);
	UL(Buffer);
	if (err && err!=eofErr) goto failure;
	BSpot = 0;
	LastSpot = FSpot = spot;
	(*Buffer)[BFilled] = '\n';			/* a marker, to expedite searches */
	return(noErr);

failure:
	CloseLine();
	return(err);
}
/**********************************************************************
 * GetLine - read a line of a given size.  returns 0 for eof, negative
 * for file manager errors, LINE_START if returning the beginning of
 * a line, LINE_MIDDLE if a partial line is being returned.
 **********************************************************************/
int GetLine(UPtr line,int size)
{
	register UPtr bp;
	register UPtr cp=line;
	int where;
	int err;
	static FILE *trace=0;
	
	if (!BFilled) return (0); 		/* we have no chars */
	CycleBalls();
	bp = LDRef(Buffer) + BSpot;
	where = (bp==*Buffer || bp[-1]=='\n') ? LINE_START : LINE_MIDDLE;
	LastSpot = FSpot + BSpot; 	/* remember where this line begins */
	for (;;)
	{
		while (*bp!='\n' && --size>0) *cp++ = *bp++;
		BSpot = bp - *Buffer;
		if (BSpot==BFilled)
		{
			FSpot += BFilled;
			BFilled = BufferSize - 1;
			err=FSRead(RefN,&BFilled,*Buffer);
			if (err==eofErr)
			{
				if (BFilled==0)
				{
					*cp = 0;
					BFilled = 0;
					UL(Buffer);
					return (where);
				}
			}
			else if (err)
			{
				UL(Buffer);
				FileSystemError(READ_MBOX,"",err);
				return(err);
			}
			(*Buffer)[BFilled] = '\n';	/* a marker, to expedite searches */
			BSpot = 0;
			bp = *Buffer;
		}
		else
		{
			if (size>0)
			{
				*cp++ = '\n';
				BSpot++;
			}
			*cp = 0;
			UL(Buffer);
			return(where);
		}
	}
}

/**********************************************************************
 * CloseLine - shut up shop.	Calling it on closed file does no harm.
 **********************************************************************/
void CloseLine(void)
{
	if (RefN != 0)
	{
		FSClose(RefN);
		RefN = 0;
	}
	if (Buffer != nil)
	{
		DisposHandle(Buffer);
		Buffer = nil;
	}
	BFilled = 0;
}

long TellLine(void)
{
	return (LastSpot);
} 

