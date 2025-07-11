// Copyright � 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UBinDecoders.h

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif


const long kOutBufferSize = 4 * 1024;

class PBinaryDecoder : public PPtrObject
{
	public:
		virtual void DecodeLine(const CStr255 &line) = 0;
		virtual Boolean IsDoneNow() = 0;
		virtual void PostProcess();
		virtual void Abort();
		
		PBinaryDecoder();
		void IBinaryDecoder(TFile *outFile);
		~PBinaryDecoder();
	protected:
		TFile *fFile;
		ParamBlockRec fPB;
		
		void OpenFork(Boolean dataFork); // for rsrc: will act as raw data fork in fFile 
		void CloseFork();
		void WriteBytes(const void *p, long noBytes);
	private:
		Ptr fOutBufferP, fOutBufferPosP;
		long fBytesFreeInOutBuffer;
		
		void FlushOutBuffer();
};


class PUUDecoder : public PBinaryDecoder
{
	public:
		void DecodeLine(const CStr255 &line);
		Boolean IsDoneNow();
		void PostProcess();
		void PrepareUU();

		PUUDecoder();
		void IUUDecoder(TFile *outFile);
		~PUUDecoder();
	private:
		enum {kInsideIt, kWaitForDelim, kAtEnd} fState;
		Boolean fGotUUDelemiter;
		Boolean fFirstUULineInBlock;
		
		void ProcessLine(CStr255 &text);
		Boolean IsValidUUEncodeLine(CStr255 &text);
};

const kBinHexBinaryBufferSize = 4 * 1024;
class PBinHexDecoder : public PBinaryDecoder
{
	public:
		void DecodeLine(const CStr255 &line);
		Boolean IsDoneNow();
		void PostProcess();

		PBinHexDecoder();
		void IBinHexDecoder(TFile *outFile);
		~PBinHexDecoder();
	private:
		enum {kAtStart, kAtEnd, kWaitForStartColon, kInsideIt, kWaitForDelim} fAsciiState;
		enum {kInHeader, kInDataFork, kInRsrcForm, kPastEnd} fBinaryState;
		
		short f6To8Index;
		char fAcc, fLastChar;
		Boolean fRunFlag;
		void ProcessAsciiLine(char *p);
		void ProcessAsciiChar(unsigned char ch);
		//
		Ptr fBinaryBufferP, fBinaryBufferPosP;
		long fBytesInBinaryBuffer;
		void StoreBinaryChar(unsigned char ch);
		void EatenBinaryData(long noBytes);
		//
		unsigned short fFinderFlags;
		long fDataLen, fRsrcLen;
		short fCRC;
		long fForkBytesLeft;
		Boolean ProcessHeaderData();
		Boolean ProcessForkData();
		void CalcCRC(const void *p, long noBytes);
		void CheckCRC();
};