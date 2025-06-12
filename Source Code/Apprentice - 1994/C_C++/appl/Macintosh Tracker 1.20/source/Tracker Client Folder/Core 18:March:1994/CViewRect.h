/* CViewRect.h */

#pragma once

#include "EventLoop.h"

/* forward */
class CWindow;
class CMenuController;
class CEventLoop;
class CEnclosure;

/* these are the possible hook values */
enum
	{
		NoHookOperation = 0, /* dummy operation that doesn't get responded to */

		/* operations for scrolling things */
		HScrollToLocation,HScrollLeftOne,HScrollLeftPage,HScrollRightOne,HScrollRightPage,
		VScrollToLocation,VScrollUpOne,VScrollUpPage,VScrollDownOne,VScrollDownPage,

		DumbHookOperation /* place holder to reduce comma insanity */
	};

/* these are values which determine where the edges of an object will */
/* go when it's enclosure resizes */
enum	{LeftEdgeStatic, LeftEdgeSticky, LeftEdgeCustom};
enum	{TopEdgeStatic, TopEdgeSticky, TopEdgeCustom};
enum	{WidthStatic, WidthSticky, WidthCustom};
enum	{HeightStatic, HeightSticky, HeightCustom};

struct	CViewRect	:	CIdle
	{
		CWindow*			Window;
		CEnclosure*		Enclosure;
		MyBoolean			Suspended;
		MyBoolean			Enabled;
		short					WidthResizeMode;
		short					HeightResizeMode;
		short					LeftMoveMode;
		short					TopMoveMode;
		LongPoint			Start; /* where with respect to enclosure's origin */
		LongPoint			Extent; /* how tall and wide is it */
		LongPoint			Origin; /* with respect to window's top-left corner */
		LongPoint			VisRectStart; /* what can be seen (in window-relative coordinates) */
		LongPoint			VisRectExtent;
		Cursor				MyCursor;
		EXECUTE(MyBoolean Initialized;)

		/* */			~CViewRect();
		/* */			CViewRect();
		void			IViewRect(LongPoint TheOrigin, LongPoint TheExtent,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			DoMouseDown(MyEventRec Event);
		void			DoMouseUp(MyEventRec Event);
		MyBoolean	DoKeyDown(MyEventRec Event);
		void			DoKeyUp(MyEventRec Event);
		MyBoolean	DoMouseMoved(MyEventRec Event);
		void			DoUpdate(void);
		void			DoSuspend(void);
		void			DoResume(void);
		void			DoDisable(void);
		void			DoEnable(void);
		long			Hook(short OperationID, long Operand1, long Operand2);
		MyBoolean	BecomeKeyReceiver(void);
		MyBoolean	RelinquishKeyReceivership(void);
		MyBoolean	DoMenuCommand(ushort MenuCommandValue);
		void			EnableMenuItems(void);
		void			DoEnclosureResized(LongPoint EnclosureAdjust);
		void			SetStickiness(short Left, short Top, short Width, short Height);
		long			CustomLeftMove(void);
		long			CustomTopMove(void);
		long			CustomWidthChange(void);
		long			CustomHeightChange(void);
		void			RecalcLocations(LongPoint EnclosureVisRectStart,
								LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin);
		void			RecalcLocsInitial(void);
		void			SetUpPort(void);
		LongPoint	MyGlobalToLocal(LongPoint GlobalPoint);
	};
