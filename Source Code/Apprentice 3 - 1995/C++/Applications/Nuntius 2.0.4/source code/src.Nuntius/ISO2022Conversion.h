// Copyright © 1993-1995 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
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


#ifndef __UDYNDYNARRAY__
#include "UDynDynArray.h"
#endif

class TTextCodeConverter;

/* Code Conversion Direction */
typedef enum {
    tcvImport,		/* Import into Macintosh */
    tcvExport 		/* Export from Macintosh */
} TCVDirection;

typedef Boolean (TTextCodeConverter::*ImportFunc)();
typedef Boolean (TTextCodeConverter::*ExportFunc)();
// returns false when reaching end of text

class TTextCodeConverter : public TObject
{
	public:
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		TTextCodeConverter();
		void ITextCodeConverter();
		void Free();
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

class TStringTextCodeConverter : public TTextCodeConverter
{
	DeclareClass(TStringTextCodeConverter);
	public:
		CStr255 fOutputString;
		
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		TStringTextCodeConverter();
		void IStringTextCodeConverter(const CStr255 &inputString);
		void Free();
	protected:
		virtual unsigned char GetNextChar();
		virtual void OutputOneChar(unsigned char ch);
		virtual void OutputTwoChars(unsigned char ch1, unsigned char ch2);

	private:
		CStr255 fInputString;
		short fInputIndex;
};

class THandleCodeConverter : public TTextCodeConverter
{
	DeclareClass(THandleCodeConverter);
	public:
		long GetOutputSize();
		Ptr GetOutputPtr();
		
		virtual void ConvertNet2Mac();
		virtual void ConvertMac2Net();

		 // inputH now owned, but must be zero terminated!!!
		THandleCodeConverter();
		void IHandleCodeConverter(Handle inputH, long outputChunk);
		void Free();
	protected:
		virtual unsigned char GetNextChar();
		virtual void OutputOneChar(unsigned char ch);
		virtual void OutputTwoChars(unsigned char ch1, unsigned char ch2);

	private:
		CChunkyHandle fOutput;
		Handle fInputH;
		long fInputOffset;
};
