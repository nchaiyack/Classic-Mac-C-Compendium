// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UBufferedFileStream.h

#define __UBUFFEREDFILESTREAM__

class TBufferedFileStream : public TStream
{
	public:
		virtual pascal long GetPosition();
		virtual pascal void SetPosition(long newPosition);
	
		virtual pascal long GetSize();
		virtual pascal void SetSize(long newSize);
	
		virtual pascal void ReadBytes(void* p, long count); 
		virtual pascal void WriteBytes(const void* p, long count); 

		virtual void FlushBuffers();
		
		virtual pascal void Initialize();
		pascal void IBufferedFileStream(TFile* itsFile, long inBufferSize, long outBufferSize);
		virtual pascal void Free();
	private:
		TFile *fFile;
		long fInBufferSize, fOutBufferSize;
		Ptr fInBufferP, fOutBufferP;
		Ptr fInBufferPosP, fOutBufferPosP;
		long fBytesInInBuffer, fBytesInOutBuffer;
		ParamBlockRec *fPBP;
		
		void FlushOutputBuffer();
		void FlushInputBuffer();
		void FillInputBuffer();
};

TBufferedFileStream *NewBufferedFileStream(TFile* itsFile, long inBufferSize, long outBufferSize);
