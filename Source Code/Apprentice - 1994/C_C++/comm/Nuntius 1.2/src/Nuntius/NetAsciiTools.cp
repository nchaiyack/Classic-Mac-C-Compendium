// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// NetAsciiTools.cp

#include "NetAsciiTools.h"
#include "Tools.h"
#include "UPrefsDatabase.h"

#include <ErrorGlobals.h>

#include <Resources.h>
#include <Errors.h>

#pragma segment MyTools
//--------------------------------------------------------------------------
CharsetTranslateTableType gUS2Finnish, gUS2swe, gUS2dk, gToLowerChar, gMac2NetAsctii;
CharsetTranslateTableType gNetAscii2Mac, gMac2NetAscii, gROT13Table;

void FillTable(CharsetTranslateTablePtr table, short first = 0, short last = 255, short offset = 0)
{
	for (short i = first; i <= last; i++)
		table[i] = i + offset;
}

void MakeTableExceptions(CharsetTranslateTablePtr table, const char *p)
{
	const unsigned char *up = (const unsigned char *)p;
	while (*up)
	{
		unsigned char uch = *up++;
		table[uch] = *up++;
	}
}

void TranslateCStr255(CStr255 &s, const CharsetTranslateTablePtr table)
{
	unsigned char *p = &s[1];
	for (short i = s.Length(); i; --i)
	{
		*p = table[*p];
		p++;
	}
}

void LoadTranslateTable(const CStr255 &rsrcName, CharsetTranslateTablePtr table1, CharsetTranslateTablePtr table2)
{
	Handle h = GetNamedResource('TCHR', rsrcName);
	if (!h && (ResError() == noErr || ResError() == resNotFound))
	{
		ParamText(rsrcName, gEmptyString, gEmptyString, gEmptyString);
		StdAlert(phMissingTranslateTable);
		return;
	}
	FailNILResource(h);
	BytesMove(*h, table1, 256);
	BytesMove(*h + 256, table2, 256);
}

void TranslateViaTable(const CharsetTranslateTablePtr table, char *textP, long len)
{
	unsigned char *p = (unsigned char*)textP;
	while (len > 0)
	{
		*p = table[*p];
		p++;
		len--;
	}
}
//--------------------------------------------------------------------------
void InitUNetAsciiTools()
{
	FillTable(gUS2dk);				MakeTableExceptions(gUS2dk, "[�\\�]�{�|�}�");
	FillTable(gUS2swe);				MakeTableExceptions(gUS2swe, "[�\\�]�{�|�}�");
	FillTable(gUS2Finnish);		MakeTableExceptions(gUS2Finnish, "[�\\�]�{�|�}�~�^�`�");
	FillTable(gToLowerChar);	FillTable(gToLowerChar, 'A', 'Z', 'a' - 'A');
	FillTable(gMac2NetAsctii);
	FillTable(gROT13Table);
	FillTable(gROT13Table, 'A', 'M',  13); FillTable(gROT13Table, 'a', 'm',  13);
	FillTable(gROT13Table, 'N', 'Z', -13); FillTable(gROT13Table, 'n', 'z', -13);
	for (short i = 128; i <= 255; i++)
		gMac2NetAsctii[i] = '!';
	CStr255 s;
	gPrefs->GetStringPrefs('Tran', s);
	LoadTranslateTable(s, gMac2NetAscii, gNetAscii2Mac);
}
//--------------------------------------------------------------------------

void StripSpaces(CStr255 &s)
{
	short i = s.Length();
	while (i && s[i] <= 32)
		--i;
	s.Length() = i;
	while (i && s[1] <= 32)
	{
		--i;
		s.Delete(1, 1);
	}
}

//--------------------------------------------------------------------------
void MakeLastCharCR(Handle h)
{
	HUnlock(h); // no restore!
	long size = GetHandleSize(h);
	char lastChar = *Ptr(*h + size - 1);
	if (lastChar == 13) 
		return;
	else if (lastChar == 10) {
		if (*Ptr(*h + size - 2) != 13)
			*Ptr(*h + size - 1) = 13; // replace LF by CR
		else {
			SetPermHandleSize(h, --size); // strip LF off
		}			
	}
	else { // Add cr
		SetPermHandleSize(h, ++size);
		*Ptr(*h + size - 1) = 13;
	}
}

void MakeLastCharNull(Handle h)
{
	HUnlock(h); // no restore!
	long size = GetHandleSize(h);
	char lastChar = *(*h + size - 1);
	if (lastChar == 0) 
		return;
	else
	{
		// Add null
		SetPermHandleSize(h, ++size);
		*(*h + size - 1) = 0;
	}
}


//--------------------------------------------------------------------------
void StoreAuthorText(CStr255 &s, CStr255 &realName, CStr255 &email, Boolean &isEmail)
{
	if (s.Length())
	{
		if (isEmail)
		{
			if (!email.Length())
				email = s;
		}
		else
		{
			if (!realName.Length())
				realName = s;
		}
		s = "";
	}
	isEmail = false;
}

void GetAuthorName(const CStr255 &text, CStr255 &realName, CStr255 &email)
{
	realName = "";
	email = "";
	CStr255 s("");
	short index = 1;
	Boolean isEmail = false;
	while (index <= text.Length())
	{
		switch (text[index])
		{
			case '"':
			case '>':
			case ')':
			case ',':
				StoreAuthorText(s, realName, email, isEmail);
				break;

			case '(':
				StoreAuthorText(s, realName, email, isEmail);
				break;
			
			case '<':
				StoreAuthorText(s, realName, email, isEmail);
				isEmail = true;
				s = "";
				break;
				
			case '@':
				isEmail = true;
				s += text[index];
				break;
				
			default:
				if (text[index] != 32 || s.Length() > 0)
					s += text[index];
		}
		++index;
	}
	StoreAuthorText(s, realName, email, isEmail);
	short i = realName.Length();
	while (i && realName[i] == 32)
		--i;
	realName.Length() = i;
	i = email.Length();
	while (i && email[i] == 32)
		--i;
	email.Length() = i;
}

void GetNationalAuthorNames(const CStr255 &text, CStr255 &realName, CStr255 &email)
{
	GetAuthorName(text, realName, email);
	if (email.Length() > 3)
	{		
		unsigned char *p = &email[email.Length() - 2];
		unsigned char uch, uch2;
		if (*p == '.')
		{
			++p;
			uch = gToLowerChar[*p];
			uch2 = gToLowerChar[*++p];
			if (     uch == 'd' && uch2 == 'k')
				TranslateCStr255(realName, gUS2dk);
			else if (uch == 's' && uch2 == 'e')
				TranslateCStr255(realName, gUS2swe);
			else if (uch == 'n' && uch2 == 'o')
				TranslateCStr255(realName, gUS2dk);
			else if (uch == 'f' && uch2 == 'i')
				TranslateCStr255(realName, gUS2Finnish);
		}
	}
}

Boolean GetPrintableAuthorName(const CStr255 &text, CStr255 &name, CStr255 &email)
{
	GetNationalAuthorNames(text, name, email);
	if (name.Length())
		return true;
	name = email;
	short atPos = name.Pos("@"); // use email instead, but remove the site name
	if (atPos > 1)
		name.Length() = atPos - 1;
	return false;
}
//--------------------------------------------------------------------------

