/* CEnclosure.c */

#include "CEnclosure.h"
#include "CSack.h"
#include "CWindow.h"


/* */			CEnclosure::CEnclosure()
	{
		CSack*	Temp;

		Temp = new CSack;
		Temp->ISack(sizeof(CViewRect*),128);
		ListOfObjects = Temp;
	}


/* dispose of things */
/* */			CEnclosure::~CEnclosure()
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::~CEnclosure called on uninitialized object."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				delete Thang;
				ListOfObjects->ResetScan();
			}
		delete ListOfObjects;
	}


void			CEnclosure::IEnclosure(LongPoint Start, LongPoint Extent,
						CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CEnclosure::IEnclosure called on already initialized object."));
		EXECUTE(Initialized = True);
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
	}


void			CEnclosure::DoMouseDown(MyEventRec Event)
	{
		CViewRect*	Thang;
		LongPoint		WindowLocalMouseLoc;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoMouseDown called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		WindowLocalMouseLoc.x = Event.Where.x - Window->Start.x;
		WindowLocalMouseLoc.y = Event.Where.y - Window->Start.y;
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				/* if click in object then send it */
				if (LongPtInRect(WindowLocalMouseLoc,LongPointOf(Thang->Origin.x,
					Thang->Origin.y),Thang->Extent) && Thang->Enabled)
					{
						LastMouseDownViewRect = Thang;
						Thang->DoMouseDown(Event);
						return; /* successful dispatch */
					}
			}
		LastMouseDownViewRect = NIL;  /* no mouse down */
	}


MyBoolean	CEnclosure::DoKeyDown(MyEventRec Event)
	{
		CViewRect*		Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoKeyDown called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				if (Thang->Enabled)
					{
						if (Thang->DoKeyDown(Event))
							{
								LastKeyDownViewRect = Thang;
								return;
							}
					}
			}
		LastKeyDownViewRect = NIL;
	}


MyBoolean	CEnclosure::DoMouseMoved(MyEventRec Event)
	{
		CViewRect*	Thang;
		LongPoint		WindowLocalMouseLoc;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoMouseMoved called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		WindowLocalMouseLoc.x = Event.Where.x - Window->Start.x;
		WindowLocalMouseLoc.y = Event.Where.y - Window->Start.y;
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				/* if click in object then send it */
				if (LongPtInRect(WindowLocalMouseLoc,LongPointOf(Thang->Origin.x,
					Thang->Origin.y),Thang->Extent) && Thang->Enabled)
					{
						if (Thang->DoMouseMoved(Event))
							{
								return True; /* successful dispatch */
							}
					}
			}
		return False;
	}


void			CEnclosure::RecalcLocations(LongPoint EnclosureVisRectStart,
						LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::RecalcLocations called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		inherited::RecalcLocations(EnclosureVisRectStart,EnclosureVisRectExtent,
			EnclosureOrigin);
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Thang->RecalcLocations(VisRectStart,VisRectExtent,Origin);
			}
	}


void			CEnclosure::DoUpdate(void)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoUpdate called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Window->SetOrigin(ZeroPoint);
				if (Window->RectVisible(Thang->VisRectStart,Thang->VisRectExtent))
					{
						Thang->DoUpdate(); /* send update to each visible object */
					}
			}
	}


void			CEnclosure::DoResume(void)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoResume called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Thang->DoResume();  /* go back into working order */
			}
		inherited::DoResume();
	}


void			CEnclosure::DoSuspend(void)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoSuspend called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Thang->DoSuspend();  /* go to suspended mode */
			}
		inherited::DoSuspend();
	}


MyBoolean	CEnclosure::DoMenuCommand(ushort MenuCommandValue)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoMenuCommand called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				if (Thang->Enabled)
					{
						if (Thang->DoMenuCommand(MenuCommandValue))
							{
								return True;
							}
					}
			}
		return False;
	}


void			CEnclosure::RegisterViewRect(CViewRect* TheViewRect)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::RegisterViewRect called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->PushElement(&TheViewRect);
	}


void			CEnclosure::DeregisterViewRect(CViewRect* TheViewRect)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DeregisterViewRect called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->KillElement(&TheViewRect);
	}


void			CEnclosure::EnableMenuItems(void)
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::EnableMenuItems called on uninitialized object."));
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				if (Thang->Enabled)
					{
						Thang->EnableMenuItems();
					}
			}
	}


void			CEnclosure::DoEnclosureResized(LongPoint EnclosureExtentAdjust)
	{
		CViewRect*		Thang;
		LongPoint			MyOldExtent;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CEnclosure::DoEnclosureResized called on uninitialized object."));
		MyOldExtent = Extent;
		inherited::DoEnclosureResized(EnclosureExtentAdjust);
		ERROR(ListOfObjects==NIL,PRERR(ForceAbort,"CEnclosure ListOfObjects is NIL."));
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Thang->DoEnclosureResized(MyOldExtent);
			}
	}
