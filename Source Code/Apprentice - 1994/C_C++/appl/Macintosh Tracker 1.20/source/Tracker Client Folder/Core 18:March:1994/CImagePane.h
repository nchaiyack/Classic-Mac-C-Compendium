/* CImagePane.h */

#pragma once

#include "CViewRect.h"

struct	CImagePane	:	CViewRect
	{
		short		PictID;

		void		IImagePane(LongPoint Start, LongPoint Extent, CWindow* TheWindow,
							CEnclosure* TheEnclosure, short ThePictID);
		void		DoUpdate(void);
	};
