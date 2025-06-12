////////////
//
//	CChessSplashScreen.cp
//
//	The Chess++ Splash Screen Class
//	
//	SUPERCLASS = CDirector
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <CDecorator.h>
#include "CChessSplashScreen.h"

extern	CDesktop	*gDesktop;
extern	CDecorator	*gDecorator;

void	CChessSplashScreen::IChessSplashScreen(CDirectorOwner *aSupervisor)
{	
	CDirector::IDirector(aSupervisor);	/* Initialize superclass			*/
	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDChessSplashScreen, true, gDesktop, this);

	DoChessSplashScreen();
}
		
void	CChessSplashScreen::DoChessSplashScreen(void)
{
	PicHandle	thePict;
	Rect		picRect;

	thePict = GetPicture(WINDChessSplashScreen);
	picRect = (*thePict)->picFrame;
	itsWindow->ChangeSize(picRect.right-picRect.left,picRect.bottom-picRect.top);
	gDecorator->CenterWindow(itsWindow);
	itsWindow->Show();

	itsWindow->Prepare();
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	
	ReleaseResource((Handle)thePict);
}