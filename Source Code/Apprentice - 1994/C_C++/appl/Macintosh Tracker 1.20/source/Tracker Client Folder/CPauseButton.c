/* CPauseButton.c */

#include "CPauseButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CPauseButton::IPauseButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(PlayPauseIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalPausePictID,HilitedPausePictID,
			0,0,TheWindow,TheWindow);
	}


MyBoolean		CPauseButton::DoThang(void)
	{
		Document->DoPause();
		return False;
	}
