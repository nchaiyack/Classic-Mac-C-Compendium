// Copyright © 1993-1995 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// ISO2022Conversion.cp

#include "ISO2022Conversion.h"

#include <ToolUtils.h>
#include <stdio.h>

#pragma segment MyArticle

/*
 * According to the ISO 2022, I think it is permitted
 * to use space character code when Japanese double code
 * (or other 96^n type multiple byte code) is indicated to G0.
 *
 * However, almost all Japanese terminal do not permit to
 * use it.
 */
#define		UGLYDUCK

/*
 * Many interfaces like nntp needs that a text ends with
 * newline code. If a internal text ends with a double byte char
 * and a newline, the result of this conversion will be
 * double byte char, newline and indication string.
 * This may not work with nntp. Define RESET_ON_NL to
 * add a roman indication string on the end of each line.
 */
#define	RESET_ON_NL

/*
 *
 * Code table sectioning
 *
 */
#define		R_		1		/* Roman */
#define		K_		2		/* Japanese Katakana */
#define		S_		4		/* Shift JIS --- Macintosh Japanese Code */
#define		Z_		8		/* Shift JIS 2nd byte */
#define		T_		0		/* Other should be unknown or through */

#ifdef		UGLYDUCK
#define		S_P		R_
#else		/* UGLYDUCK */
#define		S_P		T_
#endif		/* UGLYDUCK */

#ifdef	RESET_ON_NL
#define	_NL	R_
#else	/* RESET_ON_NL */
#define	_NL	T_
#endif	/* RESET_ON_NL */

static unsigned char secTable[256] = {
/*0*/	T_,		T_,		T_,		T_,		T_,		T_,		T_,		T_,
		T_,		T_,		_NL,	T_,		T_,		_NL,	T_,		T_,
/*1*/	T_,		T_,		T_,		T_,		T_,		T_,		T_,		T_,
		T_,		T_,		T_,		T_,		T_,		T_,		T_,		T_,
/*2*/	S_P,	R_,		R_,		R_,		R_,		R_,		R_,		R_,
		R_,		R_,		R_,		R_,		R_,		R_,		R_,		R_,
/*3*/	R_,		R_,		R_,		R_,		R_,		R_,		R_,		R_,
		R_,		R_,		R_,		R_,		R_,		R_,		R_,		R_,
/*4*/	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
		R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
/*5*/	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
		R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
/*6*/	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
		R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
/*7*/	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,
		R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	R_|Z_,	T_,
/*8*/	T_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
		S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
/*9*/	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
		S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
/*A*/	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
		K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
/*B*/	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
		K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
/*C*/	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
		K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
/*D*/	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
		K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,	K_|Z_,
/*E*/	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
		S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,	S_|Z_,
/*F*/	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,
		T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_|Z_,	T_,		T_,		T_,
};

#define		isTH(c)			(secTable[c] == T_)
#define		isRO(c)			(secTable[c] & R_)
#define		isKT(c)			(secTable[c] & K_)
#define		isSJ(c)			(secTable[c] & S_)
#define		isSJ2(c)		(secTable[c] & Z_)



#if 0
/* This array should be ordered as TCV_???? index above */
indicator indTable[] =
{
	{ 0x28, 0x42, kAscii0, 0, im_roman, ex_roman },		/* TCV_USRO */
	{ 0x28, 0x49, kAscii0, 0, im_kana,  ex_kana  },		/* TCV_JPKT */
	{ 0x28, 0x4a, kAscii0, 0, im_roman, ex_roman },		/* TCV_JPRO */
	{ 0x24, 0x40, kAscii0, 0, im_kanji, ex_kanji },		/* TCV_JPK1 */
	{ 0x24, 0x42, kAscii0, 0, im_kanji, ex_kanji }		/* TCV_JPK2 */
};
#endif

#undef Inherited
#define Inherited TObject

TTextCodeConverter::TTextCodeConverter()
{
	fImportFunc = &TTextCodeConverter::im_roman;
	fExportFunc = &TTextCodeConverter::ex_roman;
}

void TTextCodeConverter::ITextCodeConverter()
{
	Inherited::IObject();
}

void TTextCodeConverter::Free()
{
	Inherited::Free();
}

/* Which code set is now indicated by the network code. */
Boolean TTextCodeConverter::ImportSwitch()
{
	unsigned char ch1 = GetNextChar();
	unsigned char ch2;
	if (ch1 == 0x28)
	{
		ch2 = GetNextChar();
		switch (ch2)
		{
			case 0x00: return false; // end of text
			case 0x42: fImportFunc = &TTextCodeConverter::im_roman; break;
			case 0x49: fImportFunc = &TTextCodeConverter::im_kana;  break;
			case 0x4a: fImportFunc = &TTextCodeConverter::im_roman; break;
			default:
			 if (qDebug)
			 	fprintf(stderr, "Unknown encoding escape = esc,%ld,%ld, switching to roman\n", long(ch1), long(ch2));
			fImportFunc = &TTextCodeConverter::im_roman;
		}
	}
	else if (ch1 == 0x24)
	{
		ch2 = GetNextChar();
		switch (ch2)
		{
			case 0x00: return false; // end of text
			case 0x40: fImportFunc = &TTextCodeConverter::im_kanji;  break;
			case 0x42: fImportFunc = &TTextCodeConverter::im_kanji;  break;
			default:
			 if (qDebug)
			 	fprintf(stderr, "Unknown encoding escape = esc,%ld,%ld, switching to roman\n", long(ch1), long(ch2));
			fImportFunc = &TTextCodeConverter::im_roman;
		}
	}
	return true;
}

/* importing roman */
Boolean TTextCodeConverter::im_roman()
{
	while (true)
	{
		unsigned char ch = GetNextChar();
		if (!ch)
			return false;
		if (ch == chEscape)
			return true;
		OutputOneChar(ch);
	}
}

/* importing japanese katakana */
Boolean TTextCodeConverter::im_kana()
{
	while (true)
	{
		unsigned char ch = GetNextChar();
		if (!ch)
			return false;
		if (ch == chEscape)
			return true;
		if (isKT(ch))
			ch += 0x80;
		OutputOneChar(ch);
	}
}

/* importing japanese kanji */
Boolean TTextCodeConverter::im_kanji()
{
	unsigned char ch1 = GetNextChar();
	while (true)
	{
		if (!ch1)
			return false;
		if (ch1 == chEscape)
			return true;
		unsigned char ch2 = GetNextChar();
		if (isRO(ch1) && isRO(ch2)) 
		{
			ch1 |= 0x80;
			ch2 |= 0x80;
			if (ch1 & 0x01) 
			{
				ch1 = ch1 / 2 + (ch1 < 0xdf ? 0x31 : 0x71);
				ch2 = ch2 - (ch2 >= 0xe0 ? 0x60 : 0x61);
			}
			else
			{
				ch1 = ch1 / 2 + (ch1 < 0xdf ? 0x30 : 0x70);
				ch2 = ch2 - 2;
			}
			OutputTwoChars(ch1, ch2);
			ch1 = GetNextChar();
		} 
		else
		{
			OutputOneChar(ch1);
			ch1 = ch2;
		}
	}
}

void TTextCodeConverter::ExportSwitchToRoman()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x28, 0x4a);
	fExportFunc = &TTextCodeConverter::ex_roman;
}

void TTextCodeConverter::ExportSwitchToKatakana()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x28, 0x49);
	fExportFunc = &TTextCodeConverter::ex_kana;
}

void TTextCodeConverter::ExportSwitchToKanji()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x24, 0x42);		/* kanji should use the newer code */
	fExportFunc = &TTextCodeConverter::ex_kanji;
}

/* exporting roman */
Boolean TTextCodeConverter::ex_roman()
{
	unsigned char ch = fCurrentExportChar;
	while (true)
	{
		if (!ch)
			return false;
		if (isKT(ch))
		{
			ExportSwitchToKatakana();		/* katakana */
			return true;
		}
		if (isSJ(ch))
		{
			ExportSwitchToKanji();
			return true;
		}
		/* RO and UN need no conversion */
		OutputOneChar(ch);
		fCurrentExportChar = ch = GetNextChar();
	}
}

Boolean TTextCodeConverter::ex_kana()
{
	unsigned char ch = fCurrentExportChar;
	while (true)
	{
		if (!ch)
			return false;
		// disse isXXX kan laves meget mere effektivt ved at lave switch
		// direkte pŒ den v¾rdi der fŒs fra tabellen, og sŒ med mange case's
		if (isRO(ch))
		{
			ExportSwitchToRoman();		/* roman */
			return true;
		}
		if (isSJ(ch))
		{
			ExportSwitchToKanji();
			return true;
		}
		if (isKT(ch))
			ch -= 0x80;
		/* UN needs no conversion */
		OutputOneChar(ch);
		fCurrentExportChar = ch = GetNextChar();
	}
}

Boolean TTextCodeConverter::ex_kanji()
{
	unsigned char ch1 = fCurrentExportChar;
	while (true)
	{
		if (!ch1)
			return false;
		if (isRO(ch1))
		{
			ExportSwitchToRoman();		/* roman */
			return true;
		}
		if (isKT(ch1)) 
		{
			ExportSwitchToKatakana();
			return true;
		}
		unsigned char ch2 = GetNextChar();
		fCurrentExportChar = ch2;
		if (isSJ(ch1) && isSJ2(ch2)) 
		{
			if (ch2 >= 0x9f) 
			{
				ch1 = (ch1*2 - (ch1 >= 0xe0 ? 0xe0 : 0x60)) & 0x7f;
				ch2 = ch2 - 0x7e;
			}
			else
			{
				ch1 = (ch1*2 - (ch1 >= 0xe0 ? 0xe1 : 0x61)) & 0x7f;
				ch2 = (ch2 + (ch2 >= 0x7f ? 0x60 : 0x61)) & 0x7f;
			}
			OutputTwoChars(ch1, ch2);
			fCurrentExportChar = ch1 = GetNextChar();
		}
		else
		{
			OutputOneChar(ch1);
			ch1 = ch2; // ate one too much
		}
	}
}

void TTextCodeConverter::ConvertNet2Mac()
{
	fImportFunc = &TTextCodeConverter::im_roman;
	while ((this->*TTextCodeConverter::fImportFunc)())
		if (!ImportSwitch())
			return;
}

void TTextCodeConverter::ConvertMac2Net()
{
	fExportFunc = &TTextCodeConverter::ex_roman;
	fCurrentExportChar = GetNextChar();
	while ((this->*TTextCodeConverter::fExportFunc)())
		;
	if (fExportFunc != &TTextCodeConverter::ex_roman)
		ExportSwitchToRoman();
}

//==================================================================
#undef Inherited
#define Inherited TTextCodeConverter
DefineClass(TStringTextCodeConverter, TTextCodeConverter)

TStringTextCodeConverter::TStringTextCodeConverter()
	: TTextCodeConverter()
{
}

void TStringTextCodeConverter::IStringTextCodeConverter(const CStr255 &inputString)
{
	Inherited::IObject();
	fInputString = inputString;
}

void TStringTextCodeConverter::Free()
{
	Inherited::Free();
}
		
void TStringTextCodeConverter::ConvertNet2Mac()
{
	fOutputString = "";
	fInputIndex = 1;
	TTextCodeConverter::ConvertNet2Mac();
}

void TStringTextCodeConverter::ConvertMac2Net()
{	
	fOutputString = "";
	fInputIndex = 1;
	TTextCodeConverter::ConvertMac2Net();
}

unsigned char TStringTextCodeConverter::GetNextChar()
{
	if (fInputIndex <= fInputString.Length())
		return fInputString[fInputIndex++];
	else
		return 0;
}

void TStringTextCodeConverter::OutputOneChar(unsigned char ch)
{
	fOutputString += ch;
}

void TStringTextCodeConverter::OutputTwoChars(unsigned char ch1, unsigned char ch2)
{
	fOutputString += ch1;
	fOutputString += ch2;
}

#if qDebug
void DumpString(const CStr255 &s)
{
	for (short i = 1; i <= s.Length(); ++i)
	{
		if ((i % 16) == 1)
			fprintf(stderr, "%06lX:  ", long(i - 1));
		fprintf(stderr, " %02lX", long(s[i]));
		if ((i % 16) == 0)
			fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}
#endif

// giver mine ÇmacroerÈ mon Japanerne problemer??????

void Test4() 
{
#if qDebug
	CStr255 s;
	TStringTextCodeConverter cnv;
	cnv.IStringTextCodeConverter(s);
	fprintf(stderr, "net -> mac\n");
	GetIndString(s, 9999, 1);
	cnv.ConvertNet2Mac();
	DumpString(cnv.fOutputString);
	fprintf(stderr, "Mac -> net\n");
	GetIndString(s, 9999, 2);
	cnv.ConvertMac2Net();
	DumpString(cnv.fOutputString);
#endif
}
//==================================================================

#undef Inherited
#define Inherited TTextCodeConverter
DefineClass(THandleCodeConverter, TTextCodeConverter)

THandleCodeConverter::THandleCodeConverter()
	: TTextCodeConverter(), fOutput()
{
	fInputH = nil;
	fInputOffset = 0;
}

void THandleCodeConverter::IHandleCodeConverter(Handle inputH, long outputChunk)
{
	Inherited::IObject();
	FailInfo fi;
	Try(fi)
	{
		fOutput.IChunkyHandle(outputChunk);
		fInputH = inputH;
		fi.Success();
	} 
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void THandleCodeConverter::Free()
{
	fOutput.Free();
	Inherited::Free();
}

long THandleCodeConverter::GetOutputSize()
{
	return fOutput.GetSize();
}

Ptr THandleCodeConverter::GetOutputPtr()
{
	return fOutput.PtrAtOffset(0);
}

void THandleCodeConverter::ConvertNet2Mac()
{
	fOutput.DeleteAll();
	fInputOffset = 0;
	TTextCodeConverter::ConvertNet2Mac();
}

void THandleCodeConverter::ConvertMac2Net()
{	
	fOutput.DeleteAll();
	fInputOffset = 0;
	TTextCodeConverter::ConvertMac2Net();
}

unsigned char THandleCodeConverter::GetNextChar()
{
	return *(*fInputH + fInputOffset++);
}

void THandleCodeConverter::OutputOneChar(unsigned char ch)
{
	fOutput.AppendChar(ch);
}

void THandleCodeConverter::OutputTwoChars(unsigned char ch1, unsigned char ch2)
{
	fOutput.AppendChar(ch1);
	fOutput.AppendChar(ch2);
}
