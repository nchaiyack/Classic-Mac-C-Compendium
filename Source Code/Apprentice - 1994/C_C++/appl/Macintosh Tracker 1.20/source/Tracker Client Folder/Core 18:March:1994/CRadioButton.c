/* CRadioButton.c */

#include "CRadioButton.h"
#include "CSack.h"
#include "CWindow.h"
#include "Memory.h"

/* */			CRadioButton::~CRadioButton()
	{
		CRadioButton*		Temp;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CRadioButton::~CRadioButton called on uninitialized object."));
		ReleaseHandle(Name);
		Temp = this;
		RadioGroup->KillElement(&Temp);
		if (RadioGroup->NumElements() == 0)
			{
				delete RadioGroup;
			}
	}


void			CRadioButton::IRadioButton(LongPoint Start, LongPoint Extent,
						Handle NameString, char Key, short Modifiers, CSack** CurrentRadioGroup,
						short TheFontID, short ThePointSize, CWindow* TheWindow,
						CEnclosure* TheEnclosure)
	{
		CRadioButton*	Temp;

		ERROR(Initialized == True,PRERR(ForceAbort,
			"CRadioButton::IRadioButton called on already initialized object."));
		EXECUTE(Initialized = True);
		FontID = TheFontID;
		PointSize = ThePointSize;
		Name = NameString;
		ERROR(NameString==NIL,PRERR(ForceAbort,
			"CRadioButton::IRadioButton passed NIL for name."));
		IButton(Start,Extent,Key,Modifiers,TheWindow,TheEnclosure);
		State = False;
		if (*CurrentRadioGroup == NIL)
			{
				*CurrentRadioGroup = new CSack; /* if this is the first group member, then */
				(*CurrentRadioGroup)->ISack(sizeof(CRadioButton*),128); /* make new list */
			}
		RadioGroup = *CurrentRadioGroup; /* remember our associates */
		Temp = this;
		RadioGroup->PushElement(&Temp); /* add ourselves to the list */
	}


void			CRadioButton::RedrawNormal(void)
	{
		long		Top,Left,Right,Bottom;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CRadioButton::RedrawNormal called on uninitialized object."));
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
		Window->LEraseRect(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
		Window->LFrameOval(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
		if (State)
			{
				Top += 3;
				Left += 3;
				Bottom -= 6;
				Right -= 6;
				Window->LPaintOval(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
			}
		Window->SetText(FontID,0,srcOr,PointSize,0);
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		HLock(Name);
		Window->LTextBox(LongPointOf(12+6,0),LongPointOf(Extent.x-(12+6),Extent.y),
			Name,JustifyLeft);
		HUnlock(Name);
	}


void			CRadioButton::RedrawHilited(void)
	{
		long		Top,Left,Right,Bottom;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CRadioButton::RedrawHilited called on uninitialized object."));
		SetUpPort();
		Window->ResetPen();
		Left = 0;
		Top = (Extent.y / 2) - 6;
		Right = 12;
		Bottom = 12;
		Window->LEraseRect(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
		Window->LFrameOval(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
		Top += 1;
		Left += 1;
		Right -= 2;
		Bottom -= 2;
		Window->LFrameOval(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
		if (State)
			{
				Top += 2;
				Left += 2;
				Bottom -= 4;
				Right -= 4;
				Window->LPaintOval(LongPointOf(Left,Top),LongPointOf(Right,Bottom));
			}
		Window->SetText(FontID,0,srcOr,PointSize,0);
		HLock(Name);
		Window->LTextBox(LongPointOf(12+6,0),LongPointOf(Extent.x-(12+6),Extent.y),Name,JustifyLeft);
		HUnlock(Name);
	}


MyBoolean	CRadioButton::DoThang(void)
	{
		CRadioButton*	Temp;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CRadioButton::DoThang called on uninitialized object."));
		ERROR(RadioGroup==NIL,PRERR(ForceAbort,
			"CRadioButton::DoThang RadioGroup is NIL."));
		RadioGroup->ResetScan();
		while (RadioGroup->GetNext(&Temp))
			{
				if (Temp != this)
					{
						Temp->Uncheck(); /* implicit Temp->ClearState() call here. */
					}
			}
		State = True;
		SetState();
		return False;
	}


void			CRadioButton::Uncheck(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CRadioButton::Uncheck called on uninitialized object."));
		if (State != False)
			{
				ClearState();
				State = False;
				RedrawNormal();
			}
	}


/* the following two functions are to be overridden to do anything you need to do, such */
/* as maintaining a shared status flag elsewhere.  Note, ClearState always called before */
/* SetState, so that only zero or one, but not more states are ever in effect. */
void			CRadioButton::SetState(void)
	{
	}


void			CRadioButton::ClearState(void)
	{
	}
