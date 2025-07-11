// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleView.cp

#include "UArticleView.h"
#include "UArticleCmds.h"
#include "UArticle.h"
#include "UArticleCache.h"
#include "UPrefsDatabase.h"
#include "UGroupDoc.h"
#include "NetAsciiTools.h"
#include "URowSelection.h"
#include "Tools.h"
#include "ViewTools.h"
#include "UThread.h"

#include <RsrcGlobals.h>

#include <ToolUtils.h>
#include <Packages.h>

#pragma segment MyArticle

#define qDebugArticleView qDebug & 0
#define qDebugtabs qDebug & 0

const short kSubjectLine = 1;
const short kFromLine = 2;
const short kDateLine = 3;
const short kHeaderBodySeparatorLine = 4;
const short kNoShortHeaderLines = 4; // including header/body separator

TArticleView::TArticleView()
{
}

pascal void TArticleView::Initialize()
{
	inherited::Initialize();
	fArticle = nil;
	fDoc = nil;
	fArticleID = -1;
	fShowsAllHeaders = false;
	fUseROT13 = false;
	fUseJapaneseFont = false;
}

void TArticleView::IArticleView(TGroupDoc *doc, TView *superView, VPoint location,
										long articleID, TArticle *article)
{
	inherited::IGridView(doc, superView, location, VPoint(640, 0), sizeVariable, sizeVariable,
		0, 1, 0, 640, kDontAdorn, kDontAdorn, 0, 0, false);
	FailInfo fi;
	if (fi.Try())
	{
		fCursorID = 3; // plusCursor = 3
		fDoc = doc;
		fArticleID = articleID;
		fArticle = article;

		fUseJapaneseFont = fArticle->ContainsJapaneseEncoding();
		StandardGridViewTextStyle gvts;
		if (fUseJapaneseFont)
			MAGetTextStyle(kJapaneseFontTextStyle, gvts.fTextStyle);
		else
			gPrefs->GetTextStylePrefs('TSar', gvts.fTextStyle);
		CalcStandardGridViewFont(gvts);
		fGridViewTextStyle = gvts;

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
		
		fDoc->GetArticleStatus()->SetMinStatus(articleID, kArticleRead);
		
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TArticleView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TArticleView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fDoc = (TGroupDoc*) itsDocument;

	StandardGridViewTextStyle gvts;
	gPrefs->GetTextStylePrefs('TSar', gvts.fTextStyle);
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
}

pascal void TArticleView::Free()
{
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	fDoc = nil; // not mine
	inherited::Free();
}

pascal void TArticleView::Close()
{
	inherited::Close();
}

void TArticleView::SetNewFont(const TextStyle &textStyle)
{
	StandardGridViewTextStyle gvts;
	gvts.fTextStyle = textStyle;
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
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
pascal void TArticleView::DrawRangeOfCells(GridCell startCell,
										GridCell stopCell,
										const VRect &aRect)
{
#if qDebug
	if (!fArticle)
		ProgramBreak("DrawRangeOfCells called when having no TArticle");
#endif
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	inherited::DrawRangeOfCells(startCell, stopCell, aRect);
}

pascal void TArticleView::DrawCell(GridCell aCell, const VRect &aRect)
{
	CStr255 text;
	GetLineText(aCell.v, false, text);
	if (!text.Length())
		return;
	CRect r;
	ViewToQDRect(aRect, r);
	MoveTo(r.left + 2, r.top + fGridViewTextStyle.fVertOffset);
	// expand tabs to spaces
	text += char(0);
	unsigned char *p = &text[1];
	CStr255 s;
	s.Length() = 0;
	unsigned char ch = 0;
	while ((ch = *p++) != 0)
	{
		if (ch == chTab)
		{
			do
			{
#if qDebugtabs
				s += '�'; // small dot in Courier
#else
				s += char(chSpace);
#endif
			} while ((s.Length() & 7) != 0);
		}
		else if (ch < 32)
		{
			s += '^';
			s += char(ch + 'A');
		}
		else
			s += ch;
	}
	DrawString(s);
}

void TArticleView::GetLineText(short lineNo, Boolean asQuote, CStr255 &text)
{
	if (DoGetLineText(lineNo, asQuote, text) == false)
		return;
	if (!text.Length())
		return;
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
			SubstituteStringItems(s, "�line�", text);
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
	HandleOffsetLength hol;
	GridCell firstCell = FirstSelectedCell();
	GridCell lastCell  = LastSelectedCell();
	switch (lineNo)
	{
		case kSubjectLine:
			fDoc->GetSubject(fArticleID, hol);
			CopyHolToCStr255(hol, text);
			if (asQuote && firstCell == lastCell)
				break;
			MyGetIndString(s, kPreSubject);
			text.Insert(s, 1);
			MyGetIndString(s, kPostSubject);
			text += s;
#if qDebug
			if (!asQuote)
			{
				text += ", ";
				NumToString(fArticleID, s);
				text += s;
			}
#endif
			return false;

		case kFromLine:
			fDoc->GetFrom(fArticleID, hol);
			CopyHolToCStr255(hol, text);
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
}

void TArticleView::GetQuoteFromTextLine(CStr255 &text, short quoteTemplateID)
{
	MyGetIndString(text, quoteTemplateID);
	//
	CStr255 s, s2, s3;
	//
	fArticle->GetHeader("Message-id", s);
	SubstituteStringItems(text, "�msgid�", s);
	//
	fArticle->GetHeader("From", s);
	GetAuthorName(s, s2, s3);
	SubstituteStringItems(text, "�realname�", s2);
	SubstituteStringItems(text, "�email�", s3);
	//
	fArticle->GetHeader("Date", s);
	SubstituteStringItems(text, "�date�", s);
	//
	char mailboxdate[60];
	DateTimeRec dtr;
	GetTime(dtr);
	GetIndString(s, kEnglishShortMonthStrings, dtr.month);
	GetIndString(s2, kEnglishShortWeekdayStrings, dtr.dayOfWeek);
	sprintf(mailboxdate, "%3s %3s %2hd %02hd:%02hd:%02hd %hd",
		(char*)s2, (char*)s, dtr.day,
		dtr.hour, dtr.minute, dtr.second, dtr.year);
	SubstituteStringItems(text, "�mailboxdate�", mailboxdate);
}

void TArticleView::AppendSelectionAsTextForClipboard(Handle h, Boolean asQuote, 
															Boolean addHeader, short quoteTemplateID)
{
	if (!IsAnyCellSelected())
		return;
	FailInfo fi;
	if (fi.Try())
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

long TArticleView::GetArticleID()
{
	return fArticleID;
}

TArticle *TArticleView::GetArticle()
{
	return fArticle;
}

//======================== MANAGEMENT ==================================================
pascal void TArticleView::DoMouseCommand(VPoint &theMouse,
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
		TStickySelectCommand *aCommand = new TStickySelectCommand();
		aCommand->IStickySelectCommand(this, theMouse, event->IsShiftKeyPressed(), event->IsCommandKeyPressed());
		PostCommand(aCommand);
	}
}


pascal void TArticleView::DoKeyEvent(TToolboxEvent *event)// override 
{
	if (!IsEnabled())
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chBackspace:
			SetEmptySelection(kRedraw);
			break;

		default:
			inherited::DoKeyEvent(event);
			break;
	}
}

pascal void TArticleView::DoMenuCommand(CommandNumber aCommandNumber)
{
	if (!IsEnabled())
	{
		inherited::DoMenuCommand(aCommandNumber);
		return;
	}
	switch (aCommandNumber)
	{
		default:
			inherited::DoMenuCommand(aCommandNumber);
	}
}

pascal void TArticleView::DoSetupMenus()
{
	inherited::DoSetupMenus();
}
