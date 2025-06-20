/* CSkipToNextButton.c */

#include "CSkipToNextButton.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CMyDocument.h"


void				CSkipToNextButton::ISkipToNextButton(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SkipToNextIconLocID,&LocalStart,&LocalExtent);
		IPicture(LocalStart,LocalExtent,NormalSkipPictID,HilitedSkipPictID,
			0,0,TheWindow,TheWindow);
	}


MyBoolean		CSkipToNextButton::DoThang(void)
	{
		Document->DoSkipToNext();
		return False;
	}
