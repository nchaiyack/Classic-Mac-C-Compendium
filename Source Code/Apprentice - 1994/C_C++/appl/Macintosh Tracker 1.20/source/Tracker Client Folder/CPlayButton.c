/* CPlayButton.c */

#include "CPlayButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CPlayButton::IPlayButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(PlayPauseIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalPlayPictID,HilitedPlayPictID,
			0,0,TheWindow,TheWindow);
	}


MyBoolean		CPlayButton::DoThang(void)
	{
		Document->DoPlay();
		return False;
	}
