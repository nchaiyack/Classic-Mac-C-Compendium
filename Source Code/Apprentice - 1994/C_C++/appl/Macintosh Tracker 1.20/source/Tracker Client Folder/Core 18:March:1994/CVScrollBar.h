/* CVScrollBar.h */

#pragma once

#include "CViewRect.h"

class CWindow;

/* internal part IDs */
#ifndef mNone
#define mNone (0)
#endif
enum	{mUpOne = mNone + 1, mUpPage, mVBox, mDownOne, mDownPage};

struct	CVScrollBar	:	CViewRect
	{
		CViewRect*	Owner;
		long				NumCells;
		long				CellIndex;
		short				CurrentOperation;
		LongPoint		OneUpZoneTL;
		LongPoint		OneUpZoneExt;
		LongPoint		PageUpZoneTL;
		LongPoint		PageUpZoneExt;
		LongPoint		VScrollZoneTL;
		LongPoint		VScrollZoneExt;
		LongPoint		PageDownZoneTL;
		LongPoint		PageDownZoneExt;
		LongPoint		OneDownZoneTL;
		LongPoint		OneDownZoneExt;
		MyBoolean		VScrollable;

		void			IVScrollBar(LongPoint Start, LongPoint Extent, CViewRect* TheOwner,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		short			FindPart(LongPoint Where);
		void			DoMouseDown(MyEventRec Event);
		void			RedrawVBar(void);
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
