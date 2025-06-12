/* CFastForwardButton.c */

#include "CFastForwardButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CFastForwardButton::IFastForwardButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(FastForwardIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalFastForwardPictID,HilitedFastForwardPictID,
			0,0,TheWindow,TheWindow);
	}


void				CFastForwardButton::DoEnter(void)
	{
		Document->DoFastForward(True);
	}


void				CFastForwardButton::DoLeave(void)
	{
		Document->DoFastForward(False);
	}
