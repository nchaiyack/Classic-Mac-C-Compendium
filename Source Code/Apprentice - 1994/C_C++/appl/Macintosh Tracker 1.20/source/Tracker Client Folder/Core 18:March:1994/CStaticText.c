/* CStaticText.c */

#include "CStaticText.h"
#include "CWindow.h"
#include "Memory.h"


/* */		CStaticText::~CStaticText()
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CStaticText::~CStaticText called on uninitialized object."));
		ReleaseHandle(Text);
	}


void		CStaticText::IStaticText(LongPoint Start, LongPoint Extent, Handle DaText,
					short TheFontID, short ThePointSize, CWindow* TheWindow,
					CEnclosure* TheEnclosure, short TheJustification)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CStaticText::IStaticText called on already initialized object."));
		EXECUTE(Initialized = True);
		Text = DaText;
		ERROR(DaText==NIL,PRERR(ForceAbort,"CStaticText::IStaticText passed NIL for text."));
		FontID = TheFontID;
		PointSize = ThePointSize;
		Justification = TheJustification;
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
	}


void		CStaticText::DoUpdate(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CStaticText::DoUpdate called on uninitialized object."));
		SetUpPort();
		Window->ResetPen();
		Window->SetText(FontID,0,srcOr,PointSize,0);
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		ERROR(Text==NIL,PRERR(ForceAbort,"CStaticText::DoUpdate Text handle is NIL."));
		HLock(Text);
		Window->LTextBox(ZeroPoint,Extent,Text,Justification);
		HUnlock(Text);
	}


void		CStaticText::DoDisable(void)
	{
		inherited::DoDisable();
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
	}


void		CStaticText::DoEnable(void)
	{
		inherited::DoEnable();
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
	}
