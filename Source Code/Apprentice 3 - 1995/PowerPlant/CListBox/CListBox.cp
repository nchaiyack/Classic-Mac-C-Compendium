// =================================================================================
//	CListBox.cp			�1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	CListBox.h	

#include "CListBox.h"


// ---------------------------------------------------------------------------------
//		� CListBox
// ---------------------------------------------------------------------------------

CListBox::CListBox()
{
	mLDEFProc = nil;
}


// ---------------------------------------------------------------------------------
//		� CListBox(LStream*)
// ---------------------------------------------------------------------------------

CListBox::CListBox( LStream *inStream ) : LListBox(inStream)
{
	mLDEFProc = nil;
}


// ---------------------------------------------------------------------------------
//		� ~CListBox
// ---------------------------------------------------------------------------------

CListBox::~CListBox()
{
	// Dispose Toolbox ListHandle before LListBox gets a chance.
	// That way, we're still around to get the lCloseMsg.
	if (mMacListH != nil) {
		LDispose( mMacListH );
		mMacListH = nil;
	}
}


// ---------------------------------------------------------------------------------
//		� FinishCreateSelf
// ---------------------------------------------------------------------------------
// This code could have gone in the constructor for CListBox, but then
// derived class's ldef initialization code wouldn't get called.

void
CListBox::FinishCreateSelf()
{
	// Store a pointer to this object in the list handle's userHandle field.
	(**mMacListH).userHandle = (Handle) this;	

	// Call the ldef with the lInitMsg.
	Cell	dummyCell = {0,0};
	MyLDEF( lInitMsg, false, nil, dummyCell, 0, 0, mMacListH );
}


// ---------------------------------------------------------------------------------
//		� MyLDEF
// ---------------------------------------------------------------------------------

pascal void
MyLDEF(  short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lList )
{
	// Get the real object out of the list handle's userHandle field.
	CListBox	*theListBox = (CListBox *) (**lList).userHandle;

	if ( theListBox ) {
	
		switch ( lMessage ) {
		
			case lInitMsg:
				// Set up the callback mechanism by placing a UPP or ProcPtr in
				// the list's refCon. The ldef stub will call this routine back
				// for other messages. (The lInitMsg was called manually.)
#ifdef	__powerc
				theListBox->mLDEFProc = (ListDefProcPtr) NewListDefProc( MyLDEF );
#else
				theListBox->mLDEFProc = (ListDefProcPtr) MyLDEF;
#endif
				(**lList).refCon = (long) theListBox->mLDEFProc;
				
				// Calling through to the subobject's initialize routine
				// is ok since it should have been created by now, or our own
				// will be called anyway.
				theListBox->LDEFInitialize();
				break;
				
			case lDrawMsg:
				// Call the real object's draw routine.
				theListBox->LDEFDraw( lSelect, lRect, lCell, lDataOffset, lDataLen );
				break;
					
			case lHiliteMsg:
				// Call the real object's hilite routine.
				theListBox->LDEFHilite( lSelect, lRect, lCell, lDataOffset, lDataLen );
				break;

			case lCloseMsg:
				// Unfortunately, any derived object has probably been
				// destroyed at this point, so we can't call through
				// to it for the lCloseMsg.
#ifdef	__powerc
				//	Dispose of the ldef's routine descriptor.
				DisposeRoutineDescriptor( (UniversalProcPtr) theListBox->mLDEFProc );
				theListBox->mLDEFProc = nil;
#endif
				break;
		}
	}
}


// ---------------------------------------------------------------------------------
//		� LDEFInitialize
// ---------------------------------------------------------------------------------

void
CListBox::LDEFInitialize()
{
}
