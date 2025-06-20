// =================================================================================
//	CDemoList.h						�1994 AG Group, Inc. All rights reserved.
// =================================================================================
//	CDemoList.cp	

#pragma once

#include "CListBox.h"


class CDemoList : public CListBox {
public:
	static CDemoList*	CreateDemoListStream( LStream *inStream );
						CDemoList();
						CDemoList(LStream *inStream);
	virtual				~CDemoList();

protected:
	virtual void		FinishCreateSelf();

	virtual void		LDEFInitialize();
	virtual void		LDEFDraw( Boolean lSelect, Rect *lRect, Cell lCell,
							short lDataOffset, short lDataLen );
	virtual void		LDEFHilite( Boolean lSelect, Rect *lRect, Cell lCell,
							short lDataOffset, short lDataLen );
};
