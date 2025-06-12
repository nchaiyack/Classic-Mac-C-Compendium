/* CButton.c */

#include "CButton.h"
#include "CWindow.h"
#include "CEnclosure.h"


#define MinTime (5) /* minimum time to hilite a button when pressed by key */


/* initialize the button */
void			CButton::IButton(LongPoint Start, LongPoint Extent, char Key, short Modifiers,
						CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CButton::IButton called on already initialized object."));
		EXECUTE(Initialized = True);
		KeyEquivalent = Key;
		KeyModifiers = Modifiers;
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
	}


/* process mouse down & do the stuff */
void			CButton::DoMouseDown(MyEventRec Event)
	{
		MyBoolean		MouseInsideFlag;
		MyBoolean		NeedToCallDoLeave;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::DoMouseDown called on uninitialized object."));
		SetUpPort();
		if (Enabled)
			{
				MouseInsideFlag = True;
				DoEnter();
				NeedToCallDoLeave = True;
				RedrawHilited();
				ActiveModifiers = Event.Modifiers;
				while (WaitMouseUp())
					{
						if (LongPtInRect(MyGlobalToLocal(GetLongMouseLoc()),
							ZeroPoint,Extent) != MouseInsideFlag)
							{
								MouseInsideFlag = !MouseInsideFlag;
								if (MouseInsideFlag == True)
									{
										DoEnter();
										NeedToCallDoLeave = True;
										RedrawHilited();
									}
								 else
									{
										DoLeave();
										NeedToCallDoLeave = False;
										RedrawNormal();
									}
							}
						RelinquishCPU();
					}
				if (NeedToCallDoLeave)
					{
						DoLeave();
					}
				if (MouseInsideFlag)
					{
						if (!DoThang())
							{
								/* if the button destroyed the window (i.e. a close button) */
								/* it should return True */
								RedrawNormal();
							}
					}
			}
	}


MyBoolean	CButton::DoKeyDown(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::DoKeyDown called on uninitialized object."));
		if (Enabled)
			{
				if (((Event.Message & charCodeMask) == KeyEquivalent) && 
					((Event.Modifiers & (cmdKey+shiftKey+optionKey)) == KeyModifiers))
					{
						long		TimeStarted;

						TimeStarted = TickCount();
						RedrawHilited();
						if (!DoThang())
							{
								/* if the button destroyed the window (i.e. a close button) */
								/* it should return True */
								while (TickCount() - TimeStarted < MinTime)
									{
										/* do nothing */
									}
								RedrawNormal();
							}
						return True;
					}
			}
		return False;
	}


/* draw the proper image (nothing, greyed out, or normal) */
void			CButton::DoUpdate(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::DoUpdate called on uninitialized object."));
		RedrawNormal();
	}


void			CButton::DoEnable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::DoEnable called on uninitialized object."));
		inherited::DoEnable();
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
	}


void			CButton::DoDisable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::DoDisable called on uninitialized object."));
		inherited::DoDisable();
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
	}


void			CButton::RedrawHilited(void)
	{
		EXECUTE(PRERR(AllowResume,"CButton::RedrawHilited has not been overridden."));
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::RedrawHilited called on uninitialized object."));
	}


void			CButton::RedrawNormal(void)
	{
		EXECUTE(PRERR(AllowResume,"CButton::RedrawNormal has not been overridden."));
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CButton::RedrawNormal called on uninitialized object."));
	}


/* override this to make the button do what you want it to */
/* return True if the button deletes the window */
MyBoolean	CButton::DoThang(void)
	{
		return False;
	}


/* DoEnter and DoLeave are called while the button is down as the user */
/* moves over the button.  This can be used for "realtime" buttons such */
/* as fast forward buttons */
void			CButton::DoEnter(void)
	{
	}


void			CButton::DoLeave(void)
	{
	}
