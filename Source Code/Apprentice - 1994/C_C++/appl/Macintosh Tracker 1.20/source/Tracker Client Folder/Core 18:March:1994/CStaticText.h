/* CStaticText.h */

#pragma once

#include "CViewRect.h"

/* forwards */
class CWindow;
class CEnclosure;

struct	CStaticText	:	CViewRect
	{
		Handle		Text;
		short			FontID;
		short			PointSize;
		short			Justification;
		EXECUTE(MyBoolean Initialized;)

		/* */		~CStaticText();
		void		IStaticText(LongPoint Start, LongPoint Extent, Handle DaText,
							short TheFontID, short ThePointSize, CWindow* TheWindow,
							CEnclosure* TheEnclosure, short TheJustification);
		void		DoUpdate(void);
		void		DoDisable(void);
		void		DoEnable(void);
	};
