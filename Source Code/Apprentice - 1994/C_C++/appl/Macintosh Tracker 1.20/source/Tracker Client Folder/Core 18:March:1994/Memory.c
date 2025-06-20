/* Memory.c */

#define COMPILING_MEMORY_C
#include "Memory.h"
#include "MiscInfo.h"
#include "Compatibility.h"

#define EnableHeapChecking (False)

#define MemoryFillPattern (0x81)

#if EnableHeapChecking
	#define HEAPCHECK(plop) CheckHeap(plop)
#else
	#define HEAPCHECK(plop)
#endif

#if __option(mc68020) /* { */
	#define CodeFor68020
#else /* }{ */
	#define CodeFor68000
#endif /* } */

/* these allow us to make sure everything that gets allocated also gets deleted */
EXECUTE(long NumHandlesAllocated = 0;)
EXECUTE(long NumPointersAllocated = 0;)
EXECUTE(long MaxNumHandlesAllocated = 0;)
EXECUTE(long MaximumStackSize = 0;)
EXECUTE(char* TopOfStack;)

/* the following macros should be defined in MiscInfo.h */
/* PRIMARYMEMCACHESIZE:  number of bytes in the primary memory cache */
/* SECONDARYMEMCACHESIZE:  number of bytes in the secondary mem cache */
/* GRABTEMPMEM:  if True, then the memory manager grabs any temporary memory */
/*  that might be available if the first allocation fails.  If false, the program */
/*  waits until no memory can be removed from the local heap before using temp mem. */
/* USETEMPMEM:  if True, the program uses temporary memory.  if False, it won't */
/*  even try to allocate temporary memory. */

/* number of grow zone functions that can be installed */
#define MaxNumGrowZones (4)


/* handle preserving stash of emergency memory (NIL = invalid) */
static Handle			MemCache1 = NIL;
static Handle			MemCache2 = NIL;
static MyBoolean	MemCacheRestoreFlag = True;

static MyBoolean	GrowZoneInstalled = False;
static MyBoolean	DisableDefaultGrowZone = False;
static GrowZoneFuncPtrType	GrowZoneList[MaxNumGrowZones];
static short			NumGrowZones = 0;
static MyBoolean	GrowZoneInProgress = False;

static MyBoolean	AllocCanFail = False;

#ifdef MEMDEBUG /* { */
	#define IsANothing (0)
	#define IsAHandle (1)
	#define IsAPointer (2)
	#define NUMTAGCHARS (20)
	typedef struct StoreRec
		{
			void*		HandPtr; /* whatever it is (4 chars) */
			short		Type; /* handle or pointer (2 chars) */
			char		Tag[NUMTAGCHARS]; /* string for taging */
			struct StoreRec**	Next;
		} StoreRec;
	StoreRec**			TrackList;
	static short		MemDumpFile;
	void		RegisterHandle(Handle TheHandle);
	void		DeregisterHandle(Handle TheHandle);
	void		RegisterPointer(Ptr ThePointer);
	void		DeregisterPointer(Ptr ThePointer);
	void		SetUpChecking(void);
	void		ShutOffChecking(void);
	void		MemPrint(char* Str,...);
#else /* }{ */
	#define RegisterHandle(TheHandle)
	#define DeregisterHandle(TheHandle)
	#define RegisterPointer(ThePointer)
	#define DeregisterPointer(ThePointer)
	#define SetUpChecking()
	#define ShutOffChecking()
#endif /* } */


/* prototypes */
static pascal long		GlobalGrowZone(ulong BytesNeeded);


/* attempt to allocate handle */
Handle		AllocHandle(ulong Size)
	{
		Handle			Temp;
		OSErr				Err;

		HEAPCHECK();
		StackSizeTest();
		ERROR(Size >= 0x00800000,
			PRERR(AllowResume,"Allocating Handle larger than 8 Megabytes."));
		/* this prevents a purge of our zone allowing us to allocate space from temporary */
		/* memory before we use the stash */
		DisableDefaultGrowZone = True; /* prevent grow zone from purging stash. */
		/* if the cache exists, we try to allocate from our heap first.  Otherwise, we */
		/* try to allocate from temporary memory, in order to preserve as much of our */
		/* memory as we can.  Other grow zone functions still work (e.g. CDiskCache purge) */
		#if GRABTEMPMEM && USETEMPMEM /* { */
		if ((MemCache1 == NIL) && HasGoodTemporaryMemory)
			{
				Temp = TempNewHandle(Size,&Err);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				if (Temp != NIL) goto GoodHandle;
			}
		#endif /* } */
		Temp = NewHandle(Size);
		ERROR((MemErr != noErr) && (MemErr != memFullErr),
			PRERR(ForceAbort,"Memory manager error"));
		if (Temp != NIL) goto GoodHandle;
		#if USETEMPMEM /* { */
		if (HasGoodTemporaryMemory)
			{
				Temp = TempNewHandle(Size,&Err);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				if (Temp != NIL) goto GoodHandle;
			}
		#endif /* } */
		if (MemCache1 != NIL)
			{
				DisposHandle(MemCache1);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				MemCache1 = NIL;
				Temp = NewHandle(Size);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				if (Temp != NIL) goto GoodHandle;
			}
		if (MemCache2 != NIL)
			{
				DisposHandle(MemCache2);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				MemCache2 = NIL;
				Temp = NewHandle(Size);
				ERROR((MemErr != noErr) && (MemErr != memFullErr),
					PRERR(ForceAbort,"Memory manager error"));
				if (Temp != NIL) goto GoodHandle;
			}
		if ((Temp == NIL) && (!AllocCanFail))
			{
				PRERR(ForceAbort,"AllocHandle couldn't allocate any memory.");
			}
	 GoodHandle:
		DisableDefaultGrowZone = False;  /* reenable stash raiding */
		EXECUTE(if (Temp != NIL) NumHandlesAllocated += 1);
		EXECUTE(if (NumHandlesAllocated > MaxNumHandlesAllocated)
			MaxNumHandlesAllocated = NumHandlesAllocated);
		EXECUTE(if (Temp != NIL) {RegisterHandle(Temp);})
		EXECUTE(if (Temp != NIL) {long Scan; for (Scan = 0; Scan < Size; Scan += 1)
			{(*Temp)[Scan] = MemoryFillPattern;}})
		return Temp;
	}


Handle		AllocHandleCanFail(ulong Size)
	{
		Handle	Temp;

		AllocCanFail = True;
		Temp = AllocHandle(Size);
		AllocCanFail = False;
		return Temp;
	}


/* attempt to allocate a pointer */
Ptr				AllocPtr(ulong Size)
	{
		Ptr				Temp;

		HEAPCHECK();
		StackSizeTest();
		ERROR(Size > 0x00800000,
			PRERR(AllowResume,"Allocating Pointer larger than 8 Megabytes."));
		Temp = NewPtr(Size); /* grow zone function automatically raids MemCache if necessary */
		ERROR((MemErr != noErr) && (MemErr != memFullErr),
			PRERR(ForceAbort,"Memory manager error"));
		if ((Temp == NIL) && (!AllocCanFail))
			{
				PRERR(ForceAbort,"CMemory::AllocPtr couldn't allocate any memory.");
			}
		EXECUTE(NumPointersAllocated += 1);
		EXECUTE(if (Temp != NIL) {RegisterPointer(Temp);})
		EXECUTE(if (Temp != NIL) {long Scan; for (Scan = 0; Scan < Size; Scan += 1)
			{Temp[Scan] = MemoryFillPattern;}})
		return Temp;
	}


Ptr				AllocPtrCanFail(ulong Size)
	{
		Ptr		Temp;

		AllocCanFail = True;
		Temp = AllocPtr(Size);
		AllocCanFail = False;
		return Temp;
	}


/* release handle and attempt to restore cache */
void			ReleaseHandle(Handle TheHand)
	{
		StackSizeTest();
		ERROR(TheHand==NIL,PRERR(ForceAbort,"ReleaseHandle tried to dispose a NIL handle."));
		DeregisterHandle(TheHand);
		DisposHandle(TheHand);
		EXECUTE(NumHandlesAllocated -= 1);
		if (((MemCache1 == NIL) || (MemCache2 == NIL)) && (!GrowZoneInProgress)
			&& MemCacheRestoreFlag)
			{
				RestoreMemCache();
			}
		HEAPCHECK();
	}


/* release pointer and attempt to restore cache */
void			ReleasePtr(Ptr ThePtr)
	{
		StackSizeTest();
		ERROR(ThePtr==NIL,PRERR(ForceAbort,"ReleasePtr tried to dispose a NIL pointer."));
		DeregisterPointer(ThePtr);
		DisposPtr(ThePtr);
		ERROR((MemErr != noErr) && (MemErr != memFullErr),
			PRERR(ForceAbort,"Memory manager error"));
		EXECUTE(NumPointersAllocated -= 1);
		if (((MemCache1 == NIL) || (MemCache2 == NIL)) && (!GrowZoneInProgress)
			&& MemCacheRestoreFlag)
			{
				RestoreMemCache();
			}
		HEAPCHECK();
	}


#ifdef DEBUG /* { */

#pragma parameter __D0 LocalHandleSize(__A0)
pascal long LocalHandleSize(Handle h) = 0xA025;
#pragma parameter __D0 LocalPtrSize(__A0)
pascal long LocalPtrSize(Ptr p) = 0xA021;

ulong			HandleSize(Handle h)
	{
		long		Result;

		CheckHandleExistence(h);
		if ((Result = LocalHandleSize(h)) < 0)
			{
				PRERR(AllowResume,"HandleSize returned negative value.");
			}
		ERROR((MemErr != noErr) && (MemErr != memFullErr),
			PRERR(ForceAbort,"Memory manager error"));
		return Result;
	}


ulong			PtrSize(Ptr p)
	{
		long		Result;

		CheckPtrExistence(p);
		if ((Result = LocalPtrSize(p)) < 0)
			{
				PRERR(AllowResume,"PtrSize returned negative value.");
			}
		ERROR((MemErr != noErr) && (MemErr != memFullErr),
			PRERR(ForceAbort,"Memory manager error"));
		return Result;
	}

#endif /* } */


/* see if the primary cache exists (True == it does) */
MyBoolean	FirstMemCacheValid(void)
	{
		StackSizeTest();
		if ((MemCache1 == NIL) && MemCacheRestoreFlag)
			{
				RestoreMemCache(); /* try to restore it */
			}
		return (MemCache1 != NIL);
	}


/* see if the secondary cache exists (True == it does) */
MyBoolean	SecondMemCacheValid(void)
	{
		StackSizeTest();
		if ((MemCache2 == NIL) && MemCacheRestoreFlag)
			{
				RestoreMemCache(); /* try to restore it */
			}
		return (MemCache2 != NIL);
	}


/* attempt to restore the emergency cache */
void			RestoreMemCache(void)
	{
		MyBoolean			GrowZoneDisableSaveStat;

		StackSizeTest();
		GrowZoneDisableSaveStat = DisableDefaultGrowZone;
		DisableDefaultGrowZone = True;
		if (!GrowZoneInProgress)
			{
				if (MemCache2 == NIL)
					{
						MemCache2 = NewHandle(SECONDARYMEMCACHESIZE);
						ERROR((MemErr != noErr) && (MemErr != memFullErr),
							PRERR(ForceAbort,"Memory manager error"));
					}
				if ((MemCache2 != NIL) && (MemCache1 == NIL))
					{
						MemCache1 = NewHandle(PRIMARYMEMCACHESIZE);
						ERROR((MemErr != noErr) && (MemErr != memFullErr),
							PRERR(ForceAbort,"Memory manager error"));
					}
			}
		DisableDefaultGrowZone = GrowZoneDisableSaveStat;
	}


/* this lets you set whether or not ReleasePtr and ReleaseHandle should try to */
/* restore the memcaches if they have been used. */
void			SetMemCacheRestore(MyBoolean Flag)
	{
		StackSizeTest();
		MemCacheRestoreFlag = Flag;
	}


#pragma options(!mc68020) /* this code works no matter what */

/* initialize the cache */
void			InitMemory(void)
	{
		static MyBoolean	Done = False;
		long			Count;
		char			SillyVariable;

		/* this stuff is removed because it might cause bad things to happen. */
//	SetTopOfStack(&SillyVariable);
//	SetApplLimit(&SillyVariable - STACKSIZE);
		MaxApplZone();

		ERROR(Done,PRERR(ForceAbort,"InitMemory called more than once."));
		RestoreMemCache();
		if (!GrowZoneInstalled)
			{
				SetGrowZone((ProcPtr)&GlobalGrowZone);
				GrowZoneInstalled = True;
			}
		/* NumMasters should be in the MiscInfo.h file */
		for (Count = NumMasters - 1; Count >= 0; Count -= 1)
			{
				MoreMasters();
			}
		SetUpChecking();
		Done = True;
	}


void			FlushMemory(void)
	{
		static MyBoolean	Done = False;

		ERROR(Done,PRERR(ForceAbort,"FlushMemory called more than once."));
		ERROR(NumHandlesAllocated!=0,PRERR(AllowResume,
			"Some handles are still allocated just before quitting."));
		ERROR(NumPointersAllocated!=0,PRERR(AllowResume,
			"Some pointers are still allocated just before quitting."));
		ShutOffChecking();
		Done = True;
	}

#ifdef CodeFor68020 /* { */
	#pragma options(mc68020) /* turn it back on */
#endif /* } */


/* install a new grow zone function */
void			InstallGrowZone(GrowZoneFuncPtrType MyGrowZone)
	{
		StackSizeTest();
		ERROR(NumGrowZones >= MaxNumGrowZones,
			PRERR(AllowResume,"Attempt to allocate too many grow zone functions."));
		GrowZoneList[NumGrowZones++] = MyGrowZone;
	}

#define SIGVAL1 ('This')
#define SIGVAL2 (' is ')
#define SIGVAL3 ('my s')
#define SIGVAL4 ('ig!!')
EXECUTE(long			Signature1 = SIGVAL1;)
EXECUTE(long			Signature2 = SIGVAL2;)
EXECUTE(long			Signature3 = SIGVAL3;)
EXECUTE(long			Signature4 = SIGVAL4;)

/* grow zone that gets invoked */
static pascal long		GlobalGrowZone(ulong BytesNeeded)
	{
		long		AccumulatedFreeSpace;
		short		Scan;
		long		TemporaryA5;

		TemporaryA5 = SetCurrentA5();

		StackSizeTest();

		/* test to see if A5 is set up properly */
		ERROR((SIGVAL1 != Signature1) || (SIGVAL2 != Signature2)
			|| (SIGVAL3 != Signature3) || (SIGVAL4 != Signature4),Debugger());

		GrowZoneInProgress = True;
		Scan = 0;
		AccumulatedFreeSpace = 0;
		while ((Scan < NumGrowZones) && (AccumulatedFreeSpace < BytesNeeded))
			{
				ERROR((Scan >= NumGrowZones),PRERR(ForceAbort,
					"GlobalGrowZone very strange error occurred."));
				AccumulatedFreeSpace
					+= (*(GrowZoneList[Scan]))(BytesNeeded - AccumulatedFreeSpace);
				Scan += 1;
			}
		if ((AccumulatedFreeSpace < BytesNeeded) && (!DisableDefaultGrowZone)
			&& (MemCache1 != NIL))
			{
				/* if calling all installed grow zones still couldn't free enough memory */
				/* and we are allowed to empty the stash, we'll do it */
				DisposHandle(MemCache1);
				MemCache1 = NIL;
				AccumulatedFreeSpace += PRIMARYMEMCACHESIZE;
			}
		if ((AccumulatedFreeSpace < BytesNeeded) && (!DisableDefaultGrowZone)
			&& (MemCache2 != NIL))
			{
				DisposHandle(MemCache2);
				MemCache2 = NIL;
				AccumulatedFreeSpace += SECONDARYMEMCACHESIZE;
			}
		GrowZoneInProgress = False;

		SetA5(TemporaryA5);

		return AccumulatedFreeSpace; /* this is how much we managed to dig up */
	}



#ifdef MEMDEBUG /* { */

#pragma options(!mc68020) /* this code works no matter what */


Handle	DupSysHandle(Handle Original)
	{
		Handle		Temp;

		if (Original == NIL)
			{
				Temp = AllocHandle(0); /* return an empty handle */
				SetTag(Temp,"DupSysHandle Result");
				return Temp;
			}
		Temp = AllocHandle(LocalHandleSize(Original));
		SetTag(Temp,"DupSysHandle Result");
		MemCpy(*Temp,*Original,LocalHandleSize(Original));
		return Temp;
	}


void			RegisterHandle(Handle TheHandle)
	{
		StoreRec**	Temp;

		Temp = (StoreRec**)NewHandle(sizeof(StoreRec));
		(**Temp).Next = TrackList;
		TrackList = Temp;
		(**Temp).Type = IsAHandle;
		(**Temp).HandPtr = TheHandle;
		(**Temp).Tag[0] = 0;
	}


void			DeregisterHandle(Handle TheHandle)
	{
		StoreRec**	Scan;
		StoreRec**	Lag;
		Handle			Temp;

		Scan = TrackList;
		Lag = NIL;
		while ((Scan != NIL) && ((**Scan).HandPtr != TheHandle))
			{
				Lag = Scan;
				Scan = (**Scan).Next;
			}
		if (Scan == NIL)
			{
				PRERR(ForceAbort,"Deletion of nonexistent handle.");
			}
		if ((**Scan).Type != IsAHandle)
			{
				PRERR(ForceAbort,"Deletion of Handle via ReleasePtr.");
			}
		if (Lag == NIL)
			{
				Temp = (Handle)TrackList;
				TrackList = (**Scan).Next;
				DisposHandle(Temp);
			}
		 else
			{
				Temp = (Handle)(**Lag).Next;
				(**Lag).Next = (**Scan).Next;
				DisposHandle(Temp);
			}
	}


void			RegisterPointer(Ptr ThePointer)
	{
		StoreRec**	Temp;

		Temp = (StoreRec**)NewHandle(sizeof(StoreRec));
		(**Temp).Next = TrackList;
		TrackList = Temp;
		(**Temp).Type = IsAPointer;
		(**Temp).HandPtr = ThePointer;
		(**Temp).Tag[0] = 0;
	}


void			DeregisterPointer(Ptr ThePointer)
	{
		StoreRec**	Scan;
		StoreRec**	Lag;
		Handle			Temp;

		Scan = TrackList;
		Lag = NIL;
		while ((Scan != NIL) && ((**Scan).HandPtr != ThePointer))
			{
				Lag = Scan;
				Scan = (**Scan).Next;
			}
		if (Scan == NIL)
			{
				PRERR(ForceAbort,"Deletion of nonexistent pointer.");
			}
		if ((**Scan).Type != IsAPointer)
			{
				PRERR(ForceAbort,"Deletion of Pointer via ReleaseHandle.");
			}
		if (Lag == NIL)
			{
				Temp = (Handle)TrackList;
				TrackList = (**Scan).Next;
				DisposHandle(Temp);
			}
		 else
			{
				Temp = (Handle)(**Lag).Next;
				(**Lag).Next = (**Scan).Next;
				DisposHandle(Temp);
			}
	}


void			SetTag(void* TheRef, char* TheTag)
	{
		StoreRec**	Scan;
		short				TagScan;

		Scan = TrackList;
		while ((Scan != NIL) && ((**Scan).HandPtr != TheRef))
			{
				Scan = (**Scan).Next;
			}
		if (Scan == NIL)
			{
				PRERR(ForceAbort,"Handle/Ptr couldn't be found by SetTag.");
			}
		 else
			{
				for (TagScan = 0; TagScan < NUMTAGCHARS; TagScan += 1)
					{
						(**Scan).Tag[TagScan] = TheTag[TagScan];
					}
			}
	}


void		CheckHandleExistence(Handle TheHandle)
	{
		StoreRec**		Scan;

		StackSizeTest();
		Scan = TrackList;
		while (Scan != NIL)
			{
				if (((**Scan).Type == IsAHandle) && ((**Scan).HandPtr == TheHandle))
					{
						return;
					}
				Scan = (**Scan).Next;
			}
		PRERR(ForceAbort,"Undefined Handle used.");
	}


void		CheckPtrExistence(Ptr ThePointer)
	{
		StoreRec**		Scan;

		StackSizeTest();
		Scan = TrackList;
		while (Scan != NIL)
			{
				if (((**Scan).Type == IsAPointer) && ((**Scan).HandPtr == ThePointer))
					{
						return;
					}
				Scan = (**Scan).Next;
			}
		PRERR(ForceAbort,"Undefined Handle used.");
	}


void			SetUpChecking(void)
	{
		TrackList = NIL;
	}

void			ShutOffChecking(void)
	{
		char				DumpName[] = {"\p MemCheck Still Allocated Dump"};
		StoreRec**	Scan;
		char				TagTemp[NUMTAGCHARS+1];
		short				TagScan;
		short				VRefNum;

		FSDelete((unsigned char*)DumpName,0);
		ERROR(Create((unsigned char*)DumpName,0,AUDITCREATOR,'TEXT') != noErr,
			PRERR(ForceAbort,"Memory's ShutOffChecking couldn't create dump file."));
		ERROR(FSOpen((unsigned char*)DumpName,0,&MemDumpFile) != noErr,PRERR(ForceAbort,
			"Memory's ShutOffChecking couldn't open dump file for writing."));
		MemPrint("These handles and pointers are still allocated:");
		Scan = TrackList;
		while (Scan != NIL)
			{
				for (TagScan = NUMTAGCHARS - 1; TagScan >= 0; TagScan -= 1)
					{
						TagTemp[TagScan] = (**Scan).Tag[TagScan];
					}
				TagTemp[NUMTAGCHARS] = 0;
				switch ((**Scan).Type)
					{
						case IsAHandle:
							MemPrint("Handle %x '%t'",(**Scan).HandPtr,TagTemp);
							break;
						case IsAPointer:
							MemPrint("Pointer %x '%t'",(**Scan).HandPtr,TagTemp);
							break;
					}
				Scan = (**Scan).Next;
			}
		GetVRefNum(MemDumpFile,&VRefNum);
		FSClose(MemDumpFile);
		FlushVol("\p",VRefNum);
	}


/* some va_args crud for MemPrint */
typedef void *va_list;
#define __va(arg)				&arg + 1
#define va_start(p, arg)		p = __va(arg)
#define va_arg(p, type)			*(* (type **) &p)++
#define va_end(p)

/* I lifted this from Audit.c */
/* this prints a string in the same way that printf does.  it accepts these options: */
/* %x = hexadecimal long */
/* %t = C String (text) */
#define BUFSIZE (256)
void			MemPrint(char* Str,...)
	{
		va_list		pa;
		char			Buffer[BUFSIZE];
		long			BufPtr;
		static char	Hex[16] = {"0123456789abcdef"};

		BufPtr = 0;
		va_start(pa,Str);
		while (*Str != 0)
			{
				if (*Str == '%')
					{
						Str += 1;
						switch (*Str)
							{
								case 'x':
									{
										char		Buf[9];
										short		Count;
										long		Num;

										Num = va_arg(pa,long);
										for (Count = 8; Count >= 1; Count -= 1)
											{
												Buf[Count] = Hex[Num & 0x0000000f];
												Num = Num >> 4;
											}
										Buf[0] = '$';
										for (Count = 0; Count < 9; Count += 1)
											{
												Buffer[BufPtr++] = Buf[Count];
												ERROR(BufPtr>=BUFSIZE,PRERR(ForceAbort,
													"MemPrint buffer overrun."));
											}
									}
									break;
								case 't':
									{
										char*		Strp;

										Strp = va_arg(pa,char*);
										while (*Strp != 0)
											{
												Buffer[BufPtr++] = *(Strp++);
											}
										ERROR(BufPtr>=BUFSIZE,PRERR(ForceAbort,"MemPrint buffer overrun."));
									}
									break;
							}
						Str += 1;
					}
				 else
					{
						Buffer[BufPtr++] = *(Str++);
						ERROR(BufPtr>=BUFSIZE,PRERR(ForceAbort,"MemPrint buffer overrun."));
					}
			}
		Buffer[BufPtr++] = 0x0d;
		FSWrite(MemDumpFile,&BufPtr,Buffer);
	}

#ifdef CodeFor68020 /* { */
	#pragma options(mc68020) /* turn it back on if necessary */
#endif /* } */


#else /* }{  this next part is "#ifndef MEMDEBUG" */


#ifdef DEBUG /* { */

/* this is the DEBUG (but not MEMDEBUG) version */
void		CheckHandleExistence(Handle TheHandle)
	{
		StackSizeTest();
		/* check for various signs of a bad handle.  note that the last two */
		/* won't work if you have a lot of memory & are running the program way up. */
		if ((((ulong)TheHandle & 0x03) != 0)
			|| (TheHandle == NIL)
			|| (((ulong)*TheHandle & 0x03) != 0)
			|| (*TheHandle == NIL)
			|| (((ulong)TheHandle & 0xfc000000) != 0)
			|| (((ulong)*TheHandle & 0xfc000000) != 0))
			{
				PRERR(ForceAbort,"Undefined/Garbage Handle used.");
			}
	}


void		CheckPtrExistence(Ptr ThePointer)
	{
		StackSizeTest();
		/* check for various signs of a bad pointer.  note that the last two */
		/* won't work if you have a lot of memory & are running the program way up. */
		if ((((ulong)ThePointer & 0x03) != 0)
			|| (ThePointer == NIL)
			|| (((ulong)ThePointer & 0xfc000000) != 0))
			{
				PRERR(ForceAbort,"Undefined/Garbage Pointer used.");
			}
	}

#endif /* } */


#endif /* } */


#ifdef DEBUG /* { */

#define MAXITERATION (1024L*1024L)

/* this checks the heap.  It is, of course, a complete hack, probably won't */
/* run in 24 bit mode, and will need to be totally redone for any new systems. */
void			CheckHeap(void)
	{
		Zone*			MyZone;
		char*			LowestValidAddress;
		char*			HighestValidAddress;
		char*			BlockScan;
		char*			MasterPointerScan;
		char*			Temp;
		long			CurrentBlockSize;
		long			IterateCount;
		long			StupidUnusedVariable;

		StackSizeTest();
		MyZone = ApplicZone();
		LowestValidAddress = (char*)&(MyZone->heapData);
		HighestValidAddress = MyZone->bkLim;
		if (HighestValidAddress > (char*)&StupidUnusedVariable)
			{
				DebugStr("\pHeap bkLim is invalid");
				return;
			}

		BlockScan = LowestValidAddress;
		IterateCount = 0;
		while (BlockScan != HighestValidAddress)
			{
				IterateCount += 1;
				/* with a little work, we have discerned that: */
				/* first byte of block contains: 0x80 = relocatable, */
				/* 0x40 = nonrelocatable, 0x00 = free */
				/* second byte of block contains resource bits */
				/* fourth byte of block contains size correction */
				switch ((uchar)BlockScan[0])
					{
						case 0x80:
							Temp = (char*)*(long*)(BlockScan + 8); /* get relative handle */
							Temp = Temp + (long)MyZone; /* get absolute handle */
							if ((Temp < LowestValidAddress) || (Temp >= HighestValidAddress))
								{
									DebugStr("\pRelative handle invalid");
									return;
								}
							if ((char*)*(long*)Temp != (BlockScan + 12))
								{
									DebugStr("\pMaster pointer points to wrong block");
									return;
								}
							break;
						case 0x40:
							if ((char*)MyZone != (char*)*(long*)(BlockScan + 8))
								{
									DebugStr("\pNonrelocatable block invalid heap reference");
									return;
								}
							break;
						case 0x00:
							break;
						default:
							DebugStr("\pInvalid block tag");
							return;
					}
				/* now checking that block size works out properly */
				CurrentBlockSize = *(long*)(BlockScan + 4);
				BlockScan += CurrentBlockSize;
				if ((BlockScan < LowestValidAddress)
					|| (BlockScan > HighestValidAddress)
					|| ((CurrentBlockSize & 0x03) != 0)
					|| (CurrentBlockSize < 12))
					{
						DebugStr("\pCorrupt Heap");
						return;
					}
				if (IterateCount > MAXITERATION)
					{
						DebugStr("\pHeap check iteration limit exceeded");
						return;
					}
			}

		MasterPointerScan = MyZone->hFstFree;
		IterateCount = 0;
		while (MasterPointerScan != NIL)
			{
				IterateCount += 1;
				if ((MasterPointerScan < LowestValidAddress)
					|| (MasterPointerScan >= HighestValidAddress)
					|| (((long)MasterPointerScan & 0x03) != 0))
					{
						DebugStr("\pCorrupt Master Pointer List");
						return;
					}
				if (IterateCount > MAXITERATION)
					{
						DebugStr("\pHeap check iteration limit exceeded");
						return;
					}
				MasterPointerScan = (char*)*(long*)MasterPointerScan;
			}
	}


void			SetTopOfStack(char* ATopVar)
	{
		TopOfStack = ATopVar;
	}

#pragma options(!profile)

void			StackSizeTest(void)
	{
		char			AStupidVariable;

		if ((long)(TopOfStack - &AStupidVariable) > MaximumStackSize)
			{
				MaximumStackSize = (TopOfStack - &AStupidVariable);
			}
//	if (MaximumStackSize >= STACKSIZE - 256)
//		{
//			DebugStr("\pStack overflow");
//		}
	}


#endif /* } */
