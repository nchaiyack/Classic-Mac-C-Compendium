/*______________________________________________________________________

	rep.c - Report Module.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This reusable module implements scrolling reports.
	
	The caller supplies a rectangle in which the report should be
	displayed.  When the report is created the caller is returned a handle
	to a report record.  This handle must be passed to all subsequent
	routines.
	
	There are two types of reports.
	
	In type 0 reports the report is intially empty, and the routines
	rep_Append and rep_Fill are used to append one or more lines at a time.
	The maximum report size is 32k characters of text.
	
	In type 1 reports the text is fixed, in consecutive purgable STR# 
	resources specified when the report is initialized.  rep_Append and
	rep_Fill do nothing.  The maximum report size is 8k lines of text,  
	with no limit on characters.
	
	Type 0 reports are appropriate for reports generated dynamically by the
	program.  For example, the output of a volume scanner.  Type 1 reports
	are more appropriate for help windows and other kinds of fixed text.
	
	Type 1 reports use memory more efficiently than type 0 reports, and they
	can handle much larger reports (8k lines vs. 32k chars).  With type 0
	reports the entire report is stored in one big memory buffer, while with
	type 1 reports the purgable STR# resources act as a read-only virtual 
	memory.  Both types use unpurgeable memory, but type 0 requires
	1 byte per character, while type 1 only requires 4 bytes per line.
	
	The module is currently implemented using the list manager, and the 
	report handle is just a list handle.  But you shouldn't rely on 
	this - the module could be rewritten to use TextEdit or some other kind
	of code, without affecting its official interface.
	
	Multiple reports in different rectangles can be managed simultaneously.
	
	All of the code is placed in its own segment named "rep", except for the
	routine rep_Init, which is placed in segment repi.
_____________________________________________________________________*/


#pragma load "precompile"
#include "rep.h"
#include "utl.h"
#include "doc.h"

#pragma segment rep

/*______________________________________________________________________

	rep_Init - Initialize a Report.
	
	Entry:	repRect = ptr to rectangle enclosing report and its scroll bar.
				theWindow = ptr to window containing report.
				repType = report type (0 or 1).
				firstID = id of first STR# resource (type 1 only).
				lDefRezID = id of LDEF (type 1 only).
				cellRezID = id of CELL resource (type 1 only).
	
	Exit:		repHandle = handle to report record.
	
	For best results, the rectangle height should be 2 more than an even
	multiple of the window font's ascent plus descent plus leading
	(12 for Geneva 9). This will make an even number of rows appear in 
	the rectangle, which makes scrolling look nice.
	
	listDefID must be the id of the special LDEF designed just for use with
	type 1 reports.  You default value is 128, but you can change this if
	it conflicts with your own LDEFs.
_____________________________________________________________________*/


#pragma segment repi

void rep_Init (Rect *repRect, WindowPtr theWindow, 
	short repType, short firstID, 
	short lDefRezID, short cellRezID, Handle *repHandle)

{
	Rect				rView;			/* list manager rectangle */
	Rect				dataBounds;		/* dimensions of list */
	Point				cSize;			/* cell size */
	ListHandle		theList;			/* handle to list manager record */

	/* Variables used only for type 1 records. */	

	Handle			theStrings;			/* handle to STR# resource */
	Rect				frameRect;			/* rectangle enclosing the list */
	auxInfo			**aux;				/* handle to auxiliary info */
	short				i;						/* loop index */
	short				count;				/* number of lines in STR# rsrc */
	short				*pCellArray;		/* pointer into cell array */
	short				cellIndex;			/* index in cell data */
	
	/* Set the list manager rectangle.  Inset the rectangle passed by the
		caller by one pixel to accomodate a framing rectangle, and subtract
		15 from the right side to accomodate the scroll bar. */
	
	rView = *repRect;
	InsetRect(&rView, 1, 1);
	rView.right -= 15;
	
	/* Create an empty list. Save the report type in the refCon field of
		the list record. */
	
	SetPt(&cSize, 0, 0);
	SetRect(&dataBounds, 0, 0, 1, 0);
	theList = (ListHandle) LNew(&rView, &dataBounds, cSize, 
		repType ? lDefRezID : 0, theWindow, 
		false, false, false, true);
	*repHandle = (Handle) theList;
	(**theList).selFlags = lOnlyOne;
	(**theList).refCon = repType;
	
	/* If type 0 return handle to empty list. */
	
	if (!repType) return;
	
	/* If type 1 report initialize the fixed cell data. */
	
	/* Dispose of the empty cell block created by the LNew call.  Plug in a
		handle to the CELL resource precomputed by the cvrt tool.  Make the
		resource unpurgable. */
		
	DisposHandle((**theList).cells);
	(**theList).cells = GetResource('CELL', cellRezID);
	HNoPurge((**theList).cells);
	
	/* Add the proper number of rows to the list. */
	
	count = GetHandleSize((**theList).cells) / 3;
	(void) LAddRow(count, 0, theList);
	
	/* Initialize the cell array. */
	
	pCellArray = (**theList).cellArray;
	cellIndex = 0;
	while (count--) {
		*pCellArray++ = cellIndex;
		cellIndex += 3;
	};
	
	/* Create and initialize the auxiliary information. */
	
	aux = (auxInfo**)NewHandle(500);
	(**theList).userHandle = (Handle)aux;
	(**aux).cachedPictID = 0;
	SetResLoad(false);
	for (i=0; true; i++) {
		theStrings = GetResource('STR#', firstID + i);
		if (!theStrings) break;
		(**aux).auxArray[i] = theStrings;
	};
	SetResLoad(true);
	(**aux).auxArray[i] = 0;
	SetHandleSize((Handle)aux, 4*i + sizeof(auxInfo));
	
	/* Invalidate the list rectangle. */
	
	frameRect = (**theList).rView;
	frameRect.right += 15;
	InvalRect(&frameRect);
}

#pragma segment rep

/*______________________________________________________________________

	rep_Append - Append One Line to a Report.
	
	Entry:	repHandle = handle to report record.
				repLine = ptr to pascal string to append.
				drawIt = true to draw the new string, false to not draw it.
				scroll = true to scroll the report to make the new line
					visible.
	
	The new line is appended to the end of the report.
	
	If you want the report lines to become visible immediately, specify
	drawIt=true and scroll=true.  The result is similar to the behavior of
	a dumb glass teletype, with the exception that the user can scroll 
	back to previous screens.
	
	If you want to build the report invisibly and then display it,
	specify drawIt=false and scroll=false.  Then, when the report is 
	complete, simply invalidate the rectangle.  This will generate an 
	update event, and rep_Update will display the report.
	
	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
	
	The routine simply returns for type 1 reports.
_____________________________________________________________________*/


void rep_Append (Handle repHandle, Str255 repLine, 
	Boolean drawIt, Boolean scroll)

{
	ListHandle	theList;			/* handle to list record */
	Cell			theCell;			/* new cell to be added */

	theList = (ListHandle) repHandle;
	if ((**theList).refCon) return;
	SetPt(&theCell, 0, (**theList).dataBounds.bottom);
	LDoDraw(drawIt, theList);
	(void) LAddRow(1, theCell.v, theList);
	if (scroll) LScroll(0, theCell.v, theList);
	LSetCell(repLine+1, *repLine, theCell, theList);
}

/*______________________________________________________________________

	rep_Fill - Append Lines to a Report from an STR# resource.
	
	Entry:	repHandle = handle to report record.
				id = resource id of STR# resource.
				scroll = true if report should be scrolled to show last line.

	The new lines are appended to the end of the report.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
	
	The routine simply returns for type 1 reports.
_____________________________________________________________________*/

void rep_Fill (Handle repHandle, short id, 
	Boolean scroll)

{
	Handle		theStrings;		/* handle to STR# resource */
	ListHandle	theList;			/* handle to list record */
	unsigned char	*p;				/* pointer to current loc in STR# resource */
	short			count;			/* number of lines left to append */
	Point			theCell;			/* new cell to be added */
	Rect			frameRect;		/* rectangle enclosing the list */

	theList = (ListHandle) repHandle;
	if ((**theList).refCon) return;
	SetPt(&theCell, 0, (**theList).dataBounds.bottom);
	theStrings = GetResource('STR#', id);
	MoveHHi(theStrings);
	HLock(theStrings);
	p = *theStrings;
	count = *((short*) p);
	p += 2;
	LDoDraw(false, theList);
	(void) LAddRow(count, theCell.v, theList);
	while (count-- > 0) {
		LSetCell(p+1, *p, theCell, theList);
		p += *p + 1;
		theCell.v++;
	};
	HUnlock(theStrings);
	if (scroll) LScroll(0, theCell.v, theList);
	LDoDraw(true, theList);
	frameRect = (**theList).rView;
	frameRect.right += 15;
	InvalRect(&frameRect);
};

/*______________________________________________________________________

	rep_Scroll - Process a Mouse Hit in a Scroll Bar.
	
	Entry:	repHandle = handle to report record.
				where	= pointer to mouse click location, in local coords.
	
	Call this routine to respond to a mouse hit in the scroll bar.
	For convenience, you can call it for a mouse hit anywhere in the entire 
	report rectangle.  The routine checks to make sure the hit is
	really in the actual scroll bar, and returns if it isn't.
	
	Note that mouse down events in the report proper are ignored by this
	routine.  This is the routine you should call if you do not wish to
	allow users to select report lines.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
_____________________________________________________________________*/


void rep_Scroll (Handle repHandle, Point where)

{
	ListHandle	theList;			/* handle to list record */

	theList = (ListHandle) repHandle;
	if (!PtInRect(where, &(**((**theList).vScroll)).contrlRect)) return;
	LDoDraw(true, theList);
	(void) LClick(where, 0, theList);
}

/*______________________________________________________________________

	rep_Click - Process a Mouse Down in a Report.
	
	Entry:	repHandle = handle to report record.
				where	= pointer to mouse click location, in local coords.
				
	Exit:		function result = selected line number, or -1 if none.
	
	Call this routine to respond to a mouse hit in the report rectangle.
	If the mouse hit is in the scroll bar, the report is scrolled.
	Otherwise, a report line may be selected, in which case the routine
	immediately deselects the line and returns its line number.
	
	Note that mouse down events in the report proper are not ignored by
	this routine.  This is the routine you should call if you wish to
	allow users to select report lines.
	
	This routine can be used only with type 0 reports.  It simply returns
	-1 for type 1 reports.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
_____________________________________________________________________*/


short rep_Click (Handle repHandle, Point where)

{
	ListHandle	theList;			/* handle to list record */
	Cell			theCell;			/* selected cell */

	theList = (ListHandle) repHandle;
	if ((**theList).refCon) return -1;
	LDoDraw(true, theList);
	(void) LClick(where, 0, theList);
	theCell.h = theCell.v = 0;
	if (LGetSelect(true, &theCell, theList)) {
		LSetSelect(false, theCell, theList);
		return theCell.v;
	} else {
		return -1;
	};
}

/*______________________________________________________________________

	rep_Update - Process an Update Event.
	
	Entry:	repHandle = handle to report record.
	
	This routine should be called whenever an update event occurs for
	the window containing the report.  The report, scroll bar, and a 
	rectangle framing the whole thing are drawn.  The report is drawn 
	using the current window font.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine, and call BeginUpdate.
_____________________________________________________________________*/


void rep_Update (Handle repHandle)

{
	ListHandle	theList;			/* handle to list record */
	Rect			frameRect;		/* rectangle to be framed */

	theList = (ListHandle) repHandle;
	
	/* Draw the framing rectangle. */
	
	frameRect = (**theList).rView;
	frameRect.right += 15;
	InsetRect(&frameRect, -1, -1);
	FrameRect(&frameRect);
	
	/* Call LUpdate to redraw the list and the scroll bar. */
	
	LDoDraw(true, theList);
	LUpdate((**theList).port->visRgn, theList);
	
	/* The following FrameRect is a cludge for when an update event occurs
		when the scroll bar is inactive (e.g., if a screen saver deactivates).
		In this case the LUpdate call doesn't redraw the enclosing rectangle of 
		the scroll bar, because it thinks it's invisible.  So we redraw it 
		here. */
	
	FrameRect(&(**((**theList).vScroll)).contrlRect);
}

/*______________________________________________________________________

	rep_Activate - Process an Activate or Deactivate Event.
	
	Entry:	repHandle = handle to report record.
				activate = true to activate, false to deactivate.
				
	This routine should be called whenever an activate or deactivate
	event occurs.  All it does is hilite or unhilite the scroll
	bar.  You should also call it to temporarily disable the scroll
	bar, e.g., during generation of a report, when you aren't checking
	for mouse hits anyway.  This avoids the ugly "bouncing thumb"
	effect.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
_____________________________________________________________________*/


void rep_Activate (Handle repHandle, Boolean active)

{
	ListHandle	theList;			/* handle to list record */
	Rect			scrollRect;		/* scroll bar rectangle */

	theList = (ListHandle) repHandle;
	LDoDraw(true, theList);
	LActivate(active, theList);
	
	/* The following code fixes what look like bugs in the LActivate
		routine.  
		
		On an activate event LActivate doesn't properly hilite
		the scroll bar.  We invalidate the scroll bar in this case, to
		force an update event.  This causes rep_Update to be called, which 
		in turn calls LUpdate, which redraws it properly hilited.
		
		On a deactivate event LActivate erases the entire scroll bar,
		including its enclosing rectangle.  We want just the interior
		erased, so we have to do a FrameRect.  Also, for some reason
		LActivate invalidates the scroll bar in this case, which generates
		an update event.  This produces an ugly blip when the scroll bar is 
		erased and redrawn by LUpdate in the subsequent call to rep_Update.  
		So we validate the scroll bar rectangle to prevent the update event.
		
		Apparently LActivate is invalidating the scroll bar on deactivate
		events, but not on activate events.  It seems to me that it should
		be exactly the other way around.  But what do I know.
		
		I don't pretend to really understand what's going on here.  But
		my cludges here and in rep_Update seem to fix the problems. */
		
	scrollRect = (**((**theList).vScroll)).contrlRect;
	if (active){
		InvalRect(&scrollRect);
	} else {
		FrameRect(&scrollRect);
		ValidRect(&scrollRect);
	};
}

/*______________________________________________________________________

	Write0 - Write the Report File for a Type 0 Report.
	
	Entry:	theList = handle to list record.
				refNum = file reference number.
				
	Exit:		function result = result code.
_____________________________________________________________________*/


static OSErr Write0 (ListHandle theList, short refNum)

{
	OSErr				rCode;			/* result code */
	Handle			theCells;		/* handle to the list cell data */
	unsigned char	*buf;				/* ptr to beginning of output buffer */
	unsigned char	*pTo;				/* current position in output buffer */
	unsigned char	*pFrom;			/* current position in input buffer */
	short				*cellArray;		/* current position in cell array */
	short				nLines;			/* number of lines left to write */
	short				nBytes;			/* number of bytes to copy */
	long				count;			/* number of bytes to write to file */
		
	/* Get a buffer big enough to hold the entire file. */
	
	theCells = (**theList).cells;
	nLines = (**theList).dataBounds.bottom;
	pTo = buf = NewPtr(GetHandleSize(theCells) + nLines);
		
	/* Copy cell data from the list to the buffer, adding a return
		character at the end of each line. */
		
	MoveHHi(theCells);
	HLock(theCells);
	pFrom = *theCells;
	cellArray = (**theList).cellArray;
	while (nLines--) {
		nBytes = *(cellArray+1) - *cellArray;
		memcpy(pTo, pFrom, nBytes);
		pFrom += nBytes;
		pTo += nBytes;
		*pTo++ = '\n';
		cellArray++;
	};
	HUnlock(theCells);
	
	/* Write the buffer to the file and dispose of the buffer. */
	
	count = pTo-buf;
	rCode = FSWrite(refNum, &count, buf);
	DisposPtr(buf);
	return rCode;
}

/*______________________________________________________________________

	Write1 - Write the Report File for a Type 1 Report.
	
	Entry:	theList = handle to list record.
				refNum = file reference number.
				
	Exit:		function result = result code.
_____________________________________________________________________*/


static OSErr Write1 (ListHandle theList, short refNum)

{
	OSErr				rCode;			/* result code */
	auxInfo			**aux;			/* handle to auxiliary info */
	short				i;					/* index in STR# handle array */
	Handle			theStrings;		/* handle to STR# resource */	
	unsigned char	*buf;				/* ptr to beginning of output buffer */
	unsigned char	*pTo;				/* current position in output buffer */
	unsigned char	*p;				/* current position in input buffer */
	unsigned char	*q;				/* current position in input line */
	unsigned char	*qEnd;			/* pointer to end of input line */
	short				nLines;			/* number of lines left to write */
	short				nBytes;			/* number of bytes to copy */
	long				count;			/* number of bytes to write to file */
	Boolean			skip;				/* true if line should be skipped */
	
	/* Get handle to auxiliary info. */
	
	aux = (auxInfo**)(**theList).userHandle;
	
	/* The auxiliary array contains handles to the STR# resources.  The
		main loop walks this array and processes one resource at a time. */
	
	i = 0;
	while (true) {
		if (!(theStrings = (**aux).auxArray[i])) break;
		if (!*theStrings) LoadResource(theStrings);
		MoveHHi(theStrings);
		HLock(theStrings);
		
		/* Allocate a buffer big enough to write this resource. */

		p = *theStrings;
		nLines = *(short*)p;
		p += 2;
		pTo = buf = NewPtr(GetHandleSize(theStrings) - 2 + nLines);
		
		/* Copy each line from the STR# resource to the buffer, adding
			a return character at the end of each line.    Don't copy lines which 
			contain an "only" escape sequence that specifies that this line is not
			for the saved version.  Don't copy lines which contain a "pict"
			escape sequence.  Discard other escape sequences.  Don't copy
			end-of-paragraph marks. */
		
		while (nLines--) {
			nBytes = *p;
			q = p+1;
			qEnd = q + nBytes;
			skip = false;
			while (q < qEnd && *q < 31) {
				if (*q == docPict ||
					(*q == docOnly && !(*(q+2) & docSave))) {
					skip = true;
					break;
				};
				nBytes -= *(q+1);
				q += *(q+1);
			};
			if (!skip) {
				if (nBytes && *(q+nBytes-1) == docEop) nBytes--;
				memcpy(pTo, q, nBytes);
				pTo += nBytes;
				*pTo++ = '\n';
			};
			p += *p + 1;
		};
		HUnlock(theStrings);
		
		/* Write the buffer to the file and dispose of the buffer. */
		
		count = pTo - buf;
		rCode = FSWrite(refNum, &count, buf);
		DisposPtr(buf);
		if (rCode) return rCode;
		
		i++;
	};
	
	return noErr;
}

/*______________________________________________________________________

	Write - Write the Report File.
	
	Entry:	repHandle = handle to report record.
				creator = creator type for the file.
				fName = pointer to file name.
				vRefNum = volume reference number.
				
	Exit:		function result = result code.
_____________________________________________________________________*/


static OSErr Write (Handle repHandle, OSType creator, 
	Str255 fName, short vRefNum)

{
	ListHandle		theList;			/* handle to list record */
	OSErr				rCode;			/* result code */
	short				refNum;			/* file reference number */
	
	/* Create the file.  Delete it if it already exists. */
	
	if ((rCode = Create(fName, vRefNum, creator, 'TEXT')) == dupFNErr) {
		if (rCode = FSDelete(fName, vRefNum)) return rCode;
		rCode = Create(fName, vRefNum, creator, 'TEXT');
	};
	if (rCode) return rCode;
	
	/* Open the file. */
	
	if (rCode = FSOpen(fName, vRefNum, &refNum)) return rCode;
	
	/* Write the file. */
	
	theList = (ListHandle) repHandle;
	if ((**theList).refCon) {
		rCode = Write1 (theList, refNum); 
	} else {
		rCode = Write0 (theList, refNum);
	};
	if (rCode) {
		(void) FSClose(refNum);
		return rCode;
	};
	
	/* Close the file. */
	
	if (rCode = FSClose(refNum)) return rCode;
	return noErr;
}

/*______________________________________________________________________

	rep_Save - Save a Report as a Text File.
	
	Entry:	repHandle = handle to report record.
				prompt = pointer to SFPutFile prompt sting.
				defName = pointer to default file name.
				creator = creator type for the file.
				menuPick = true if save operation was initiated via a 
					menu pick, false if it was initiated via a command key.
				
	Exit:		good = true if report saved, false if canceled by user.
				function result = result code.
_____________________________________________________________________*/


OSErr rep_Save (Handle repHandle, Str255 prompt, 
	Str255 defName, OSType creator, Boolean *good, Boolean menuPick)

{
	Point				where;			/* location of SFPutFile dialog */
	SFReply			reply;			/* SFPutFile reply */
	Handle			dlgHandle;		/* handle to SFPutFile DLOG resource */
	Rect				dlgRect;			/* SFPutFile dialog rectangle */
	OSErr				rCode;			/* result code */
	CursHandle		watch;			/* handle to watch cursor */
	
	/* Center the SFPutFile dialog. */
	
	dlgHandle = GetResource('DLOG', putDlgID);
	dlgRect = **(Rect**)dlgHandle;
	utl_CenterDlogRect(&dlgRect, menuPick);
	SetPt(&where, dlgRect.left, dlgRect.top);
	
	/* Call SFPutFile.  Return if dialog canceled. */
	
	utl_FixStdFile();
	SFPutFile(where, prompt, defName, nil, &reply);
	if (!(*good = reply.good)) return noErr;
	
	/* Create and write the file. */
	
	watch = GetCursor(watchCursor);
	SetCursor(*watch);
	rCode = Write(repHandle, creator, reply.fName, reply.vRefNum);
	InitCursor();
	if (rCode) {
		(void) FSDelete(reply.fName, reply.vRefNum);
		return rCode;
	};
	return FlushVol(nil, reply.vRefNum);
}

/*______________________________________________________________________

	rep_Key - Process Up and Down Arrow Keys.
	
	Entry:	repHandle = handle to report record.
				key = key pressed (up or down arrow).
				modifiers = modifier keys.
				
	Exit:		function result = error code.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
	
	If the key down event is not an up or down arrow key then the routine
	returns without doing anything.
_____________________________________________________________________*/


OSErr rep_Key (Handle repHandle, short key, short modifiers)

{

	ListHandle		theList;				/* handle to list record */
	short				direc;				/* +1 or -1 for direction to scroll */
	short				height;				/* height of report in lines */
	
	if (key == upArrow) {
		direc = -1;
	} else if (key == downArrow) {
		direc = 1;
	} else {
		return noErr;
	};
	theList = (ListHandle) repHandle;
	LDoDraw(true, theList);
	if (modifiers & cmdKey) {
		if (modifiers & shiftKey) {
			LScroll(0, direc*(**theList).dataBounds.bottom, theList);
		} else {
			height = ((**theList).rView.bottom - (**theList).rView.top) /
				(**theList).cellSize.v - 1;
			LScroll(0, direc*height, theList);
		};
	} else {
		LScroll(0, direc, theList);
	};
	return noErr;
}

/*______________________________________________________________________

	rep_Jump - Scroll (jump) to a Specified Line.
	
	Entry:	repHandle = handle to report record.
				lineNum = line number to jump to.
				doDraw = true to redraw the report rectangle.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
_____________________________________________________________________*/


void rep_Jump (Handle repHandle, short lineNum, Boolean doDraw)

{
	ListHandle		theList;			/* handle to list record */
	
	theList = (ListHandle) repHandle;
	LDoDraw(doDraw, theList);
	LScroll(0, lineNum - (**theList).visible.top, theList);
}

/*______________________________________________________________________

	rep_Clear - Clear a Report.
	
	Entry:	repHandle = handle to report record.

	Don't forget to do a SetPort to the window containing the report
	before calling this routine.
	
	The routine simply returns for type 1 reports.
_____________________________________________________________________*/


void rep_Clear (Handle repHandle)

{
	ListHandle	theList;			/* handle to list record */

	theList = (ListHandle) repHandle;
	if ((**theList).refCon) return;
	LDoDraw(true, theList);
	LDelRow(0, 0, theList);
}

/*______________________________________________________________________

	rep_Full	- Check a Report to See if it's Full.
	
	Entry:	repHandle = handle to report record.
				slop = number of bytes to guarantee free.
				
	Exit:		funtion result = true if not at least slop free bytes.
_____________________________________________________________________*/


Boolean rep_Full (Handle repHandle, long slop)

{
	ListHandle		theList;			/* handle to list record */
	long				repSize;			/* size of list cell data */

	theList = (ListHandle) repHandle;
	repSize = GetHandleSize((**theList).cells);
	return slop > (0x7fff - repSize);
}

/*______________________________________________________________________

	rep_Dispose	- Dispose of a report.
	
	Entry:	repHandle = handle to report record.
				cellOption = CELL resource disposal option (type 1 reports 
					only):
					0 = release the CELL resource.
					1 = keep the CELL resource, but make it purgable.
					2 = keep the CELL resource, and leave it unpurgable.
_____________________________________________________________________*/


void rep_Dispose (Handle repHandle, short cellOption)

{
	ListHandle			theList;				/* handle to list record */
	auxInfo				**aux;				/* handle to auxiliary info */

	theList = (ListHandle) repHandle;
	
	/* For a type 1 report, dispose of the auxiliary info, and process 
		the cell option. */

	if ((**theList).refCon) {
		aux = (auxInfo**)(**theList).userHandle;
		if ((**aux).cachedPictID) DisposPtr((**aux).cachedBitMap.baseAddr);
		DisposHandle((Handle)aux);
		switch (cellOption) {
			case 0:
				ReleaseResource((**theList).cells);
				break;
			case 1:
				HPurge((**theList).cells);
				break;
			case 2:
				break;
		};
		(**theList).cells = NewHandle(0);
	};
	
	/* Dispose of the list. */
	
	LDispose(theList);
}

/*______________________________________________________________________

	rep_GetSize - Get the Size of a Report.
	
	Entry:	repHandle = handle to report record.
	
	Exit:		function result = number of lines in report.
_____________________________________________________________________*/


short rep_GetSize (Handle repHandle)

{
	ListHandle		theList;				/* handle to list record */
	
	theList = (ListHandle)repHandle;
	return (**theList).dataBounds.bottom;
}

/*______________________________________________________________________

	rep_Tag - Lookup a Tag.
	
	Entry:	id = TAG resource id.
				tag = tag to lookup in TAG resource.
	
	Exit:		function result = corresponding line number from
					tag resource, or 0 if none found.
_____________________________________________________________________*/


short rep_Tag (short id, short tag)

{
	Handle				h;				/* handle to TAG resource */
	char					*p;			/* pointer into TAG resource */
	short					n;				/* number of entries in TAG resource */
		
	h = GetResource('TAG ', id);
	if (!h) return 0;
	p = *h;
	n = *(short*)p;
	p += 4;
	while (n--) {
		if (tag == *(short*)p) return *(short*)(p-2);
		p += 4;
	};
	return 0;
}

/*______________________________________________________________________

	rep_Height - Set New Report Height.
	
	Entry:	repHandle = handle to report record.
				bottom = new bottom coord.
				
	The new height is rounded down to the nearest multiple of the cell
	height.
_____________________________________________________________________*/


void rep_Height (Handle repHandle, short bottom)

{
	ListHandle		theList;				/* handle to list record */
	short				oldHeight;			/* old height of list rect */
	short				newHeight;			/* new height of list rect */
	Rect				inval;				/* rect to invalidate */
	
	theList = (ListHandle)repHandle;
	oldHeight = (**theList).rView.bottom - (**theList).rView.top;
	newHeight = bottom - (**theList).rView.top;
	newHeight -= newHeight % (**theList).cellSize.v;
	if (oldHeight == newHeight) return;
	LDoDraw(false, theList);
	LSize((**theList).rView.right - (**theList).rView.left,
		newHeight, theList);
	LDoDraw(true, theList);
	inval = (**theList).rView;
	InsetRect(&inval, -1, -1);
	inval.right += 15;
	inval.bottom = (**theList).port->portRect.bottom;
	InvalRect(&inval);
}

/*______________________________________________________________________

	rep_GetRect - Get Report Rectangle.
	
	Entry:	repHandle = handle to report record.
				
	Exit:		rect = report rectangle.
_____________________________________________________________________*/


void rep_GetRect (Handle repHandle, Rect *rect)

{
	ListHandle		theList;				/* handle to list record */
	
	theList = (ListHandle)repHandle;
	*rect = (**theList).rView;
	rect->right += 15;
}

/*______________________________________________________________________

	rep_GetPos - Get Report Scroll Position.
	
	Entry:	repHandle = handle to report record.
				
	Exit:		function result = line number of top visible line in report.
_____________________________________________________________________*/


short rep_GetPos (Handle repHandle)

{
	ListHandle		theList;				/* handle to list record. */
	
	theList = (ListHandle)repHandle;
	return (**theList).visible.top;
}