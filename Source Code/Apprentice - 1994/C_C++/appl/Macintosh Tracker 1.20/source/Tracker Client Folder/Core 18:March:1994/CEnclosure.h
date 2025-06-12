/* CEnclosure.h */

#pragma once

#include "CViewRect.h"

/* forwards */
class CSack;
class CEnclosure;

struct CEnclosure	:	CViewRect
	{
		CSack*			ListOfObjects;
		EXECUTE(MyBoolean Initialized;)

		/* */			CEnclosure();
		/* */			~CEnclosure();
		void			IEnclosure(LongPoint Start, LongPoint Extent,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			RegisterViewRect(CViewRect* TheViewRect);
		void			DeregisterViewRect(CViewRect* TheViewRect);
		void			DoMouseDown(MyEventRec Event);
		MyBoolean	DoKeyDown(MyEventRec Event);
		MyBoolean	DoMouseMoved(MyEventRec Event);
		void			DoUpdate(void);
		void			DoSuspend(void);
		void			DoResume(void);
		MyBoolean	DoMenuCommand(ushort MenuCommandValue);
		void			EnableMenuItems(void);
		void			RecalcLocations(LongPoint EnclosureVisRectStart,
								LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin);
		void			DoEnclosureResized(LongPoint EnclosureExtentAdjust);
	};
