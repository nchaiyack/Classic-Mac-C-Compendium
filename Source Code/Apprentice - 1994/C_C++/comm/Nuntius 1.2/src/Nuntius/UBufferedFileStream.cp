// Copyright � 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UBufferedFileStream.cp

#include "UBufferedFileStream.h"
#include "FileTools.h"
#include "Tools.h"

#include <Errors.h>

#pragma segment MyTools

pascal void TBufferedFileStream::Initialize()
{
	inherited::Initialize();
	fFile = nil;
	fInBufferP = nil;
	fOutBufferP = nil;
	fInBufferPosP = nil;
	fOutBufferPosP = nil;
	fBytesInInBuffer = 0;
	fBytesInOutBuffer = 0;
	fPBP = nil;
}
	
pascal void TBufferedFileStream::IBufferedFileStream(TFile* itsFile, long inBufferSize, long outBufferSize)
{
	inherited::IStream();
	FailNonObject(itsFile);
	fFile = itsFile;
	fPBP = (ParamBlockRec*)NewPermPtr(sizeof(ParamBlockRec));
	fInBufferSize = inBufferSize;
	fOutBufferSize = outBufferSize;
	if (fInBufferSize)
	{
		fInBufferP = NewPermPtr(inBufferSize);
		fInBufferPosP = fInBufferP;
	}
	if (fOutBufferSize)
	{
		fOutBufferP = NewPermPtr(outBufferSize);
		fOutBufferPosP = fOutBufferP;
	}
	FailOSErr(fFile->SetDataMark(0, fsFromStart));
}

pascal void TBufferedFileStream::Free()
{
	FlushBuffers();
	fInBufferP = DisposeIfPtr(fInBufferP);
	fOutBufferP = DisposeIfPtr(fOutBufferP);
	DisposeIfPtr(Ptr(fPBP)); fPBP = nil;
	inherited::Free();
}

void TBufferedFileStream::FlushBuffers()
{
	FlushInputBuffer();
	FlushOutputBuffer();
}

void TBufferedFileStream::FlushOutputBuffer()
{
	if (fOutBufferP == nil)
		return;
	if (fBytesInOutBuffer == 0)
		return;
	WriteASyncToFile(*fPBP, fFile, fOutBufferP, fBytesInOutBuffer);
	fOutBufferPosP = fOutBufferP;
	fBytesInOutBuffer = 0;
}

void TBufferedFileStream::FlushInputBuffer()
{
	if (fBytesInInBuffer)
		FailOSErr(fFile->SetDataMark(-fBytesInInBuffer, fsFromMark)); // rewind
	fBytesInInBuffer = 0;
}

void TBufferedFileStream::FillInputBuffer()
{
	FailNIL(fInBufferP);
	long noReadBytes = fInBufferSize;
	ReadASyncFromFile(*fPBP, fFile, fInBufferP, noReadBytes);
#if qDebug & 0
	fprintf(stderr, "Read %ld bytes async:\n", noReadBytes);
	for (long i = 0; i <= 20 && i <= noReadBytes; ++i)
		fprintf(stderr, "%6ld: %ld\n", i, long(fInBufferP[i]));
#endif
	fBytesInInBuffer = noReadBytes;
	fInBufferPosP = fInBufferP;
}

pascal long TBufferedFileStream::GetPosition()
{
	FlushBuffers();
	long itsDataMark;
	FailOSErr(fFile->GetDataMark(itsDataMark));
	return itsDataMark;
}

pascal void TBufferedFileStream::SetPosition(long newPosition)
{
	FlushBuffers();
	FailOSErr(fFile->SetDataMark(newPosition, fsFromStart));
}

pascal long TBufferedFileStream::GetSize()
{
	FlushOutputBuffer();
	long itsDataLength;
	FailOSErr(fFile->GetDataLength(itsDataLength));
	return itsDataLength;
}

pascal void TBufferedFileStream::SetSize(long newSize)
{
	FlushBuffers();
	FailOSErr(fFile->SetDataLength(newSize));
}	

pascal void TBufferedFileStream::ReadBytes(void* p, long count)
{
	if (!fInBufferP)
		FailNIL(fInBufferP);
	Ptr destP = Ptr(p);
	while (true)
	{
		long subCount = Min(count, fBytesInInBuffer);
		BytesMove(fInBufferPosP, destP, subCount);
		fBytesInInBuffer -= subCount;
		fInBufferPosP += subCount;
		destP += subCount;
		count -= subCount;
		if (!count)
			break;
		if (count > fInBufferSize)
		{
			subCount = count;
			ReadASyncFromFile(*fPBP, fFile, destP, subCount);
			return;
		}
		FillInputBuffer();
		if (!fBytesInInBuffer)
			FailOSErr(eofErr);
	}
}

pascal void TBufferedFileStream::WriteBytes(const void* p, long count)
{
	if (!fOutBufferP)
		FailNIL(fOutBufferP);
	const char *fromP = (const char*)p;
	while (count)
	{
		long subCount = Min(count, fOutBufferSize - fBytesInOutBuffer);
		BytesMove(fromP, fOutBufferPosP, subCount);
		fBytesInOutBuffer += subCount;
		fOutBufferPosP += subCount;
		fromP += subCount;
		count -= subCount;
		if (!count)
			break;
		FlushOutputBuffer();
		if (count > fOutBufferSize)
		{
			WriteASyncToFile(*fPBP, fFile, fromP, count);
			return;
		}
	}
}

//--------------

TBufferedFileStream *NewBufferedFileStream(TFile* itsFile, long inBufferSize, long outBufferSize)
{
	TBufferedFileStream *bfs = new TBufferedFileStream();
	bfs->IBufferedFileStream(itsFile, inBufferSize, outBufferSize);
	return bfs;
}
