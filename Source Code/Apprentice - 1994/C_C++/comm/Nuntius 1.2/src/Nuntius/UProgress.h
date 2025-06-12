// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UProgress.h

#define __UPROGRESS__

class TProgressMeterView;
class TStaticText;
class TProgressMeterBehavior;
class TProgressAbortButton;

const long kCandyStribes = 0;
class TThread;

class TProgress : public TEventHandler 
{
	public:
		void SetStandardProgressType();
		void SetProgressType(Boolean modal, Boolean twoLineWindow);

		Boolean SetAbortDisable(Boolean disableAbort);
		 // reset with SetProgressWindowType, returns old state, cannot fail
		
		void StartProgress(Boolean windowToFront);
		void SetWorkToDo(long todo);// uses stribes if (todo == kCandyStribes)
		// work here
		void WorkDone(); // hides window, cannot fail
		
		void Worked(long delta = 0); // Does Failure(0, 0) if aborting
		long GetWork();
		void SetWorkDone(long work);
		long GetWorkDone();
		Boolean IsWorking();
		
		void Abort();
		
		TWindow *GetWindow();
		void SetUpperText(const CStr255 &msg);
		void SetUpperText(short id);

		void SetLowerText(const CStr255 &msg);
		void SetLowerText(short id);

		void SetText(short id); // upper if one line, lower if two line
		void SetText(const CStr255 &msg);
		void ClearTexts();
		
		void SetTitle(const CStr255 &title);
		void SetTitle(short id);
		
		pascal Boolean DoIdle(IdlePhase phase);
		void SetThread(TThread *thread = nil);

		TProgress();
		pascal void Initialize();
		void IProgress();
		pascal void Free();
	protected:
		TThread *fThread;
		TWindow *fWindow;
		TDialogBehavior *fDialogBehavior;
		TProgressMeterView *fProgressMeterView;
		TStaticText *fUpperText;
		TStaticText *fLowerText;
		TProgressAbortButton *fAbortButton;
		long fLastWorkTick;
		Boolean fIsWorking;
		Boolean fIsModal;
		Boolean fIsTwoLine;
		Boolean fCoHandlerInstalled;
		Boolean fIsCandy;
		Boolean fAbortIsDisabled;

		void CreateNewWindow();
		void UpdateCandyState();
		void InstallIdleUpdating();
		void RemoveIdleUpdating();
};

extern TProgress *gCurProgress; // see TThread
