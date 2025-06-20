// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UProgress.cp

#include "UProgress.h"
#include "UThread.h"
#include "Tools.h"
#include "FontTools.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>
#include <ProgressGlobals.h>

#include <UDialogBehavior.h>
#include <UDialog.h>
#include <Resources.h>
#include <ToolUtils.h>
#include <Script.h>

const short kNoCandyStribes = 4;
const long kCandyTicks = 15;
const long kCandyTimeout = 4 * 60;

#define qDebugProgressText qDebug & 0
#define qDebugCandy qDebug & 0
#define qDebugProgressCreate qDebug & 0
#define qDebugAbortDisable qDebug & 0


#pragma segment MyThread

// Note: The Windows should not be free'd when they are closed 
// as this is managed by TProgressCache.


#define macroCheckIterator()													\
if (GetWindow() && GetWindow()->fIteratorPtr)					\
	fprintf(stderr, "Got fIteratorPtr at $%lx\n", long(fWindow->fIteratorPtr); \
	ProgramBreak(gEmptyString);

//------------------------- TProgressMeter --------------------------------

class TProgressMeterView : public TView
{
	public:
		void SetMax(long max);
		long GetMax();
		void SetValue(long value); // updates screen
		void DeltaValue(long delta); // updates screen
		long GetValue();
		
		pascal void Draw(const VRect &area);
		
		void UpdateCandy();
		
		TProgressMeterView();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		TWindow *fWindow;
		long fValue, fMax;
		long fLastCandyTick;
		short fCandyIndex;
		short fPixels;
};

TProgressMeterView::TProgressMeterView()
{
}

pascal void TProgressMeterView::Initialize()
{
	inherited::Initialize();
	fMax = kCandyStribes;
	fValue = 0;
	fPixels = 0;
	fCandyIndex = 0;
	fLastCandyTick = 0;
	fWindow = nil;
}

pascal void TProgressMeterView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TProgressMeterView::Free()
{
	inherited::Free();
}

pascal void TProgressMeterView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fWindow = GetWindow();
}

void TProgressMeterView::SetMax(long max)
{
	fLastCandyTick = 0;
	fValue = 0;
	if (max == kCandyStribes)
	{
		if (fMax != kCandyStribes)
		{
			// the candy stribes are too short for one-line progress windows, so erase unused area
			Focus();
			EraseRect(CRect(0, 0, short(fSize.h), short(fSize.v)));
		}
	}
	else
	{
		fValue = 0;
		fPixels = 0;
	}
	fMax = max;
	Focus();
	Draw(VRect(gZeroVPt, fSize));
}

long TProgressMeterView::GetMax()
{
	return fMax;
}

pascal void TProgressMeterView::Draw(const VRect& /* area */)
{
	if (fMax == kCandyStribes) // candy
	{
		Boolean gotColor = false;
		if (gConfiguration.hasColorQD)
		{
			VRect vr(VPoint(0, 0), fWindow->fSize);
			vr += fWindow->fLocation;
			GDHandle dev = GetMaxDevice(vr.ToRect());
			if (dev)
			{
				PixMapHandle pixMap = (**dev).gdPMap;
				gotColor = (**pixMap).pixelSize > 1;
			}
		}
		Handle h;
		if (gotColor)
			h = GetResource('PICT', kFirstProgressCandyPictID + fCandyIndex);
		else
			h = GetResource('PICT', kFirstProgressCandyBWPictID + fCandyIndex);
		FailNILResource(h);
		PicHandle ph = PicHandle(h);
		CRect r = (*ph)->picFrame;
		r -= r[topLeft];
		DrawPicture(ph, r);
		return;
	}
	CRect fr(0, 0, short(fSize.h), short(fSize.v));
	FrameRect(fr);
	CRect r(1, 1, fPixels, short(fSize.v) - 1);
	if (fPixels > 1)
	{
		RGBForeColor(CRGBColor(0x4000, 0x4000, 0x4000));
		FillRect(r, &qd.black);
	}
	MoveTo(r.right, r.top - 1);
	LineTo(r.right, r.bottom - 1);
	r.left = r.right + 1;
	r.right = short(fSize.h) - 1;
	if (r.right > r.left)
	{
		RGBForeColor(CRGBColor(0xCCCC, 0xCCCC, 0xFFFF));
		FillRect(r, &qd.white);
	}
	RGBForeColor(CRGBColor(0, 0, 0));
}

void TProgressMeterView::SetValue(long value)
{
	if (fMax == kCandyStribes)
		return;
	if (value > fMax)
		value = fMax;
	fValue = value;
	short newPixels = short(value * (fSize.h - 2) / fMax);
	if (newPixels == fPixels)
		return;
	fPixels = newPixels;
	Focus();
	Draw(VRect(gZeroVPt, fSize));
}

void TProgressMeterView::DeltaValue(long delta)
{
	SetValue(fValue + delta);
}

long TProgressMeterView::GetValue()
{
	return fValue;
}

void TProgressMeterView::UpdateCandy()
{
	long currTick = TickCount();
	if (currTick < fLastCandyTick + (kCandyTicks - 1)) 
		return; // don't miss one due to round-of error
	fCandyIndex++;
	if (fCandyIndex == kNoCandyStribes)
		fCandyIndex = 0;
	Focus();
	Draw(VRect(gZeroVPt, fSize));
	fLastCandyTick = currTick;
}

//--------------------- TProgressDialogView -------------------------------
class TProgressAbortButton : public TButton
{
	public:
		void SetProgressRef(TProgress *progress);

		virtual pascal void DoEvent(EventNumber eventNumber, 
							TEventHandler *source, TEvent *event);

		TProgressAbortButton();
		virtual pascal void Initialize();
	private:
		TProgress *fProgress;
};

TProgressAbortButton::TProgressAbortButton()
{
}

pascal void TProgressAbortButton::Initialize()
{
	inherited::Initialize();
	fProgress = nil;
}

void TProgressAbortButton::SetProgressRef(TProgress *progress)
{
	fProgress = progress;
}

pascal void TProgressAbortButton::DoEvent(EventNumber eventNumber, 
								TEventHandler *source, TEvent *event)
{
	inherited::DoEvent(eventNumber, source, event);
	if (fProgress && eventNumber == fEventNumber)
		fProgress->Abort();
}
//--------------------------- TProgress -----------------------------------

TProgress::TProgress()
{
}

pascal void TProgress::Initialize()
{
	inherited::Initialize();
	macroDontDeadStrip(TProgressMeterView);
	macroDontDeadStrip(TProgressAbortButton);
	fThread = nil;
	fWindow = nil;
	fDialogBehavior = nil;
	fProgressMeterView = nil;
	fUpperText = nil;
	fLowerText = nil;
	fAbortButton = nil;
	fIsWorking = false;
	fIsModal = false;
	fIsTwoLine = false;
	fCoHandlerInstalled = false;
	fLastWorkTick = 0;
	fIsCandy = true;
	fAbortIsDisabled = false;
}

void TProgress::IProgress()
{
	inherited::IObject();
#if qDebugProgressCreate
	fprintf(stderr, "TProgress created at $%lx\n", long(this));
#endif
	FailInfo fi;
	if (fi.Try())
	{
		CreateNewWindow();
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TProgress::Free()
{
#if qDebugProgressCreate
	fprintf(stderr, "TProgress free'd  at $%lx\n", long(this));
#endif
	RemoveIdleUpdating();
	if (fWindow)
		fWindow->CloseByUser();
	inherited::Free();
}

void TProgress::SetThread(TThread *thread)
{
#if qDebug
	if (thread && !IsObject(thread))
		ProgramBreak("thread is not object");
#endif
	fThread = thread;
}

void TProgress::SetStandardProgressType()
{
	SetProgressType(false, false);
}

void TProgress::SetProgressType(Boolean modal, Boolean twoLineWindow)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress at $%lx, SetProgressType: modal = %hd, twoLine = %hd\n", long(this), modal, twoLineWindow);
#endif
	fIsWorking = false;
	if (modal != fIsModal || twoLineWindow != fIsTwoLine)
	{
		if (fIsWorking)
			WorkDone();
		fIsModal = modal;
		fIsTwoLine = twoLineWindow;
		CreateNewWindow();
	}
	ClearTexts();
	SetAbortDisable(false);
}

Boolean TProgress::SetAbortDisable(Boolean disableAbort)
{
	if (fWindow && fDialogBehavior)
		fDialogBehavior->fDismissed = false;
	if (disableAbort == fAbortIsDisabled)
		return disableAbort;
	fAbortIsDisabled = disableAbort;
	FailInfo fi; // we have promised not to fail (maybe SetPermAlloc(false)???)
	if (fi.Try())
	{
		fAbortButton->DimState(disableAbort, kRedraw);
		fAbortButton->SetEnable(!disableAbort);
		fWindow->Update();
#if qDebugAbortDisable
		fprintf(stderr, "Changed abort disable for progress at $%lx: new state = %ld\n", long(this), long(disableAbort));
#endif
		fi.Success();
		return !disableAbort; // swapped state
	}
	else // fail
	{
#if qDebug
		ProgramBreak("Failed in TProgress::SetAbortDisable (but OK, just very unexpected)\n");
#endif
	}
}

void TProgress::CreateNewWindow()
{
	if (fWindow)
	{
		fWindow->CloseByUser();
		fWindow = nil;
	}
#if qDebug
#define macroCheckSubView(view, x) if (!view) ProgramBreak(x);
#else
#define macroCheckSubView(view, x) FailNonObject(view)
#endif
	CRect oldResize = gStandardWindowSizeRect;
	gStandardWindowSizeRect[topLeft] = CPoint(0, 0);
	short id;
	if (fIsModal)
		id = fIsTwoLine ? kModalProgressTwoLineView : kModalProgressOneLineView;
	else
		id = fIsTwoLine ? kProgressTwoLineView : kProgressOneLineView;
	fWindow = gViewServer->NewTemplateWindow(id, nil);
	macroCheckSubView(fWindow, "did not find subview: window");
	gStandardWindowSizeRect = oldResize;

	fProgressMeterView = (TProgressMeterView*)fWindow->FindSubView(kProgressMeter);
	macroCheckSubView(fProgressMeterView, "did not find subview: kProgressMeter");

	fUpperText = (TStaticText*)fWindow->FindSubView(kProgressUpperText);
	macroCheckSubView(fUpperText, "did not find subview: kProgressUpperText");

	if (fIsTwoLine)
	{
		fLowerText = (TStaticText*)fWindow->FindSubView(kProgressLowerText);
		macroCheckSubView(fLowerText, "did not find subview: kProgressLowerText");
	}
	else
		fLowerText = nil;

	fAbortButton = (TProgressAbortButton*)fWindow->FindSubView('cncl');
	macroCheckSubView(fAbortButton, "did not find subview: cncl");
	fAbortButton->SetProgressRef(this);

#if 1
	fWindow->SetDialogItems(kNoIdentifier, fAbortButton->fIdentifier);
	fDialogBehavior = fWindow->GetDialogBehavior();
	fDialogBehavior->fDismissed = false;
#else
	TDialogBehavior *dbe = new TDialogBehavior();
	dbe->IDialogBehavior(fIsModal, kNoIdentifier, fAbortButton->fIdentifier);
	fWindow->AddBehavior(dbe);
	fDialogBehavior = dbe;
#endif
}

//...................................................................... Housekeeping
void TProgress::StartProgress(Boolean windowToFront)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::StartProgress at $%lx, windowToFront = %hd\n", long(this), windowToFront);
#endif
	gApplication->UpdateAllWindows();
	fIsWorking = true;
	if (windowToFront)
		fWindow->Select();
	fWindow->Open();
	fWindow->Show(true, kRedraw);
	gApplication->UpdateAllWindows();
	InvalidateMenus(); // for windows menu
	fLastWorkTick = TickCount();
}

void TProgress::UpdateCandyState()
{
	if (fIsCandy)
	{
		fLastWorkTick = TickCount();
		InstallIdleUpdating();
		SetIdleFreq(kCandyTicks);
	}
	else
	{
		RemoveIdleUpdating();
		SetIdleFreq(kMaxIdleTime);
	}
}

void TProgress::SetWorkToDo(long todo)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::SetWorkToDo at $%lx, todo = %ld\n", long(this), todo);
#endif
	fIsCandy = (todo == kCandyStribes);
	UpdateCandyState();
	fProgressMeterView->SetMax(todo);
}

void TProgress::WorkDone()
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::WorkDone at $%lx\n", long(this));
#endif
	if (!fIsWorking)
		return;
	FailInfo fi; // we have promised not to fail (maybe SetPermAlloc(false)???)
	if (fi.Try())
	{
		RemoveIdleUpdating();
		SetIdleFreq(kMaxIdleTime);
		fIsWorking = false;
#if qDebug
		if (!VerboseIsObject(fWindow))
			ProgramBreak("fWindow is not object");
#endif
		fWindow->Show(false, kDontRedraw);
		fi.Success();
	}
	else // fail
	{
#if qDebug
		ProgramBreak("Fail in TProgress::WorkDone (OK, but very unexpected");
#endif
		fi.ReSignal();
	}
}

void TProgress::Worked(long delta)
{
	if (!fIsWorking)
	{
#if qDebug & 0
		ProgramBreak("Worked called for non-working TProgress");
#endif
		return;
	}
	if (fIsCandy)
		UpdateCandyState();
	else
		fProgressMeterView->DeltaValue(delta);
}

long TProgress::GetWork()
{
	if (!fIsWorking)
	{
#if qDebug
		ProgramBreak("GetWork called for non-working TProgress");
#endif
	}
	return fProgressMeterView->GetValue();
}

void TProgress::SetWorkDone(long work)
{
#if 0
	fprintf(stderr, "TProgress::SetWorkDone at $%lx, work = %ld\n", long(this), work);
#endif
	if (!fIsWorking)
	{
#if qDebug & 0
		ProgramBreak("SetWorkDone called for non-working TProgress");
#endif
		return;
	}
	UpdateCandyState();
	fProgressMeterView->SetValue(work);
}

long TProgress::GetWorkDone()
{
	return fProgressMeterView->GetValue();
}

pascal Boolean TProgress::DoIdle(IdlePhase phase)
{
	fProgressMeterView->UpdateCandy();
	if (fCoHandlerInstalled && TickCount() - fLastWorkTick > kCandyTimeout)
		RemoveIdleUpdating();
	return inherited::DoIdle(phase);
}

void TProgress::InstallIdleUpdating()
{
	if (!fCoHandlerInstalled)
	{
#if qDebugCandy
		fprintf(stderr, "TProgress at $%lx, Installs cohandler\n", long(this));
#endif
		gApplication->InstallCohandler(this, true);
		fCoHandlerInstalled = true;
	}
}

void TProgress::RemoveIdleUpdating()
{
	if (fCoHandlerInstalled)
	{
#if qDebugCandy
		fprintf(stderr, "TProgress at $%lx, removes cohandler\n", long(this));
#endif
		gApplication->InstallCohandler(this, false);
		fCoHandlerInstalled = false;
	}
}

//......................................................... Texts
void TruncProgressText(TStaticText *st, CStr255 &s)
{
	GrafPtr oldPort;
	GetPort(oldPort);
	SetPort(gWorkPort);
	TextStyle ts = st->fTextStyle;
	SetPortTextStyle(ts);
	TruncString(short(st->fSize.h), s, smTruncEnd);
	SetPort(oldPort);
}

void TProgress::SetUpperText(const CStr255 &msg)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::SetUpperText at $%lx, text = '%s'\n", long(this), (char*)msg);
#endif
	if (fUpperText)
	{
		CStr255 text(msg);
		TruncProgressText(fUpperText, text);
		fUpperText->SetText(text, kRedraw);
	}
	else
	{
#if qDebug
		fprintf(stderr, "TProgress::SetUpperText at $%lx called when fUpperText == nil\n", long(this));
#endif
	}
	if (fIsWorking)
		fWindow->Update();
}

void TProgress::SetUpperText(short id)
{
	CStr255 s;
	MyGetIndString(s, id);
	SetUpperText(s);
}

void TProgress::SetLowerText(const CStr255 &msg)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::SetLowerText at $%lx, text = '%s'\n", long(this), (char*)msg);
#endif
	if (fLowerText)
	{
		CStr255 text(msg);
		TruncProgressText(fLowerText, text);
		fLowerText->SetText(text, kRedraw);
	}
	else
	{
#if qDebug
		fprintf(stderr, "TProgress::SetLowerText at $%lx called when fLowerText == nil\n", long(this));
#endif
	}
	if (fIsWorking)
		fWindow->Update();
}

void TProgress::SetLowerText(short id)
{
	CStr255 s;
	MyGetIndString(s, id);
	SetLowerText(s);
}

void TProgress::SetText(const CStr255 &msg)
{
	if (fIsTwoLine)
		SetLowerText(msg);
	else
		SetUpperText(msg);
}

void TProgress::SetText(short id)
{
	if (fIsTwoLine)
		SetLowerText(id);
	else
		SetUpperText(id);
}

void TProgress::ClearTexts()
{
	if (fUpperText)
		fUpperText->SetText(gEmptyString, kRedraw);
	if (fLowerText)
		fLowerText->SetText(gEmptyString, kRedraw);
	if (fIsWorking)
		fWindow->Update();
}

void TProgress::SetTitle(const CStr255 &title)
{
#if qDebugProgressText
	fprintf(stderr, "TProgress::SetTitle at $%lx, title = '%s'\n", long(this), (char*)title);
#endif
	if (fWindow)
	{
		CStr255 text(title);
		TruncSystemFontString(short(fWindow->fSize.h - 64), text, smTruncEnd);
		fWindow->SetTitle(text);
	}
	else
	{
#if qDebug
		fprintf(stderr, "TProgress::SetTitle at $%lx called when fWindow == nil\n", long(this));
#endif
	}
}

void TProgress::SetTitle(short id)
{
	CStr255 s;
	MyGetIndString(s, id);
	SetTitle(s);
}

void TProgress::Abort()
{
	if (fWindow && fDialogBehavior)
		fDialogBehavior->fDismissed = false;
	if (!fIsWorking)
	{
#if qDebug
		ProgramBreak("Abort called for non-working TProgress");
#endif
		return;
	}
	if (fAbortIsDisabled)
		PanicExitToShell("Abort called for TProgress with fAbortIsDisabled == TRUE");
#if qDebug
	fprintf(stderr, "TProgress at %lx: aborting\n", long(this));
#endif
	if (fThread)
		fThread->Abort();
	else
	{
#if qDebug
		ProgramBreak("Abort called for TProgress without a thread");
#endif
		Failure(minErr, 0);
	}
}

TWindow *TProgress::GetWindow()
{
	if (fWindow)
		return fWindow;
	else
		PanicExitToShell("TProgress::GetWindow called when fWindow == nil");
}

Boolean TProgress::IsWorking()
{
	return fIsWorking;
}
