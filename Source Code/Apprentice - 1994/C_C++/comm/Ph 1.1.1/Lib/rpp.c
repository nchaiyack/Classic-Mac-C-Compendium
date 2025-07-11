/*______________________________________________________________________

	rpp.c - Report Printing Module.
	
	Copyright � 1988-1991 Northwestern University.
	
	This reusable module implements the printing of reports.
	
	All the code is placed in its own segment named "rpp".
_____________________________________________________________________*/

#pragma load "precompile"
#include "rpp.h"
#include "utl.h"
#include "doc.h"

#pragma segment rpp

/*______________________________________________________________________

	Globals.
_____________________________________________________________________*/

static Rect			PrintBox;			/* box enclosing text */
static short		PrintBoxHeight;	/* print box height */
static short		PrintBoxWidth;		/* print box width */
static short		FirstPage;			/* first page number to print */
static short		LastPage;			/* last page number to print */
static Str255		NowDate;				/* current date */
static Str255		NowTime;				/* current time */
static short		TitleSize;			/* font size for headers */
static short		TconCompute = 0;	/* 0 if tcon width variables not yet computed, 
													else 1 */
static Boolean		TruncateRight;		/* true if print box too narrow */
static Boolean		TruncateBottom;	/* true if print box too short */

/* Page break array for type 0 reports.
	
	(**Breaks0)[n] = info for page FirstPage+n,
	for n = 0 through LastPage-FirstPage.
	
*/

typedef struct Break0Info {
	short				firstLine;			/* line number of first line on page */
	short				numLines;			/* number of lines on page */
} Break0Info;

static Break0Info		(**Breaks0)[];		/* handle to page break info */

/* Page break array for type 1 reports.
	
	(**Breaks1)[n] = info for page n,
	for n = 1 through the total number of pages in the document.
	
	Note that for type 1 reports we build and keep the entire page break
	array, not just the portion from FirstPage through LastPage.  This is
	because the entire array is needed to print the table of contents.
	
*/

typedef struct Break1Info {
	short				auxInxB;			/* index in aux array of beginning
												of page */
	short				offsetB;			/* offset in STR# resource of beginning
												of page */
	short				auxInx;			/* index in aux array of first non-blank
												line on page */
	short				offset;			/* offset in STR# resource of first
												non-blank line on page */
	Boolean			header;			/* true if header on this page */
} Break1Info;

static Break1Info		(**Breaks1)[];		/* handle to page break info */

/*	Item numbers of items in custom page setup dialog. */

#define	sepLineItem		1
#define	fontListItem	2
#define	sizeTEItem		3
#define	sizeListItem	4
#define	reverseItem		5
#define	leftTEItem		6
#define	rightTEItem		7
#define	topTEItem		8
#define	bottomTEItem	9

/*	Global variables for custom page setup dialog. */

static TPPrDlg 			PrtStlDialog;		/* pointer to stream object for page setup */
static TPPrDlg				PrtJobDialog;		/* pointer to stream object for printing */
static short 				FirstItem;			/* our first item number - 1 */
static rpp_PrtBlock		*PBlock;				/* pointer to rpp_PrtBlock param block */
static PItemProcPtr		StdItemProc;		/* pointer to standard item proc */
static ModalFilterProcPtr	StdFltrProc;	/* pointer to standard filter proc */
static ListHandle			FontList;			/* handle to font list */	
static ListHandle			SizeList;			/* handle to size list */

/*______________________________________________________________________

	GetLSep - Get Line Separation
	
	Entry:	fontNum = font number.
				fontSize = font size.
	
	Exit:		function result = line speparation.
_____________________________________________________________________*/

static short GetLSep (short fontNum, short fontSize)

{
	FontInfo			fontInfo;		/* font info */
	
	TextFont(fontNum);
	TextSize(fontSize);
	TextFace(normal);
	GetFontInfo(&fontInfo);
	return fontInfo.ascent + fontInfo.descent + fontInfo.leading;
}

/*______________________________________________________________________

	GetHH - Get Header Height
	
	Entry:	p = pointer to parameter block
	
	Exit:		function result = header height.
_____________________________________________________________________*/

static short GetHH (rpp_PrtBlock *p)

{
	FontInfo			fontInfo;		/* font info */
	
	TextFont(p->titleFont);
	TextSize(TitleSize);
	TextFace(p->titleStyle);
	GetFontInfo(&fontInfo);
	return fontInfo.ascent + fontInfo.descent + 
		p->titleSep * (**p->hPrint).prInfo.iVRes / 100;
}

/*______________________________________________________________________

	CheckHeaderWidth - Check Page Header Width.
	
	Entry:	p = pointer to parameter block.
_____________________________________________________________________*/

static void CheckHeaderWidth (rpp_PrtBlock *p)

{
	Str255		rhSide;				/* date, time, and page number */
	
	if (!p->header) return;
	TextFont(p->titleFont);
	TextSize(TitleSize);
	TextFace(p->titleStyle);
	utl_PlugParams(p->titleTmpl, rhSide, NowDate, NowTime, "\p999", nil);
	if (StringWidth(p->title) + StringWidth(rhSide) + 10 > PrintBoxWidth)
		TruncateRight = true;
}

/*______________________________________________________________________

	CheckCancel - Check for Cancel of Printing Operation.
	
	Exit:			function result = true if canceled, else false.
_____________________________________________________________________*/

static Boolean CheckCancel(void)

{
	EventRecord		myEvent;				/* event record */
	char				key;					/* key pressed */
	
	if (utl_WaitNextEvent(everyEvent ^ diskMask, &myEvent, 0, nil)) {
		switch (myEvent.what) {
			case keyDown:
				key = myEvent.message & charCodeMask;
				if (((myEvent.modifiers & cmdKey) && key == '.') ||
					key == escapeKey) return true;
				break;
			default:
				break;
		}
	}
	return false;
}

/*______________________________________________________________________

	Break0 - Compute Page Breaks for a Type 0 Report.
	
	Entry:	theList = handle to list record.
				p = pointer to parameter block.
				
	Exit:		function result = true if canceled by user.
_____________________________________________________________________*/

static Boolean Break0 (ListHandle theList, rpp_PrtBlock *p)

{
	GrafPtr			port;				/* current grafport */
	short				oldFont;			/* saved font number */
	short				oldFace;			/* saved font style */
	short				oldSize;			/* saved font size */
	long				lineNum;			/* line number of first line on cur page */
	short				lastLine;		/* last line number in report + 1 */
	short				pageNum;			/* current page number */
	short				lineSep;			/* line separation */
	short				height;			/* height of page */
	long				linesPerPage;	/* number of lines per page */
	Break0Info		breakInfo;		/* page break info for current page */
	short				endPage;			/* last page number */
	short				numLines;		/* number of lines left to check */
	Cell				theCell;			/* list manager cell */
	char				line[256];		/* line to be printed */
	short				lineLen;			/* length of line */
	Boolean			canceled;		/* true if canceled by user */
	
	/* Save font info. */
	
	GetPort(&port);
	oldFont = port->txFont;
	oldFace = port->txFace;
	oldSize = port->txSize;	
	
	/* Check header width. */
	
	CheckHeaderWidth(p);

	/* Compute linesPerPage = number of report lines per page. */
	
	height = PrintBoxHeight;	
	if (p->header) height -= GetHH(p);
	lineSep = GetLSep(p->fontNum, p->fontSize);
	linesPerPage = height/lineSep;
	
	/* Check to see if any lines are too wide. */
	
	lastLine = (**theList).dataBounds.bottom;
	numLines = lastLine;
	SetPt(&theCell, 0, 0);
	canceled = false;
	while (numLines--) {
		if (canceled = CheckCancel()) break;
		lineLen = 256;
		LGetCell(line, &lineLen, theCell, theList);
		if (TextWidth(line, 0, lineLen) > PrintBoxWidth) {
			TruncateRight = true;
			break;
		}
		theCell.v++;
	}
	
	/* Restore font info. */
	
	TextFont(oldFont);
	TextFace(oldFace);
	TextSize(oldSize);
	
	/* Compute the PageBreaks array. */
	
	lineNum = (FirstPage-1)*linesPerPage;
	if (lineNum >= lastLine) {
		LastPage = FirstPage-1;
		Breaks0 = (Break0Info(**)[])NewHandle(0);
	} else {
		endPage = FirstPage + (lastLine-lineNum-1)/linesPerPage;
		if (endPage < LastPage) LastPage = endPage;
		Breaks0 = 
			(Break0Info(**)[])NewHandle(sizeof(Break0Info)*(LastPage-FirstPage+1));
		for (pageNum = FirstPage; pageNum <= LastPage; pageNum++) {
			breakInfo.firstLine = lineNum;
			lineNum += linesPerPage;
			breakInfo.numLines = (lineNum <= lastLine) ? linesPerPage :
				(lastLine - breakInfo.firstLine);
			(**Breaks0)[pageNum-FirstPage] = breakInfo;
		}
	}
	return canceled;
}

/*______________________________________________________________________

	LoadStrings - Load STR# Resource.
	
	Entry:	theList = handle to list record.
				auxInx = index in aux array.
				
	Exit:		theStrings = handle to loaded and locked STR# resource.
				endStrings = pointer to byte following STR# resource.
				endOfDoc = true if end of STR# resources.
_____________________________________________________________________*/

static void LoadStrings (ListHandle theList, short auxInx, 
	Handle *theStrings, unsigned char **endStrings, Boolean *endOfDoc)

{
	auxInfo			**aux;			/* handle to auxiliary info */
	Handle			s;					/* handle to STR# resource */

	aux = (auxInfo**)(**theList).userHandle;
	if (!(s = (**aux).auxArray[auxInx])) {
		*endOfDoc = true;
		*theStrings = nil;
		return;
	}
	if (!*s) LoadResource(s);
	MoveHHi(s);
	HLock(s);
	*theStrings = s;
	*endStrings = (unsigned char*)*s + GetHandleSize(s);
}

/*______________________________________________________________________

	Wrap - Wrap Paragraph.
	
	Entry:	printing = true to print paragraph, false to just compute
					paragraph size.
				v = vertical position on page.
				fontNum = font number.
				fontSize = font size.
				fontStyle = font style.
				just = justification (docLeft, docCenter, or docRight).
				theLine = pointer to first line of paragraph.
				firstChar = pointer to first char of paragraph (following
					any escape sequences in first line).
	
	Exit:		allBlank = true if paragraph is all blanks.
				nLines = number of lines in paragraph.
				lastLine = pointer to last line in paragraph.
_____________________________________________________________________*/

static void Wrap (Boolean printing, short v, short fontNum, 
	short fontSize, short fontStyle, short just, unsigned char *theLine, 
	unsigned char *firstChar, Boolean *allBlank, short *nLines, 
	unsigned char **lastLine)

{
	char					pl[200];			/* line to be printed */
	short					npl;				/* length of print line */
	short					n;					/* number of lines */
	short					nbl;				/* num chars in blank run */
	short					xbl;				/* set to 1 for interpolated blank */
	short					nw;				/* number of words */
	unsigned char		*q;				/* pointer into cur line */
	unsigned char		*qend;			/* pointer to end+1 of cur line */
	Boolean				eop;				/* true if end of paragraph */
	Boolean				allb;				/* true if all blank paragraph */
	short					lineSep;			/* line separation */
	FontInfo				fontInfo;		/* font info */
	short					w;					/* actual width of printed line */
	
	/* Get font info and set proper font number, size, and style. */
	
	TextFont(fontNum);
	TextSize(fontSize);
	TextFace(normal);
	GetFontInfo(&fontInfo);
	TextFace(fontStyle);
	
	/* Initialize variables. */
	
	n = 1;
	allb = true;
	npl = 0;
	q = firstChar;
	qend = theLine + *theLine + 1;
	eop = false;
	lineSep = fontInfo.leading + fontInfo.ascent + fontInfo.descent;
	if (printing) v += fontInfo.leading + fontInfo.ascent;
	xbl = 0;
	
	/* Main loop.  Wrap the entire paragraph. */
	
	while (!eop) {
	
		/* Check to see if this is the last line in the paragraph.
			If it is, back over the eop marker at the end of the line. */
	
		if (eop = (q < qend && *(qend-1) == docEop)) qend--;
		
		/* Break out blank runs and words from the line one at a time. */
		
		while (q < qend) {
		
			/* Get nbl = num blanks in next blank run. */
			
			nbl = xbl;
			xbl = 0;
			while (q < qend && *q == ' ') {
				q++;
				nbl++;
			}
			if (q == qend) break;
			
			/* Get nw = num chars in next word */
			
			allb = false;
			nw = 0;
			while (q < qend && *q != ' ') {
				q++;
				nw++;
			}
			
			/* Append blanks and word to current line. */
			
			if (nbl) memset(pl+npl, ' ', nbl);
			npl += nbl;
			memcpy(pl+npl, q-nw, nw);
			npl += nw;
			
			/* Check to see if it's time to wrap.  If it is, print the
				current line and start a new one.  Put the word we just
				broke out of the paragraph at the beginning of the next
				line. */
			
			w = TextWidth(pl, 0, npl);
			if (w > PrintBoxWidth) {
				if (printing) {
					switch (just) {
						case docLeft:
							MoveTo(PrintBox.left, v);
							break;
						case docCenter:
							MoveTo(PrintBox.left + ((PrintBoxWidth-w)>>1), v);
							break;
						case docRight:
							MoveTo(PrintBox.right-w, v);
							break;
					}
					DrawText(pl, 0, npl-nw-nbl);
					v += lineSep;
				} else if (nw == npl) TruncateRight = true;
				n++;
				memcpy(pl, q-nw, nw);
				npl = nw;
			}
			
		}
		
		/* Break if end of paragraph, else advance to next line.
			Set xbl = 1 to force a blank to be interpolated at the 
			beginning of the next line. */
		
		if (eop) break;
		theLine += *theLine + 1;
		q = theLine + 1;
		qend = q + *theLine;
		xbl = 1;
		
	}
	
	/* Print the last line. */
	
	if (printing) {
		w = TextWidth(pl, 0, npl);
		switch (just) {
			case docLeft:
				MoveTo(PrintBox.left, v);
				break;
			case docCenter:
				MoveTo(PrintBox.left + ((PrintBoxWidth-w)>>1), v);
				break;
			case docRight:
				MoveTo(PrintBox.right-w, v);
				break;
		}
		DrawText(pl, 0, npl);
	}
	
	/* Set exit params and return. */
	
	*allBlank = allb;
	*nLines = n;
	*lastLine = theLine;
}

/*______________________________________________________________________

	Break1 - Compute Page Breaks for a Type 1 Report.
	
	Entry:	theList = handle to list record.
				p = pointer to parameter block.
				
	Exit:		Break1 array records positions of page breaks.
				function result = true if canceled by command-period.
_____________________________________________________________________*/

static Boolean Break1 (ListHandle theList, rpp_PrtBlock *p)

{
	GrafPtr			port;					/* current grafport */
	short				oldFont;				/* saved font number */
	short				oldFace;				/* saved font style */
	short				oldSize;				/* saved font size */
	short				nalloc;				/* number of elements allocated in
													Breaks1 array */
	Break1Info		breakInfo;			/* break info */													
	short				pageNum;				/* current page number */
	short				v;						/* current vert pos on page */
	short				auxInx;				/* cur index in aux array */
	Boolean			allBlank;			/* true if in initial blank portion
													of block */
	Boolean			allBPar;				/* true if all blank paragrah */													
	short				hb;					/*	height of initial blank portion
													of block */
	short				hnb;					/* height of non-blank portion of block */
	Boolean			force;				/* true if page break forced by
													\page directive */
	short				lineSep;				/* line separation for normal lines */
	short				h;						/* height of next section */
	Boolean			header;				/* true if header on next page */
	Boolean			endOfDoc;			/* true if end of doc */
	short				headerHeight;		/* height of page header */	
	Handle			theStrings;			/* handle to STR# resource */
	unsigned char	*endStrings;		/* ptr to byte followng STR# rsrc */
	unsigned char	*theLine;			/* pointer to cur line in STR# rsrc */
	unsigned char	*q;					/* pointer to cur pos in line */
	unsigned char	*qEnd;				/* pointer to end of line */
	Boolean			keepBlock;			/* true if \keep directive encountered */
	Boolean			done;					/* true when end of section found */
	Boolean			paragraph;			/* true if start of paragraph */
	short				lSep;					/* size adjusted line separation */
	short				newSize;				/* scaled font size */
	Boolean			pict;					/* true if picture */
	short				picID;				/* pict id */
	PicHandle		picHandle;			/* pict handle */
	Rect				picRect;				/* pict frame rectangle */
	short				style;				/* font style */
	short				nLines;				/* num lines in paragraph */
	short				cellHeight;			/* height of list manager cells */
	Boolean			laser;				/* true if laserwriter */
	Boolean			canceled;			/* true if canceled */
	Boolean			tcon;					/* true if tcon escape seq encountered */
	short				tconMaxT;			/* width of widest tcon title */
	short				tconMaxN;			/* width of widest tcon page number */
	unsigned char	*tconPtr;			/* ptr to cur pos in TCON rsrc */
	short				tconCount;			/* num TCON entries left to process */
	Handle			tconHandle;			/* handle to TCON rsrc */
	short				tconTWidth;			/* width of title */
	unsigned char	*tconStart;			/* ptr to first non-blank char of tcon
													title */
	short				tconNBl;				/* number of leading blanks in tcon
													title */
	char				tconFlags;			/* saved tconHandle flag byte */
	
	/* Save font number, style, and size. */
	
	GetPort(&port);
	oldFont = port->txFont;
	oldFace = port->txFace;
	oldSize = port->txSize;
	
	/* Check header width. */
	
	CheckHeaderWidth(p);

	/* Initialize variables. */
	
	pageNum = 0;
	v = PrintBox.bottom + 1;
	auxInx = 0;
	lineSep = GetLSep(p->fontNum, p->fontSize);
	headerHeight = GetHH(p);
	cellHeight = (**theList).cellSize.v;
	endOfDoc = false;
	laser = utl_IsLaser(p->hPrint);
	canceled = false;
	tcon = false;
	theStrings = nil;
	
	/* Load the first STR# resource and lock it. */
	
	LoadStrings(theList, auxInx, &theStrings, &endStrings, &endOfDoc);
	theLine = (unsigned char*)*theStrings+2;
	
	/* Allocate 10 initial elements in Breaks1 array. */
	
	Breaks1 = (Break1Info(**)[])NewHandle(10*sizeof(Break1Info));
	nalloc = 10;
	
	/* Main loop. Paginate the entire document. */
	
	while (true) {
	
		/* Check for command-period */
		
		if (canceled = CheckCancel()) break;
	
		hb = hnb = 0;
		force = false;
		keepBlock = false;
		breakInfo.auxInxB = breakInfo.auxInx = auxInx;
		breakInfo.offsetB = breakInfo.offset = 
			theLine - (unsigned char*)*theStrings;
		
		/* Find next "block".  A block is defined to be a possibly empty
			sequence of blank lines, followed by a keep block, a paragraph,
			or a picture.
			
			Our goal is to compute the following:
			
			hb = height of initial blank portion of block.
			hnb = height of non-blank portion of block.
			theLine = pointer to line following block. 
			breakInfo.auxInx = index in aux array of beginning of non-blank
				portion of block.
			breakInfo.offset = offset in STR# of beginning of non-blank
				portion of block. */
		
		while (true) {
		
			h = 0;
			allBlank = true;
			done = false;
			
			/* Find next "section".  A section is defined to be an empty
				line, a keep block, a paragraph, or a picture.
				
				Our goal is to compute the following:
				
				h = height of section.
				theLine = pointer to line following section.
				force = true if \page directive encountered.
				allBlank = true if blank line. */
			
			while (!done) {
				
				lSep = lineSep;
				q = theLine+1;
				qEnd = q + *theLine;
				paragraph = true;
				pict = false;
				style = normal;
				newSize = p->fontSize;
				
				/* Crack escape sequences. */
				
				while (q < qEnd && *q < 31) {
					switch (*q) {
						case docStyle:
							style = *(q+2);
							break;
						case docSize:
							newSize = utl_ScaleFontSize(p->fontNum, p->fontSize,
								*(q+2)<<8 | *(q+3), laser);
							lSep = GetLSep(p->fontNum, newSize);
							break;
						case docOnly:
							paragraph = *(q+2) & docPrint;
							break;
						case docPict:
							pict = true;
							picID = *(q+2)<<8 | *(q+3);
							paragraph = false;
							break;
						case docPage:
							force = true;
							header = !*(q+2);
							paragraph = false;
							break;
						case docKeep:
							keepBlock = true;
							paragraph = false;
							break;
						case docEndKeep:
							done = true;
							paragraph = false;
							break;
						case docITcon:
							h += lineSep;
							allBlank = false;
							done = true;
							paragraph = false;
							tcon = true;
							break;
					}
					if (!paragraph) break;
					q += *(q+1);
				}
				
				/* Rewrap paragraph. */
				
				if (paragraph) {
					Wrap(false, 0, p->fontNum, newSize, style, docLeft, theLine,
						q, &allBPar, &nLines, &theLine);
					allBlank &= allBPar;
					h += nLines * lSep;
					done = !keepBlock;
				}
				
				if (pict) {
				
					/* For a picture, count the bands and compute h = the picture
						height. */
						
					while (true) {
						h += cellHeight;
						theLine += *theLine + 1;
						q = theLine + 1;
						qEnd = q + *theLine;
						pict = false;
						while (q < qEnd && *q < 31) {
							if (*q == docPict) {
								pict = true;
								break;
							}
							q += *(q+1);
						}
						if (!pict) break;
					}
					allBlank = false;
					done = !keepBlock;
					
					/* Check to see if picture is too wide for specified margins. */
					
					picHandle = GetPicture(picID);
					picRect = (**picHandle).picFrame;
					if ((picRect.right - picRect.left) > PrintBoxWidth) TruncateRight = true;
					
				} else {
				
					/* If not a picture, advance to next line. */
				
					theLine += *theLine + 1;
					
				}
				
				/* Advance to next STR# resource if necessary. */
				
				if (theLine >= endStrings) {
					HUnlock(theStrings);
					auxInx++;
					LoadStrings(theList, auxInx, &theStrings, &endStrings, 
						&endOfDoc);
					if (endOfDoc) break;
					theLine = (unsigned char*)*theStrings + 2;
				}
				
			}
			
			/* At this point we have found the next section, and h = the
				height of the section. */
			
			if (allBlank) {
			
				/* Accumulate height of blank line and loop for next section. */
			
				hb += h;
				breakInfo.auxInx = auxInx;
				breakInfo.offset = theLine - (unsigned char*)*theStrings;
				if (endOfDoc) break;
				
			} else {
			
				/* Not blank line - set height of keep block, picture, or
					paragraph and break out of the section loop. */
			
				hnb = h;
				break;
				
			}
			
		}
		
		/* At this point we have found the next block, and we have: 
			hb = height of blank portion at beginning of block.
			hnb = height of non-blank portion (keep block, pict, or paragraph).
		
			Check to see if there's enough room on the page for this block. */
		
		if (v+hb+hnb > PrintBox.bottom || force) {
			
			/* Not enough room for block or \page encountered - do a page break.  
				Don't put blank lines at the top of the page. */
		
			pageNum++;
			breakInfo.header = header;
			if (pageNum >= nalloc) {
				SetHandleSize((Handle)Breaks1, 
					GetHandleSize((Handle)Breaks1) + 10*sizeof(Break1Info));
				nalloc += 10;
			}
			(**Breaks1)[pageNum] = breakInfo;
			v = PrintBox.top;
			if (header) v += headerHeight;
			v += hnb;
			if (v > PrintBox.bottom) TruncateBottom = true; 
			
		} else {
		
			/* Enough room for block - advance position on page. */
		
			v += hb + hnb;
			
		}
		
		if (endOfDoc) break;
		
	}
	
	/* Set LastPage and adjust final size of Break1 array. */
	
	if (pageNum < LastPage) LastPage = pageNum;
	if (LastPage < FirstPage) LastPage = FirstPage-1;
	SetHandleSize((Handle)Breaks1, (pageNum+1)*sizeof(Break1Info));
	
	/* If a tcon escape sequence was encountered, check to see if the tcon
		entries are wider than the print box width. */
		
	if (tcon && !canceled) {
		TextFont(p->fontNum);
		TextSize(p->fontSize);
		TextFace(normal);
		tconHandle = GetResource('TCON', p->tabConID);
		if (!*tconHandle) LoadResource(tconHandle);
		tconFlags = utl_HGetState(tconHandle);
		MoveHHi(tconHandle);
		HLock(tconHandle);
		tconCount = **(short**)tconHandle;
		tconPtr = (unsigned char*)*tconHandle+2;
		tconMaxT = 0;
		while (tconCount--) {
			if (*(tconPtr+2) & docPrint) {
				tconStart = tconPtr+4;
				while (*tconStart == ' ') tconStart++;
				tconNBl = tconStart - tconPtr - 4;
				tconTWidth = ((tconNBl*p->fontSize)>>1) +
					TextWidth(tconStart, 0, *(tconPtr+3)-tconNBl);
				if (tconTWidth > tconMaxT) tconMaxT = tconTWidth;
			}
			tconPtr += *(tconPtr+3) + 4;
			if ((long)tconPtr & 1) tconPtr++;
		}
		tconMaxN = StringWidth("\p999");
		if (tconMaxT+tconMaxN+10 > PrintBoxWidth) TruncateRight = true;
		utl_HSetState(tconHandle, tconFlags);
	}
	
	if (theStrings) HUnlock(theStrings);
	
	/* Restore font number, style, and size. */
	
	TextFont(oldFont);
	TextFace(oldFace);
	TextSize(oldSize);
	
	return canceled;
}

/*______________________________________________________________________

	PrintHeader - Print Page Header.
	
	Entry:	p = pointer to parameter block.
				pageNum = page number to print.
_____________________________________________________________________*/

static void PrintHeader (rpp_PrtBlock *p, short pageNum)

{
	FontInfo		fontInfo;			/* font info */
	short			v;						/* vert coord of header base line */
	Str255		pNum;					/* page number */
	Str255		rhSide;				/* date, time, and page number */
	
	TextFont(p->titleFont);
	TextSize(TitleSize);
	TextFace(p->titleStyle);
	GetFontInfo(&fontInfo);
	v = PrintBox.top + fontInfo.ascent;
	MoveTo(PrintBox.left, v);
	DrawString(p->title);
	NumToString(pageNum, pNum);
	utl_PlugParams(p->titleTmpl, rhSide, NowDate, NowTime, pNum, nil);
	MoveTo(PrintBox.right - StringWidth(rhSide), v);
	DrawString(rhSide);
}

/*______________________________________________________________________

	Print0 - Print one Page from a Type 0 Report.
	
	Entry:	theList = handle to list record.
				p = pointer to parameter block.
				pageNum = page number to print.
_____________________________________________________________________*/

static void Print0 (ListHandle theList, rpp_PrtBlock *p, short pageNum)

{
	short				v;				/* current vertical coord on page */
	FontInfo			fontInfo;	/* font info */
	short				lineSep;		/* line separation */
	Cell				theCell;		/* list manager cell */
	char				line[256];	/* line to be printed */
	short				lineLen;		/* length of line */
	Break0Info		breakInfo;	/* page break info */

	v = PrintBox.top;
	
	/* Print the page header, if requested. */
	
	if (p->header) {
		PrintHeader(p, pageNum);
		v += GetHH(p);
	}
	if (CheckCancel()) return;
	
	/* Print the page body. */
	
	TextFont(p->fontNum);
	TextSize(p->fontSize);
	TextFace(normal);
	GetFontInfo(&fontInfo);
	lineSep = fontInfo.ascent + fontInfo.descent + fontInfo.leading;
	breakInfo = (**Breaks0)[pageNum-FirstPage];
	SetPt(&theCell, 0, breakInfo.firstLine);
	while (breakInfo.numLines--) {
		if (CheckCancel()) return;
		v += lineSep;
		lineLen = 256;
		LGetCell(line, &lineLen, theCell, theList);
		MoveTo(PrintBox.left, v);
		DrawText(&line, 0, lineLen);
		theCell.v++;
	}
	return;
}

/*______________________________________________________________________

	Print1 - Print one Page from a Type 1 Report..
	
	Entry:	theList = handle to list record.
				p = pointer to parameter block.
				pageNum = page number to print.
_____________________________________________________________________*/

static void Print1 (ListHandle theList, rpp_PrtBlock *p, short pageNum)

{
	short				v;					/* current vertical coord on page */
	Break1Info		breakInfo;		/* page break info */
	short				auxInx;			/* cur index in aux array */
	short				offset;			/* cur offset in aux array */
	short				endAuxInx;		/* ending index in aux array */
	short				endOffset;		/* ending offset in STR# rsrc */
	FontInfo			fontInfo;		/* font info */
	short				lineSep;			/* line separation */
	Handle			theStrings;		/* handle to STR# rsrc */
	unsigned char	*endStrings;	/* ptr to byte following STR# rsrc */
	unsigned char	*theLine;		/* ptr to cur line in STR# rsrc */
	unsigned char	*q;				/* ptr to cur pos in line */
	unsigned char	*qEnd;			/* ptr to end of line */
	Boolean			endOfDoc;		/* true if end of doc */
	Boolean			endOfPage;		/* true if end of page */
	Boolean			allBlank;		/* true if all blank paragraph (not used) */
	Boolean			paragraph;		/* true if start of paragraph */
	Boolean			pict;				/* true if start of picture */
	short				lSep;				/* size adjusted line separation */
	short				newSize;			/* scaled font size */
	short				style;			/* font style */
	short				escJust;			/* true if just escape sequence */
	short				just;				/* justification */
	short				nLines;			/* num lines in paragraph */
	short				nBands;			/* num bands in picture */
	short				picID;			/* resource id of picture */
	PicHandle		picHandle;		/* handle to picture */
	Rect				picRect;			/* picture dest rect */
	short				picLeft;			/* left coord of picture */
	short				tconMaxT;		/* width of widest tcon title */
	unsigned char	*tconPtr;		/* ptr to cur pos in TCON rsrc */
	short				tconCount;		/* num TCON entries left to process */
	Handle			tconHandle;		/* handle to TCON rsrc */
	Break1Info		*tconBPtr;		/* ptr to cur pos in Breaks1 array */
	Break1Info		*tconBEnd;		/* ptr to end of Breaks1 array */
	short				tconPass;		/* pass number - 0 or 1 */
	short				tconLine;		/* line number */
	unsigned char	*tconData;		/* pointer to cell data */
	short				tconAuxInx;		/* index in aux array */
	short				tconOffset;		/* offset in STR# resource */
	short				tconPageNum;	/* page number */
	Str255			tconPNum;		/* page number as a string */
	short				tconTWidth;		/* width of title */
	short				tconNWidth;		/* width of page numbers */
	short				tconHPeriod;	/* hor coord of dot */
	short				tconPerSep;		/* tcon dot separation */
	unsigned char	*tconStart;		/* ptr to first non-blank char of tcon
												title */
	short				tconNBl;			/* number of leading blanks in tcon
												title */
	char				tconFlags;		/* saved tconHandle flag byte */
	unsigned char	tconLineNum;	/* tcon line number to be printed */
	Boolean			laser;			/* true if laserwriter */
	
	/* The following local variables are declared static so that they will survive
		across calls.  The variables are recomputed only once per print job. */
		
	static short	tconLeft;			/* left margin for tcon */
	static short	tconRight;			/* right margin for tcon */
	static short	tconMaxN;			/* width of widest tcon page number */

	v = PrintBox.top;
	laser = utl_IsLaser(p->hPrint);
	theStrings = nil;
	
	/* Get page break info for this page. */
	
	breakInfo = (**Breaks1)[pageNum];
	auxInx = breakInfo.auxInx;
	offset = breakInfo.offset;
	
	/* Get info about where this page ends:
		endAuxInx = index in aux array where page ends.
		endOffset = offset in STR# rsrc where page ends. */
		
	if ((pageNum+1)*sizeof(Break1Info) < GetHandleSize((Handle)Breaks1)) {
		endAuxInx = (**Breaks1)[pageNum+1].auxInx;
		endOffset = (**Breaks1)[pageNum+1].offset;
	} else {
		endAuxInx = endOffset = 0x7fff;
	}
	
	/* Print the page header, if any. */
	
	if (breakInfo.header && p->header) {
		PrintHeader(p, pageNum);
		v += GetHH(p);
	}
	if (CheckCancel()) return;
	
	/* Set the font number, size, and style, and get font info. */
	
	TextFont(p->fontNum);
	TextSize(p->fontSize);
	TextFace(normal);
	GetFontInfo(&fontInfo);
	lineSep = fontInfo.ascent + fontInfo.descent + fontInfo.leading;
	
	/* Load the first STR# resource and lock it.  Initialize theLine
		to point to the first line to be printed. */
	
	endOfDoc = false;
	LoadStrings(theList, auxInx, &theStrings, &endStrings, &endOfDoc);
	theLine = (unsigned char*)*theStrings + offset;
	
	/* Initialize other variables. */
	
	endOfPage = endOfDoc;
	
	/* Main loop.  Print the page. */
	
	while (!endOfPage) {
	
		if (CheckCancel()) break;
		lSep = lineSep;
		q = theLine+1;
		qEnd = q + *theLine;
		paragraph = true;
		pict = false;
		style = normal;
		newSize = p->fontSize;
		escJust = false;
		
		/* Crack escape sequences. */
		
		while (q < qEnd && *q < 31) {
			switch (*q) {
				case docStyle:
					style = *(q+2);
					break;
				case docSize:
					newSize = utl_ScaleFontSize(p->fontNum, p->fontSize,
						*(q+2)<<8 | *(q+3), laser);
					lSep = GetLSep(p->fontNum, newSize);
					break;
				case docJust:
					escJust = true;
					just = *(q+2);
					break;
				case docOnly:
					paragraph = *(q+2) & docPrint;
					break;
				case docPict:
					pict = true;
					picID = *(q+2)<<8 | *(q+3);
					paragraph = false;
					break;
				case docPage:
				case docKeep:
				case docEndKeep:
					paragraph = false;
					break;
				case docITcon:
					TextFont(p->fontNum);
					TextSize(p->fontSize);
					TextFace(normal);
					tconHandle = GetResource('TCON', p->tabConID);
					if (!*tconHandle) LoadResource(tconHandle);
					tconFlags = utl_HGetState(tconHandle);
					MoveHHi(tconHandle);
					HLock(tconHandle);
					for (tconPass = TconCompute; tconPass < 2; tconPass++) {
						tconLineNum = *(q+2);
						tconCount = **(short**)tconHandle;
						tconPtr = (unsigned char*)*tconHandle+2;
						if (!tconPass) tconMaxT = tconMaxN = 0;
						while (tconCount--) {
							if (CheckCancel()) return;
							if (*(tconPtr+2) & docPrint) {
								if (!tconPass || !tconLineNum) {
									tconLine = *(short*)tconPtr;
									tconData = (unsigned char*)*(**theList).cells + 
										(**theList).cellArray[tconLine];
									tconAuxInx = *tconData;
									tconOffset = *(tconData+1)<<8 | *(tconData+2);
									tconBPtr = (Break1Info*)*Breaks1;
									tconBEnd = (Break1Info*)((char*)tconBPtr + 
										GetHandleSize((Handle)Breaks1));
									tconBPtr++;
									tconPageNum = 0;
									while (tconBPtr < tconBEnd) {
										if (tconAuxInx < tconBPtr->auxInxB) break;
										if (tconAuxInx == tconBPtr->auxInxB &&
											tconOffset < tconBPtr->offsetB) break;
										tconBPtr++;
										tconPageNum++;
									}
									tconStart = tconPtr+4;
									while (*tconStart == ' ') tconStart++;
									tconNBl = tconStart - tconPtr - 4;
									tconTWidth = ((tconNBl*p->fontSize)>>1) +
										TextWidth(tconStart, 0, *(tconPtr+3)-tconNBl);
									NumToString(tconPageNum, tconPNum);
									tconNWidth = StringWidth(tconPNum);
									if (!tconPass) {
										if (tconTWidth > tconMaxT) tconMaxT = tconTWidth;
										if (tconNWidth > tconMaxN) tconMaxN = tconNWidth;
									} else {
										v += lineSep;
										MoveTo(tconLeft + ((tconNBl*p->fontSize)>>1), v);
										DrawText(tconStart, 0, *(tconPtr+3)-tconNBl);
										MoveTo(tconRight-tconNWidth, v);
										DrawString(tconPNum);
										tconPerSep = (p->fontSize - 4) >> 1;
										if (tconPerSep < 3) tconPerSep = 3;
										tconHPeriod = tconRight-tconMaxN-(tconPerSep<<1);
										while (tconHPeriod > tconLeft+tconTWidth+tconPerSep) {
											MoveTo(tconHPeriod, v);
											DrawChar('.');
											tconHPeriod -= tconPerSep;
										}
										break;
									}
								}
								tconLineNum--;
							}
							tconPtr += *(tconPtr+3) + 4;
							if ((long)tconPtr & 1) tconPtr++;
						}
						if (!tconPass) {
							if (tconMaxT+tconMaxN+72 > PrintBoxWidth) {
								tconLeft = PrintBox.left;
								tconRight = PrintBox.right;
							} else {
								tconLeft = PrintBox.left + 
									((PrintBoxWidth - (tconMaxT+tconMaxN+72))>>1);
								tconRight = tconLeft + tconMaxT + tconMaxN + 72;
							}
							TconCompute = 1;
						}
					}
					utl_HSetState(tconHandle, tconFlags);					
					paragraph = false;
					break;
			}
			if (!paragraph) break;
			q += *(q+1);
		}
		
		/* Rewrap paragraph. */
		
		if (paragraph) {
			if (!escJust) just = docLeft;
			Wrap(true, v, p->fontNum, newSize, style, just, theLine,
				q, &allBlank, &nLines, &theLine);
			v += nLines * lSep;
		}
		
		if (pict) {
		
			/* Draw picture. */
		
			nBands = 0;
			while (true) {
				nBands++;
				theLine += *theLine + 1;
				q = theLine + 1;
				qEnd = q + *theLine;
				pict = false;
				while (q < qEnd && *q < 31) {
					if (*q == docPict) {
						pict = true;
						break;
					}
					q += *(q+1);
				}
				if (!pict) break;
			}
			picHandle = GetPicture(picID);
			picRect = (**picHandle).picFrame;
			if (!escJust) just = docCenter;
			switch (just) {
				case docLeft:
					picLeft = PrintBox.left;
					break;
				case docCenter:
					picLeft = PrintBox.left + ((PrintBoxWidth -
						(picRect.right-picRect.left))>>1);
					break;
				case docRight:
					picLeft = PrintBox.right - (picRect.right-picRect.left);
					break;
			}
			OffsetRect(&picRect, picLeft-picRect.left, v-picRect.top);
			HLock((Handle)picHandle);
			DrawPicture(picHandle, &picRect);
			HUnlock((Handle)picHandle);
			v += nBands * (**theList).cellSize.v;
					
		} else {
		
			/* If not a picture, advance to next line. */
		
			theLine += *theLine + 1;
			
		}
		
		/* Advance to next STR# resource if necessary. */
		
		if (theLine >= endStrings) {
			HUnlock(theStrings);
			auxInx++;
			LoadStrings(theList, auxInx, &theStrings, &endStrings, 
				&endOfDoc);
			if (endOfDoc) break;
			theLine = (unsigned char*)*theStrings + 2;
		}
		
		/* Check for end of page. */
		
		endOfPage = auxInx >= endAuxInx && 
			(theLine - (unsigned char*)*theStrings) >= endOffset;
		
	}
	
	if (theStrings) HUnlock(theStrings);
}

/*______________________________________________________________________

	CalcPrintBox - Calculate Printing Rectangle
	
	Entry:	hPrint = handle to print record.
				leftMargin = left margin in 1/100 inch.
				rightMargin = right margin in 1/100 inch.
				topMargin = top margin in 1/100 inch.
				botMargin = bot margin in 1/100 inch.
				
	Exit:		global variable PrintBox = printing rectangle.
_____________________________________________________________________*/

static void CalcPrintBox (THPrint hPrint, short leftMargin, 
	short rightMargin, short topMargin, short botMargin)

{
	Rect				rPaper;				/* paper rect */
	Rect				rPage;				/* page rect */
	short				iVRes;				/* vert printer resolution */
	short				iHRes;				/* horiz printer resolution */
	
	rPaper = (**hPrint).rPaper;
	rPage = (**hPrint).prInfo.rPage;
	iVRes = (**hPrint).prInfo.iVRes;
	iHRes = (**hPrint).prInfo.iHRes;
	PrintBox.left = rPaper.left + leftMargin*iHRes/100;
	if (PrintBox.left < 0) PrintBox.left = 0;
	PrintBox.right = rPaper.right - rightMargin*iHRes/100;
	if (PrintBox.right > rPage.right) PrintBox.right = rPage.right;
	PrintBox.top = rPaper.top + topMargin*iVRes/100;
	if (PrintBox.top < 0) PrintBox.top = 0;
	PrintBox.bottom = rPaper.bottom - botMargin*iVRes/100;
	if (PrintBox.bottom > rPage.bottom) PrintBox.bottom = rPage.bottom;
	PrintBoxHeight = PrintBox.bottom - PrintBox.top;
	PrintBoxWidth = PrintBox.right - PrintBox.left;
}

/*______________________________________________________________________

	MyJobDlgInit - Initialize custom printing dialog.
	
	Entry:	hPrint = handle to print record.
	
	Exit:		function result = pointer to print dialog stream object.
_____________________________________________________________________*/

static pascal TPPrDlg MyJobDlgInit (THPrint hPrint)

{
#pragma unused (hPrint)

	DialogPtr		theDialog;		/* pointer to dialog */
	Rect				dlogRect;		/* dialog rectangle */
	
	theDialog = (DialogPtr)PrtJobDialog;
	
	/* Center the dialog. */
	
	dlogRect = theDialog->portRect;
	utl_CenterDlogRect(&dlogRect, PBlock->menuPick);
	MoveWindow(theDialog, dlogRect.left, dlogRect.top, false);
	
	return PrtJobDialog;
}

/*______________________________________________________________________

	rpp_Print - Print a Report.
	
	Entry:	repHandle = handle to report record.
				printOne = true to bypass print job dialog.
				p = pointer to parameter block, with fields set as follows:
	
				hPrint = handle to print record.
				fontNum = font number.
				fontSize = font size.
				topMargin = top page margin, in 1/100 inch.
				botMargin = bottom page margin, in 1/100 inch.
				leftMargin = left page margin, in 1/100 inch.
				rightMargin = right page margin, in 1/100 inch.
				reverseOrder = true to print pages in reverse order.
				header = true to print page headers.
				title = ptr to header title.  If header=true it this
					string is printed at the top of each page left-justified.
				titleSep = vertical separation between page header and 
					first line of text, in 1/100 inch.
				titleFont = font number for headers.
				titleStyle = font style for headers.
				titleSize = font size scaling factor for headers.
				titleTmpl = ptr to template for header date, time, and page number.
					e.g., "^0  ^1  page ^2".  The place-holders ^0, ^1, and ^2
					are replaced by the current date, time, and page number.
					This string is drawn right-justified at the top of each page,
					if header=true.
				docName = ptr to document name for printing in progress
					dialog.
				dlogID = resource id of printing dialog.  The place-holder
					^0 is replaced by the document name.
				tabConID = TCON resource id, for type 1 reports only.
				emptyPageRangeID = no pages in page range alert.
				truncateRightID = resource id of page box too narrow alert.
				truncateBottomID = resource id of page box too short alert.
				boxTooSmallID = resource id of page box too small alert.
				menuPick = true if Print command was via menu pick, 
					false if command key used.
				updateAll = pointer to function to handle all pending update
					events, or nil if none. The function is called after the
					job dialog is dismissed, and before the printing in progress
					dialog is presented.
					
	Exit:		function result = error code.
	
	The caller must allocate and initialize a print record, and call 
	PrStlDialog to present the page setup dialog and initialize the print
	record.  rep_Print calls PrJobDialog to present the job dialog.
	The printing manager should be closed before calling rep_Print.
_____________________________________________________________________*/

OSErr rpp_Print (Handle repHandle, Boolean printOne, rpp_PrtBlock *p)

{
	ListHandle		theList;				/* handle to list record */
	OSErr				rCode;				/* print manager error code */
	Boolean			printOK;				/* true if user said OK in job dialog */
	GrafPtr			savedPort;			/* saved grafport */
	short				savedResFile;		/* saved cur res file */
	short				printingResFile;	/* printing res file */
	TPPrPort			port;					/* printing port */
	TPrStatus		prStatus;			/* printer status record */
	short				pageNum;				/* current page number */
	unsigned long	secs;					/* current time */
	CursHandle		watch;				/* handle to watch cursor */
	DialogPtr		dlog;					/* pointer to printing dialog */
	short				nCopies;				/* number of copies */
	Boolean			laser;				/* true if laserwriter */
	short				first;				/* first page number */
	short				last;					/* last page number */
	Str255			firstStr;			/* first page number */
	Str255			lastStr;				/* last page number */
	Boolean			canceled;			/* true if canceled by Cmd-Period */
	Rect				dlogRect;			/* printing in progress dialog rectangle */
	
	PBlock = p;
	theList = (ListHandle)repHandle;
	GetPort(&savedPort);
	savedResFile = CurResFile();
	TconCompute = 0;
	
	/* Enable fractional character widths for better looking 
		LaserWriter output. */
	
	SetFractEnable(true);
	
	/* Get current date and time for headers. */
	
	if (p->header) {
		GetDateTime(&secs);
		IUDateString(secs, shortDate, NowDate);
		IUTimeString(secs, true, NowTime);
	}
	
	/* Compute header font size. */
	
	laser = utl_IsLaser(p->hPrint);
	TitleSize = utl_ScaleFontSize(p->fontNum, p->fontSize, p->titleSize, laser); 
	
	CalcPrintBox(p->hPrint, p->leftMargin, p->rightMargin, p->topMargin,
		p->botMargin);
	
	if (!(rCode = PrError())) {
	
		/* Put up the standard job dialog */

		if (!printOne) {
			PrtJobDialog = PrJobInit(p->hPrint);
			printOK = PrDlgMain(p->hPrint, MyJobDlgInit);
			rCode = PrError();
			if (PBlock->updateAll) (*PBlock->updateAll)();
		} else {
			printOK = true;
			(**p->hPrint).prJob.iFstPage = 1;
			(**p->hPrint).prJob.iLstPage = 999;
		}
	
		/* Print the report. */
		
		if (!rCode && printOK) {
			watch = GetCursor(watchCursor);
			SetCursor(*watch);
			printingResFile = CurResFile();
			UseResFile(savedResFile);
			UseResFile(printingResFile);
			dlog = GetNewDialog(p->dlogID, nil, (WindowPtr)-1);
			dlogRect = dlog->portRect;
			utl_CenterDlogRect(&dlogRect, p->menuPick);
			MoveWindow(dlog, dlogRect.left, dlogRect.top, false);
			SetWTitle((WindowPtr)dlog, p->docName);
			ParamText(p->docName, nil, nil, nil);
			ShowWindow((WindowPtr)dlog);
			DrawDialog(dlog);
			first = FirstPage = (**p->hPrint).prJob.iFstPage;
			last = LastPage = (**p->hPrint).prJob.iLstPage;
			if (FirstPage <= 1) FirstPage = 1;
			if (LastPage > 999) LastPage = 999;
			(**p->hPrint).prJob.iFstPage = 1;
			(**p->hPrint).prJob.iLstPage = 999;
			TruncateRight = TruncateBottom = false;
			if ((**theList).refCon) {
				canceled = Break1(theList, p);
			} else {
				canceled = Break0(theList, p);
			}
			if (!canceled) {
				if (FirstPage > LastPage || TruncateRight || TruncateBottom) {
					DisposDialog(dlog);
					if (PBlock->updateAll) (*PBlock->updateAll)();
					dlog = nil;
					InitCursor();
					if (FirstPage > LastPage) {
						NumToString(first, firstStr);
						NumToString(last, lastStr);
						ParamText(firstStr, lastStr, nil, nil);
						utl_StopAlert(p->emptyPageRangeID, nil, 0);
					} else if (TruncateRight) {
						utl_StopAlert(p->truncateRightID, nil, 0);
					} else if (TruncateBottom) {
						utl_StopAlert(p->truncateBottomID, nil, 0);
					}
				} else {
					nCopies = ((**p->hPrint).prJob.bJDocLoop == bDraftLoop && 
						!laser) ? (**p->hPrint).prJob.iCopies : 1;
					PrValidate(p->hPrint);		/* set doc title - see TN 149 */
					while (nCopies--) {
						port = PrOpenDoc(p->hPrint, nil, nil);
						pageNum = p->reverseOrder ? LastPage : FirstPage;
						while (true) {
							if (rCode = PrError() || CheckCancel()) break;
							if (p->reverseOrder) {
								if (pageNum < FirstPage) break;
							} else {
								if (pageNum > LastPage) break;
							}
							PrOpenPage(port, nil);
							if (!(rCode = PrError())) {
								SetPort((GrafPtr)port);
								ClipRect(&PrintBox);
								if ((**theList).refCon) {
									Print1(theList, p, pageNum);
								} else {
									Print0(theList, p, pageNum);
								}
							}
							PrClosePage(port);
							if (p->reverseOrder) {
								pageNum--;
							} else {
								pageNum++;
							}
						}
						PrCloseDoc(port);
						if (!(rCode = PrError())) {
							if ((**p->hPrint).prJob.bJDocLoop == bSpoolLoop) {
								PrPicFile(p->hPrint, nil, nil, nil, &prStatus);
								rCode = PrError();
							}
						}
					}
				}
			}
			if ((**theList).refCon) {
				DisposHandle((Handle)Breaks1);
			} else {
				DisposHandle((Handle)Breaks0);
			}
			if (dlog) DisposDialog(dlog);
			InitCursor();
		}
	}
	
	SetPort(savedPort);
	
	/* Disable fractional character widths. */
	
	SetFractEnable(false);
	
	return PrError();
}

/*______________________________________________________________________

	DrawList - Draw List Manager Dialog User Item.
	
	Entry:	theWindow = pointer to page setup dialog.
				itemNo = item number.
_____________________________________________________________________*/

static pascal void DrawList (WindowPtr theWindow, short itemNo)

{
	short			itemType;			/* item type */
	Handle		item;					/* item handle */
	Rect			box;					/* item rectangle */

	GetDItem(theWindow, itemNo, &itemType, &item, &box);
	FrameRect(&box);
	if (itemNo == FirstItem+fontListItem) {
		LUpdate((**FontList).port->visRgn, FontList);
	} else {
		LUpdate((**SizeList).port->visRgn, SizeList);
	}
}

/*______________________________________________________________________

	BuildSizeList - Build Font Size List.
	
	Entry:		theDialog = pointer to dialog.
_____________________________________________________________________*/

static void BuildSizeList (DialogPtr theDialog)

{
	short			itemType;		/* item type */
	Handle		item;				/* item handle */
	Rect			box;				/* item rectangle */
	Cell			cell;				/* List Manager cell */
	short			len;				/* length of font name */
	Str255		str;				/* multi-purpose string */
	Str255		str1;				/* current size from size TE box */
	short			fontNum;			/* font number */
	short			i;					/* loop index */

	LDoDraw(false, SizeList);
	SetPt(&cell, 0, 0);
	LGetSelect(true, &cell, FontList);
	len = 255;
	LGetCell(str+1, &len, cell, FontList);
	*str = len;
	GetFNum(str, &fontNum);
	LDelRow(0, 0, SizeList);
	SetPt(&cell, 0, 0);
	GetDItem(theDialog, FirstItem+sizeTEItem, &itemType, &item, &box);
	GetIText(item, str1);
	for (i = PBlock->minFontSize; i <= PBlock->maxFontSize; i++) {
		if (RealFont(fontNum, i)) {
			LAddRow(1, cell.v, SizeList);
			NumToString(i, str);
			LSetCell(str+1, *str, cell, SizeList);
			if (EqualString(str, str1, false, false)) 
				LSetSelect(true, cell, SizeList);
			cell.v++;
		}
	}
	LDoDraw(true,SizeList);
	if (((WindowPeek)theDialog)->visible) {
		box = (**SizeList).rView;
		EraseRect(&box);
		box.right += 15;
		InvalRect(&box);
	}
}

/*______________________________________________________________________

	MyFltrProc - Filter proc for custom page setup dialog.
	
	Entry:	theDialog = pointer to dialog.
				theEvent = pointer to event record.
				itemHit = pointer to item number.
				
	This filter proc validates key presses when one of our extra textedit
	fields is active.
	
	For the font size field, we permit at most 2 digits.
	
	For the margin fields, we permit at most 6 characters, consisting
	of digits and at most one decimal point.
_____________________________________________________________________*/

static pascal Boolean MyFltrProc (DialogPtr theDialog, EventRecord *theEvent,
	short *itemHit)
	
{
	short				key;				/* ascii code of key pressed */
	short				item;				/* item number of current textedit field */
	TEHandle			textH;			/* handle to TextEdit record */
	short				selStart;		/* start of selection range */
	short				selEnd;			/* end of selection range */
	short				selSize;			/* size of selected part of item */
	short				oldSize;			/* old text length */
	short				newSize;			/* new size of field */
	Boolean			isdig;			/* true if key is a digit */
	char				*p;				/* pointer into text */
	char				*pStart;			/* pointer to beginning of text */
	char				*pEnd;			/* pointer to end of text */
	short				pIndex;			/* index in text of decimal point */

	if (theEvent->what == keyDown || theEvent->what == autoKey) {
		key = theEvent->message & charCodeMask;
		if (key != returnKey && key != enterKey && key != deleteKey &&
			key != tabKey && key != leftArrow && key != rightArrow) {
			item = ((DialogPeek)theDialog)->editField + 1;
			if (item > FirstItem) {
				item -= FirstItem;
				textH = ((DialogPeek)theDialog)->textH;
				selStart = (**textH).selStart;
				selEnd = (**textH).selEnd;
				selSize = selEnd - selStart;
				oldSize = (**textH).teLength;
				newSize = oldSize + 1 - selSize;
				isdig = isdigit(key);
				if (item == sizeTEItem) {
					if (isdig && newSize <= 2) return false;
				} else {
					if (newSize <= 6) {
						if (isdig) return false;
						if (key == '.') {
							pStart = p = *(**textH).hText;
							pEnd = p + oldSize;
							while (p < pEnd && *p != '.') p++;
							if (p == pEnd) return false;
							pIndex = p - pStart;
							if (pIndex >= selStart && pIndex < selEnd) return false;
						}
					}
				}
				SysBeep(10);
				theEvent->what = nullEvent;
				return false;
			}
		}
	}
	if (StdFltrProc) {
		return (*StdFltrProc)(theDialog, theEvent, itemHit);
	} else {
		return false;
	}
}

/*______________________________________________________________________

	MyItemProc - Handle custom page setup item hit.
	
	Entry:	hPrint = handle to print record.
	
	Exit:		function result = pointer to print dialog stream object.
_____________________________________________________________________*/

static pascal void MyItemProc (DialogPtr theDialog, short itemNo)

{
	short				itemType;		/* item type */
	Handle			item;				/* item handle */
	Rect				box;				/* item rectangle */
	Str255			str;				/* multi-purpose string */
	Str255			str1;				/* second string */
	unsigned char	*pstr;			/* pointer to cur pos in string */
	unsigned char	lstr;				/* length of string */
	long				val;				/* value of string */
	short				i;					/* loop index */
	short				whole;			/* whole part of decimal number */
	short				frac;				/* fraction part of decimal number */
	unsigned char	*point;			/* pointer to decimal point in string */
	short				margin;			/* margin value in 1/100 inches */
	Point				where;			/* location of mouse click */
	Cell				cell;				/*	old selected List Manager cell */
	Cell				newCell;			/* new selected List Manager cell */
	short				len;				/* length of selected font name */
	short				fontNum;			/* font number */
	short				fontSize;		/* font size */
	short				topMargin;		/* top margin */
	short				botMargin;		/* bottom margin */
	short				leftMargin;		/* left margin */
	short				rightMargin;	/* right margin */
	
	if (itemNo > FirstItem) {
	
		/* Process a hit on one of our extra dialog items. */
	
		GetDItem(theDialog, itemNo, &itemType, &item, &box);
		switch (itemNo - FirstItem) {
			case reverseItem:
				SetCtlValue((ControlHandle)item, 1 - GetCtlValue((ControlHandle)item));
				break;
			case fontListItem:
				SetPort(theDialog);
				SetPt(&cell, 0, 0);
				LGetSelect(true, &cell, FontList);
				GetMouse(&where);
				LClick(where, 0, FontList);
				SetPt(&newCell, 0, 0);
				if (LGetSelect(true, &newCell, FontList)) {
					if (cell.v != newCell.v) BuildSizeList(theDialog);
				} else {
					LSetSelect(true, cell, FontList);
				}
				break;
			case sizeListItem:
				SetPort(theDialog);
				GetMouse(&where);
				LClick(where, 0, SizeList);
				SetPt(&cell, 0, 0);
				if (LGetSelect(true, &cell, SizeList)) {
					len = 255;
					LGetCell(str+1, &len, cell, SizeList);
					*str = len;
					GetDItem(theDialog, FirstItem+sizeTEItem, &itemType,
						&item, &box);
					SetIText(item, str);
				}
				break;
		}
		
	} else {
	
		/* Process a hit on the standard OK button. */
	
		if (itemNo == ok) {
		
			/* Fetch the font. */
			
			SetPt(&cell, 0, 0);
			LGetSelect(true, &cell, FontList);
			len = 255;
			LGetCell(str+1, &len, cell, FontList);
			*str = len;
			GetFNum(str, &fontNum);
				
			/* Fetch and validate the font size. */
		
			GetDItem(theDialog, FirstItem+sizeTEItem, &itemType, 
				&item, &box);
			GetIText(item, str);
			StringToNum(str, &val);
			if (val < PBlock->minFontSize || val > PBlock->maxFontSize) {
				NumToString(PBlock->minFontSize, str);
				NumToString(PBlock->maxFontSize, str1);
				ParamText(str, str1, nil, nil);
				utl_StopAlert(PBlock->sizeRangeID, nil, 0);
				SelIText(theDialog, FirstItem+sizeTEItem, 0, 32767);
				return;
			}
			fontSize = val; 
		
			/* Fetch and evaluate the four margins. */
			
			for (i = 0; i < 4; i++) {
				GetDItem(theDialog, FirstItem+leftTEItem+i, &itemType, 
					&item, &box);
				GetIText(item, str);
				pstr = str+1;
				lstr = *str;
				while (lstr) {
					if (*pstr == '.') break;
					pstr++;
					lstr--;
				}
				*str1 = pstr-str-1;
				memcpy(str1+1, str+1, *str1);
				StringToNum(str1, &val);
				whole = val;
				frac = 0;
				if (lstr) {
					point = pstr;
					pstr++;
					lstr--;
					*str1 = 2;
					*(str1+1) = '0';
					*(str1+2) = '0';
					if (lstr) {
						*(str1+1) = *(point+1);
						if (lstr > 1) *(str1+2) = *(point+2);
					}
					StringToNum(str1, &val);
					frac = val;
				}
				margin = 100*whole + frac;
				switch (i) {
					case 0: leftMargin = margin; break;
					case 1: rightMargin = margin; break;
					case 2: topMargin = margin; break;
					case 3: botMargin = margin; break;
				}
			}
			
			/* Check for margins too big.  We require at least 5 inches
				of printing area both horizontally and vertically. */
			
			CalcPrintBox(PBlock->hPrint, leftMargin, rightMargin, topMargin, botMargin);
			if (PrintBoxWidth < 5*(**PBlock->hPrint).prInfo.iHRes) {
				utl_StopAlert(PBlock->marginsTooBigID, nil, 0);
				SelIText(theDialog, FirstItem+leftTEItem, 0, 32767);
				return;
			} else if (PrintBoxHeight < 5*(**PBlock->hPrint).prInfo.iVRes) {
				utl_StopAlert(PBlock->marginsTooBigID, nil, 0);
				SelIText(theDialog, FirstItem+topTEItem, 0, 32767);
				return;
			}
		
			/* Set the new values of the print record fields. */
			
			PBlock->fontNum = fontNum;
			PBlock->fontSize = fontSize;
			PBlock->topMargin = topMargin;
			PBlock->botMargin = botMargin;
			PBlock->leftMargin = leftMargin;
			PBlock->rightMargin = rightMargin;
			GetDItem(theDialog, FirstItem+reverseItem, &itemType, &item, &box);
			PBlock->reverseOrder = GetCtlValue((ControlHandle)item);
			
		}
		
		/* Call the standard item handler. */
		
		(*StdItemProc)(theDialog, itemNo);
		
	}
}

/*______________________________________________________________________

	MyStlDlgInit - Initialize custom page setup dialog.
	
	Entry:	hPrint = handle to print record.
	
	Exit:		function result = pointer to print dialog stream object.
_____________________________________________________________________*/

static pascal TPPrDlg MyStlDlgInit (THPrint hPrint)

{
	DialogPtr		theDialog;		/* pointer to dialog */
	short				itemType;		/* item type */
	Handle			item;				/* item handle */
	Rect				box;				/* item rectangle */
	Str255			str;				/* multi-purpose string */
	Str255			str1;				/* second multi-purpose string */
	short				i;					/* loop index */
	short				val;				/* value */
	short				whole;			/* whole part of decimal num */
	short				frac;				/* fraction part of decimal num */
	unsigned char	len;				/* length of string */
	Boolean			laser;			/* true if laserwriter */
	MenuHandle		tempMenu;		/* tempory menu for enumerating fonts */
	short				numFonts;		/* number of fonts */
	Rect				dataBounds;		/* dimensions of font list */
	Point				cSize;			/* font list cell size */
	Cell				cell;				/* list manager cell */
	Rect				dlogRect;		/* dialog rectangle */
	
	theDialog = (DialogPtr)PrtStlDialog;
	laser = utl_IsLaser(hPrint);
	
	/* Append our extra dialog items. */
	
	FirstItem = utl_AppendDITL(theDialog, PBlock->ditlID) - 1;
	
	/* Center the dialog. */
	
	dlogRect = theDialog->portRect;
	utl_CenterDlogRect(&dlogRect, PBlock->menuPick);
	MoveWindow(theDialog, dlogRect.left, dlogRect.top, false);
	
	/* Initialize line separator user item. */
	
	GetDItem(theDialog, FirstItem+sepLineItem, &itemType, &item, &box);
	SetDItem(theDialog, FirstItem+sepLineItem, itemType, 
		(Handle)utl_FrameItem, &box);
		
	/* Initialize font list user item.  See TN 191. */
		
	GetDItem(theDialog, FirstItem+fontListItem, &itemType, &item, &box);
	SetDItem(theDialog, FirstItem+fontListItem, itemType, 
		(Handle)DrawList, &box);
	GetFontName(PBlock->fontNum, str);
	if (!*str) GetFontName(applFont, str);
	tempMenu = NewMenu(9999, "\px");
	AddResMenu(tempMenu, 'FONT');
	numFonts = CountMItems(tempMenu);
	InsetRect(&box, 1, 1);
	box.right -= 15;
	SetRect(&dataBounds, 0, 0, 1, numFonts);
	SetPt(&cSize, 0, 0);
	FontList = LNew(&box, &dataBounds, cSize, 0, theDialog, false, false,
		false, true);
	LDoDraw(false, FontList);
	(**FontList).selFlags = lOnlyOne;
	SetPt(&cell, 0, 0);
	for (i = 1; i <= numFonts; i++) {
		GetItem(tempMenu, i, str1);
		LSetCell(str1+1, *str1, cell, FontList);
		if (EqualString(str, str1, false, false)) {
			LSetSelect(true, cell, FontList);
			LAutoScroll(FontList);
		}
		cell.v++;
	}
	DisposeMenu(tempMenu);
	LDoDraw(true, FontList);
		
	/* Initialize size text edit item. */
	
	GetDItem(theDialog, FirstItem+sizeTEItem, &itemType, &item, &box);
	NumToString(PBlock->fontSize, str);
	SetIText(item, str);
	
	/* Initialize font size list user item. */
	
	GetDItem(theDialog, FirstItem+sizeListItem, &itemType, &item, &box);
	SetDItem(theDialog, FirstItem+sizeListItem, itemType,
		(Handle)DrawList, &box);
	InsetRect(&box, 1, 1);
	box.right -= 15;
	SetRect(&dataBounds, 0, 0, 1, 0);
	SetPt(&cSize, 0, 0);
	SizeList = LNew(&box, &dataBounds, cSize, 0, theDialog, false, false,
		false, true);
	(**SizeList).selFlags = lOnlyOne;
	BuildSizeList(theDialog);
		
	/* Initialize reverse order checkbox. */
	
	GetDItem(theDialog, FirstItem+reverseItem, &itemType, &item, &box);
	SetCtlValue((ControlHandle)item, PBlock->reverseOrder);
	
	/* Initialize margin text edit items. */
	
	for (i = 0; i < 4; i++) {
		switch (i) {
			case 0: val = PBlock->leftMargin; break;
			case 1: val = PBlock->rightMargin; break;
			case 2: val = PBlock->topMargin; break;
			case 3: val = PBlock->botMargin; break;
		}
		whole = val/100;
		frac = val%100;
		NumToString(whole, str);
		NumToString(frac, str1);
		if (*str1 == 1) {
			*str1 = 2;
			*(str1+2) = *(str1+1);
			*(str1+1) = '0';
		}
		len = *str;
		*(str+len+1) = '.';
		memcpy(str+len+2, str1+1, *str1);
		*str += *str1 + 1;
		GetDItem(theDialog, FirstItem+leftTEItem+i, &itemType, &item, &box);
		SetIText(item, str);
	}
	
	/* Save standard item handler and filterproc addresses, and patch in the addresses
		of our item handler and filterproc. */ 
		
	StdItemProc = PrtStlDialog->pItemProc;
	StdFltrProc = PrtStlDialog->pFltrProc;
	PrtStlDialog->pItemProc = MyItemProc;
	PrtStlDialog->pFltrProc = MyFltrProc;
	
	/* Return. */
		
	return PrtStlDialog;
}

/*______________________________________________________________________

	rpp_StlDlog - Present Customized Page Setup Dialog.
	
	Entry:	p = pointer to parameter block, with fields set as follows:
	
				hPrint = handle to print record.
				fontNum = font number.
				fontSize = font size.
				topMargin = top margin, in 1/100 inch.
				botMargin = bottom margin, in 1/100 inch.
				leftMargin = left margin, in 1/100 inch.
				rightMargin = right margin, in 1/100 inch.
				reverseOrder = true if pages to be printed in reverse order.
				ditlID = resource id of DITL to be appended to standard
					page setup dialog.
				sizeRangeID = resource id of font size range error alert.
				marginsTooBigID = page margins too big alert.
				minFontSize = min legal font size.
				maxFontSize = max legal font size.
				menuPick = true if Page Setup command was via menu pick, 
					false if command key used.
					
	Exit:		canceled = true if Cancel button clicked, 
					false if OK button clicked.
				function result = error code.
					
				Following fields are updated in parameter block if the OK button
				was clicked:
					
				fontNum = font number.
				fontSize = font size.
				topMargin = top margin, in 1/100 inch.
				botMargin = bottom margin, in 1/100 inch.
				leftMargin = left margin, in 1/100 inch.
				rightMargin = right margin, in 1/100 inch.
				reverseOrder = true if pages to be printed in reverse order.
_____________________________________________________________________*/

OSErr rpp_StlDlog (rpp_PrtBlock *p, Boolean *canceled)

{
	Boolean 		okClicked;				/* true if OK clicked */
	OSErr			rCode;					/* result code */

	PBlock = p;
	FontList = nil;
	SizeList = nil;
	PrtStlDialog = PrStlInit(p->hPrint);
	if (rCode = PrError()) return rCode;
	okClicked = PrDlgMain(p->hPrint, MyStlDlgInit);
	if (FontList) {
		(**FontList).vScroll = nil;
		LDispose(FontList);
	}
	if (SizeList) {
		(**SizeList).vScroll = nil;
		LDispose(SizeList);
	}
	*canceled = !okClicked;
	return PrError();
}
