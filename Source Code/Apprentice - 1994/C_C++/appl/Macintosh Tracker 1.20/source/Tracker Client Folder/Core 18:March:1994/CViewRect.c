/* CViewRect.c */

#include "CViewRect.h"
#include "CWindow.h"
#include "CEnclosure.h"


/* */		CViewRect::~CViewRect()
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::~CViewRect called on uninitialized object."));
		if (KeyReceiverViewRect == this) {KeyReceiverViewRect = NIL;}
		if (LastKeyDownViewRect == this) {LastKeyDownViewRect = NIL;}
		if (LastMouseDownViewRect == this) {LastMouseDownViewRect = NIL;}
		if (Enclosure != NIL)
			{
				Enclosure->DeregisterViewRect(this);
			}
		if ((Window != this) && (Window != NIL)) /* filter the two bad possibilities */
			{
				/* i.e. don't do this if we are the window. */
				SetUpPort();
				Window->InvalidateLong(ZeroPoint,Extent);
				Window->DeregisterClient(this);
			}
	}


/* */		CViewRect::CViewRect()
	{
		EXECUTE(Initialized = False;)
		MyCursor = arrow;
		SetStickiness(LeftEdgeStatic,TopEdgeStatic,WidthStatic,HeightStatic);
		Enabled = True;
		Suspended = True;
	}


void		CViewRect::IViewRect(LongPoint TheOrigin, LongPoint TheExtent,
					CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CViewRect::IViewRect called on already initialized object."));
		EXECUTE(Initialized = True);
		Start = TheOrigin;
		Extent = TheExtent;
		Window = TheWindow;
		ERROR(TheWindow==NIL,PRERR(ForceAbort,
			"CViewRect::IViewRect passed NIL for a window."));
		Enclosure = TheEnclosure;
		if (TheEnclosure != NIL)
			{
				/* this means it is NOT a window (only windows don't have enclosures) */
				TheEnclosure->RegisterViewRect(this);
				RecalcLocsInitial();
				TheWindow->RegisterClient(this);
			}
	}


void			CViewRect::DoMouseDown(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoMouseDown called on uninitialized object."));
	}


void			CViewRect::DoMouseUp(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoMouseUp called on uninitialized object."));
	}


MyBoolean	CViewRect::DoKeyDown(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoKeyDown called on uninitialized object."));
		return False;
	}


void			CViewRect::DoKeyUp(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoKeyUp called on uninitialized object."));
	}


MyBoolean	CViewRect::DoMouseMoved(MyEventRec Event)
	{
		SetCursor(&MyCursor);
		return True;
	}


void			CViewRect::DoUpdate(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoUpdate called on uninitialized object."));
	}


/* this is when the window ceases to be active */
void			CViewRect::DoSuspend(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoSuspend called on uninitialized object."));
		ERROR(Suspended,PRERR(ForceAbort,
			"CViewRect::DoSuspend called on a suspended object."));
		if ((KeyReceiverViewRect != this) || ((KeyReceiverViewRect == this)
			&& (RelinquishKeyReceivership())))
			{
				Suspended = True;
			}
	}


/* this is when the window again becomes active */
void			CViewRect::DoResume(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoResume called on uninitialized object."));
		ERROR(!Suspended,PRERR(ForceAbort,
			"CViewRect::DoResume called on an object that isn't suspended."));
		Suspended = False;
	}


/* this is when the object becomes disabled */
void			CViewRect::DoDisable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoDisable called on uninitialized object."));
		if ((KeyReceiverViewRect != this) || ((KeyReceiverViewRect == this)
			&& (RelinquishKeyReceivership())))
			{
				Enabled = False;
			}
	}


/* this is when the object becomes enabled */
void			CViewRect::DoEnable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoEnable called on uninitialized object."));
		Enabled = True;
	}


MyBoolean	CViewRect::BecomeKeyReceiver(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::BecomeKeyReceiver called on uninitialized object."));
		if (ActiveWindow != Window)
			{
				EXECUTE(PRERR(AllowResume,"CViewRect::BecomeKeyReceiver called when object's "
					"window was not the active window."));
				return False;
			}
		 else
			{
				if (KeyReceiverViewRect != this)
					{
						if ((KeyReceiverViewRect == NIL) || ((KeyReceiverViewRect != NIL)
							&& (KeyReceiverViewRect->RelinquishKeyReceivership())))
							{
								KeyReceiverViewRect = this;
								return True;
							}
						 else
							{
								return False;
							}
					}
				 else
					{
						return False; /* if we already are, we can't become */
					}
			}
	}


MyBoolean	CViewRect::RelinquishKeyReceivership(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::RelinquishKeyReceivership called on uninitialized object."));
		ERROR(KeyReceiverViewRect != this,PRERR(ForceAbort,
			"CViewRect::RelinquishKeyReceivership sent to object that isn't a key Receiver."));
		KeyReceiverViewRect = NIL;
		return True;
	}


MyBoolean	CViewRect::DoMenuCommand(ushort MenuCommandValue)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoMenuCommand called on uninitialized object."));
		return False;
	}


/* enable the menu items this object can handle */
void			CViewRect::EnableMenuItems(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::EnableMenuItems called on uninitialized object."));
	}


/* recalculate the visible rectangle and origin */
void			CViewRect::RecalcLocations(LongPoint EnclosureVisRectStart,
						LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::RecalcLocations called on uninitialized object."));
		Origin.x = Start.x + EnclosureOrigin.x;
		Origin.y = Start.y + EnclosureOrigin.y;
		SectLongRect(EnclosureVisRectStart,EnclosureVisRectExtent,Origin,Extent,
			&VisRectStart,&VisRectExtent);
	}


/* this is called when object is first created.  it allows things such as scrolling */
/* views to wait for their contents to be initialized before recalcing */
void			CViewRect::RecalcLocsInitial(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::RecalcLocsInitial called on uninitialized object."));
		RecalcLocations(Enclosure->VisRectStart,Enclosure->VisRectExtent,Enclosure->Origin);
	}


/* set up the window for our drawing environment */
void			CViewRect::SetUpPort(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::SetUpPort called on uninitialized object."));
		ERROR(Window==NIL,PRERR(ForceAbort,
			"CViewRect::SetUpPort called, but Window is NIL."));
		Window->SetMyPort();
		Window->SetOrigin(Origin);
		Window->SetClipRect(ZeroPoint,Extent);
	}


LongPoint	CViewRect::MyGlobalToLocal(LongPoint GlobalPoint)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::MyGlobalToLocal called on uninitialized object."));
		ERROR(Window==NIL,PRERR(ForceAbort,
			"CViewRect::MyGlobalToLocal called, but Window is NIL."));
		GlobalPoint.x = GlobalPoint.x - Window->Start.x - Origin.x;
		GlobalPoint.y = GlobalPoint.y - Window->Start.y - Origin.y;
		return GlobalPoint;
	}


long			CViewRect::Hook(short OperationID, long Operand1, long Operand2)
	{
		return 0;
	}


void			CViewRect::SetStickiness(short Left, short Top, short Width, short Height)
	{
		WidthResizeMode = Width;
		HeightResizeMode = Height;
		LeftMoveMode = Left;
		TopMoveMode = Top;
	}


void			CViewRect::DoEnclosureResized(LongPoint EnclosureAdjust)
	{
		long			LeftAdjust;
		long			TopAdjust;
		long			WidthAdjust;
		long			HeightAdjust;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CViewRect::DoEnclosureResized called on uninitialized object."));
		/* invalidate where we are [soon to be were] */
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);

		switch (LeftMoveMode)
			{
				case LeftEdgeStatic:
					LeftAdjust = 0;
					break;
				case LeftEdgeSticky:
					LeftAdjust = EnclosureAdjust.x;
					break;
				case LeftEdgeCustom:
					LeftAdjust = CustomLeftMove();
					break;
			}
		switch (WidthResizeMode)
			{
				case WidthStatic:
					WidthAdjust = 0;
					break;
				case WidthSticky:
					WidthAdjust = EnclosureAdjust.x;
					break;
				case WidthCustom:
					WidthAdjust = CustomWidthChange();
					break;
			}
		switch (TopMoveMode)
			{
				case TopEdgeStatic:
					TopAdjust = 0;
					break;
				case TopEdgeSticky:
					TopAdjust = EnclosureAdjust.y;
					break;
				case TopEdgeCustom:
					TopAdjust = CustomTopMove();
					break;
			}
		switch (HeightResizeMode)
			{
				case HeightStatic:
					HeightAdjust = 0;
					break;
				case HeightSticky:
					HeightAdjust = EnclosureAdjust.y;
					break;
				case HeightCustom:
					HeightAdjust = CustomHeightChange();
					break;
			}
		Start.x += LeftAdjust;
		Start.y += TopAdjust;
		Extent.x += WidthAdjust;
		Extent.y += HeightAdjust;
		RecalcLocations(Enclosure->VisRectStart,Enclosure->VisRectExtent,
			Enclosure->Origin);

		/* invalidate where we moved to */
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
	}


long			CViewRect::CustomLeftMove(void)
	{
		return 0;
	}


long			CViewRect::CustomTopMove(void)
	{
		return 0;
	}


long			CViewRect::CustomWidthChange(void)
	{
		return 0;
	}


long			CViewRect::CustomHeightChange(void)
	{
		return 0;
	}
