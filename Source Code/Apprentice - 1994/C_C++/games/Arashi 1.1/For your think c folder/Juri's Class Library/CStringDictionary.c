/*/
     Project Arashi: CStringDictionary.c
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, June 29, 1993, 2:16
     Created: Friday, December 4, 1992, 11:45

     Copyright � 1992-1993, Juri Munkki
/*/

/*
**	See include file for more details on how this class works.
*/
#include "CStringDictionary.h"
#include <RamFiles.h>

/*
**	Given a counted string of length len, HashString
**	will return a value from 0 to HASHTABLESIZE-1.
**	This value can be used as the hashing value for
**	that string.
*/ 
short	HashString(
	unsigned char 	*string,
	short			len)
{	
	if(len>0)
	{	return (string[0]+string[len-1]+string[len>>1]) & (HASHTABLESIZE - 1);
	}
	else
		return 0;
}
/*
**	Initialize a CStringDictionary. The directory will be empty.
*/
void	CStringDictionary::IStringDictionary()
{
	int		i;
	
	dictCount = 0;					//	Dictionary is empty.
	logicalDictSize = 0;
	realDictSize = 0;
	dictionary = (DictEntry **)NewHandle(0);
	
	logicalWordListSize = 0;		//	Dictionary words are listed separately.
	realWordListSize = 0;
	wordList = (unsigned char **)NewHandle(0);
		
	for(i=0;i<HASHTABLESIZE;i++)	//	Hash table is empty.
	{	hashTable[i] = -1;
	}

	IBaseObject();
}

/*
**	Return count of items in dictionary.
*/
short	CStringDictionary::GetDictionarySize()
{
	return dictCount;
}

/*
**	Copy the string for entry index to theEntry
*/
void	CStringDictionary::GetIndEntry(
	short		index,
	StringPtr	theEntry)
{
	DictEntry	*thisEntry;
	StringPtr	entryString;
	short		i;
	
	if(index >= 0 && index < dictCount)
	{
		thisEntry = index + (DictEntry *) *dictionary;
		entryString = thisEntry->nameOffset + *wordList;
		
		i = 1+entryString[0];
		while(i--)
		{	theEntry[i] = entryString[i]; 
		}
	}
}

/*
**	Add a dictionary entry regardless
**	of wether it already is there or not. Normally you would
**	first look for a dictionary entry before calling this
**	routine. Call this routine if you are absolutely sure
**	that the entry isn't already in the dictionary.
**
**	NOTE: A negative return value signifies and error.
*/
tokentype	CStringDictionary::AddDictEntry(
	unsigned char	*entry,
	short			len)
{
	long			where;
	tokentype		entryIndex = 0;
	long			theErr = noErr;
	short			hashed;
	short			state;
	DictEntry		*entryP;
	
	if(len<0) len = *entry;
	if(len>255) len = 255;

	state = HGetState((Handle) this);	//	Lock the object for a while.
	HLock((Handle)this);
	
	//	Increase the dictionary size, if memory allows.
	if(IncreaseByClump(	(Handle)dictionary,
						&realDictSize,
						&logicalDictSize,
						sizeof(DictEntry),
						DICTIONARYCLUMPSIZE) == noErr)
	{
		where = logicalWordListSize;	//	New word is written at this offset.
		
		//	Increase the word list size, if memory allows.
		if(IncreaseByClump(	(Handle)wordList,
							&realWordListSize,
							&logicalWordListSize,
							len+1,
							WORDCLUMPSIZE) == noErr)
		{	BlockMove(entry+1,1+where+*wordList,len);	//	Write the word.
			(*wordList)[where] = len;					//	Store the length
			entryIndex = dictCount++;					//	Get a new token value (or index).
			entryP = entryIndex + *dictionary;			//	A pointer to the entry.
			
			hashed = HashString(entry+1,len);			//	Hash the entry by adding it
			entryP->hashLink = hashTable[hashed];		//	in front of all other entries
			entryP->nameOffset = where;					//	in the linked list of words
			hashTable[hashed] = entryIndex;				//	hashing to the same entry.
		}
		else
		{	logicalDictSize -= sizeof(DictEntry);		//	Memory was full or handle locked.
			theErr = memFullErr;						//	Assume that memory was full...
		}
	}
	else
	{	theErr = memFullErr;							//	Memory was full or handle locked.
	}

	HSetState((Handle)this,state);			//	Restore the object lock state.
	return theErr ? theErr : entryIndex;	//	Return an error or the entry.
}

/*
**	Look for an entry and return it if found. Return
**	-1 if the entry is not found.
*/
tokentype	CStringDictionary::SearchForEntry(
	unsigned char	*entry,
	short			len)
{
	short			hashWalk;
	DictEntry		*thisEntry;
	unsigned char	*p1,*p2;
	
	if(len<0) len = *entry;
	if(len>255) len = 255;
	
	hashWalk = hashTable[HashString(entry+1,len)];	//	Find linked list head.
	while(hashWalk >= 0)							//	While there are items in the list.
	{	p1 = entry+1;
		thisEntry = hashWalk + *dictionary;
		p2 = thisEntry->nameOffset + *wordList;		//	Compare the entries:
		
		if(len == *p2++)							//	Compare length.
		{	int	i;
		
			i = len;
			while((i > 0) & (*p1++ == *p2++))		//	Compare characters.
				i--;
		
			if(i == 0)
				return hashWalk;					//	Found! Return index.
			else
				hashWalk = thisEntry->hashLink;		//	Not found, look at next entry.
		}
		else
		{	hashWalk = thisEntry->hashLink;			//	Not found, look at next entry.
		}
	}

	return -1;		//	Return -1 when entry was not found.
}

/*
**	Look for an entry and if doesn't exist, create it.
**	This routine only returns an error if there is a
**	problem with memory management.
*/
tokentype	CStringDictionary::FindEntry(
	unsigned char 	*entry,
	short			len)
{
	tokentype	theToken;
	
	theToken = SearchForEntry(entry,len);
	if (theToken >= 0) 
		return theToken;
	else
		return AddDictEntry(entry,len);
}

/*
**	Unlock and dispose storage.
*/
void	CStringDictionary::Dispose()
{
	ForceUnlock();
	DisposHandle(wordList);
	DisposHandle(dictionary);

	inherited::Dispose();
}

/*
**	Lock everything.
*/
void	CStringDictionary::Lock()
{
	if(!lockCounter)
	{	HLock(wordList);
		HLock(dictionary);
	}
	inherited::Lock();

}
/*
**	Unlock everything.
*/
void	CStringDictionary::Unlock()
{
	if(lockCounter == 1)
	{	HUnlock(wordList);
		HUnlock(dictionary);
	}
	inherited::Unlock();
}

/*
**	Read the dictionary string from a STR# resource.
*/
void	CStringDictionary::ReadFromStringList(
	short strListID)
{
	Str255	theString;
	short	stringCount,i;
	
	stringCount = **(short **)GetResource('STR#',strListID);
	for(i=1;i<=stringCount;i++)
	{	GetIndString(theString,strListID,i);
		AddDictEntry(theString, *theString);
	}
}
