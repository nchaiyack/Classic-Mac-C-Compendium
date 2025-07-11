// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UTextScroller.cp

#include "UTextScroller.h"

#pragma segment MyViewTools

//----------------------------------------------------------------
class TSuperScrollBar : public TScrollerScrollBar {
	public:
		virtual pascal void SetLongVal(VCoordinate itsVal, Boolean redraw);
		virtual pascal void DeltaValue(VCoordinate delta);
};

pascal void TSuperScrollBar::SetLongVal(VCoordinate itsVal,
								Boolean redraw)
{
	itsVal = Max(fLongMin, itsVal);
	if (itsVal != fLongVal)
	{
		fLongVal = itsVal;
		itsVal = Min(fLongVal, fLongMax); // postpone max-check
		this->SetVal((short)(itsVal >> fBitsToShift), redraw);
	}
}

pascal void TSuperScrollBar::DeltaValue(VCoordinate delta)
{
	if (delta != 0)
		this->SetLongVal(fLongVal, TRUE); // SetLongVal does index check
}


//----------------------------------------------------------------

TTextScroller::TTextScroller()
{
}

pascal void TTextScroller::Initialize()
{
	inherited::Initialize();
}

pascal void TTextScroller::IRes(TDocument *itsDocument,
							 TView *itsSuperView,
							 TStream *itsParams)
{
	inherited::IRes(itsDocument, itsSuperView, itsParams);
}

pascal void TTextScroller::Free()
{
	inherited::Free();
}

pascal void TTextScroller::CreateScrollBar(VHSelect itsDirection)
{
	if (itsDirection == hSel)
	{
		inherited::CreateScrollBar(itsDirection);
		return;
	}
	FailInfo fi;
	if (fi.Try())
	{
		TSuperScrollBar *ssb = new TSuperScrollBar;
		ssb->IScrollerScrollBar(fSuperView, gZeroVPt, gZeroVPt, 
				sizeVariable, sizeVariable, itsDirection, 
				fMaxTranslation[itsDirection], this);
		fi.Success();
	}
	else	// Recover
	{
		this->Free();
		fi.ReSignal();
	}
}

void TTextScroller::DoScrollUp()
{
	gApplication->InvalidateHelpRgn();
	ScrollBy(VPoint(0, -fScrollUnit.v), kRedraw);
	Update();
}

void TTextScroller::DoScrollDown()
{
	gApplication->InvalidateHelpRgn();
	TScrollerScrollBar *sb = fScrollBars[vSel];
	if (sb->fLongVal >= sb->fLongMax)
		return;
	ScrollBy(VPoint(0, fScrollUnit.v), kRedraw);
	Update();
}

void TTextScroller::DoPageUp()
{
	gApplication->InvalidateHelpRgn();
	fScrollBars[vSel]->DeltaValue(VCoordinate(ScrollStep(vSel, inPageUp)));
	Update();
}

void TTextScroller::DoPageDown()
{
	gApplication->InvalidateHelpRgn();
	TScrollerScrollBar *sb = fScrollBars[vSel];
	if (sb->fLongVal >= sb->fLongMax)
		return;
	fScrollBars[vSel]->DeltaValue(VCoordinate(ScrollStep(vSel, inPageDown)));
	Update();
}

void TTextScroller::DoGoHome()
{
	gApplication->InvalidateHelpRgn();
	SetLocalOrigin(gZeroVPt, kRedraw);
	Update();
}

void TTextScroller::DoGoEnd()
{
	gApplication->InvalidateHelpRgn();
	SetLocalOrigin(VPoint(0, fMaxTranslation.Copy().v), kRedraw);
	Update();
}				

pascal void TTextScroller::DoKeyEvent(TToolboxEvent *event)
{
	if (!IsEnabled() || !fRespondsToFunctionKeys)
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chUp:
			if (!event->IsOptionKeyPressed())
				DoScrollUp();
			break;
			
		case chDown:
			if (!event->IsOptionKeyPressed())
				DoScrollDown();
			break;

		case chPageUp:
			DoPageUp();
			break;
			
		case chPageDown:
			DoPageDown();
			break;
			
		case chHome:
			DoGoHome();
			break;				
		case chEnd:
			DoGoEnd();
			break;
			
		default:
			inherited::DoKeyEvent(event);
			break;
	}
}

pascal void TTextScroller::DoCommandKeyEvent(TToolboxEvent *event)
{
	if (!IsEnabled() || !fRespondsToFunctionKeys)
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chUp:
			if (event->IsCommandKeyPressed() && event->IsOptionKeyPressed())
				DoGoHome();
			else if (event->IsCommandKeyPressed() && !event->IsOptionKeyPressed())
				DoPageUp();
			else
				inherited::DoCommandKeyEvent(event);
			break;

		case chDown:
			if (event->IsCommandKeyPressed() && event->IsOptionKeyPressed())
				DoGoEnd();
			else if (event->IsCommandKeyPressed() && !event->IsOptionKeyPressed())
				DoPageDown();
			else
				inherited::DoCommandKeyEvent(event);
			break;

		default:
			inherited::DoCommandKeyEvent(event);
	}
}

pascal VCoordinate TTextScroller::ScrollStep(VHSelect vhs,
										 short partCode)
{
	short scrollUnit = (short)fScrollUnit[vhs];
	VCoordinate delta;
	VCoordinate adjustment;
	VPoint deltaPt = gZeroVPt;

	switch (partCode)
	{
		case inUpButton:
		case inDownButton:
			delta = scrollUnit;
			break;
			
		case inPageUp:
		case inPageDown:
			delta = fSize[vhs] - scrollUnit;
			break;
			
#if qDebug
		default:
			{
			CStr255 theString;
			ConcatNumber("TScroller::ScrollStep: bad part code =", partCode, theString);
			ProgramBreak(theString);
			break;
			}
#endif

	}

	if ((partCode == inUpButton) || (partCode == inPageUp))
		delta = -delta;

	// Constrain if necessary 
	if (fConstrain[vhs] && (scrollUnit != 0))
	{
		adjustment = (Max(0, fTranslation[vhs] + delta)) % scrollUnit;
		if (adjustment != 0)
			if (delta > 0)
				delta -= adjustment;
			else
				delta += (scrollUnit - adjustment);
	}
	deltaPt[vhs] = delta;

	DoScroll(deltaPt, kRedraw);
	return delta;
}


pascal void TTextScroller::DoScroll(const VPoint &delta,
								Boolean redraw)
{
	VPoint localDelta(delta);

	for (VHSelect vhs = vSel; vhs <= hSel; ++vhs)
	{
		if (localDelta[vhs] < 0)
		{
			// !!!FIX the following 2 lines after MPW C compiler bug is corrected.
			long temp = Max(localDelta[vhs], -fTranslation[vhs]);
			localDelta[vhs] = temp;
		}
		else if (localDelta[vhs] > 0)
		{
			// !!!FIX the following 2 lines after MPW C compiler bug is corrected.
			long temp1 = Min(localDelta[vhs], (fMaxTranslation[vhs] /* - fTranslation[vhs] */));
			localDelta[vhs] = temp1;
		}
		fTranslation[vhs] += localDelta[vhs];
	}

	if (localDelta != gZeroVPt)
	{
		this->InvalidateFocus();
		this->UpdateCoordinates();

		// !!! In the future the decision will be made differently as to how to render
		if (redraw)
			this->ScrollDraw(localDelta, kInvalidate);
	}
}
