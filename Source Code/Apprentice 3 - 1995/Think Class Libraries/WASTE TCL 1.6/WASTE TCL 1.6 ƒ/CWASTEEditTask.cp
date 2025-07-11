/*
 *
 *	File:		CWASTEEditTask.cp
 *	Module:		CWASTEEditTask method definitions
 *	System:		Mark's Class Library
 *	Evironment:	MacOS 7.0/THINK C 7.0/TCL 2.0
 *	Author:		Mark Alldritt
 *
 *
 *		Copyright � 1994	All Rights Reserved
 *		Mark Alldritt
 *		1571 Deep Cove Road
 *		N. Vancouver, B.C. CANADA
 *
 *	
 *	Description:
 *
 *  This class modifies the CTextEditTask to allow it to work with CWASTEText.
 *
 *
 *	Acknowledgemets:
 *
 *	None.
 *
 *
 *	Notes:
 *
 *
 *	Edit History:
 *
 *	V1-0.0	Mark Alldritt	10-May-1994
 *	- Initial version of module.
 *  V1.0.1  Modifications by Dan Crevier to TCL 2.0
 *  V1.0.2  Added code by Mark Alldritt for undo/redo
 *
 */


#include "CWASTEText.h"
#include "CWASTEEditTask.h"

#include <CClipboard.h>

extern CClipboard *gClipboard;

CWASTEEditTask::CWASTEEditTask(CWASTEText *aTextPane, long anEditCmd, 
							   short firstTaskIndex)
{
	Handle	h;
	
	origStyleScrap = deletedStyles = insertedStyles = NULL;

	// can't call through constructor, or our SaveRange won't be called
	ITextEditTask( aTextPane, anEditCmd, firstTaskIndex);

	gClipboard->GetData( 'styl', &h);
	origStyleScrap = (StScrpHandle) h;

}


CWASTEEditTask::~CWASTEEditTask()
{
	ForgetHandle( origStyleScrap);
	ForgetHandle( deletedStyles);
	ForgetHandle( insertedStyles);
}


void CWASTEEditTask::SaveRange( tRangeSelector whichRange)
{
	StScrpHandle	h;
	
	inherited::SaveRange( whichRange);
	
	/* what we need to do is to copy the sel starts and ends and styles
	either of what was inserted or deleted  mfnov 1994*/
	
	
	h=(StScrpHandle)NewHandleCanFail(1);
	FailNIL(h);

	if (whichRange == kInsertedRange)
		((CWASTEText*) itsTextPane)->CopyRangeWithStyle(inserted.start,inserted.end, (Handle)NULL,
			h);
	else
		((CWASTEText*) itsTextPane)->CopyRangeWithStyle(deleted.start,deleted.end, (Handle)NULL,
			h);
			
	if (whichRange == kInsertedRange)
		insertedStyles = h;
	else
		deletedStyles = h;
								
								
	
/* OLD	itsTextPane->GetSelection( &oldStart, &oldEnd);
	
	if (whichRange == kInsertedRange)
		itsTextPane->SetSelection( inserted.start, inserted.end, FALSE);
	else
		itsTextPane->SetSelection( deleted.start, deleted.end, FALSE);
		
	h = ((CWASTEText*) itsTextPane)->GetTheStyleScrap();
	
	itsTextPane->SetSelection( oldStart, oldEnd, FALSE);
	
	if (whichRange == kInsertedRange)
		insertedStyles = h;
	else
		deletedStyles = h;  */
	
}


void CWASTEEditTask::RestoreRange( tRangeSelector whichRange, Boolean killData)
{
	Handle			text;
	StScrpHandle 	styles;
	long			selStart, selEnd, start;
	short			styleFixup;
	
	if (whichRange == kInsertedRange)
	{
		text = inserted.text;
		styles = insertedStyles;
		start = inserted.start;
		selStart = inserted.selStart;
		selEnd = inserted.selEnd;
		if (killData)
		{
			inserted.text = NULL;
			insertedStyles = NULL;
		}
	}
	else
	{
		text = deleted.text;
		styles = deletedStyles;
		start = deleted.start;
		selStart = deleted.selStart;
		selEnd = deleted.selEnd;
		if (killData)
		{
			deleted.text = NULL;
			deletedStyles = NULL;
		}
	}
	
	if (text)
	{
		// Due to backspacing, the save styled scrap handle may
		// begin with negative offsets. Before attempting to
		// restore these styles we must fix up the offsets
		// to start at 0.
		
		styleFixup = -(**styles).scrpStyleTab[0].scrpStartChar;
		if (styleFixup > 0)
		{
			ScrpSTElement	*table = (**styles).scrpStyleTab;
			short i = (**styles).scrpNStyles;
			
			while (--i >= 0)
			{ 
				table++->scrpStartChar += styleFixup;
			}
		}

		itsTextPane->SetSelection( start, start, TRUE);
		MoveHHi( text);
		HLock( text);
		((CWASTEText *)itsTextPane)->InsertWithStyle(*text, GetHandleSize( text), styles, FALSE);
		HUnlock( text);
	}
	if (killData)
	{
		ForgetHandle(text);
		ForgetHandle( styles);
	}
	itsTextPane->SetSelection( selStart, selEnd, TRUE);

}


void CWASTEEditTask::StoreToClip( tClipSelector whichClip)
{
	StScrpHandle 	h;

	inherited::StoreToClip( whichClip);
		
	if (whichClip == kOldClip)
		h = origStyleScrap;
	else
		h = deletedStyles;

	if (h)
		gClipboard->PutData( 'styl', (Handle) h);
		

}


void CWASTEEditTask::DoBackspace( void)
{
	long 			selStart, selEnd;
	ScrpSTElement 	scrapEl;
	WERunInfo		runInfo;
	
	itsTextPane->GetSelection( &selStart, &selEnd);
	
	/*
	 *	We only need to do something when the user is backspacing
	 *  over original text that was not selected at the start
	 *  of this task. When that happens, we must check the style
	 *  of the char being deleted. If it is different from the
	 *  style currently at the front of the deletedStyle handle
	 *  then we must insert it to the handle.
	 */
	
	if ((selStart > 0) && (selStart == deleted.start) && (selStart == selEnd))
	{
		WEGetRunInfo(selStart - 1, &runInfo, ((CWASTEText *)itsTextPane)->macWE);
		
		/* make up a style scrap element for the char to be deleted.	*/
		/* Then we'll see if it matches the first style in the			*/
		/* saved scrap. If not, we need to insert it to the scrap		*/
		
		scrapEl.scrpStartChar = (**deletedStyles).scrpStyleTab[0].scrpStartChar-1;
		scrapEl.scrpHeight = runInfo.runHeight;
		scrapEl.scrpAscent = runInfo.runAscent;
		/* remaining fields are equivalent to a TextStyle record	*/
		*(TextStyle*) &scrapEl.scrpFont = runInfo.runStyle;
		
		/* CheckNewStyle inserts the new style at the head of the	*/
		/* style scrap if its new. Otherwise, we just decrement		*/
		/* the starting offset for the first style. This is			*/
		/* faster than running through the whole table, fixing		*/
		/* up offsets each time the user backspaces, but we have	*/
		/* to be sure and fix these up later, before attempting to	*/
		/* use these styles											*/
		
		if (!CheckNewStyle( &scrapEl, deletedStyles, TRUE))		
			(**deletedStyles).scrpStyleTab[0].scrpStartChar--;
	}

	inherited::DoBackspace();
}


void CWASTEEditTask::DoFwdDelete( void)
{
	long 			selStart, selEnd, length;
	ScrpSTElement 	scrapEl;
	WERunInfo		runInfo;
	
	itsTextPane->GetSelection( &selStart, &selEnd);
	length = itsTextPane->GetLength();
	
	if ((selStart == selEnd) && (selEnd < length))
	{
		WEGetRunInfo(selStart - 1, &runInfo, ((CWASTEText *)itsTextPane)->macWE);

		/* make up a style scrap element for the char to be deleted.	*/
		/* Then we'll see if it matches the last style in the			*/
		/* saved scrap. If not, we need to insert it to the scrap		*/
		
		scrapEl.scrpStartChar = deleted.end - deleted.start + 
					(**deletedStyles).scrpStyleTab[0].scrpStartChar;
		scrapEl.scrpHeight = runInfo.runHeight;
		scrapEl.scrpAscent = runInfo.runAscent;
		/* remaining fields are equivalent to a TextStyle record	*/
		*(TextStyle*) &scrapEl.scrpFont = runInfo.runStyle;
		
		CheckNewStyle( &scrapEl, deletedStyles, FALSE);	
	}
	inherited::DoFwdDelete();
}


Boolean CWASTEEditTask::CheckNewStyle(ScrpSTElement *scrapEl, 
									  StScrpHandle styleH,
									  Boolean atStart)
{
	short 	index = atStart ? 0 : (**styleH).scrpNStyles-1;
	long	currSize;
	Boolean wasNew = FALSE;
	
	/* compare scrapEl with either first or last scrapElement in scrap handle	*/
	/* but don't compare the scrpStartChar fields								*/
	/* If they don't match, then a new ScrpSTElement is inserted				*/
	
	if (!EqualMem( &scrapEl->scrpHeight, 
			&(**styleH).scrpStyleTab[index].scrpHeight, 
			sizeof(TextStyle) - sizeof(long)))
	{
			
		currSize = GetHandleSize( (Handle) styleH);
		ResizeHandleCanFail( (Handle) styleH, currSize + sizeof(ScrpSTElement));
		FailMemError();
		
		/* if atStart, then move old styles down			*/
		
		if (atStart)
		{
			BlockMove( &(**styleH).scrpStyleTab[0],
					&(**styleH).scrpStyleTab[1],
					currSize - sizeof(short));
		}
		else
			index++; // place at end
		
		/* place new style in position, increment count		*/
		
		(**styleH).scrpStyleTab[index] = *scrapEl;
		(**styleH).scrpNStyles++;
				
		wasNew = TRUE;
	}

	return wasNew;
}

/*
void CWASTEEditTask::Undo()
{   inherited::Undo();
	CTask::Undo();
	
	if (doText)
	{
		if (stillTyping)
			CancelTyping();

		SaveRange(kInsertedRange);
	
		itsTextPane->Prepare();
		itsTextPane->SetSelection(inserted.start, inserted.end, FALSE);
		RestoreRange(kDeletedRange, FALSE);
	}
	
	if (doClip)
		StoreToClip(kOldClip);
}
 */
/*
void CWASTEEditTask::Redo()
{
	undone = FALSE;
	if (editCmd == cmdNull) stillTyping = TRUE;

	if (doText)
	{
		itsTextPane->Prepare();
		itsTextPane->SetSelection(deleted.start, deleted.end, FALSE);
		RestoreRange(kInsertedRange, TRUE);

		if (editCmd == cmdNull)
			stillTyping = TRUE;
	}

	if (doClip)
		StoreToClip(kNewClip);

	ReportChange(TRUE);
}

*/


