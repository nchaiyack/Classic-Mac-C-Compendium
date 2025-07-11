/*
	List.h
	
	Routines for maintaining ordered linked lists
	
	For Graphic Elements release version 1.0b1

	Copyright 1993, 1994 by Al Evans -- all rights reserved
	
	3/7/94
		
*/

#ifndef LLISTS
#define LLISTS

typedef struct LMember *LMemberPtr;

typedef struct LMember {
	LMemberPtr		next;
	long			data;
} LMember;

typedef struct LHeader *LHeaderPtr;

typedef struct LHeader {
	LMemberPtr		listHead;
	LMemberPtr		free;
	LHeaderPtr		nextChunk;
	short			entrySize;
	short			chunkSize;
} LHeader;

typedef Boolean (*CompareProc) (Ptr p1, Ptr p2);

#ifdef __cplusplus
extern "C" {
#endif

LHeaderPtr InitList(short chunkSize, short entrySize);

LMemberPtr AllocateEntry(LHeaderPtr thisList);

void InsertEntry(LHeaderPtr thisList, LMemberPtr thisMember, CompareProc goesAfter);

void DeleteEntry(LHeaderPtr thisList, LMemberPtr thisMember);

void ClearList(LHeaderPtr thisList);

void DeleteList(LHeaderPtr thisList);

#ifdef __cplusplus
}
#endif

#endif




