/* Memory.h */

#ifndef Included_Memory_h
#define Included_Memory_h

/* Memory module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MyMalloc */

/* if the following symbol is defined in the header, then all allocated */
/* pointers will be added to a table to see if they are being disposed twice. */
/* #define MEMDEBUG */

/* routine for changing the tag associated with an allocated object. */
#if MEMDEBUG
	#if !DEBUG
		#error "MEMDEBUG can't be enabled if DEBUG is disabled!"
	#endif
	/* alter the identification tag on an object */
	void		SetTag(void* TheRef, char* TheTag);
#else
	#define SetTag(crud1,crud2)
#endif

#if DEBUG
	/* debugging code */
	/* allocate a single-indirection block of memory; returns NIL if it can't be done */
	#define AllocPtrCanFail(Size,Tag) EepAllocPtr((Size),(Tag))
	/* verify that a pointer is actually valid; abort the program with an error */
	/* message if it isn't.  This disappears when the DEBUG macro is undefined */
	void		CheckPtrExistence(void* ThePointer);
	/* checks the range of access of a ptr to see if it is within the ptr's size */
	/* AccessSize is how large an 'object' will be accessed.  Pass sizeof(type) to it. */
	void		PRNGCHK(void* ThePointer, void* EffectiveAddress, signed long AccessSize);
#else
	/* production code */
	#define AllocPtrCanFail(Size,Tag) EepAllocPtr((Size))
	#define CheckPtrExistence(brapp)
	#define PRNGCHK(Poin,Addr,Mode)
#endif

/* Initialize the memory subsystem.  Must be called at the beginning of the */
/* program before any other modules are initialized */
MyBoolean	Eep_InitMemory(void);

/* clean up anything that must be disposed of before the program terminates */
/* and create a list of pointers that are still allocated.  This */
/* must be called as the last instruction in the program */
void			Eep_FlushMemory(void);

/* these functions are local to the implementation */
#if DEBUG
	/* debugging code */
	char*			EepAllocPtr(long Size, char* Tag);
#else
	/* production code */
	char*			EepAllocPtr(long Size);
#endif

/* Dispose of a block of memory that is no longer needed */
void			ReleasePtr(char* ThePtr);

/* obtain the size of the specified Ptr in bytes */
long			PtrSize(char* p);

/* resize a pointer.  The pointer might have to be entirely reallocated, in which */
/* case the new one will be returned and the old one will be disposed. */
char*			ResizePtr(char* ThePointer, long NewSize);

#endif
