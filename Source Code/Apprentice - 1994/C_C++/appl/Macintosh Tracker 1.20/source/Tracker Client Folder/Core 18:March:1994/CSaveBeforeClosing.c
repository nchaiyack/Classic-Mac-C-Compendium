/* CSaveBeforeClosing.c */

#include "CSaveBeforeClosing.h"
#include "CImagePane.h"
#include "CStaticText.h"
#include "Memory.h"
#include "StringUtils.h"


#define SaveBeforeClosingWindowLocID (140L*65536L + 1)
#define YesSaveButtonLocID (140L*65536L + 2)
#define NoSaveButtonLocID (140L*65536L + 3)
#define CancelButtonLocID (140L*65536L + 4)
#define SaveBeforeClosingTextLoc (140L*65536L + 5)
#define SaveBeforeClosingPictID (136)
#define PictLocID (130L*65536L + SaveBeforeClosingPictID)

#define YesSaveButtonTextID (140L*65536L + 2)
#define NoSaveButtonTextID (140L*65536L + 3)
#define CancelButtonTextID (140L*65536L + 4)
#define SaveBeforeClosingTextID (140L*65536L + 5)


short			CSaveBeforeClosingWindow::SaveBeforeClosing(PString Name)
	{
		CAskButton*		Button;
		CStaticText*	Text;
		CImagePane*		PaneInTheButt;
		MyBoolean			YesSave;
		MyBoolean			NoSave;
		MyBoolean			Cancelled;
		LongPoint			Start;
		LongPoint			Extent;
		Handle				Temp1;
		Handle				Temp2;
		Handle				Temp3;

		APRINT(("+CSaveBeforeClosingWindow::SaveBeforeClosing"));
		GetRect(SaveBeforeClosingWindowLocID,&Start,&Extent);
		IModalDialog(CenterRect(Extent,MainScreenSize()),Extent,DontAllowMenus);

		GetRect(SaveBeforeClosingTextLoc,&Start,&Extent);
		Text = new CStaticText;
		BeginStringOperation();
		Temp1 = GetCString(SaveBeforeClosingTextID);
		RegisterString(Temp1);
		Temp2 = PString2Handle(Name);
		RegisterString(Temp2);
		Temp3 = PString2Handle("\p_");
		RegisterString(Temp3);
		Temp1 = ReplaceStr(Temp1,Temp3,Temp2);
		EndStringOperation(Temp1);
		Text->IStaticText(Start,Extent,Temp1,systemFont,12,this,this,JustifyLeft);

		GetRect(PictLocID,&Start,&Extent);
		PaneInTheButt = new CImagePane;
		PaneInTheButt->IImagePane(Start,Extent,this,this,SaveBeforeClosingPictID);

		GetRect(YesSaveButtonLocID,&Start,&Extent);
		Button = new CAskButton;
		Button->IAskButton(Start,Extent,GetCString(YesSaveButtonTextID),0x0d,0,
			this,this,&YesSave);

		GetRect(NoSaveButtonLocID,&Start,&Extent);
		Button = new CAskButton;
		Button->IAskButton(Start,Extent,GetCString(NoSaveButtonTextID),0,0,
			this,this,&NoSave);

		GetRect(CancelButtonLocID,&Start,&Extent);
		Button = new CAskButton;
		Button->IAskButton(Start,Extent,GetCString(CancelButtonTextID),'.',cmdKey,
			this,this,&Cancelled);

		YesSave = False;
		NoSave = False;
		Cancelled = False;
		DoEventLoop();
		APRINT(("-CSaveBeforeClosingWindow::SaveBeforeClosing"));
		if (YesSave) {return Yes_Save;}
		if (NoSave) {return No_Save;}
		if (Cancelled) {return Cancel_Close;}
		EXECUTE(PRERR(AllowResume,"CSaveBeforeClosingWindow::SaveBeforeClosing "
			"Received no result from it's buttons."));
		EXECUTE(return Cancel_Close);
	}


void			CAskButton::IAskButton(LongPoint Start, LongPoint Extent, Handle NameString,
						char Key, short Modifiers, CWindow* TheWindow, CEnclosure* TheEnclosure,
						MyBoolean* TheAnswerLoc)
	{
		APRINT(("+CAskButton::IAskButton"));
		AnswerLoc = TheAnswerLoc;
		ISimpleButton(Start,Extent,NameString,Key,Modifiers,TheWindow,TheEnclosure);
		APRINT(("-CAskButton::IAskButton"));
	}


MyBoolean	CAskButton::DoThang(void)
	{
		APRINT(("+CAskButton::DoThang"));
		(*AnswerLoc) = True; /* export result */
		delete Window; /* make window go away */
		APRINT(("-CAskButton::DoThang"));
		return True;
	}
