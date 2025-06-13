// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Tools.h

#define __TOOLS__

void InitUTools();

void MyGetIndString(CStr255 &s, short id);
void MyGetIndString(CStr255 &s, short id, ResType theType);
short MyCountIndStrings(short rsrcID, ResType theType = 'STR#');

// returns index
short SubstituteOneStringItem(CStr255 &string, const char *marker, const CStr255 &substString);
short SubstituteStringItems(CStr255 &string, const CStr255 &marker, const CStr255 &substString);
short SubstituteStringItems(CStr255 &string, const char *marker, const CStr255 &substString);
short SubstituteStringItems(CStr255 &string, const char *marker, const char *substString);
short SubstituteStringItems(CStr255 &string, const char *marker, short number);
short SubstituteStringItems(CStr255 &string, const char *marker, long number);

void VersionToString(long version, CStr255 &text);

void AppendStringToHandle(const CStr255 &text, Handle h);
void AppendStringToHandle(const char *p, Handle h);
void CopyCString2String(const CStr255 &src, StringPtr dest);

Boolean CStr255HasChar(const CStr255 &s, char ch);
short CStr255CharPos(const CStr255 &s, char ch);
short CStr255CharPos(const CStr255 &s, char ch, short maxPos);
short CStr255CharPosBackwards(const CStr255 &s, char ch, short startPos);
short StrCmp(const CStr255 &item1, const CStr255 &item2); // see UList.h for return value
Boolean StrEqual(const CStr255 &item1, const CStr255 &item2);

void Long2Hex(long l, CStr255 &s);
const char *OSType2String(OSType ot);

void CopyDynamicArray(TDynamicArray *from, TDynamicArray *to);

#if qDebug
void BytesMove(const void *src, void *dest, long size);// handles >32K
#else
inline void BytesMove(const void *src, void *dest, long size)
{
	memcpy(dest, src, (unsigned int)size);
}
#endif

#if qDebug
void MyBlockMove(const void *src, void *dest, long size);// handles >32K
#else
inline void MyBlockMove(const void *src, void *dest, long size)
{
	memmove(dest, src, (unsigned int)size);
}
#endif
inline void BlockSet(void *p, long byteCount, unsigned char setVal)
{
	BlockSet(Ptr(p), byteCount, setVal);
}

void p2cstrcpy(char *d, const CStr255 &s);

// still not const in MacApp
void MyInsertElementsBefore(TDynamicArray *array, ArrayIndex index,
												const void* ElementPtr, ArrayIndex count);
TLongintList *NewLongintList();
void AppendLongintListWithZeros(TLongintList *list, long newSize);
Handle CloneHandle(Handle h);
