/* Alert.c */

#include "Alert.h"
#include "CModalDialog.h"
#include "CSimpleButton.h"
#include "Memory.h"
#include "CImagePane.h"
#include "CStaticText.h"
#include "StringUtils.h"


#define AlertWindowLocID (0x00810001)
#define KillButtonLocID (0x00810003)
#define TextLocID (0x00810004)
#define PictLocID (0x00820000) /* add PICT ID to find local index */

#define KillButtonNameID (0x00810001)

#define ErrorPictID (136)


/********************************************************************************/

struct	CAlertDialog	:	CModalDialog
	{
		void		DoEventLoop(void);
	};


void		CAlertDialog::DoEventLoop(void)
	{
		FlushEvents(everyEvent,0);
		inherited::DoEventLoop();
	}


/********************************************************************************/

struct	CKillButton	:	CSimpleButton
	{
		void			IKillButton(CModalDialog* TheDialog);
		MyBoolean	DoThang(void);
	};


void		CKillButton::IKillButton(CModalDialog* TheDialog)
	{
		LongPoint	Start;
		LongPoint	Extent;

		GetRect(KillButtonLocID,&Start,&Extent);
		ISimpleButton(Start,Extent,GetCString(KillButtonNameID),0x0d,0,TheDialog,TheDialog);
	}


MyBoolean	CKillButton::DoThang(void)
	{
		delete Window;
		return True;
	}


/********************************************************************************/


void		AlertError(ulong MessageID, Handle ExtraText)
	{
		CModalDialog*		AlertWindow;
		CKillButton*		Button;
		CImagePane*			Icon;
		CStaticText*		StaticText;
		LongPoint				Start;
		LongPoint				Extent;
		Handle					MessageText;
		Handle					AccumulatedText;


		StackSizeTest();
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

		GetRect(AlertWindowLocID,&Start,&Extent);
		Start = AlertCenterRect(Extent,LongPointOf(screenBits.bounds.right
			- screenBits.bounds.left,screenBits.bounds.bottom - screenBits.bounds.top));
		AlertWindow = new CAlertDialog;
		AlertWindow->IModalDialog(Start,Extent,DontAllowMenus);

		GetRect(TextLocID,&Start,&Extent);
		StaticText = new CStaticText;
		StaticText->IStaticText(Start,Extent,AccumulatedText,0,12,AlertWindow,
			AlertWindow,JustifyLeft);

		GetRect(PictLocID + ErrorPictID,&Start,&Extent);
		Icon = new CImagePane;
		Icon->IImagePane(Start,Extent,AlertWindow,AlertWindow,ErrorPictID);

		Button = new CKillButton;
		Button->IKillButton(AlertWindow);

		SysBeep(20);
		AlertWindow->DoEventLoop();
	}
