/* CCheckbox.c */

#include "CCheckbox.h"
#include "CWindow.h"
#include "Memory.h"


/* */			CCheckbox::~CCheckbox()
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CCheckbox::~CCheckbox called on uninitialized object."));
		ReleaseHandle(Name);
	}


void			CCheckbox::ICheckbox(LongPoint Start, LongPoint Extent, Handle NameString,
						char Key, short Modifiers, short TheFontID, short ThePointSize,
						CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CCheckbox::ICheckbox called on already initialized object."));
		EXECUTE(Initialized = True);
		Name = NameString;
		ERROR(NameString==NIL,PRERR(ForceAbort,"CCheckbox::ICheckbox passed NIL for name."));
		IButton(Start,Extent,Key,Modifiers,TheWindow,TheEnclosure);
		State = False;
		FontID = TheFontID;
		PointSize = ThePointSize;
	}


void			CCheckbox::RedrawNormal(void)
	{
		long		Top,Left,Right,Bottom;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CCheckbox::RedrawNormal called on uninitialized object."));
		SetUpPort();
		Window->ResetPen();
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		Left = 0;
		Top = (Extent.y / 2) - 6;
		Right = 12;
		Bottom = 12;
		Window->LEraseRect(LongPointOf(Left + 1,Top + 1),LongPointOf(Right - 1,Bottom - 1));
		Window->LFrameRect(LongPointOf(Left,Top),LongPointOf(Right + 1,Bottom + 1));
		if (State)
			{
				Window->DrawLine(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
				Window->DrawLine(LongPointOf(Left,Top + Bottom),LongPointOf(Right,-Bottom));
			}
		Window->SetText(FontID,0,srcOr,PointSize,0);
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		Window->LTextBox(LongPointOf(12+6,0),LongPointOf(Extent.x-(12+6),Extent.y),
			Name,JustifyLeft);
	}


void			CCheckbox::RedrawHilited(void)
	{
		long		Top,Left,Right,Bottom;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CCheckbox::RedrawHilited called on uninitialized object."));
		SetUpPort();
		Window->ResetPen();
		Left = 0;
		Top = (Extent.y / 2) - 6;
		Right = 12;
		Bottom = 12;
		Window->LEraseRect(LongPointOf(Left + 2,Top + 2),LongPointOf(Right - 3,Bottom - 3));
		Window->LFrameRect(LongPointOf(Left,Top),LongPointOf(Right + 1,Bottom + 1));
		Window->LFrameRect(LongPointOf(Left + 1,Top + 1),LongPointOf(Right - 1,Bottom - 1));
		if (State)
			{
				Window->DrawLine(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
				Window->DrawLine(LongPointOf(Left,Top + Bottom),LongPointOf(Right,-Bottom));
			}
		Window->SetText(FontID,0,srcOr,PointSize,0);
		Window->LTextBox(LongPointOf(12+6,0),LongPointOf(Extent.x-(12+6),Extent.y),
			Name,JustifyLeft);
	}


MyBoolean	CCheckbox::DoThang(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CCheckbox::DoThang called on uninitialized object."));
		State = !State;
		return False;
	}
