// =================================================================================
//	CDemoList.cp		�1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	CDemoList.h

#include <stdio.h>
#include <string.h>
#include <LStream.h>

#include "CDemoList.h"


// ---------------------------------------------------------------------------------
//		� CreateDemoListStream
// ---------------------------------------------------------------------------------

CDemoList*
CDemoList::CreateDemoListStream( LStream *inStream )
{
	return (new CDemoList(inStream));
}


// ---------------------------------------------------------------------------------
//		� CDemoList
// ---------------------------------------------------------------------------------

CDemoList::CDemoList()
{
}


// ---------------------------------------------------------------------------------
//		� CDemoList(LStream*)
// ---------------------------------------------------------------------------------

CDemoList::CDemoList( LStream *inStream ) : CListBox(inStream)
{
}


// ---------------------------------------------------------------------------------
//		� ~CDemoList
// ---------------------------------------------------------------------------------

CDemoList::~CDemoList()
{
}

// ---------------------------------------------------------------------------------
//		� FinishCreateSelf
// ---------------------------------------------------------------------------------

void
CDemoList::FinishCreateSelf()
{
	// Important! Must call the inherited routine!
	CListBox::FinishCreateSelf();
	
	::LDoDraw( false, mMacListH );
	
	(**mMacListH).selFlags = (lOnlyOne);

	// Make sure there's at least one column.
	if ((**mMacListH).dataBounds.right == 0) {
		::LAddColumn( 1, 0, mMacListH );
	}

	// Add some cells.
	short	i, dummy;
	Cell	cell;
	Str255	string;
	for ( i=0; i<20; ++i ) {
		dummy = LAddRow( 1, i, mMacListH );
		::SetPt( &cell, 0, i );
		sprintf( (char *) string, "Row #%d", i );
		::LAddToCell( string, strlen((char *) string), cell, mMacListH );
	}

	::LDoDraw( true, mMacListH );
}


// ---------------------------------------------------------------------------------
//		� LDEFInitialize
// ---------------------------------------------------------------------------------

void
CDemoList::LDEFInitialize()
{
	FontInfo	fInfo;
	::GetFontInfo( &fInfo );
	(**mMacListH).indent.h = 2;
	(**mMacListH).indent.v = ((**mMacListH).cellSize.v -
		(fInfo.ascent+fInfo.descent)) / 2 + fInfo.ascent;
}


// ---------------------------------------------------------------------------------
//		� LDEFDraw
// ---------------------------------------------------------------------------------

void
CDemoList::LDEFDraw( Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen )
{
	::EraseRect( lRect );
	Str255	theString;
	::LGetCell( &theString[1], &lDataLen, lCell, mMacListH );
	theString[0] = lDataLen;
	::MoveTo( lRect->left+2, lRect->top + (**mMacListH).indent.v );
	::DrawString( theString );
	if ( lSelect ) {
		LMSetHiliteMode( LMGetHiliteMode() & ~(1 << hiliteBit) );
		::InvertRect( lRect );
	}
}

						
// ---------------------------------------------------------------------------------
//		� LDEFHilite
// ---------------------------------------------------------------------------------

void
CDemoList::LDEFHilite( Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen )
{
	LMSetHiliteMode( LMGetHiliteMode() & ~(1 << hiliteBit) );
	::InvertRect( lRect );
}
