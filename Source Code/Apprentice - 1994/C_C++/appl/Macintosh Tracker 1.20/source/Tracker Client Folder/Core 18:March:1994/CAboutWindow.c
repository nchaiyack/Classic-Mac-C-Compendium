/* CAboutWindow.c */

#include "CAboutWindow.h"
#include "EventLoop.h"
#include "Memory.h"
#include "CMyApplication.h"
#include "MenuController.h"


#define ApplicationVersionStringID (0x00800003)
#define AboutWindowLocationID (0x00800001)
#define ApplicationVersionLocationID (0x00800003)
#define PictureLocationID (0x00800004)
#define ShowDelay (60*5)
#define PictureID 128

#define DevelopmentRelease (0x20)
#define AlphaRelease (0x40)
#define BetaRelease (0x60)
#define FinalRelease (0x80)
#define DevelopmentStringID (0x00800004)
#define AlphaStringID (0x00800005)
#define BetaStringID (0x00800006)
#define NonFinalLocID (0x00800005)


/* */			CAboutWindow::~CAboutWindow()
	{
		DeregisterIdler(this);
		Application->AboutWindow = NIL; /* indicate we are gone */
	}


void			CAboutWindow::IAboutWindow(MyBoolean TheAutoFlag)
	{
		LongPoint							Start;
		LongPoint							Extent;
		CAboutWindowPicture*	AboutWindowPicture;

		GetRect(AboutWindowLocationID,&Start,&Extent);
		IWindow(CenterRect(Extent,LongPointOf(screenBits.bounds.right
			- screenBits.bounds.left,screenBits.bounds.bottom - screenBits.bounds.top)),
			Extent,ModelessWindow,NoGrowable,NoZoomable);
		if (TheAutoFlag)
			{
				RegisterIdler(this,60);
			}
		AboutWindowPicture = new CAboutWindowPicture;
		AboutWindowPicture->IAboutWindowPicture(this);
		MomentOfInstantiation = TickCount();
		AutoFlag = TheAutoFlag;
		BecomeActiveWindow();
	}


void			CAboutWindow::DoIdle(long TimeSinceLastEvent)
	{
		if (AutoFlag)
			{
				if (TickCount()-MomentOfInstantiation > ShowDelay)
					{
						GoAway();
						return;
					}
			}
		inherited::DoIdle(TimeSinceLastEvent);
	}


void			CAboutWindow::GoAway(void)
	{
		delete this;
	}


MyBoolean	CAboutWindow::DoMenuCommand(ushort MenuCommandValue)
	{
		if (MenuCommandValue == mFileClose)
			{
				GoAway();
				return True;
			}
		return Application->DoMenuCommand(MenuCommandValue);
	}


void			CAboutWindow::EnableMenuItems(void)
	{
		MyEnableItem(mFileClose);
		Application->EnableMenuItems();
	}


/********************************************************************************/


void		CAboutWindowPicture::IAboutWindowPicture(CWindow* TheWindow)
	{
		LongPoint		Start;
		LongPoint		Extent;

		GetRect(PictureLocationID,&Start,&Extent);
		IViewRect(ZeroPoint,Extent,TheWindow,TheWindow);
	}


void		CAboutWindowPicture::DoUpdate(void)
	{
		PicHandle		Image;
		Handle			Text;
		Handle			VersRes;
		Handle			Text2;
		Handle			Text3;
		LongPoint		VersLocStart;
		LongPoint		VersLocExtent;
		LongPoint		VBoxLocStart;
		LongPoint		VBoxLocExtent;
		short				ReleaseType;
		short				NonReleaseVersion;

		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		SetUpPort();
		Image = (PicHandle)GetResource('�Cpc',PictureID);
		ERROR(Image==NIL,PRERR(ForceAbort,"�Cpc image resource not present."));
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		Window->LDrawPicture(Image,ZeroPoint,Extent);
		ReleaseResource((Handle)Image);
		Window->SetText(GetFontID("\pGeneva"),0,srcOr,9,0);
		VersRes = GetResource('vers',1); /* get application version information */
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		HLock(VersRes);
		ReleaseType = (*(uchar**)VersRes)[2]; /* pluck out release type */
		NonReleaseVersion = ((*(uchar**)VersRes)[3] & 0x0f)
			+ 10*(((*(uchar**)VersRes)[3] & 0xf0) >> 4); /* pluck out non release version */
		ERROR(VersRes==NIL,PRERR(ForceAbort,
			"CImagePane::DoUpdate couldn't find 'vers' 1 resource."));
		Text = PString2Handle(&(((uchar*)*VersRes)[6])); /* extract version string */
		HUnlock(VersRes);
		ReleaseResource(VersRes);
		Text2 = GetCString(ApplicationVersionStringID); /* get "Version " */
		Text3 = HStrCat(Text2,Text);
		ReleaseHandle(Text);
		ReleaseHandle(Text2); /* get rid of original handles */
		GetRect(ApplicationVersionLocationID,&VersLocStart,&VersLocExtent);
		VBoxLocStart.x = VersLocStart.x - 4;
		VBoxLocStart.y = VersLocStart.y - 4;
		VBoxLocExtent.x = VersLocExtent.x + 8;
		VBoxLocExtent.y = VersLocExtent.y + 8;
		Window->LEraseRect(VBoxLocStart,VBoxLocExtent);
		Window->LTextBox(VersLocStart,VersLocExtent,Text3,JustifyLeft);
		Window->LFrameRect(VBoxLocStart,VBoxLocExtent);
		ReleaseHandle(Text3);
		if (ReleaseType != FinalRelease)
			{
				switch (ReleaseType)
					{
						case DevelopmentRelease:
							Text = GetCString(DevelopmentStringID);
							break;
						case AlphaRelease:
							Text = GetCString(AlphaStringID);
							break;
						case BetaRelease:
							Text = GetCString(BetaStringID);
							break;
						default:
							PRERR(ForceAbort,"Invalid version resource.");
					}
				Text2 = Int2String(NonReleaseVersion);
				Text3 = HStrCat(Text,Text2);
				ReleaseHandle(Text);
				ReleaseHandle(Text2);
				GetRect(NonFinalLocID,&VersLocStart,&VersLocExtent);
				VBoxLocStart.x = VersLocStart.x - 4;
				VBoxLocStart.y = VersLocStart.y - 4;
				VBoxLocExtent.x = VersLocExtent.x + 8;
				VBoxLocExtent.y = VersLocExtent.y + 8;
				Window->LEraseRect(VBoxLocStart,VBoxLocExtent);
				Window->LTextBox(VersLocStart,VersLocExtent,Text3,JustifyLeft);
				Window->LFrameRect(VBoxLocStart,VBoxLocExtent);
				ReleaseHandle(Text3);
			}
	}
