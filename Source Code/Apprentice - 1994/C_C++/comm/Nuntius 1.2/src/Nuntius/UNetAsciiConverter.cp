// Copyright � 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UNetAsciiConverter.cp

#include "UNetAsciiConverter.h"

#include <ToolUtils.h>

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
#define		_R		1		/* Roman */
#define		_K		2		/* Japanese Katakana */
#define		_S		4		/* Shift JIS --- Macintosh Japanese Code */
#define		_2		8		/* Shift JIS 2nd byte */
#define		_T		0		/* Other should be unknown or through */

#ifdef		UGLYDUCK
#define		_SP		_R
#else		/* UGLYDUCK */
#define		_SP		_T
#endif		/* UGLYDUCK */

#ifdef	RESET_ON_NL
#define	_NL	_R
#else	/* RESET_ON_NL */
#define	_NL	_T
#endif	/* RESET_ON_NL */

static unsigned char secTable[256] = {
/*0*/	_T,		_T,		_T,		_T,		_T,		_T,		_T,		_T,
		_T,		_T,		_NL,	_T,		_T,		_NL,	_T,		_T,
/*1*/	_T,		_T,		_T,		_T,		_T,		_T,		_T,		_T,
		_T,		_T,		_T,		_T,		_T,		_T,		_T,		_T,
/*2*/	_SP,	_R,		_R,		_R,		_R,		_R,		_R,		_R,
		_R,		_R,		_R,		_R,		_R,		_R,		_R,		_R,
/*3*/	_R,		_R,		_R,		_R,		_R,		_R,		_R,		_R,
		_R,		_R,		_R,		_R,		_R,		_R,		_R,		_R,
/*4*/	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
		_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
/*5*/	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
		_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
/*6*/	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
		_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
/*7*/	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,
		_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_R|_2,	_T,
/*8*/	_T|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
		_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
/*9*/	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
		_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
/*A*/	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
		_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
/*B*/	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
		_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
/*C*/	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
		_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
/*D*/	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
		_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,	_K|_2,
/*E*/	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
		_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,	_S|_2,
/*F*/	_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T|_2,
		_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T|_2,	_T,		_T,		_T,
};

#define		isTH(c)			(secTable[c] == _T)
#define		isRO(c)			(secTable[c] & _R)
#define		isKT(c)			(secTable[c] & _K)
#define		isSJ(c)			(secTable[c] & _S)
#define		isSJ2(c)		(secTable[c] & _2)



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

PTextCodeConverter::PTextCodeConverter()
	: PPtrObject()
{
	fImportFunc = &PTextCodeConverter::im_roman;
	fExportFunc = &PTextCodeConverter::ex_roman;
}

PTextCodeConverter::~PTextCodeConverter()
{
}

/* Which code set is now indicated by the network code. */
Boolean PTextCodeConverter::ImportSwitch()
{
	unsigned char ch1 = GetNextChar();
	unsigned char ch2;
	if (ch1 == 0x28)
	{
		ch2 = GetNextChar();
		switch (ch2)
		{
			case 0x00: return false; // end of text
			case 0x42: fImportFunc = &PTextCodeConverter::im_roman; break;
			case 0x49: fImportFunc = &PTextCodeConverter::im_kana;  break;
			case 0x4a: fImportFunc = &PTextCodeConverter::im_roman; break;
			default:
			 if (qDebug)
			 	fprintf(stderr, "Unknown encoding escape = esc,%ld,%ld, switching to roman\n", long(ch1), long(ch2));
			fImportFunc = &PTextCodeConverter::im_roman;
		}
	}
	else if (ch1 == 0x24)
	{
		ch2 = GetNextChar();
		switch (ch2)
		{
			case 0x00: return false; // end of text
			case 0x40: fImportFunc = &PTextCodeConverter::im_kanji;  break;
			case 0x42: fImportFunc = &PTextCodeConverter::im_kanji;  break;
			default:
			 if (qDebug)
			 	fprintf(stderr, "Unknown encoding escape = esc,%ld,%ld, switching to roman\n", long(ch1), long(ch2));
			fImportFunc = &PTextCodeConverter::im_roman;
		}
	}
	return true;
}

/* importing roman */
Boolean PTextCodeConverter::im_roman()
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
Boolean PTextCodeConverter::im_kana()
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
Boolean PTextCodeConverter::im_kanji()
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

void PTextCodeConverter::ExportSwitchToRoman()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x28, 0x4a);
	fExportFunc = &PTextCodeConverter::ex_roman;
}

void PTextCodeConverter::ExportSwitchToKatakana()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x28, 0x49);
	fExportFunc = &PTextCodeConverter::ex_kana;
}

void PTextCodeConverter::ExportSwitchToKanji()
{
	OutputOneChar(chEscape);
	OutputTwoChars(0x24, 0x42);		/* kanji should use the newer code */
	fExportFunc = &PTextCodeConverter::ex_kanji;
}

/* exporting roman */
Boolean PTextCodeConverter::ex_roman()
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

Boolean PTextCodeConverter::ex_kana()
{
	unsigned char ch = fCurrentExportChar;
	while (true)
	{
		if (!ch)
			return false;
		// disse isXXX kan laves meget mere effektivt ved at lave switch
		// direkte p� den v�rdi der f�s fra tabellen, og s� med mange case's
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

Boolean PTextCodeConverter::ex_kanji()
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

void PTextCodeConverter::ConvertNet2Mac()
{
	fImportFunc = &PTextCodeConverter::im_roman;
	while ((this->*PTextCodeConverter::fImportFunc)())
		if (!ImportSwitch())
			return;
}

void PTextCodeConverter::ConvertMac2Net()
{
	fExportFunc = &PTextCodeConverter::ex_roman;
	fCurrentExportChar = GetNextChar();
	while ((this->*PTextCodeConverter::fExportFunc)())
		;
	if (fExportFunc != &PTextCodeConverter::ex_roman)
		ExportSwitchToRoman();
}

//==================================================================
PStringTextCodeConverter::PStringTextCodeConverter(const CStr255 &inputString)
	: PTextCodeConverter(), fInputString(inputString)
{
}

PStringTextCodeConverter::~PStringTextCodeConverter()
{
}
		
void PStringTextCodeConverter::ConvertNet2Mac()
{
	fOutputString = "";
	fInputIndex = 1;
	PTextCodeConverter::ConvertNet2Mac();
}

void PStringTextCodeConverter::ConvertMac2Net()
{	
	fOutputString = "";
	fInputIndex = 1;
	PTextCodeConverter::ConvertMac2Net();
}

unsigned char PStringTextCodeConverter::GetNextChar()
{
	if (fInputIndex <= fInputString.Length())
		return fInputString[fInputIndex++];
	else
		return 0;
}

void PStringTextCodeConverter::OutputOneChar(unsigned char ch)
{
	fOutputString += ch;
}

void PStringTextCodeConverter::OutputTwoChars(unsigned char ch1, unsigned char ch2)
{
	fOutputString += ch1;
	fOutputString += ch2;
}

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

// giver mine �macroer� mon Japanerne problemer??????

void Test4() 
{
	CStr255 s;
	PStringTextCodeConverter cnv(s);
	fprintf(stderr, "net -> mac\n");
	GetIndString(s, 9999, 1);
	cnv.ConvertNet2Mac();
	DumpString(cnv.fOutputString);
	fprintf(stderr, "Mac -> net\n");
	GetIndString(s, 9999, 2);
	cnv.ConvertMac2Net();
	DumpString(cnv.fOutputString);
}
