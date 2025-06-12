/* CStopButton.c */

#include "CStopButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CStopButton::IStopButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(StopIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalStopPictID,HilitedStopPictID,
			0,0,TheWindow,TheWindow);
	}


MyBoolean		CStopButton::DoThang(void)
	{
		Document->DoStop();
		return False;
	}
