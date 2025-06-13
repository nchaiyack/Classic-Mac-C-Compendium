// =================================================================================
//	CListBox.h			©1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	CListBox.cp	

#pragma once

#include <LListBox.h>

#ifndef __LISTS__
#include <Lists.h>
#endif

#define __CLISTBOX__


class CListBox : public LListBox {
public:
					CListBox();
					CListBox(LStream *inStream);
	virtual			~CListBox();

protected:
	virtual void	FinishCreateSelf();
	virtual void	LDEFInitialize();
	virtual void	LDEFDraw( Boolean lSelect, Rect *lRect, Cell lCell,
						short lDataOffset, short lDataLen ) = 0;
	virtual void	LDEFHilite( Boolean lSelect, Rect *lRect, Cell lCell,
						short lDataOffset, short lDataLen ) = 0;
				
private:
	ListDefProcPtr	mLDEFProc;

	friend static pascal void 	MyLDEF( short lMessage, Boolean lSelect,
									Rect *lRect, Cell lCell, short lDataOffset,
									short lDataLen, ListHandle lList );
};
