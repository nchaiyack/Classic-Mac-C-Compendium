// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UBinDecoders.cp

#include "UBinDecoders.h"
#include "FileTools.h"
#include "Tools.h"
#include "UPrefsDatabase.h"

#include <ErrorGlobals.h>

#pragma segment MyArticle

// errNoExtractor may be deleted now


#define qDebugBinHexAscii qDebug & 0
#define qDebugBinHexRun		qDebug & 0


#if qDebug
#define DD(x) fprintf(stderr, x)
#else
#define DD(x)
#endif

#if qDebugExtract
#define DDE(x) fprintf(stderr, x)
#else
#define DDE(x)
#endif

#if qDebug
short gDebugLineNo = 0;
void DebugDumpLine(const CStr255 &text)
{
	fprintf(stderr, "Line %hd is '%s'\n", gDebugLineNo, (char*)text);
}
#else
#define DebugDumpLine(x)
#endif

//-------------------------------------------------------------------
PBinaryDecoder::PBinaryDecoder()
{
	fOutBufferP = nil;
	fOutBufferPosP = nil;
	fFile = nil;
}

void PBinaryDecoder::IBinaryDecoder(TFile *outFile)
{
	FailInfo fi;
	if (fi.Try())
	{
		fOutBufferP = NewPermPtr(kOutBufferSize);
		fOutBufferPosP = fOutBufferP;
		fBytesFreeInOutBuffer = kOutBufferSize;
		FailNIL(outFile);
		fFile = outFile;
		fi.Success();
	}
	else // fail
	{
		FreeIfPtrObject(this);
	}
}

PBinaryDecoder::~PBinaryDecoder()
{
	DisposeIfPtr(fOutBufferP);
}

void PBinaryDecoder::FlushOutBuffer()
{
	long numBytes = fOutBufferPosP - fOutBufferP;
	if (numBytes)
	{
		WriteASyncToFile(fPB, fFile, fOutBufferP, numBytes);
		fOutBufferPosP -= numBytes;
		fBytesFreeInOutBuffer += numBytes;
	}
}

void PBinaryDecoder::WriteBytes(const void *p, long noBytes)
{
	const char *fromP = (const char*)p;
	while (noBytes)
	{
		long subBytes = Min(noBytes, fBytesFreeInOutBuffer);
		BytesMove(fromP, fOutBufferPosP, subBytes);
		noBytes -= subBytes;
		fromP += subBytes;
		fOutBufferPosP += subBytes;
		fBytesFreeInOutBuffer -= subBytes;
		if (!fBytesFreeInOutBuffer)
			FlushOutBuffer();
	}
}

void PBinaryDecoder::OpenFork(Boolean dataFork)
{
	FlushOutBuffer();
	FailOSErr(fFile->CloseDataFork());
	if (dataFork)
	{
		FailOSErr(fFile->OpenDataFork(fsRdWrPerm));
		return;
	}
	FSSpec spec;
	fFile->GetFileSpec(spec);
	short refNum;
	FailOSErr(FSpOpenRF(spec, fsRdWrPerm, refNum));
	fFile->fDataRefNum = refNum;
}

void PBinaryDecoder::CloseFork()
{
	FlushOutBuffer();
	FailOSErr(fFile->CloseFile());
}

void PBinaryDecoder::Abort()
{
//@ should probably catch the errors here
	FailOSErr(fFile->CloseDataFork());
	if (FileExist(fFile))
		FailOSErr(fFile->DeleteFile());
}

void PBinaryDecoder::PostProcess()
{
	FlushOutBuffer();
	FailOSErr(fFile->CloseDataFork());
}

#if 0
void PBinaryDecoder::DecodeLine(const CStr255 & /* line */)
{
	SubClassResponsibility();
}

Boolean PBinaryDecoder::IsDoneNow()
{
	SubClassResponsibility();
	return true;
}
#endif
//========================================================================
unsigned char uu_table_1_1[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
	 64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
	128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
	192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
	  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
	 64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124};

unsigned char uu_table_1_2[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
	  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1};

unsigned char uu_table_2_1[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
	  0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240};

unsigned char uu_table_2_2[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   1,   1,   1,   1,   2,   2,   2,   2,   3,   3,   3,   3,
	  4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,
	  8,   8,   8,   8,   9,   9,   9,   9,  10,  10,  10,  10,  11,  11,  11,  11,
	 12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,  15,  15,  15,
	  0,   0,   0,   0,   1,   1,   1,   1,   2,   2,   2,   2,   3,   3,   3,   3,
	  4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7};

unsigned char uu_table_3_1[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,
	  0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192,   0,  64, 128, 192};

unsigned char uu_table_3_2[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31};

PUUDecoder::PUUDecoder()
{
//	inherited::Initialize();
	fGotUUDelemiter = false;
	fFirstUULineInBlock = true;
	fState = kInsideIt;
}

void PUUDecoder::IUUDecoder(TFile *outFile)
{
	IBinaryDecoder(outFile);
}

PUUDecoder::~PUUDecoder()
{
}

Boolean PUUDecoder::IsDoneNow()
{
//@	return false; // nothing is certain with uuencode
	return (fState == kAtEnd);
}

void PUUDecoder::PostProcess()
{
	PBinaryDecoder::PostProcess();
	if (fState != kAtEnd)
		if (fGotUUDelemiter || fState != kInsideIt) // special case for bad posed uuencoded files
			FailOSErr(errBadBinaryFile);
}

void PUUDecoder::PrepareUU()
{
	OpenFork(true);
}

void PUUDecoder::DecodeLine(const CStr255 &line)
{
	CStr255 text(line);
	ProcessLine(text);
}

void PUUDecoder::ProcessLine(CStr255 &text)
{
	// not const argument as I want to be able to add a few spaces
	unsigned char len = text.Length(); // easier to read
	char *p = (char*)&text[1];
	char binBuffer[200];
	switch (fState)
	{
		case kInsideIt:
			if (fFirstUULineInBlock && *p != 'M')
			{
				DDE("First line in block and non-'M' char as length -> kWaitForDelim\n");
				fState = kWaitForDelim;
				return;
			}
#if qDebug
			gDebugLineNo++;
#endif
#if qDebugExtract
			if (gDebugLineNo < 10)
			{
				DebugDumpLine(text);
			}
#endif
			if (!len)
			{
				DD("empty line found -> ignored\n");
				return;
			}
			fFirstUULineInBlock = false;
			if (*p == '`' && len == 1)
			{
				DDE("non-standard ` end symbol found -> kAtEnd\n");
				fState = kAtEnd; // bad 'end' found in some uuencoded files
				return;
			}
			if (*p == 32 && len == 1)
			{
				DDE("single-space real-way-to-terminate symbol found -> kAtEnd\n");
				fState = kAtEnd;
				return;
			}
			if (*p == 'E' && strncmp(p, "END", 3) == 0)
			{
				if (len <= 30) // E == 53 bytes
				{
					DDE("END delemiter found -> kWaitForDelim\n");
					fGotUUDelemiter = true;
					fState = kWaitForDelim;
					return;
				}
				if (!IsValidUUEncodeLine(text))
				{
					DD("bogous 'END' separator found -> kWaitForDelim\n");
					DebugDumpLine(text);
					fGotUUDelemiter = true;
					fState = kWaitForDelim;
					return;
				}
				DD("got line starting with 'END', but found no invalid chars (line is used)\n");
				DebugDumpLine(text);
			}
			if (*p == 'e' && strncmp(p, "end", 3) == 0)
			{
				DDE("lowercase end symbol found -> kAtEnd\n");
				fState = kAtEnd;
				return;
			}
			if (!IsValidUUEncodeLine(text))
				break;
			++p;
			short index = 0;
			unsigned char lineLen = text[1] - 32;
			long triplets = short(lineLen + 2) / 3;
			long rest = triplets;
			while (rest > 0)
			{
				unsigned char uc;
				uc = uu_table_1_1[*p++];
				uc += uu_table_1_2[*p];
				binBuffer[index++] = uc;
				uc = uu_table_2_1[*p++];
				uc += uu_table_2_2[*p];
				binBuffer[index++] = uc;
				uc = uu_table_3_1[*p++];
				uc += uu_table_3_2[*p++];
				binBuffer[index++] = uc;
				rest -= 1;
			}
			WriteBytes(binBuffer, lineLen);
			break;

		case kWaitForDelim:
			if (*p == 'B' && strncmp(p, "BEGIN" , 5) == 0)
			{
				DDE("BEGIN found -> kInsideIt\n");
				fState = kInsideIt;
				fFirstUULineInBlock = true;
			}
			else if (len >= 9 && strncmp(p, " CUT HERE", 9) == 0)
			{
				DDE("Buggy ' CUT HERE' found -> kInsideIt\n");
				fState = kInsideIt;
				fFirstUULineInBlock = true;
			}
			else if (*p == 'M' && (len >= 61 && len <= 63))
			{
				DDE("UUencoded stuff begins without starting separator:\n");
				DebugDumpLine(text);
				fState = kInsideIt;
				fFirstUULineInBlock = true;
				ProcessLine(text);
			}
			break;

		case kAtEnd:
			break;
			
#if qDebug
		default:
			fprintf(stderr, "Invalid fState == %ld\n", long(fState));
			ProgramBreak(gEmptyString);
#endif
	}
}

Boolean PUUDecoder::IsValidUUEncodeLine(CStr255 &text)
{
	unsigned char lineLen = text[1] - 32;
	if (lineLen < 0 || lineLen >= 64)
	{
		DD("bad line: lineLen < 0 || lineLen >= 64 -> ignored\n");
		DebugDumpLine(text);
		return false; // line too long or too short
	}
	if (!lineLen)
		return true;
	long triplets = short(lineLen + 2) / 3;
	long tripletsLen = triplets * 4;
	unsigned char len = text.Length();
	if (len > tripletsLen + 4)
	{
		DD("Too much garbage at end of line -> ignored\n");
		DebugDumpLine(text);
		return false;
	}
	if (tripletsLen > text.Length())
	{
#if qDebugExtract
		fprintf(stderr, "Found too few chars in line (eaten spaces case)\n");
		fprintf(stderr, " len = %ld, lineLen = %ld, tripletsLen = %ld, noTriplets = %ld\n", long(text.Length()), long(lineLen), tripletsLen, triplets);
		DebugDumpLine(text);
#endif			
		while (tripletsLen > text.Length())
			text += 32; // add eaten spaces
	}
	unsigned char *p = &text[2];
	--len;
	while (len > 0)
	{
		if (*p < 32 || *p > 96)
		{
#if qDebug
			fprintf(stderr, "Found invalid char (ascii = %ld) -> line ignored\n", *p);
			fprintf(stderr, "Line = %ld, char offset = %ld, '%s'\n", gDebugLineNo, p - &text[1], (char*)text);
			DebugDumpLine(text);
#endif
			return false; // found invalid char
		}
		++p;
		--len;
	}
	return true;
}
//=========================================================================
const char kBadBinHexChar = ' ';
unsigned char gBinHexTable[256];
Boolean gBinHexTableInited = false;

PBinHexDecoder::PBinHexDecoder()
{
//	inherited::Initialize();
	fBinaryBufferP = nil;
	fBinaryBufferPosP = nil;
	fBytesInBinaryBuffer = 0;
	fRunFlag = false;
	f6To8Index = 0;
	fForkBytesLeft = 0;
	fAsciiState = kAtStart;
	fBinaryState = kInHeader;
}

void PBinHexDecoder::IBinHexDecoder(TFile *outFile)
{
	IBinaryDecoder(outFile);
	FailInfo fi;
	if (fi.Try())
	{
		fAsciiState = kAtStart;
		fBinaryState = kInHeader;
		fBinaryBufferP = NewPermPtr(kBinHexBinaryBufferSize);
		fBinaryBufferPosP = fBinaryBufferP;
		fBytesInBinaryBuffer = 0;
		if (!gBinHexTableInited)
		{
			gBinHexTableInited = true;
			BlockSet(Ptr(gBinHexTable), 256, kBadBinHexChar);
			const char *p = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
			unsigned ch = 0;
			while (*p)
				gBinHexTable[*p++] = ch++;
		}			
		fi.Success();
	}
	else
	{
		FreeIfPtrObject(this);
	}
}
		
PBinHexDecoder::~PBinHexDecoder()
{
	DisposeIfPtr(fBinaryBufferP);
//	inherited::Free();
}

Boolean PBinHexDecoder::IsDoneNow()
{
	return (fAsciiState == kAtEnd);
}

void PBinHexDecoder::PostProcess()
{
	if (fAsciiState != kAtEnd || fBinaryState != kPastEnd)
		FailOSErr(errBadBinHex);
	if (fBytesInBinaryBuffer || fRunFlag || fForkBytesLeft)
		FailOSErr(errBadBinHex);
	PBinaryDecoder::PostProcess();
}

void PBinHexDecoder::DecodeLine(const CStr255 &line)
{
	CStr255 text(line);
#if qDebugBinHexAscii
	fprintf(stderr, "Decoding line '%s'\n", (char*)text);
#endif
	// want to be able to modify it (punch a couple of holes)
	unsigned char len = text.Length(); // easier to read
	while (len && text[len] <= 32)
		len--;
	char *p = (char*)&text[1];
	switch (fAsciiState)
	{
		case kAtStart:
			if (len && strncmp(p, "(This file must be converted with BinHex 4.0", 40) == 0)
			{
				DDE("Found '(This file mustÉ' -> kWaitForStartColon\n");
				fAsciiState = kWaitForStartColon;
			}
			break;

		case kWaitForStartColon:
			if (len && *p == ':')
			{
				if (text[len] == ':')
				{
					DDE("Found starting and endning colon in line (shorty one) -> kAtEnd\n");
					fAsciiState = kAtEnd;
					text[len] = char(0);
				}
				else
				{
					DDE("Found starting colon -> kInsideIt\n");
					fAsciiState = kInsideIt;
					text += char(0);
				}
				ProcessAsciiLine(p + 1);
			}
			break;

		case kInsideIt:
			if (len >= 1 && text[len] == ':')
			{
				DDE("Found endning colon -> kAtEnd\n");
				fAsciiState = kAtEnd;
				text[len] = char(0);
				ProcessAsciiLine(p);
			}
			else if (len >= 15 && strncmp(p, "--- end of part", 15) == 0)
			{
				DDE("Found '--- end of part' -> kWaitForDelim\n");
				fAsciiState = kWaitForDelim;
			}
			else
			{
				text += char(0);
				ProcessAsciiLine(p);
			}
			break;

		case kWaitForDelim:
			if (len == 3 && strncmp(p, "---", 3) == 0)
			{
				DDE("Found '---' -> kInsideIt\n");
				fAsciiState = kInsideIt;
			}
			else if (len >= 4 && strncmp(p, "--- ", 4) == 0)
			{
				DDE("Found '--- É' -> kInsideIt\n");
				fAsciiState = kInsideIt;
			}
			break;

		case kAtEnd:
			break;
			
#if qDebug
		default:
			fprintf(stderr, "Invalid fAsciiState == %ld\n", long(fAsciiState));
			ProgramBreak(gEmptyString);
#endif
	}
}

void PBinHexDecoder::ProcessAsciiLine(char *p)
{
	while (*p)
	{
		ProcessAsciiChar( *(unsigned char*)p);
		p++;
	}
	while (fBytesInBinaryBuffer)
	{
		switch (fBinaryState)
		{
			case kInHeader:
				if (!ProcessHeaderData())
					return;
				break;

			case kInDataFork:
			case kInRsrcForm:
				if (!ProcessForkData())
					return;
				break;
			
			case kPastEnd:
				FailOSErr(errBadBinHex);
				
#if qDebug
			default:
				fprintf(stderr, "Invalid fBinaryState == %ld\n", long(fBinaryState));
				ProgramBreak(gEmptyString);
#endif
		}
	} // while
}

void PBinHexDecoder::ProcessAsciiChar(unsigned char ch)
{
	char sixBits = gBinHexTable[ch];
	if (sixBits == kBadBinHexChar)
		FailOSErr(errBadBinHex);
	unsigned char outChar;
	switch (f6To8Index)
	{
		case 0:
			fAcc = sixBits << 2;
			f6To8Index++;
			return;
			
		case 1:
			outChar = fAcc | (sixBits >> 4);
			fAcc = sixBits << 4;
			f6To8Index++;
			break;
		
		case 2:
			outChar = fAcc | (sixBits >> 2);
			fAcc = sixBits << 6;
			f6To8Index++;
			break;
		
		case 3:
			outChar = fAcc | sixBits;
			f6To8Index = 0;
			break;

#if qDebug
		default:
			fprintf(stderr, "Invalid f6To8Index == %ld\n", long(f6To8Index));
			ProgramBreak(gEmptyString);
#endif
	}
#if qDebugBinHexAscii & 0
	fprintf(stderr, "outChar = %ld, fRunFlag = %ld, fLastChar = %ld\n", long(outChar), long(fRunFlag), long(fLastChar));
#endif
	if (fRunFlag)
	{
		if (outChar == 0)
		{
#if qDebugBinHexRun
			fprintf(stderr, "RunFlag: got 0: stores 0x90\n");
#endif
			StoreBinaryChar(0x90);
			fLastChar = 0x90;
		}
		else
		{
#if qDebugBinHexRun
			fprintf(stderr, "RunFlag, got run-length = %ld, writes copies\n", long(outChar));
#endif
			for (short i = 2; i <= outChar; i++)
				StoreBinaryChar(fLastChar);
		}
		fRunFlag = false;
	}
	else if (outChar == 0x90)
	{
#if qDebugBinHexRun
		fprintf(stderr, "Got run byte: setting RunFlag\n");
#endif
		fRunFlag = true;
	}
	else
	{
		fLastChar = outChar;
		StoreBinaryChar(outChar);
	}
}

//---------------------------------------------
void PBinHexDecoder::StoreBinaryChar(unsigned char ch)
{
#if qDebugBinHexAscii
	if (ch >= 32)
		fprintf(stderr, "stores '%c' = $%lx\n", ch, long(ch)&0xFF);
	else
		fprintf(stderr, "stores 0x%lx\n", long(ch)&0xFF);
#endif
	if (fBytesInBinaryBuffer >= kBinHexBinaryBufferSize)
		FailOSErr(errBadBinHex); // just a check for not writing past buffer
	*fBinaryBufferPosP++ = ch;
	fBytesInBinaryBuffer++;
}

void PBinHexDecoder::EatenBinaryData(long noBytes)
{
	if (noBytes == fBytesInBinaryBuffer)
	{
		fBytesInBinaryBuffer = 0;
		fBinaryBufferPosP = fBinaryBufferP;
	}
	else
	{
		fBytesInBinaryBuffer -= noBytes;
		BytesMove(fBinaryBufferP + noBytes, fBinaryBufferP, fBytesInBinaryBuffer);
		fBinaryBufferPosP -= noBytes;
	}
}
const short kBinHexInitialCRC = 0;
//---------------------------------------------
typedef struct BinHexInfo
{
	OSType fFileType;
	OSType fCreator;
	unsigned short fdFlags;
	long fDataLength;
	long fRsrcLength;
};


Boolean PBinHexDecoder::ProcessHeaderData()
{
	if (!fBytesInBinaryBuffer)
		return false; // want something
	short nameLen = *(unsigned char*)fBinaryBufferP;
	if (fBytesInBinaryBuffer < 22 + nameLen)
		return false; // want hole header
	fCRC = kBinHexInitialCRC;
	// extract header
	Ptr p = fBinaryBufferP + 1;
	CStr255 filename(p);
	p += nameLen;
	if (*p++)
		FailOSErr(errBadBinHex);
	BinHexInfo bhi;
	BytesMove(p, &bhi, sizeof(bhi));
	CalcCRC(fBinaryBufferP, 20 + nameLen);
	EatenBinaryData(20 + nameLen);
	CheckCRC();
	// specify the file
	fFile->fUsesDataFork = false;
	fFile->fUsesRsrcFork = noResourceFork;
	FSSpec spec;
	gPrefs->GetSilentDirAliasPrefs('FBin', spec);
	CopyCString2String(filename, spec.name);
	fFile->Specify(spec);
	fFile->fCreator = bhi.fCreator;
	fFile->fFileType = bhi.fFileType;
	const short invMask = (1 << 0) | (1 << 8); // see TB 09 - Finder Flags
	fFinderFlags = bhi.fdFlags & ~invMask;
	fDataLen = bhi.fDataLength;
	fRsrcLen = bhi.fRsrcLength;
	// open file
	MakeFilenameUnique(fFile);
	FailOSErr(fFile->CreateDataFork());
	if (bhi.fDataLength)
		OpenFork(true);
	fCRC = kBinHexInitialCRC;
	fForkBytesLeft = fDataLen;
	fBinaryState = kInDataFork;
	return true;
}

Boolean PBinHexDecoder::ProcessForkData()
{
	if (fForkBytesLeft)
	{
		long numBytes = Min(fForkBytesLeft, fBytesInBinaryBuffer);
		WriteBytes(fBinaryBufferP, numBytes);
		CalcCRC(fBinaryBufferP, numBytes);
		EatenBinaryData(numBytes);
		fForkBytesLeft -= numBytes;
		return true;
	}
	if (fBytesInBinaryBuffer < 2)
		return false;
	CheckCRC();
	CloseFork();
	if (fBinaryState == kInDataFork)
	{
		if (fRsrcLen)
			OpenFork(false);
		fCRC = kBinHexInitialCRC;
		fBinaryState = kInRsrcForm;
		fForkBytesLeft = fRsrcLen;
		return true;
	}
	else
	{
		FInfo fndrInfo;
		FailOSErr(fFile->GetFinderInfo(fndrInfo));
		fndrInfo.fdFlags = fFinderFlags;
		FailOSErr(fFile->SetFinderInfo(fndrInfo));
		fBinaryState = kPastEnd;
		return true;
	}
}

void PBinHexDecoder::CalcCRC(const void *p, long noBytes)
{
	short crc = fCRC;
	const unsigned char *up = (const unsigned char *)p;
	while (noBytes)
	{
		short byte = *up++;
		noBytes--;
		for (short i = 8; i; i--)
		{
			Boolean hasHiBit = (crc & 0x8000) != 0; // crc >> 15
			crc <<= 1;
			crc |= byte >> 7;
			if (hasHiBit)
				crc ^= 0x1021;
			byte <<= 1;
			byte &= 0xff;
		}
	}
	fCRC = crc;
}

void PBinHexDecoder::CheckCRC()
{
	const short null_crc = 0;
	CalcCRC(&null_crc, 2);
	short file_crc = *(short*)fBinaryBufferP;
	EatenBinaryData(2);
	if (file_crc != fCRC)
	{
#if qDebug
		fprintf(stderr, "Bad CRC: calc = $%lx, file_crc = $%lx\n", long(fCRC)&0xFFFF, long(file_crc)&0xFFFF);
#endif
		FailOSErr(errBadBinHex);
	}
}
