// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// StreamTools.h

#define __STREAMTOOLS__

void MyStreamReadHandle(TStream *aStream, Handle h);
void MyStreamWriteHandle(TStream *aStream, Handle h);
long MyStreamSizeOfHandle(Handle h);
void DoIronAgeFormatReadHandle(TStream *aStream, Handle h);


void ReadDynamicArray(TStream *aStream, TDynamicArray *array);
void WriteDynamicArray(TStream *aStream, TDynamicArray *array);
long MyStreamSizeOfDynamicArray(TDynamicArray *array);

void MyStreamCheckVersion(long theVersion, long minVersion, long maxVersion, const char *debugName);

Boolean MyCheckVersion(long theVersion, long minVersion, long maxVersion, const char *debugName);
// to old: return false, OK: return true, too new: failure

