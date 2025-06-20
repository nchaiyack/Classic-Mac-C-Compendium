/******************************************************************************
	CDisplayIndex.c
	
	Methods for a text list with selectable lines.
		
	Copyright � 1989 Symantec Corporation. All rights reserved.
    Copyright � 1991 Manuel A. P�rez.  All rights reserved.

 ******************************************************************************/


#include "CDisplayIndex.h"
#include <Commands.h>
#include <CDocument.h>
#include <CBartender.h>
#include <Constants.h>
#include "Global.h"
#include <string.h>
#include "BrowserCmds.h"

/*** Global Variables ***/
extern	CBartender	*gBartender;
extern EventRecord  gLastMouseUp;

/***
 *
 * IDisplayIndex
 *
 ***/

void CDisplayIndex::IDisplayIndex(CView *anEnclosure, CBureaucrat *aSupervisor,
		short vLoc, short vHeight, BrowserDirPtr theDir, long index_displayed)
{

	inherited::ISelectLine(anEnclosure, aSupervisor, vLoc, vHeight);
	itsDir = theDir;
	itsSelItem = NULL;

// JRB addition - geneva looks better
	SetSelection( 0, 32767, false);
	SetFontNumber(geneva);
	SetFontSize(10);
// END JRB addition


	SetIndex(index_displayed);
}

/***
 *
 * SetIndex
 *
 *	Build a text handle with the information for the index list, and
 *	stuff it into the TE item.
 *
 ***/

void	CDisplayIndex::SetIndex(long index_displayed)
{
BrowserItemPtr p;
long	offset;
long	totalSize;
short	len;
Handle	data;
char *a;
long i, selLine;

	// as part of the initialization process, check how much
	// data we have in the header, create a handle and copy
	// it there.

	totalSize = 0;
	for (p = itsDir->topItem; p != NULL; p = brGetNext(p))
		switch (index_displayed) {
			case cmdIndexFrom:
//following line deleted by JRB
//			default:
				totalSize += strlen(p->from) + 1;		// 1 is for new line
				break;
			case cmdIndexDate:
				totalSize += strlen(p->date) + 1;		// 1 is for new line
				break;
			case cmdIndexSubject:
				totalSize += strlen(p->subject) + 1;		// 1 is for new line
				break;
// JRB addition - new subject line option
			default:
			case cmdIndexComposite:
				totalSize += strlen(p->composite) + 1;
				break;
// end JRB
		}

	FailNIL(data = NewHandle(totalSize));
	HLock(data);
	offset = 0;
	for (p = itsDir->topItem; p != NULL; p = brGetNext(p)) {
		switch (index_displayed) {
			case cmdIndexFrom:
// following line deleted by JRB
//			default:
				a = p->from;
				break;
			case cmdIndexDate:
				a = p->date;
				break;
			case cmdIndexSubject:
				a = p->subject;
				break;
// JRB addition - new subject line option
			default:
			case cmdIndexComposite:
				a = p->composite;
				break;
// end JRB addition
		}
		len = strlen(a);
		BlockMove(a, (*data)+offset, len);
		offset += len;
		(*data)[offset++] = '\r';		// TE new line
	}
	(*data)[--offset] = ' ';			// remove the last new line
	HUnlock(data);
	SetTextHandle(data);
	DisposHandle(data);

	// Now that we have store the text in the TE item, lets restore the
	// marked items.

	// Save the currently selected line			
	selLine = GetSelectedLine();

	// set all the old tags
	for (i = 0, p = itsDir->topItem; p != NULL; i++, p = brGetNext(p))
		if (brGetMark(p))	// if it was already on
			TagLine(i, true, false, false);

	// Setting the selected line back to what it was
	SetSelectedLine(selLine, true);
}

/***
 *
 * Dispose
 *
 ***/
void CDisplayIndex::Dispose(void)
{
BrowserItemPtr p, q;

	// release memory occupied by link list
	for (p = itsDir->topItem; p != NULL;) {
		q = brGetNext(p);
		Deallocate(p);
		p = q;
	}
	itsDir->topItem = NULL;

	// and close file
	fclose(itsDir->fp);
	inherited::Dispose();

}

/***
 * 
 * SelectionChanged
 *
 * Called after the selection may have changed to notify dependants.
 *
 ***/

void CDisplayIndex::SelectionChanged(void)
{
register BrowserItemPtr p;
long i;

	// find item
	p  = itsDir->topItem;
	for (i = 0; i != selLine && p; i++)
		p = brGetNext(p);

	BroadcastChange(textSelectionChanged, (void *)p);
}

/***
 *
 * TagLine(long line, Boolean setSelection, Boolean redraw)
 *
 ***/
void CDisplayIndex::TagLine(long line, Boolean tag, Boolean setSelection, Boolean redraw)
{
long i, selLine;


	if (setSelection) {
		SetSelectedLine(line, redraw);
		SetFontStyle(bold);
		brSetMark(itsSelItem, tag);	// itsSelItem set by SetSelectedLine
	}
	else {
		// Save the currently selected line			
		selLine = GetSelectedLine();

		SetSelectedLine(line, false);
		SetFontStyle(bold);
		brSetMark(itsSelItem, tag);

		// Setting the selected line back to what it was
		SetSelectedLine(selLine, true);
	}
}

/***
 *
 * SetSelectedLine
 *
 ***/
void CDisplayIndex::SetSelectedLine( long line, Boolean redraw)
{
register BrowserItemPtr p;
long i;

	inherited::SetSelectedLine(line, redraw);

	// find item
	p  = itsDir->topItem;
	for (i = 0; i != selLine && p; i++)
		p = brGetNext(p);
	itsSelItem = p;
}

/***
 *
 * TagItem
 *
 ***/
void CDisplayIndex::TagItem(void)
{
	SetFontStyle(bold);
	brToggleMark(itsSelItem);
}

/***
 *
 * GetSelectedItem
 *
 ***/
BrowserItemPtr CDisplayIndex::GetSelectedItem(void)
{
	return itsSelItem;
}

/***
 *
 * GetDirectory
 *
 ***/
BrowserDirPtr CDisplayIndex::GetDirectory(void)
{
	return itsDir;
}