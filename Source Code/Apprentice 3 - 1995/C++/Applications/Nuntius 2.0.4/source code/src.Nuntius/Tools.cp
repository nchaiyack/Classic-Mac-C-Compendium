// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Tools.cp

#include "Tools.h"
#include "FileTools.h"
#include "UFatalError.h"

#include <Resources.h>
#include <Packages.h>
#include <Errors.h>
#include <ToolUtils.h>
#include <stdio.h>

#pragma segment MyTools

void InitUTools()
{
}

Handle GetStringResource(ResType theType, short id)
{
	// some sun days, you just can't avoid optimizing stupid things
	struct entry
	{
		Handle mH;
		ResType mType;
		short mID;
	};
	static const kBufferSize = 50;
	static entry sAllEntries[kBufferSize + 1];
	static long gNextFreeIndex = 0;
	entry *entryP = sAllEntries;
	for (long i = gNextFreeIndex; i; --i, ++entryP)
	{
		if (entryP->mID == id && entryP->mType == theType)
			return entryP->mH;
	}
	Handle h = GetResource('STR#', id);
	FailNILResource(h);
	FailNIL(h);
	if (gNextFreeIndex < kBufferSize)
	{
		HLockHi(h);
		entryP->mID = id;
		entryP->mType = theType;
		entryP->mH = h;
		gNextFreeIndex++;
	}
	return h;
}

void MyGetIndString(CStr255 &s, short id, ResType theType)
{
#if qDebug
	if (id < 200)
	{
		fprintf(stderr, "MyGetIndString, id = %ld\n", long(id));
		ProgramBreak(gEmptyString);
	}
#endif
	s = "";
	const short kListSize = 20;
	short rsrcID = (id / kListSize) * kListSize;
	short index = id % kListSize;
	Handle h = GetStringResource(theType, rsrcID);
	short maxIndex = *( (short*) *h);
	if (index < 1 || index > maxIndex)
	{
		sprintf(gPanicBuffer, "MyGetIndString: string %ld nonexistent, type = %s\n", id, OSType2String(theType));
		ReportFatalError(gPanicBuffer);
		return;
	}
	unsigned char *p = (unsigned char *)*h + 2;
	for (short i = 1; i < index; i++)
		p += *p + 1;
	memcpy(&s, p, *p + 1);
}

void MyGetIndString(CStr255 &s, short id)
{
	MyGetIndString(s, id, 'STR#');
}

short MyCountIndStrings(short rsrcID, ResType theType)
{
	Handle h = GetStringResource(theType, rsrcID);
	return *( (short*) *h);
}

void VersionToString(long version, CStr255 &text)
{
	NumToString((version >> 8) & 0xFFFFFF, text);
	CStr255 s;
	NumToString((version >> 4) & 0xF, s);
	text += "." + s;
	NumToString(version & 0xF, s);
	text += "." + s;
}

short CStr255CharPos(const CStr255 &s, char ch)
{
	unsigned char uch = (unsigned char)ch;
	for (short i = 1; i <= s.Length(); ++i)
		if (s[i] == uch)
			return i;
	return 0;
}

Boolean CStr255HasChar(const CStr255 &s, char ch)
{
	if (!s.Length())
		return false;
	const unsigned char *p = (const unsigned char*)&s;
	unsigned char uch = (unsigned char)ch;
	for (short i = s.Length(); i; --i)
		if (*p++ == uch)
			return true;
	return false;
}

short CStr255CharPos(const CStr255 &s, char ch, short maxPos)
{
	unsigned char uch = (unsigned char)ch;
	short mmax = short(Min(maxPos, s.Length()));
	for (short i = 1; i <= mmax; ++i)
		if (s[i] == uch)
			return i;
	return 0;
}

short CStr255CharPosBackwards(const CStr255 &s, char ch, short startPos)
{
	unsigned char uch = (unsigned char)ch;
	short mmax = short(Min(startPos, s.Length()));
	for (short i = mmax; i; --i)
		if (s[i] == uch)
			return i;
	return 0;
}

short StrCmp(const CStr255 &item1, const CStr255 &item2) // see UList.h for return value
{
	short i = 1;
	short maxI = short(Min(item1.Length(), item2.Length()));
	while (i <= maxI)
	{
		unsigned char ch1 = item1[i];
		unsigned char ch2 = item2[i];
		if (ch1 < ch2)
			return kItem1LessThanItem2;
		else if (ch1 > ch2)
			return kItem1GreaterThanItem2;
		++i;
	}
	// got past one of the strings
	if (item1.Length() == item2.Length())
		return kItem1EqualItem2;
	--i; // point at last common char
	if (i < item1.Length()) // not past end of string
		return kItem1GreaterThanItem2;
#if qDebug
	if (i == item2.Length())
	{
		fprintf(stderr, "Bad stop in StrCmp: item1= '%s', item2 = '%s'\n", (char*)item1, (char*)item2);
		ProgramBreak(gEmptyString);
	}
#endif
	return kItem1LessThanItem2;
}

Boolean StrEqual(const CStr255 &item1, const CStr255 &item2)
{
	const unsigned char *p1 = (const unsigned char *)&item1;
	const unsigned char *p2 = (const unsigned char *)&item2;
	if (*p1++ != *p2++) /// compare length
		return false;
	for (short i = item1.Length(); i; --i)
		if (*p1++ != *p2++)
			return false;
	return true;
}

void CopyDynamicArray(TDynamicArray *from, TDynamicArray *to)
{
#if qDebug
	if (!IsObject(from))
		ProgramBreak("from is not object");
	if (!IsObject(to))
		ProgramBreak("to is not object");
	if (from->fElementSize != to->fElementSize)
		ProgramBreak("from->fElementSize != to->fElementSize");
#endif
	long noElem = from->GetSize();
	to->SetArraySize(noElem);
	to->fSize = noElem;
	if (noElem)
	{
		Ptr p1 = from->ComputeAddress(1);
		Ptr p2 = to->ComputeAddress(1);
		BytesMove(p1, p2, noElem * from->fElementSize);
	}
}

short SubstituteOneStringItem(CStr255 &string, const char *marker, const CStr255 &substString)
{
	short pos = string.Pos(marker);
	if (pos)
	{
		string.Delete(pos, strlen(marker));
		string.Insert(substString, pos);
	}
	return pos;
}

short SubstituteStringItems(CStr255 &string, const CStr255 &marker, const CStr255 &substString)
{
	short pos = string.Pos(marker);
	short pos1 = pos;
	while (pos)
	{
		string.Delete(pos, marker.Length());
		string.Insert(substString, pos);
		pos = string.Pos(marker);
	}
	return pos1;
}

short SubstituteStringItems(CStr255 &string, const char *marker, const CStr255 &substString)
{
	short pos = string.Pos(marker);
	short pos1 = pos;
	while (pos)
	{
		string.Delete(pos, strlen(marker));
		string.Insert(substString, pos);
		pos = string.Pos(marker);
	}
	return pos1;
}

short SubstituteStringItems(CStr255 &string, const char *marker, const char *substString)
{
	short pos = string.Pos(marker);
	short pos1 = pos;
	while (pos)
	{
		string.Delete(pos, strlen(marker));
		string.Insert(substString, pos);
		pos = string.Pos(marker);
	}
	return pos1;
}

short SubstituteStringItems(CStr255 &string, const char *marker, long number)
{
	CStr255 s;
	NumToString(number, s);
	return SubstituteStringItems(string, marker, s);
}

short SubstituteStringItems(CStr255 &string, const char *marker, short number)
{
	return SubstituteStringItems(string, marker, long(number));
}

#if qDebug
void BytesMove(const void *src, void *dest, long size)
{
	if (size < 0)
	{
		ProgramBreak("BytesMove: size < 0");
		return;
	}
	memcpy(dest, src, (unsigned int)size);
}
#endif

#if qDebug
void MyBlockMove(const void *src, void *dest, long size)
{
	if (size < 0)
	{
		ProgramBreak("MyBlockMove: size < 0");
		return;
	}
	memmove(dest, src, (unsigned int)size);
}
#endif

void CopyCString2String(const CStr255 &src, StringPtr dest)
{
	const unsigned char *ucp = (const unsigned char*) &src;
	BytesMove(ucp, dest, *ucp + 1);
}

void AppendStringToHandle(const CStr255 &text, Handle h)
{
	if (text.Length())
	{
		long oldSize = GetHandleSize(h);
		SetPermHandleSize(h, oldSize + text.Length());
		const unsigned char *ucp = (const unsigned char*) &text;
		BytesMove(ucp + 1, *h + oldSize, text.Length());
	}
}

void AppendStringToHandle(const char *p, Handle h)
{
	long textLen = strlen(p);
	if (textLen)
	{
		long oldSize = GetHandleSize(h);
		SetPermHandleSize(h, oldSize + textLen);
		BytesMove(p, *h + oldSize, textLen);
	}
}

void Long2Hex(long l, CStr255 &s)
{
	s.Length() = 8;
	for (short i = 8; i > 0; i--)
	{
		s[i] = "0123456789ABCDEF"[l & 15];
		l >>= 4;
	}
}

const char *OSType2String(OSType ot)
{
	const short kTempCStrings = 4;
	static short currentCString = 0;
	static char cStrings[kTempCStrings][8];
	currentCString = (currentCString + 1) % kTempCStrings;
	char *p = cStrings[currentCString];
	*(long*)p = ot;
	p[5] = 0;
	return p;
}

void MyInsertElementsBefore(TDynamicArray *array, ArrayIndex index,
												const void* ElementPtr, ArrayIndex count)
{
	Ptr indexPtr,  nextIndexPtr, lastElementPtr;
	ArrayIndex countBytes;
	if (qRangeCheck && ((index < 1) || (index > array->fSize + 1)))
	{
		fprintf(stderr, "fSize == %1d  index == %1d\n", array->fSize, index);
		ProgramBreak("Range Check in TDynamicArray.InsertBefore");
	}
	array->SetArraySize(array->fSize + count);				// make sure there's room if needed 
	indexPtr = array->ComputeAddress(index);
	nextIndexPtr = array->ComputeAddress(index + count);
	lastElementPtr = array->ComputeAddress(array->fSize + 1);
	countBytes = count << array->fElementSizeShift;
	if (index <= array->fSize)							// clear out a hole? 
		BlockMove(indexPtr, nextIndexPtr, (long)lastElementPtr - (long)indexPtr);
	// !!! we still don't account for multiple element moves with non power of 2 element sizes.
	// Would it be best to create a MoveElements method and put the smarts in it?
	if (ElementPtr)
	{
		if ((countBytes == sizeof(long)) &&!odd((long)ElementPtr) &&!odd((long)indexPtr))
			*((long*)indexPtr) = *((const long*)ElementPtr);// shortcut for longs 
		else
			BlockMove(ElementPtr, indexPtr, countBytes);// longcut for shorts (and other sizes) 
	}
	array->fSize += count;
	if (array->fIteratorPtr)							// Keep all iterations apprised
		array->fIteratorPtr->InsertElementBefore(index, count);
}

TLongintList *NewLongintList()
{
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	return lList;
}

void p2cstrcpy(char *d, const CStr255 &s)
{
	const unsigned char *sp = (const unsigned char *)&s;
	short len = *sp++;
	while (len)
	{
		*d++ = *sp++;
		--len;
	}
	*d = 0;
}

void AppendLongintListWithZeros(TLongintList *list, long newSize)
{
	ArrayIndex oldSize = list->fSize;
#if qDebug
	if (newSize < oldSize)
		fprintf(stderr, "Warning: AppendLongintListWithZeros, size = %ld < newSize = %ld\n", oldSize, newSize);
#endif
	if (newSize <= oldSize)
		return;
	if (newSize > list->fAllocatedSize)
		list->SetArraySize(newSize);
	list->fSize = newSize;
	BlockSet(list->ComputeAddress(oldSize + 1), (newSize - oldSize) * sizeof(long), 0);
}

Handle CloneHandle(Handle h)
{
	long size = GetHandleSize(h);
	Handle newH = NewPermHandle(size);
	BytesMove(*h, *newH, size);
	return newH;
}
