/* BigText - Supports large line-wrapped text buffers                         */

#include "BigText.h"

		// � Implementation use
#ifndef __LOGVIEW__
#include "LogView.h"
#endif

#ifndef __NOTRACE__
#include "NoTrace.h"
#endif

//------------------------------------------------------------------------------

short pCharLocs [kMaxChunk + 1];
RgnHandle BTScroll_clip, BTScroll_scrl, Draw_clip, Draw_tmp;
Boolean pWChars [256] = {
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,	/* $00 - $0F */
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,	/* $10 - $1F */
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,	/* $20 - $2F */
	FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,  FALSE, FALSE,
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,	/* $30 - $3F */
	TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	FALSE, TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,	/* $40 - $4F */
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,	/* $50 - $5F */
	TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE, TRUE,
	FALSE, TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,	/* $60 - $6F */
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,	/* $70 - $7F */
	TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE
	};

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTActivate (Boolean activate, Boolean redraw)
{
	if (fActive != activate) {
		fActive = activate;
		if (redraw) Highlight (fSelStart, fSelEnd);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTAppend (unsigned char *buf, long count,
		Boolean redraw)
{
	VRect area;
	Rect r;
	long drop, oldlen, act;
	
	do {
		if (gMemIsLow)
			drop = count - fMaxLowSize + fLength +
				((fBounds.bottom - fBounds.top) * sizeof (short)) / fLineHeight;
		else if (fMaxLength > 0)
			drop = count - fMaxLength + fLength;
		else
			drop = 0;
		if (drop > 0) {
			oldlen = fLength;
			BTDrop (drop, redraw);
			drop -= oldlen - fLength;
			if (drop > 0) {
				buf += drop;
				count -= drop;
			}
		}
		BuildReserve (FALSE);
		area = fBounds;
		area.top = area.bottom - fLineHeight;
		act = AppendChars (buf, count);
		if (redraw) {
			area.bottom = fBounds.bottom;
			VRectToRect (&area, &r);
			Draw (&r);
		}
		BuildReserve (TRUE);
		if (!gMemIsLow) {
			fMaxLowSize = fLength + ((fBounds.bottom - fBounds.top) *
				sizeof (short)) / fLineHeight;
			if (fMemReserve) fMaxLowSize += GetHandleSize (fMemReserve);
		}
		buf += act;
		count -= act;
	} while (count > 0);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTClick (Point *pt, short clicks, Boolean extend)
{
	long anchorstart, anchorend, cloc, runstart, runend;
	Point loc;
	Boolean down;
	
	if (extend) {
		FindRun (clicks, pt->h, pt->v, &runstart, &runend, &cloc);
		if (cloc > (fSelStart + fSelEnd) / 2)
			anchorstart = fSelStart;
		else
			anchorstart = fSelEnd;
		anchorend = anchorstart;
	} else {
		FindRun (clicks, pt->h, pt->v, &anchorstart, &anchorend, &cloc);
		if ((fSelStart != anchorstart) || (fSelEnd != anchorend)) {
			Highlight (fSelStart, fSelEnd);
			Highlight (anchorstart, anchorend);
			fSelStart = anchorstart;
			fSelEnd = anchorend;
		}
	}
	do {
		down = StillDown ();
		GetMouse (&loc);
		((TLogView *) fSuperView)->ClikLoop (loc);
		FindRun (clicks, loc.h, loc.v, &runstart, &runend, &cloc);
		if (cloc > (anchorstart + anchorend) / 2) {
			Highlight (fSelStart, anchorstart);
			fSelStart = anchorstart;
			Highlight (fSelEnd, runend);
			fSelEnd = runend;
		} else {
			Highlight (fSelEnd, anchorend);
			fSelEnd = anchorend;
			Highlight (fSelStart, runstart);
			fSelStart = runstart;
		}
	} while (down);
	if (fSelStart > fSelEnd) {
		cloc = fSelStart;
		fSelStart = fSelEnd;
		fSelEnd = cloc;
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTDrop (long size, Boolean redraw)
{
	long len;
	LSHandle lh;
	
	while (size > 0) {
		len = (**fText).bLength;
		size -= (**fText).bLength;
		if ((**fText).bNext) {
			DropBuf ();
			fSelStart = (fSelStart > len) ? fSelStart - len : 0;
			fSelEnd = (fSelEnd > len) ? fSelEnd - len : 0;
		} else {
			(**fText).bLength = 0;
			(**fText).bLines = 1;
			SetPermHandleSize ((Handle) fText, sizeof (TextBuffer) - kTBSize);
			lh = (**fText).bStarts;
			SetPermHandleSize ((Handle) lh, sizeof (short) * 2);
			(**lh) [1] = 0;
			fLength = 0;
			fBounds.top = fDisplay.bottom;
			fBounds.bottom = fDisplay.bottom + fLineHeight;
			fSelStart = 0;
			fSelEnd = 0;
			size = 0;
		}
	}
	if (fBounds.top > fDisplay.top)
		BTScroll (0, fDisplay.top - fBounds.top, redraw);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Handle TBigText::BTGetText (void)
{
	long size, pos, epos;
	Handle h;
	unsigned char *cp;
	TBHandle th, eth;
	
	size = fSelEnd - fSelStart;
	h = NewPermHandle (size);
	if (h) {
		Char2Buf (fSelStart, &th, &pos);
		Char2Buf (fSelEnd, &eth, &epos);
		cp = (unsigned char *) *h;
		while (th && (th != eth)) {
			size = (**th).bLength - pos;
			BlockMove ((**th).bBuf + pos, cp, size);
			cp += size;
			th = (**th).bNext;
			pos = 0;
		}
		if (th) BlockMove ((**th).bBuf + pos, cp, epos - pos);
	}
	return h;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTResize (Rect *display, long bleft, long bright,
		Boolean redraw)
{
	long delta, pos, oldv;
	TBHandle th;
	
	if (fAutoWrap && (fBounds.right - fBounds.left != bright - bleft)) {
		fCurVers += 1;
		if (fCurVers == 0) fCurVers = 1;
	}
	fBounds.left = bleft;
	fBounds.right = bright;
	if ((fDisplay.top > fBounds.top) && (fDisplay.bottom >= fBounds.bottom)) {
		Recalc (fBounds.bottom + display->top - display->bottom,
			fBounds.bottom, FALSE);
		delta = display->bottom - fBounds.bottom;
	} else {
		Vert2Buf (fDisplay.top, &th, &pos);
		oldv = Buf2Vert (th, pos);
		delta = display->top - fDisplay.top;
		Recalc (fDisplay.top, display->bottom - delta, FALSE);
		delta += oldv - Buf2Vert (th, pos);
		if (display->bottom > fBounds.bottom + delta)
			delta = display->bottom - fBounds.bottom;
	}
	if (fBounds.top + delta > display->top)
		delta = display->top - fBounds.top;
	fBounds.top += delta;
	fBounds.bottom += delta;
	fDisplay = *display;
	if (redraw) {
		PenNormal ();
		EraseRect (display);
		Draw (display);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTScroll (long h, long v, Boolean redraw)
{
	Rect area;
	long ah, av;
	Boolean toend;
	
	if ((h != 0) || (v != 0)) {
		toend = fBounds.bottom + v <= fDisplay.bottom;
		Recalc (fDisplay.top - v, fDisplay.bottom - v, (v > 0) &&
			(v <= fDisplay.bottom - fDisplay.top));
		if (toend || (fBounds.bottom + v < fDisplay.bottom))
			v = fDisplay.bottom - fBounds.bottom;
		if (fBounds.top + v > fDisplay.top) v = fDisplay.top - fBounds.top;
		fBounds.top += v;
		fBounds.left += h;
		fBounds.bottom += v;
		fBounds.right += h;
		if (redraw) {
			area = fDisplay;
			ah = (h > 0) ? h : -h;
			av = (v > 0) ? v : -v;
			if ((ah < area.right - area.left) &&
					(av < area.bottom - area.top)) {
				ScrollRect (&area, (short) h, (short) v, BTScroll_scrl);
				GetClip (BTScroll_clip);
				SectRgn (qd.thePort->clipRgn, BTScroll_scrl, BTScroll_scrl);
				SetClip (BTScroll_scrl);
				EraseRgn (BTScroll_scrl);
				area = (**BTScroll_scrl).rgnBBox;
				Draw (&area);
				SetClip (BTScroll_clip);
			} else {
				EraseRect (&area);
				Draw (&area);
			}
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTSetSelect (long selStart, long selEnd,
		Boolean redraw)
{
	if (redraw && fActive) Highlight (fSelStart, fSelEnd);
	fSelStart = selStart;
	fSelEnd = selEnd;
	if (redraw && fActive) Highlight (fSelStart, fSelEnd);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BTSetStyle (TextStyle *theStyle, long tabChars,
		Boolean redraw)
{
	GrafPtr savedPort;
	FontInfo info;
	long delta, newheight, newbottom, oldv, pos;
	TBHandle th;
	Rect disp;
	
	GetPort (&savedPort);
	SetPort (gWorkPort);
	SetPortTextStyle (theStyle);
	if ((theStyle->tsFont != fStyle.tsFont) ||
			(theStyle->tsSize != fStyle.tsSize) ||
			(CharWidth (' ') * tabChars != fTabWidth)) {
		disp = fDisplay;
		fStyle = *theStyle;
		GetFontInfo (&info);
		newheight = info.ascent + info.descent + info.leading;
		newbottom = fBounds.top + ((fBounds.bottom - fBounds.top) /
			fLineHeight) * newheight;
		fTabWidth = CharWidth (' ') * tabChars;
		fCurVers += 1;
		if (fCurVers == 0) fCurVers = 1;
		if ((disp.top > fBounds.top) && (disp.bottom >= fBounds.bottom)) {
			fLineHeight = newheight;
			fBounds.bottom = newbottom;
			Recalc (newbottom + disp.top - disp.bottom, newbottom, FALSE);
			delta = disp.bottom - fBounds.bottom;
		} else {
			Vert2Buf (disp.top, &th, &pos);
			oldv = Buf2Vert (th, pos);
			delta = oldv - ((oldv - fBounds.top) / fLineHeight) * newheight -
				fBounds.top;
			fLineHeight = newheight;
			fBounds.top += delta;
			fBounds.bottom = newbottom + delta;
			Recalc (oldv, disp.bottom, FALSE);
			delta = oldv - Buf2Vert (th, pos);
			if (disp.bottom > fBounds.bottom + delta)
				delta = disp.bottom - fBounds.bottom;
		}
		if (fBounds.top + delta > disp.top) delta = disp.top - fBounds.top;
		fBounds.top += delta;
		fBounds.bottom += delta;
		SetPort (savedPort);
		if (redraw) {
			PenNormal ();
			EraseRect (&disp);
			Draw (&disp);
		}
	} else
		SetPort (savedPort);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal TBHandle TBigText::AppendBuf (TBHandle th, long size)
{
	TBHandle h;
	LSHandle lh;
	
	h = (TBHandle) NewPermHandle (sizeof (TextBuffer) - kTBSize + size);
	lh = (LSHandle) NewPermHandle (sizeof (short) * 2);
	if (h && lh) {
		(**h).bNext = NULL;
		(**h).bLength = 0;
		(**h).bLines = 1;
		(**h).bVersion = 0;
		(**h).bStarts = lh;
		(**lh) [0] = 0;
		(**lh) [1] = 0;
		if (th)
			(**th).bNext = h;
		else
			fText = h;
		fBounds.bottom += fLineHeight;
	} else {
		if (h) DisposeIfHandle (h);
		if (lh) DisposeIfHandle (lh);
	}
	return h;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::AppendChars (unsigned char *buf, long count)
{
	long pos, len, lsize, act;
	Boolean newBuf, oldMemLow;
	unsigned char *b, ch;
	TBHandle th, oth;
	
	oldMemLow = gMemIsLow;
	th = fText;
	while ((**th).bNext) th = (**th).bNext;
	pos = (**th).bLength;
	oth = th;
	ch = (pos > 0) ? (**th).bBuf [pos - 1] : 0;
	newBuf = (pos >= kTBSize) && (ch == chReturn);
	if (newBuf) pos = 0;
	act = 0;
	while (act < count) {
		if (!newBuf) ClrLines (th);
		len = 0;
		b = buf;
		while ((len < count - act) && ((ch != chReturn) ||
				(pos + len < kTBSize)) && (pos + len < kMaxTBSize)) {
			ch = *b++;
			len += 1;
		}
		if (newBuf) {
			th = AppendBuf (th, len);
			if (!th) Debugger ();
		} else {
			lsize = sizeof (TextBuffer) - kTBSize + pos + len;
			SetPermHandleSize ((Handle) th, lsize);
			if (GetHandleSize ((Handle) th) != lsize) Debugger ();
		}
		BlockMove (buf, (**th).bBuf + pos, len);
		buf += len;
		(**th).bLength = (short) (pos + len);
		(**(**th).bStarts) [(**th).bLines] = (short) (pos + len);
		act += len;
		fLength += len;
		newBuf = TRUE;
		pos = 0;
		if ((gMemIsLow && !oldMemLow) || (!gMemReserve && !gMemReserve2)) break;
	}
	while (oth) {
		RecalcBuf (oth);
		oth = (**oth).bNext;
	}
	return act;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::Buf2Char (TBHandle th, long pos)
{
	long p;
	TBHandle h;
	
	h = fText;
	p = 0;
	while (h && (h != th)) {
		p += (**h).bLength;
		h = (**h).bNext;
	}
	if (h) p += pos;
	return p;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::Buf2Hor (TBHandle th, long pos)
{
	TextStyle theStyle;
	long len, p, hor, width;
	SignedByte oldState;
	
	theStyle = fStyle;
	SetPortTextStyle (&theStyle);
	p = (**(**th).bStarts) [FindLine (th, pos)];
	hor = 0;
	while (p < pos) {
		len = ChunkSize (th, p, pos);
		if (len > 0) {
			oldState = HGetState ((Handle) th);
			HLock ((Handle) th);
			hor += TextWidth ((**th).bBuf, (short) p, (short) (len));
			HSetState ((Handle) th, oldState);
			p += len;
		} else {
			if ((**th).bBuf [p] == chTab)
				hor += fTabWidth - (hor % fTabWidth);
			else
				hor = fBounds.right;
			p += 1;
		}
	}
	if (fAutoWrap) {
		width = fBounds.right - fBounds.left;
		if (width < 0) width = 0;
		if (hor > width) hor = width;
	}
	return fBounds.left + hor;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::Buf2Vert (TBHandle th, long pos)
{
	TBHandle h;
	long lines;
	
	h = fText;
	lines = 0;
	while (h != th) {
		lines += (**h).bLines;
		h = (**h).bNext;
	}
	return fBounds.top + ((lines + FindLine (th, pos)) * fLineHeight);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::BuildReserve (Boolean build)
{
	long size;
	
	if (fMemReserve) {
		DisposIfHandle (fMemReserve);
		fMemReserve = NULL;
	}
	if (build) {
		size = fLength + (sizeof (short) * (fBounds.bottom - fBounds.top)) /
			fLineHeight;
		if (size < kBTRsrv) fMemReserve = NewPermHandle (kBTRsrv - size);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::Char2Buf (long charPos, TBHandle *th, long *pos)
{
	TBHandle h;
	
	h = fText;
	while (h && (**h).bNext && (charPos >= (**h).bLength)) {
		charPos -= (**h).bLength;
		h = (**h).bNext;
	}
	*th = h;
	*pos = (charPos < (**h).bLength) ? charPos : (**h).bLength;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::ChunkSize (TBHandle th, long pos, long epos)
{
	long l, max;
	unsigned char *cp, ch;
	
	cp = (**th).bBuf + pos;
	max = epos - pos;
	for (l = 0; l < max; l++) {
		ch = *cp++;
		if ((ch == chTab) || (ch == chReturn)) break;
	}
	return l;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::ClrLines (TBHandle th)
{
	LSHandle lh;

	fBounds.bottom -= ((**th).bLines - 1) * fLineHeight;
	(**th).bLines = 1;
	lh = (**th).bStarts;
	SetPermHandleSize ((Handle) lh, sizeof (short) * 2);
	(**lh) [1] = (**th).bLength;
	(**th).bVersion = 0;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::Draw (Rect *area)
{
	TextStyle theStyle;
	FontInfo info;
	TBHandle th;
	long pos, base, len, hor, epos, height, top;
	VRect vr;
	Rect dr;
	SignedByte oldState;
	Point pt;

	dr = fDisplay;
	if (SectRect (&dr, area, &dr)) {
		PenNormal ();
		theStyle = fStyle;
		SetPortTextStyle (&theStyle);
		RectToVRect (area, &vr);
		Vert2Buf (vr.top, &th, &pos);
		top = Buf2Vert (th, pos);
		height = vr.bottom - top;
		GetFontInfo (&info);
		base = area->top - vr.top + top + info.ascent;
		GetClip (Draw_clip);
		RectRgn (Draw_tmp, &dr);
		SectRgn (qd.thePort->clipRgn, Draw_tmp, Draw_tmp);
		SetClip (Draw_tmp);
		EraseRect (area);
		while (th && (height > 0)) {
			epos = (**(**th).bStarts) [FindLine (th, pos) + 1];
			hor = 0;
			while (pos < epos) {
				len = ChunkSize (th, pos, epos);
				if (len > 0) {
					MoveTo ((short) (fBounds.left + hor), (short) base);
					oldState = HGetState ((Handle) th);
					HLock ((Handle) th);
					DrawText ((**th).bBuf, (short) pos, (short) len);
					HSetState ((Handle) th, oldState);
					GetPen (&pt);
					hor = pt.h - fBounds.left;
					pos += len;
				} else {
					if ((**th).bBuf [pos] == chTab)
						hor += fTabWidth - (hor % fTabWidth);
					pos += 1;
				}
			}
			if (epos == (**th).bLength) {
				th = (**th).bNext;
				pos = 0;
			}
			base += fLineHeight;
			height -= fLineHeight;
		}
		if (fActive) Highlight (fSelStart, fSelEnd);
		SetClip (Draw_clip);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::DropBuf (void)
{
	TBHandle th;
	
	th = fText;
	fBounds.top += (**th).bLines * fLineHeight;
	fLength -= (**th).bLength;
	fText = (**th).bNext;
	DisposeIfHandle ((**th).bStarts);
	DisposeIfHandle (th);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TBigText::DropSome (void)
{
	long actsize;
	
	fMaxLowSize = kBTRsrv + ((fMaxLowSize - kBTRsrv) * 9) / 10;
	actsize = fLength + (sizeof (short) * (fBounds.bottom - fBounds.top)) /
		fLineHeight;
	if (actsize > fMaxLowSize) {
		fSuperView->Focus ();
		BTDrop (actsize - fMaxLowSize, kRedraw);
		((TLogView *) fSuperView)->SynchScrollBar (kRedraw);
		return TRUE;
	} else
		return FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TBigText::FindLine (TBHandle th, long pos)
{
	long l, m, r;
	LSHandle lh;
	
	lh = (**th).bStarts;
	l = 0;
	r = (**th).bLines - 1;
	while (l < r) {
		m = (l + r + 1) / 2;
		if (pos >= (**lh) [m])
			l = m;
		else if (m == l + 1)
			break;
		else
			r = m;
	}
	return l;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::FindRun (short clicks, long h, long v, long *cbeg,
		long *cend, long *cloc)
{
	TBHandle th;
	LSHandle lh;
	long pos, bpos, epos, bline, eline;
	Boolean left;
	
	if (fLength == 0) {
		*cbeg = 0;
		*cend = 0;
		*cloc = 0;
	} else {
		if (v < fDisplay.top) {
			v = fDisplay.top;
			h = fBounds.left - 1;
		} else if (v > fDisplay.bottom) {
			v = fDisplay.bottom;
			h = fBounds.left - 1;
		}
		Vert2Buf (v, &th, &bpos);
		if (bpos >= (**th).bLength) {
			epos = bpos;
			if (bpos > 0) bpos -= 1;
			left = FALSE;
		} else {
			Hor2Buf (h, th, &bpos, &left);
			epos = (bpos >= (**th).bLength) ? bpos : bpos + 1;
		}
		pos = left ? bpos : epos;
		if (clicks == 1) {
			bpos = pos;
			epos = pos;
		} else if (clicks == 2) {
			if (pWChars [(**th).bBuf [bpos]]) {
				while ((bpos > 0) && pWChars [(**th).bBuf [bpos - 1]])
					bpos -= 1;
				while ((epos < (**th).bLength) && pWChars [(**th).bBuf [epos]])
					epos += 1;
			}
		} else /* clicks > 2 */ {
			lh = (**th).bStarts;
			bline = FindLine (th, bpos);
			eline = bline + 1;
			while ((bline > 0) &&
					((**th).bBuf [(**lh) [bline] - 1] != chReturn))
				bline -= 1;
			while ((eline < (**th).bLines) &&
					((**th).bBuf [(**lh) [eline] - 1] != chReturn))
				eline += 1;
			bpos = (**lh) [bline];
			epos = (**lh) [eline];
		}
		*cbeg = Buf2Char (th, bpos);
		*cend = epos - bpos + *cbeg;
		*cloc = pos - bpos + *cbeg;
	}
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TBigText::Free (void)
{
	while (fText) DropBuf ();
	if (fMemReserve) DisposIfHandle (fMemReserve);
	fMemReserve = NULL;
	inherited::Free ();
}

//------------------------------------------------------------------------------

pascal void TBigText::Highlight (long start, long end)
{
	TBHandle th;
	long pos, tmp;
	VRect vr;
	Rect r, tr;
	short left, right;

	if (start == end) return;
	if (start > end) {
		tmp = end; end = start; start = tmp;
	}
	Char2Buf (start, &th, &pos);
	vr.top = Buf2Vert (th, pos);
	vr.left = Buf2Hor (th, pos);
	Char2Buf (end, &th, &pos);
	vr.bottom = Buf2Vert (th, pos) + fLineHeight;
	vr.right = Buf2Hor (th, pos);
	VRectToRect (&vr, &r);
	if ((r.top > fDisplay.bottom) || (r.bottom < fDisplay.top)) return;
	left = (fBounds.left < -32000) ? -32000 : (short) fBounds.left;
	right = (fBounds.right > 32000) ? 32000 : (short) fBounds.right;
	PenPat (qd.gray);
	PenMode (patXor);
	UseSelectionColor ();
	if (r.top + fLineHeight != r.bottom) {
		tr = r;
		tr.bottom = r.top + (short) fLineHeight;
		tr.right = right;
		if (tr.right > tr.left) InvertRect (&tr);
		tr.top = r.top + (short) fLineHeight;
		tr.bottom = r.bottom - (short) fLineHeight;
		tr.left = left;
		tr.right = right;
		if (tr.top < tr.bottom) InvertRect (&tr);
		tr = r;
		tr.top = r.bottom - (short) fLineHeight;
		tr.left = left;
		if (tr.right > tr.left) InvertRect (&tr);
	} else
		InvertRect (&r);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::Hor2Buf (long h, TBHandle th, long *pos, Boolean *left)
{
	TextStyle theStyle;
	long p, len, hor, i, w, epos;
	SignedByte oldState;
	
	theStyle = fStyle;
	SetPortTextStyle (&theStyle);
	p = *pos;
	h -= fBounds.left;
	hor = 0;
	w = 0;
	epos = (**(**th).bStarts) [FindLine (th, p) + 1];
	while (p < epos) {
		len = ChunkSize (th, p, epos);
		if (len > 0) {
			if (len > kMaxChunk) len = kMaxChunk;
			oldState = HGetState ((Handle) th);
			HLock ((Handle) th);
			MeasureText ((short) len, (Ptr) ((**th).bBuf + p),
				(Ptr) pCharLocs);
			HSetState ((Handle) th, oldState);
			for (i = 1; i <= len; i++) {
				w = pCharLocs [i] - pCharLocs [i - 1];
				if (hor + w > h) break;
				hor += w;
				p += 1;
			}
			if (i <= len) break;
		} else if ((**th).bBuf [p] == chTab) {
			w = fTabWidth - (hor % fTabWidth);
			if (hor + w > h) break;
			hor += w;
			p += 1;
		} else /* chReturn */ {
			w = (fBounds.right - fBounds.left) * 2;
			break;
		}
	}
	*left = (p < epos) && (h < hor + (w / 2));
	if (p < epos)
		*pos = p;
	else if (epos > *pos)
		*pos = epos - 1;
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TBigText::IBigText (Rect *display, TextStyle *theStyle,
		long tabChars, TView *itsView)
{
	if (!BTScroll_clip) {
		BTScroll_clip = NewRgn ();
		FailNIL (BTScroll_clip);
	}
	if (!BTScroll_scrl) {
		BTScroll_scrl = NewRgn ();
		FailNIL (BTScroll_scrl);
	}
	if (!Draw_clip) {
		Draw_clip = NewRgn ();
		FailNIL (Draw_clip);
	}
	if (!Draw_tmp) {
		Draw_tmp = NewRgn ();
		FailNIL (Draw_tmp);
	}
	fText = NULL;
	fMemReserve = NULL;
	IObject ();
	fSuperView = itsView;
	IObject ();
	fSelStart = 0;
	fSelEnd = 0;
	fLength = 0;
	fMaxLength = 0;
	fMaxLowSize = kBTRsrv;
	fCurVers = 1;
	fDisplay = *display;
	fBounds.top = fDisplay.top;
	fBounds.left = fDisplay.left;
	fBounds.bottom = fDisplay.top;
	fBounds.right = fDisplay.right;
	fActive = FALSE;
	fAutoWrap = TRUE;
	fLineHeight = 11; /* dummy, needed for AppendBuf, changed in BTSetStyle */
	FailNIL (AppendBuf (NULL, 0));
	BuildReserve (TRUE);
	FailNIL (fMemReserve);
	BTSetStyle (theStyle, tabChars, kDontRedraw);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::Recalc (long vfrom, long vto, Boolean adjTop)
{
	TBHandle th, eh;
	long oldBot, delta, pos;
	
	BuildReserve (FALSE);
	do { /* recalc more than was asked (simplifies scrolling) */
		delta = 0;
		if (vto > fBounds.bottom) delta = fBounds.bottom - vto;
		if (vfrom + delta < fBounds.top) delta = fBounds.top - vfrom;
		vfrom += delta;
		vto += delta;
		Vert2Buf (vfrom, &th, &pos);
		Vert2Buf (vto, &eh, &pos);
		eh = (**eh).bNext;
		oldBot = fBounds.bottom;
		while (th && (th != eh)) {
			RecalcBuf (th);
			th = (**th).bNext;
		}
		delta = fBounds.bottom - oldBot;
		if (adjTop) {
			fBounds.top -= delta;
			fBounds.bottom -= delta;
		}
	} while (delta < 0); /* recalc again if number of lines decreased */
	BuildReserve (TRUE);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::RecalcBuf (TBHandle th)
{
	TextStyle theStyle;
	long pos, epos, line, len, maxlen, hor, width;
	long l, brk, oldbrk, oldloc;
	LSHandle lh;
	SignedByte oldState;
	unsigned char ch;
	Boolean done;
	
	if ((**th).bVersion == fCurVers) return;
	done = TRUE;
	lh = (**th).bStarts;
	(**lh) [0] = 0;
	line = 1;
	pos = 0;
	if (fAutoWrap) {
		theStyle = fStyle;
		SetPortTextStyle (&theStyle);
		epos = (**th).bLength;
		width = fBounds.right - fBounds.left;
		if (width <= 0) width = 1;
		hor = 0;
		while (pos < epos) {
			maxlen = ChunkSize (th, pos, epos);
			len = (maxlen > kMaxChunk) ? kMaxChunk : maxlen;
			if (len > 0) {
				oldState = HGetState ((Handle) th);
				HLock ((Handle) th);
				MeasureText ((short) len, (Ptr) ((**th).bBuf + pos),
					(Ptr) pCharLocs);
				HSetState ((Handle) th, oldState);
				oldloc = 0;
			}
			oldbrk = 0;
			while (oldbrk < len) {
				l = oldbrk + 2;
				while ((l <= len) && (hor + pCharLocs [l] - oldloc <= width))
					l += 1;
				if ((l <= len) && ((**th).bBuf [pos + l - 1] == chSpace))
					l += 1;
				brk = l - 1;
				if (l > len) {
					if ((len < maxlen) && (oldbrk > 0)) {
						pos += oldbrk;
						break;
					}
					hor += pCharLocs [brk] - oldloc;
					oldloc = pCharLocs [brk];
					pos += brk;
				} else {
					while ((brk > oldbrk) &&
							((**th).bBuf [pos + brk - 1] != chSpace))
						brk -= 1;
					if ((brk <= oldbrk) && (hor == 0)) brk = l - 1;
					if ((brk <= oldbrk) && (hor == 0)) brk = oldbrk + 1;
					done &= SetLSVal (lh, line++, (short) (pos + brk));
					hor = 0;
					oldloc = pCharLocs [brk];
				}
				oldbrk = brk;
			}
			while ((pos < epos) && ((len == maxlen) || (oldbrk == 0))) {
				ch = (**th).bBuf [pos];
				if (ch == chTab) {
					if (hor >= width) {
						done &= SetLSVal (lh, line++, (short) pos);
						hor = 0;
					}
					hor += fTabWidth - (hor % fTabWidth);
					pos += 1;
				} else if (ch == chReturn) {
					pos += 1;
					done &= SetLSVal (lh, line++, (short) pos);
					hor = 0;
				} else
					break;
			}
		}
		if ((epos > 0) && ((**th).bBuf [epos - 1] == chReturn))
			line -= 1;
		else
			done &= SetLSVal (lh, line, (short) epos);
	} else {
		epos = (**th).bLength - 1;
		while (pos < epos)
			if ((**th).bBuf [pos++] == chReturn)
				done &= SetLSVal (lh, line++, (short) pos);
		done &= SetLSVal (lh, line, (short) epos + 1);
	}
	if (done)
		(**th).bVersion = fCurVers;
	else {
		line = 1;
		SetLSVal (lh, line, (**th).bLength);
		(**th).bVersion = 0;
	}
	SetPermHandleSize ((Handle) lh, sizeof (short) * (long) (line + 1));
	fBounds.bottom += (line - (**th).bLines) * fLineHeight;
	(**th).bLines = (short) line;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TBigText::SetLSVal (LSHandle lh, long line, short val)
{
	long size;
	Boolean done;
	
	size = GetHandleSize ((Handle) lh) / sizeof (short);
	if (size <= line) {
		size += line + kLSChunk;
		size *= sizeof (short);
		SetPermHandleSize ((Handle) lh, size);
		done = GetHandleSize ((Handle) lh) == size;
	} else
		done = TRUE;
	if (done) (**lh) [line] = val;
	return done;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TBigText::Vert2Buf (long v, TBHandle *th, long *pos)
{
	TBHandle h;
	
	v = (v - fBounds.top) / fLineHeight;
	if (v < 0) v = 0;
	h = fText;
	while ((**h).bNext && (v >= (**h).bLines)) {
		v -= (**h).bLines;
		h = (**h).bNext;
	}
	*th = h;
	if (v >= (**h).bLines)
		*pos = (**h).bLength;
	else
		*pos = (**(**h).bStarts) [v];
}

//------------------------------------------------------------------------------
