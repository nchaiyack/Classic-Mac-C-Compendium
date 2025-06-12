/* CRewindButton.c */

#include "CRewindButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CRewindButton::IRewindButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(RewindIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalRewindPictID,HilitedRewindPictID,
			0,0,TheWindow,TheWindow);
	}


void				CRewindButton::DoEnter(void)
	{
		Document->DoRewind(True);
	}


void				CRewindButton::DoLeave(void)
	{
		Document->DoRewind(False);
	}
