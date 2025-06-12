/* Memory.h */

#pragma once

typedef long (*GrowZoneFuncPtrType)(ulong);

/* if the following symbol is defined in the header, then all allocated handles and */
/* pointers will be added to a table to see if they are being disposed twice. */
/* #define MEMDEBUG */

#ifdef MEMDEBUG
	#ifndef DEBUG
		#error MEMDEBUG can't be enabled if DEBUG is disabled!
	#endif
	Handle	DupSysHandle(Handle Original);
	void		SetTag(void* TheRef, char* TheTag);
	void		CheckHandleExistence(Handle TheHandle);
	void		CheckPtrExistence(Ptr ThePointer);
#else
	#define DupSysHandle(Original) HDuplicate(Original)
	#define SetTag(crud1,crud2)
	#ifndef DEBUG
		#define CheckHandleExistence(apadap)
		#define CheckPtrExistence(brapp)
	#else
		void		CheckHandleExistence(Handle TheHandle);
		void		CheckPtrExistence(Ptr ThePointer);
	#endif
#endif

MyBoolean	FirstMemCacheValid(void);
MyBoolean	SecondMemCacheValid(void);
void			InitMemory(void);
void			FlushMemory(void);
Handle		AllocHandle(ulong Size);
Handle		AllocHandleCanFail(ulong Size);
Ptr				AllocPtr(ulong Size);
Ptr				AllocPtrCanFail(ulong Size);
void			ReleaseHandle(Handle TheHand);
void			ReleasePtr(Ptr ThePtr);
void			InstallGrowZone(GrowZoneFuncPtrType MyGrowZone);
void			SetMemCacheRestore(MyBoolean Flag);
void			RestoreMemCache(void);

#ifdef DEBUG
	ulong			HandleSize(Handle h);
	ulong			PtrSize(Ptr p);
	void			CheckHeap(void);
	void			SetTopOfStack(char* ATopVar);
	void			StackSizeTest(void);
#else
	#pragma parameter __D0 HandleSize(__A0)
	pascal ulong HandleSize(Handle h) = 0xA025;
	#pragma parameter __D0 PtrSize(__A0)
	pascal ulong PtrSize(Ptr p) = 0xA021;
	#define CheckHeap()
	#define SetTopOfStack(silly)
	#define StackSizeTest()
#endif

#ifndef COMPILING_MEMORY_C
	EXECUTE(extern long NumHandlesAllocated;)
	EXECUTE(extern long NumPointersAllocated;)
	EXECUTE(extern long MaxNumHandlesAllocated;)
#endif
