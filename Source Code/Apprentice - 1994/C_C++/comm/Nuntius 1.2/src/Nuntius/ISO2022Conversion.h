// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// ISO2022Conversion.h

/*
 *
 * Japanese Text Code Converter
 * Between Network Code and Macintosh Native Code
 *
 * Copyright (c) 1993 by Shigeru Kanemoto
 *                   and HappySize Incorporated
 *
 *
 */

// Ported to C++ by Peter Speck, juni 1993


#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

#ifndef __UDYNDYNARRAY__
#include "UDynDynArray.h"
#endif

class PTextCodeConverter;

/* Code Conversion Direction */
typedef enum {
    tcvImport,		/* Import into Macintosh */
    tcvExport,		/* Export from Macintosh */
} TCVDirection;

typedef Boolean (PTextCodeConverter::*ImportFunc)();
typedef Boolean (PTextCodeConverter::*ExportFunc)();
// returns false when reaching end of text

class PTextCodeConverter : public PPtrObject
{
	public:
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		PTextCodeConverter();
		virtual ~PTextCodeConverter();
	protected:
		virtual unsigned char GetNextChar() = 0;
		// return Ascii0 if end of text
		virtual void OutputOneChar(unsigned char ch) = 0;
		virtual void OutputTwoChars(unsigned char ch1, unsigned char ch2) = 0;
	
	private:
		ImportFunc fImportFunc;
		ExportFunc fExportFunc;
		unsigned char fCurrentExportChar;
		
		Boolean im_roman();
		Boolean im_kana();
		Boolean im_kanji();
		Boolean ImportSwitch();
		Boolean ex_roman();
		Boolean ex_kana();
		Boolean ex_kanji();
		void ExportSwitchToRoman();
		void ExportSwitchToKatakana();
		void ExportSwitchToKanji();
};

class PStringTextCodeConverter : public PTextCodeConverter
{
	public:
		CStr255 fOutputString;
		
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		PStringTextCodeConverter(const CStr255 &inputString);
		virtual ~PStringTextCodeConverter();
	protected:
		virtual unsigned char GetNextChar();
		virtual void OutputOneChar(unsigned char ch);
		virtual void OutputTwoChars(unsigned char ch1, unsigned char ch2);

	private:
		const CStr255 &fInputString;
		short fInputIndex;
};

class PHandleCodeConverter : public PTextCodeConverter
{
	public:
		long GetOutputSize();
		Ptr GetOutputPtr();
		
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		 // inputH now owned, but must be zero terminated!!!
		PHandleCodeConverter();
		void IHandleCodeConverter(Handle inputH, long outputChunk);
		virtual ~PHandleCodeConverter();
	protected:
		virtual unsigned char GetNextChar();
		virtual void OutputOneChar(unsigned char ch);
		virtual void OutputTwoChars(unsigned char ch1, unsigned char ch2);

	private:
		CChunkyHandle fOutput;
		Handle fInputH;
		long fInputOffset;
};
