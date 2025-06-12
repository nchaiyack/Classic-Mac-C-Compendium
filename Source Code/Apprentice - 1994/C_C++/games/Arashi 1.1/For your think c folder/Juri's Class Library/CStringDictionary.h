/*/
     Project Arashi: CStringDictionary.h
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, June 22, 1993, 1:44
     Created: Friday, December 4, 1992, 11:44

     Copyright © 1992-1993, Juri Munkki
/*/

/*
**	This class maintains a sort of dictionary of words. The idea
**	is to have a fast way to find a dictionary word. A hashed
**	table is used to speed up the searches while still keeping
**	this class fairly simple.
**
**	NOTE:
**		While data is being added to the object, the
**		object can not be locked.
*/
#pragma once
#include "CBaseObject.h"

#define	DICTIONARYCLUMPSIZE		(16*sizeof(DictEntry))
#define	WORDCLUMPSIZE			256
#define HASHTABLESIZE			128	/* Must be a power of 2	*/

/*
**	The amount of storage required for a single item is
**	determined by the token type. A tokentype of "short"
**	will allow 32767 dictionary entries, which should be
**	sufficient for the kinds of data that this class was
**	written for.
*/
typedef	short	tokentype;

/*
**	Dictionary entries are stored into two separate handles.
**	The other one is the list of words and the other one
**	a list of hash table links and offsets to the words.
*/
typedef struct
{
	long	nameOffset;
	short	hashLink;
} DictEntry;

class	CStringDictionary : public CBaseObject
{
public:
				/*	Variables:			*/
	short		dictCount;				//	Amount of dictionary entries.
	long		logicalDictSize;		//	Used memory of dictionary handle.
	long		realDictSize;			//	Actual size of dictionary handle.
	DictEntry	**dictionary;			//	Handle to dictionary entries.
	
	long		logicalWordListSize;	//	Used memory of word list handle.
	long		realWordListSize;		//	Actual size of word list handle.
	unsigned char **wordList;			//	Word list.
		
	tokentype	hashTable[HASHTABLESIZE];	//	A hash table is used to accelerate lookups.

				/*	Methods:			*/	
	void		IStringDictionary();
	tokentype	AddDictEntry(unsigned char *entry, short len);
	tokentype	FindEntry(unsigned char *entry, short len);
	tokentype	SearchForEntry(unsigned char *entry, short len);
	void		ReadFromStringList(short strListID);
	
	short		GetDictionarySize();
	void		GetIndEntry(short index, StringPtr theEntry);
	
	void		Dispose();
	void		Lock();
	void		Unlock();
};