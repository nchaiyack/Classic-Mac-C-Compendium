/* Memory.c */
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
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <Memory.h>
#include <Errors.h>
#include <Files.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "Memory.h"
#include "MyMalloc.h"


#define EnableHeapChecking (True)

#define MemoryFillPattern (0x81)

#if EnableHeapChecking
	#define HEAPCHECK() CheckHeap()
#else
	#define HEAPCHECK() ((void)0)
#endif


EXECUTE(long NumPtrsAllocated = 0;)
EXECUTE(MyBoolean Initialized = False;)

#if MEMDEBUG /* { */
	#define IsANothing (0)
	#define IsAHandle (1)
	#define IsAPointer (2)
	typedef struct StoreRec
		{
			struct StoreRec*	Next;
			void*							HandPtr; /* whatever it is */
			short							Type; /* handle or pointer */
			char*							Tag; /* string for tagging */
		} StoreRec;
	static StoreRec*		TrackList;
	static short				MemDumpFile;
	static MyBoolean		RegisterPointer(char* ThePointer);
	static void					DeregisterPointer(char* ThePointer);
	static void					SetUpChecking(void);
	static void					ShutOffChecking(void);
	static void					MemPrint(char* Str,...);
#else /* }{ */
	#define RegisterPointer(ThePointer) (True)
	#define DeregisterPointer(ThePointer) ((void)0)
	#define SetUpChecking() ((void)0)
	#define ShutOffChecking() ((void)0)
#endif /* } */


/* attempt to allocate a pointer */
#if DEBUG
	char*				EepAllocPtr(long Size, char* Tag)
#else
	char*				EepAllocPtr(long Size)
#endif
	{
		char*						Temp;

		ERROR(!Initialized,PRERR(ForceAbort,"Memory subsystem hasn't been initialized"));
		HEAPCHECK();
		ERROR((Size >= 0x00800000 - 16) || (Size < 0),
			PRERR(AllowResume,"AllocPtr:  Requested size is out of range."));
		if ((Size >= 0x00800000 - 16) || (Size < 0))
			{
				/* this prevents us from crashing in the final version when allocating */
				/* really big things */
				return NIL;
			}
		Temp = (char*)BlockNew(Size);
#if DEBUG
		if (Temp != NIL)
			{
				if (!RegisterPointer(Temp))
					{
						BlockRelease(Temp);
						return NIL;
					}
			}
		if (Temp != NIL)
			{
				long						Scan;

				for (Scan = 0; Scan < Size; Scan += 1)
					{
						Temp[Scan] = MemoryFillPattern;
					}
			}
		if (Temp != NIL)
			{
				SetTag(Temp,Tag);
			}
		if (Temp != NIL)
			{
				NumPtrsAllocated += 1;
			}
#endif
		return Temp;
	}


/* release pointer and attempt to restore cache */
void			ReleasePtr(char* ThePtr)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Memory subsystem hasn't been initialized"));
		ERROR(ThePtr==NIL,PRERR(ForceAbort,"ReleasePtr:  Tried to dispose a NIL pointer."));
#if DEBUG
		if (ThePtr != NIL)
			{
				NumPtrsAllocated -= 1;
			}
		{
			long						Scan;
			long						Limit;

			Limit = PtrSize(ThePtr);
			for (Scan = 0; Scan < Limit; Scan += 1)
				{
					ThePtr[Scan] = MemoryFillPattern;
				}
		}
#endif
		DeregisterPointer(ThePtr);
		BlockRelease(ThePtr);
		HEAPCHECK();
	}


long			PtrSize(char* p)
	{
		long		Result;

		ERROR(!Initialized,PRERR(ForceAbort,"Memory subsystem hasn't been initialized"));
		CheckPtrExistence(p);
		return BlockSize(p);
	}


char*		ResizePtr(char* ThePointer, long NewSize)
	{
		void*			NewOne;
#if MEMDEBUG
		StoreRec*			Scan;
#endif

		ERROR(!Initialized,PRERR(ForceAbort,"Memory subsystem hasn't been initialized"));
		NewOne = BlockResize(ThePointer,NewSize);
#if MEMDEBUG
		Scan = TrackList;
		while (Scan != NIL)
			{
				if ((Scan->Type == IsAPointer) && (Scan->HandPtr == ThePointer))
					{
						Scan->HandPtr = NewOne;
						goto SpiffyPoint;
					}
				Scan = Scan->Next;
			}
		APRINT(("ResizePtr: Undefined pointer used: %r",ThePointer));
		PRERR(ForceAbort,"ResizePtr:  Undefined pointer used.");
#endif
	 SpiffyPoint:
		return (char*)NewOne;
	}


/* initialize the cache */
MyBoolean	Eep_InitMemory(void)
	{
		long			Count;
		MyBoolean	ReturnValue;

		APRINT(("+InitMemory"));
		MaxApplZone();
		ERROR(Initialized,PRERR(ForceAbort,"InitMemory called more than once."));
		for (Count = 4; Count >= 0; Count -= 1)
			{
				MoreMasters();
			}
		SetUpChecking();
		EXECUTE(Initialized = True;)
		ReturnValue = InitializeMyMalloc();
		APRINT(("-InitMemory"));
		return ReturnValue;
	}


void			Eep_FlushMemory(void)
	{
		APRINT(("+FlushMemory"));
		ERROR(!Initialized,PRERR(ForceAbort,"Memory subsystem hasn't been initialized"));
		ERROR(NumPtrsAllocated!=0,PRERR(AllowResume,
			"FlushMemory:  Some pointers are still allocated just before quitting."));
		ShutOffChecking();
		EXECUTE(Initialized = False;)
		APRINT(("-FlushMemory"));
	}


#if MEMDEBUG /* { */


static MyBoolean		RegisterPointer(char* ThePointer)
	{
		StoreRec*					Temp;

		Temp = (StoreRec*)BlockNew(sizeof(StoreRec));
		if (Temp == NIL)
			{
				return False;
			}
		Temp->Next = TrackList;
		TrackList = (StoreRec*)Temp;
		Temp->Type = IsAPointer;
		Temp->HandPtr = ThePointer;
		Temp->Tag = NIL;
		return True;
	}


static void					DeregisterPointer(char* ThePointer)
	{
		StoreRec*					Scan;
		StoreRec*					Lag;
		char*							Temp;

		Scan = TrackList;
		Lag = NIL;
		while ((Scan != NIL) && (Scan->HandPtr != ThePointer))
			{
				Lag = Scan;
				Scan = Scan->Next;
			}
		if (Scan == NIL)
			{
				APRINT(("Deletion of nonexistent pointer %r",ThePointer));
				PRERR(ForceAbort,"Deletion of nonexistent pointer.");
			}
		if (Scan->Type != IsAPointer)
			{
				APRINT(("Deletion of pointer of unknown type %r",ThePointer));
				PRERR(ForceAbort,"Deletion of pointer of unknown type.");
			}
		if (Lag == NIL)
			{
				Temp = (char*)TrackList;
				TrackList = Scan->Next;
				BlockRelease(Temp);
			}
		 else
			{
				Temp = (char*)Lag->Next;
				Lag->Next = Scan->Next;
				BlockRelease(Temp);
			}
	}


void			SetTag(void* TheRef, char* TheTag)
	{
		StoreRec*		Scan;
		short				TagScan;

		Scan = TrackList;
		while ((Scan != NIL) && (Scan->HandPtr != TheRef))
			{
				Scan = Scan->Next;
			}
		if (Scan == NIL)
			{
				PRERR(ForceAbort,"Handle/Ptr couldn't be found by SetTag.");
			}
		 else
			{
				Scan->Tag = TheTag;
			}
	}


/* this checks to see if a pointer exists.  as a performance enhancement (since */
/* we search this list constantly) referenced elements are moved to the head */
/* of the list */
void		CheckPtrExistence(void* ThePointer)
	{
		StoreRec*			Scan;
		StoreRec*			Lag;

		HEAPCHECK();
		if (ThePointer == NIL)
			{
				PRERR(ForceAbort,"CheckPtrExistence:  Pointer is NIL");
			}
		Lag = NIL;
		Scan = TrackList;
		while (Scan != NIL)
			{
				if ((Scan->Type == IsAPointer) && (Scan->HandPtr == ThePointer))
					{
						/* found -- and correct */
						if (Lag != NIL)
							{
								/* if element isn't at the beginning of the list, move it */
								/* there. */
								Lag->Next = Scan->Next; /* removed from old place */
								Scan->Next = TrackList; /* added to beginning */
								TrackList = Scan;
							}
						return;
					}
				Lag = Scan;
				Scan = Scan->Next;
			}
		APRINT(("Undefined pointer used: %r",ThePointer));
		PRERR(ForceAbort,"Undefined pointer used.");
	}


static void					SetUpChecking(void)
	{
		TrackList = NIL;
	}


static void					ShutOffChecking(void)
	{
		unsigned char			DumpName[] = {"\p!!MemCheck Still Allocated Dump"};
		StoreRec*					Scan;
		short							VRefNum;

		CheckHeap();
		FSDelete(DumpName,0);
		ERROR(Create(DumpName,0,AUDITCREATOR,'TEXT') != noErr,
			PRERR(ForceAbort,"Memory's ShutOffChecking couldn't create dump file."));
		ERROR(FSOpen(DumpName,0,&MemDumpFile) != noErr,PRERR(ForceAbort,
			"Memory's ShutOffChecking couldn't open dump file for writing."));
		/* MemPrint("These handles and pointers are still allocated:"); */
		Scan = TrackList;
		while (Scan != NIL)
			{
				switch (Scan->Type)
					{
						case IsAHandle:
							MemPrint("Handle %x '%t'",Scan->HandPtr,Scan->Tag);
							break;
						case IsAPointer:
							MemPrint("Pointer %x '%t'",Scan->HandPtr,Scan->Tag);
							break;
					}
				Scan = Scan->Next;
			}
		GetVRefNum(MemDumpFile,&VRefNum);
		FSClose(MemDumpFile);
		FlushVol("\p",VRefNum);
	}


/* some va_args crud for MemPrint */
typedef void *va_list;
#define __va(arg)  &arg + 1
#define va_start(p, arg)  p = __va(arg)
#define va_arg(p, type)  *(* (type **) &p)++
#define va_end(p) ((void)0)

/* I lifted this from Audit.c */
/* this prints a string in the same way that printf does.  it accepts these options: */
/* %x = hexadecimal long */
/* %t = C String (text) */
#define BUFSIZE (256)
static void					MemPrint(char* Str,...)
	{
		va_list						pa;
		char							Buffer[BUFSIZE];
		long							BufPtr;
		static char				Hex[] = "0123456789abcdef";

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
										char						Buf[9];
										short						Count;
										unsigned long		Num;

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


#else /* }{  this next part is "#if !MEMDEBUG" */


#if DEBUG /* { */

#define AlignmentMask (0x03) /* should be 0x03??? */

/* this is the DEBUG (but not MEMDEBUG) version */
void		CheckPtrExistence(void* ThePointer)
	{
		static MyBoolean		ZoneIsValid = False;
		static Zone*				Zone;
		static char*				ZoneBeginning;
		char*								ZoneEnd;

		/* check for various signs of a bad pointer.  note that the last two */
		/* won't work if you have more than 64 Megabytes. */
		if (!ZoneIsValid)
			{
				Zone = GetZone();
				ZoneIsValid = True;
				ZoneBeginning = (char*)&(Zone->heapData);
			}
		ZoneEnd = (char*)Zone->bkLim;
		if ((((unsigned long)ThePointer & AlignmentMask) != 0) || (ThePointer == NIL)
			|| ((char*)ThePointer < ZoneBeginning) || ((char*)ThePointer >= ZoneEnd))
			{
				PRERR(ForceAbort,"Undefined/Garbage Pointer used.");
				CheckHeap();
			}
		/* HEAPCHECK(); */
	}

#endif /* } */


#endif /* } */


#if DEBUG
void			PRNGCHK(void* ThePointer, void* EffectiveAddress, signed long AccessSize)
	{
		signed long			PSize;
		signed long			Difference;

		PSize = PtrSize((char*)ThePointer);
		Difference = (char*)EffectiveAddress - (char*)ThePointer;
		if ((Difference < 0) || (Difference + AccessSize > PSize))
			{
				APRINT(("PRNGCHK pointer access range error: (%xl..%xl):%xl(+%l)",ThePointer,
					(char*)ThePointer + PSize - 1,EffectiveAddress,AccessSize));
				PRERR(ForceAbort,"Pointer access out of range.");
			}
	}
#endif
