/*/
     Project Arashi: CTagBase.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, February 21, 1993, 16:03
     Created: Thursday, January 7, 1993, 1:05

     Copyright © 1993, Juri Munkki
/*/

#include "CTagBase.h"
#include <RAMFiles.h>

int		HashLongTag(
	long	theTag)
{
	theTag += theTag >> 16;
	theTag += theTag >> 8;
	
	return theTag & (TAGBASEHASHSIZE - 1);
}

void	CTagBase::ITagBase()
{
	int		i;
	
	IBaseObject();

	for(i=0;i<TAGBASEHASHSIZE;i++)
	{	hashTable[i] = -1;
	}

	masterCount = 0;
	realMasterSize = 0;
	logicalMasterSize = 0;
	masterBlocks = (TagMasterBlock **)NewHandle(0);
	
	firstFreeMaster = -1;	//	No free masters.
	
	realTagBaseSize = 0;
	logicalTagBaseSize = 0;
	tagBaseBlocks = NewHandle(0);
	
	wastedSpace = 0;
}

short	CTagBase::CreateMoreMasters()
{
	long			oldOffset = logicalMasterSize;
	TagMasterBlock	*blockP;
	int				result = memFullErr;
	short			state;
	
	state = HGetState(this);
	HLock(this);

	if(IncreaseRamFile(	(Handle)masterBlocks,
						&realMasterSize,
						&logicalMasterSize,
						sizeof(TagMasterBlock)*TAGMASTERCLUMP) == noErr)
	{	blockP = masterCount+*masterBlocks;
		
		while(oldOffset + sizeof(TagMasterBlock) <= logicalMasterSize)
		{	blockP->offset = -1;	//	Signifies free block
			blockP->lf.link = firstFreeMaster;
			firstFreeMaster = masterCount++;
			blockP++;
			oldOffset += sizeof(TagMasterBlock);
		}
		
		result = noErr;
	}
	
	HSetState(this,state);
	return result;
}

long	CTagBase::AllocateSpace(
	long	len,
	void	*theData)
{
	long			result = memFullErr;
	long			offset;
	short			state;
	TagDataBlock	*dataP;
	
	if(wastedSpace > TAGMAXWASTE)
		GarbageCollect();

	state = HGetState(this);
	HLock(this);
	
	offset = logicalTagBaseSize;
	if(IncreaseByClump(	tagBaseBlocks,
						&realTagBaseSize,
						&logicalTagBaseSize,
						sizeof(TagDataBlock)+len,
						TAGBASECLUMP) == noErr)
	{	result = offset;
		dataP = (TagDataBlock *)(result + *tagBaseBlocks);
		BlockMove(theData,&(dataP->data),len);
		dataP->dataLen = len;
	}
	
	HSetState(this,state);
	return result;
}

long	CTagBase::StoreData(
	long		tag,
	long		len,
	void		*theData)
{
	long			result = memFullErr;
	long			offset;
	TagMasterBlock	*masterP;
	TagDataBlock	*dataP;
	
	offset = AllocateSpace(len, theData);

	if(offset>=0)
	{
		if(firstFreeMaster<0)
			CreateMoreMasters();
		
		dataP = (TagDataBlock *)(offset + *tagBaseBlocks);

		if(firstFreeMaster>=0)
		{	short	hashValue;
		
			result = firstFreeMaster;
			masterP = result + *masterBlocks;
			firstFreeMaster = masterP->lf.link;
			masterP->lf.flags = 0;
			masterP->offset = offset;
			masterP->tag = tag;

			hashValue = HashLongTag(tag);
			masterP->hashLink = hashTable[hashValue];
			hashTable[hashValue] = result;
		}

		dataP->masterIndex = result;
	}
	
	return result;
}

long	CTagBase::LookupTag(
	long		tag)
{
	long	foundIndex;
	
	foundIndex = hashTable[HashLongTag(tag)];
	
	while(foundIndex >= 0)
	{
		if((*masterBlocks)[foundIndex].tag == tag)
				break;
		else	foundIndex = (*masterBlocks)[foundIndex].hashLink;
	}
	
	return foundIndex;
}

long	CTagBase::WriteEntry(
	long		tag,
	long		len,
	void		*theData)
{
	long			foundIndex;
	long			offset;
	TagMasterBlock	*itsMaster;
	TagDataBlock	*itsBlock;
	
	foundIndex = LookupTag(tag);
	
	if(foundIndex < 0)	//	Not found, create entry
	{	foundIndex = StoreData(tag,len,theData);
	}
	else				//	Found. Is it the same size as the new entry?
	{	itsMaster = *masterBlocks + foundIndex;
		itsBlock = (TagDataBlock *)(itsMaster->offset + *tagBaseBlocks);
		
		if(itsBlock->dataLen == len)	//	Compare old and new size
		{								//	Same: re-use space
			BlockMove(theData, itsBlock->data, len);
		}
		else							//	Different, try to create a new block.
		{	offset = AllocateSpace(len, theData);
			if(offset >= 0)
			{	//	Following pointers may have moved, so recalculate them:
			
				itsMaster = *masterBlocks + foundIndex;
				itsBlock = (TagDataBlock *)(itsMaster->offset + *tagBaseBlocks);
				
				itsBlock->masterIndex = -1;	//	Release old block
			 	wastedSpace += sizeof(TagDataBlock) + itsBlock->dataLen;
			 	itsMaster->offset = offset;
			 	itsBlock = (TagDataBlock *)(offset + *tagBaseBlocks);
			 	itsBlock->masterIndex = foundIndex;
			}
		}	
	}
	
	return foundIndex;
}

long	CTagBase::WriteDefault(
	long		tag,
	long		len,
	void		*theData)
{
	long			foundIndex;
	
	foundIndex = LookupTag(tag);
	
	if(foundIndex < 0)	//	Not found, create entry
	{	foundIndex = StoreData(tag,len,theData);
	}

	return foundIndex;
}

short	CTagBase::RecallData(
	long		index,
	long		*len,
	void		*theData)
{
	int				result = noErr;
	TagMasterBlock	*masterP;
	TagDataBlock	*dataP;
	long			offset;

	if(index < 0 || index >= masterCount)
	{	result = eofErr;
	}
	else
	{	masterP = index + *masterBlocks;
		offset = masterP->offset;
		if(offset >= 0)
		{	dataP = (TagDataBlock *)(offset + *tagBaseBlocks);
			if(*len > dataP->dataLen)
			{	*len = dataP->dataLen;
			}
			if(*len > 0)
				BlockMove(&(dataP->data),theData,*len);

			result = noErr;
		}
		else
		{	result = fnfErr;
		}
	}
	return result;
}

short	CTagBase::ReadEntry(
	long	tag,
	long	*len,
	void	*theData)
{
	long	foundIndex;
	
	foundIndex = LookupTag(tag);
	
	if(foundIndex >= 0)
	{	return RecallData(foundIndex, len, theData);
	}
	else
	{	*len = 0;
		return foundIndex;
	}
}

long	CTagBase::GetEntrySize(
	long	tag)
{
	long	foundIndex;
	
	foundIndex = LookupTag(tag);
	
	if(foundIndex >= 0)
	{
		TagMasterBlock	*masterP;
		TagDataBlock	*dataP;
		long			offset;

		masterP = foundIndex + *masterBlocks;
		offset = masterP->offset;

		if(offset >= 0)
		{	dataP = (TagDataBlock *)(offset + *tagBaseBlocks);
			return dataP->dataLen;
		}
		else
			return 0;
	}
	else
	{
		return 0;
	}	

}
void *	CTagBase::GetEntryPointer(
	long	tag)
{
	long	foundIndex;
	
	foundIndex = LookupTag(tag);
	
	if(foundIndex >= 0)
	{
		TagMasterBlock	*masterP;
		TagDataBlock	*dataP;
		long			offset;

		masterP = foundIndex + *masterBlocks;
		offset = masterP->offset;

		if(offset >= 0)
		{	dataP = (TagDataBlock *)(offset + *tagBaseBlocks);
			return &(dataP->data);
		}
		else
			return 0;
	}
	else
	{
		return 0;
	}	
}

Handle		CTagBase::ReadIntoHandle(
	long tag)
{
	long			index;
	long			len;
	long			offset;
	Handle			theHandle = NULL;
	
	index = LookupTag(tag);
	
	if(index >= 0)
	{
		offset = (*masterBlocks)[index].offset;
		len = ((TagDataBlock *)(offset + *tagBaseBlocks))->dataLen;
		
		theHandle = NewHandle(len);
		if(theHandle)
		{
			BlockMove(((TagDataBlock *)(offset + *tagBaseBlocks))->data,
						*theHandle,
						len);
		}
	}

	return theHandle;
}

void	CTagBase::GarbageCollect()
{
	TagDataBlock	*dataP;
	TagMasterBlock	*masterP;
	long			sourceOffset, destOffset;
	long			blockLen;
	
	sourceOffset = 0;
	destOffset = 0;
	
	while(sourceOffset < logicalTagBaseSize)
	{
		dataP = (TagDataBlock *)(sourceOffset + *tagBaseBlocks);
		blockLen = dataP->dataLen+sizeof(TagDataBlock);
		blockLen += blockLen & 1;	//	It's always even.

		if(dataP->masterIndex >= 0)
		{	if(sourceOffset != destOffset)
			{	masterP = dataP->masterIndex + *masterBlocks;
				masterP->offset = destOffset;
				BlockMove(	dataP,
							(TagDataBlock *)(destOffset + *tagBaseBlocks),
							blockLen);
			}
			destOffset += blockLen;
		}
		
		sourceOffset += blockLen;
	
	}
	
	logicalTagBaseSize = destOffset;
	wastedSpace = 0;
}

short	CTagBase::ReleaseData(
	long	index)
{
	int				result = noErr;
	TagMasterBlock	*masterP;
	TagDataBlock	*dataP;
	long			offset;
	long			waste;
	long			hashList;

	if(index < 0 || index >= masterCount)
	{	result = eofErr;
	}
	else
	{	masterP = index + *masterBlocks;
		offset = masterP->offset;
		if(offset >= 0)
		{	
			hashList = hashTable[HashLongTag(masterP->tag)];
			while(hashList >= 0)
			{	if((hashList + *masterBlocks)->tag == masterP->tag)
				{	(hashList + *masterBlocks)->hashLink = masterP->hashLink;
					hashList = -1;
				}
				else
				{	hashList = (hashList + *masterBlocks)->hashLink;
				}
			}
			
			dataP = (TagDataBlock *)(offset + *tagBaseBlocks);
			dataP->masterIndex = -1;
			waste = sizeof(TagDataBlock) + dataP->dataLen;
			waste += waste & 1;		// size is always really even.
			wastedSpace += waste;
			
			masterP->offset = -1;
			masterP->lf.link = firstFreeMaster;
			firstFreeMaster = index;
			result = noErr;
		}
		else
		{	result = fnfErr;
		}
	}
	return result;
}

short	CTagBase::GetFlags(
	long	index)
{
	int				result = noErr;
	TagMasterBlock	*masterP;

	if(index < 0 || index >= masterCount)
	{	result = 0;
	}
	else
	{	masterP = index + *masterBlocks;
		if(masterP->offset >= 0)
		{	result = masterP->lf.flags;
		}
		else
		{	result = 0;
		}
	}
	return result;
}

short	CTagBase::SetFlags(
	long	index,
	short	flags)
{
	int			result = noErr;
	TagMasterBlock	*masterP;

	if(index < 0 || index >= masterCount)
	{	result = eofErr;
	}
	else
	{	masterP = index + *masterBlocks;
		if(masterP->offset >= 0)
		{	masterP->lf.flags = flags;
			result = noErr;
		}
		else
		{	result = fnfErr;
		}
	}
	return result;
}

void	CTagBase::ReleaseFlagged(
	short	flagMask,
	short	flagValue)
{
	TagMasterBlock	*masterP;
	short		theFlags;
	long		i;
	
	i = masterCount;
	
	while(i--)
	{	masterP = i + *masterBlocks;
		
		if(masterP->offset >= 0)
		{	if((masterP->lf.flags & flagMask) == flagValue)
			{	ReleaseData(i);
			}
		}
	}
}

void	CTagBase::Dispose()
{	
	DisposHandle(tagBaseBlocks);
	DisposHandle(masterBlocks);
	
	inherited::Dispose();
}

/*
**	TagBase objects usually have to be saved to files.
**	To facilitate this, the whole contents of the database
**	can be dumped into a single handle. The following methods
**	dump the contents to a handle and allow you to read it
**	back from a handle of the same format.
*/
Handle	CTagBase::ConvertToHandle()
{
	Handle						theHandle;
	long						neededSize;
	TagMasterBlock				*masterP;
	TagDataBlock				*dataP;
	StoredTagDataBaseElement	*element;
	long						i;
	
	i = masterCount;
	neededSize = 0;	//	First time around, find out how much RAM is needed.
	
	while(i--)
	{	masterP = i + *masterBlocks;
		
		if(masterP->offset >= 0)
		{	dataP = (TagDataBlock *)(masterP->offset + *tagBaseBlocks);
			neededSize += sizeof(StoredTagDataBaseElement) + dataP->dataLen;
			neededSize += neededSize & 1;	//	Keep it even.
		}
	}
	theHandle = NewHandle(neededSize);

	i = masterCount;
	neededSize = 0;	//	Use as an offset from now on.
	while(i--)
	{	masterP = i + *masterBlocks;
		
		if(masterP->offset >= 0)
		{
			element = (StoredTagDataBaseElement *)(*theHandle + neededSize);
			element->tag = masterP->tag;
			dataP = (TagDataBlock *)(masterP->offset + *tagBaseBlocks);
			element->dataLen = dataP->dataLen;
			BlockMove(dataP->data, element->data, dataP->dataLen);
			
			neededSize += sizeof(StoredTagDataBaseElement) + dataP->dataLen;
			neededSize += neededSize & 1;	//	Keep it even.
		}
	}
	
	return theHandle;
}

void	CTagBase::ConvertFromHandle(
	Handle	theHandle)
{
	short						state;
	long						offset;
	long						totalSize;
	StoredTagDataBaseElement	*element;

	if(theHandle)
	{	state = HGetState(theHandle);
		HLock(theHandle);
		
		offset = 0;
		totalSize = GetHandleSize(theHandle);
		
		while(offset < totalSize)
		{
			element = (StoredTagDataBaseElement *)(offset + *theHandle);
			offset += element->dataLen + sizeof(StoredTagDataBaseElement);
			offset += offset & 1;
			WriteEntry(element->tag, element->dataLen, element->data);
		}
		
		HSetState(theHandle, state);
	}
}

void	CTagBase::WriteString(
	long		tag,
	StringPtr	theString)
{
	WriteEntry(tag, *theString, theString+1);
}

void	CTagBase::ReadString(
	long		tag,
	StringPtr	theString)
{
	long	len = 255;
	
	ReadEntry(tag, &len, theString+1);
	*theString = len;
}

short	CTagBase::ReadShort(
	long		tag,
	short		defaultValue)
{
	long	len = sizeof(short);
	
	ReadEntry(tag, &len, &defaultValue);
	
	return defaultValue;
}

void	CTagBase::WriteShort(
	long	tag,
	short	value)
{
	WriteEntry(tag, sizeof(short), &value);
}

long	CTagBase::ReadLong(
	long		tag,
	long		defaultValue)
{
	long	len = sizeof(long);
	
	ReadEntry(tag, &len, &defaultValue);
	
	return defaultValue;
}

void	CTagBase::WriteLong(
	long	tag,
	long	value)
{
	WriteEntry(tag, sizeof(long), &value);
}

void	CTagBase::WriteHandle(
	long	tag,
	Handle	contents)
{
	long	theSize;
	short	state;
	
	theSize = GetHandleSize(contents);
	state = HGetState(contents);
	HLock(contents);
	WriteEntry(tag, theSize, *contents);
	HSetState(contents, state);
}
Handle	CTagBase::ReadHandle(
	long tag)
{
	long			totalSize;
	Handle			returnHandle = 0;
	long			foundIndex;
	TagMasterBlock	*masterP;
	TagDataBlock	*dataP;
	
	totalSize = 0;
	
	foundIndex = LookupTag(tag);
	if(foundIndex >= 0)
	{	dataP = (TagDataBlock *)(*tagBaseBlocks + (*masterBlocks + foundIndex)->offset);
		totalSize = dataP->dataLen;

		returnHandle = NewHandle(totalSize);
		if(returnHandle)
		{	HLock(returnHandle);
			RecallData(foundIndex, &totalSize, *returnHandle);
			HUnlock(returnHandle);
		}
	}
	
	return returnHandle;
}

void	CTagBase::Lock()
{
	if(!lockCounter)
	{	HLock(masterBlocks);
		HLock(tagBaseBlocks);
	}
	inherited::Lock();
}

void	CTagBase::Unlock()
{
	if(lockCounter == 1)
	{	HUnlock(masterBlocks);
		HUnlock(tagBaseBlocks);
	}
	inherited::Unlock();
}
