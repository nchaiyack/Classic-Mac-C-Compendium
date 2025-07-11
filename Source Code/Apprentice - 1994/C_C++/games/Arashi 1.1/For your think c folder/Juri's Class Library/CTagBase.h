/*/
     Project Arashi: CTagBase.h
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, February 21, 1993, 16:02
     Created: Thursday, January 7, 1993, 1:05

     Copyright � 1993, Juri Munkki
/*/

#pragma once
#include <CBaseObject.h>

typedef	struct
{
	long	tag;
	long	dataLen;
	char	data[];
} StoredTagDataBaseElement;

typedef struct
{
	long	offset;			//	If negative, then link to next free master block.
	long	tag;
	long	hashLink;		//	Index of next item in this hash list
	union
	{	long	link;		//	Link to next free master pointer.
		short	flags;		//	A field for your own use. See instructions near method declarations.
	} lf;
} TagMasterBlock;

typedef struct
{
	long	masterIndex;	//	Index to owner master block. Negative, if block has been released.
	long	dataLen;		//	Length of data
	char	data[];			//	the data.
} TagDataBlock;

#define	TAGMASTERCLUMP	32L		//	Allocate 32 masters at a time
#define	TAGBASECLUMP	1024L	//	Allocate at least 1KB of string space every time
#define	TAGMAXWASTE		2048L	//	Don't waste more than 2KB of string space before compacting
#define TAGBASEHASHSIZE	64

class	CTagBase : public CBaseObject
{
public:
	long		hashTable[TAGBASEHASHSIZE];

	long		masterCount;		//	Amount of allocated master blocks.
	long		realMasterSize;		//	Real memory allocated to master blocks.
	long		logicalMasterSize;	//	Used memory for master blocks.
	TagMasterBlock	**masterBlocks;	//	Handle to master blocks.
	
	long		firstFreeMaster;	//	Index of first free master block or -1 if none.
	
	long		realTagBaseSize;		//	Real memory allocated for TagBase blocks.
	long		logicalTagBaseSize;	//	Used memory for TagBase blocks (with garbage blocks).
	Handle		tagBaseBlocks;		//	Handle to string blocks.
	
	long		wastedSpace;		//	Amount of garbage bytes in logicalStringSize.

	void		ITagBase();			//	Initialize to empty - always call after new
	void		Dispose();			//	To release all memory
	
	void		Lock();
	void		Unlock();

	long		LookupTag(long tag);
	long		WriteEntry(long tag, long len, void *theData);
	short		ReadEntry(long tag, long *len, void *theData);
	Handle		ReadIntoHandle(long tag);
	long		WriteDefault(long tag, long len, void *theData);
	
	void *		GetEntryPointer(long tag);
	long		GetEntrySize(long tag);

	long		StoreData(long tag, long len, void *theData);
	short		RecallData(long index, long *len, void *theData);
	short		ReleaseData(long index);	//	Purge string and master block from heap.

/*
**	Some utility routines for storing and retrieving common data types:
*/
	void		WriteString(long tag, StringPtr theString);
	void		ReadString(long tag, StringPtr theString);
	
	void		WriteShort(long tag, short value);
	short		ReadShort(long tag, short defaultValue);

	void		WriteLong(long tag, long value);
	long		ReadLong(long tag, long defaultValue);
	
	void		WriteHandle(long tag, Handle contents);
	Handle		ReadHandle(long tag);
/*
**	TagBase objects usually have to be saved to files.
**	To facilitate this, the whole contents of the database
**	can be dumped into a single handle. The following methods
**	dump the contents to a handle and allow you to read it
**	back from a handle of the same format.
*/
	Handle		ConvertToHandle();		//	Convert to something that can be stored as a resource.
	void		ConvertFromHandle();	//	Does the opposite of ConvertToHandle.
/*
**	The flags field can be used to mark strings with a number. You can
**	then use this number as you wish, although it was originally designed
**	so that you can use any bit combination to signify a string that should
**	be deleted. You can for instance mark all the strings you use with a
**	certain value (let's say 1). If you then waste some of your reference
**	indices (by overwriting them while deleting data from a table, for instance),
**	you don't need to worry about deallocation, since after the operation,
**	you can again change the flag value (let's say to 0) and then call
**	ReleaseFlagged with the value you first stored (ReleaseFlagged(1,1)).
**
**	The result is that only those strings that remain in your table will
**	be stored in the string heap. Neat, huh?
*/
	short		GetFlags(long index);				//	Get the flags field.
	short		SetFlags(long index, short flags);	//	Set the flags field.
	void		ReleaseFlagged(short flagMask, short flagValue);	// read above.

//	Internal routines:
	short		CreateMoreMasters();				//	Called when running out of master blocks
	long		AllocateSpace(long len, void *theData);//	Used to write allocate and copy data
	void		GarbageCollect();					//	Compact heap. You may use it, if necessary.
};