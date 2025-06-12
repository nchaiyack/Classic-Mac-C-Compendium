/* CYesNoBox.c */

#include "CYesNoBox.h"
#include "CImagePane.h"
#include "CStaticText.h"
#include "Memory.h"
#include "StringUtils.h"

#define YesButtonLoc (131L*65536L + 1)
#define NoButtonLoc (131L*65536L + 2)
#define TextBoxLoc (131L*65536L + 3)
#define WindowLoc (131L*65536L + 4)

#define KillButtonLocID (0x00810003)
#define ErrorPictID (136)
#define PictLocID (0x00820000) /* add PICT ID to find local index */


void				CYesButton::IYesButton(CWindow* TheModalDialog, MyBoolean* Result,
							Handle Text)
	{
		LongPoint		Start;
		LongPoint		Extent;

		AnswerLoc = Result;
		GetRect(YesButtonLoc,&Start,&Extent);
		ISimpleButton(Start,Extent,Text,13,0,TheModalDialog,TheModalDialog);
	}


MyBoolean		CYesButton::DoThang(void)
	{
		*AnswerLoc = True;
		delete Window;
		return True;
	}


/****************/


void				CNoButton::INoButton(CWindow* TheModalDialog, MyBoolean* Result,
							Handle Text)
	{
		LongPoint		Start;
		LongPoint		Extent;

		AnswerLoc = Result;
		GetRect(NoButtonLoc,&Start,&Extent);
		ISimpleButton(Start,Extent,Text,'.',cmdKey,TheModalDialog,TheModalDialog);
	}


MyBoolean		CNoButton::DoThang(void)
	{
		*AnswerLoc = False;
		delete Window;
		return True;
	}


/****************/


MyBoolean		CYesNoBox::ShouldIDoIt(long MessageID, Handle ExtraText, long YesTextID,
							long NoTextID)
	{
		CImagePane*			Icon;
		CStaticText*		StaticText;
		CYesButton*			YesButton;
		CNoButton*			NoButton;
		LongPoint				Start;
		LongPoint				Extent;
		Handle					MessageText;
		Handle					AccumulatedText;

		MyBoolean				Result;


		GetRect(WindowLoc,&Start,&Extent);
		Start = CenterRect(Extent,LongPointOf(screenBits.bounds.right
			- screenBits.bounds.left,screenBits.bounds.bottom - screenBits.bounds.top));
		IModalDialog(Start,Extent,DontAllowMenus);

		YesButton = new CYesButton;
		YesButton->IYesButton(this,&Result,GetCString(YesTextID));

		NoButton = new CNoButton;
		NoButton->INoButton(this,&Result,GetCString(NoTextID));

		if (MessageID != 0)
			{
				MessageText = GetCString(MessageID);
			}
		 else
			{
				MessageText = AllocHandle(0);
			}
		if (ExtraText == NIL)
			{
				ExtraText = AllocHandle(0);
			}

		BeginStringOperation();
		RegisterString(MessageText);
		RegisterString(ExtraText);
		AccumulatedText = ReplaceStr(MessageText,CString("_"),ExtraText);
		EndStringOperation(AccumulatedText);

		GetRect(TextBoxLoc,&Start,&Extent);
		StaticText = new CStaticText;
		StaticText->IStaticText(Start,Extent,AccumulatedText,0,12,this,this,JustifyLeft);

		GetRect(PictLocID + ErrorPictID,&Start,&Extent);
		Icon = new CImagePane;
		Icon->IImagePane(Start,Extent,this,this,ErrorPictID);

		DoEventLoop();
		return Result;
	}
