/* CDefaultStereoOn.c */

#include "CDefaultStereoOn.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CDefaultStereoOn::IDefaultStereoOn(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultStereoID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(DefaultStereoID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CDefaultStereoOn::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->StereoOn != State)
			{
				Document->SetDefaultStereoOn(State);
			}
		return Result;
	}
