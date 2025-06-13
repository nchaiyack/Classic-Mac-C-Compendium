// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleView.cp

#if !qDebug

#include "UArticleView.h"
#include "UArticleCmds.h"
#include "UArticle.h"
#include "UArticleEntry.h"
#include "UArticleCache.h"
#include "UPrefsDatabase.h"
#include "UGroupDoc.h"
#include "NetAsciiTools.h"
#include "URowSelection.h"
#include "Tools.h"
#include "ViewTools.h"
#include "UThread.h"
#include "UAppleScriptCmds.h"

#include "RsrcGlobals.h"

#include <ToolUtils.h>
#include <Packages.h>
#include <stdio.h>

#pragma segment MyArticle


#define qDebugArticleView qDebug & 0
#define qDebugtabs qDebug & 0

const short kSubjectLine = 1;
const short kFromLine = 2;
const short kDateLine = 3;
const short kHeaderBodySeparatorLine = 4;
const short kNoShortHeaderLines = 4; // including header/body separator

#undef Inherited
#define Inherited TGridView
DefineClass(TArticleView, TGridView);
TArticleView::TArticleView()
{
	fArticle = nil;
	fArticleEntry = nil;
	fDoc = nil;
	fShowsAllHeaders = false;
	fUseROT13 = false;
	fUseJapaneseFont = false;
}

void TArticleView::IArticleView(TGroupDoc *doc, TView *superView, VPoint location,
											CArticleEntry *articleEntry)
{
	Inherited::IGridView(doc, superView, location, VPoint(640, 0), sizeVariable, sizeVariable,
		0, 1, 0, 640, kDontAdorn, kDontAdorn, 0, 0, false);
	FailInfo fi;
	Try(fi)
	{
		fCursorID = plusCursor;
		fDoc = doc;
		fArticleEntry = articleEntry;
		fArticle = gArticleCache->GetArticle(fDoc->GetGroupDotName(), articleEntry->GetID());

		fUseJapaneseFont = fArticle->ContainsJapaneseEncoding();
		if (fUseJapaneseFont)
			MAGetTextStyle(kJapaneseFontTextStyle, fGridViewTextStyle.fTextStyle);
		else
			gPrefs->GetTextStylePrefs('TSar', fGridViewTextStyle.fTextStyle);
		CalcStandardGridViewFont(fGridViewTextStyle);

		fShowsAllHeaders = gPrefs->GetBooleanPrefs('Head');
		long noLines, bodyStartLineNo;
		fArticle->GetLineInfo(noLines, bodyStartLineNo);
		fBodyStartLineNo = bodyStartLineNo;
		ArrayIndex noRows;
		if (fShowsAllHeaders)
			noRows = noLines;
		else
		{
			ArrayIndex noBodyLines = noLines - bodyStartLineNo + 1;
			noRows = kNoShortHeaderLines + noBodyLines;
		}
		InsRowLast(short(noRows), fGridViewTextStyle.fRowHeight);
#if qDebugArticleView
		fprintf(stderr, "TArticleView: inserted %ld rows of height %ld, article = %ld\n", noRows, long(fGridViewTextStyle.fRowHeight), articleID);
#endif
		
		fDoc->GetArticleStatus()->SetMinStatus(articleEntry->GetID(), kArticleRead);
		
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TArticleView::Free()
{
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	fDoc = nil; // not mine
	Inherited::Free();
}

void TArticleView::Close()
{
	Inherited::Close();
}

void TArticleView::SetNewFont(const TextStyle &textStyle)
{
	fGridViewTextStyle.fTextStyle = textStyle;
	CalcStandardGridViewFont(fGridViewTextStyle);
	if (fNumOfRows)
		SetRowHeight(1, fNumOfRows, fGridViewTextStyle.fRowHeight);
	Focus();
	ForceRedraw();
}

void TArticleView::SetUseROT13(Boolean useROT13)
{
	if (fUseROT13 == useROT13)
		return;
	fUseROT13 = useROT13;
	Focus();
	ForceRedraw();
}

void TArticleView::UpdateDisplay()
{
	Boolean doShowHeaders = gPrefs->GetBooleanPrefs('Head');
	if (fShowsAllHeaders != doShowHeaders)
	{
		ArrayIndex diff = fBodyStartLineNo - (kNoShortHeaderLines + 1);
		if (fShowsAllHeaders)
			diff = -diff;
		DirectDeltaRows(this, kNoShortHeaderLines + 1, short(diff), fGridViewTextStyle.fRowHeight);
		fShowsAllHeaders = doShowHeaders;
		VRect vr1, vr2;
		CellToVRect(GridCell(1, 1), vr1);
		CellToVRect(GridCell(1, kNoShortHeaderLines), vr2);
		Focus();
		InvalidateVRect(vr1 | vr2);
	}
}
//============================================== DRAWING ================================
void TArticleView::DrawRangeOfCells(GridCell startCell,
										GridCell stopCell,
										const VRect &aRect)
{
#if qDebug
	if (!fArticle)
		ProgramBreak("DrawRangeOfCells called when having no TArticle");
#endif
	SetPortTextStyle(fGridViewTextStyle.fTextStyle);
	Inherited::DrawRangeOfCells(startCell, stopCell, aRect);
}

void TArticleView::DrawCell(GridCell aCell, const VRect &aRect)
{
	CStr255 text;
	GetLineText(aCell.v, false, text);
	if (!text.Length())
		return;
	CStr255 s;
	MakeStringPrintable(text, s);
	CRect r;
	ViewToQDRect(aRect, r);
	MoveTo(4, r.top + fGridViewTextStyle.fVertOffset);
	DrawString(s);
}

void TArticleView::GetLineText(short lineNo, Boolean asQuote, CStr255 &text)
{
	if (DoGetLineText(lineNo, asQuote, text) == false)
		return;
	if (!text.Length())
		return;
	StripWhiteSpaceAtEnd(text);
	if (!fUseJapaneseFont)
	{
		TranslateCStr255(text, gNetAscii2Mac);
		if (fUseROT13)
			TranslateCStr255(text, gROT13Table);
	}
}

// DoGetLineText returns false if the text already have been char-translated.
Boolean TArticleView::DoGetLineText(short lineNo, Boolean asQuote, CStr255 &text)
{
	if (fShowsAllHeaders)
	{
		fArticle->GetLine(lineNo, text);
		if (asQuote)
		{
			CStr255 s;
			MyGetIndString(s, kQuoteLineTemplate);
			SubstituteStringItems(s, "ÇlineÈ", text);
			text = s;
		}
		return true;
	}
	if (lineNo > kNoShortHeaderLines)
	{
		fArticle->GetLine(lineNo - kNoShortHeaderLines - 1 + fBodyStartLineNo, text);
		if (asQuote)
			text.Insert(">", 1);
		return true;
	}
	CStr255 s, email;
	GridCell firstCell = FirstSelectedCell();
	GridCell lastCell  = LastSelectedCell();
	switch (lineNo)
	{
		case kSubjectLine:
			if (asQuote && firstCell == lastCell)
			{
				text = fArticleEntry->GetSubject();
				break;
			}
			MyGetIndString(text, kPreSubject);
			text += fArticleEntry->GetSubject();
			MyGetIndString(s, kPostSubject);
			text += s;
#if qDebug
			if (!asQuote)
			{
				text += ", ";
				NumToString(fArticleEntry->GetID(), s);
				text += s;
			}
#endif
			return false;

		case kFromLine:
			text = fArticleEntry->GetFrom();
			Boolean hadRealName = GetPrintableAuthorName(text, s, email);
			if (asQuote && firstCell == lastCell)
			{
				text = email;
				if (hadRealName)
					text += " (" + s + ")"; // RFC-822 style
				break;
			}
			MyGetIndString(text, kPreFrom);
			text += s;
			if (hadRealName)
			{
				MyGetIndString(s, kArticleReal2EmailSeparator);
				text += s;
				text += email;
			}
			MyGetIndString(s, kPostFrom);
			text += s;
			return false;

		case kDateLine:
			MyGetIndString(text, kPreDate);
			fArticle->GetHeader("Date", s);
			text += s;
			MyGetIndString(s, kPostDate);
			text += s;
			return false;
			
		case kHeaderBodySeparatorLine:
			if (asQuote)
				GetQuoteFromTextLine(text, kArticleQuoteLine);
			else
				text = "";
			return false;
	} // case
	return false;
}

void TArticleView::GetQuoteFromTextLine(CStr255 &text, short quoteTemplateID)
{
	MyGetIndString(text, quoteTemplateID);
	//
	CStr255 s, s2, s3;
	//
	fArticle->GetHeader("Message-id", s);
	SubstituteStringItems(text, "ÇmsgidÈ", s);
	//
	fArticle->GetHeader("From", s);
	GetAuthorName(s, s2, s3);
	SubstituteStringItems(text, "ÇrealnameÈ", s2);
	SubstituteStringItems(text, "ÇemailÈ", s3);
	//
	fArticle->GetHeader("Date", s);
	SubstituteStringItems(text, "ÇdateÈ", s);
	//
	char mailboxdate[60];
	DateTimeRec dtr;
	GetTime(&dtr);
	GetIndString(s, kEnglishShortMonthStrings, dtr.month);
	GetIndString(s2, kEnglishShortWeekdayStrings, dtr.dayOfWeek);
	sprintf(mailboxdate, "%3s %3s %2hd %02hd:%02hd:%02hd %hd",
		(char*)s2, (char*)s, dtr.day,
		dtr.hour, dtr.minute, dtr.second, dtr.year);
	SubstituteStringItems(text, "ÇmailboxdateÈ", mailboxdate);
}

void TArticleView::AppendSelectionAsTextForClipboard(Handle h, Boolean asQuote, 
															Boolean addHeader, short quoteTemplateID)
{
	if (!IsAnyCellSelected())
		return;
	FailInfo fi;
	Try(fi)
	{
		CStr255 text;
		Boolean gotHeader = !addHeader; 
		// if we don't want the header, make it look like we have already added it
		ArrayIndex sepLine = (fShowsAllHeaders ? fBodyStartLineNo - 1 : kNoShortHeaderLines);
		CSelectedCellIterator iter(this);
		for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
		{
			if (!gotHeader)
			{
				if (aCell.v >= sepLine)
				{
					GetQuoteFromTextLine(text, quoteTemplateID);
					if (!text.Length() || text[text.Length()] != 13)
						text += "\n";
					AppendStringToHandle(text, h);
					if (aCell.v == sepLine)
						continue;
				}
				gotHeader = true;
			}
			GetLineText(aCell.v, asQuote, text);
			text += "\n";
			AppendStringToHandle(text, h);
		}
		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}
}

CArticleEntry *TArticleView::GetArticleEntry()
{
	return fArticleEntry;
}

TArticle *TArticleView::GetArticle()
{
	return fArticle;
}

//======================== MANAGEMENT ==================================================
void TArticleView::DoOptionClick(VPoint &theMouse, GridCell clickCell)
{
#if !qDebug
	CStr255 url("");
	CStr255 s;
	Boolean gotCR;
	// Find first line of URL
	while (clickCell.v > 1)
	{
		if (DoGetLineText(clickCell.v - 1, false, s))
			TranslateCStr255(s, gNetAscii2Mac);
		gotCR = (s.Length() == 0 || s[s.Length()] == 13);
		if (gotCR)
			break;
		clickCell.v--;
	}
	// collect complete URL	
	while (clickCell.v <= fNumOfRows)
	{
		if (DoGetLineText(clickCell.v, false, s))
			TranslateCStr255(s, gNetAscii2Mac);
		gotCR = (s.Length() == 0 || s[s.Length()] == 13);
		url += s;
		if (gotCR)
			break;
		clickCell.v++;
	}
	StripWhiteSpaceAtEnd(url);
#if qDebug
	fprintf(stderr, "URL: '%s'\n", (char*)url);
#endif
	if (s.Length() == 0)
	{
		SysBeep(1);
		return;
	}
	TOpenURLCommand *cmd = new TOpenURLCommand();
	cmd->IOpenURLCommand(url);
	PostCommand(cmd);
#else
	Focus();
	SetPortTextStyle(fGridViewTextStyle.fTextStyle);
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	Try(fi)
	{
		short clickOffset = 0;
		CStr255 s, printable;
		// Find first line of URL
		Boolean gotCR;
		GridCell startCell = clickCell;
		while (startCell.v > 1)
		{
			if (DoGetLineText(startCell.v - 1, false, s))
				TranslateCStr255(s, gNetAscii2Mac);
			gotCR = (s.Length() == 0 || s[s.Length()] == 13);
			if (gotCR)
				break;
			startCell.v--;
		}
		h = NewPermHandle(1);
		**h = 0; // start with zero-terminate
		clickOffset++;
		// collect complete line
		GridCell aCell = startCell;
		while (aCell.v <= fNumOfRows)
		{
			if (DoGetLineText(aCell.v, false, s))
				TranslateCStr255(s, gNetAscii2Mac);
			gotCR = (s.Length() == 0 || s[s.Length()] == 13);
			if (gotCR)
				s.Length()--;
			MakeStringPrintable(s, printable);
			if (printable.Length())
				FailOSErr(PtrAndHand(&printable[1], h, printable.Length()));
			if (aCell.v < clickCell.v)
				clickOffset += printable.Length();
			else if (aCell.v == clickCell.v && printable.Length()) 
			{
				Boolean leadingEdge;
				Fixed widthRemaining;
				short offset = PixelToChar(Ptr(&printable[1]), printable.Length(), 0, 
					(theMouse.h - 4) << 16, &leadingEdge, &widthRemaining, 
									onlyStyleRun, CPoint(1, 1), CPoint(1, 1));
#if qDebug
				fprintf(stderr, "offset = %hd\n", offset);
#endif
				clickOffset += Min(offset, printable.Length() - 1);
			}
			if (gotCR)
				break;
			aCell.v++;
		}
		FailOSErr(PtrAndHand("\0", h, 1));

		HLock(h);
		fprintf(stderr, "URL-line = '%s'\n", *h + 1);
		unsigned char *p = (unsigned char *)*h + clickOffset;
		unsigned char ch;
		unsigned char sepChar = 0;
		while ((ch = gToLowerChar[*p]) != 0
		&& ch >= 'a' && ch <= 'z')
			p++;
		short numUrlTypes = MyCountIndStrings(kValidURLTypeStrings);
		Boolean hasPassedSpace = false;
		while (*p) 
		{
			if (*p == ' ')
				hasPassedSpace = true;
				 // ugly hack for non-escaped URLs with spaces in them
			if (*p-- != ':')
				continue;
			while ((ch = gToLowerChar[*p]) != 0
			&& ch >= 'a' && ch <= 'z')
				p--;
			sepChar = *p;
			unsigned char *p2 = p + 1;
			CStr255 urlType(gEmptyString);
			while (*p2 != ':')
				urlType += gToLowerChar[*p2++];
			for (short i = 1; i <= numUrlTypes; i++)
			{
				GetIndString(s, kValidURLTypeStrings, i);
				TranslateCStr255(s, gToLowerChar);
#if qDebug
				fprintf(stderr, "Compare '%s' to '%s'\n", (char*)urlType, (char*)s);
#endif
				if (urlType == s)
					goto foundURL;
			}
		}
		SysBeep(1);
		Failure(0, 0);
foundURL:
#if qDebug
		if (sepChar)
			fprintf(stderr, "SepChar = '%c'\n", sepChar);
		else
			fprintf(stderr, "SepChar is null\n");
#endif
		switch (sepChar) {
			case '\'': break;
			case '"': break;
			case '(': sepChar = ')'; break;
			case '{': sepChar = '}'; break;
			case '[': sepChar = ']'; break;
			case '<': sepChar = '>'; break;
			default: sepChar = hasPassedSpace ? 0 : ' '; break;
		}
		CStr255 url("");
		while (*++p && *p != sepChar)
			url += *p;
#if qDebug
		fprintf(stderr, "URL: '%s'\n", (char*)url);
#endif
		TOpenURLCommand *cmd = new TOpenURLCommand();
		cmd->IOpenURLCommand(url);
		PostCommand(cmd);
		
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
#endif
}

void TArticleView::DoMouseCommand(VPoint &theMouse,
														TToolboxEvent *event, CPoint /* hysteresis */ )
{
	GridCell aCell;
	if (IdentifyPoint(theMouse, aCell) == badChoice)
		return;
	if (event->fClickCount > 1 && IsAnyCellSelected())
	{
		// OpenSelection();
	}
	else // not double-click
	{
		if (IsAnyCellSelected() && FirstSelectedCell().v == 1 && LastSelectedCell().v == fNumOfRows && !event->IsShiftKeyPressed() && !event->IsCommandKeyPressed())
		{
			// against any HIG, but users want it
			SetEmptySelection(kHighlight);
			return;
		}
		if (event->IsOptionKeyPressed())
		{
			DoOptionClick(theMouse, aCell);
			return;
		}
		TStickySelectCommand *aCommand = new TStickySelectCommand();
		aCommand->IStickySelectCommand(this, theMouse, event->IsShiftKeyPressed(), event->IsCommandKeyPressed());
		PostCommand(aCommand);
	}
}

#endif


// Other not-so-old unused code
/*
Boolean CheckURL(Handle h, CStr255 &urlType) // h must be zero-terminated
{
	urlType = gEmptyString;
	HLock(h);
#if qDebugURL
	fprintf(stderr, "CheckURL of '%s'\n", *h);
#endif
	Ptr p = *h;
	while (*p && *p != ':')
		++p;
	if (!*p || p - *h > 100)
		return false;
	long typeLen = p - *h;
	urlType.Length() = typeLen;
	if (typeLen)	
		BytesMove(*h, &urlType[1], typeLen);
	short numUrlTypes = MyCountIndStrings(kValidURLTypeStrings);
	CStr255 s;
	for (short i = 1; i <= numUrlTypes; i++)
	{
		GetIndString(s, kValidURLTypeStrings, i);
		TranslateCStr255(s, gToLowerChar);
#if qDebug & 0
		fprintf(stderr, "Compare '%s' to '%s'\n", (char*)urlType, (char*)s);
#endif
		if (urlType == s)
			return true;
	}
#if qDebug
	fprintf(stderr, "URL type '%s' unknown\n", (char*)urlType);
#endif
	return false;
}

void TArticleView::CollectURLString(const CSelectionPoint &clickLoc, Handle h)
{
	SetPermHandleSize(h, 1);
	**h = 0; // start with zero term
	long clickOffset = 1;
	CStr255 s, printable;
	// Find first line of URL
	Boolean gotCR;
	long startLineNum = clickLoc.fLineNum;
	while (startLineNum > 1)
	{
		if (DoGetLineText(startLineNum - 1, s))
			TranslateCStr255(s, gNetAscii2Mac);
		gotCR = (s.Length() == 0 || s[s.Length()] == 13);
		if (gotCR)
			break;
		startLineNum--;
	}
	// collect complete line
	long lineNum = startLineNum;
	while (lineNum <= fNumLines)
	{
		if (DoGetLineText(lineNum, s))
			TranslateCStr255(s, gNetAscii2Mac);
		gotCR = (s.Length() == 0 || s[s.Length()] == 13);
		if (gotCR)
			s.Length()--;
		MakeStringPrintable(s, printable);
		if (printable.Length())
			FailOSErr(PtrAndHand(&printable[1], h, printable.Length()));
		if (lineNum < clickLoc.fLineNum)
			clickOffset += printable.Length();
		else if (lineNum == clickLoc.fLineNum) 
			clickOffset += Min(printable.Length(), clickLoc.fCharOffset);
		if (gotCR)
			break;
		lineNum++;
	}
	FailOSErr(PtrAndHand("\0", h, 1));

	HLock(h);
#if qDebugURL
	fprintf(stderr, "URL-line = '%s'\n", *h + 1);
#endif
	unsigned char *p = (unsigned char *)*h + clickOffset;
	unsigned char *p2;
	unsigned char ch;
	unsigned char sepChar = 0;
	while ((ch = gToLowerChar[*p]) != 0
	&& ch >= 'a' && ch <= 'z')
		p++;
	short numUrlTypes = MyCountIndStrings(kValidURLTypeStrings);
	Boolean hasPassedSpace = false;
	while (true) 
	{
		if (!*p)
		{
			SysBeep(1);
			Failure(0, 0);
		}
		if (*p == ' ')
			hasPassedSpace = true;
			 // ugly hack for non-escaped URLs with spaces in them
		if (*p-- != ':')
			continue;
		while ((ch = gToLowerChar[*p]) != 0
		&& ch >= 'a' && ch <= 'z')
			p--;
		sepChar = *p;
		p2 = ++p;
		CStr255 urlType(gEmptyString);
		while (*p2 != ':')
			urlType += gToLowerChar[*p2++];
		for (short i = 1; i <= numUrlTypes; i++)
		{
			GetIndString(s, kValidURLTypeStrings, i);
			TranslateCStr255(s, gToLowerChar);
#if qDebug & 0
			fprintf(stderr, "Compare '%s' to '%s'\n", (char*)urlType, (char*)s);
#endif
			if (urlType == s)
				goto foundURL;
		}
	}
foundURL:
#if qDebugURL
	if (sepChar)
		fprintf(stderr, "SepChar = '%c'\n", sepChar);
	else
		fprintf(stderr, "SepChar is null\n");
#endif
	switch (sepChar) {
		case '\'': break;
		case '"': break;
		case '(': sepChar = ')'; break;
		case '{': sepChar = '}'; break;
		case '[': sepChar = ']'; break;
		case '<': sepChar = '>'; break;
		default: sepChar = hasPassedSpace ? 0 : ' '; break;
	}
	p2 = p;
	while (*p && *p != sepChar)
		p++;
	if (Ptr(p) < *h + clickOffset)
	{
#if qDebug
		fprintf(stderr, "Aborts, didn't even get to clickLoc\n");
#endif
		SysBeep(1);
		Failure(0, 0);
	}
	if (p2 - p > GetHandleSize(h))
	{
#if qDebug
		ProgramBreak("p2 - p > GetHandleSize(h)!!!");
#endif
		Failure(minErr, 0);
	}
	if (Ptr(p2) > *h)
		BlockMoveData(p2, *h, p - p2);
	SetPermHandleSize(h, p - p2 + 1); // downsizing
	*(*h + (p - p2)) = 0;
#if qDebugURL
	fprintf(stderr, "Collected URL: '%s'\n", *h);
#endif
}		

void TArticleView::DoURLClick(VPoint &theMouse)
{
	Focus();
	SetPortTextStyle(fGridViewTextStyle.fTextStyle);
	CSelectionPoint clickLoc;
	ViewPtToSelectionPoint(theMouse, clickLoc);
	Handle h = nil;
	VOLATILE(h);
	CStr255 urlType(gEmptyString);
	FailInfo fi;
	Try(fi)
	{
		if (fSelection.Contains(clickLoc))
		{
#if qDebugURL
			fprintf(stderr, "Selection contains click, used as URL\n");
#endif
			h = NewPermHandle(0);
			AppendSelectionToHandle(h, gEmptyString);
			FailOSErr(PtrAndHand("\0", h, 1));
			if (!CheckURL(h, urlType))
			{
#if qDebug
				fprintf(stderr, "Selection is not an URL, uses parsing\n");
#endif
				h = DisposeIfHandle(h);
			}
		}
		if (h == nil)
		{
			h = NewPermHandle(0);
			CollectURLString(clickLoc, h);
			if (!CheckURL(h, urlType))
			{
				SysBeep(1);
				Failure(0, 0);
			}
		}
		FailOSErr(PtrAndHand("\0", h, 1));
#if qDebug
		HLock(h);
		fprintf(stderr, "TOpenURLCommand of '%s'\n", *h);
		HUnlock(h);
#endif
#if qDebugURL
		fprintf(stderr, "-----------------\n");
#else
		TOpenURLCommand *cmd = new TOpenURLCommand();
		cmd->IOpenURLCommand(h);
		PostCommand(cmd);
#endif
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}
*/
