/* CSimpleButton.c */

#include "CSimpleButton.h"
#include "CWindow.h"
#include "Memory.h"

#define OvalRadius (10)


/* kill things */
/* */			CSimpleButton::~CSimpleButton()
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSimpleButton::~CSimpleButton called on uninitialized object."));
		ReleaseHandle(Name);
	}


/* initialize the button */
void			CSimpleButton::ISimpleButton(LongPoint Start, LongPoint Extent,
						Handle NameString, char Key, short Modifiers, CWindow* TheWindow,
						CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CSimpleButton::ISimpleButton called on already initialized object."));
		EXECUTE(Initialized = True);
		Name = NameString;
		ERROR(NameString==NIL,PRERR(ForceAbort,"CSimpleButton::ISimpleButton passed NIL for name text."));
		IButton(Start,Extent,Key,Modifiers,TheWindow,TheEnclosure);
	}


void			CSimpleButton::RedrawNormal(void)
	{
		LongPoint		EffectiveStart;
		LongPoint		EffectiveExtent;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSimpleButton::RedrawNormal called on uninitialized object."));
		SetUpPort();
		Window->ResetPen();
		Window->SetText(0,0,srcOr,12,0);
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		EffectiveStart = ZeroPoint;
		EffectiveExtent = Extent;
		if ((KeyEquivalent == 0x0d) && (KeyModifiers == 0))
			{
				EffectiveExtent.x -= 2;
				EffectiveExtent.y -= 2;
				Window->SetPenSize(3,3);
				Window->LFrameRoundRect(EffectiveStart,EffectiveExtent,OvalRadius+4,OvalRadius+4);
				EffectiveStart.x += 4;
				EffectiveStart.y += 4;
				EffectiveExtent.x -= 8;
				EffectiveExtent.y -= 8;
				Window->SetPenSize(1,1);
			}
		Window->LEraseRoundRect(EffectiveStart,EffectiveExtent,OvalRadius,OvalRadius);
		Window->LFrameRoundRect(EffectiveStart,EffectiveExtent,OvalRadius,OvalRadius);
		HLock(Name);
		Window->LDrawText(EffectiveStart,EffectiveExtent,Name,JustifyCenter);
		HUnlock(Name);
	}


void			CSimpleButton::RedrawHilited(void)
	{
		LongPoint		EffectiveStart;
		LongPoint		EffectiveExtent;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSimpleButton::RedrawHilited called on uninitialized object."));
		SetUpPort();
		EffectiveStart = ZeroPoint;
		EffectiveExtent = Extent;
		if ((KeyEquivalent == 0x0d) && (KeyModifiers == 0))
			{
				EffectiveStart.x += 4;
				EffectiveStart.y += 4;
				EffectiveExtent.x -= 10;
				EffectiveExtent.y -= 10;
			}
		Window->ResetPen();
		Window->SetText(0,0,srcBic,12,0);
		Window->LPaintRoundRect(EffectiveStart,EffectiveExtent,OvalRadius,OvalRadius);
		HLock(Name);
		Window->LDrawText(EffectiveStart,EffectiveExtent,Name,JustifyCenter);
		HUnlock(Name);
	}
