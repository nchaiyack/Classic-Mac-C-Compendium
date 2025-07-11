/* CHScrollBar.h */

#pragma once

#include "CViewRect.h"

class CWindow;

/* internal part IDs */
#ifndef mNone
#define mNone (0)
#endif
enum	{mLeftOne = mNone + 1, mLeftPage, mHBox, mRightOne, mRightPage};

struct	CHScrollBar	:	CViewRect
	{
		CViewRect*	Owner;
		long				NumCells;
		long				CellIndex;
		short				CurrentOperation;
		LongPoint		OneLeftZoneTL;
		LongPoint		OneLeftZoneExt;
		LongPoint		PageLeftZoneTL;
		LongPoint		PageLeftZoneExt;
		LongPoint		HScrollZoneTL;
		LongPoint		HScrollZoneExt;
		LongPoint		PageRightZoneTL;
		LongPoint		PageRightZoneExt;
		LongPoint		OneRightZoneTL;
		LongPoint		OneRightZoneExt;
		MyBoolean		HScrollable;

		void			IHScrollBar(LongPoint Start, LongPoint Extent, CViewRect* TheOwner,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		short			FindPart(LongPoint Where);
		void			DoMouseDown(MyEventRec Event);
		void			RedrawHBar(void);
		void			DoUpdate(void);
		void			DoEnable(void);
		void			DoDisable(void);
		void			DoResume(void);
		void			DoSuspend(void);
		void			RecalcScrollRects(void);
		void			RecalcLocations(LongPoint EnclosureVisRectStart,
								LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin);
		void			SetPosition(long NewCellIndex, long NewNumCells);
	};
